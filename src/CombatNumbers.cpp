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
		if (hit.Flags & HitFlag_Critical) strcat_s(tags, " crit");
		if (hit.Flags & HitFlag_Lucky)    strcat_s(tags, " lucky");
		if (hit.Flags & HitFlag_Twincast) strcat_s(tags, " twincast");
		if (hit.Flags & HitFlag_Locked)   strcat_s(tags, " locked");
		const char* skillName = GetSkillName(hit.Skill);
		WriteChatf("\ag[MQEQGFX]\ax \atHIT\ax atk=%d tgt=%d dmg=%lld spell=%d skill=%d(%s) flags=0x%X%s",
			hit.AttackerID, hit.DamagedID, static_cast<long long>(hit.DamageCaused),
			hit.SpellID, hit.Skill, skillName ? skillName : "spell/other", hit.Flags, tags);
	}

	PushDamageEvent({ hit.AttackerID, hit.DamagedID, static_cast<int>(hit.DamageCaused),
		hit.SpellID, hit.Skill, false, 0, hit.Flags, NowMs() });
	if (hit.SpellID > 0 && hit.DamageCaused > 0)
		SpellFxNoteLanded(hit.AttackerID, hit.DamagedID, hit.SpellID, false);
	if (hit.SpellID <= 0)
		AttackFxNoteHit(hit.AttackerID, hit.DamagedID, static_cast<int>(hit.DamageCaused), hit.Flags);
}

void RecordHealEvent(const EQSuccessfulHeal& heal)
{
	if (s_dmgDebug.load(std::memory_order_relaxed))
	{
		char tags[48] = "";
		if (heal.EffectBitmask & HealFlag_Critical) strcat_s(tags, " crit");
		if (heal.EffectBitmask & HealFlag_Lucky)    strcat_s(tags, " lucky");
		if (heal.EffectBitmask & HealFlag_Twincast) strcat_s(tags, " twincast");
		if (heal.EffectBitmask & HealFlag_OverTime) strcat_s(tags, " hot");
		WriteChatf("\ag[MQEQGFX]\ax \atHEAL\ax healed=%d healer=%d actual=%lld total=%lld spell=%d flags=0x%X%s",
			heal.HealedID, heal.HealerID, static_cast<long long>(heal.ActualHeal),
			static_cast<long long>(heal.TotalHeal), heal.SpellID, heal.EffectBitmask, tags);
	}

	PushDamageEvent({ heal.HealerID, heal.HealedID, static_cast<int>(heal.ActualHeal),
		heal.SpellID, 0, true, static_cast<int>(heal.TotalHeal), heal.EffectBitmask, NowMs() });
	if (heal.SpellID > 0)
		SpellFxNoteLanded(heal.HealerID, heal.HealedID, heal.SpellID, true);
}

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

class CEverQuestDmgHook
{
public:
	DETOUR_TRAMPOLINE_DEF_MEMBER(CEverQuestDmgHook, bool, ReportHit_Trampoline, (EQSuccessfulHit*, bool, int))
	bool ReportHit_Detour(EQSuccessfulHit* hit, bool showText, int p3)
	{
		if (hit)
		{
			DumpRawReport("HIT", reinterpret_cast<const uint8_t*>(hit), sizeof(EQSuccessfulHit));
			RecordHitEvent(*hit);
		}
		return ReportHit_Trampoline(hit, showText, p3);
	}

	DETOUR_TRAMPOLINE_DEF_MEMBER(CEverQuestDmgHook, int, ReportHeal_Trampoline, (EQSuccessfulHeal*))
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
bool      s_dmgTapFailed = false;

} // namespace

void InstallDamageTap()
{
	if (s_dmgTapInstalled || s_dmgTapFailed) return;
	if (!CEverQuest__ReportSuccessfulHit || !CEverQuest__ReportSuccessfulHeal) return;
	s_dmgHitAddr = CEverQuest__ReportSuccessfulHit;
	if (!EzDetour(s_dmgHitAddr,
		&CEverQuestDmgHook::ReportHit_Detour, &CEverQuestDmgHook::ReportHit_Trampoline))
	{
		s_dmgHitAddr = 0;
		s_dmgTapFailed = true;
		return;
	}
	s_dmgHealAddr = CEverQuest__ReportSuccessfulHeal;
	if (!EzDetour(s_dmgHealAddr,
		&CEverQuestDmgHook::ReportHeal_Detour, &CEverQuestDmgHook::ReportHeal_Trampoline))
	{
		s_dmgHealAddr = 0;
		RemoveDetour(s_dmgHitAddr);
		s_dmgHitAddr = 0;
		s_dmgTapFailed = true;
		return;
	}
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
