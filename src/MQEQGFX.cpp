#include <mq/Plugin.h>

#include "MQEQGFX.h"
#include "SettingsFile.h"

#include "eqlib/graphics/CameraInterface.h"
#include "eqlib/graphics/SceneGraphInterface.h"
#include "eqlib/graphics/Actors.h"

#include "gpu/LineFX.h"
#include "gpu/FxCatalog.h"
#include "gpu/FxCamera.h"
#include "gpu/Glyphs.h"
#include "unitframes/UnitFrames.h"
#include "ui/NativeUi.h"

#include <sol/sol.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

PreSetup("MQEQGFX");
PLUGIN_VERSION(0.3);

namespace {

constexpr float kPi = 3.14159265358979323846f;

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
 
struct SceneFrame
{
	std::vector<Segment>    segments;
	std::vector<ScreenRect> rects;

	void clear() { segments.clear(); rects.clear(); }
	bool empty() const { return segments.empty() && rects.empty(); }
};

std::mutex s_sceneMutex;
SceneFrame s_front;
SceneFrame s_back;

void PresentScene(bool startNewFrame)
{
	{
		std::scoped_lock lock(s_sceneMutex);
		std::swap(s_front, s_back);
	}
	if (startNewFrame)
		s_back.clear();
}

constexpr int kMaxArcSegments = 128;

bool SceneFull()
{
	return s_back.segments.size() + s_back.rects.size() >= eqgfx::gpu::kMaxInstances;
}

int                     s_thickness = 3;
std::atomic<uint32_t>   s_sceneCalls{ 0 };
std::atomic<uint32_t>   s_lastDraws{ 0 };

struct UiRect
{
	float x0, y0, x1, y1;
	int nameIdx;
};

constexpr int kMaxUiRects = 128;
constexpr size_t kMaxWindowNameLen = 200;

std::mutex               s_uiMutex;
std::vector<std::string> s_uiNames;

struct UiNameList
{
	std::vector<std::string> names;
	std::vector<int>         fromUnion;
};
std::unordered_map<const void*, UiNameList> s_uiNameLists;
std::vector<UiRect>      s_uiRects;
int                      s_uiStatNames = 0;
int                      s_uiStatFound = 0;
std::atomic<bool>        s_uiRectsWanted{ false };

void RebuildUiNames()
{
	std::unordered_map<std::string, int> index;
	s_uiNames.clear();
	for (const auto& [listOwner, list] : s_uiNameLists)
	{
		for (const std::string& name : list.names)
		{
			if (index.emplace(name, static_cast<int>(s_uiNames.size())).second)
				s_uiNames.push_back(name);
		}
	}
	for (auto& [listOwner, list] : s_uiNameLists)
	{
		list.fromUnion.assign(s_uiNames.size(), -1);
		for (int i = 0; i < static_cast<int>(list.names.size()); ++i)
		{
			int& slot = list.fromUnion[index[list.names[i]]];
			if (slot < 0)
				slot = i;
		}
	}
	s_uiRects.clear();
}

enum { kPortraitSlots = 2 };
enum PortraitSubject { PS_NONE = 0, PS_SELF = 1, PS_TARGET = 2 };

struct PortraitConfig
{
	bool        enabled = false;
	int         subject = PS_SELF;
	std::string window;
	int         x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	uint32_t    color = 0xFFFFFFFF;
};

std::mutex     s_portraitMutex;
PortraitConfig s_portraits[kPortraitSlots];

struct PortraitEngineState
{
	bool             enabled = false;
	CActorInterface* actor = nullptr;
	std::string      tag;
	void*            boundActor = nullptr;
};
PortraitEngineState s_portraitEngine[kPortraitSlots];

eqgfx::gpu::LineFX                    s_lineFX;
std::mutex                            s_fxMutex;
std::vector<eqgfx::gpu::FxInstance>   s_fxSubmit;
std::vector<eqgfx::gpu::FxInstance>   s_fxLua;
std::vector<eqgfx::gpu::FxInstance>   s_fxDraw;
std::atomic<bool>                     s_fxEnabled{ true };
std::atomic<uint32_t>                 s_fxLastInstances{ 0 };
std::atomic<uint32_t>                 s_fxLastSegments{ 0 };
double                                s_fxStartTime = 0.0;

struct FxDriverCfg
{
	bool  enabled     = false;
	bool  includePCs  = true;
	bool  npcs        = true;
	float radius      = 200.0f;
	int   maxSpawns   = 60;
	int   slotMask    = 0x1;
	AlwaysOnStyle style;
};
FxDriverCfg s_fxDriver;
std::atomic<uint32_t> s_fxDriverSpawns{ 0 };

std::atomic<bool> s_fxTest{ false };

int         s_renderCallbackId = -1;
bool        s_active = false;
std::string s_lastArmor;

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

void UiSweep()
{
	if (!s_uiRectsWanted.exchange(false, std::memory_order_relaxed))
		return;

	int width = 0, height = 0;
	GetScreenSize(width, height);
	const float sw = static_cast<float>(width), sh = static_cast<float>(height);

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

		const CXRect rect = wnd->GetScreenRect();
		float x0 = static_cast<float>(rect.left), y0 = static_cast<float>(rect.top);
		float x1 = static_cast<float>(rect.right), y1 = static_cast<float>(rect.bottom);
		if (sw > 0.0f && sh > 0.0f)
		{
			x0 = std::max(x0, 0.0f);
			y0 = std::max(y0, 0.0f);
			x1 = std::min(x1, sw);
			y1 = std::min(y1, sh);
			if ((x1 - x0) * (y1 - y0) >= sw * sh * 0.9f)
				continue;
		}
		if (x1 <= x0 || y1 <= y0)
			continue;

		s_uiRects.push_back({ x0, y0, x1, y1, idx });
	}

	s_uiStatNames = static_cast<int>(s_uiNames.size());
	s_uiStatFound = static_cast<int>(s_uiRects.size());
}

void SubtractRect(std::vector<std::array<float, 4>>& pieces, const UiRect& w, size_t cap)
{
	std::vector<std::array<float, 4>> next;
	for (const std::array<float, 4>& r : pieces)
	{
		if (w.x0 >= r[2] || w.x1 <= r[0] || w.y0 >= r[3] || w.y1 <= r[1])
		{
			next.push_back(r);
			continue;
		}
		const float iy0 = std::max(r[1], w.y0), iy1 = std::min(r[3], w.y1);
		if (r[1] < iy0) next.push_back({ r[0], r[1], r[2], iy0 });
		if (iy1 < r[3]) next.push_back({ r[0], iy1, r[2], r[3] });
		const float ix0 = std::max(r[0], w.x0), ix1 = std::min(r[2], w.x1);
		if (r[0] < ix0) next.push_back({ r[0], iy0, ix0, iy1 });
		if (ix1 < r[2]) next.push_back({ ix1, iy0, r[2], iy1 });
	}
	if (next.size() > cap)
	{
		std::sort(next.begin(), next.end(), [](const std::array<float, 4>& a, const std::array<float, 4>& b) {
			return (a[2] - a[0]) * (a[3] - a[1]) > (b[2] - b[0]) * (b[3] - b[1]);
		});
		next.resize(cap);
	}
	pieces = std::move(next);
}

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

	const char* tag = subject->mActorClient.ActorDef;
	if (!tag || !tag[0])
		return nullptr;

	PortraitEngineState& es = s_portraitEngine[slot];
	if (es.actor && es.tag == tag)
		return es.actor;

	ReleasePreviewActor(slot, render, eng);

	CActorDefinitionInterface* def = nullptr;
	if (eng->pResourceManager->GetActorDefinition(tag, &def) != eStatusSuccess || def == nullptr)
		return nullptr;

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
			render->EnableObjectPreview(true, slot);
			if (eng && eng->pSceneGraph)
			{
				CCameraInterface* cam = nullptr;
				eng->pSceneGraph->CreateObjectPreviewCamera(&cam, static_cast<EObjectPreviewMode>(slot));
			}
			es.enabled = true;
			es.boundActor = nullptr;
		}
		if (es.boundActor != actor)
		{
			render->SetObjectPreviewActor(actor, slot);
			es.boundActor = actor;
		}

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

} // namespace

static double SteadySeconds()
{
	return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

static double FxClockSeconds()
{
	return SteadySeconds() - s_fxStartTime;
}

float FxClockNow()
{
	return static_cast<float>(FxClockSeconds());
}

namespace {

void OnSceneRender()
{
	s_sceneCalls.fetch_add(1, std::memory_order_relaxed);

	if (GetGameState() != GAMESTATE_INGAME)
		return;

	DrawParticleGpu();
	UiSweep();

	CRender* render = g_pDrawHandler.get();
	CDisplay* display = pDisplay.get();
	if (!render || !display || !display->pCamera)
		return;

	DrawPortraits(render);

	RenderGpuEffects();

	s_lastDraws.store(s_fxLastInstances.load(std::memory_order_relaxed),
		std::memory_order_relaxed);
}

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

	{
		std::scoped_lock lock(s_fxMutex);
		const float now = FxClockNow();
		std::erase_if(s_fxLua, [now](const eqgfx::gpu::FxInstance& ix) {
			return ix.life > 0.0f && !(ix.flags & eqgfx::gpu::FxFlag_NoEnvelope) && now > ix.birth + ix.life;
		});
		s_fxDraw = s_fxSubmit;
		const size_t room = eqgfx::gpu::kMaxInstances - std::min(s_fxDraw.size(), size_t(eqgfx::gpu::kMaxInstances));
		s_fxDraw.insert(s_fxDraw.end(), s_fxLua.begin(),
			s_fxLua.begin() + static_cast<ptrdiff_t>(std::min(room, s_fxLua.size())));
	}

	{
		std::scoped_lock lock(s_sceneMutex);

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
			UnpackArgb(seg.color, ix.rgba);

			if (seg.kind == Seg_Screen)
			{
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
			UnpackArgb(rect.color, ix.rgba);
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

	const double t = FxClockSeconds();

	s_lineFX.Render(s_fxDraw.data(), s_fxDraw.size(), viewProj, camPos,
		camRight, camUp, t,
		static_cast<float>(width), static_cast<float>(height));

	s_fxLastInstances.store(s_lineFX.LastInstanceCount(), std::memory_order_relaxed);
	s_fxLastSegments.store(s_lineFX.LastSegmentCount(), std::memory_order_relaxed);
}

uint32_t ToColor(int64_t color)
{
	return static_cast<uint32_t>(color & 0xFFFFFFFF);
}

uint32_t Channel(int64_t value)
{
	return static_cast<uint32_t>(std::clamp<int64_t>(value, 0, 255));
}

void PrintEqgfxHelp()
{
	WriteChatf("\ag[MQEQGFX]\ax \ag--- unit frames ---\ax");
	eqgfx::unitframes::PrintHelp();

	WriteChatf("\ag[MQEQGFX]\ax \ag--- targeting and casting ---\ax");
	WriteChatf("\ag[MQEQGFX]\ax \ay/attarget <unit> <command>\ax - run any target-consuming command "
		"at 2Tar | Group1-6 | Raid1-72 | XTar1-20 | MouseHover | Me | Target | <id> | <name>");
	WriteChatf("\ag[MQEQGFX]\ax   e.g. \ay/attarget MouseHover /cast 3\ax - your own target never changes");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx focus [clear|target|marker|window|<name>]\ax - the secondary target");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx frames [on|off|raid|worldhover|fallback]\ax");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx xtarget [on|off]\ax, \ay/eqgfx units\ax - status");

	WriteChatf("\ag[MQEQGFX]\ax \ag--- spell animations ---\ax");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx spells [on|off|self|group|all|radius <n>|persist off|self|group|list|why <spell>|test <spell>]\ax"
		" - %d native looks picked from what each spell does", SpellFxLookCount());

	WriteChatf("\ag[MQEQGFX]\ax \ag--- performance ---\ax");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx nametags [on|off]\ax - draw every nametag in one batch"
		" (status shows the client's draw count per frame)");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx uitext [on|off]\ax - draw runs of adjacent UI strings together");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx animlod [on|off|distance <n>|interval <n>]\ax"
		" - animate far actors less often");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx particles [on|off|budget <n>]\ax - EQ's spell and zone particles drawn on the gpu");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx debug\ax - test panel: feature toggles, stats, spell looks, forms and glyphs");

	WriteChatf("\ag[MQEQGFX]\ax \ag--- native ui ---\ax");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx ui [list|all on|off|native|<window> [on|off]]\ax"
		" - draw a stock window ourselves; it keeps working as the game's own");

	WriteChatf("\ag[MQEQGFX]\ax \ag--- world effects ---\ax");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx fx [on|off|test|list|slots <mask>|radius <n>|max <n>|pcs|npcs]\ax"
		" - the per-class passive animations, \aroff\ax unless you turn them on");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx highlight [clear|off|<0xAARRGGBB>] [tint|ambient|both]\ax"
		" - recolour your target's model");

	WriteChatf("\ag[MQEQGFX]\ax \ag--- models and capture ---\ax");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx showmodel [<armor>|off]\ax, \ay/eqgfx portrait self|target [on|off]\ax \ao(in development)\ax");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx dumpchar\ax, \ay/eqgfx dumpmodel\ax - what the client says you are wearing");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx capture [off|save|clear|status]\ax - record the appearance calls");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx dmgdebug [on|off]\ax - log raw damage events");
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx\ax alone prints build, render and gpu status");

	WriteChatf("\ag[MQEQGFX]\ax \ag--- lua ---\ax");
	WriteChatf("\ag[MQEQGFX]\ax \ay/lua run eqgfx/nameplates\ax (\ay/npmenu\ax), "
		"\ay/lua run eqgfx/indicators\ax (\ay/aemenu\ax), \ay/lua run eqgfx/dmgnums\ax (\ay/dnmenu\ax), "
		"\ay/lua run eqgfx/highlight\ax");
}

void EqgfxCommand(PlayerClient*, const char* szLine)
{
	char szCmd[MAX_STRING] = { 0 };
	GetArg(szCmd, szLine, 1);

	if (ci_equals(szCmd, "help") || ci_equals(szCmd, "?"))
	{
		PrintEqgfxHelp();
		return;
	}

	if (ci_equals(szCmd, "debug"))
	{
		char szState[MAX_STRING] = { 0 };
		GetArg(szState, szLine, 2);
		ShowDebugWindow(ci_equals(szState, "off") ? false : ci_equals(szState, "on") ? true : !DebugWindowShown());
		return;
	}

	if (ci_equals(szCmd, "uitext"))
	{
		UiTextBatchCommand(szLine);
		return;
	}

	if (ci_equals(szCmd, "ui"))
	{
		eqgfx::ui::NativeUiCommand(szLine);
		return;
	}

	if (ci_equals(szCmd, "particles"))
	{
		ParticleGpuCommand(szLine);
		return;
	}

	if (ci_equals(szCmd, "animlod"))
	{
		AnimationLodCommand(szLine);
		return;
	}

	if (ci_equals(szCmd, "nametags") || ci_equals(szCmd, "perf"))
	{
		NametagBatchCommand(szLine);
		return;
	}

	if (ci_equals(szCmd, "spells") || ci_equals(szCmd, "spellfx"))
	{
		SpellFxCommand(szLine);
		return;
	}

	if (UnitsCommand(szCmd, szLine))
		return;

	if (ci_equals(szCmd, "capture"))
	{
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

		char szMode[MAX_STRING] = { 0 };
		GetArg(szMode, szLine, 3);

		uint32_t color = 0xFFFF3030;
		if (ci_equals(szArg, "tint") || ci_equals(szArg, "ambient") || ci_equals(szArg, "both"))
			strcpy_s(szMode, szArg);
		else if (szArg[0])
		{
			char* end = nullptr;
			const long long value = strtoll(szArg, &end, 0);
			if (*end || value < INT32_MIN || value > UINT32_MAX)
			{
				WriteChatf("\ag[MQEQGFX]\ax usage: /eqgfx highlight [clear|off|<0xAARRGGBB>] [tint|ambient|both]");
				return;
			}
			color = static_cast<uint32_t>(value);
		}

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
		else if (ci_equals(a1, "radius")) { const float r = GetFloatFromString(a2, s_fxDriver.radius); if (std::isfinite(r)) s_fxDriver.radius = std::clamp(r, 20.0f, 600.0f); WriteChatf("\ag[MQEQGFX]\ax fx radius %.0f", s_fxDriver.radius); }
		else if (ci_equals(a1, "max"))    { s_fxDriver.maxSpawns = std::clamp(GetIntFromString(a2, s_fxDriver.maxSpawns), 1, 200); WriteChatf("\ag[MQEQGFX]\ax fx max %d", s_fxDriver.maxSpawns); }
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
		WriteChatf("\ag[MQEQGFX]\ax %s portrait %s \ao(in development)\ax", szWho, on ? "\agon\ax" : "\ayoff\ax");
		return;
	}

	if (szCmd[0] && !ci_equals(szCmd, "status"))
	{
		WriteChatf("\ag[MQEQGFX]\ax unknown subcommand '%s' - \ay/eqgfx help\ax lists every command", szCmd);
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
		WriteChatf("\ag[MQEQGFX]\ax portrait slot %d: enabled=%d actor=%s tag='%s'",
			slot, es.enabled ? 1 : 0, es.actor ? "yes" : "no", es.tag.c_str());
	}
#ifdef CEverQuest__StartCasting_x
	const char* castMode = !s_castTapInstalled ? (s_castTapFailed ? "failed" : "inactive")
		: (s_castTrackerOwned ? "plugin-owned tracker" : "MacroQuest");
	WriteChatf("\ag[MQEQGFX]\ax cast tracking: %s, %d cast events queued",
		castMode, static_cast<int>(castQueued));
#else
	WriteChatf("\ag[MQEQGFX]\ax cast tracking unavailable on this client");
#endif
	WriteChatf("\ag[MQEQGFX]\ax \ay/eqgfx help\ax lists every command");
}

const void* s_sceneOwner = nullptr;
const void* s_fxOwner = nullptr;
std::array<const void*, kPortraitSlots> s_portraitOwners = {};

void ClearScene()
{
	s_back.clear();
	std::scoped_lock lock(s_sceneMutex);
	s_front.clear();
}

void ClearLuaFx()
{
	std::scoped_lock lock(s_fxMutex);
	s_fxLua.clear();
}

void ReleaseLuaOwner(const void* owner)
{
	if (!s_active || !owner)
		return;
	if (s_sceneOwner == owner)
	{
		s_sceneOwner = nullptr;
		ClearScene();
	}
	if (s_fxOwner == owner)
	{
		s_fxOwner = nullptr;
		ClearLuaFx();
	}
	{
		std::scoped_lock lock(s_uiMutex);
		if (s_uiNameLists.erase(owner))
			RebuildUiNames();
	}
	ClearHighlightsOwnedBy(owner);
	for (int slot = 0; slot < kPortraitSlots; ++slot)
	{
		if (s_portraitOwners[slot] != owner)
			continue;
		s_portraitOwners[slot] = nullptr;
		std::scoped_lock lock(s_portraitMutex);
		s_portraits[slot] = PortraitConfig{};
	}
}

struct LuaStateGuard
{
	explicit LuaStateGuard(const void* o) : owner(o) {}
	LuaStateGuard(LuaStateGuard&& other) noexcept : owner(std::exchange(other.owner, nullptr)) {}
	LuaStateGuard(const LuaStateGuard&) = delete;
	LuaStateGuard& operator=(const LuaStateGuard&) = delete;
	LuaStateGuard& operator=(LuaStateGuard&&) = delete;
	~LuaStateGuard() { ReleaseLuaOwner(owner); }

	const void* owner = nullptr;
};

sol::table BuildModuleTable(sol::state_view sv)
{
	sol::table m = sv.create_table();
	const void* owner = sol::main_thread(sv.lua_state(), sv.lua_state());
	m["__owner"] = LuaStateGuard(owner);


	m.set_function("init", []() {
		if (!s_active)
			return std::make_tuple(false, sol::optional<std::string>("MQEQGFX plugin is not active"));
		if (GetGameState() != GAMESTATE_INGAME)
			return std::make_tuple(false, sol::optional<std::string>("not in game yet"));
		if (!g_pDrawHandler.get())
			return std::make_tuple(false, sol::optional<std::string>("render interface is null (not in game yet?)"));
		return std::make_tuple(true, sol::optional<std::string>());
	});

	m.set_function("clear", [owner]() { s_sceneOwner = owner; PresentScene(true); });

	m.set_function("present", [owner]() { s_sceneOwner = owner; PresentScene(false); });

	auto blankScene = [owner]() {
		s_sceneOwner = owner;
		ClearScene();
	};
	m.set_function("blank", blankScene);
	m.set_function("shutdown", blankScene);

	m.set_function("build", []() { return __DATE__ " " __TIME__; });

	m.set_function("stats", []() -> std::tuple<uint32_t, uint32_t> {
		return { s_sceneCalls.load(std::memory_order_relaxed), s_lastDraws.load(std::memory_order_relaxed) };
	});

	m.set_function("add_circle", [](float x, float y, float z, float radius, int64_t color,
		sol::optional<int> segments)
	{
		if (SceneFull())
			return;
		int segs = std::clamp(segments.value_or(48), 3, kMaxArcSegments);
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
		if (SceneFull())
			return;
		int segs = std::clamp(segments.value_or(24), 1, kMaxArcSegments);
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
		if (SceneFull())
			return;
		s_back.segments.push_back({ x1, y1, z1, x2, y2, z2, ToColor(color), Seg_World });
	});

	m.set_function("add_screen_line", [](float x1, float y1, float x2, float y2, int64_t color) {
		if (SceneFull())
			return;
		s_back.segments.push_back({ x1, y1, 0.0f, x2, y2, 0.0f, ToColor(color), Seg_Screen });
	});

	m.set_function("add_screen_rect", [](float x0, float y0, float x1, float y1, int64_t color) {
		if (SceneFull())
			return;
		s_back.rects.push_back({ x0, y0, x1, y1, ToColor(color) });
	});

	m.set_function("set_thickness", [](int t) { s_thickness = std::max(0, t); });

	m.set_function("highlight", [owner](int spawnId, int64_t color, sol::optional<int> mode) {
		SetSpawnHighlight(spawnId, ToColor(color),
			static_cast<uint32_t>(mode.value_or(static_cast<int>(Highlight_Tint))), owner);
	});

	m.set_function("unhighlight", [](int spawnId) { ClearSpawnHighlight(spawnId); });
	m.set_function("clear_highlights", []() { ClearAllHighlights(); });

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

	m.set_function("fx_clear", [owner]() {
		s_fxOwner = owner;
		ClearLuaFx();
	});

	m.set_function("fx_add", [owner](int form, float x, float y, float z, sol::optional<sol::table> opts) {
		if (form < 0 || form >= static_cast<int>(eqgfx::gpu::FxForm_Count))
			return false;
		s_fxOwner = owner;
		std::scoped_lock lock(s_fxMutex);
		if (s_fxLua.size() >= eqgfx::gpu::kMaxInstances)
			return false;

		eqgfx::gpu::FxInstance ix = {};
		ix.pos[0] = x; ix.pos[1] = y; ix.pos[2] = z;
		ix.form = static_cast<uint32_t>(form);
		ix.radius = 3.0f; ix.height = 4.0f; ix.speed = 1.0f;
		ix.segments = 40; ix.width = 0.05f;
		ix.rgba[0] = ix.rgba[1] = ix.rgba[2] = ix.rgba[3] = 1.0f;
		bool segmentsGiven = false;

		if (opts)
		{
			sol::table o = *opts;
			ix.radius   = o.get_or("radius", ix.radius);
			ix.height   = o.get_or("height", ix.height);
			ix.speed    = o.get_or("speed", ix.speed);
			ix.tilt     = o.get_or("tilt", 0.0f);
			ix.phase    = o.get_or("phase", 0.0f);
			ix.width    = o.get_or("width", ix.width);
			if (sol::optional<int> segments = o["segments"])
			{
				ix.segments = static_cast<uint32_t>(std::clamp(*segments, 1, static_cast<int>(eqgfx::gpu::kMaxSegsPerInstance)));
				segmentsGiven = true;
			}
			ix.p0 = o.get_or("p0", 0.0f); ix.p1 = o.get_or("p1", 0.0f);
			ix.p2 = o.get_or("p2", 0.0f); ix.p3 = o.get_or("p3", 0.0f);

			if (sol::optional<int64_t> c = o["color"])
			{
				UnpackArgb(ToColor(*c), ix.rgba);
			}
			if (o.get_or("comet", false))       ix.flags |= eqgfx::gpu::FxFlag_Comet;
			if (o.get_or("additive", false))    ix.flags |= eqgfx::gpu::FxFlag_Additive;
			if (o.get_or("nodepth", false))     ix.flags |= eqgfx::gpu::FxFlag_NoDepth;
			if (o.get_or("reverse", false))     ix.flags |= eqgfx::gpu::FxFlag_Reverse;
			if (o.get_or("screenwidth", false)) ix.flags |= eqgfx::gpu::FxFlag_ScreenWidth;
			if (o.get_or("noenvelope", false))  ix.flags |= eqgfx::gpu::FxFlag_NoEnvelope;
			ix.p4  = o.get_or("p4", 0.0f);
			ix.p5  = o.get_or("p5", 0.0f);
			ix.yaw = o.get_or("yaw", 0.0f);
			if (sol::optional<int64_t> c2 = o["color2"])
			{
				UnpackArgb(ToColor(*c2), ix.rgba2);
			}
			if (sol::optional<float> life = o["life"])
			{
				ix.life  = std::max(*life, 0.05f);
				ix.birth = FxClockNow() + o.get_or("delay", 0.0f);
			}

			if (sol::optional<sol::table> p2t = o["to"])
			{
				ix.pos2[0] = (*p2t).get_or(1, x);
				ix.pos2[1] = (*p2t).get_or(2, y);
				ix.pos2[2] = (*p2t).get_or(3, z);
			}
		}
		if (form == eqgfx::gpu::FxForm_Glyph && !segmentsGiven)
		{
			const uint32_t copies = static_cast<uint32_t>(std::clamp(std::ceil(ix.p2), 1.0f, 128.0f));
			ix.segments = std::min(eqgfx::gpu::kMaxSegsPerInstance,
				eqgfx::gpu::GlyphSegments(static_cast<uint32_t>(ix.p0)) * copies);
		}
		s_fxLua.push_back(ix);
		return true;
	});

	m.set_function("fx_preset", [owner](const char* cls, int index, float x, float y, float z,
		sol::optional<float> phase, sol::optional<float> alpha)
	{
		s_fxOwner = owner;
		const eqgfx::gpu::ClassFx* c = eqgfx::gpu::FindClassFx(cls);
		if (!c || index < 0 || index >= c->count)
			return false;
		std::scoped_lock lock(s_fxMutex);
		if (s_fxLua.size() >= eqgfx::gpu::kMaxInstances)
			return false;
		const float pos[3] = { x, y, z };
		eqgfx::gpu::FxInstance ix;
		eqgfx::gpu::MakeInstance(c->defs[index], pos, phase.value_or(0.0f),
			alpha.value_or(1.0f), ix);
		s_fxLua.push_back(ix);
		return true;
	});

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
	m["FX_FLAME"]     = static_cast<int>(eqgfx::gpu::FxForm_Flame);
	m["FX_VINES"]     = static_cast<int>(eqgfx::gpu::FxForm_Vines);
	m["FX_THORNS"]    = static_cast<int>(eqgfx::gpu::FxForm_Thorns);
	m["FX_GLYPH"]     = static_cast<int>(eqgfx::gpu::FxForm_Glyph);
	m["FX_WINGS"]     = static_cast<int>(eqgfx::gpu::FxForm_Wings);
	m["FX_LIGHTNING"] = static_cast<int>(eqgfx::gpu::FxForm_Lightning);
	m["FX_BOLT"]      = static_cast<int>(eqgfx::gpu::FxForm_Bolt);
	m["FX_RAIN"]      = static_cast<int>(eqgfx::gpu::FxForm_Rain);
	m["FX_SPARKLE"]   = static_cast<int>(eqgfx::gpu::FxForm_Sparkle);
	m["FX_RUNE"]      = static_cast<int>(eqgfx::gpu::FxForm_Rune);
	m["FX_VORTEX"]    = static_cast<int>(eqgfx::gpu::FxForm_Vortex);
	m["FX_DOME"]      = static_cast<int>(eqgfx::gpu::FxForm_Dome);
	m["FX_CRYSTAL"]   = static_cast<int>(eqgfx::gpu::FxForm_Crystal);
	m["FX_DRAIN"]     = static_cast<int>(eqgfx::gpu::FxForm_Drain);
	m["FX_BUBBLES"]   = static_cast<int>(eqgfx::gpu::FxForm_Bubbles);
	m["FX_WIND"]      = static_cast<int>(eqgfx::gpu::FxForm_Wind);
	m["FX_BURST"]     = static_cast<int>(eqgfx::gpu::FxForm_Burst);
	m["FX_CRACKS"]    = static_cast<int>(eqgfx::gpu::FxForm_Cracks);

	{
		sol::table g = sv.create_table();
		for (uint32_t id = 0; id < eqgfx::gpu::Glyph_Count; ++id)
			g[eqgfx::gpu::GlyphName(id)] = static_cast<int>(id);
		m["GLYPH"] = g;
		sol::table a = sv.create_table();
		a["rise"]   = static_cast<int>(eqgfx::gpu::GlyphAnim_Rise);
		a["bob"]    = static_cast<int>(eqgfx::gpu::GlyphAnim_Bob);
		a["spin"]   = static_cast<int>(eqgfx::gpu::GlyphAnim_Spin);
		a["pop"]    = static_cast<int>(eqgfx::gpu::GlyphAnim_Pop);
		a["wiggle"] = static_cast<int>(eqgfx::gpu::GlyphAnim_Wiggle);
		a["ascend"] = static_cast<int>(eqgfx::gpu::GlyphAnim_Ascend);
		a["descend"] = static_cast<int>(eqgfx::gpu::GlyphAnim_Descend);
		a["roll"]   = static_cast<int>(eqgfx::gpu::GlyphAnim_Roll);
		m["GLYPH_ANIM"] = a;
	}

	m.set_function("fx_now", []() { return FxClockNow(); });

	m.set_function("spellfx_enabled", [](sol::optional<bool> on) {
		if (on) SetSpellFxEnabled(*on);
		return SpellFxEnabled();
	});

	m.set_function("spellfx_play", [](int spellId, sol::optional<int> caster,
		sol::optional<int> target, sol::optional<int> castMs)
	{
		const int me = pLocalPlayer ? pLocalPlayer->SpawnID : 0;
		const int tgt = target.value_or(pTarget ? pTarget->SpawnID : me);
		SpellFxPlay(spellId, caster.value_or(me), tgt, static_cast<uint32_t>(std::max(0, castMs.value_or(0))));
	});

	m.set_function("spellfx_look", [](int spellId) { return SpellFxDescribe(spellId); });

	m.set_function("focus", [](sol::optional<int> id) {
		if (id) SetSecondaryTarget(*id);
		return GetSecondaryTarget();
	});

	m.set_function("hover", [](sol::optional<int> id) {
		if (id) SetHoverUnit(*id);
		return GetHoverUnit();
	});

	m.set_function("cast_unit", []() { return ResolveCastUnit(); });

	m.set_function("cast_at", [](int spawnId, int gem) {
		return CastAtSpawn(spawnId, gem);
	});

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

	m.set_function("spawn_search", [](sol::this_state L, const std::string& spec, sol::optional<int> max) {
		sol::state_view sv(L);
		sol::table out = sv.create_table();
		PlayerClient* me = pLocalPlayer.get();
		if (!me || spec.size() >= MAX_STRING)
			return out;

		MQSpawnSearch search;
		ClearSearchSpawn(&search);
		ParseSearchSpawn(spec.c_str(), &search);

		std::vector<std::pair<float, int>> found;
		for (PlayerClient* sp = pSpawnList; sp; sp = sp->pNext)
		{
			if (SpawnMatchesSearch(&search, me, sp))
				found.emplace_back(GetDistanceSquared3D(me, sp), static_cast<int>(sp->SpawnID));
		}
		std::sort(found.begin(), found.end());

		const size_t limit = std::min(found.size(), static_cast<size_t>(std::max(max.value_or(INT_MAX), 0)));
		for (size_t i = 0; i < limit; ++i)
			out[i + 1] = found[i].second;
		return out;
	});

	m.set_function("spell_geom", [](sol::this_state L, int spellID) -> sol::object {
		EQ_Spell* spell = GetSpellByID(spellID);
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

	m.set_function("set_ui_names", [owner](sol::table names) {
		std::vector<std::string> list;
		size_t count = names.size();
		list.reserve(count);
		for (size_t i = 1; i <= count; ++i)
		{
			sol::optional<std::string> name = names[i];
			if (name && !name->empty() && name->size() <= kMaxWindowNameLen)
				list.push_back(std::move(*name));
		}

		std::scoped_lock lock(s_uiMutex);
		s_uiNameLists[owner].names = std::move(list);
		RebuildUiNames();
	});

	m.set_function("ui_subtract", [owner](sol::this_state L, float x0, float y0, float x1, float y1,
		sol::optional<int> cap) -> sol::object
	{
		s_uiRectsWanted.store(true, std::memory_order_relaxed);
		const size_t limit = static_cast<size_t>(std::clamp(cap.value_or(24), 1, 64));
		std::vector<std::array<float, 4>> pieces{ { x0, y0, x1, y1 } };
		bool covered = false;
		{
			std::scoped_lock lock(s_uiMutex);
			auto it = s_uiNameLists.find(owner);
			if (it != s_uiNameLists.end())
			{
				const std::vector<int>& fromUnion = it->second.fromUnion;
				for (const UiRect& w : s_uiRects)
				{
					if (w.nameIdx < 0 || w.nameIdx >= static_cast<int>(fromUnion.size()) || fromUnion[w.nameIdx] < 0)
						continue;
					if (w.x0 >= x1 || w.x1 <= x0 || w.y0 >= y1 || w.y1 <= y0)
						continue;
					covered = true;
					SubtractRect(pieces, w, limit);
				}
			}
		}
		if (!covered)
			return sol::make_object(L, sol::nil);

		sol::state_view sv(L);
		sol::table t = sv.create_table(static_cast<int>(pieces.size()), 0);
		for (size_t i = 0; i < pieces.size(); ++i)
			t[i + 1] = sv.create_table_with(1, pieces[i][0], 2, pieces[i][1], 3, pieces[i][2], 4, pieces[i][3]);
		return t;
	});

	m.set_function("get_ui_rects", [owner](sol::this_state L) {
		s_uiRectsWanted.store(true, std::memory_order_relaxed);
		std::vector<std::pair<UiRect, std::string>> snapshot;
		{
			std::scoped_lock lock(s_uiMutex);
			auto it = s_uiNameLists.find(owner);
			if (it != s_uiNameLists.end())
			{
				const std::vector<int>& fromUnion = it->second.fromUnion;
				for (const UiRect& rect : s_uiRects)
				{
					if (rect.nameIdx >= 0 && rect.nameIdx < static_cast<int>(fromUnion.size()) && fromUnion[rect.nameIdx] >= 0)
						snapshot.push_back({ { rect.x0, rect.y0, rect.x1, rect.y1, fromUnion[rect.nameIdx] }, s_uiNames[rect.nameIdx] });
				}
			}
		}

		sol::state_view sv(L);
		sol::table t = sv.create_table(static_cast<int>(snapshot.size()), 0);
		int i = 1;
		for (const auto& [rect, name] : snapshot)
		{
			sol::table rt = sv.create_table(4, 2);
			rt[1] = rect.x0;
			rt[2] = rect.y0;
			rt[3] = rect.x1;
			rt[4] = rect.y1;
			rt["idx"] = rect.nameIdx + 1;
			rt["name"] = name;
			t[i++] = rt;
		}
		return t;
	});

	m.set_function("ui_stats", []() -> std::tuple<int, int> {
		std::scoped_lock lock(s_uiMutex);
		return { s_uiStatNames, s_uiStatFound };
	});

	m.set_function("argb", [](int64_t a, int64_t r, int64_t g, int64_t b) -> int32_t {
		return static_cast<int32_t>((Channel(a) << 24) | (Channel(r) << 16) | (Channel(g) << 8) | Channel(b));
	});
	m.set_function("abgr", [](int64_t a, int64_t b, int64_t g, int64_t r) -> int32_t {
		return static_cast<int32_t>((Channel(a) << 24) | (Channel(b) << 16) | (Channel(g) << 8) | Channel(r));
	});

	m.set_function("cast_feed_live", []() -> bool {
#ifdef CEverQuest__StartCasting_x
		return s_castTapInstalled;
#else
		return false;
#endif
	});

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

	m.set_function("portrait_slots", []() { return static_cast<int>(kPortraitSlots); });

	m.set_function("set_portrait", [owner](int slot, sol::table opts) {
		if (slot < 0 || slot >= kPortraitSlots) return;
		s_portraitOwners[slot] = owner;
		PortraitConfig cfg;
		cfg.enabled = opts["enabled"].get_or(true);
		std::string who = opts["subject"].get_or(std::string("self"));
		cfg.subject = (who == "target") ? PS_TARGET : (who == "none") ? PS_NONE : PS_SELF;
		cfg.window = opts["window"].get_or(std::string());
		if (cfg.window.size() > kMaxWindowNameLen) return;
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
		s_portraitOwners[slot] = nullptr;
		std::scoped_lock lock(s_portraitMutex);
		s_portraits[slot] = PortraitConfig{};
	});

	return m;
}

} // namespace

FxStats GetFxStats()
{
	return { s_fxLastInstances.load(std::memory_order_relaxed), s_fxLastSegments.load(std::memory_order_relaxed),
		s_lineFX.Ready(), s_lineFX.LastError() };
}

GfxOptions GetGfxOptions()
{
	GfxOptions o;
	o.classFx          = s_fxDriver.enabled;
	o.classFxPCs       = s_fxDriver.includePCs;
	o.classFxNPCs      = s_fxDriver.npcs;
	o.classFxRadius    = s_fxDriver.radius;
	o.classFxMaxSpawns = s_fxDriver.maxSpawns;
	o.classFxSlotMask  = s_fxDriver.slotMask;
	o.classFxStyle     = s_fxDriver.style;
	o.testRing         = s_fxTest.load(std::memory_order_relaxed);
	o.gpuEffects       = s_fxEnabled.load(std::memory_order_relaxed);
	o.damageDebug      = s_dmgDebug.load(std::memory_order_relaxed);
	return o;
}

void SetGfxOptions(const GfxOptions& opts)
{
	s_fxDriver.enabled    = opts.classFx;
	s_fxDriver.includePCs = opts.classFxPCs;
	s_fxDriver.npcs       = opts.classFxNPCs;
	s_fxDriver.radius     = opts.classFxRadius;
	s_fxDriver.maxSpawns  = opts.classFxMaxSpawns;
	s_fxDriver.slotMask   = opts.classFxSlotMask;
	s_fxDriver.style      = opts.classFxStyle;
	s_fxTest.store(opts.testRing, std::memory_order_relaxed);
	s_fxEnabled.store(opts.gpuEffects, std::memory_order_relaxed);
	s_dmgDebug.store(opts.damageDebug, std::memory_order_relaxed);
}

PLUGIN_API bool CreateLuaModule(sol::this_state L, sol::object& outModule)
{
	outModule = BuildModuleTable(sol::state_view(L));
	return true;
}

float FxPhaseFor(int spawnId)
{
	uint32_t h = static_cast<uint32_t>(spawnId) * 2654435761u;
	h ^= h >> 15;
	h *= 2246822519u;
	h ^= h >> 13;
	return (h & 0xFFFFFF) / static_cast<float>(0x1000000);
}

struct SeenTimes { float first; float last; };
static std::unordered_map<int, SeenTimes> s_classFxSeen;

void PulseClassFx()
{
	std::vector<eqgfx::gpu::FxInstance> out;

	const FxDriverCfg cfg = s_fxDriver;
	PlayerClient* me = pLocalPlayer.get();

	if (s_fxTest.load(std::memory_order_relaxed) && me)
	{
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
		const float now = FxClockNow();
		const float motion = cfg.style.motion;
		for (PlayerClient* sp = pSpawnList; sp && taken < cfg.maxSpawns; sp = sp->pNext)
		{
			if (sp == me)
				continue;

			const eSpawnType type = GetSpawnType(sp);
			const bool isPC = (type == PC || type == MERCENARY);
			if (isPC ? !cfg.includePCs : (type != NPC || !cfg.npcs))
				continue;

			if (GetDistanceSquared3D(me, sp) > r2)
				continue;

			const eqgfx::gpu::ClassFx* fx = eqgfx::gpu::FindClassFxById(sp->GetClass());
			if (!fx)
				continue;

			const float phase = FxPhaseFor(sp->SpawnID);

			SeenTimes& seen = s_classFxSeen[sp->SpawnID];
			if (seen.last == 0.0f || now - seen.last > 2.0f)
				seen.first = now;
			seen.last = now;
			const float alpha = AlwaysOnFade(cfg.style, now - seen.first, phase);

			float pos[3] = { sp->X, sp->Y, sp->Z };
			if (motion > 0.0f)
			{
				const float h = sp->AvatarHeight > 0.5f ? sp->AvatarHeight : 6.0f;
				const float a = now * 0.6f * motion + phase * 6.2831853f;
				pos[0] += std::cos(a) * 0.6f * motion;
				pos[1] += std::sin(a) * 0.6f * motion;
				pos[2] += (0.5f - 0.5f * std::cos(now * 0.45f * motion + phase * 6.2831853f)) * h * 0.55f;
			}

			for (int slot = 0; slot < fx->count && slot < 32; ++slot)
			{
				if ((cfg.slotMask & (1 << slot)) == 0)
					continue;
				if (out.size() >= eqgfx::gpu::kMaxInstances)
					break;
				eqgfx::gpu::FxInstance ix;
				eqgfx::gpu::MakeInstance(fx->defs[slot], pos, phase, alpha, ix);
				if (motion > 0.0f)
				{
					ix.flags |= eqgfx::gpu::FxFlag_Drift;
					ix.p5 = 0.3f * motion * (slot % 2 ? -1.0f : 1.0f);
				}
				out.push_back(ix);
			}
			++taken;
		}
		s_fxDriverSpawns.store(static_cast<uint32_t>(taken), std::memory_order_relaxed);

		if ((static_cast<int>(now) % 16) == 0)
			for (auto it = s_classFxSeen.begin(); it != s_classFxSeen.end(); )
				it = (now - it->second.last > 30.0f) ? s_classFxSeen.erase(it) : std::next(it);
	}
	else
	{
		s_fxDriverSpawns.store(0, std::memory_order_relaxed);
	}

	PulseSpellFx(out);
	BuildUnitMarkers(out);
	BuildAttackFx(out);
	BuildTargetRing(out);

	std::scoped_lock lock(s_fxMutex);
	s_fxSubmit.swap(out);
}

PLUGIN_API void OnPulse()
{
	if (!s_active) return;

	eqgfx::settings::Global().Pulse();

	eqgfx::ui::PulseNativeUi();

	if (GetGameState() != GAMESTATE_INGAME) return;

	if (!s_captureInstalled)
		InstallSwapCapture(true);

	if (!s_dmgTapInstalled)
		InstallDamageTap();

	PulseCastTap();

	InstallNametagBatch();
	InstallAnimationLod();
	InstallParticleGpu();
	SampleActorAnimationStats();
	SampleParticleGpuStats();
	PulseAnimationLod();
	PulseGfxSettings();
	InstallSelfCastHook();
	SampleEqDrawCounter();

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
	++tick;

	const bool doHeavy = gbInForeground || (tick % 8) == 0;

	if (doHeavy)
	{
		PulseHighlights();
		PulseClassFx();
	}

	PulseUnits();

	if ((tick % 64) == 0)
		RefreshHighlights();
	if ((tick % 256) == 0 && s_captureDirty.load(std::memory_order_relaxed))
		SaveCapture();

	if (s_reloadModel.exchange(false) && s_showModel.load(std::memory_order_relaxed))
	{
		std::string src, note;
		BuildModelView(s_lastArmor, src, note);
	}
}

PLUGIN_API void OnRemoveSpawn(PlayerClient* pSpawn)
{
	if (pSpawn)
	{
		OnHighlightSpawnRemoved(pSpawn->SpawnID);
		OnUnitSpawnRemoved(pSpawn->SpawnID);
		OnSpellFxSpawnRemoved(pSpawn->SpawnID);
	}
}

bool ProjectToScreen(float x, float y, float z, float& sx, float& sy)
{
	return WorldToScreen(x, y, z, sx, sy);
}

PLUGIN_API void OnCleanUI()  { eqgfx::ui::NativeUiCleanUI(); }

PLUGIN_API bool OnIncomingChat(const char* Line, DWORD Color)
{
	eqgfx::ui::NativeUiIncomingChat(Line, Color);
	return false;
}

PLUGIN_API void OnWriteChatColor(const char* Line, int Color, int Filter)
{
	(void)Filter;
	eqgfx::ui::NativeUiWriteChat(Line, Color);
}
PLUGIN_API void OnReloadUI() { eqgfx::ui::NativeUiReloadUI(); }

PLUGIN_API void OnBeginZone()
{
	ForgetHighlights();
	eqgfx::unitframes::ForgetUnits();
	ForgetSpellFx();
	ForgetNametagBatch();
	TrimParticleGpu();
	s_classFxSeen.clear();
}

PLUGIN_API void InitializePlugin()
{
	s_active = true;

	MQRenderCallbacks callbacks;
	callbacks.GraphicsSceneRender = OnSceneRender;
	callbacks.CreateDeviceObjects = []() {};
	callbacks.InvalidateDeviceObjects = []() {
		s_lineFX.ReleaseDeviceObjects();
		ReleaseNametagBatchDevice();
		ReleaseParticleGpuDevice();
		if (!s_showModel.load(std::memory_order_relaxed))
			s_modelView.ReleaseAll();
	};
	s_renderCallbackId = AddRenderCallbacks(callbacks);

	s_fxStartTime = SteadySeconds();

	const std::filesystem::path configDir(gPathConfig);

	s_lineFX.SetShaderCacheFile((configDir / "MQEQGFX_shaders.bin").string());

	eqgfx::settings::Global().Open(configDir / "MQEQGFX.json", INIFileName);

	InstallCastTap();
	LoadGfxSettings();
	LoadAttackSettings();
	LoadSpellFxSettings();
	LoadTargetRingSettings();
	InitUnits();
	eqgfx::ui::InitNativeUi();

	AddCommand("/eqgfx", EqgfxCommand);
	AddCommand("/attarget", AtTargetCommand);
}

PLUGIN_API void ShutdownPlugin()
{
	s_active = false;

	RemoveCommand("/eqgfx");
	RemoveCommand("/attarget");

	RemoveCastTap();
	RemoveNametagBatch();
	RemoveAnimationLod();
	RemoveParticleGpu();
	RemoveSelfCastHook();
	ShutdownUnits();
	eqgfx::ui::ShutdownNativeUi();

	if (s_renderCallbackId != -1)
	{
		RemoveRenderCallbacks(s_renderCallbackId);
		s_renderCallbackId = -1;
	}

	ClearAllHighlights();

	if (s_captureDirty.load(std::memory_order_relaxed))
		SaveCapture();
	RemoveSwapCapture();
	RemoveDamageTap();

	ClearScene();
	{
		std::scoped_lock lock(s_uiMutex);
		s_uiNameLists.clear();
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

	{
		CRenderInterface* render = g_pDrawHandler.get();
		SGraphicsEngine*  eng = pGraphicsEngine.get();
		for (int slot = 0; slot < kPortraitSlots; ++slot)
		{
			ReleasePreviewActor(slot, render, eng);
			if (render && s_portraitEngine[slot].enabled)
				render->EnableObjectPreview(false, slot);
			s_portraitEngine[slot] = PortraitEngineState{};
		}
	}
	{
		std::scoped_lock lock(s_portraitMutex);
		for (PortraitConfig& cfg : s_portraits)
			cfg = PortraitConfig{};
	}

	{
		std::scoped_lock lock(s_fxMutex);
		s_fxSubmit.clear();
		s_fxLua.clear();
	}
	s_fxDraw.clear();
	s_lineFX.ReleaseAll();

	s_showModel.store(false);
	s_modelView.ReleaseAll();

	FlushSpellFxSettings();
	FlushGfxSettings();
	FlushAttackSettings();
	FlushTargetRingSettings();
	eqgfx::settings::Global().Close();
	eqgfx::settings::FlushWrites();
}
