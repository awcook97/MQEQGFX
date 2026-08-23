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
 * Segments that cross the camera's near plane are clipped rather than
 * dropped, so shapes drawn around the player stay whole (see SegKind below).
 * Nothing here depth-tests against the world.
 *
 * This file owns the plugin lifecycle, the retained scene + render callback,
 * the 3D portraits, the /eqgfx command, and the Lua module. The event taps
 * live in their own files: CombatNumbers.cpp (damage/heal), Spells.cpp
 * (cast-start + CastingData tracker), Models.cpp (toy with EQ models WIP),
 * Highlight.cpp (recoloring spawn models).
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

#include "gpu/LineFX.h"
#include "gpu/FxCatalog.h"
#include "gpu/FxCamera.h"

#include "eqlib/game/ClassInfo.h"

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
// to the engine's (Y, X, Z) at projection time.

// How a segment reaches the screen:
//   Seg_World   projected through the camera here, drawn as a thick 2D line.
//               Segments that cross the near plane are clipped, not dropped.
//   Seg_Screen  already pixels, drawn as-is.
//
// There is deliberately no "let the engine project it" kind. CRender's
// DrawLine3D (vtable 0x120) runs its endpoints through TransformWorldToCamera
// and appends the result to the same primitive buffer DrawLine2D feeds - but
// that buffer is rasterized as pre-transformed screen-space vertices, so
// camera-space coordinates land in a degenerate smear near the origin. It
// cannot draw anything useful on this client; don't reach for it.
enum SegKind : uint8_t
{
	Seg_World = 0,
	Seg_Screen = 1,
};

struct Segment
{
	float x1, y1, z1;
	float x2, y2, z2;
	uint32_t color;
	uint8_t kind;
};

struct ScreenRect
{
	float x0, y0, x1, y1;
	uint32_t color;
};
 
// DOUBLE BUFFERED, and it has to be.
//
// Lua builds a frame with a clear() followed by a run of add_* calls, while
// the render thread draws whenever the engine says so. With one shared buffer
// the render thread lands in the middle of a rebuild - after the clear, part
// way through the adds - and draws a half-built scene. Every frame. That
// reads as flicker and tearing, and no amount of care in the script can fix
// it, because the script does not control when the render callback fires.
//
// So: Lua writes s_back, the render thread reads s_front, and they are
// swapped in one locked step at a frame boundary. The render thread therefore
// only ever sees a scene somebody finished building.
//
// Threading: s_back is touched only from the main thread (MQ2Lua and the
// /eqgfx command both run there), so it needs no lock. s_front is shared, and
// the mutex now guards exactly two things - the swap and the draw - instead
// of being taken and released once per line.
struct SceneFrame
{
	std::vector<Segment>    segments;
	std::vector<ScreenRect> rects;

	void clear() { segments.clear(); rects.clear(); }
	bool empty() const { return segments.empty() && rects.empty(); }
};

std::mutex s_sceneMutex;        // guards s_front only
SceneFrame s_front;             // guarded - what the render thread draws
SceneFrame s_back;              // main thread only - what Lua is building

// Publish the built frame and start a new one. Called by clear() (every
// animated script's loop is clear -> add -> repeat, so that is the natural
// frame boundary) and by present().
void PresentScene(bool startNewFrame)
{
	{
		std::scoped_lock lock(s_sceneMutex);
		std::swap(s_front, s_back);
	}
	// s_back now holds the previous front buffer. Reusing it keeps its
	// capacity, so a steady scene stops allocating after the first few frames.
	if (startNewFrame)
		s_back.clear();
}
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

// ---------------------------------------------------------------------------
// GPU effect path. The CPU writes one record per effect and the vertex shader
// generates all of its geometry, which is what makes hundreds of animated
// spawns affordable. World segments coming from Lua ride the same buffer as
// FxForm_Segment instances, so custom scripts get the GPU path for free and
// keep working unchanged.
//
// Render thread only, except s_fxSubmit which producers fill under s_fxMutex.

eqgfx::gpu::LineFX                    s_lineFX;
std::mutex                            s_fxMutex;
std::vector<eqgfx::gpu::FxInstance>   s_fxSubmit;   // guarded - filled by Lua/OnPulse
std::vector<eqgfx::gpu::FxInstance>   s_fxDraw;     // render thread scratch
std::atomic<bool>                     s_fxEnabled{ true };
std::atomic<uint32_t>                 s_fxLastInstances{ 0 };
std::atomic<uint32_t>                 s_fxLastSegments{ 0 };
double                                s_fxStartTime = 0.0;

// Per-spawn driver for the compiled catalogue. This is what actually puts the
// shipped animations on screen: walk the spawn list, look each spawn's class up
// in FxCatalog, and submit one instance per enabled slot. Runs on OnPulse (main
// thread) because the spawn list is only safe there; the render thread just
// consumes whatever was last submitted.
struct FxDriverCfg
{
	bool  enabled     = true;
	bool  includePCs  = true;
	bool  npcs        = true;
	float radius      = 200.0f;
	int   maxSpawns   = 60;
	int   slotMask    = 0x1;   // bit N = show catalogue slot N (bit 0 = signature)
	float alpha       = 1.0f;
};
FxDriverCfg s_fxDriver;
std::atomic<uint32_t> s_fxDriverSpawns{ 0 };

// Debug: a big unmissable ring at your feet. Proves the whole pipeline -
// device, shader, matrix, blend, depth - without needing any other spawn.
std::atomic<bool> s_fxTest{ false };

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

void RenderGpuEffects();

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

	// Nothing is drawn through the engine any more: every primitive - world
	// lines, screen lines and filled rects alike - is generated on the GPU in
	// RenderGpuEffects below. The old path projected each segment on the CPU and
	// issued 2N+1 DrawLine2D calls per line, and filled a rect with one
	// DrawLine2D per pixel row, which is what made this too slow to use.

	DrawPortraits(render);

	RenderGpuEffects();

	s_lastDraws.store(s_fxLastInstances.load(std::memory_order_relaxed),
		std::memory_order_relaxed);
}

// GPU pass. Runs from the same callback, which MQ drives off its detour on
// CParticleSystem::Render - that lands after RenderScenePasses, so the depth
// buffer holds the world and our effects occlude against it properly. MQ wraps
// this callback in a full DX11 ScopedStateBlock, so we set state freely.
void RenderGpuEffects()
{
	if (!s_fxEnabled.load(std::memory_order_relaxed))
		return;
	if (!gbDeviceAcquired || gpD3D11Device == nullptr)
		return;
	if (!s_lineFX.EnsureInit(gpD3D11Device))
		return;

	float viewProj[16], camPos[3], camRight[3], camUp[3];
	if (!eqgfx::gpu::BuildViewProj(viewProj, camPos, camRight, camUp))
		return;

	// Snapshot the submit list; producers keep filling the other one.
	{
		std::scoped_lock lock(s_fxMutex);
		s_fxDraw = s_fxSubmit;
	}

	// World segments from the retained scene join the same draw as
	// FxForm_Segment, so the Lua API is GPU-accelerated without changing.
	{
		std::scoped_lock lock(s_sceneMutex);

		auto unpack = [](uint32_t c, float* rgba) {
			rgba[0] = ((c >> 16) & 0xFF) / 255.0f;
			rgba[1] = ((c >> 8) & 0xFF) / 255.0f;
			rgba[2] = (c & 0xFF) / 255.0f;
			rgba[3] = ((c >> 24) & 0xFF) / 255.0f;
		};
		const float halfWidth = static_cast<float>(std::max(1, s_thickness)) * 0.5f;

		for (const Segment& seg : s_front.segments)
		{
			if (s_fxDraw.size() >= eqgfx::gpu::kMaxInstances)
				break;

			eqgfx::gpu::FxInstance ix = {};
			ix.form = eqgfx::gpu::FxForm_Segment;
			ix.segments = 1;
			ix.speed = 1.0f;
			ix.width = halfWidth;
			unpack(seg.color, ix.rgba);

			if (seg.kind == Seg_Screen)
			{
				// Pixels straight to clip - no projection, always on top.
				ix.pos[0] = seg.x1; ix.pos[1] = seg.y1;
				ix.pos2[0] = seg.x2; ix.pos2[1] = seg.y2;
				ix.flags = eqgfx::gpu::FxFlag_ScreenSpace;
			}
			else
			{
				ix.pos[0] = seg.x1; ix.pos[1] = seg.y1; ix.pos[2] = seg.z1;
				ix.pos2[0] = seg.x2; ix.pos2[1] = seg.y2; ix.pos2[2] = seg.z2;
				ix.flags = eqgfx::gpu::FxFlag_ScreenWidth;
			}
			s_fxDraw.push_back(ix);
		}

		// Filled rects are one quad each now. The old path drew a horizontal
		// DrawLine2D per pixel row, so a 200px bar cost 200 engine calls.
		for (const ScreenRect& rect : s_front.rects)
		{
			if (s_fxDraw.size() >= eqgfx::gpu::kMaxInstances)
				break;
			eqgfx::gpu::FxInstance ix = {};
			ix.form = eqgfx::gpu::FxForm_Rect;
			ix.segments = 1;
			ix.speed = 1.0f;
			ix.pos[0] = rect.x0;  ix.pos[1] = rect.y0;
			ix.pos2[0] = rect.x1; ix.pos2[1] = rect.y1;
			ix.flags = eqgfx::gpu::FxFlag_ScreenSpace;
			unpack(rect.color, ix.rgba);
			s_fxDraw.push_back(ix);
		}
	}

	if (s_fxDraw.empty())
	{
		s_fxLastInstances.store(0, std::memory_order_relaxed);
		s_fxLastSegments.store(0, std::memory_order_relaxed);
		return;
	}

	int width = 0, height = 0;
	GetScreenSize(width, height);

	const float t = static_cast<float>(
		(double)::GetTickCount64() / 1000.0 - s_fxStartTime);

	s_lineFX.Render(s_fxDraw.data(), s_fxDraw.size(), viewProj, camPos,
		camRight, camUp, t,
		static_cast<float>(width), static_cast<float>(height));

	s_fxLastInstances.store(s_lineFX.LastInstanceCount(), std::memory_order_relaxed);
	s_fxLastSegments.store(s_lineFX.LastSegmentCount(), std::memory_order_relaxed);
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

	// focus / hovercast / focuscast / frames / units live in Units.cpp.
	if (UnitsCommand(szCmd, szLine))
		return;

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
	// /eqgfx highlight [off|<0xAARRGGBB>] [tint|ambient|both] - target-based
	// smoke test for the model tint path. Scripts use eqgfx.highlight().
	if (ci_equals(szCmd, "highlight"))
	{
		char szArg[MAX_STRING] = { 0 };
		GetArg(szArg, szLine, 2);

		if (ci_equals(szArg, "clear"))
		{
			ClearAllHighlights();
			WriteChatf("\ag[MQEQGFX]\ax highlights cleared");
			return;
		}

		PlayerClient* target = pTarget;
		if (!target)
		{
			WriteChatf("\ag[MQEQGFX]\ax highlight: no target");
			return;
		}

		if (ci_equals(szArg, "off"))
		{
			ClearSpawnHighlight(target->SpawnID);
			WriteChatf("\ag[MQEQGFX]\ax unhighlighted %s", target->DisplayedName);
			return;
		}

		uint32_t color = 0xFFFF3030;                 // default: red
		if (szArg[0])
			color = static_cast<uint32_t>(strtoull(szArg, nullptr, 0));

		char szMode[MAX_STRING] = { 0 };
		GetArg(szMode, szLine, 3);
		uint32_t mode = Highlight_Tint;
		if (ci_equals(szMode, "ambient"))
			mode = Highlight_Ambient;
		else if (ci_equals(szMode, "both"))
			mode = Highlight_Tint | Highlight_Ambient;

		SetSpawnHighlight(target->SpawnID, color, mode);
		WriteChatf("\ag[MQEQGFX]\ax highlighting %s color 0x%08X mode %u",
			target->DisplayedName, color, mode);
		return;
	}

	// /eqgfx fx [on|off|test|slots <mask>|radius <n>|max <n>|pcs|npcs|list]
	if (ci_equals(szCmd, "fx"))
	{
		char a1[MAX_STRING] = { 0 }, a2[MAX_STRING] = { 0 };
		GetArg(a1, szLine, 2);
		GetArg(a2, szLine, 3);

		if (ci_equals(a1, "off"))        { s_fxDriver.enabled = false; WriteChatf("\ag[MQEQGFX]\ax class fx OFF"); }
		else if (ci_equals(a1, "on"))    { s_fxDriver.enabled = true;  WriteChatf("\ag[MQEQGFX]\ax class fx ON"); }
		else if (ci_equals(a1, "test"))
		{
			bool on = !s_fxTest.load();
			s_fxTest.store(on);
			WriteChatf("\ag[MQEQGFX]\ax fx test ring %s - if this does not appear, the GPU path is broken, not the catalogue",
				on ? "\agON\ax" : "\ayOFF\ax");
		}
		else if (ci_equals(a1, "slots"))  { s_fxDriver.slotMask = (int)strtoul(a2, nullptr, 0); WriteChatf("\ag[MQEQGFX]\ax slot mask 0x%x", s_fxDriver.slotMask); }
		else if (ci_equals(a1, "radius")) { s_fxDriver.radius = (float)atof(a2); WriteChatf("\ag[MQEQGFX]\ax fx radius %.0f", s_fxDriver.radius); }
		else if (ci_equals(a1, "max"))    { s_fxDriver.maxSpawns = atoi(a2); WriteChatf("\ag[MQEQGFX]\ax fx max %d", s_fxDriver.maxSpawns); }
		else if (ci_equals(a1, "pcs"))    { s_fxDriver.includePCs = !s_fxDriver.includePCs; WriteChatf("\ag[MQEQGFX]\ax fx on PCs %d", s_fxDriver.includePCs); }
		else if (ci_equals(a1, "npcs"))   { s_fxDriver.npcs = !s_fxDriver.npcs; WriteChatf("\ag[MQEQGFX]\ax fx on NPCs %d", s_fxDriver.npcs); }
		else if (ci_equals(a1, "list"))
		{
			int n = 0;
			const eqgfx::gpu::ClassFx* all = eqgfx::gpu::AllClassFx(n);
			for (int i = 0; i < n; ++i)
			{
				char line[MAX_STRING] = { 0 };
				for (int j = 0; j < all[i].count; ++j)
				{
					strcat_s(line, all[i].defs[j].name);
					if (j + 1 < all[i].count) strcat_s(line, ", ");
				}
				WriteChatf("\ag[MQEQGFX]\ax %-4s (%d) %s", all[i].shortName, all[i].count, line);
			}
			WriteChatf("\ag[MQEQGFX]\ax %d compiled animations total", eqgfx::gpu::TotalFxCount());
		}
		else
		{
			WriteChatf("\ag[MQEQGFX]\ax class fx %s | %u spawns | %u instances, %u segments | gpu %s%s%s",
				s_fxDriver.enabled ? "\agon\ax" : "\ayoff\ax",
				s_fxDriverSpawns.load(), s_fxLastInstances.load(), s_fxLastSegments.load(),
				s_lineFX.Ready() ? "\agready\ax" : "\arnot ready\ax",
				*s_lineFX.LastError() ? " - " : "", s_lineFX.LastError());
			WriteChatf("\ag[MQEQGFX]\ax /eqgfx fx on|off|test|list|slots <mask>|radius <n>|max <n>|pcs|npcs");
		}
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
		segments = s_front.segments.size();
		rects = s_front.rects.size();
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
	WriteChatf("\ag[MQEQGFX]\ax highlights: %d active", HighlightCount());
	WriteChatf("\ag[MQEQGFX]\ax gpu fx: %s | %u instances, %u segments | catalog %d effects%s%s",
		s_lineFX.Ready() ? "\agready\ax" : "\aynot ready\ax",
		s_fxLastInstances.load(), s_fxLastSegments.load(),
		eqgfx::gpu::TotalFxCount(),
		*s_lineFX.LastError() ? " | " : "", s_lineFX.LastError());
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

	// clear() is the frame boundary: it publishes whatever was built since the
	// last one and starts a fresh build. That keeps the usual
	// clear -> add_* -> repeat loop working unchanged while making it
	// flicker-free, at the cost of the scene being one build behind - which is
	// invisible, since the alternative was showing a partial build.
	m.set_function("clear", []() { PresentScene(/*startNewFrame*/ true); });

	// present() publishes without starting a new build. For a scene you build
	// once and leave up: add_* the shapes, present(), stop touching it.
	m.set_function("present", []() { PresentScene(/*startNewFrame*/ false); });

	// blank() empties both buffers now, so the scene is gone this frame rather
	// than after the next clear() publishes an empty build. What you want when
	// a script pauses or switches off.
	auto blankScene = []() {
		s_back.clear();
		std::scoped_lock lock(s_sceneMutex);
		s_front.clear();
	};
	m.set_function("blank", blankScene);
	m.set_function("shutdown", blankScene);

	m.set_function("build", []() { return __DATE__ " " __TIME__; });

	m.set_function("stats", []() -> std::tuple<uint32_t, uint32_t> {
		return { s_sceneCalls.load(std::memory_order_relaxed), s_lastDraws.load(std::memory_order_relaxed) };
	});

	// -- world-space primitives ---------------------------------------------

	m.set_function("add_circle", [](float x, float y, float z, float radius, int64_t color,
		sol::optional<int> segments)
	{
		int segs = std::max(3, segments.value_or(48));
		uint32_t c = ToColor(color);

		float px = x + radius, py = y;
		for (int i = 1; i <= segs; ++i)
		{
			float t = (2.0f * kPi * i) / segs;
			float cx = x + radius * cosf(t), cy = y + radius * sinf(t);
			s_back.segments.push_back({ px, py, z, cx, cy, z, c, Seg_World });
			px = cx;
			py = cy;
		}
	});

	m.set_function("add_arc", [](float cx, float cy, float cz, float radius, float startRad, float endRad,
		int64_t color, sol::optional<int> segments)
	{
		int segs = std::max(1, segments.value_or(24));
		uint32_t c = ToColor(color);

		float ex0 = cx + radius * cosf(startRad), ey0 = cy + radius * sinf(startRad);
		s_back.segments.push_back({ cx, cy, cz, ex0, ey0, cz, c, Seg_World });
		float px = ex0, py = ey0;
		for (int i = 1; i <= segs; ++i)
		{
			float t = startRad + (endRad - startRad) * static_cast<float>(i) / segs;
			float ax = cx + radius * cosf(t), ay = cy + radius * sinf(t);
			s_back.segments.push_back({ px, py, cz, ax, ay, cz, c, Seg_World });
			px = ax;
			py = ay;
		}
		s_back.segments.push_back({ px, py, cz, cx, cy, cz, c, Seg_World });
	});

	m.set_function("add_line", [](float x1, float y1, float z1, float x2, float y2, float z2, int64_t color) {
		s_back.segments.push_back({ x1, y1, z1, x2, y2, z2, ToColor(color), Seg_World });
	});

	// -- screen-space primitives (pixels, no projection) ---------------------

	m.set_function("add_screen_line", [](float x1, float y1, float x2, float y2, int64_t color) {
		s_back.segments.push_back({ x1, y1, 0.0f, x2, y2, 0.0f, ToColor(color), Seg_Screen });
	});

	m.set_function("add_screen_rect", [](float x0, float y0, float x1, float y1, int64_t color) {
		s_back.rects.push_back({ x0, y0, x1, y1, ToColor(color) });
	});

	m.set_function("set_thickness", [](int t) { s_thickness = std::max(0, t); });

	// -- model highlighting ---------------------------------------------------

	// highlight(spawnID, color[, mode]) - recolor the spawn's actual 3D model.
	// mode is a bitmask: 1 tints every material, 2 adds a constant ambient
	// (glow), 3 both. Defaults to tint. The look is written on the next pulse
	// and rewritten whenever the model reloads; it is NOT part of the retained
	// scene, so clear() does not remove it - use unhighlight().
	m.set_function("highlight", [](int spawnId, int64_t color, sol::optional<int> mode) {
		SetSpawnHighlight(spawnId, ToColor(color),
			static_cast<uint32_t>(mode.value_or(static_cast<int>(Highlight_Tint))));
	});

	m.set_function("unhighlight", [](int spawnId) { ClearSpawnHighlight(spawnId); });
	m.set_function("clear_highlights", []() { ClearAllHighlights(); });

	// highlights() -> { {id=, color=, mode=, applied=}, ... }
	m.set_function("highlights", [](sol::this_state L) {
		sol::state_view sv(L);
		sol::table out = sv.create_table();
		int i = 1;
		for (const auto& [id, color, mode, applied] : HighlightList())
		{
			sol::table row = sv.create_table();
			row["id"] = id;
			row["color"] = static_cast<int64_t>(color);
			row["mode"] = static_cast<int>(mode);
			row["applied"] = applied;
			out[i++] = row;
		}
		return out;
	});

	// -- GPU effects ----------------------------------------------------------

	// fx_clear() then fx_add(...) each pass, mirroring clear()/add_*. Effects
	// are generated on the GPU: the cost of one is a struct write, so segment
	// counts and spawn counts stop being a budget.
	m.set_function("fx_clear", []() {
		std::scoped_lock lock(s_fxMutex);
		s_fxSubmit.clear();
	});

	// fx_add(form, x, y, z, opts) - opts is a table of any of:
	//   radius height speed tilt phase width segments color
	//   comet additive nodepth reverse screenwidth
	//   p0 p1 p2 p3   (form-specific; see gpu/LineFX.h)
	m.set_function("fx_add", [](int form, float x, float y, float z, sol::optional<sol::table> opts) {
		if (form < 0 || form >= static_cast<int>(eqgfx::gpu::FxForm_Count))
			return;
		std::scoped_lock lock(s_fxMutex);
		if (s_fxSubmit.size() >= eqgfx::gpu::kMaxInstances)
			return;

		eqgfx::gpu::FxInstance ix = {};
		ix.pos[0] = x; ix.pos[1] = y; ix.pos[2] = z;
		ix.form = static_cast<uint32_t>(form);
		ix.radius = 3.0f; ix.height = 4.0f; ix.speed = 1.0f;
		ix.segments = 40; ix.width = 0.05f;
		ix.rgba[0] = ix.rgba[1] = ix.rgba[2] = ix.rgba[3] = 1.0f;

		if (opts)
		{
			sol::table o = *opts;
			ix.radius   = o.get_or("radius", ix.radius);
			ix.height   = o.get_or("height", ix.height);
			ix.speed    = o.get_or("speed", ix.speed);
			ix.tilt     = o.get_or("tilt", 0.0f);
			ix.phase    = o.get_or("phase", 0.0f);
			ix.width    = o.get_or("width", ix.width);
			ix.segments = static_cast<uint32_t>(o.get_or("segments", 40));
			ix.p0 = o.get_or("p0", 0.0f); ix.p1 = o.get_or("p1", 0.0f);
			ix.p2 = o.get_or("p2", 0.0f); ix.p3 = o.get_or("p3", 0.0f);

			if (sol::optional<int64_t> c = o["color"])
			{
				const uint32_t v = ToColor(*c);
				ix.rgba[0] = ((v >> 16) & 0xFF) / 255.0f;
				ix.rgba[1] = ((v >> 8) & 0xFF) / 255.0f;
				ix.rgba[2] = (v & 0xFF) / 255.0f;
				ix.rgba[3] = ((v >> 24) & 0xFF) / 255.0f;
			}
			if (o.get_or("comet", false))       ix.flags |= eqgfx::gpu::FxFlag_Comet;
			if (o.get_or("additive", false))    ix.flags |= eqgfx::gpu::FxFlag_Additive;
			if (o.get_or("nodepth", false))     ix.flags |= eqgfx::gpu::FxFlag_NoDepth;
			if (o.get_or("reverse", false))     ix.flags |= eqgfx::gpu::FxFlag_Reverse;
			if (o.get_or("screenwidth", false)) ix.flags |= eqgfx::gpu::FxFlag_ScreenWidth;

			if (sol::optional<sol::table> p2t = o["to"])   // FxForm_Segment endpoint
			{
				ix.pos2[0] = (*p2t).get_or(1, x);
				ix.pos2[1] = (*p2t).get_or(2, y);
				ix.pos2[2] = (*p2t).get_or(3, z);
			}
		}
		s_fxSubmit.push_back(ix);
	});

	// fx_preset(class, index, x, y, z, phase, alpha) - drop one of the compiled
	// catalogue entries at a position. This is how a script reuses a shipped
	// look instead of rebuilding it by hand.
	m.set_function("fx_preset", [](const char* cls, int index, float x, float y, float z,
		sol::optional<float> phase, sol::optional<float> alpha)
	{
		const eqgfx::gpu::ClassFx* c = eqgfx::gpu::FindClassFx(cls);
		if (!c || index < 0 || index >= c->count)
			return false;
		std::scoped_lock lock(s_fxMutex);
		if (s_fxSubmit.size() >= eqgfx::gpu::kMaxInstances)
			return false;
		const float pos[3] = { x, y, z };
		eqgfx::gpu::FxInstance ix;
		eqgfx::gpu::MakeInstance(c->defs[index], pos, phase.value_or(0.0f),
			alpha.value_or(1.0f), ix);
		s_fxSubmit.push_back(ix);
		return true;
	});

	// fx_catalog() -> { WIZ = { "Arcane", "Fire", ... }, ... }
	m.set_function("fx_catalog", [](sol::this_state L) {
		sol::state_view sv(L);
		sol::table out = sv.create_table();
		int n = 0;
		const eqgfx::gpu::ClassFx* all = eqgfx::gpu::AllClassFx(n);
		for (int i = 0; i < n; ++i)
		{
			sol::table names = sv.create_table();
			for (int j = 0; j < all[i].count; ++j)
				names[j + 1] = all[i].defs[j].name;
			out[all[i].shortName] = names;
		}
		return out;
	});

	m.set_function("fx_enabled", [](sol::optional<bool> on) {
		if (on) s_fxEnabled.store(*on);
		return s_fxEnabled.load();
	});

	m.set_function("fx_stats", []() -> std::tuple<uint32_t, uint32_t, bool, std::string> {
		return { s_fxLastInstances.load(), s_fxLastSegments.load(),
			s_lineFX.Ready(), std::string(s_lineFX.LastError()) };
	});

	m["FX_RING"]    = static_cast<int>(eqgfx::gpu::FxForm_Ring);
	m["FX_HELIX"]   = static_cast<int>(eqgfx::gpu::FxForm_Helix);
	m["FX_ORBIT"]   = static_cast<int>(eqgfx::gpu::FxForm_Orbit);
	m["FX_POLYGON"] = static_cast<int>(eqgfx::gpu::FxForm_Polygon);
	m["FX_BLOOM"]   = static_cast<int>(eqgfx::gpu::FxForm_Bloom);
	m["FX_SPIRE"]   = static_cast<int>(eqgfx::gpu::FxForm_Spire);
	m["FX_LATTICE"] = static_cast<int>(eqgfx::gpu::FxForm_Lattice);
	m["FX_SEGMENT"] = static_cast<int>(eqgfx::gpu::FxForm_Segment);

	// --- unit frames / hover healing -------------------------------------
	// Exposed so a script can build its own frames, or bind a hovercast to
	// something smarter than a fixed gem, without giving up the C++ path.

	m.set_function("focus", [](sol::optional<int> id) {
		if (id) SetSecondaryTarget(*id);
		return GetSecondaryTarget();
	});

	m.set_function("hover", [](sol::optional<int> id) {
		if (id) SetHoverUnit(*id);
		return GetHoverUnit();
	});

	// hover -> focus -> target, the WoW mouseover-macro fallback chain.
	m.set_function("cast_unit", []() { return ResolveCastUnit(); });

	// Cast a memorised gem (1-based) at any spawn without changing target.
	m.set_function("cast_at", [](int spawnId, int gem) {
		return CastAtSpawn(spawnId, gem);
	});

	// spellId, elapsedMs, castMs - nil when that spawn isn't casting.
	m.set_function("unit_cast", [](int spawnId, sol::this_state L)
		-> sol::object {
		UnitCast uc;
		if (!GetSpawnCast(spawnId, uc))
			return sol::lua_nil;
		sol::state_view sv(L);
		sol::table t = sv.create_table();
		t["spell"] = uc.spellId;
		t["start"] = uc.startMs;
		t["total"] = uc.castMs;
		return t;
	});

	m["HIGHLIGHT_TINT"] = static_cast<int>(Highlight_Tint);
	m["HIGHLIGHT_AMBIENT"] = static_cast<int>(Highlight_Ambient);

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

// Stable per-spawn phase, so two wizards look like wizards without pulsing in
// lockstep. Same trick the Lua path used, just hashed here instead.
float FxPhaseFor(int spawnId)
{
	uint32_t h = static_cast<uint32_t>(spawnId) * 2654435761u;
	h ^= h >> 15;
	h *= 2246822519u;
	h ^= h >> 13;
	return (h & 0xFFFFFF) / static_cast<float>(0x1000000);
}

// Walk the spawn list and submit catalogue instances. Main thread only.
void PulseClassFx()
{
	std::vector<eqgfx::gpu::FxInstance> out;

	const FxDriverCfg cfg = s_fxDriver;
	PlayerClient* me = pLocalPlayer.get();

	if (s_fxTest.load(std::memory_order_relaxed) && me)
	{
		// Deliberately large, bright and additive so it cannot be mistaken for
		// nothing. If this does not appear, the problem is the pipeline, not
		// the catalogue or the spawn scan.
		eqgfx::gpu::FxInstance ix = {};
		ix.pos[0] = me->X; ix.pos[1] = me->Y; ix.pos[2] = me->Z;
		ix.form = eqgfx::gpu::FxForm_Ring;
		ix.radius = 12.0f; ix.height = 1.0f; ix.speed = 1.0f;
		ix.segments = 64; ix.width = 0.35f;
		ix.p1 = 0.6f; ix.p2 = 1.0f; ix.p3 = 0.25f;
		ix.flags = eqgfx::gpu::FxFlag_Comet | eqgfx::gpu::FxFlag_Additive
			| eqgfx::gpu::FxFlag_NoDepth;
		ix.rgba[0] = 1.0f; ix.rgba[1] = 0.2f; ix.rgba[2] = 0.9f; ix.rgba[3] = 1.0f;
		out.push_back(ix);
	}

	if (cfg.enabled && me)
	{
		const float r2 = cfg.radius * cfg.radius;
		int taken = 0;
		for (PlayerClient* sp = pSpawnList; sp && taken < cfg.maxSpawns; sp = sp->pNext)
		{
			if (sp == me)
				continue;

			const eSpawnType type = GetSpawnType(sp);
			const bool isPC = (type == PC || type == MERCENARY);
			if (isPC ? !cfg.includePCs : (type != NPC || !cfg.npcs))
				continue;

			const float dx = sp->X - me->X, dy = sp->Y - me->Y, dz = sp->Z - me->Z;
			if (dx * dx + dy * dy + dz * dz > r2)
				continue;

			const int cls = sp->GetClass();
			if (cls <= 0 || cls >= (int)(sizeof(eqlib::ClassInfo) / sizeof(eqlib::ClassInfo[0])))
				continue;
			const eqgfx::gpu::ClassFx* fx =
				eqgfx::gpu::FindClassFx(eqlib::ClassInfo[cls].UCShortName);
			if (!fx)
				continue;

			const float pos[3] = { sp->X, sp->Y, sp->Z };
			const float phase = FxPhaseFor(sp->SpawnID);
			for (int slot = 0; slot < fx->count && slot < 32; ++slot)
			{
				if ((cfg.slotMask & (1 << slot)) == 0)
					continue;
				if (out.size() >= eqgfx::gpu::kMaxInstances)
					break;
				eqgfx::gpu::FxInstance ix;
				eqgfx::gpu::MakeInstance(fx->defs[slot], pos, phase, cfg.alpha, ix);
				out.push_back(ix);
			}
			++taken;
		}
		s_fxDriverSpawns.store(static_cast<uint32_t>(taken), std::memory_order_relaxed);
	}
	else
	{
		s_fxDriverSpawns.store(0, std::memory_order_relaxed);
	}

	// The focus marker is appended here rather than submitted separately: this
	// vector is swapped wholesale each pulse, so a second producer would just
	// lose the race.
	BuildUnitMarkers(out);

	std::scoped_lock lock(s_fxMutex);
	s_fxSubmit.swap(out);
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

	// Apply new highlights and reap despawned ones every pulse; the periodic
	// rewrite below covers material rebuilds that keep the same actor.
	PulseHighlights();
	PulseClassFx();
	PulseUnits();

	static uint32_t tick = 0;
	++tick;
	if ((tick % 64) == 0)
		RefreshHighlights();
	if ((tick % 256) == 0 && s_captureDirty.load(std::memory_order_relaxed))
		SaveCapture();

	// The ImGui sliders only set a flag; the rebuild does file I/O, so it runs here.
	if (s_reloadModel.exchange(false) && s_showModel.load(std::memory_order_relaxed))
	{
		std::string src, note;
		BuildModelView(s_lastArmor, src, note);
	}
}

// A despawn takes its actor with it, so there is nothing to restore - just
// drop the entry before the spawn ID gets recycled onto something else.
PLUGIN_API void OnRemoveSpawn(PlayerClient* pSpawn)
{
	if (pSpawn)
	{
		OnHighlightSpawnRemoved(pSpawn->SpawnID);
		OnUnitSpawnRemoved(pSpawn->SpawnID);
	}
}

// Zoning destroys every actor in the old zone. Forget rather than restore:
// the pointers we would write through are already dead.
PLUGIN_API void OnBeginZone()
{
	ForgetHighlights();
}

PLUGIN_API void InitializePlugin()
{
	s_active = true;

	// Safe pre-device: the graphics engine defers device-dependent work and
	// only fires GraphicsSceneRender once the device is acquired.
	MQRenderCallbacks callbacks;
	callbacks.GraphicsSceneRender = OnSceneRender;
	// The GPU objects are device-dependent: drop them on a reset and let the
	// next frame rebuild them, or we render with dangling shaders.
	callbacks.CreateDeviceObjects = []() { /* rebuilt lazily by EnsureInit */ };
	callbacks.InvalidateDeviceObjects = []() { s_lineFX.ReleaseAll(); };
	s_renderCallbackId = AddRenderCallbacks(callbacks);

	s_fxStartTime = (double)::GetTickCount64() / 1000.0;

	InstallCastTap();
	InitUnits();

	// The damage tap installs lazily from OnPulse; the game offsets are still
	// null this early and patching a null address crashes on unload.

	AddCommand("/eqgfx", EqgfxCommand);
	AddCommand("/attarget", AtTargetCommand);
}

PLUGIN_API void ShutdownPlugin()
{
	s_active = false;

	RemoveCommand("/eqgfx");
	RemoveCommand("/attarget");

	RemoveCastTap();
	ShutdownUnits();

	if (s_renderCallbackId != -1)
	{
		RemoveRenderCallbacks(s_renderCallbackId);
		s_renderCallbackId = -1;
	}

	// Put every highlighted model back before we go; nothing else will.
	ClearAllHighlights();

	if (s_captureDirty.load(std::memory_order_relaxed))
		SaveCapture();
	RemoveSwapCapture();
	RemoveDamageTap();

	{
		s_back.clear();
		std::scoped_lock lock(s_sceneMutex);
		s_front.clear();
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

	// GPU effects: the render callback is already removed, so nothing else will
	// touch these.
	{
		std::scoped_lock lock(s_fxMutex);
		s_fxSubmit.clear();
	}
	s_fxDraw.clear();
	s_lineFX.ReleaseAll();

	// Model view (render callback + OnUpdateImGui already removed by MQ).
	s_showModel.store(false);
	s_modelView.ReleaseAll();
}
