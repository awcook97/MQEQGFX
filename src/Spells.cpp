/*
 * Spells.cpp - the cast-start tap + CastingData tracker. Three-way gate:
 *   1. eqlib doesn't ship CEverQuest__StartCasting_x -> everything here
 *      compiles out and poll_casts() just stays empty.
 *   2. This MacroQuest build ships the tracker (MQ2Spawns.cpp detours
 *      StartCasting, populates CastingData and exports AddStartCastingCallback)
 *      -> subscribe to its tap and change nothing else.
 *   3. Vanilla MacroQuest -> the plugin owns the tracker: detour StartCasting
 *      (recv 0xCCF6) to feed OnCastStart and populate pSpawn->CastingData for
 *      remote spawns (the native client only writes it for the local PC),
 *      sweep expired casts each pulse, and provide ${Spawn[...].CastTimeLeft}
 *      via a datatype extension. Mirrors src/main/MQ2Spawns.cpp +
 *      MQ2SpawnType.cpp on the patched build.
 *
 * StartCast payload:
 *   +0x00 int32  SpellID
 *   +0x04 int16  SpawnID
 *   +0x06 uint32 CastTimeMs  (unaligned)
 */

#include "MQEQGFX.h"

std::mutex             s_castMutex;
std::vector<CastEvent> s_castEvents;

namespace {

void OnCastStart(int spellId, int spawnId, uint32_t castTimeMs)
{
	CDisplay* display = pDisplay.get();
	uint32_t timestamp = display ? display->TimeStamp : 0;

	std::scoped_lock lock(s_castMutex);
	if (s_castEvents.size() >= kMaxCastEvents)
		s_castEvents.erase(s_castEvents.begin());
	s_castEvents.push_back({ spellId, spawnId, castTimeMs, timestamp });
}

} // namespace

#ifdef CEverQuest__StartCasting_x

bool s_castTapInstalled  = false;  // one of the two modes is active
bool s_castTapFailed     = false;  // detour conflict - don't retry every pulse
bool s_castTrackerOwned  = false;  // mode 3: detour + sweep + extension are ours

namespace {

// MQ2Main's tap exports, resolved dynamically: a static import would keep this
// DLL from loading at all on a MacroQuest build that lacks them.
using fEqgfxCastCallback          = void (*)(int spellId, int spawnId, uint32_t castTimeMs);
using fAddStartCastingCallback    = int  (*)(fEqgfxCastCallback);
using fRemoveStartCastingCallback = void (*)(int);
fRemoveStartCastingCallback s_fnRemoveCastCallback = nullptr;

int       s_castCallbackId    = -1;     // mode 2: MQ2Main tap subscription
uintptr_t s_castDetourAddr    = 0;      // exact address we patched, so remove == install

DETOUR_TRAMPOLINE_DEF(void, StartCasting_Trampoline, (void*, void*))
void StartCasting_Detour(void* pThis, void* param2)
{
	if (param2)
	{
		int spellId = *static_cast<const int*>(param2);
		int spawnId = static_cast<short>(static_cast<const int*>(param2)[1]);
		uint32_t castTimeMs = *reinterpret_cast<const uint32_t*>(static_cast<const char*>(param2) + 6);

		// Tap consumers see the raw packet before any filtering below.
		OnCastStart(spellId, spawnId, castTimeMs);

		// Mirrors the native local-PC gates: only write on a real durational
		// cast from a fresh idle slot. Skip instants (castTimeMs <= 100) and
		// don't stomp an in-progress cast; expiry is the sweep's job.
		if (pDisplay && spellId > 0 && spawnId > 0 && castTimeMs > 100)
		{
			PlayerClient* pSpawn = GetSpawnByID(spawnId);
			if (pSpawn && pSpawn != pLocalPlayer && pSpawn->CastingData.SpellID == -1)
			{
				pSpawn->CastingData.SpellID  = spellId;
				pSpawn->CastingData.SpellETA = static_cast<uint32_t>(pDisplay->TimeStamp + castTimeMs);
			}
		}
	}

	StartCasting_Trampoline(pThis, param2);
}

// Clears CastingData once SpellETA elapses - emu servers don't always send an
// end-cast notification.
void SweepExpiredCastingData()
{
	if (!pDisplay || !pSpawnList)
		return;

	uint32_t now = pDisplay->TimeStamp;
	for (PlayerClient* pSpawn = pSpawnList; pSpawn; pSpawn = pSpawn->pNext)
	{
		if (pSpawn == pLocalPlayer)
			continue;

		auto& cd = pSpawn->CastingData;
		if (cd.SpellID != -1 && cd.SpellETA != 0 && now >= cd.SpellETA)
		{
			cd.SpellID  = -1;
			cd.SpellETA = 0;
		}
	}
}

// ${Spawn[...].CastTimeLeft} for MacroQuest builds whose spawn type lacks the
// member. Registered on "spawn" AND "target": target's GetMember forwards to
// the spawn type with a direct C++ call, which bypasses extension lookup.
class SpawnCastExtType : public MQ2Type
{
public:
	enum class Members { CastTimeLeft = 1 };

	SpawnCastExtType() : MQ2Type("EQGFXSpawnCast")
	{
		ScopedTypeMember(Members, CastTimeLeft);
	}

	bool GetMember(MQVarPtr VarPtr, const char* Member, char* Index, MQTypeVar& Dest) override
	{
		MQTypeMember* pMember = FindMember(Member);
		if (!pMember)
			return false;

		PlayerClient* pSpawn = mq::datatypes::MQ2SpawnType::GetSpawnPtr(VarPtr);
		if (!pSpawn || !pDisplay)
			return false;

		switch (static_cast<Members>(pMember->ID))
		{
		case Members::CastTimeLeft:
			Dest.Int64 = 0;
			Dest.Type = mq::datatypes::pTimeStampType;
			if (pSpawn->CastingData.SpellETA)
			{
				int64_t delta = pSpawn->CastingData.SpellETA - pDisplay->TimeStamp;
				if (delta > 0)
					Dest.Int64 = delta;
			}
			return true;
		}
		return false;
	}
};

SpawnCastExtType* s_castExtType = nullptr;
constexpr const char* kCastExtTypeNames[] = { "spawn", "target" };

} // namespace

void InstallCastTap()
{
	if (s_castTapInstalled || s_castTapFailed)
		return;

	// Mode 2: this MacroQuest build ships the tracker - just subscribe.
	if (HMODULE hMain = GetModuleHandleA("MQ2Main.dll"))
	{
		auto add = reinterpret_cast<fAddStartCastingCallback>(
			GetProcAddress(hMain, "AddStartCastingCallback"));
		auto remove = reinterpret_cast<fRemoveStartCastingCallback>(
			GetProcAddress(hMain, "RemoveStartCastingCallback"));
		if (add && remove)
		{
			s_fnRemoveCastCallback = remove;
			s_castCallbackId = add(OnCastStart);
			s_castTapInstalled = true;
			return;
		}
	}

	// Mode 3: vanilla build - own the tracker. The runtime offset is 0 until
	// eqlib has patched the game; retry from OnPulse in that case.
	if (!CEverQuest__StartCasting)
		return;

	if (!EzDetour(CEverQuest__StartCasting, StartCasting_Detour, StartCasting_Trampoline))
	{
		// Address already detoured (another plugin owns it) - leave it be.
		s_castTapFailed = true;
		return;
	}
	s_castDetourAddr = CEverQuest__StartCasting;

	MQ2Type* spawnType = FindMQ2DataType("spawn");
	if (spawnType && !spawnType->FindMember("CastTimeLeft"))
	{
		s_castExtType = new SpawnCastExtType();
		for (const char* name : kCastExtTypeNames)
			AddMQ2TypeExtension(name, s_castExtType);
	}

	s_castTrackerOwned = true;
	s_castTapInstalled = true;
}

void RemoveCastTap()
{
	if (s_castCallbackId != -1 && s_fnRemoveCastCallback)
	{
		s_fnRemoveCastCallback(s_castCallbackId);
		s_castCallbackId = -1;
	}

	if (s_castDetourAddr)
	{
		RemoveDetour(s_castDetourAddr);
		s_castDetourAddr = 0;
	}

	if (s_castExtType)
	{
		for (const char* name : kCastExtTypeNames)
			RemoveMQ2TypeExtension(name, s_castExtType);
		delete s_castExtType;
		s_castExtType = nullptr;
	}

	s_castTrackerOwned = false;
	s_castTapInstalled = false;
}

// Per-pulse upkeep: late install (the offset resolves after eqlib patches the
// game) and the expiry sweep when we own the tracker.
void PulseCastTap()
{
	if (!s_castTapInstalled)
		InstallCastTap();
	if (s_castTrackerOwned)
		SweepExpiredCastingData();
}

#else // CEverQuest__StartCasting_x

void InstallCastTap() {}
void RemoveCastTap() {}
void PulseCastTap() {}

#endif // CEverQuest__StartCasting_x
