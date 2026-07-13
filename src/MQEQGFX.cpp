/*
 * MQEQGFX - MacroQuest plugin exposing the EQ graphics engine to Lua.
 *
 *
 * Lua interface: MQ2Lua resolves require('plugin.MQEQGFX') through our
 * exported CreateLuaModule (one sol table per Lua state, all wrapping the
 * shared native scene below). The eqgfx package's init.lua shim keeps
 * require('eqgfx') working unchanged for the feature scripts, and MQ2Lua
 * auto-terminates dependent scripts when this plugin unloads.
 *
 * HOW WE RENDER (DX11 client)
 * ---------------------------
 * World primitives are
 * projected to screen space through the engine's own camera
 * (CCamera::ProjectWorldCoordinatesToScreen) and drawn as 2D segments from
 * MacroQuest's GraphicsSceneRender callback - the sanctioned render hook.
 *
 * This file owns the plugin lifecycle, the retained scene + render callback,
 * the 3D portraits, the /eqgfx command, and the Lua module. The event taps
 * live in their own files: CombatNumbers.cpp (damage/heal), Spells.cpp
 * (cast-start + CastingData tracker), Models.cpp (toy with EQ models WIP). 
 * Shared state is declared in MQEQGFX.h.
 */

#include <mq/Plugin.h>

#include "MQEQGFX.h"

#include "eqlib/graphics/CameraInterface.h"
#include "eqlib/graphics/Render.h"
#include "eqlib/graphics/GraphicsEngine.h"
#include "eqlib/graphics/SceneGraphInterface.h"
#include "eqlib/graphics/ResourceManagerInterface.h"
#include "eqlib/graphics/Actors.h"

#include <sol/sol.hpp>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

PreSetup("MQEQGFX");
PLUGIN_VERSION(0.3);

namespace {

constexpr float kPi = 3.14159265358979323846f;

// ---------------------------------------------------------------------------
// Retained scene: Lua pushes primitives, the render callback draws them every
// frame until the next clear(). World coordinates are MQ/TLO (X, Y, Z), swapped
// to the engine's (Y, X, Z) at projection time. screen=true segments are
// already screen-space pixels (no projection).

struct Segment
{
	float x1, y1, z1;
	float x2, y2, z2;
	uint32_t color;
	bool screen;
};

struct ScreenRect
{
	float x0, y0, x1, y1;
	uint32_t color;
};

std::mutex              s_sceneMutex;
std::vector<Segment>    s_segments;
std::vector<ScreenRect> s_rects;
int                     s_thickness = 3;       // line half-width in pixels
std::atomic<uint32_t>   s_sceneCalls{ 0 };
std::atomic<uint32_t>   s_lastDraws{ 0 };

// ---------------------------------------------------------------------------
// Native UI window sweep (plate occlusion). Runs on the render thread and
// caches the rects; Lua's get_ui_rects() just snapshots them.

struct UiRect
{
	float x0, y0, x1, y1;
	int nameIdx;                                // 0-based index into s_uiNames
};

constexpr int kMaxUiRects = 128;

std::mutex               s_uiMutex;
std::vector<std::string> s_uiNames;
std::vector<UiRect>      s_uiRects;
int                      s_uiStatNames = 0;
int                      s_uiStatFound = 0;

// ---------------------------------------------------------------------------
// 3D portraits via the engine's ObjectPreview API: bind an actor to a slot,
// the engine renders it offscreen, blit the result into a UI rect. Two
// hardware slots (self + target). (Not working yet.)

enum { kPortraitSlots = 2 };
enum PortraitSubject { PS_NONE = 0, PS_SELF = 1, PS_TARGET = 2 };

struct PortraitConfig
{
	bool        enabled = false;
	int         subject = PS_SELF;
	std::string window;                          // e.g. "TargetWindow"; empty = explicit rect
	int         x0 = 0, y0 = 0, x1 = 0, y1 = 0;  // explicit pixel rect (used when window empty)
	uint32_t    color = 0xFFFFFFFF;
};

std::mutex     s_portraitMutex;                  // guards s_portraits (config)
PortraitConfig s_portraits[kPortraitSlots];

// Per-slot engine/actor state. Touched ONLY on the render thread (OnSceneRender)
// and in ShutdownPlugin after the render callback is removed, so it needs no lock.
struct PortraitEngineState
{
	bool             enabled = false;      // EnableObjectPreview(slot) state
	CActorInterface* actor = nullptr;      // plugin-owned standalone actor (DestroyActor to free)
	std::string      tag;                  // ActorDef tag the actor was built from
	void*            boundActor = nullptr; // last actor handed to SetObjectPreviewActor
};
PortraitEngineState s_portraitEngine[kPortraitSlots];

int         s_renderCallbackId = -1;
bool        s_active = false;
std::string s_lastArmor;                         // last showmodel arg (main thread only)

// ---------------------------------------------------------------------------
// Engine access

// World -> screen through the engine's own camera. Callers pass MQ/TLO order
// (x=X, y=Y); the engine's CVector3 is (Y, X, Z), matching /loc YXZ - swap
// the first two before projecting. Returns false when behind the camera.
bool WorldToScreen(float x, float y, float z, float& sx, float& sy)
{
	CDisplay* display = pDisplay.get();
	if (!display || !display->pCamera)
		return false;

	sx = sy = 0.0f;
	return display->pCamera->ProjectWorldCoordinatesToScreen(CVector3(y, x, z), sx, sy);
}

void GetScreenSize(int& width, int& height)
{
	CRender* render = g_pDrawHandler.get();
	width = render ? render->GetDisplayWidth() : 0;
	height = render ? render->GetDisplayHeight() : 0;
}

void DrawLine2DRaw(CRender* render, float x1, float y1, float x2, float y2, uint32_t color)
{
	render->DrawLine2D(CVector3(x1, y1, 0.0f), CVector3(x2, y2, 0.0f), color);
}

// Thick screen-space line: parallel copies offset along the perpendicular.
void DrawThickScreenLine(CRender* render, float x1, float y1, float x2, float y2, uint32_t color)
{
	float dx = x2 - x1, dy = y2 - y1;
	float len = sqrtf(dx * dx + dy * dy);
	if (len < 0.001f)
	{
		DrawLine2DRaw(render, x1, y1, x2, y2, color);
		return;
	}

	float px = -dy / len, py = dx / len;        // unit perpendicular
	for (int k = -s_thickness; k <= s_thickness; ++k)
		DrawLine2DRaw(render, x1 + px * k, y1 + py * k, x2 + px * k, y2 + py * k, color);
}

// Render-thread sweep of the candidate window list.
void UiSweep()
{
	std::scoped_lock lock(s_uiMutex);
	if (s_uiNames.empty())
		return;

	s_uiRects.clear();
	int idx = -1;
	for (const std::string& name : s_uiNames)
	{
		++idx;
		if (static_cast<int>(s_uiRects.size()) >= kMaxUiRects)
			break;

		CXWnd* wnd = FindMQ2Window(name.c_str());
		if (!wnd || !wnd->IsReallyVisible() || wnd->IsMinimized())
			continue;

		CXRect rect = wnd->GetScreenRect();
		if (rect.right <= rect.left || rect.bottom <= rect.top)
			continue;

		s_uiRects.push_back({ static_cast<float>(rect.left), static_cast<float>(rect.top),
			static_cast<float>(rect.right), static_cast<float>(rect.bottom), idx });
	}

	s_uiStatNames = static_cast<int>(s_uiNames.size());
	s_uiStatFound = static_cast<int>(s_uiRects.size());
}

// Release a slot's plugin-owned standalone actor: unbind it from the preview,
// then destroy it via the scene graph. Render thread only.
void ReleasePreviewActor(int slot, CRenderInterface* render, SGraphicsEngine* eng)
{
	PortraitEngineState& es = s_portraitEngine[slot];
	if (es.boundActor)
	{
		if (render)
			render->SetObjectPreviewActor(nullptr, slot);
		es.boundActor = nullptr;
	}
	if (es.actor)
	{
		if (eng && eng->pSceneGraph)
			eng->pSceneGraph->DestroyActor(es.actor);
		es.actor = nullptr;
	}
	es.tag.clear();
}

CActorInterface* AcquirePreviewActor(const PortraitConfig& cfg, int slot,
	CRenderInterface* render, SGraphicsEngine* eng)
{
	if (!eng || !eng->pResourceManager || !eng->pSceneGraph)
		return nullptr;

	PlayerClient* subject = (cfg.subject == PS_SELF)   ? pLocalPlayer.get()
	                      : (cfg.subject == PS_TARGET) ? pTarget.get()
	                                                   : nullptr;
	if (!subject)
		return nullptr;

	const char* tag = subject->mActorClient.ActorDef;   // resolved model tag, e.g. "HUM_ACTORDEF"
	if (!tag || !tag[0])
		return nullptr;

	PortraitEngineState& es = s_portraitEngine[slot];
	if (es.actor && es.tag == tag)
		return es.actor;                                // already built for this appearance

	// Subject/appearance changed -> tear down the old actor and rebuild.
	ReleasePreviewActor(slot, render, eng);

	CActorDefinitionInterface* def = nullptr;
	if (eng->pResourceManager->GetActorDefinition(tag, &def) != eStatusSuccess || def == nullptr)
		return nullptr;

	// Create at the local player's (valid) position so creation succeeds; we hide
	// it immediately and the preview renders it by pointer regardless of where it
	// nominally stands.
	CVector3 pos(0.0f, 0.0f, 0.0f), orient(0.0f, 0.0f, 0.0f);
	if (PlayerClient* me = pLocalPlayer.get())
		pos = CVector3(me->X, me->Y, me->Z);


	CActorInterface* actor = nullptr;
	CCollisionGroup grp(CCollisionGroup::eStatic);
	eng->pSceneGraph->CreateActorInstance(def, &pos, &orient, eCollisionVolumeNone,
		1.0f, 1.0f, 1.0f, false, true, true, true, &actor, grp, 0, false);
	if (actor == nullptr)
		return nullptr;


	es.actor = actor;
	es.tag = tag;
	return actor;
}


void DrawPortraits(CRender* render)
{
	SGraphicsEngine* eng = pGraphicsEngine.get();

	for (int slot = 0; slot < kPortraitSlots; ++slot)
	{
		PortraitConfig cfg;
		{
			std::scoped_lock lock(s_portraitMutex);
			cfg = s_portraits[slot];
		}

		CActorInterface* actor = cfg.enabled
			? AcquirePreviewActor(cfg, slot, render, eng)
			: nullptr;

		PortraitEngineState& es = s_portraitEngine[slot];

		// Nothing valid to show -> disable the slot (frees its RT) + drop the actor.
		if (actor == nullptr)
		{
			if (es.enabled)
			{
				render->EnableObjectPreview(false, slot);
				es.enabled = false;
			}
			ReleasePreviewActor(slot, render, eng);
			continue;
		}

		if (!es.enabled)
		{
			render->EnableObjectPreview(true, slot);      // lazily allocates the RT
			// The preview stays black until the per-slot camera exists;
			// EnableObjectPreview doesn't create it, so create it here (idempotent).
			if (eng && eng->pSceneGraph)
			{
				CCameraInterface* cam = nullptr;
				eng->pSceneGraph->CreateObjectPreviewCamera(&cam, static_cast<EObjectPreviewMode>(slot));
			}
			es.enabled = true;
			es.boundActor = nullptr;                      // force a re-bind below
		}
		if (es.boundActor != actor)
		{
			render->SetObjectPreviewActor(actor, slot);
			es.boundActor = actor;
		}

		// Destination rect: a tracked window's screen rect, or an explicit pixel
		// rect (the Lua ImGui overlay passes its content rect here).
		CXRect rect;
		bool haveRect = false;
		if (!cfg.window.empty())
		{
			if (CXWnd* wnd = FindMQ2Window(cfg.window.c_str()))
			{
				if (wnd->IsReallyVisible() && !wnd->IsMinimized())
				{
					rect = wnd->GetScreenRect();
					haveRect = true;
				}
			}
		}
		else
		{
			rect = CXRect(cfg.x0, cfg.y0, cfg.x1, cfg.y1);
			haveRect = rect.right > rect.left && rect.bottom > rect.top;
		}

		if (haveRect)
			render->DrawObjectPreview(rect, cfg.color, slot);
	}
}

void OnSceneRender()
{
	s_sceneCalls.fetch_add(1, std::memory_order_relaxed);

	if (GetGameState() != GAMESTATE_INGAME)
		return;

	UiSweep();

	CRender* render = g_pDrawHandler.get();
	CDisplay* display = pDisplay.get();
	if (!render || !display || !display->pCamera)
		return;

	std::scoped_lock lock(s_sceneMutex);
	uint32_t drawn = 0;

	for (const Segment& seg : s_segments)
	{
		if (seg.screen)
		{
			DrawThickScreenLine(render, seg.x1, seg.y1, seg.x2, seg.y2, seg.color);
			++drawn;
			continue;
		}

		// Draw only when both endpoints are in front of the camera.
		float ax, ay, bx, by;
		if (WorldToScreen(seg.x1, seg.y1, seg.z1, ax, ay)
			&& WorldToScreen(seg.x2, seg.y2, seg.z2, bx, by))
		{
			DrawThickScreenLine(render, ax, ay, bx, by, seg.color);
			++drawn;
		}
	}

	// Filled 2D rects (health bars etc.) - solid fill via 1px horizontal scanlines.
	for (const ScreenRect& rect : s_rects)
	{
		float y0 = std::min(rect.y0, rect.y1);
		float y1 = std::max(rect.y0, rect.y1);
		for (float yy = y0; yy <= y1; yy += 1.0f)
			DrawLine2DRaw(render, rect.x0, yy, rect.x1, yy, rect.color);
		++drawn;
	}

	DrawPortraits(render);

	s_lastDraws.store(drawn, std::memory_order_relaxed);
}

// Lua passes colors as the (possibly negative) int32 that LuaJIT bit.bor
// produces; normalize to the packed uint32 the engine wants.
uint32_t ToColor(int64_t color)
{
	return static_cast<uint32_t>(color & 0xFFFFFFFF);
}

void EqgfxCommand(PlayerClient*, const char* szLine)
{
	char szCmd[MAX_STRING] = { 0 };
	GetArg(szCmd, szLine, 1);
	if (ci_equals(szCmd, "capture"))
	{
		// Recording is always on; this toggles verbose logging.
		char szState[MAX_STRING] = { 0 };
		GetArg(szState, szLine, 2);
		if (ci_equals(szState, "off")) { s_captureVerbose = false; WriteChatf("\ag[MQEQGFX]\ax capture logging OFF (still recording)"); }
		else if (ci_equals(szState, "save")) { SaveCapture(); WriteChatf("\ag[MQEQGFX]\ax capture saved"); }
		else if (ci_equals(szState, "clear"))
		{
			std::scoped_lock lk(s_capMutex);
			s_matEvents.clear(); s_headMesh.clear(); s_headTint = 0; s_haveCapture = false;
			WriteChatf("\ag[MQEQGFX]\ax capture cleared");
		}
		else if (ci_equals(szState, "status"))
		{
			std::scoped_lock lk(s_capMutex);
			WriteChatf("\ag[MQEQGFX]\ax capture: %s, %d renames, head='%s' dye=%06X, actorDef='%s'",
				s_captureInstalled ? "installed" : "not installed", (int)s_matEvents.size(),
				s_headMesh.empty() ? "-" : s_headMesh.c_str(), s_headTint, s_captureActorDef.c_str());
		}
		else { InstallSwapCapture(); s_captureVerbose = true; WriteChatf("\ag[MQEQGFX]\ax capture logging \agON\ax (zone/re-equip to see calls). /eqgfx capture off to quiet."); }
		return;
	}
	if (ci_equals(szCmd, "dmgdebug"))
	{
		char szState[MAX_STRING] = { 0 };
		GetArg(szState, szLine, 2);
		bool on = !ci_equals(szState, "off");
		s_dmgDebug.store(on);
		WriteChatf("\ag[MQEQGFX]\ax damage debug %s (do a melee kick + a DoT, paste the raw lines)",
			on ? "\agON\ax" : "\ayOFF\ax");
		return;
	}
	if (ci_equals(szCmd, "dumpchar"))
	{
		DumpCharCommand();
		return;
	}
	if (ci_equals(szCmd, "dumpmodel"))
	{
		DumpModelCommand();
		return;
	}
	if (ci_equals(szCmd, "showmodel"))
	{
		// /eqgfx showmodel [off | plate|chain|cloth|leather|ring|scale]
		//   no arg  -> naked body; armor type -> merge that Velious set's lower body.
		char szState[MAX_STRING] = { 0 };
		GetArg(szState, szLine, 2);
		if (ci_equals(szState, "off"))
		{
			s_showModel.store(false);
			WriteChatf("\ag[MQEQGFX]\ax model view off");
			return;
		}
		std::string armor;
		for (const char* t : { "plate", "chain", "cloth", "leather", "ring", "scale" })
			if (ci_equals(szState, t)) { armor = t; break; }
		s_lastArmor = armor;
		std::string src, note;
		if (!BuildModelView(armor, src, note))
		{
			WriteChatf("\ay[MQEQGFX]\ax %s", note.c_str());
			return;
		}
		s_showModel.store(true);
		WriteChatf("\ag[MQEQGFX]\ax rendering %s (see the 'EQGFX Model' window)", src.c_str());
		return;
	}
	if (ci_equals(szCmd, "portrait"))
	{
		char szWho[MAX_STRING] = { 0 }, szState[MAX_STRING] = { 0 };
		GetArg(szWho, szLine, 2);
		GetArg(szState, szLine, 3);

		int slot, subject;
		const char* window;
		if (ci_equals(szWho, "target"))    { slot = 1; subject = PS_TARGET; window = "TargetWindow"; }
		else if (ci_equals(szWho, "self")) { slot = 0; subject = PS_SELF;   window = "PlayerWindow"; }
		else { WriteChatf("\ag[MQEQGFX]\ax usage: /eqgfx portrait self|target [on|off]"); return; }

		bool on = !ci_equals(szState, "off");
		{
			std::scoped_lock lock(s_portraitMutex);
			PortraitConfig& cfg = s_portraits[slot];
			cfg.enabled = on;
			cfg.subject = subject;
			cfg.window  = window;
			cfg.x0 = cfg.y0 = cfg.x1 = cfg.y1 = 0;
			cfg.color = 0xFFFFFFFF;
		}
		WriteChatf("\ag[MQEQGFX]\ax %s portrait %s", szWho, on ? "\agon\ax" : "\ayoff\ax");
		return;
	}

	int width = 0, height = 0;
	GetScreenSize(width, height);

	size_t segments, rects;
	{
		std::scoped_lock lock(s_sceneMutex);
		segments = s_segments.size();
		rects = s_rects.size();
	}
	size_t castQueued;
	{
		std::scoped_lock lock(s_castMutex);
		castQueued = s_castEvents.size();
	}

	WriteChatf("\ag[MQEQGFX]\ax build %s | screen %dx%d | render callback id %d",
		__DATE__ " " __TIME__, width, height, s_renderCallbackId);
	WriteChatf("\ag[MQEQGFX]\ax sceneCalls %u, lastDraws %u, queued: %d segments, %d rects",
		s_sceneCalls.load(std::memory_order_relaxed), s_lastDraws.load(std::memory_order_relaxed),
		static_cast<int>(segments), static_cast<int>(rects));
	WriteChatf("\ag[MQEQGFX]\ax ui sweep: %d names -> %d rects", s_uiStatNames, s_uiStatFound);
	for (int slot = 0; slot < kPortraitSlots; ++slot)
	{
		const PortraitEngineState& es = s_portraitEngine[slot];
		WriteChatf("\ag[MQEQGFX]\ax portrait slot %d: enabled=%d actor=%p tag='%s'",
			slot, es.enabled ? 1 : 0, (const void*)es.actor, es.tag.c_str());
	}
#ifdef CEverQuest__StartCasting_x
	const char* castMode = !s_castTapInstalled ? (s_castTapFailed ? "failed" : "inactive")
		: (s_castTrackerOwned ? "plugin-owned tracker" : "MQ2Main tap");
	WriteChatf("\ag[MQEQGFX]\ax StartCasting @ 0x%llX (%s), %d cast events queued",
		static_cast<unsigned long long>(CEverQuest__StartCasting), castMode, static_cast<int>(castQueued));
#else
	WriteChatf("\ag[MQEQGFX]\ax StartCasting offset not available in this build");
#endif
}

// ---------------------------------------------------------------------------
// The Lua module. Called once per Lua state; every table wraps the shared
// native state above (test/smoke.lua is the API contract).

sol::table BuildModuleTable(sol::state_view sv)
{
	sol::table m = sv.create_table();

	// Compatibility fields, both constant now.
	m["dll_stale"] = false;
	m["ui_native"] = true;

	// init() -> ok, err: whether the engine is ready to draw.
	m.set_function("init", []() {
		if (!s_active)
			return std::make_tuple(false, sol::optional<std::string>("MQEQGFX plugin is not active"));
		if (!g_pDrawHandler.get())
			return std::make_tuple(false, sol::optional<std::string>("render interface is null (not in game yet?)"));
		return std::make_tuple(true, sol::optional<std::string>());
	});

	// shutdown()/clear() just drop the queued primitives.
	auto clearScene = []() {
		std::scoped_lock lock(s_sceneMutex);
		s_segments.clear();
		s_rects.clear();
	};
	m.set_function("shutdown", clearScene);
	m.set_function("clear", clearScene);

	m.set_function("build", []() { return __DATE__ " " __TIME__; });

	m.set_function("stats", []() -> std::tuple<uint32_t, uint32_t> {
		return { s_sceneCalls.load(std::memory_order_relaxed), s_lastDraws.load(std::memory_order_relaxed) };
	});

	// -- world-space primitives ---------------------------------------------

	m.set_function("add_circle", [](float x, float y, float z, float radius, int64_t color, sol::optional<int> segments) {
		int segs = std::max(3, segments.value_or(48));
		uint32_t c = ToColor(color);

		std::scoped_lock lock(s_sceneMutex);
		float px = x + radius, py = y;
		for (int i = 1; i <= segs; ++i)
		{
			float t = (2.0f * kPi * i) / segs;
			float cx = x + radius * cosf(t), cy = y + radius * sinf(t);
			s_segments.push_back({ px, py, z, cx, cy, z, c, false });
			px = cx;
			py = cy;
		}
	});

	m.set_function("add_arc", [](float cx, float cy, float cz, float radius, float startRad, float endRad,
		int64_t color, sol::optional<int> segments)
	{
		int segs = std::max(1, segments.value_or(24));
		uint32_t c = ToColor(color);

		std::scoped_lock lock(s_sceneMutex);
		float ex0 = cx + radius * cosf(startRad), ey0 = cy + radius * sinf(startRad);
		s_segments.push_back({ cx, cy, cz, ex0, ey0, cz, c, false });
		float px = ex0, py = ey0;
		for (int i = 1; i <= segs; ++i)
		{
			float t = startRad + (endRad - startRad) * static_cast<float>(i) / segs;
			float ax = cx + radius * cosf(t), ay = cy + radius * sinf(t);
			s_segments.push_back({ px, py, cz, ax, ay, cz, c, false });
			px = ax;
			py = ay;
		}
		s_segments.push_back({ px, py, cz, cx, cy, cz, c, false });
	});

	m.set_function("add_line", [](float x1, float y1, float z1, float x2, float y2, float z2, int64_t color) {
		std::scoped_lock lock(s_sceneMutex);
		s_segments.push_back({ x1, y1, z1, x2, y2, z2, ToColor(color), false });
	});

	// -- screen-space primitives (pixels, no projection) ---------------------

	m.set_function("add_screen_line", [](float x1, float y1, float x2, float y2, int64_t color) {
		std::scoped_lock lock(s_sceneMutex);
		s_segments.push_back({ x1, y1, 0.0f, x2, y2, 0.0f, ToColor(color), true });
	});

	m.set_function("add_screen_rect", [](float x0, float y0, float x1, float y1, int64_t color) {
		std::scoped_lock lock(s_sceneMutex);
		s_rects.push_back({ x0, y0, x1, y1, ToColor(color) });
	});

	m.set_function("set_thickness", [](int t) { s_thickness = std::max(0, t); });

	// -- projection -----------------------------------------------------------

	// world_to_screen() -> sx, sy, visible: in front of the camera AND on screen.
	m.set_function("world_to_screen", [](float x, float y, float z) -> std::tuple<float, float, bool> {
		float sx = 0.0f, sy = 0.0f;
		bool visible = WorldToScreen(x, y, z, sx, sy);
		if (visible)
		{
			int width, height;
			GetScreenSize(width, height);
			visible = sx >= 0 && sx <= static_cast<float>(width) && sy >= 0 && sy <= static_cast<float>(height);
		}
		return { sx, sy, visible };
	});

	// project() -> sx, sy, infront: raw pixels (no viewport clamp) plus the
	// in-front-of-camera bool. Right test for shapes that go off screen.
	m.set_function("project", [](float x, float y, float z) -> std::tuple<float, float, bool> {
		float sx = 0.0f, sy = 0.0f;
		bool infront = WorldToScreen(x, y, z, sx, sy);
		return { sx, sy, infront };
	});

	m.set_function("get_screen", []() -> std::tuple<int, int> {
		int width, height;
		GetScreenSize(width, height);
		return { width, height };
	});

	// -- spell geometry -------------------------------------------------------

	m.set_function("spell_geom", [](sol::this_state L, int spellID) -> sol::object {
		ClientSpellManager* mgr = pSpellMgr.get();
		if (!mgr || spellID <= 0)
			return sol::make_object(L, sol::nil);

		EQ_Spell* spell = mgr->GetSpellByID(spellID);
		if (!spell)
			return sol::make_object(L, sol::nil);

		sol::state_view sv(L);
		sol::table t = sv.create_table(0, 5);
		t["targetType"] = static_cast<int>(spell->TargetType);
		t["range"] = spell->Range;
		t["aeRange"] = spell->AERange;
		t["coneStart"] = spell->ConeStartAngle;
		t["coneEnd"] = spell->ConeEndAngle;
		return t;
	});

	// -- native UI occlusion --------------------------------------------------

	m.set_function("set_ui_names", [](sol::table names) {
		std::vector<std::string> list;
		size_t count = names.size();
		list.reserve(count);
		for (size_t i = 1; i <= count; ++i)
		{
			sol::optional<std::string> name = names[i];
			if (name && !name->empty())
				list.push_back(std::move(*name));
		}

		std::scoped_lock lock(s_uiMutex);
		s_uiNames = std::move(list);
	});

	// get_ui_rects() -> { {x0,y0,x1,y1, idx=1-based-name-index}, ... }
	m.set_function("get_ui_rects", [](sol::this_state L) {
		std::vector<UiRect> snapshot;
		{
			std::scoped_lock lock(s_uiMutex);
			snapshot = s_uiRects;
		}

		sol::state_view sv(L);
		sol::table t = sv.create_table(static_cast<int>(snapshot.size()), 0);
		int i = 1;
		for (const UiRect& rect : snapshot)
		{
			sol::table rt = sv.create_table(4, 1);
			rt[1] = rect.x0;
			rt[2] = rect.y0;
			rt[3] = rect.x1;
			rt[4] = rect.y1;
			rt["idx"] = rect.nameIdx + 1;
			t[i++] = rt;
		}
		return t;
	});

	// Kept for API compatibility.
	m.set_function("ui_find_mode", []() { return 1; });
	m.set_function("ui_stats", []() -> std::tuple<int, int, int> {
		std::scoped_lock lock(s_uiMutex);
		return { s_uiStatNames, s_uiStatFound, 0 };
	});

	// -- colors ---------------------------------------------------------------

	// Same int32 bit pattern LuaJIT's bit.bor produced, so persisted settings
	// and comparisons behave identically.
	m.set_function("argb", [](int64_t a, int64_t r, int64_t g, int64_t b) -> int32_t {
		return static_cast<int32_t>((ToColor(a) << 24) | (ToColor(r) << 16) | (ToColor(g) << 8) | ToColor(b));
	});
	m.set_function("abgr", [](int64_t a, int64_t b, int64_t g, int64_t r) -> int32_t {
		return static_cast<int32_t>((ToColor(a) << 24) | (ToColor(b) << 16) | (ToColor(g) << 8) | ToColor(r));
	});

	// -- calibration/debug helpers (tools/) -----------------------------------

	m.set_function("get_eye", []() -> std::tuple<float, float, float> {
		CVector3 eye(0.0f, 0.0f, 0.0f);
		if (CRender* render = g_pDrawHandler.get())
			render->GetEyeOffset(eye);
		return { eye.X, eye.Y, eye.Z };
	});

	// Engine's world->camera transform. Raw coordinates, no TLO swap.
	m.set_function("world_to_camera", [](float x, float y, float z) -> std::tuple<float, float, float> {
		CVector3 cam(0.0f, 0.0f, 0.0f);
		if (CRender* render = g_pDrawHandler.get())
			render->TransformWorldToCamera(CVector3(x, y, z), cam);
		return { cam.X, cam.Y, cam.Z };
	});

	m.set_function("dump_matrix", [](sol::this_state L) {
		sol::state_view sv(L);
		sol::table t = sv.create_table(16, 0);
		if (CRender* render = g_pDrawHandler.get())
		{
			const float* f = reinterpret_cast<const float*>(&render->matrixViewProj);
			for (int i = 0; i < 16; ++i)
				t[i + 1] = f[i];
		}
		else
		{
			for (int i = 0; i < 16; ++i)
				t[i + 1] = 0.0f;
		}
		return t;
	});

	// No-op calibration knobs kept for API compatibility
	// (tools/calibrate.lua still calls them).
	auto noop = [](sol::variadic_args) {};
	m.set_function("set_axis_mode", noop);
	m.set_function("set_convention", noop);
	m.set_function("set_eyerel", noop);
	m.set_function("set_wsign", noop);
	m.set_function("set_flipx", noop);
	m.set_function("set_flipy", noop);

	// -- combat event feeds ---------------------------------------------------

	// Debug helper: resolved address of the cast-start handler, 0 if unavailable.
	m.set_function("start_casting_addr", []() -> double {
#ifdef CEverQuest__StartCasting_x
		return static_cast<double>(CEverQuest__StartCasting);
#else
		return 0.0;
#endif
	});

	// poll_casts() -> { {spellID=, spawnID=, castTimeMs=, timestamp=}, ... }
	// Drains the queue: single consumer by design.
	m.set_function("poll_casts", [](sol::this_state L) {
		std::vector<CastEvent> events;
		{
			std::scoped_lock lock(s_castMutex);
			events.swap(s_castEvents);
		}

		sol::state_view sv(L);
		sol::table t = sv.create_table(static_cast<int>(events.size()), 0);
		int i = 1;
		for (const CastEvent& event : events)
		{
			sol::table et = sv.create_table(0, 4);
			et["spellID"] = event.spellId;
			et["spawnID"] = event.spawnId;
			et["castTimeMs"] = event.castTimeMs;
			et["timestamp"] = event.timestampMs;
			t[i++] = et;
		}
		return t;
	});

	// poll_damage() -> { {attackerID=, targetID=, damage=, spellID=, skill=,
	//                     heal=, totalHeal=, flags=, timestamp=}, ... }
	// Drains the damage/heal queue: melee, spells, misses (damage <= 0), heals.
	// Single consumer by design.
	m.set_function("poll_damage", [](sol::this_state L) {
		std::vector<DamageEvent> events;
		{
			std::scoped_lock lock(s_dmgMutex);
			events.swap(s_dmgEvents);
		}

		sol::state_view sv(L);
		sol::table t = sv.create_table(static_cast<int>(events.size()), 0);
		int i = 1;
		for (const DamageEvent& event : events)
		{
			sol::table et = sv.create_table(0, 9);
			et["attackerID"] = event.attackerId;
			et["targetID"] = event.targetId;
			et["damage"] = event.damage;
			et["spellID"] = event.spellId;
			et["skill"] = event.skill;
			et["heal"] = event.heal;
			et["totalHeal"] = event.totalHeal;
			et["flags"] = event.flags;
			et["timestamp"] = event.timestampMs;
			t[i++] = et;
		}
		return t;
	});

	// -- 3D portraits (ObjectPreview) -----------------------------------------
	// opts: { enabled=true, subject='self'|'target'|'none',
	// window='PlayerWindow'/'TargetWindow' (tracked), OR x0,y0,x1,y1 pixel rect,
	// color=argb }
	m.set_function("portrait_slots", []() { return static_cast<int>(kPortraitSlots); });

	m.set_function("set_portrait", [](int slot, sol::table opts) {
		if (slot < 0 || slot >= kPortraitSlots) return;
		PortraitConfig cfg;
		cfg.enabled = opts["enabled"].get_or(true);
		std::string who = opts["subject"].get_or(std::string("self"));
		cfg.subject = (who == "target") ? PS_TARGET : (who == "none") ? PS_NONE : PS_SELF;
		cfg.window = opts["window"].get_or(std::string());
		cfg.x0 = opts["x0"].get_or(0);
		cfg.y0 = opts["y0"].get_or(0);
		cfg.x1 = opts["x1"].get_or(0);
		cfg.y1 = opts["y1"].get_or(0);
		cfg.color = ToColor(opts["color"].get_or(static_cast<int64_t>(0xFFFFFFFF)));
		std::scoped_lock lock(s_portraitMutex);
		s_portraits[slot] = std::move(cfg);
	});

	m.set_function("clear_portrait", [](int slot) {
		if (slot < 0 || slot >= kPortraitSlots) return;
		std::scoped_lock lock(s_portraitMutex);
		s_portraits[slot] = PortraitConfig{};
	});

	return m;
}

} // namespace

// Resolved by MQ2Lua's package loader for require('plugin.MQEQGFX').
PLUGIN_API bool CreateLuaModule(sol::this_state L, sol::object& outModule)
{
	outModule = BuildModuleTable(sol::state_view(L));
	return true;
}

// Background upkeep: lazy tap installs, capture priming, throttled saves.
PLUGIN_API void OnPulse()
{
	if (!s_active || GetGameState() != GAMESTATE_INGAME) return;

	if (!s_captureInstalled)
		InstallSwapCapture(/*auto_*/ true);

	// The taps install lazily: the game offsets resolve after startup.
	if (!s_dmgTapInstalled)
		InstallDamageTap();

	PulseCastTap();

	// Prime the capture from disk so a fresh session renders the saved look
	// without a rezone. Guarded so a missing file isn't reopened every frame.
	if (!s_haveCapture)
	{
		static std::string triedDef;
		if (PlayerClient* me = pLocalPlayer.get())
		{
			std::string def = me->mActorClient.ActorDef;
			if (!def.empty() && def != triedDef) { triedDef = def; LoadCapture(def); }
		}
	}

	static uint32_t tick = 0;
	if ((++tick % 256) == 0 && s_captureDirty.load(std::memory_order_relaxed))
		SaveCapture();

	// The ImGui sliders only set a flag; the rebuild does file I/O, so it runs here.
	if (s_reloadModel.exchange(false) && s_showModel.load(std::memory_order_relaxed))
	{
		std::string src, note;
		BuildModelView(s_lastArmor, src, note);
	}
}

PLUGIN_API void InitializePlugin()
{
	s_active = true;

	// Safe pre-device: the graphics engine defers device-dependent work and
	// only fires GraphicsSceneRender once the device is acquired.
	MQRenderCallbacks callbacks;
	callbacks.GraphicsSceneRender = OnSceneRender;
	s_renderCallbackId = AddRenderCallbacks(callbacks);

	InstallCastTap();

	// The damage tap installs lazily from OnPulse; the game offsets are still
	// null this early and patching a null address crashes on unload.

	AddCommand("/eqgfx", EqgfxCommand);
}

PLUGIN_API void ShutdownPlugin()
{
	s_active = false;

	RemoveCommand("/eqgfx");

	RemoveCastTap();

	if (s_renderCallbackId != -1)
	{
		RemoveRenderCallbacks(s_renderCallbackId);
		s_renderCallbackId = -1;
	}

	if (s_captureDirty.load(std::memory_order_relaxed))
		SaveCapture();
	RemoveSwapCapture();
	RemoveDamageTap();

	{
		std::scoped_lock lock(s_sceneMutex);
		s_segments.clear();
		s_rects.clear();
	}
	{
		std::scoped_lock lock(s_uiMutex);
		s_uiNames.clear();
		s_uiRects.clear();
	}
	{
		std::scoped_lock lock(s_castMutex);
		s_castEvents.clear();
	}
	{
		std::scoped_lock lock(s_dmgMutex);
		s_dmgEvents.clear();
	}

	// Safe: the render callback is already removed, DrawPortraits won't run again.
	{
		CRenderInterface* render = g_pDrawHandler.get();
		SGraphicsEngine*  eng = pGraphicsEngine.get();
		for (int slot = 0; slot < kPortraitSlots; ++slot)
		{
			ReleasePreviewActor(slot, render, eng);       // unbind + DestroyActor
			if (render)
				render->EnableObjectPreview(false, slot);
			s_portraitEngine[slot] = PortraitEngineState{};
		}
	}
	{
		std::scoped_lock lock(s_portraitMutex);
		for (PortraitConfig& cfg : s_portraits)
			cfg = PortraitConfig{};
	}

	// Model view (render callback + OnUpdateImGui already removed by MQ).
	s_showModel.store(false);
	s_modelView.ReleaseAll();
}
