/*
 * Units.cpp - WoW-style secondary target ("focus") and hover healing.
 *
 * WHY THIS CAN WORK AT ALL
 * ------------------------
 * EverQuest has no "cast spell at unit X" entry point. CharacterZoneClient::
 * CastSpell (eqgame 0x1400DD130) takes a gem and a spell, never a target, and
 * every targeted spell is aimed at whatever the global pTarget happens to be.
 * That is why the usual macro answer is /target then /cast, which costs a
 * server round trip and steals the player's selection.
 *
 * Reading the function shows that is not necessary. The outgoing packet is a
 * 0x2C-byte record serialized just before the send:
 *
 *     if (pinstTarget == NULL) hdr.targetId = 0;
 *     else                     hdr.targetId = pinstTarget->SpawnID;   // +0x168
 *     ...
 *     CPacketScrambler::hton(pinstCPacketScrambler, 0xead6);
 *     EQNetwork::SendPacket(__gWorld, 4, buf, len);
 *
 * The target ID is read once, into the packet, synchronously - and the client
 * also copies it onto the character (+0x250) as the in-progress cast's target,
 * so the rest of the cast never consults pTarget again. Swapping pTarget,
 * casting, and swapping back therefore aims the spell without the player ever
 * losing their selection and without a single extra packet.
 *
 * We drive the cast through the game's own /cast handler (cmdCast) rather than
 * calling CastSpell directly: it is the same code path one step up, it does all
 * the gem/mana/recast validation for us, and its signature cannot drift out of
 * sync with the client the way a hand-declared export can.
 *
 * WHERE THE DATA COMES FROM
 * -------------------------
 * Buffs: MacroQuest already detours CTargetWnd::RefreshTargetBuffs and caches
 * what it parses in gCachedBuffMap, keyed by the spawn ID carried in the packet
 * header - not by "the current target". So every unit whose buffs the server
 * has ever sent us is queryable via FilterCachedBuffs(). We consume that rather
 * than installing a second detour on the same function and fighting MQ for it.
 *
 * Cast bars: CEverQuest::StartCasting is already tapped in Spells.cpp for the
 * Lua cast queue. It hands us (spellId, spawnId, castTimeMs) the moment the
 * packet lands, which is exactly what a focus-target cast bar needs - including
 * for the mob you are watching, so you can see its cast without targeting it.
 *
 * THREADING
 * ---------
 * DrawUnitsUI is render thread. Everything else is the main pulse thread. The
 * shared state is small and guarded by s_unitMutex; the two ids are atomics so
 * the UI can read them without taking a lock every frame.
 */

#include "MQEQGFX.h"

#include "gpu/FxCatalog.h"

#include "eqlib/game/ClassInfo.h"
#include "eqlib/graphics/CameraInterface.h"
#include "eqlib/Offsets.h"
#include "eqlib/offsets/private-eqgame.h"

#include <imgui.h>
#include <mq/imgui/Widgets.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <unordered_map>
#include <unordered_set>

namespace {

// ---------------------------------------------------------------------------
// State

std::mutex                            s_unitMutex;
std::unordered_map<int, UnitCast>     s_casts;         // guarded - spawnId -> cast

std::atomic<int>  s_secondary{ 0 };      // the focus target's spawn id, 0 = none
std::atomic<int>  s_hover{ 0 };          // spawn under the mouse this frame

// All three default visible. A window you have to already know about in order
// to turn on may as well not exist; each has a close button, and closing it
// sticks for the session.
std::atomic<bool> s_showFrames{ true };  // the group/raid unit-frame window
std::atomic<bool> s_showFocus{ true };   // the focus panel
std::atomic<bool> s_showXTarget{ true }; // the extended target list
std::atomic<bool> s_worldHover{ true };  // hovering a spawn in the 3D world counts
std::atomic<bool> s_includeRaid{ true };
std::atomic<bool> s_focusMarker{ true }; // draw a GPU ring under the focus
// Off by default: unlike everything else here, it puts packets on the wire.
// See SendTargetNotify for exactly what it sends and what it costs.
std::atomic<bool> s_buffProbe{ false };
std::atomic<bool> s_hoverFallback{ true };  // hover -> focus -> target, vs hover only

// Cast bars older than this are dropped even if we never saw an end event.
// Emu servers do not always send one, and a stuck bar is worse than a missing
// one. Generous enough for the longest real cast plus latency.
constexpr uint32_t kCastMaxMs = 60000;

// How close (in pixels) the mouse must be to a spawn's projected position for
// world hover to claim it. Roughly a nameplate's worth of slack.
constexpr float kWorldHoverRadius = 42.0f;

// Deferred restore: when the player asks for it, we hold the swapped target
// until the cast completes rather than putting it back immediately. Stored as
// an id, not a pointer, because the spawn can die inside that window.
int      s_restoreToId  = 0;
uint32_t s_restoreAtMs  = 0;
bool     s_restorePending = false;
bool     s_restoreOnComplete = false;    // the mode itself

// The gem a plain click on a unit frame casts. 0 = clicking only selects.
int s_clickGem = 0;

// Spell icon sheet, built lazily on the render thread.
CTextureAnimation* s_spellIcons = nullptr;

// ---------------------------------------------------------------------------
// Buff probe: asking the server about a spawn we are not targeting.
//
// The server only volunteers a spawn's buffs when it believes that spawn is
// your target, so the cache alone can never answer "what is on someone I have
// never clicked". Reading PlayerZoneClient::ChangeTarget (1400f3b40 - really
// the client's per-frame state-diff sender) shows how the server is told:
//
//     DAT_140de5b50 = pinstTarget;                     // shadow of last sent
//     hton(pinstCPacketScrambler, 0x1994);
//     <2 bytes opcode><4 bytes spawn id>
//     EQNetwork::SendPacket(__gWorld, 4, buf, 6);
//
// It fires only when pTarget differs from that shadow - which is also why the
// hover cast is silent: we put pTarget back inside one pulse, the diff never
// sees a change, and nothing goes out.
//
// A probe sends that same 6-byte notification by hand for the spawn we want,
// immediately followed by one for the real target. The server answers the
// first with a buff packet (which MQ's RefreshTargetBuffs detour caches by the
// spawn id in the packet header) and the second restores its notion of our
// target. pTarget is never touched, so nothing flickers locally.
//
// The cost is worth stating plainly: for the moment between those two packets
// the server believes we are targeting someone we are not. Anything
// server-side that reads our target in that window - another player's /assist,
// say - sees the probe target. That is why this is opt-in and hard rate
// limited rather than simply on.

uint32_t NowMs();                        // defined with the helpers below

constexpr uint16_t kOpTargetNotify = 0x1994;

// One probe at a time, and never faster than this no matter how many frames
// ask. Buffs do not change quickly enough to justify more.
constexpr uint32_t kProbeIntervalMs = 750;

// Don't re-probe the same spawn until its data is genuinely stale.
constexpr uint32_t kProbeSpawnCooldownMs = 15000;

std::unordered_set<int>           s_probeWant;     // guarded - what the UI asked for
std::unordered_map<int, uint32_t> s_probeLast;     // guarded - spawn -> last probe
uint32_t                          s_probeLastAny = 0;

using fnHton       = uint16_t (*)(void*, uint16_t);
using fnSendPacket = uint8_t  (*)(uintptr_t, int, const void*, unsigned int);

#pragma pack(push, 1)
struct TargetNotifyPacket
{
	uint16_t opcode;
	uint32_t spawnId;
};
#pragma pack(pop)
static_assert(sizeof(TargetNotifyPacket) == 6, "target notify is 6 bytes on the wire");

/// Tell the server our target is `spawnId`, without touching pTarget. False
/// when any of the game-side pieces aren't resolved yet.
bool SendTargetNotify(int spawnId)
{
	if (!__gWorld || !CPacketScrambler__hton)
		return false;

	// eqlib's globals hold the ADDRESS of the game's pointer rather than the
	// pointer itself - that indirection is what ForeignPointer wraps - so both
	// of these need a deref before use.
	const uintptr_t world = *reinterpret_cast<uintptr_t*>(__gWorld);
	if (!world)
		return false;

	void** scramblerSlot = reinterpret_cast<void**>(
		eqlib::FixEQGameOffset(pinstCPacketScrambler_x));
	if (!scramblerSlot || !*scramblerSlot)
		return false;

	const uintptr_t sendAddr = eqlib::FixEQGameOffset(EQNetwork__SendPacket_x);
	if (!sendAddr)
		return false;

	TargetNotifyPacket pkt;
	pkt.opcode  = reinterpret_cast<fnHton>(CPacketScrambler__hton)(*scramblerSlot, kOpTargetNotify);
	pkt.spawnId = static_cast<uint32_t>(spawnId);

	return reinterpret_cast<fnSendPacket>(sendAddr)(world, 4, &pkt, sizeof(pkt)) != 0;
}

/// Ask for this spawn's buffs at the next opportunity. Safe from the render
/// thread - the packet itself only ever goes out from the pulse thread.
void WantBuffs(int spawnId)
{
	if (spawnId <= 0 || !s_buffProbe.load(std::memory_order_relaxed))
		return;

	std::scoped_lock lock(s_unitMutex);
	if (s_probeWant.size() < 64)
		s_probeWant.insert(spawnId);
}

/// Send at most one probe. Pulse thread only.
void ServiceBuffProbe()
{
	if (!s_buffProbe.load(std::memory_order_relaxed))
		return;

	const uint32_t now = NowMs();
	if (now - s_probeLastAny < kProbeIntervalMs)
		return;

	int pick = 0;
	{
		std::scoped_lock lock(s_unitMutex);
		for (auto it = s_probeWant.begin(); it != s_probeWant.end(); )
		{
			const int id = *it;
			auto last = s_probeLast.find(id);
			if (last != s_probeLast.end() && now - last->second < kProbeSpawnCooldownMs)
			{
				it = s_probeWant.erase(it);      // still fresh - drop the request
				continue;
			}
			pick = id;
			it = s_probeWant.erase(it);
			break;
		}
	}

	if (pick == 0 || !GetSpawnByID(pick))
		return;

	// Probe, then hand the server straight back to the real target. Both go out
	// in order on the same connection, so the window where its idea of our
	// target is wrong is as small as it can be made.
	if (!SendTargetNotify(pick))
		return;
	SendTargetNotify(pTarget ? pTarget->SpawnID : 0);

	s_probeLastAny = now;

	std::scoped_lock lock(s_unitMutex);
	s_probeLast[pick] = now;
}

// ---------------------------------------------------------------------------
// Helpers

uint32_t NowMs()
{
	return static_cast<uint32_t>(EQGetTime());
}

/// The class's signature colour from the compiled catalogue - the same World of
/// Warcraft palette the 3D effects use, so a healer's frames and the rings
/// under their feet agree.
ImU32 ClassColor(PlayerClient* sp, float alpha = 1.0f)
{
	if (sp)
	{
		const int cls = sp->GetClass();
		if (cls > 0 && cls < static_cast<int>(sizeof(eqlib::ClassInfo) / sizeof(eqlib::ClassInfo[0])))
		{
			if (const eqgfx::gpu::ClassFx* fx = eqgfx::gpu::FindClassFx(eqlib::ClassInfo[cls].UCShortName))
			{
				const float* c = fx->defs[0].rgba;
				return ImGui::GetColorU32(ImVec4(c[0], c[1], c[2], alpha));
			}
		}
	}
	return ImGui::GetColorU32(ImVec4(0.75f, 0.75f, 0.75f, alpha));
}

const char* ClassShort(PlayerClient* sp)
{
	if (!sp)
		return "";
	const int cls = sp->GetClass();
	if (cls > 0 && cls < static_cast<int>(sizeof(eqlib::ClassInfo) / sizeof(eqlib::ClassInfo[0])))
		return eqlib::ClassInfo[cls].UCShortName;
	return "";
}

/// Health as 0..1. For anyone but ourselves the client only knows a percent,
/// which it stores in HPCurrent with HPMax at 100.
float PctHP(PlayerClient* sp)
{
	if (!sp)
		return 0.0f;
	const int64_t maxHp = sp->HPMax;
	if (maxHp <= 0)
		return 0.0f;
	const float pct = static_cast<float>(sp->HPCurrent) / static_cast<float>(maxHp);
	return std::clamp(pct, 0.0f, 1.0f);
}

float DistanceTo(PlayerClient* sp)
{
	PlayerClient* me = pLocalPlayer.get();
	if (!me || !sp)
		return 0.0f;
	const float dx = sp->X - me->X, dy = sp->Y - me->Y, dz = sp->Z - me->Z;
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

/// Green through red, the way every raid frame in existence does it.
ImVec4 HealthColor(float pct)
{
	if (pct > 0.5f)
	{
		const float t = (pct - 0.5f) * 2.0f;             // 0 at half, 1 at full
		return ImVec4(1.0f - t * 0.85f, 0.78f, 0.16f, 0.9f);
	}
	const float t = pct * 2.0f;                          // 0 at dead, 1 at half
	return ImVec4(0.85f, 0.16f + t * 0.62f, 0.16f, 0.9f);
}

/// The list of units the frames show: your group, then the rest of the raid.
/// Deduplicated, self first, missing/out-of-zone members skipped - we can only
/// draw and cast at spawns that actually exist in the zone.
void CollectUnits(std::vector<PlayerClient*>& out)
{
	out.clear();

	PlayerClient* me = pLocalPlayer.get();
	if (me)
		out.push_back(me);

	auto pushUnique = [&out](PlayerClient* sp) {
		if (!sp)
			return;
		if (std::find(out.begin(), out.end(), sp) == out.end())
			out.push_back(sp);
	};

	if (pLocalPC && pLocalPC->Group)
	{
		const uint32_t maxSize = pLocalPC->Group->GetMaxGroupSize();
		for (uint32_t i = 0; i < maxSize; ++i)
		{
			if (CGroupMember* gm = pLocalPC->Group->GetGroupMember(static_cast<int>(i)))
				pushUnique(gm->GetPlayer());
		}
	}

	if (s_includeRaid.load(std::memory_order_relaxed) && pRaid && pRaid->RaidMemberCount > 0)
	{
		const int count = std::min<int>(pRaid->RaidMemberCount, MAX_RAID_SIZE);
		for (int i = 0; i < count; ++i)
		{
			const RaidMember& rm = pRaid->RaidMember[i];
			if (rm.Name[0] == 0)
				continue;
			pushUnique(GetSpawnByName(rm.Name));
		}
	}
}

/// Cast progress for a spawn as 0..1, or -1 when it isn't casting. Also yields
/// the spell so the caller can name it.
float CastProgress(int spawnId, int& spellIdOut)
{
	spellIdOut = 0;

	UnitCast uc;
	if (!GetSpawnCast(spawnId, uc) || uc.castMs == 0)
		return -1.0f;

	const uint32_t now = NowMs();
	if (now < uc.startMs)
		return -1.0f;
	const uint32_t elapsed = now - uc.startMs;
	if (elapsed >= uc.castMs)
		return -1.0f;

	spellIdOut = uc.spellId;
	return static_cast<float>(elapsed) / static_cast<float>(uc.castMs);
}

// ---------------------------------------------------------------------------
// World hover: which spawn is the mouse pointing at?
//
// The engine's own picker works on actors and would need the scene graph walked
// per frame. Projecting the handful of plausible candidates is cheaper and,
// more usefully, agrees exactly with where the player sees the nameplate - the
// same camera call the rest of the plugin projects through.

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
		if (sp == me)
			continue;

		const eSpawnType type = GetSpawnType(sp);
		if (type != PC && type != NPC && type != MERCENARY && type != PET)
			continue;

		const float dist = DistanceTo(sp);
		if (dist > 400.0f)
			continue;

		// Aim at the middle of the model, which is where the mouse naturally
		// goes, rather than at the feet.
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

// ---------------------------------------------------------------------------
// UI pieces

/// Row of cached buff icons for a unit, with names on hover. The cache is fed
/// by MQ's RefreshTargetBuffs detour, so it is populated for anyone the server
/// has sent buffs about - which for a healer means everyone they have targeted.
void DrawBuffRow(PlayerClient* sp, float iconSize)
{
	if (!sp)
		return;

	std::vector<CachedBuff> buffs = FilterCachedBuffs(sp, [](const CachedBuff&) { return true; });
	if (buffs.empty())
	{
		// Nothing cached for this unit. With the probe enabled we can ask the
		// server for it; without it, the only way this fills in is the player
		// targeting them at some point.
		WantBuffs(sp->SpawnID);
		ImGui::TextDisabled(s_buffProbe.load(std::memory_order_relaxed)
			? "(requesting buffs...)"
			: "(no buff data - target them once, or /eqgfx frames probe)");
		return;
	}

	if (!s_spellIcons)
	{
		s_spellIcons = new CTextureAnimation();
		if (CTextureAnimation* src = pSidlMgr ? pSidlMgr->FindAnimation("A_SpellIcons") : nullptr)
			*s_spellIcons = *src;
	}

	const float avail = ImGui::GetContentRegionAvail().x;
	float used = 0.0f;

	for (const CachedBuff& b : buffs)
	{
		EQ_Spell* spell = GetSpellByID(b.spellId);
		if (!spell)
			continue;

		if (used > 0.0f && used + iconSize > avail)
			used = 0.0f;                       // wrap
		else if (used > 0.0f)
			ImGui::SameLine(0.0f, 2.0f);

		ImGui::PushID(b.spellId);
		if (s_spellIcons && s_spellIcons->GetWidth() > 0)
		{
			s_spellIcons->SetCurCell(spell->SpellIcon);
			mq::imgui::DrawTextureAnimation(s_spellIcons,
				CXSize(static_cast<int>(iconSize), static_cast<int>(iconSize)));
		}
		else
		{
			// No icon sheet (early load, or a UI without it): a coloured chip
			// still tells you something is there and the tooltip does the rest.
			ImGui::ColorButton("##b", ImVec4(0.35f, 0.55f, 0.85f, 1.0f),
				ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop,
				ImVec2(iconSize, iconSize));
		}

		if (ImGui::IsItemHovered())
		{
			const uint32_t left = b.Duration();
			ImGui::SetTooltip("%s\n%s%.1f min remaining\n%s",
				spell->Name,
				b.duration < 0 ? "permanent\n" : "",
				b.duration < 0 ? 0.0 : left / 60000.0,
				b.casterName[0] ? b.casterName : "");
		}
		ImGui::PopID();

		used += iconSize + 2.0f;
	}
}

/// A cast bar, used for both the focus target and any unit frame. Returns false
/// when the unit isn't casting, so callers can skip the row entirely.
bool DrawCastBar(int spawnId, float width)
{
	int spellId = 0;
	const float progress = CastProgress(spawnId, spellId);
	if (progress < 0.0f)
		return false;

	EQ_Spell* spell = GetSpellByID(spellId);
	const char* name = spell ? spell->Name : "Casting";

	// Amber, and it fills - the standard "something is coming" read.
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.95f, 0.72f, 0.18f, 0.95f));
	ImGui::ProgressBar(progress, ImVec2(width, 14.0f), name);
	ImGui::PopStyleColor();
	return true;
}

/// One unit frame. Returns true if this row is hovered.
bool DrawUnitFrame(PlayerClient* sp, bool showBuffs, float width)
{
	if (!sp)
		return false;

	const int id = sp->SpawnID;
	ImGui::PushID(id);

	const float pct = PctHP(sp);
	const bool isFocus = (id == s_secondary.load(std::memory_order_relaxed));
	const bool isTarget = (pTarget && pTarget->SpawnID == id);

	// The whole row is one hit box, so hovering anywhere on the frame counts as
	// hovering the unit - that is the behaviour a mouseover heal needs.
	const ImVec2 start = ImGui::GetCursorScreenPos();
	const float rowH = 34.0f;

	ImGui::InvisibleButton("##row", ImVec2(width, rowH));
	const bool hovered = ImGui::IsItemHovered();
	const bool clickedL = ImGui::IsItemClicked(ImGuiMouseButton_Left);
	const bool clickedR = ImGui::IsItemClicked(ImGuiMouseButton_Right);
	const bool clickedM = ImGui::IsItemClicked(ImGuiMouseButton_Middle);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	const ImVec2 end(start.x + width, start.y + rowH);

	dl->AddRectFilled(start, end, ImGui::GetColorU32(ImVec4(0.08f, 0.08f, 0.10f, 0.85f)), 3.0f);

	// Health fill.
	const ImVec4 hc = HealthColor(pct);
	dl->AddRectFilled(ImVec2(start.x + 1, start.y + 1),
		ImVec2(start.x + 1 + (width - 2) * pct, end.y - 1),
		ImGui::GetColorU32(hc), 3.0f);

	// Class stripe down the left edge, so you can read the frame's role at a
	// glance without parsing text.
	dl->AddRectFilled(start, ImVec2(start.x + 4.0f, end.y), ClassColor(sp), 3.0f);

	// Borders: focus and target get their own, and hover wins over both.
	if (hovered)
		dl->AddRect(start, end, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.95f)), 3.0f, 0, 2.0f);
	else if (isFocus)
		dl->AddRect(start, end, ImGui::GetColorU32(ImVec4(0.85f, 0.35f, 1.0f, 0.95f)), 3.0f, 0, 2.0f);
	else if (isTarget)
		dl->AddRect(start, end, ImGui::GetColorU32(ImVec4(1.0f, 0.85f, 0.25f, 0.9f)), 3.0f, 0, 1.5f);

	char label[128];
	std::snprintf(label, sizeof(label), "%s  %s  %.0f%%  %.0fm",
		sp->DisplayedName, ClassShort(sp), pct * 100.0f, DistanceTo(sp));
	dl->AddText(ImVec2(start.x + 8.0f, start.y + 3.0f),
		ImGui::GetColorU32(ImVec4(1, 1, 1, 0.95f)), label);

	// A cast bar drawn into the bottom strip of the same row keeps the frame
	// one fixed height whether or not the unit is casting.
	int spellId = 0;
	const float progress = CastProgress(id, spellId);
	if (progress >= 0.0f)
	{
		const float by0 = end.y - 11.0f;
		dl->AddRectFilled(ImVec2(start.x + 6.0f, by0), ImVec2(end.x - 6.0f, end.y - 3.0f),
			ImGui::GetColorU32(ImVec4(0, 0, 0, 0.6f)), 2.0f);
		dl->AddRectFilled(ImVec2(start.x + 6.0f, by0),
			ImVec2(start.x + 6.0f + (width - 12.0f) * progress, end.y - 3.0f),
			ImGui::GetColorU32(ImVec4(0.95f, 0.72f, 0.18f, 0.95f)), 2.0f);
		if (EQ_Spell* spell = GetSpellByID(spellId))
			dl->AddText(ImVec2(start.x + 9.0f, by0 - 2.0f),
				ImGui::GetColorU32(ImVec4(1, 1, 1, 0.9f)), spell->Name);
	}

	if (hovered)
	{
		s_hover.store(id, std::memory_order_relaxed);
		ImGui::SetTooltip("%s\nleft: %s\nright: target   middle: set focus",
			sp->DisplayedName,
			s_clickGem > 0 ? "cast bound gem" : "target");
	}

	if (clickedL)
	{
		const int gem = s_clickGem;
		if (gem > 0)
			CastAtSpawn(id, gem);
		else
			pTarget = sp;
	}
	if (clickedR)
		pTarget = sp;
	if (clickedM)
		SetSecondaryTarget(id);

	if (showBuffs)
		DrawBuffRow(sp, 18.0f);

	ImGui::PopID();
	return hovered;
}

void DrawFocusWindow()
{
	const int focusId = s_secondary.load(std::memory_order_relaxed);
	PlayerClient* sp = focusId ? GetSpawnByID(focusId) : nullptr;

	// Draw the empty state rather than vanishing. A window that only appears
	// once you have already used the command teaches nobody the command.
	if (!sp)
	{
		bool emptyOpen = true;
		ImGui::SetNextWindowSize(ImVec2(260, 0), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("EQGFX Focus", &emptyOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextDisabled("no secondary target");
			ImGui::Separator();
			if (ImGui::SmallButton("Set from target"))
			{
				if (pTarget)
					SetSecondaryTarget(pTarget->SpawnID);
			}
			ImGui::TextDisabled("or /eqgfx focus");
			ImGui::TextDisabled("cast at it: /attarget 2Tar /cast 1");
		}
		ImGui::End();

		if (!emptyOpen)
			s_showFocus.store(false);
		return;
	}

	bool open = true;
	ImGui::SetNextWindowSize(ImVec2(260, 0), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("EQGFX Focus", &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ClassColor(sp));
		ImGui::Text("%s", sp->DisplayedName);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::TextDisabled("L%d %s", sp->Level, ClassShort(sp));

		const float pct = PctHP(sp);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, HealthColor(pct));
		char hp[32];
		std::snprintf(hp, sizeof(hp), "%.0f%%", pct * 100.0f);
		ImGui::ProgressBar(pct, ImVec2(-1, 16.0f), hp);
		ImGui::PopStyleColor();

		ImGui::Text("%.0f units away", DistanceTo(sp));

		// The focus target's own target - the other half of what a focus frame
		// is for, since it tells you who a mob is about to hit.
		if (PlayerClient* theirTarget = GetSpawnByID(sp->TargetOfTarget))
		{
			ImGui::TextDisabled("targeting");
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ClassColor(theirTarget));
			ImGui::Text("%s", theirTarget->DisplayedName);
			ImGui::PopStyleColor();
		}

		if (!DrawCastBar(focusId, -1.0f))
			ImGui::TextDisabled("not casting");

		ImGui::Separator();
		DrawBuffRow(sp, 20.0f);

		ImGui::Separator();
		if (ImGui::SmallButton("Target"))
			pTarget = sp;
		ImGui::SameLine();
		if (ImGui::SmallButton("Clear"))
			SetSecondaryTarget(0);
	}
	ImGui::End();

	if (!open)
		s_showFocus.store(false);
}

/// The extended target list, which is the other half of hover casting: the
/// unit frames cover who you heal, this covers what you damage. Same frame
/// widget, so hovering a row feeds the same hover id and /eqgfx hovercast
/// works against a mob without ever selecting it.
void DrawXTargetWindow()
{
	if (!pLocalPC || !pLocalPC->pExtendedTargetList)
		return;

	ExtendedTargetList* xtm = pLocalPC->pExtendedTargetList;

	bool open = true;
	ImGui::SetNextWindowSize(ImVec2(300, 340), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("EQGFX Extended Targets", &open))
	{
		const float width = ImGui::GetContentRegionAvail().x;
		const int slots = xtm->GetNumSlots();
		int shown = 0;

		for (int i = 0; i < slots; ++i)
		{
			ExtendedTargetSlot* xts = xtm->GetSlot(i);
			if (!xts || xts->SpawnID == 0)
				continue;

			PlayerClient* sp = GetSpawnByID(static_cast<int>(xts->SpawnID));
			if (!sp)
				continue;                    // slot filled but out of zone

			// The role (Auto Hater, My Pet's Target, ...) is what makes the
			// list readable, so label the row with it rather than the index.
			const char* role = xtm->ExtendedTargetRoleName(xts->xTargetType);
			if (role && *role)
				ImGui::TextDisabled("%s", role);

			DrawUnitFrame(sp, false, width);
			ImGui::Spacing();
			++shown;
		}

		if (shown == 0)
			ImGui::TextDisabled("no extended targets in zone");
	}
	ImGui::End();

	if (!open)
		s_showXTarget.store(false);
}

void DrawFramesWindow()
{
	static std::vector<PlayerClient*> units;
	static bool showBuffs = true;

	CollectUnits(units);

	bool open = true;
	ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("EQGFX Unit Frames", &open))
	{
		ImGui::Checkbox("buffs", &showBuffs);
		ImGui::SameLine();
		bool raid = s_includeRaid.load(std::memory_order_relaxed);
		if (ImGui::Checkbox("raid", &raid))
			s_includeRaid.store(raid);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70.0f);
		int gem = s_clickGem;
		if (ImGui::InputInt("click gem", &gem, 0, 0))
			s_clickGem = std::clamp(gem, 0, NUM_SPELL_GEMS);

		ImGui::Separator();

		const float width = ImGui::GetContentRegionAvail().x;
		for (PlayerClient* sp : units)
		{
			DrawUnitFrame(sp, showBuffs, width);
			ImGui::Spacing();
		}

		if (units.empty())
			ImGui::TextDisabled("no group or raid members in zone");
	}
	ImGui::End();

	if (!open)
		s_showFrames.store(false);
}

} // namespace

// ---------------------------------------------------------------------------
// Public API

int  GetSecondaryTarget() { return s_secondary.load(std::memory_order_relaxed); }
int  GetHoverUnit()       { return s_hover.load(std::memory_order_relaxed); }
void SetHoverUnit(int id) { s_hover.store(id, std::memory_order_relaxed); }

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

	// Instants have no bar worth drawing and would only flicker.
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

	// The swap window is exactly one synchronous call wide. CastSpell copies
	// the target id into the packet and onto the character before it returns,
	// so by the time we put pTarget back the spell is already aimed.
	PlayerClient* prev = pTarget;
	pTarget = unit;

	char arg[16];
	std::snprintf(arg, sizeof(arg), "%d", gem);
	cmdCast(me, arg);

	if (s_restoreOnComplete && prev != unit)
	{
		// Some servers re-validate the caster's target when the cast lands.
		// Holding the swap until then trades a visibly changed target for
		// certainty; it is off by default because the immediate restore is
		// what makes this feel like a WoW mouseover in the first place.
		EQ_Spell* spell = GetSpellByID(spellId);
		const uint32_t castMs = spell ? static_cast<uint32_t>(spell->CastTime) : 0;
		s_restoreToId = prev ? prev->SpawnID : 0;
		s_restoreAtMs = NowMs() + castMs + 250;
		s_restorePending = true;
	}
	else
	{
		pTarget = prev;
	}

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

	// Purple, matching the focus border in the UI, and additive so it reads
	// through the world without being mistaken for a class effect.
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

	// Say once, in game, what this feature is and how to drive it. Everything
	// here is bindable rather than automatic, which is only useful if you know
	// it is there.
	static bool announced = false;
	if (!announced && pLocalPlayer)
	{
		announced = true;
		WriteChatf("\ag[MQEQGFX]\ax unit frames, focus and extended target windows are open. "
			"\ay/eqgfx frames|focus|xtarget\ax to toggle.");
		WriteChatf("\ag[MQEQGFX]\ax cast at anything without targeting it: "
			"\ay/attarget <2Tar|Group1-6|Raid1-72|XTar1-20|MouseHover> <command>\ax "
			"- bind it to a key. \ay/attarget\ax alone for help.");
	}

	// Deferred retarget, if the player asked for the safer restore mode.
	if (s_restorePending && now >= s_restoreAtMs)
	{
		s_restorePending = false;
		pTarget = s_restoreToId ? GetSpawnByID(s_restoreToId) : nullptr;
	}

	// The focus target and whatever the mouse is on are the two units whose
	// buffs are worth having before they are asked for, so keep them warm.
	WantBuffs(s_secondary.load(std::memory_order_relaxed));
	WantBuffs(s_hover.load(std::memory_order_relaxed));

	ServiceBuffProbe();

	// Drop finished and stale cast bars. Small map, cheap sweep.
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

	std::scoped_lock lock(s_unitMutex);
	s_casts.erase(spawnId);
}

void DrawUnitsUI()
{
	if (GetGameState() != GAMESTATE_INGAME)
		return;

	// Hover is recomputed from scratch every frame: a stale id would aim a heal
	// at whatever the mouse used to be over, which is the one failure mode this
	// feature cannot have.
	s_hover.store(0, std::memory_order_relaxed);

	if (s_showFrames.load(std::memory_order_relaxed))
		DrawFramesWindow();       // sets s_hover if a row is hovered

	if (s_showXTarget.load(std::memory_order_relaxed))
		DrawXTargetWindow();      // also sets s_hover

	if (s_showFocus.load(std::memory_order_relaxed))
		DrawFocusWindow();

	// World hover only when the pointer isn't over one of our frames or any
	// other ImGui window - otherwise the two would fight over the same mouse.
	if (s_worldHover.load(std::memory_order_relaxed)
		&& s_hover.load(std::memory_order_relaxed) == 0
		&& !ImGui::GetIO().WantCaptureMouse)
	{
		s_hover.store(PickSpawnUnderMouse(ImGui::GetIO().MousePos), std::memory_order_relaxed);
	}
}

void InitUnits()
{
	s_secondary.store(0);
	s_hover.store(0);
	s_restorePending = false;
}

void ShutdownUnits()
{
	s_secondary.store(0);
	s_hover.store(0);
	s_restorePending = false;

	{
		std::scoped_lock lock(s_unitMutex);
		s_casts.clear();
	}

	delete s_spellIcons;
	s_spellIcons = nullptr;
}

// ---------------------------------------------------------------------------
// /attarget - run any target-consuming command against a unit of your choice.
//
// This is the general form of the hover cast, and the reason it is a separate
// command rather than a plugin setting: the binding is the player's to make.
// You create the hotkey, you decide it says "MouseHover" and "/cast 3", and
// nothing happens that you did not explicitly ask for.
//
//     /attarget <unit> <command>
//
//     unit     2Tar | Group1-6 | Raid1-72 | XTar1-20 | MouseHover
//              (also: Me, Target, a spawn id, or a name)
//     command  anything that acts on your target - /cast, /alt act,
//              /doability, /useitem, ...
//
// The command runs with pTarget swapped and is restored immediately after.
// All four families above send their packet synchronously, carrying the target
// id with them (see the header of this file), so by the time DoCommand returns
// the action is already aimed and the swap can be undone. DoCommand is called
// with delayed=false for exactly that reason - a queued command would run
// after the restore and hit the wrong unit.

/// Resolve a unit selector to a spawn id. 0 means "could not resolve".
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

	// GroupN - the raw group array, so N matches the slot you see in the
	// group window (1 is you) rather than shifting when someone drops.
	if (_strnicmp(sel, "group", 5) == 0)
	{
		const int n = GetIntFromString(sel + 5, 0);
		if (n < 1 || n > MAX_GROUP_SIZE || !pLocalPC || !pLocalPC->Group)
			return 0;
		CGroupMember* gm = pLocalPC->Group->GetGroupMember(n - 1);
		PlayerClient* sp = gm ? gm->GetPlayer() : nullptr;
		return sp ? sp->SpawnID : 0;
	}

	// RaidN - by name, because the raid roster is names and levels, not spawns.
	if (_strnicmp(sel, "raid", 4) == 0)
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

	// XTarN - the extended target list, which is where the damage targets live.
	if (_strnicmp(sel, "xtar", 4) == 0)
	{
		const int n = GetIntFromString(sel + 4, 0);
		if (n < 1 || !pLocalPC || !pLocalPC->pExtendedTargetList)
			return 0;
		if (n > pLocalPC->pExtendedTargetList->GetNumSlots())
			return 0;
		ExtendedTargetSlot* xts = pLocalPC->pExtendedTargetList->GetSlot(n - 1);
		return xts ? static_cast<int>(xts->SpawnID) : 0;
	}

	// Bare spawn id, then finally a name - handy for scripts and for testing a
	// binding without having to set anything up first.
	if (const int id = GetIntFromString(sel, 0))
	{
		if (GetSpawnByID(id))
			return id;
	}

	PlayerClient* byName = GetSpawnByName(sel);
	return byName ? byName->SpawnID : 0;
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

	// Everything after the selector is the command, passed through untouched so
	// quoting and arguments survive.
	const char* rest = GetNextArg(szLine, 1);
	while (*rest == ' ' || *rest == '\t')
		++rest;

	if (*rest == 0)
	{
		WriteChatf("\ag[MQEQGFX]\ax \arno command given\ax - /attarget %s <command>", szUnit);
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

	DoCommand(rest, false);          // false: must run before we restore

	pTarget = prev;
}

// ---------------------------------------------------------------------------
// Commands

bool UnitsCommand(const char* szCmd, const char* szLine)
{
	char a1[MAX_STRING] = { 0 }, a2[MAX_STRING] = { 0 };
	GetArg(a1, szLine, 2);
	GetArg(a2, szLine, 3);

	// /eqgfx focus [clear|target|<name>]
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
			const bool on = !s_showFocus.load();
			s_showFocus.store(on);
			WriteChatf("\ag[MQEQGFX]\ax focus window %s", on ? "\agON\ax" : "\ayOFF\ax");
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

	// hovercast/focuscast used to live here. They are gone: /attarget covers
	// both and every other target-consuming command besides, and it puts the
	// binding where it belongs - something the player creates on purpose.
	if (ci_equals(szCmd, "hovercast") || ci_equals(szCmd, "focuscast"))
	{
		WriteChatf("\ag[MQEQGFX]\ax that is now \ay/attarget\ax:");
		WriteChatf("\ag[MQEQGFX]\ax   /attarget MouseHover /cast <gem>");
		WriteChatf("\ag[MQEQGFX]\ax   /attarget 2Tar /cast <gem>");
		return true;
	}

	// /eqgfx frames [on|off|clickgem <n>|raid|worldhover|fallback|restore]
	if (ci_equals(szCmd, "frames"))
	{
		if (a1[0] == 0 || ci_equals(a1, "toggle"))
		{
			const bool on = !s_showFrames.load();
			s_showFrames.store(on);
			WriteChatf("\ag[MQEQGFX]\ax unit frames %s", on ? "\agON\ax" : "\ayOFF\ax");
		}
		else if (ci_equals(a1, "on"))  { s_showFrames.store(true);  WriteChatf("\ag[MQEQGFX]\ax unit frames ON"); }
		else if (ci_equals(a1, "off")) { s_showFrames.store(false); WriteChatf("\ag[MQEQGFX]\ax unit frames OFF"); }
		else if (ci_equals(a1, "clickgem"))
		{
			s_clickGem = std::clamp(GetIntFromString(a2, 0), 0, NUM_SPELL_GEMS);
			WriteChatf("\ag[MQEQGFX]\ax frame click casts gem \ay%d\ax%s",
				s_clickGem, s_clickGem == 0 ? " (targets instead)" : "");
		}
		else if (ci_equals(a1, "raid"))
		{
			const bool on = !s_includeRaid.load();
			s_includeRaid.store(on);
			WriteChatf("\ag[MQEQGFX]\ax raid members in frames %s", on ? "\agON\ax" : "\ayOFF\ax");
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
		else if (ci_equals(a1, "probe"))
		{
			const bool on = !s_buffProbe.load();
			s_buffProbe.store(on);
			if (on)
				WriteChatf("\ag[MQEQGFX]\ax buff probe \agON\ax - asks the server for buffs on "
					"units you have never targeted. Sends a target notification for them and "
					"immediately one for your real target; your own target never changes, but "
					"the server briefly believes otherwise.");
			else
				WriteChatf("\ag[MQEQGFX]\ax buff probe \ayOFF\ax - cached buffs only, no packets");
		}
		else if (ci_equals(a1, "restore"))
		{
			s_restoreOnComplete = !s_restoreOnComplete;
			WriteChatf("\ag[MQEQGFX]\ax target restore: %s",
				s_restoreOnComplete ? "\ayon cast complete\ax (safer, target visibly changes)"
									: "\agimmediate\ax (no visible target change)");
		}
		else
		{
			WriteChatf("\ag[MQEQGFX]\ax frames [on|off|clickgem <n>|raid|worldhover|fallback|restore|probe]");
		}
		return true;
	}

	// /eqgfx xtarget [on|off]
	if (ci_equals(szCmd, "xtarget") || ci_equals(szCmd, "xt"))
	{
		if (ci_equals(a1, "on"))       s_showXTarget.store(true);
		else if (ci_equals(a1, "off")) s_showXTarget.store(false);
		else                           s_showXTarget.store(!s_showXTarget.load());
		WriteChatf("\ag[MQEQGFX]\ax extended target window %s",
			s_showXTarget.load() ? "\agON\ax" : "\ayOFF\ax");
		return true;
	}

	// /eqgfx units - status
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
		WriteChatf("\ag[MQEQGFX]\ax frames %s, focus window %s, world hover %s, marker %s, xtarget %s",
			s_showFrames.load() ? "\agon\ax" : "\ayoff\ax",
			s_showFocus.load() ? "\agon\ax" : "\ayoff\ax",
			s_worldHover.load() ? "\agon\ax" : "\ayoff\ax",
			s_focusMarker.load() ? "\agon\ax" : "\ayoff\ax",
			s_showXTarget.load() ? "\agon\ax" : "\ayoff\ax");
		WriteChatf("\ag[MQEQGFX]\ax click gem %d, fallback %s, restore %s, probe %s, %d cast bars live",
			s_clickGem,
			s_hoverFallback.load() ? "on" : "off",
			s_restoreOnComplete ? "on-complete" : "immediate",
			s_buffProbe.load() ? "\agon\ax" : "off",
			static_cast<int>(casts));
		return true;
	}

	return false;
}
