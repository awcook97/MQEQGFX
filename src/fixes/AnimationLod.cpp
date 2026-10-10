#include "../MQEQGFX.h"

#include "../PatternScan.h"
#include "../Signatures.h"

#include "eqlib/graphics/Actors.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>

namespace {

AnimationLodOptions s_opts;
uint32_t s_frame = 0;
bool     s_framePending = false;

CVector3 s_eye;
bool     s_haveEye = false;

constexpr int kMaxFullRate = MAX_GROUP_SIZE + 3;
const CActorInterface* s_fullRate[kMaxFullRate] = {};
int s_fullRateCount = 0;

uintptr_t s_updateAddr = 0;
bool      s_scanned = false;

void ResetView()
{
	s_fullRateCount = 0;
	s_haveEye = false;
}

bool FullRate(const CActor* actor)
{
	for (int i = 0; i < s_fullRateCount; ++i)
		if (s_fullRate[i] == actor)
			return true;
	return false;
}

int IntervalFor(CHierarchicalActor* actor)
{
	if (!s_haveEye || s_opts.distance <= 0.0f || FullRate(actor))
		return 1;
	const CVector3& p = actor->GetPosition();
	const float dx = p.X - s_eye.X, dy = p.Y - s_eye.Y, dz = p.Z - s_eye.Z;
	const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
	if (dist <= s_opts.distance)
		return 1;
	return std::clamp(static_cast<int>(dist / s_opts.distance) + 1, 1, std::max(1, s_opts.maxInterval));
}

class ActorUpdateHook
{
public:
	DETOUR_TRAMPOLINE_DEF_MEMBER(ActorUpdateHook, void, Update_Trampoline, ())
	void Update_Detour()
	{
		CHierarchicalActor* actor = reinterpret_cast<CHierarchicalActor*>(this);
		if (actor->bAnimatedThisFrame)
			return Update_Trampoline();

		if (!s_opts.enabled)
		{
			NoteActorAnimation(false, false);
			return Update_Trampoline();
		}

		if (GetGameState() != GAMESTATE_INGAME)
		{
			ResetView();
			return Update_Trampoline();
		}

		if (s_framePending)
		{
			s_framePending = false;
			++s_frame;
		}

		const int every = IntervalFor(actor);
		if (every <= 1
			|| (s_frame + static_cast<uint32_t>(reinterpret_cast<uintptr_t>(actor) >> 4)) % every == 0)
		{
			NoteActorAnimation(every > 1, false);
			return Update_Trampoline();
		}

		NoteActorAnimation(true, true);
		CBoneGroupInterface* groups[3];
		std::copy(std::begin(actor->pBoneGroups), std::end(actor->pBoneGroups), groups);
		std::fill(std::begin(actor->pBoneGroups), std::end(actor->pBoneGroups), nullptr);
		Update_Trampoline();
		std::copy(std::begin(groups), std::end(groups), actor->pBoneGroups);
	}
};

void AddFullRate(PlayerClient* spawn)
{
	if (spawn && s_fullRateCount < kMaxFullRate)
		if (CActorInterface* actor = spawn->GetActor())
			s_fullRate[s_fullRateCount++] = actor;
}

void Status()
{
	WriteChatf("\ag[MQEQGFX]\ax animation lod %s: distance %.0f, up to every %d frames; %s",
		s_opts.enabled ? "\agon\ax" : "\aroff\ax", s_opts.distance, s_opts.maxInterval,
		s_updateAddr ? "ready" : "\arunavailable\ax");
	const ActorAnimationStats st = GetActorAnimationStats();
	WriteChatf("\ag[MQEQGFX]\ax last frame: %u animated, %u far, %u skipped",
		st.animated, st.distant, st.skipped);
}

} // namespace

void InstallAnimationLod()
{
	if (s_updateAddr || s_scanned || !eqlib::EQGraphicsBaseAddress)
		return;
	s_scanned = true;
	s_updateAddr = eqgfx::scan::Find(eqgfx::scan::Graphics(), eqgfx::sig::CActor__Update);
	if (s_updateAddr && !EzDetour(s_updateAddr, &ActorUpdateHook::Update_Detour, &ActorUpdateHook::Update_Trampoline))
		s_updateAddr = 0;
}

void RemoveAnimationLod()
{
	if (!s_updateAddr)
		return;
	RemoveDetour(s_updateAddr);
	s_updateAddr = 0;
}

void PulseAnimationLod()
{
	s_framePending = true;

	ResetView();
	PlayerClient* me = pLocalPlayer.get();
	if (!me)
		return;
	if (CActorInterface* actor = me->GetActor())
	{
		s_eye = actor->GetPosition();
		s_haveEye = true;
	}
	AddFullRate(me);
	AddFullRate(pTarget.get());
	if (me->PetID > 0)
		AddFullRate(GetSpawnByID(me->PetID));
	if (pLocalPC && pLocalPC->Group)
		for (int g = 1; g < MAX_GROUP_SIZE; ++g)
			if (CGroupMember* gm = pLocalPC->Group->GetGroupMember(g))
				AddFullRate(gm->GetPlayer());
}

AnimationLodOptions GetAnimationLodOptions() { return s_opts; }

void SetAnimationLodOptions(const AnimationLodOptions& o)
{
	s_opts = o;
	if (!std::isfinite(s_opts.distance))
		s_opts.distance = AnimationLodOptions{}.distance;
	s_opts.distance = std::clamp(s_opts.distance, 10.0f, 2000.0f);
	s_opts.maxInterval = std::clamp(s_opts.maxInterval, 1, 30);
}

bool AnimationLodInstalled() { return s_updateAddr != 0; }

bool AnimationLodCommand(const char* szLine)
{
	char verb[MAX_STRING] = { 0 };
	char value[MAX_STRING] = { 0 };
	GetArg(verb, szLine, 2);
	GetArg(value, szLine, 3);

	AnimationLodOptions o = s_opts;
	if (ci_equals(verb, "on"))
		o.enabled = true;
	else if (ci_equals(verb, "off"))
		o.enabled = false;
	else if (ci_equals(verb, "distance") && value[0])
		o.distance = GetFloatFromString(value, s_opts.distance);
	else if (ci_equals(verb, "interval") && value[0])
		o.maxInterval = std::atoi(value);
	else if (verb[0])
	{
		WriteChatf("\ag[MQEQGFX]\ax usage: /eqgfx animlod [on|off|distance <n>|interval <n>]");
		return true;
	}
	SetAnimationLodOptions(o);
	Status();
	return true;
}
