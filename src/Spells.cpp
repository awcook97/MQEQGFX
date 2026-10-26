#include "MQEQGFX.h"

std::mutex             s_castMutex;
std::vector<CastEvent> s_castEvents;

namespace {

void OnCastStart(int spellId, int spawnId, uint32_t castTimeMs)
{
	spawnId = static_cast<uint16_t>(spawnId);
	CDisplay* display = pDisplay.get();
	uint32_t timestamp = display ? display->TimeStamp : 0;

	UnitsNoteCastStart(spellId, spawnId, castTimeMs);
	SpellFxNoteCastStart(spellId, spawnId, castTimeMs);

	std::scoped_lock lock(s_castMutex);
	if (s_castEvents.size() >= kMaxCastEvents)
		s_castEvents.erase(s_castEvents.begin());
	s_castEvents.push_back({ spellId, spawnId, castTimeMs, timestamp });
}

} // namespace

#ifdef CEverQuest__StartCasting_x

bool s_castTapInstalled  = false;
bool s_castTapFailed     = false;
bool s_castTrackerOwned  = false;

namespace {

using fEqgfxCastCallback          = void (*)(int spellId, int spawnId, uint32_t castTimeMs);
using fAddStartCastingCallback    = int  (*)(fEqgfxCastCallback);
using fRemoveStartCastingCallback = void (*)(int);
fRemoveStartCastingCallback s_fnRemoveCastCallback = nullptr;

int       s_castCallbackId    = -1;
uintptr_t s_castDetourAddr    = 0;

DETOUR_TRAMPOLINE_DEF(void, StartCasting_Trampoline, (void*, void*))
void StartCasting_Detour(void* pThis, void* param2)
{
	if (param2)
	{
		int spellId = *static_cast<const int*>(param2);
		int spawnId = static_cast<uint16_t>(static_cast<const int*>(param2)[1]);
		uint32_t castTimeMs = *reinterpret_cast<const uint32_t*>(static_cast<const char*>(param2) + 6);

		OnCastStart(spellId, spawnId, castTimeMs);

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

void SweepExpiredCastingData(bool all = false)
{
	if (!pDisplay || !pSpawnList)
		return;

	uint32_t now = pDisplay->TimeStamp;
	for (PlayerClient* pSpawn = pSpawnList; pSpawn; pSpawn = pSpawn->pNext)
	{
		if (pSpawn == pLocalPlayer)
			continue;

		auto& cd = pSpawn->CastingData;
		if (cd.SpellID != -1 && cd.SpellETA != 0 && (all || now >= cd.SpellETA))
		{
			cd.SpellID  = -1;
			cd.SpellETA = 0;
		}
	}
}

class SpawnCastExtType : public MQ2Type
{
public:
	enum class Members { CastTimeLeft = 1 };

	SpawnCastExtType() : MQ2Type("EQGFXSpawnCast")
	{
		ScopedTypeMember(Members, CastTimeLeft);
	}

	bool GetMember(MQVarPtr VarPtr, const char* Member, char*, MQTypeVar& Dest) override
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
				int64_t delta = static_cast<int64_t>(pSpawn->CastingData.SpellETA) - static_cast<int64_t>(pDisplay->TimeStamp);
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

	if (!CEverQuest__StartCasting)
		return;

	if (!EzDetour(CEverQuest__StartCasting, StartCasting_Detour, StartCasting_Trampoline))
	{
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

	if (s_castTrackerOwned && GetGameState() == GAMESTATE_INGAME)
		SweepExpiredCastingData(true);

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

void PulseCastTap()
{
	if (!s_castTapInstalled)
		InstallCastTap();
	if (s_castTrackerOwned)
		SweepExpiredCastingData();
}

#else

void InstallCastTap() {}
void RemoveCastTap() {}
void PulseCastTap() {}

#endif
