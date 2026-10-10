#include "MQEQGFX.h"

#include "gpu/FxCatalog.h"
#include "unitframes/UnitWindow.h"

#include "eqlib/graphics/CameraInterface.h"
#include "eqlib/Offsets.h"

#include <imgui.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <unordered_map>
#include <unordered_set>

namespace {
std::mutex                            s_unitMutex;
std::unordered_map<int, UnitCast>     s_casts;

std::atomic<int>  s_secondary{ 0 };
std::atomic<int>  s_hover{ 0 };

std::atomic<bool> s_worldHover{ true };
std::atomic<bool> s_focusMarker{ true };
std::atomic<bool> s_hoverFallback{ true };

constexpr uint32_t kCastMaxMs = 60000;

constexpr float kWorldHoverRadius = 42.0f;

uint32_t NowMs();

void WantBuffs(int) {}
void ServiceBuffProbe() {}

uint32_t NowMs()
{
	return static_cast<uint32_t>(EQGetTime());
}

int PickSpawnUnderMouse(const ImVec2& mouse)
{
	CDisplay* display = pDisplay.get();
	PlayerClient* me = pLocalPlayer.get();
	if (!display || !display->pCamera || !me)
		return 0;

	int    best = 0;
	float  bestScore = kWorldHoverRadius * kWorldHoverRadius;

	for (PlayerClient* sp = pSpawnList; sp; sp = sp->pNext)
	{
		if (sp == me || !sp->Targetable)
			continue;

		const eSpawnType type = GetSpawnType(sp);
		if (type != PC && type != NPC && type != MERCENARY && type != PET)
			continue;

		if (GetDistanceSquared3D(me, sp) > 400.0f * 400.0f)
			continue;

		float sx = 0.0f, sy = 0.0f;
		const float aimZ = sp->Z + sp->AvatarHeight * 0.5f;
		if (!display->pCamera->ProjectWorldCoordinatesToScreen(CVector3(sp->Y, sp->X, aimZ), sx, sy))
			continue;

		const float dx = sx - mouse.x, dy = sy - mouse.y;
		const float d2 = dx * dx + dy * dy;
		if (d2 < bestScore)
		{
			bestScore = d2;
			best = sp->SpawnID;
		}
	}

	return best;
}

} // namespace

int  GetSecondaryTarget() { return s_secondary.load(std::memory_order_relaxed); }
int  GetHoverUnit()       { return s_hover.load(std::memory_order_relaxed); }
void SetHoverUnit(int id) { s_hover.store(id, std::memory_order_relaxed); }
void UnitsWantBuffs(int spawnId) { WantBuffs(spawnId); }

bool BuffProbeAvailable()
{
	return false;
}

UnitOptions GetUnitOptions()
{
	UnitOptions o;
	o.worldHover        = s_worldHover.load(std::memory_order_relaxed);
	o.focusMarker       = s_focusMarker.load(std::memory_order_relaxed);
	o.hoverFallback     = s_hoverFallback.load(std::memory_order_relaxed);
	o.buffProbe         = false;
	return o;
}

void SetUnitOptions(const UnitOptions& opts)
{
	s_worldHover.store(opts.worldHover, std::memory_order_relaxed);
	s_focusMarker.store(opts.focusMarker, std::memory_order_relaxed);
	s_hoverFallback.store(opts.hoverFallback, std::memory_order_relaxed);
}

void SetSecondaryTarget(int spawnId)
{
	s_secondary.store(spawnId, std::memory_order_relaxed);
}

bool GetSpawnCast(int spawnId, UnitCast& out)
{
	std::scoped_lock lock(s_unitMutex);
	auto it = s_casts.find(spawnId);
	if (it == s_casts.end())
		return false;
	out = it->second;
	return true;
}

void UnitsNoteCastStart(int spellId, int spawnId, uint32_t castTimeMs)
{
	if (spawnId <= 0 || spellId <= 0)
		return;

	if (castTimeMs <= 100)
		return;

	std::scoped_lock lock(s_unitMutex);
	s_casts[spawnId] = UnitCast{ spellId, NowMs(), castTimeMs };
}

int ResolveCastUnit()
{
	const int hover = s_hover.load(std::memory_order_relaxed);
	if (hover != 0 && GetSpawnByID(hover))
		return hover;

	if (!s_hoverFallback.load(std::memory_order_relaxed))
		return 0;

	const int focus = s_secondary.load(std::memory_order_relaxed);
	if (focus != 0 && GetSpawnByID(focus))
		return focus;

	return pTarget ? pTarget->SpawnID : 0;
}

bool CastAtSpawn(int spawnId, int gem)
{
	if (gem < 1 || gem > NUM_SPELL_GEMS)
	{
		WriteChatf("\ag[MQEQGFX]\ax \arcast: gem must be 1-%d\ax", NUM_SPELL_GEMS);
		return false;
	}

	PlayerClient* unit = GetSpawnByID(spawnId);
	PlayerClient* me = pLocalPlayer.get();
	if (!unit || !me)
		return false;

	const int spellId = GetMemorizedSpell(gem - 1);
	if (spellId <= 0)
	{
		WriteChatf("\ag[MQEQGFX]\ax \arcast: gem %d is empty\ax", gem);
		return false;
	}

	if (!cmdCast)
	{
		WriteChatf("\ag[MQEQGFX]\ax \arcast: the game's /cast handler is unavailable\ax");
		return false;
	}

	PlayerClient* prev = pTarget;
	pTarget = unit;

	char arg[16];
	std::snprintf(arg, sizeof(arg), "%d", gem);
	cmdCast(me, arg);

	pTarget = prev;

	return true;
}

void BuildUnitMarkers(std::vector<eqgfx::gpu::FxInstance>& out)
{
	if (!s_focusMarker.load(std::memory_order_relaxed))
		return;

	const int focusId = s_secondary.load(std::memory_order_relaxed);
	if (focusId == 0)
		return;

	PlayerClient* sp = GetSpawnByID(focusId);
	if (!sp || out.size() >= eqgfx::gpu::kMaxInstances)
		return;
	eqgfx::gpu::FxInstance ix = {};
	ix.pos[0] = sp->X; ix.pos[1] = sp->Y; ix.pos[2] = sp->Z;
	ix.form = eqgfx::gpu::FxForm_Ring;
	ix.radius = 4.5f;
	ix.height = 0.4f;
	ix.speed = 0.7f;
	ix.width = 0.22f;
	ix.segments = 48;
	ix.p1 = 0.5f; ix.p2 = 1.0f; ix.p3 = 0.3f;
	ix.flags = eqgfx::gpu::FxFlag_Comet | eqgfx::gpu::FxFlag_Additive;
	ix.rgba[0] = 0.85f; ix.rgba[1] = 0.35f; ix.rgba[2] = 1.0f; ix.rgba[3] = 0.9f;
	out.push_back(ix);
}

void PulseUnits()
{
	const uint32_t now = NowMs();

	static bool announced = false;
	if (!announced && pLocalPlayer)
	{
		announced = true;
		WriteChatf("\ag[MQEQGFX]\ax unit frames are up. \ay/eqgfx uf\ax for the windows, "
			"\ay/eqgfx bind\ax for click and hover gems, \ay/eqgfx help\ax for everything.");
		WriteChatf("\ag[MQEQGFX]\ax cast at anything without targeting it: "
			"\ay/attarget <2Tar|Group1-6|Raid1-72|XTar1-20|MouseHover> <command>\ax "
			"- bind it to a key. \ay/attarget\ax alone for help.");
	}
	WantBuffs(s_secondary.load(std::memory_order_relaxed));
	WantBuffs(s_hover.load(std::memory_order_relaxed));

	ServiceBuffProbe();

	eqgfx::unitframes::Pulse();
	eqgfx::unitframes::FlushSettings();

	std::scoped_lock lock(s_unitMutex);
	for (auto it = s_casts.begin(); it != s_casts.end(); )
	{
		const UnitCast& uc = it->second;
		const uint32_t age = now - uc.startMs;
		if (age > uc.castMs || age > kCastMaxMs)
			it = s_casts.erase(it);
		else
			++it;
	}
}

void OnUnitSpawnRemoved(int spawnId)
{
	if (s_secondary.load(std::memory_order_relaxed) == spawnId)
		s_secondary.store(0, std::memory_order_relaxed);
	if (s_hover.load(std::memory_order_relaxed) == spawnId)
		s_hover.store(0, std::memory_order_relaxed);

	eqgfx::unitframes::ForgetUnit(spawnId);

	std::scoped_lock lock(s_unitMutex);
	s_casts.erase(spawnId);
}

void DrawUnitsUI()
{
	s_hover.store(0, std::memory_order_relaxed);

	eqgfx::unitframes::Draw();

	if (!eqgfx::unitframes::ShouldDraw())
		return;

	if (s_worldHover.load(std::memory_order_relaxed)
		&& s_hover.load(std::memory_order_relaxed) == 0
		&& !ImGui::GetIO().WantCaptureMouse)
	{
		s_hover.store(PickSpawnUnderMouse(eqgfx::unitframes::MousePos()), std::memory_order_relaxed);
	}
}

void InitUnits()
{
	s_secondary.store(0);
	s_hover.store(0);

	eqgfx::unitframes::LoadSettings();
}

void ShutdownUnits()
{
	s_secondary.store(0);
	s_hover.store(0);

	{
		std::scoped_lock lock(s_unitMutex);
		s_casts.clear();
	}

	eqgfx::unitframes::SaveSettings();
	eqgfx::unitframes::ShutdownIcons();
	eqgfx::unitframes::ShutdownWindows();
}

int ResolveSelector(const char* sel)
{
	if (!sel || !*sel)
		return 0;

	if (ci_equals(sel, "2tar") || ci_equals(sel, "focus") || ci_equals(sel, "secondary"))
		return GetSecondaryTarget();

	if (ci_equals(sel, "mousehover") || ci_equals(sel, "hover") || ci_equals(sel, "mouseover"))
		return GetHoverUnit();

	if (ci_equals(sel, "me") || ci_equals(sel, "self"))
		return pLocalPlayer ? pLocalPlayer->SpawnID : 0;

	if (ci_equals(sel, "target"))
		return pTarget ? pTarget->SpawnID : 0;

	if (_strnicmp(sel, "group", 5) == 0 && IsNumber(sel + 5))
	{
		const int n = GetIntFromString(sel + 5, 0);
		if (n < 1 || n > MAX_GROUP_SIZE || !pLocalPC || !pLocalPC->Group)
			return 0;
		CGroupMember* gm = pLocalPC->Group->GetGroupMember(n - 1);
		PlayerClient* sp = gm ? gm->GetPlayer() : nullptr;
		return sp ? sp->SpawnID : 0;
	}

	if (_strnicmp(sel, "raid", 4) == 0 && IsNumber(sel + 4))
	{
		const int n = GetIntFromString(sel + 4, 0);
		if (n < 1 || n > MAX_RAID_SIZE || !pRaid)
			return 0;
		const RaidMember& rm = pRaid->RaidMember[n - 1];
		if (rm.Name[0] == 0)
			return 0;
		PlayerClient* sp = GetSpawnByName(rm.Name);
		return sp ? sp->SpawnID : 0;
	}

	if (_strnicmp(sel, "xtar", 4) == 0 && IsNumber(sel + 4))
	{
		const int n = GetIntFromString(sel + 4, 0);
		if (n < 1 || !pLocalPC || !pLocalPC->pExtendedTargetList)
			return 0;
		if (n > pLocalPC->pExtendedTargetList->GetNumSlots())
			return 0;
		ExtendedTargetSlot* xts = pLocalPC->pExtendedTargetList->GetSlot(n - 1);
		return xts ? static_cast<int>(xts->SpawnID) : 0;
	}

	if (const int id = GetIntFromString(sel, 0))
	{
		PlayerClient* byId = GetSpawnByID(id);
		if (byId && byId->Targetable)
			return id;
	}

	PlayerClient* byName = GetSpawnByName(sel);
	return byName && byName->Targetable ? byName->SpawnID : 0;
}

void AtTargetCommand(PlayerClient*, const char* szLine)
{
	char szUnit[MAX_STRING] = { 0 };
	GetArg(szUnit, szLine, 1);

	if (szUnit[0] == 0)
	{
		WriteChatf("\ag[MQEQGFX]\ax usage: /attarget <unit> <command>");
		WriteChatf("\ag[MQEQGFX]\ax   unit: \ay2Tar\ax | \ayGroup1-%d\ax | \ayRaid1-%d\ax | "
			"\ayXTar1-%d\ax | \ayMouseHover\ax | Me | Target | <id> | <name>",
			MAX_GROUP_SIZE, MAX_RAID_SIZE,
			(pLocalPC && pLocalPC->pExtendedTargetList)
				? pLocalPC->pExtendedTargetList->GetNumSlots() : 20);
		WriteChatf("\ag[MQEQGFX]\ax   e.g. /attarget MouseHover /cast 3");
		WriteChatf("\ag[MQEQGFX]\ax        /attarget 2Tar /alt act 1234");
		WriteChatf("\ag[MQEQGFX]\ax        /attarget Group2 /useitem \"Rod of Mending\"");
		return;
	}

	const char* rest = GetNextArg(szLine, 1);
	while (*rest == ' ' || *rest == '\t')
		++rest;

	if (*rest == 0)
	{
		WriteChatf("\ag[MQEQGFX]\ax \arno command given\ax - /attarget %s <command>", szUnit);
		return;
	}

	if (*rest != '/')
	{
		WriteChatf("\ag[MQEQGFX]\ax \arthe command must start with /\ax - /attarget %s /cast 3", szUnit);
		return;
	}

	const int spawnId = ResolveSelector(szUnit);
	if (spawnId == 0)
	{
		WriteChatf("\ag[MQEQGFX]\ax \ar%s did not resolve to a spawn in zone\ax", szUnit);
		return;
	}

	PlayerClient* unit = GetSpawnByID(spawnId);
	if (!unit)
	{
		WriteChatf("\ag[MQEQGFX]\ax \arspawn %d is not in zone\ax", spawnId);
		return;
	}

	PlayerClient* prev = pTarget;
	pTarget = unit;

	DoCommand(rest, false);

	pTarget = prev;
}

bool UnitsCommand(const char* szCmd, const char* szLine)
{
	char a1[MAX_STRING] = { 0 }, a2[MAX_STRING] = { 0 };
	GetArg(a1, szLine, 2);
	GetArg(a2, szLine, 3);

	if (eqgfx::unitframes::Command(szCmd, szLine))
		return true;

	if (ci_equals(szCmd, "focus"))
	{
		if (a1[0] == 0)
		{
			if (!pTarget)
			{
				WriteChatf("\ag[MQEQGFX]\ax \arno target to focus\ax");
				return true;
			}
			SetSecondaryTarget(pTarget->SpawnID);
			WriteChatf("\ag[MQEQGFX]\ax focus set to \ay%s\ax", pTarget->DisplayedName);
		}
		else if (ci_equals(a1, "clear") || ci_equals(a1, "off"))
		{
			SetSecondaryTarget(0);
			WriteChatf("\ag[MQEQGFX]\ax focus cleared");
		}
		else if (ci_equals(a1, "target"))
		{
			if (PlayerClient* sp = GetSpawnByID(GetSecondaryTarget()))
			{
				pTarget = sp;
				WriteChatf("\ag[MQEQGFX]\ax targeting focus \ay%s\ax", sp->DisplayedName);
			}
		}
		else if (ci_equals(a1, "marker"))
		{
			const bool on = !s_focusMarker.load();
			s_focusMarker.store(on);
			WriteChatf("\ag[MQEQGFX]\ax focus marker %s", on ? "\agON\ax" : "\ayOFF\ax");
		}
		else if (ci_equals(a1, "window"))
		{
			eqgfx::unitframes::UnitWindow& focus =
				eqgfx::unitframes::WindowFor(eqgfx::unitframes::Window::Focus);
			focus.Show(!focus.Shown());
			eqgfx::unitframes::SaveSettings();
			WriteChatf("\ag[MQEQGFX]\ax focus window %s", focus.Shown() ? "\agON\ax" : "\ayOFF\ax");
		}
		else
		{
			if (PlayerClient* sp = GetSpawnByName(a1))
			{
				SetSecondaryTarget(sp->SpawnID);
				WriteChatf("\ag[MQEQGFX]\ax focus set to \ay%s\ax", sp->DisplayedName);
			}
			else
			{
				WriteChatf("\ag[MQEQGFX]\ax \arno spawn named '%s'\ax", a1);
			}
		}
		return true;
	}

	if (ci_equals(szCmd, "hovercast") || ci_equals(szCmd, "focuscast"))
	{
		WriteChatf("\ag[MQEQGFX]\ax that is now \ay/attarget\ax:");
		WriteChatf("\ag[MQEQGFX]\ax   /attarget MouseHover /cast <gem>");
		WriteChatf("\ag[MQEQGFX]\ax   /attarget 2Tar /cast <gem>");
		return true;
	}

	if (ci_equals(szCmd, "frames"))
	{
		using eqgfx::unitframes::Window;
		using eqgfx::unitframes::WindowFor;

		if (a1[0] == 0 || ci_equals(a1, "toggle") || ci_equals(a1, "on") || ci_equals(a1, "off"))
		{
			const bool on = ci_equals(a1, "on") ? true
				: ci_equals(a1, "off") ? false
				: !WindowFor(Window::Group).Shown();
			WindowFor(Window::Group).Show(on);
			eqgfx::unitframes::SaveSettings();
			WriteChatf("\ag[MQEQGFX]\ax group frames %s", on ? "\agON\ax" : "\ayOFF\ax");
		}
		else if (ci_equals(a1, "clickgem"))
		{
			WriteChatf("\ag[MQEQGFX]\ax click gems are per window now: "
				"\ay/eqgfx bind group left %s\ax (also hotkey and right)", a2[0] ? a2 : "<gems>");
		}
		else if (ci_equals(a1, "raid"))
		{
			const bool on = !WindowFor(Window::Raid).Shown();
			WindowFor(Window::Raid).Show(on);
			eqgfx::unitframes::SaveSettings();
			WriteChatf("\ag[MQEQGFX]\ax raid window %s", on ? "\agON\ax" : "\ayOFF\ax");
		}
		else if (ci_equals(a1, "worldhover"))
		{
			const bool on = !s_worldHover.load();
			s_worldHover.store(on);
			WriteChatf("\ag[MQEQGFX]\ax world hover %s", on ? "\agON\ax" : "\ayOFF\ax");
		}
		else if (ci_equals(a1, "fallback"))
		{
			const bool on = !s_hoverFallback.load();
			s_hoverFallback.store(on);
			WriteChatf("\ag[MQEQGFX]\ax hovercast fallback to focus/target %s", on ? "\agON\ax" : "\ayOFF\ax");
		}
		else
		{
			WriteChatf("\ag[MQEQGFX]\ax frames [on|off|raid|worldhover|fallback]"
				" - every window and its gems live under \ay/eqgfx uf\ax and \ay/eqgfx bind\ax");
		}
		return true;
	}

	if (ci_equals(szCmd, "xtarget") || ci_equals(szCmd, "xt"))
	{
		using eqgfx::unitframes::Window;
		using eqgfx::unitframes::WindowFor;

		const bool on = ci_equals(a1, "on") ? true
			: ci_equals(a1, "off") ? false
			: !WindowFor(Window::XTarget).Shown();
		WindowFor(Window::XTarget).Show(on);
		eqgfx::unitframes::SaveSettings();
		WriteChatf("\ag[MQEQGFX]\ax extended target window %s", on ? "\agON\ax" : "\ayOFF\ax");
		return true;
	}

	if (ci_equals(szCmd, "units"))
	{
		const int focus = GetSecondaryTarget();
		const int hover = GetHoverUnit();
		PlayerClient* fs = GetSpawnByID(focus);
		PlayerClient* hs = GetSpawnByID(hover);

		size_t casts;
		{
			std::scoped_lock lock(s_unitMutex);
			casts = s_casts.size();
		}

		WriteChatf("\ag[MQEQGFX]\ax focus: %s | hover: %s",
			fs ? fs->DisplayedName : "\aynone\ax",
			hs ? hs->DisplayedName : "\aynone\ax");
		WriteChatf("\ag[MQEQGFX]\ax world hover %s, focus marker %s - windows and gems: \ay/eqgfx uf\ax",
			s_worldHover.load() ? "\agon\ax" : "\ayoff\ax",
			s_focusMarker.load() ? "\agon\ax" : "\ayoff\ax");
		WriteChatf("\ag[MQEQGFX]\ax fallback %s, %d cast bars live",
			s_hoverFallback.load() ? "on" : "off",
			static_cast<int>(casts));
		return true;
	}

	return false;
}
