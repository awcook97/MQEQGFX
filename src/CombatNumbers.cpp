/*
 * CombatNumbers.cpp - the damage/heal tap feeding poll_damage().
 *
 * Detours CEverQuest::ReportSuccessfulHit/Heal (every hit, miss, and heal the
 * client reports) and makes the events available for Lua.
 */

#include "MQEQGFX.h"

#include <cstdio>
#include <cstring>

std::mutex               s_dmgMutex;
std::vector<DamageEvent> s_dmgEvents;
std::atomic<bool>        s_dmgDebug{ false };
bool                     s_dmgTapInstalled = false;

namespace {

void PushDamageEvent(const DamageEvent& ev)
{
	std::scoped_lock lock(s_dmgMutex);
	if (s_dmgEvents.size() >= kMaxDamageEvents)
		s_dmgEvents.erase(s_dmgEvents.begin());
	s_dmgEvents.push_back(ev);
}

uint32_t NowMs()
{
	CDisplay* display = pDisplay.get();
	return display ? display->TimeStamp : 0;
}

void RecordHitEvent(const EQSuccessfulHit& hit)
{
	if (s_dmgDebug.load(std::memory_order_relaxed))
	{
		char tags[48] = "";
		if (hit.Flags & HitFlag_Critical) std::strcat(tags, " crit");
		if (hit.Flags & HitFlag_Lucky)    std::strcat(tags, " lucky");
		if (hit.Flags & HitFlag_Twincast) std::strcat(tags, " twincast");
		if (hit.Flags & HitFlag_Locked)   std::strcat(tags, " locked");
		const char* skillName = GetSkillName(hit.Skill);   // null for the 0xe7 spell marker
		WriteChatf("\ag[MQEQGFX]\ax \atHIT\ax atk=%d tgt=%d dmg=%lld spell=%d skill=%d(%s) flags=0x%X%s",
			hit.AttackerID, hit.DamagedID, static_cast<long long>(hit.DamageCaused),
			hit.SpellID, hit.Skill, skillName ? skillName : "spell/other", hit.Flags, tags);
	}

	PushDamageEvent({ hit.AttackerID, hit.DamagedID, static_cast<int>(hit.DamageCaused),
		hit.SpellID, hit.Skill, false, 0, hit.Flags, NowMs() });
}

void RecordHealEvent(const EQSuccessfulHeal& heal)
{
	if (s_dmgDebug.load(std::memory_order_relaxed))
	{
		char tags[48] = "";
		if (heal.EffectBitmask & HealFlag_Critical) std::strcat(tags, " crit");
		if (heal.EffectBitmask & HealFlag_Lucky)    std::strcat(tags, " lucky");
		if (heal.EffectBitmask & HealFlag_Twincast) std::strcat(tags, " twincast");
		if (heal.EffectBitmask & HealFlag_OverTime) std::strcat(tags, " hot");
		WriteChatf("\ag[MQEQGFX]\ax \atHEAL\ax healed=%d healer=%d actual=%lld total=%lld spell=%d flags=0x%X%s",
			heal.HealedID, heal.HealerID, static_cast<long long>(heal.ActualHeal),
			static_cast<long long>(heal.TotalHeal), heal.SpellID, heal.EffectBitmask, tags);
	}

	PushDamageEvent({ heal.HealerID, heal.HealedID, static_cast<int>(heal.ActualHeal),
		heal.SpellID, 0, true, static_cast<int>(heal.TotalHeal), heal.EffectBitmask, NowMs() });
}

// Dump a report struct's raw bytes. Only when /eqgfx dmgdebug is on.
void DumpRawReport(const char* tag, const uint8_t* p, size_t n)
{
	if (!s_dmgDebug.load(std::memory_order_relaxed) || !p) return;
	char hex[3 * 0x30 + 1] = { 0 };
	size_t o = 0;
	for (size_t i = 0; i < n && o + 3 < sizeof(hex); ++i)
		o += static_cast<size_t>(std::snprintf(hex + o, sizeof(hex) - o, "%02X ", p[i]));
	WriteChatf("\ag[MQEQGFX]\ax \ay%s raw\ax: %s", tag, hex);
}

} // namespace

#if defined(CEverQuest__ReportSuccessfulHit_x) && defined(CEverQuest__ReportSuccessfulHeal_x)

namespace {

// Record and pass through. DamageCaused <= 0 is a miss/defense code.
class CEverQuestDmgHook
{
public:
	DETOUR_TRAMPOLINE_DEF(bool, ReportHit_Trampoline, (EQSuccessfulHit*, bool, int))
	bool ReportHit_Detour(EQSuccessfulHit* hit, bool showText, int p3)
	{
		if (hit)
		{
			DumpRawReport("HIT", reinterpret_cast<const uint8_t*>(hit), sizeof(EQSuccessfulHit));
			RecordHitEvent(*hit);
		}
		return ReportHit_Trampoline(hit, showText, p3);
	}

	DETOUR_TRAMPOLINE_DEF(int, ReportHeal_Trampoline, (EQSuccessfulHeal*))
	int ReportHeal_Detour(EQSuccessfulHeal* heal)
	{
		if (heal)
		{
			DumpRawReport("HEAL", reinterpret_cast<const uint8_t*>(heal), sizeof(EQSuccessfulHeal));
			RecordHealEvent(*heal);
		}
		return ReportHeal_Trampoline(heal);
	}
};

uintptr_t s_dmgHitAddr = 0;
uintptr_t s_dmgHealAddr = 0;

} // namespace

void InstallDamageTap()
{
	if (s_dmgTapInstalled) return;
	if (!CEverQuest__ReportSuccessfulHit || !CEverQuest__ReportSuccessfulHeal) return;
	s_dmgHitAddr = CEverQuest__ReportSuccessfulHit;
	s_dmgHealAddr = CEverQuest__ReportSuccessfulHeal;
	EzDetour(s_dmgHitAddr,
		&CEverQuestDmgHook::ReportHit_Detour, &CEverQuestDmgHook::ReportHit_Trampoline);
	EzDetour(s_dmgHealAddr,
		&CEverQuestDmgHook::ReportHeal_Detour, &CEverQuestDmgHook::ReportHeal_Trampoline);
	s_dmgTapInstalled = true;
}

void RemoveDamageTap()
{
	if (!s_dmgTapInstalled) return;
	if (s_dmgHitAddr)  { RemoveDetour(s_dmgHitAddr);  s_dmgHitAddr = 0; }
	if (s_dmgHealAddr) { RemoveDetour(s_dmgHealAddr); s_dmgHealAddr = 0; }
	s_dmgTapInstalled = false;
}
#else
void InstallDamageTap() {}
void RemoveDamageTap() {}
#endif
