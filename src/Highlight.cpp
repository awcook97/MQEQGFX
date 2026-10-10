#include <mq/Plugin.h>

#include "MQEQGFX.h"

#include "eqlib/graphics/Actors.h"

#include <cstdint>
#include <map>
#include <mutex>
#include <vector>

namespace {
constexpr size_t kVtSetMaterialTintAll = 0x230;

using FnSetMaterialTintAll = void(__fastcall*)(void*, const eqlib::RGB*, bool);

template <typename Fn>
Fn ActorVCall(void* actor, size_t byteOffset)
{
	uintptr_t vtable = *reinterpret_cast<uintptr_t*>(actor);
	return *reinterpret_cast<Fn*>(vtable + byteOffset);
}

constexpr uint32_t kTintIdentity    = 0xFFFFFFFF;
constexpr uint32_t kAmbientIdentity = 0xFF000000;

struct HighlightEntry
{
	uint32_t color = 0xFFFFFFFF;
	uint32_t mode = Highlight_Tint;
	CActorInterface* applied = nullptr;
	uint32_t appliedMode = 0;
	bool     dirty = false;
	const void* owner = nullptr;
};

std::mutex                        s_hlMutex;
std::map<int, HighlightEntry>     s_highlights;

void ApplyToActor(CActorInterface* actor, uint32_t color, uint32_t mode)
{
	if (mode & Highlight_Tint)
	{
		const eqlib::RGB tint(color);
		ActorVCall<FnSetMaterialTintAll>(actor, kVtSetMaterialTintAll)(actor, &tint, true);
	}
	if (mode & Highlight_Ambient)
	{
		const eqlib::RGB ambient(color);
		actor->SetConstantAmbient(&ambient);
	}
}

void RestoreActor(CActorInterface* actor, uint32_t appliedMode)
{
	if (appliedMode & Highlight_Tint)
	{
		const eqlib::RGB tint(kTintIdentity);
		ActorVCall<FnSetMaterialTintAll>(actor, kVtSetMaterialTintAll)(actor, &tint, true);
	}
	if (appliedMode & Highlight_Ambient)
	{
		const eqlib::RGB ambient(kAmbientIdentity);
		actor->SetConstantAmbient(&ambient);
	}
}

CActorInterface* ActorForSpawn(int spawnId)
{
	PlayerClient* spawn = GetSpawnByID(spawnId);
	return spawn ? spawn->GetActor() : nullptr;
}

} // namespace

void SetSpawnHighlight(int spawnId, uint32_t color, uint32_t mode, const void* owner)
{
	if (spawnId <= 0)
		return;
	if ((mode & (Highlight_Tint | Highlight_Ambient)) == 0)
		mode = Highlight_Tint;

	std::scoped_lock lock(s_hlMutex);
	if (s_highlights.size() >= kMaxHighlights && s_highlights.count(spawnId) == 0)
		return;

	HighlightEntry& entry = s_highlights[spawnId];
	entry.owner = owner;
	if (entry.color != color || entry.mode != mode)
	{
		entry.color = color;
		entry.mode = mode;
		entry.dirty = true;
	}
}

void ClearSpawnHighlight(int spawnId)
{
	std::scoped_lock lock(s_hlMutex);
	auto it = s_highlights.find(spawnId);
	if (it == s_highlights.end())
		return;

	if (it->second.applied && ActorForSpawn(spawnId) == it->second.applied)
		RestoreActor(it->second.applied, it->second.appliedMode);

	s_highlights.erase(it);
}

void ClearAllHighlights()
{
	std::vector<int> ids;
	{
		std::scoped_lock lock(s_hlMutex);
		ids.reserve(s_highlights.size());
		for (const auto& [id, entry] : s_highlights)
			ids.push_back(id);
	}
	for (int id : ids)
		ClearSpawnHighlight(id);
}

void ClearHighlightsOwnedBy(const void* owner)
{
	std::vector<int> ids;
	{
		std::scoped_lock lock(s_hlMutex);
		for (const auto& [id, entry] : s_highlights)
			if (entry.owner == owner)
				ids.push_back(id);
	}
	for (int id : ids)
		ClearSpawnHighlight(id);
}

int HighlightCount()
{
	std::scoped_lock lock(s_hlMutex);
	return static_cast<int>(s_highlights.size());
}

std::vector<std::tuple<int, uint32_t, uint32_t, bool>> HighlightList()
{
	std::scoped_lock lock(s_hlMutex);
	std::vector<std::tuple<int, uint32_t, uint32_t, bool>> out;
	out.reserve(s_highlights.size());
	for (const auto& [id, entry] : s_highlights)
		out.emplace_back(id, entry.color, entry.mode, entry.applied != nullptr && !entry.dirty);
	return out;
}

void PulseHighlights()
{
	std::scoped_lock lock(s_hlMutex);
	if (s_highlights.empty())
		return;

	for (auto it = s_highlights.begin(); it != s_highlights.end(); )
	{
		HighlightEntry& entry = it->second;
		PlayerClient* spawn = GetSpawnByID(it->first);
		if (!spawn)
		{
			it = s_highlights.erase(it);
			continue;
		}

		CActorInterface* actor = spawn->GetActor();
		if (actor && (actor != entry.applied || entry.dirty))
		{
			if (actor == entry.applied)
				RestoreActor(actor, entry.appliedMode & ~entry.mode);
			ApplyToActor(actor, entry.color, entry.mode);
			entry.applied = actor;
			entry.appliedMode = entry.mode;
			entry.dirty = false;
		}
		++it;
	}
}

void RefreshHighlights()
{
	std::scoped_lock lock(s_hlMutex);
	for (auto& [id, entry] : s_highlights)
	{
		if (!entry.applied || entry.dirty)
			continue;
		if (ActorForSpawn(id) != entry.applied)
			continue;
		ApplyToActor(entry.applied, entry.color, entry.mode);
	}
}

void OnHighlightSpawnRemoved(int spawnId)
{
	std::scoped_lock lock(s_hlMutex);
	s_highlights.erase(spawnId);
}

void ForgetHighlights()
{
	std::scoped_lock lock(s_hlMutex);
	s_highlights.clear();
}
