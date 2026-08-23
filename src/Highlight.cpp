/*
 * Highlight.cpp - recolor a spawn's actual 3D model.
 *
 * The engine keeps per-actor material state that we can overwrite: a tint
 * multiplied into every material, and a constant ambient added on top of
 * scene lighting. Setting those makes the model itself glow/recolor in the
 * world, which is something no amount of ImGui overlay can do.
 *
 * WHY WE CALL THROUGH THE VTABLE BY HAND
 * --------------------------------------
 * eqlib declares CActorInterface's two SetMaterialTint overloads in the
 * opposite order from the shipped vtable:
 *
 *     eqlib slot 0x228  SetMaterialTint(const RGB*, bool)
 *     eqlib slot 0x230  SetMaterialTint(int, const RGB*)
 *
 * The DLL has them the other way round. Verified in EQGraphics (Aug 11 test
 * build): slot 0x228 -> sub_180062f50 forwards (index, tint) straight to the
 * material palette and null-checks its *third* argument as a pointer, while
 * slot 0x230 -> sub_180062fc0 null-checks its *second* argument and loops
 * every material index applying the same tint. So 0x230 is the all-materials
 * form we want. Calling by name would hand &tint to an int parameter.
 *
 * Rather than fight the header, this file resolves both entries by slot with
 * locally-declared signatures. If eqlib's declarations are ever corrected,
 * this code keeps working instead of silently swapping its arguments.
 *
 * Note SetHighlightMode (slot 0x478) looks like the obvious API for this and
 * is a no-op: it points at the engine's shared "return 0" vtable stub in
 * this build. Don't bother with it.
 *
 * THREADING
 * ---------
 * Everything here runs on the main thread. The game's own material calls
 * happen during main-thread spawn processing, and OnPulse is where the spawn
 * list is safe to walk, so PulseHighlights drives all apply/restore work.
 * Lua setters only record intent; they never touch an actor.
 */

#include <mq/Plugin.h>

#include "MQEQGFX.h"

#include "eqlib/graphics/Actors.h"

#include <cstdint>
#include <map>
#include <mutex>
#include <vector>

namespace {

// CActorInterface vtable byte offsets, as shipped (see the header comment).
constexpr size_t kVtSetConstantAmbient = 0x210;   // void(const RGB*)
constexpr size_t kVtSetMaterialTintAll = 0x230;   // void(const RGB*, bool)

using FnSetConstantAmbient = void(__fastcall*)(void*, const eqlib::RGB*);
using FnSetMaterialTintAll = void(__fastcall*)(void*, const eqlib::RGB*, bool);

template <typename Fn>
Fn ActorVCall(void* actor, size_t byteOffset)
{
	uintptr_t vtable = *reinterpret_cast<uintptr_t*>(actor);
	return *reinterpret_cast<Fn*>(vtable + byteOffset);
}

// Identity values: a white tint multiplies to no change, and zero constant
// ambient hands lighting back to the scene. There is no getter for either on
// CActorInterface, so restore means writing these rather than replaying a
// saved value - a highlighted actor that the game had already tinted (dyed
// armor) comes back plain. Acceptable for NPCs; noted in the docs for PCs.
constexpr uint32_t kTintIdentity    = 0xFFFFFFFF;
constexpr uint32_t kAmbientIdentity = 0xFF000000;

struct HighlightEntry
{
	uint32_t color = 0xFFFFFFFF;
	uint32_t mode = Highlight_Tint;
	void*    applied = nullptr;    // actor the current look is written to
	uint32_t appliedMode = 0;      // what we actually wrote, for restore
};

std::mutex                        s_hlMutex;
std::map<int, HighlightEntry>     s_highlights;

void ApplyToActor(void* actor, uint32_t color, uint32_t mode)
{
	if (mode & Highlight_Tint)
	{
		const eqlib::RGB tint(color);
		ActorVCall<FnSetMaterialTintAll>(actor, kVtSetMaterialTintAll)(actor, &tint, true);
	}
	if (mode & Highlight_Ambient)
	{
		const eqlib::RGB ambient(color);
		ActorVCall<FnSetConstantAmbient>(actor, kVtSetConstantAmbient)(actor, &ambient);
	}
}

// Undo only what we wrote. Writing the identity for a mode we never set would
// clobber a tint the game itself applied.
void RestoreActor(void* actor, uint32_t appliedMode)
{
	if (appliedMode & Highlight_Tint)
	{
		const eqlib::RGB tint(kTintIdentity);
		ActorVCall<FnSetMaterialTintAll>(actor, kVtSetMaterialTintAll)(actor, &tint, true);
	}
	if (appliedMode & Highlight_Ambient)
	{
		const eqlib::RGB ambient(kAmbientIdentity);
		ActorVCall<FnSetConstantAmbient>(actor, kVtSetConstantAmbient)(actor, &ambient);
	}
}

// The actor behind a spawn, or null while its model is still loading.
void* ActorForSpawn(int spawnId)
{
	PlayerClient* spawn = GetSpawnByID(spawnId);
	return spawn ? static_cast<void*>(spawn->GetActor()) : nullptr;
}

} // namespace

void SetSpawnHighlight(int spawnId, uint32_t color, uint32_t mode)
{
	if (spawnId <= 0)
		return;
	if ((mode & (Highlight_Tint | Highlight_Ambient)) == 0)
		mode = Highlight_Tint;

	std::scoped_lock lock(s_hlMutex);
	if (s_highlights.size() >= kMaxHighlights && s_highlights.count(spawnId) == 0)
		return;

	HighlightEntry& entry = s_highlights[spawnId];
	if (entry.color != color || entry.mode != mode)
	{
		entry.color = color;
		entry.mode = mode;
		entry.applied = nullptr;      // force a rewrite on the next pulse
	}
}

void ClearSpawnHighlight(int spawnId)
{
	std::scoped_lock lock(s_hlMutex);
	auto it = s_highlights.find(spawnId);
	if (it == s_highlights.end())
		return;

	// Restore against the actor we actually wrote to. If the spawn is already
	// gone its actor went with it and there is nothing left to undo.
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
		out.emplace_back(id, entry.color, entry.mode, entry.applied != nullptr);
	return out;
}

// Main-thread upkeep. Three jobs: write the look onto actors that don't have
// it yet, notice when an actor pointer changes (model reload, zone, armor
// swap) and rewrite, and drop entries whose spawn has despawned.
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
			// Spawn is gone; its actor went with it, so there is nothing to
			// restore and holding the entry would leak into the next spawn
			// that reuses the ID.
			it = s_highlights.erase(it);
			continue;
		}

		void* actor = spawn->GetActor();
		if (actor && actor != entry.applied)
		{
			ApplyToActor(actor, entry.color, entry.mode);
			entry.applied = actor;
			entry.appliedMode = entry.mode;
		}
		++it;
	}
}

// Periodic rewrite. Equipment changes and LOD swaps rebuild an actor's
// materials without replacing the actor pointer, which silently drops our
// tint. Cheap enough to just redo them on a slow cadence.
void RefreshHighlights()
{
	std::scoped_lock lock(s_hlMutex);
	for (auto& [id, entry] : s_highlights)
	{
		if (!entry.applied)
			continue;
		if (ActorForSpawn(id) != entry.applied)
			continue;                      // PulseHighlights will re-apply
		ApplyToActor(entry.applied, entry.color, entry.mode);
	}
}

// A despawn invalidates the actor without giving us a chance to restore it.
// Drop the entry so a recycled spawn ID doesn't inherit the highlight.
void OnHighlightSpawnRemoved(int spawnId)
{
	std::scoped_lock lock(s_hlMutex);
	s_highlights.erase(spawnId);
}

// Zoning destroys every actor in the old zone. Forget everything rather than
// try to restore through dangling pointers.
void ForgetHighlights()
{
	std::scoped_lock lock(s_hlMutex);
	s_highlights.clear();
}
