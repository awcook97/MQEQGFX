/*
 * gpu/FxCatalog.h - the compiled effect catalogue.
 *
 * Every default look ships as DATA here, not as code: a form id plus its
 * parameters, which the vertex shader in LineFX.cpp expands. Adding an effect
 * is a table row. That is the only way per-class x per-ability stays
 * maintainable once you want dozens of them.
 *
 * WHAT EACH CLASS GETS
 * --------------------
 * Not a fixed four. A class gets one entry per thing it actually DOES, so the
 * count varies: a Magician has four pet elements and gets four elemental
 * looks plus its signature; a Warrior has no elements at all and instead gets
 * its disciplines and stances; a Bard gets song families. Slot 0 is always the
 * class signature - the shape you recognise it by at a distance - and the rest
 * are the abilities that class is actually known for.
 *
 * Colours lean on the same World of Warcraft class palette the Lua version
 * used for the signature slot, then shift per ability so a Wizard's fire and
 * ice read differently while both still read "wizard".
 */
#pragma once

#include "LineFX.h"

#include <cstdint>

namespace eqgfx::gpu {

/// One catalogue entry: a form plus everything the shader needs to draw it.
/// Deliberately a plain aggregate so the whole table is a compile-time constant.
struct FxDef
{
	const char* name;        ///< what this represents, for /eqgfx fx and the menu
	uint32_t    form;        ///< FxForm
	float       radius;
	float       height;
	float       speed;       ///< animation rate multiplier
	float       tilt;        ///< radians
	float       width;       ///< ribbon half-width
	uint32_t    segments;
	uint32_t    flags;       ///< FxFlag bits
	float       p0, p1, p2, p3;
	float       rgba[4];
};

/// The effects for one class, in slot order. Slot 0 is the signature.
struct ClassFx
{
	const char*  shortName;  ///< Spawn.Class.ShortName(), e.g. "WIZ"
	const char*  fullName;
	const FxDef* defs;
	int          count;
};

/// Look up a class by its ShortName. Returns nullptr when unknown (most NPCs).
const ClassFx* FindClassFx(const char* shortName);

/// Every class in display order, for menus and /eqgfx fx list.
const ClassFx* AllClassFx(int& countOut);

/// Total catalogue entries across all classes - the "how many animations do we
/// actually ship" number.
int TotalFxCount();

/// Fill an FxInstance from a catalogue entry at a world position. `phase`
/// should be stable per spawn (hash the spawn id) so neighbours of the same
/// class do not pulse in lockstep.
void MakeInstance(const FxDef& def, const float pos[3], float phase,
	float alphaScale, FxInstance& out);

} // namespace eqgfx::gpu
