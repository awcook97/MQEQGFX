#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

#include <cstring>

namespace eqgfx::spellfx {

namespace {

constexpr Pair kWarder    { { 0.70f, 1.00f, 0.85f, 0.90f }, { 0.25f, 0.65f, 0.55f, 0.80f } };
constexpr Pair kPrimal    { { 1.00f, 0.78f, 0.40f, 0.95f }, { 0.60f, 0.30f, 0.08f, 0.85f } };
constexpr Pair kSavage    { { 1.00f, 0.55f, 0.25f, 0.95f }, { 0.50f, 0.04f, 0.04f, 0.85f } };
constexpr Pair kPelt      { { 0.85f, 0.70f, 0.50f, 0.95f }, { 0.45f, 0.32f, 0.18f, 0.90f } };
constexpr Pair kMoon      { { 0.92f, 0.94f, 1.00f, 0.90f }, { 0.50f, 0.56f, 0.95f, 0.75f } };
constexpr Pair kIceFang   { { 0.92f, 1.00f, 1.00f, 0.95f }, { 0.40f, 0.82f, 1.00f, 0.85f } };
constexpr Pair kFrostVen  { { 0.80f, 1.00f, 0.95f, 0.95f }, { 0.30f, 0.80f, 0.35f, 0.80f } };
constexpr Pair kVenom     { { 0.80f, 1.00f, 0.35f, 0.95f }, { 0.25f, 0.45f, 0.05f, 0.85f } };
constexpr Pair kParagon   { { 0.85f, 0.95f, 1.00f, 0.95f }, { 1.00f, 0.80f, 0.35f, 0.85f } };
constexpr Pair kSpiritBst { { 0.75f, 1.00f, 0.95f, 0.90f }, { 0.30f, 0.70f, 0.95f, 0.80f } };
constexpr Pair kSloth     { { 0.78f, 0.86f, 0.72f, 0.85f }, { 0.36f, 0.44f, 0.32f, 0.70f } };
constexpr Pair kShaPanther{ { 0.70f, 0.60f, 0.85f, 0.90f }, { 0.18f, 0.12f, 0.30f, 0.85f } };

constexpr uint32_t GA_Emerge = GlyphAnim_Rise | GlyphAnim_Pop;

LOOK(kBstWarder, "warder",
	F(RE, FxForm_Vortex, 1.8f, 7.0f, 112, 5, 1.2f, 2.5f, 4.0f, FIX, kWarder, 4.0f, 0.07f),
	G(RE, Glyph_Wolf, 1.2f, 6.5f, 4, 1, GA_Emerge, 0, FIX, kWarder, 4.0f, 0.4f),
	G(RE, Glyph_Paw, 0.45f, 0.1f, 2, 6, GA_RingRise, 2.4f, FIX, kPrimal, 4.0f),
	F(RE, FxForm_Sparkle, 2.2f, 8.0f, 60, 30, 0.14f, 2, 0, FIX, kWarder, 3.5f),
	F(RE, FxForm_Rune, 2.8f, 0, 70, 6, 1, 12, 0, FIX, kPrimal, 4.0f, 0.05f));

LOOK(kBstMoon, "at the moon",
	G(IM, Glyph_Moon, 1.2f, 10.0f, 4, 1, GA_PopBob, 0, FIX, kMoon, 3.5f),
	F(IM, FxForm_Spire, 0.7f, 10.0f, 64, 10, 0.6f, 2.0f, 0, FIX, kMoon, 2.6f, 0.05f),
	G(IM, Glyph_Wolf, 0.7f, 2.5f, 3, 3, GA_RingRise, 2.2f, FIX, kMoon, 3.5f, 0.4f),
	G(RE, Glyph_Mouth, 0.5f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kMoon, 1.8f));

LOOK(kBstFeralCall, "feral call",
	G(IM, Glyph_Wolf, 0.8f, 3.0f, 3, 2, GA_Emerge, 1.8f, FIX, kSavage, 3.0f),
	F(IM, FxForm_Cracks, 2.2f, 0, 48, 6, 1.2f, 0, 0, FIX, kPelt, 3.0f, 0.07f, OVER),
	F(IM, FxForm_Burst, 2.2f, 0.3f, 64, 2, 0.3f, 12, 0, FIX, kSavage, 1.0f, 0.10f, ADD, 1, 0.35f));

LOOK(kBstEmpathy, "bestial empathy",
	G(IM, Glyph_Spirit, 0.7f, 3.0f, 3, 3, GA_RingRise, 2.0f, FIX, kWarder, 3.0f),
	G(IM, Glyph_Wolf, 0.6f, 5.5f, 3, 3, GA_Orbit, 1.6f, FIX, kSpiritBst, 3.0f, 0.3f),
	F(IM, FxForm_Bloom, 2.0f, 1.0f, 48, 3, 0, 0, 0, FIX, kWarder, 2.4f, 0.05f));

LOOK(kBstFeralgia, "feralgia",
	G(RE, Glyph_Wolf, 0.6f, 3.0f, 3, 3, GA_RingRise, 2.2f, FIX, kSavage, 2.8f),
	G(RE, Glyph_Mouth, 0.6f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kPrimal, 2.0f),
	F(RE, FxForm_Burst, 2.6f, 0.5f, 72, 2, 1.0f, 16, 0, FIX, kSavage, 1.2f, 0.10f));

LOOK(kBstSalve, "salve",
	G(IM, Glyph_Leaf, 1.0f, 8.5f, 4, 1, GA_Drop, 0, FIX, kNature, 2.2f),
	F(IM, FxForm_Flame, 0.8f, 8.0f, 72, 18, 0.1f, 2.4f, 0, FIX, kHeal, 1.8f, 0.07f, ADD, 1, 0.4f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kWarder, 2.2f));

LOOK(kBstMending, "warder mending",
	G(IM, Glyph_Paw, 0.8f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 2.2f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 64, 2, 1.4f, 0, 0.5f, FIX, kWarder, 2.4f, 0.05f, COMET),
	G(IM, Glyph_Cross, 0.3f, 4.5f, 0, 3, GA_Orbit, 1.2f, FIX, kHeal, 2.4f));

LOOK(kBstMelioration, "melioration",
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kWarder, 2.4f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Paw, 0.45f, 4.5f, 3, 6, GA_RingRise, 2.4f, FIX, kHeal, 2.6f),
	F(IM, FxForm_Flame, 0.75f, 7.0f, 64, 16, 0.1f, 2.2f, 0, FIX, kHeal, 1.6f, 0.06f));

LOOK(kBstMiracle, "miracle",
	G(IM, Glyph_Heart, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kHealGold, 2.2f),
	F(IM, FxForm_Burst, 2.0f, 1.5f, 64, 2, 2.0f, 12, 0, FIX, kHealGold, 1.0f, 0.08f));

LOOK(kBstPromised, "promised",
	G(IM, Glyph_Hourglass, 0.6f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kWarder, 3.0f),
	G(IM, Glyph_Leaf, 0.3f, 3.5f, 0, 3, GA_Orbit, 1.3f, FIX, kNature, 3.0f),
	F(IM, FxForm_Ring, 1.2f, 3.0f, 40, 0, 1.0f, 1.0f, 0.4f, FIX, kHeal, 3.0f, 0.05f, COMET));

LOOK(kBstProcHeal, "spirit mending",
	G(IM, Glyph_Paw, 0.3f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 1.4f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 20, 10, 0.10f, 0, 0, FIX, kWarder, 1.2f));

LOOK(kBstFocus, "focus",
	G(IM, Glyph_Spirit, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kPrimal, 2.6f),
	G(IM, Glyph_ArrowUp, 0.3f, 4.5f, 0, 4, GA_Orbit, 1.3f, FIX, kPrimal, 2.6f),
	F(IM, FxForm_Lattice, 1.3f, 6.0f, 72, 8, 0.5f, 0, 0, FIX, kPelt, 2.4f, 0.04f));

LOOK(kBstVivacity, "spiritual vivacity",
	G(IM, Glyph_ArrowUp, 0.4f, 3.0f, 3, 4, GA_RingRise, 1.6f, FIX, kSpiritBst, 2.6f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 72, 3, 1.4f, 0, 0.5f, FIX, kSpiritBst, 2.6f, 0.05f, COMET));

LOOK(kBstEpiphany, "spiritual epiphany",
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, 3.0f, FIX, kMana, 2.4f, 0.05f),
	F(IM, FxForm_Sparkle, 1.6f, 7.0f, 48, 24, 0.12f, 2, 0, FIX, kSpiritBst, 2.4f),
	G(IM, Glyph_Drop, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kMana, 2.6f));

LOOK(kBstFerocity, "ferocity",
	G(IM, Glyph_Fist, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kSavage, 2.4f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.8f, 0.9f, 0, FIX, kSavage, 2.4f, 0.07f),
	G(IM, Glyph_Claw, 0.3f, 4.5f, 0, 3, GA_Orbit, 1.2f, FIX, kPrimal, 2.4f));

LOOK(kBstSharedFerocity, "shared ferocity",
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 1.5f, 20, 0, FIX, kSavage, 1.4f, 0.12f, ADD, 1, 0, AE),
	G(RE, Glyph_Fist, 0.5f, 4.0f, 3, 5, GA_RingRise, 2.4f, FIX, kSavage, 2.6f),
	F(IM, FxForm_Flame, 0.9f, 5.0f, 64, 16, 0.8f, 0.8f, 0, FIX, kSavage, 1.8f, 0.06f));

LOOK(kBstRampart, "rampart",
	G(IM, Glyph_Shield, 0.9f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kPelt, 2.6f, 0, OVER),
	F(IM, FxForm_Crystal, 1.3f, 4.5f, 50, 10, 1, 0.9f, 0, FIX, kStone, 2.6f, 0.07f, OVER),
	F(IM, FxForm_Dome, 1.6f, 6.4f, 88, 6, 2, 0, 0, FIX, kPelt, 2.4f, 0.04f));

LOOK(kBstSpellbreaker, "spellbreaker",
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kSpiritBst, 2.6f, 0.04f),
	F(IM, FxForm_Rune, 1.8f, 0, 60, 6, 1, 12, 0, FIX, kArcane, 2.6f, 0.05f),
	G(IM, Glyph_Shield, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.4f, FIX, kSpiritBst, 2.6f));

LOOK(kBstGrowl, "growl",
	G(IM, Glyph_Mouth, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kPrimal, 2.2f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 48, 2, 0, 12, 0, FIX, kPrimal, 1.0f, 0.08f, ADD | FxFlag_Reverse),
	G(IM, Glyph_Claw, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kSavage, 2.4f));

LOOK(kBstFeralGrowl, "feral growl",
	G(IM, Glyph_Wolf, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSavage, 2.4f),
	F(IM, FxForm_Bloom, 1.8f, 2.0f, 48, 3, 0, 0, 0, FIX, kSavage, 2.0f, 0.05f),
	F(IM, FxForm_Flame, 0.9f, 4.5f, 64, 16, 0.8f, 0.8f, 0, FIX, kPrimal, 2.0f, 0.06f));

LOOK(kBstVelocity, "velocity",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 56, 12, 2.5f, 0, 0, FIX, kAir, 2.4f, 0.05f),
	G(IM, Glyph_Paw, 0.3f, 1.5f, 0, 4, GA_Orbit, 1.3f, FIX, kPrimal, 2.4f),
	G(IM, Glyph_ArrowUp, 0.5f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kAmber, 2.0f));

LOOK(kBstSpiritBuff, "spirit of",
	G(IM, Glyph_Spirit, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.4f, FIX, kSpiritBst, 3.0f),
	G(IM, Glyph_Claw, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kPrimal, 2.6f),
	F(IM, FxForm_Ring, 1.2f, 3.0f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kSpiritBst, 2.6f, 0.05f, COMET));

LOOK(kBstJaws, "jaws",
	G(IM, Glyph_Fang, 0.9f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kIron, 2.2f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0, 12, 0, FIX, kIron, 0.8f, 0.07f, ADD, 1, 0.3f),
	F(IM, FxForm_Ring, 1.1f, 3.0f, 40, 0, 1.6f, 1.0f, 0.6f, FIX, kPelt, 2.2f, 0.05f, COMET));

LOOK(kBstWarderWard, "warder ward",
	F(IM, FxForm_Dome, 1.6f, 6.4f, 88, 6, 2, 0, 0, FIX, kWarder, 2.6f, 0.04f),
	G(IM, Glyph_Paw, 0.7f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kWarder, 2.6f),
	F(IM, FxForm_Sparkle, 1.3f, 6.0f, 32, 16, 0.10f, 0, 0, FIX, kHeal, 2.4f));

LOOK(kBstAggression, "aggression",
	G(IM, Glyph_Horn, 0.8f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kSavage, 2.4f),
	F(IM, FxForm_Ring, 1.3f, 1.0f, 40, 0, 1.8f, 1.0f, 0.6f, FIX, kSavage, 2.4f, 0.05f, COMET),
	G(IM, Glyph_ArrowUp, 0.3f, 4.0f, 0, 3, GA_Orbit, 1.2f, FIX, kSavage, 2.4f));

LOOK(kBstProtection, "protection",
	G(IM, Glyph_Helm, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kPelt, 2.6f, 0, OVER),
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kPelt, 2.4f, 0.04f));

LOOK(kBstAlliance, "natural alliance",
	G(IM, Glyph_Link, 0.35f, 4.5f, 0, 4, GA_Orbit, 1.3f, FIX, kNature, 3.0f),
	G(IM, Glyph_Leaf, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNature, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 72, 2, 1.2f, 0, 0.5f, FIX, kWarder, 3.0f, 0.05f, COMET));

LOOK(kBstVenomAlliance, "venomous alliance",
	G(IM, Glyph_Snake, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kVenom, 3.0f),
	G(IM, Glyph_Link, 0.3f, 4.5f, 0, 4, GA_Orbit, 1.3f, FIX, kPoison, 3.0f),
	F(IM, FxForm_Bubbles, 1.1f, 5.5f, 48, 8, 0.2f, 0, 0, FIX, kVenom, 3.0f, 0.04f));

LOOK(kBstUnity, "spiritual unity",
	G(IM, Glyph_Spirit, 0.5f, 3.0f, 3, 4, GA_RingRise, 1.8f, FIX, kSpiritBst, 3.0f),
	F(IM, FxForm_Rune, 2.0f, 0, 70, 6, 1, 12, 0, FIX, kPrimal, 3.0f, 0.05f),
	G(IM, Glyph_ArrowUp, 0.5f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kParagon, 2.4f));

LOOK(kBstWarderUnity, "warder's unity",
	G(IM, Glyph_Wolf, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kWarder, 3.0f),
	G(IM, Glyph_Spirit, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.4f, FIX, kWarder, 3.0f),
	F(IM, FxForm_Rune, 1.8f, 0, 60, 5, 2, 8, 0, FIX, kWarder, 3.0f, 0.05f));

LOOK(kBstReflexive, "reflexive",
	G(IM, Glyph_Claw, 0.35f, 5.0f, 0, 4, GA_Orbit, 1.4f, FIX, kIron, 2.4f),
	F(IM, FxForm_Ring, 1.3f, 3.0f, 40, 0, 2.0f, 1.0f, 0.8f, FIX, kBlood, 2.2f, 0.05f, COMET));

LOOK(kBstVindication, "vindication",
	G(IM, Glyph_Fist, 0.9f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kGold, 2.4f),
	F(IM, FxForm_Burst, 2.2f, 2.0f, 64, 2, 1.5f, 16, 0, FIX, kPrimal, 1.0f, 0.09f),
	F(IM, FxForm_Spire, 0.7f, 7.0f, 64, 10, 0.6f, 2.0f, 0, FIX, kGold, 1.6f, 0.05f));

LOOK(kBstAuspice, "auspice",
	F(IM, FxForm_Dome, 1.6f, 6.4f, 88, 6, 2, 0, 0, FIX, kMoon, 2.6f, 0.04f),
	G(IM, Glyph_Moon, 0.6f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kMoon, 2.6f));

LOOK(kBstVivisection, "bestial vivisection",
	G(IM, Glyph_Claw, 0.9f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kBlood, 2.4f),
	G(IM, Glyph_Fang, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kBone, 2.6f),
	Persist(F(AU, FxForm_Flame, 1.05f, 5.5f, 96, 24, 0.9f, 0.9f, 0, FIX, kBlood, 0, 0.07f)));

LOOK(kBstSavage, "savage fury",
	Persist(F(AU, FxForm_Flame, 1.1f, 6.0f, 112, 28, 0.9f, 1.1f, 0, FIX, kSavage, 0, 0.09f)),
	G(IM, Glyph_Wolf, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kSavage, 2.4f),
	F(IM, FxForm_Burst, 2.6f, 0.5f, 72, 2, 1.0f, 16, 0, FIX, kSavage, 1.0f, 0.10f));

LOOK(kBstMonkey, "monkey's spirit",
	G(IM, Glyph_Spirit, 0.9f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kAmber, 2.6f),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 48, 10, 2.0f, 0, 0, FIX, kAmber, 2.4f),
	Persist(F(AU, FxForm_Orbit, 1.2f, 5.0f, 40, 10, 0, 0, 0, FIX, kAmber, 0, 0.04f, ADD, 2.5f)));

LOOK(kBstBreather, "breather",
	G(IM, Glyph_Drop, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kGold, 3.0f),
	G(IM, Glyph_Leaf, 0.6f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kNature, 2.4f),
	F(IM, FxForm_Bloom, 1.6f, 1.0f, 40, 2, 0, 0, 0, FIX, kGold, 2.4f, 0.04f));

LOOK(kBstProtectiveSpirit, "protective spirit",
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kSpiritBst, 3.0f, 0.04f),
	G(IM, Glyph_Spirit, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kSpiritBst, 3.0f),
	G(IM, Glyph_Shield, 0.3f, 4.5f, 0, 3, GA_Orbit, 1.4f, FIX, kPelt, 3.0f, 0, OVER));

LOOK(kBstClaws, "flurry of claws",
	G(IM, Glyph_Claw, 0.6f, 7.0f, 4, 4, GA_Rain, 1.0f, FIX, kBlood, 1.4f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 48, 2, 0.3f, 16, 0, FIX, kBlood, 0.8f, 0.09f, ADD, 1, 0.35f),
	F(IM, FxForm_Sparkle, 1.2f, 5.0f, 32, 16, 0.12f, 1, 0, FIX, kPrimal, 1.4f));

LOOK(kBstFocusedClaws, "focused claws",
	G(IM, Glyph_Claw, 1.3f, 6.0f, 4, 1, GA_PopRoll, 0, FIX, kBlood, 1.4f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kBlood, 1.8f, 0.06f, OVER),
	F(IM, FxForm_Burst, 2.4f, 2.0f, 64, 2, 0, 16, 0, FIX, kSavage, 0.8f, 0.10f, ADD, 1, 0.2f));

LOOK(kBstAssault, "warder assault",
	G(IM, Glyph_Paw, 1.1f, 8.0f, 4, 1, GA_Drop, 0, FIX, kPelt, 1.6f, 0, OVER),
	F(IM, FxForm_Cracks, 2.2f, 0, 48, 6, 1.2f, 0, 0, FIX, kPelt, 2.0f, 0.07f, OVER, 1, 0.3f),
	F(IM, FxForm_Burst, 2.4f, 0.4f, 64, 2, 0.4f, 12, 0, FIX, kPrimal, 1.0f, 0.10f, ADD, 1, 0.3f));

LOOK(kBstBlood, "venom blood",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.45f, 1.2f, FIX, kVenom, 0.8f, 0.18f),
	G(IM, Glyph_Snake, 0.8f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kVenom, 1.4f),
	F(IM, FxForm_Bubbles, 1.1f, 5.5f, 48, 10, 0.25f, 0, 0, FIX, kPoison, 2.0f, 0.04f),
	F(IM, FxForm_Burst, 2.2f, 1.5f, 64, 2, 1.5f, 12, 0, FIX, kVenom, 0.9f, 0.10f));

LOOK(kBstLance, "lance",
	F(TR, FxForm_Bolt, 1, 0, 128, 2, 0.1f, 1.0f, 0.3f, FIX, kIceFang, 0.7f, 0.16f),
	F(IM, FxForm_Crystal, 1.2f, 5.0f, 60, 12, 0, 1.0f, 0, FIX, kCold, 2.0f, 0.06f),
	G(IM, Glyph_Snowflake, 0.35f, 5.5f, 0, 3, GA_Orbit, 1.2f, FIX, kCold, 1.8f),
	F(IM, FxForm_Burst, 2.4f, 1.5f, 72, 2, 1.0f, 16, 0, FIX, kIceFang, 0.9f, 0.10f));

LOOK(kBstRoar, "roar",
	G(RE, Glyph_Mouth, 0.7f, 7.6f, 4, 1, GlyphAnim_Pop, 0, FIX, kIceFang, 1.2f),
	F(TR, FxForm_Bolt, 1, 0, 128, 5, 0.9f, 0.6f, 0.8f, FIX, kCold, 0.8f, 0.22f),
	F(IM, FxForm_Wind, 1.4f, 5.0f, 60, 12, 2.5f, 0, 0, FIX, kIceFang, 1.4f),
	F(IM, FxForm_Crystal, 1.4f, 4.0f, 50, 10, 1, 0.9f, 0, FIX, kCold, 2.2f, 0.06f));

LOOK(kBstFrozenVenom, "frozen venom",
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 0.6f, 0.5f, 1.4f, FIX, kFrostVen, 0.8f, 0.20f),
	F(IM, FxForm_Crystal, 1.2f, 5.0f, 50, 10, 0, 1.0f, 0, FIX, kCold, 2.2f, 0.06f),
	F(IM, FxForm_Bubbles, 1.1f, 5.5f, 48, 8, 0.2f, 0, 0, FIX, kPoison, 2.4f, 0.04f),
	G(IM, Glyph_Snake, 0.35f, 5.0f, 0, 2, GA_Orbit, 1.2f, FIX, kVenom, 2.4f),
	G(IM, Glyph_Snowflake, 0.35f, 5.0f, 0, 2, GA_Orbit, 1.2f, FIX, kCold, 2.4f));

LOOK(kBstEndemic, "endemic",
	G(IM, Glyph_Insect, 0.3f, 5.5f, 0, 6, GA_Orbit, 1.4f, FIX, kDisease, 3.0f, 0, OVER),
	F(IM, FxForm_Bubbles, 1.0f, 5.0f, 48, 8, 0.2f, 0, 0, FIX, kDisease, 3.0f, 0.04f, OVER),
	F(IM, FxForm_Bloom, 1.6f, 1.0f, 40, 2, 0, 0, 0, FIX, kDisease, 2.0f, 0.05f, OVER));

LOOK(kBstBite, "bite",
	G(IM, Glyph_Fang, 0.9f, 7.2f, 4, 1, GA_PopRoll, 0, FIX, kVenom, 2.2f),
	F(IM, FxForm_Bubbles, 1.0f, 5.5f, 40, 8, 0.2f, 0, 0, FIX, kPoison, 2.4f, 0.04f),
	F(IM, FxForm_Ring, 1.1f, 4.0f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kVenom, 2.2f, 0.05f, COMET));

LOOK(kBstDebuffBite, "withering bite",
	G(IM, Glyph_Fang, 0.8f, 7.2f, 4, 1, GA_PopRoll, 0, FIX, kCorrupt, 2.4f),
	G(IM, Glyph_ArrowDown, 0.3f, 5.5f, 0, 4, GA_Orbit, 1.2f, FIX, kCorrupt, 2.6f),
	F(IM, FxForm_Bloom, 1.6f, 2.0f, 40, 2, 0, 0, 0, FIX, kCorrupt, 2.0f, 0.04f, OVER | FxFlag_Reverse));

LOOK(kBstFatigue, "fatiguing bite",
	G(IM, Glyph_Fang, 0.8f, 7.2f, 4, 1, GA_PopRoll, 0, FIX, kAmber, 2.2f),
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.0f, 12, 0, FIX, kGold, 1.8f, 0.07f),
	G(IM, Glyph_Hourglass, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kAmber, 2.2f));

LOOK(kBstChill, "chill",
	G(IM, Glyph_Snowflake, 0.35f, 5.0f, 0, 4, GA_Orbit, 1.4f, FIX, kCold, 3.0f),
	F(IM, FxForm_Crystal, 1.4f, 3.5f, 50, 10, 1, 0.8f, 0, FIX, kIceFang, 3.0f, 0.06f),
	F(IM, FxForm_Ring, 1.4f, 0.4f, 48, 0, 0.3f, 1.0f, 0.2f, FIX, kCold, 3.0f, 0.05f, COMET));

LOOK(kBstMaelstrom, "maelstrom",
	F(IM, FxForm_Vortex, 1.4f, 6.0f, 96, 5, 1.2f, 2.5f, 5.0f, FIX, kFrostVen, 2.0f, 0.06f),
	G(IM, Glyph_Snowflake, 0.35f, 4.5f, 0, 2, GA_RingRise, 1.5f, FIX, kCold, 2.0f),
	G(IM, Glyph_Snake, 0.35f, 4.5f, 0, 2, GA_RingRise, 1.5f, FIX, kVenom, 2.0f),
	G(IM, Glyph_Insect, 0.35f, 4.5f, 0, 2, GA_RingRise, 1.5f, FIX, kDisease, 2.0f));

LOOK(kBstShaSlow, "sha's slow",
	G(IM, Glyph_Hourglass, 0.9f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kSloth, 3.0f),
	G(IM, Glyph_Claw, 0.35f, 4.5f, 0, 3, GA_Orbit, 1.3f, FIX, kShaPanther, 3.0f, 0, OVER),
	F(IM, FxForm_Ring, 1.5f, 0.4f, 48, 0, 0.2f, 1.0f, 0.15f, FIX, kSloth, 3.0f, 0.05f, COMET, 0.3f));

LOOK(kBstShaReprisal, "sha's reprisal",
	G(IM, Glyph_Spirit, 1.0f, 8.0f, 4, 1, GA_Drop, 0, FIX, kShaPanther, 2.4f, 0, OVER),
	G(IM, Glyph_Hourglass, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.4f, FIX, kSloth, 3.2f, 0.3f),
	F(IM, FxForm_Bloom, 1.8f, 0.5f, 48, 3, 0, 0, 0, FIX, kShaPanther, 2.4f, 0.05f, OVER, 1, 0.3f));

LOOK(kBstNaturesFury, "nature's fury",
	G(IM, Glyph_Leaf, 0.35f, 9.0f, 4, 6, GA_Rain, 1.6f, FIX, kNature, 2.2f),
	G(IM, Glyph_Fist, 0.8f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kPrimal, 2.0f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 64, 2, 1.5f, 12, 0, FIX, kNature, 1.0f, 0.09f));

LOOK(kBstDicho, "dichotomic fury",
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 2, FIX, kSavage, 2.6f, 0.05f),
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 2, FIX, kSpiritBst, 2.6f, 0.05f, ADD, -1.0f),
	G(IM, Glyph_Claw, 0.6f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kPrimal, 2.6f));

LOOK(kAaAttackWarder, "attack of the warder",
	G(IM, Glyph_Wolf, 0.7f, 8.0f, 4, 3, GA_Rain, 1.8f, FIX, kWarder, 2.6f),
	F(IM, FxForm_Burst, 2.6f, 0.4f, 72, 2, 0.4f, 12, 0, FIX, kWarder, 1.0f, 0.10f, ADD, 1, 0.4f),
	F(IM, FxForm_Sparkle, 1.8f, 6.0f, 40, 20, 0.12f, 2, 0, FIX, kSpiritBst, 2.4f));

LOOK(kAaBestialAlignment, "bestial alignment",
	G(IM, Glyph_Wolf, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kPrimal, 3.5f),
	G(IM, Glyph_Paw, 0.45f, 0.1f, 2, 6, GA_RingRise, 2.2f, FIX, kPrimal, 3.5f),
	F(IM, FxForm_Lattice, 1.3f, 6.5f, 96, 10, 0.5f, 0, 0, FIX, kPrimal, 3.5f, 0.05f),
	F(IM, FxForm_Flame, 1.05f, 5.5f, 96, 24, 0.9f, 0.9f, 0, FIX, kSavage, 3.0f, 0.07f));

LOOK(kAaGroupAlignment, "group bestial alignment",
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 1.5f, 24, 0, FIX, kPrimal, 1.4f, 0.12f, ADD, 1, 0, AE),
	G(RE, Glyph_Wolf, 0.6f, 3.5f, 3, 5, GA_RingRise, 2.4f, FIX, kPrimal, 3.0f),
	F(IM, FxForm_Lattice, 1.2f, 6.0f, 72, 8, 0.5f, 0, 0, FIX, kSavage, 2.6f, 0.04f));

LOOK(kAaBloodlust, "bloodlust",
	G(IM, Glyph_Drop, 0.3f, 9.0f, 4, 6, GA_Rain, 1.4f, FIX, kBlood, 2.2f),
	F(IM, FxForm_Flame, 1.05f, 6.0f, 96, 24, 0.9f, 1.0f, 0, FIX, kBlood, 3.0f, 0.08f),
	G(IM, Glyph_Fang, 0.7f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kBlood, 2.6f));

LOOK(kAaChameleon, "chameleon strike",
	G(IM, Glyph_Claw, 0.8f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kPrism, 1.8f),
	F(IM, FxForm_Bloom, 1.6f, 3.0f, 40, 2, 0, 0, 0, FIX, kPrism, 1.8f, 0.05f, OVER | FxFlag_Reverse),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.12f, 0, 0, FIX, kPrism, 1.8f));

LOOK(kAaCheetah, "cheetah's pounce",
	F(RE, FxForm_Wind, 1.4f, 5.0f, 72, 14, 3.5f, 0, 0, FIX, kAmber, 1.6f, 0.05f),
	G(RE, Glyph_Paw, 0.4f, 0.1f, 2, 5, GA_PopSpin, 1.6f, FIX, kAmber, 1.6f));

LOOK(kAaConsumption, "consumption of spirit",
	F(IM, FxForm_Drain, 1, 0, 48, 3, 1.2f, 12, 0, FIX, kBlood, 1.6f, 0.07f),
	G(IM, Glyph_Heart, 0.6f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kBlood, 1.8f),
	G(IM, Glyph_Drop, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kMana, 2.4f, 0.5f),
	F(IM, FxForm_Vortex, 1.1f, 5.0f, 64, 4, 1.2f, 2.0f, 3.0f, FIX, kMana, 2.0f, 0.05f, ADD, 1, 0.5f));

LOOK(kAaEnduringFrenzy, "enduring frenzy",
	G(IM, Glyph_Fist, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSavage, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 72, 3, 1.6f, 0, 0.6f, FIX, kSavage, 3.0f, 0.05f, COMET),
	G(IM, Glyph_Drop, 0.3f, 4.0f, 0, 3, GA_Orbit, 1.2f, FIX, kGold, 3.0f));

LOOK(kAaFalsifiedDeath, "falsified death",
	G(IM, Glyph_Skull, 0.8f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kGrey, 2.2f, 0, OVER),
	F(IM, FxForm_Bloom, 1.6f, 1.0f, 40, 2, 0, 0, 0, FIX, kGrey, 2.0f, 0.05f, OVER | FxFlag_Reverse));

LOOK(kAaFerociousness, "ferociousness",
	G(IM, Glyph_Fist, 1.1f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kSavage, 3.0f),
	F(IM, FxForm_Burst, 3.0f, 0.5f, 96, 3, 1.0f, 20, 0, FIX, kSavage, 1.2f, 0.12f),
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.9f, 1.2f, 0, FIX, kSavage, 3.0f, 0.09f));

LOOK(kAaParagon, "paragon of spirit",
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 2.0f, 20, 0, FIX, kParagon, 1.6f, 0.10f, ADD, 1, 0, AE),
	G(RE, Glyph_Spirit, 0.5f, 3.5f, 3, 5, GA_RingRise, 2.4f, FIX, kParagon, 3.0f),
	G(IM, Glyph_Drop, 0.3f, 5.0f, 0, 2, GA_Orbit, 1.3f, FIX, kMana, 2.6f),
	G(IM, Glyph_Drop, 0.3f, 5.0f, 0, 2, GA_Orbit, 1.3f, FIX, kGold, 2.6f, 0.2f));

LOOK(kAaFocusedParagon, "focused paragon of spirit",
	G(IM, Glyph_Spirit, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kParagon, 3.0f),
	F(IM, FxForm_Spire, 0.7f, 8.5f, 64, 12, 0.6f, 2.4f, 0, FIX, kParagon, 2.2f, 0.05f),
	F(IM, FxForm_Ring, 1.2f, 3.0f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kMana, 2.6f, 0.05f, COMET));

LOOK(kAaFrenzyOfSpirit, "frenzy of spirit",
	G(IM, Glyph_Spirit, 0.35f, 5.0f, 0, 4, GA_Orbit, 1.4f, FIX, kSavage, 3.0f),
	F(IM, FxForm_Flame, 1.05f, 6.0f, 96, 24, 0.9f, 1.0f, 0, FIX, kSavage, 3.0f, 0.08f),
	G(IM, Glyph_Claw, 0.8f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kPrimal, 2.6f));

LOOK(kAaFrenziedSwipes, "frenzied swipes",
	G(IM, Glyph_Claw, 0.4f, 5.0f, 0, 5, GA_Orbit | GlyphAnim_Roll, 1.3f, FIX, kBlood, 2.6f),
	F(IM, FxForm_Ring, 1.3f, 4.0f, 40, 0, 2.5f, 1.0f, 1.0f, FIX, kSavage, 2.6f, 0.05f, COMET));

LOOK(kAaFeralSwipe, "feral swipe",
	G(IM, Glyph_Claw, 1.0f, 6.5f, 4, 1, GA_PopRoll, 0, FIX, kPrimal, 1.2f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 64, 1, 0.3f, 16, 0, FIX, kPrimal, 0.7f, 0.10f, ADD, 1, 0.2f));

LOOK(kAaGelidClaw, "gelid claw",
	G(IM, Glyph_Claw, 1.0f, 6.5f, 4, 1, GA_PopRoll, 0, FIX, kIceFang, 1.4f),
	F(IM, FxForm_Crystal, 1.3f, 4.5f, 50, 10, 0, 1.0f, 0, FIX, kIceFang, 2.0f, 0.06f, ADD, 1, 0.2f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 48, 1, 0.3f, 16, 0, FIX, kCold, 0.7f, 0.08f, ADD, 1, 0.2f));

LOOK(kAaGorillaSmash, "gorilla smash",
	G(IM, Glyph_Fist, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kPelt, 1.4f, 0, OVER),
	F(IM, FxForm_Cracks, 2.6f, 0, 64, 8, 1.4f, 0, 0, FIX, kEarth, 2.4f, 0.08f, OVER, 1, 0.35f),
	F(IM, FxForm_Burst, 3.0f, 0.4f, 96, 3, 0.5f, 16, 0, FIX, kPelt, 1.0f, 0.12f, OVER, 1, 0.35f),
	Stars(0.4f));

LOOK(kAaHobble, "hobble of spirits",
	G(IM, Glyph_Shackle, 0.8f, 1.5f, 4, 1, GA_PopBob, 0, FIX, kSloth, 3.0f),
	G(IM, Glyph_Link, 0.3f, 1.0f, 0, 4, GA_Orbit, 1.2f, FIX, kSpiritBst, 3.0f),
	F(IM, FxForm_Ring, 1.3f, 0.4f, 40, 0, 0.3f, 1.0f, 0.2f, FIX, kSpiritBst, 3.0f, 0.05f, COMET));

LOOK(kAaNaturalInvis, "natural invisibility",
	G(IM, Glyph_Leaf, 0.3f, 4.5f, 0, 4, GA_Orbit | GlyphAnim_Ascend, 1.3f, FIX, kNature, 2.0f),
	F(IM, FxForm_Bloom, 1.8f, 3.0f, 48, 3, 0, 0, 0, FIX, kGhost, 2.0f, 0.05f, ADD | FxFlag_Reverse));

LOOK(kAaNaturesSalve, "nature's salve",
	G(IM, Glyph_Leaf, 0.35f, 9.0f, 4, 6, GA_Rain, 1.4f, FIX, kHeal, 2.2f),
	G(IM, Glyph_Drop, 0.7f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kWhite, 2.0f),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.12f, 1, 0, FIX, kHeal, 2.0f));

LOOK(kAaPactWurine, "pact of the wurine",
	G(IM, Glyph_Moon, 0.8f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kMoon, 3.0f),
	G(IM, Glyph_Paw, 0.4f, 0.1f, 2, 6, GA_RingRise, 2.0f, FIX, kPelt, 3.0f),
	F(IM, FxForm_Lattice, 1.2f, 6.0f, 72, 8, 0.5f, 0, 0, FIX, kMoon, 3.0f, 0.04f));

LOOK(kAaPlayingPossum, "playing possum",
	G(IM, Glyph_Z, 0.5f, 5.0f, 0, 3, GA_Orbit | GlyphAnim_Ascend, 1.0f, FIX, kGrey, 2.4f),
	G(IM, Glyph_Paw, 0.8f, 0.1f, 2, 1, GlyphAnim_Pop, 0, FIX, kPelt, 2.4f, 0, OVER));

LOOK(kAaPrimalistSynergy, "primalist's synergy",
	G(IM, Glyph_Link, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kPrimal, 2.4f),
	G(IM, Glyph_Spirit, 0.7f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kSpiritBst, 2.2f),
	F(IM, FxForm_Burst, 2.0f, 2.0f, 48, 1, 1.5f, 12, 0, FIX, kPrimal, 0.9f, 0.08f));

LOOK(kAaProtectionWarder, "protection of the warder",
	F(IM, FxForm_Dome, 2.0f, 7.4f, 128, 10, 4, 0, 0, FIX, kWarder, 3.5f, 0.06f),
	G(IM, Glyph_Wolf, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kWarder, 3.5f),
	G(IM, Glyph_Shield, 0.35f, 4.5f, 0, 3, GA_Orbit, 1.6f, FIX, kWarder, 3.5f));

LOOK(kAaRavensClaw, "raven's claw",
	G(IM, Glyph_Feather, 0.35f, 9.0f, 4, 5, GA_Rain, 1.4f, FIX, kShadow, 2.2f, 0, OVER),
	G(IM, Glyph_Claw, 0.9f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kShadow, 1.8f, 0.2f, OVER),
	F(IM, FxForm_Burst, 2.0f, 3.0f, 48, 1, 0.3f, 14, 0, FIX, kShadow, 0.8f, 0.08f, OVER, 1, 0.3f));

LOOK(kAaRoarThunder, "roar of thunder",
	G(RE, Glyph_Mouth, 0.8f, 7.6f, 4, 1, GlyphAnim_Pop, 0, FIX, kShock, 1.2f),
	F(IM, FxForm_Lightning, 1, 36.0f, 64, 3, 0.8f, 12.0f, 1.0f, FIX, kShock, 0.8f, 0.14f),
	F(IM, FxForm_Burst, 2.8f, 1.0f, 96, 3, 1.0f, 20, 0, FIX, kShock, 1.0f, 0.12f, ADD, 1, 0.3f),
	Stars(0.3f));

LOOK(kAaRoaringStrike, "roaring strike",
	G(IM, Glyph_Mouth, 0.7f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kSavage, 1.6f),
	G(IM, Glyph_Claw, 1.0f, 6.0f, 4, 1, GA_PopRoll, 0, FIX, kSavage, 1.4f, 0.2f),
	F(IM, FxForm_Burst, 2.6f, 2.0f, 72, 2, 0.5f, 16, 0, FIX, kSavage, 0.9f, 0.10f, ADD, 1, 0.2f));

LOOK(kAaSpire, "spire of the savage lord",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kPrimal, 2.6f, 0.10f),
	G(RE, Glyph_Wolf, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kPrimal, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kSavage, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaBloodrage, "bestial bloodrage",
	G(IM, Glyph_Fang, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kBlood, 3.0f),
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.9f, 1.2f, 0, FIX, kBlood, 3.0f, 0.09f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kSavage, 3.0f));

LOOK(kAaWarderGift, "warder's gift",
	G(IM, Glyph_Heart, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kWarder, 2.6f),
	F(IM, FxForm_Sparkle, 1.4f, 7.0f, 40, 20, 0.12f, 2, 0, FIX, kWarder, 2.4f),
	G(IM, Glyph_Paw, 0.3f, 4.0f, 0, 3, GA_Orbit, 1.2f, FIX, kWarder, 2.6f));

LOOK(kAaWarderBenevolence, "warder's benevolence",
	G(IM, Glyph_Cross, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 2.4f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 72, 3, 1.2f, 0, 0.5f, FIX, kWarder, 2.6f, 0.05f, COMET),
	G(IM, Glyph_Wolf, 0.3f, 4.0f, 0, 3, GA_Orbit, 1.3f, FIX, kHeal, 2.6f));

LOOK(kAaWarderShielding, "warder's shielding",
	G(IM, Glyph_Shield, 0.8f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kPelt, 2.6f, 0, OVER),
	G(IM, Glyph_Paw, 0.3f, 4.0f, 0, 3, GA_Orbit, 1.3f, FIX, kPelt, 2.6f, 0, OVER),
	F(IM, FxForm_Ring, 1.3f, 3.0f, 40, 0, 1.0f, 1.0f, 0.4f, FIX, kWarder, 2.6f, 0.05f, COMET));

LOOK(kAaMendCompanion, "mend companion",
	G(IM, Glyph_Cross, 0.8f, 7.6f, 4, 1, GA_Drop, 0, FIX, kHealGold, 2.2f),
	F(IM, FxForm_Flame, 0.8f, 7.0f, 64, 16, 0.1f, 2.2f, 0, FIX, kHeal, 1.8f, 0.06f, ADD, 1, 0.3f),
	G(IM, Glyph_Paw, 0.3f, 4.0f, 0, 3, GA_Orbit, 1.2f, FIX, kWarder, 2.2f));

enum Line : int
{
	Warder, Moon, FeralCall, Empathy, Feralgia, Salve, Mending, Melioration, Miracle, Promised, ProcHeal,
	Focus, Vivacity, Epiphany, Ferocity, SharedFerocity, Rampart, Spellbreaker, Growl, FeralGrowl,
	Velocity, SpiritBuff, Jaws, WarderWard, Aggression, Protection, Alliance, VenomAlliance, Unity,
	WarderUnity, Reflexive, Vindication, Auspice, Vivisection, Savage, Monkey, Breather, ProtectiveSpirit,
	Claws, FocusedClaws, Assault, Blood, Lance, Roar, FrozenVenom, Endemic, Bite, DebuffBite, Fatigue,
	Chill, Maelstrom, ShaSlow, ShaReprisal, NaturesFury, Dicho,
	AttackWarder, BestialAlignment, GroupAlignment, Bloodlust, Chameleon, Cheetah, Consumption,
	EnduringFrenzy, FalsifiedDeath, Ferociousness, Paragon, FocusedParagon, FrenzyOfSpirit, FrenziedSwipes,
	FeralSwipe, GelidClaw, GorillaSmash, Hobble, NaturalInvis, NaturesSalve, PactWurine, PlayingPossum,
	PrimalistSynergy, ProtectionWarder, RavensClaw, RoarThunder, RoaringStrike, Spire, Bloodrage,
	WarderGift, WarderBenevolence, WarderShielding, MendCompanion,
};

const LineLook kLines[] = {
	{ "Warder (pet)",                     &kBstWarder },
	{ "At the Moon (wolf swarm)",         &kBstMoon },
	{ "Feralgia's Call (beast swarm)",    &kBstFeralCall },
	{ "Bestial Empathy (spirit swarm)",   &kBstEmpathy },
	{ "Feralgia (swarm, growl)",          &kBstFeralgia },
	{ "Salve (heal)",                     &kBstSalve },
	{ "Mending (warder heal)",            &kBstMending },
	{ "Melioration (group heal)",         &kBstMelioration },
	{ "Miracle (heal proc)",              &kBstMiracle },
	{ "Promised (delayed heal)",          &kBstPromised },
	{ "spirit mending proc",              &kBstProcHeal },
	{ "Focus (hit points)",               &kBstFocus },
	{ "Spiritual Vivacity (stats)",       &kBstVivacity },
	{ "Spiritual Epiphany (mana regen)",  &kBstEpiphany },
	{ "Ferocity (attack)",                &kBstFerocity },
	{ "Shared Ferocity (group attack)",   &kBstSharedFerocity },
	{ "Rampart / Guard",                  &kBstRampart },
	{ "Spellbreaker's (warder ward)",     &kBstSpellbreaker },
	{ "Growl (warder damage)",            &kBstGrowl },
	{ "Feralgia Growl",                   &kBstFeralGrowl },
	{ "Velocity (warder haste)",          &kBstVelocity },
	{ "Spirit of (warder proc)",          &kBstSpiritBuff },
	{ "Jaws (warder bite)",               &kBstJaws },
	{ "Warder (warder ward)",             &kBstWarderWard },
	{ "Aggression (warder hate)",         &kBstAggression },
	{ "Protection (warder mitigation)",   &kBstProtection },
	{ "Natural Alliance",                 &kBstAlliance },
	{ "Venomous Alliance",                &kBstVenomAlliance },
	{ "Spiritual / Feralist's Unity",     &kBstUnity },
	{ "Warder's Unity",                   &kBstWarderUnity },
	{ "Reflexive (melee proc)",           &kBstReflexive },
	{ "Vindication",                      &kBstVindication },
	{ "Auspice / Aegis (warder ward)",    &kBstAuspice },
	{ "Bestial Vivisection (discipline)", &kBstVivisection },
	{ "Savage (discipline)",              &kBstSavage },
	{ "Monkey's Spirit (discipline)",     &kBstMonkey },
	{ "Breather (discipline)",            &kBstBreather },
	{ "Protective Spirit (discipline)",   &kBstProtectiveSpirit },
	{ "Claws (melee nuke)",               &kBstClaws },
	{ "Focused Claws",                    &kBstFocusedClaws },
	{ "Assault (warder strike)",          &kBstAssault },
	{ "Blood (poison nuke)",              &kBstBlood },
	{ "Lance (cold nuke)",                &kBstLance },
	{ "Roar (cold nuke)",                 &kBstRoar },
	{ "Frozen Venom (cold, poison)",      &kBstFrozenVenom },
	{ "Endemic (disease)",                &kBstEndemic },
	{ "Bite (poison)",                    &kBstBite },
	{ "Withering Bite (debuff)",          &kBstDebuffBite },
	{ "Fatiguing Bite (endurance drain)", &kBstFatigue },
	{ "Chill (cold)",                     &kBstChill },
	{ "Maelstrom (warder nukes)",         &kBstMaelstrom },
	{ "Sha's (slow)",                     &kBstShaSlow },
	{ "Sha's Reprisal (slow)",            &kBstShaReprisal },
	{ "Nature's Fury",                    &kBstNaturesFury },
	{ "Dichotomic Fury",                  &kBstDicho },

	{ "AA: Attack of the Warder",         &kAaAttackWarder },
	{ "AA: Bestial Alignment",            &kAaBestialAlignment },
	{ "AA: Group Bestial Alignment",      &kAaGroupAlignment },
	{ "AA: Bloodlust",                    &kAaBloodlust },
	{ "AA: Chameleon Strike",             &kAaChameleon },
	{ "AA: Cheetah's Pounce",             &kAaCheetah },
	{ "AA: Consumption of Spirit",        &kAaConsumption },
	{ "AA: Enduring Frenzy",              &kAaEnduringFrenzy },
	{ "AA: Falsified Death",              &kAaFalsifiedDeath },
	{ "AA: Ferociousness",                &kAaFerociousness },
	{ "AA: Paragon of Spirit",            &kAaParagon },
	{ "AA: Focused Paragon of Spirit",    &kAaFocusedParagon },
	{ "AA: Frenzy of Spirit",             &kAaFrenzyOfSpirit },
	{ "AA: Frenzied Swipes",              &kAaFrenziedSwipes },
	{ "AA: Feral Swipe",                  &kAaFeralSwipe },
	{ "AA: Gelid Claw",                   &kAaGelidClaw },
	{ "AA: Gorilla Smash",                &kAaGorillaSmash },
	{ "AA: Hobble of Spirits",            &kAaHobble },
	{ "AA: Natural Invisibility",         &kAaNaturalInvis },
	{ "AA: Nature's Salve",               &kAaNaturesSalve },
	{ "AA: Pact of the Wurine",           &kAaPactWurine },
	{ "AA: Playing Possum",               &kAaPlayingPossum },
	{ "AA: Primalist's Synergy",          &kAaPrimalistSynergy },
	{ "AA: Protection of the Warder",     &kAaProtectionWarder },
	{ "AA: Raven's Claw",                 &kAaRavensClaw },
	{ "AA: Roar of Thunder",              &kAaRoarThunder },
	{ "AA: Roaring Strike",               &kAaRoaringStrike },
	{ "AA: Spire of the Savage Lord",     &kAaSpire },
	{ "AA: Bestial Bloodrage",            &kAaBloodrage },
	{ "AA: Warder's Gift",                &kAaWarderGift },
	{ "AA: Warder's Benevolence",         &kAaWarderBenevolence },
	{ "AA: Warder's Shielding",           &kAaWarderShielding },
	{ "AA: Mend Companion",               &kAaMendCompanion },
};

const GroupLine kNumbered[] = {
	{ 0, Salve },       { 1, Blood },        { 2, Mending },     { 3, Focus },       { 4, Vivacity },
	{ 5, Promised },    { 6, ProcHeal },     { 7, Velocity },    { 8, SpiritBuff },  { 10, Jaws },
	{ 12, Lance },      { 13, Epiphany },    { 14, Rampart },    { 15, Endemic },    { 16, Ferocity },
	{ 17, Bite },       { 18, Spellbreaker }, { 20, Assault },   { 21, Rampart },    { 23, Growl },
	{ 24, Melioration }, { 25, FrozenVenom }, { 26, WarderWard }, { 27, Feralgia },  { 29, Aggression },
	{ 30, Protection }, { 31, Claws },       { 32, ShaReprisal }, { 33, NaturesFury }, { 34, Roar },
	{ 35, Maelstrom },  { 36, FrozenVenom }, { 38, Alliance },   { 39, Savage },     { 40, FeralGrowl },
	{ 41, ProcHeal },   { 42, Reflexive },   { 43, DebuffBite }, { 44, Chill },      { 45, FocusedClaws },
	{ 46, Monkey },     { 47, Unity },       { 48, Fatigue },    { 49, WarderUnity }, { 50, SharedFerocity },
	{ 51, Dicho },      { 52, Dicho },       { 53, VenomAlliance },
};

const GroupLine kModern[] = {
	{ 10, Salve },      { 20, Blood },       { 30, Mending },    { 40, Focus },      { 50, Vivacity },
	{ 60, Promised },   { 61, ProcHeal },    { 70, Velocity },   { 80, SpiritBuff }, { 100, Lance },
	{ 110, Epiphany },  { 130, Endemic },    { 150, Bite },      { 160, Spellbreaker }, { 161, ProcHeal },
	{ 170, Moon },      { 171, Moon },       { 180, Assault },   { 200, Growl },     { 210, Melioration },
	{ 211, Miracle },   { 220, FrozenVenom }, { 221, FrozenVenom }, { 230, WarderWard }, { 240, Feralgia },
	{ 241, FeralGrowl }, { 242, FeralCall }, { 250, Vivisection }, { 260, Aggression }, { 270, Protection },
	{ 280, Claws },     { 310, Roar },       { 320, Maelstrom }, { 330, Alliance },  { 350, Reflexive },
	{ 370, Chill },     { 390, Monkey },     { 400, Unity },     { 410, Fatigue },   { 420, WarderUnity },
	{ 440, Dicho },     { 441, Dicho },      { 450, VenomAlliance }, { 460, Vindication },
};

const GroupLine kOther[] = {
	{ 7000, Salve },     { 7001, Blood },     { 7004, Mending },   { 7005, Focus },     { 7006, Vivacity },
	{ 7007, Promised },  { 7009, Velocity },  { 7010, SpiritBuff }, { 7012, Jaws },     { 7014, Lance },
	{ 7016, Epiphany },  { 7017, Rampart },   { 7019, Endemic },   { 7020, Ferocity },  { 7021, Bite },
	{ 7023, Spellbreaker }, { 7025, Moon },   { 1510131, SpiritBuff },
	{ 11872, Moon },     { 14154, Moon },     { 18166, Moon },     { 27225, Moon },     { 30374, Moon },
	{ 36431, Moon },     { 36531, Moon },
	{ 2228, FrozenVenom }, { 2328, Vivisection }, { 2428, Vivisection }, { 2528, Vivisection },
	{ 2728, Vivisection }, { 2928, Vivisection },
	{ 57401, Miracle },  { 60631, Miracle },  { 64102, Miracle },
	{ 2102054, Vindication }, { 2202054, Vindication }, { 2315054, Vindication }, { 2415054, Vindication },
	{ 7543, Auspice },   { 7643, Auspice },   { 7743, Auspice },   { 7843, Auspice },   { 7943, Auspice },
	{ 125130430, Auspice }, { 9524, Auspice }, { 9624, Auspice },  { 9724, Auspice },   { 9824, Auspice },
	{ 9924, Auspice },   { 125110190, Auspice },
	{ 3311, Breather },  { 3411, Breather },  { 3511, Breather },  { 3611, Breather },  { 3737, Breather },
	{ 3837, Breather },  { 3937, Breather },  { 125010040, Breather }, { 126010040, Breather },
	{ 8233, Savage },    { 51820, Unity },
};

const NamedLine kAbilities[] = {
	{ "attack of the warder",                  AttackWarder },
	{ "bestial alignment",                     BestialAlignment },
	{ "bestial alignment form",                BestialAlignment },
	{ "group bestial alignment",               GroupAlignment },
	{ "group bestial alignment form",          GroupAlignment },
	{ "bloodlust",                             Bloodlust },
	{ "chameleon strike",                      Chameleon },
	{ "cheetah's pounce",                      Cheetah },
	{ "consumption of spirit",                 Consumption },
	{ "enduring frenzy",                       EnduringFrenzy },
	{ "enduring frenzy restoration",           EnduringFrenzy },
	{ "falsified death",                       FalsifiedDeath },
	{ "falsified death pet fade",              FalsifiedDeath },
	{ "ferociousness",                         Ferociousness },
	{ "paragon of spirit",                     Paragon },
	{ "focused paragon of spirit",             FocusedParagon },
	{ "frenzy of spirit",                      FrenzyOfSpirit },
	{ "frenzied swipes",                       FrenziedSwipes },
	{ "feral swipe",                           FeralSwipe },
	{ "gelid claw",                            GelidClaw },
	{ "gorilla smash",                         GorillaSmash },
	{ "hobble of spirits",                     Hobble },
	{ "hobble of spirits snare",               Hobble },
	{ "natural invisibility",                  NaturalInvis },
	{ "perfected natural invisibility",        NaturalInvis },
	{ "improved natural invisibility",         NaturalInvis },
	{ "perfected improved natural invisibility", NaturalInvis },
	{ "nature's salve",                        NaturesSalve },
	{ "nature's salve warder cure",            NaturesSalve },
	{ "pact of the wurine",                    PactWurine },
	{ "pact of the wurine form",               PactWurine },
	{ "playing possum",                        PlayingPossum },
	{ "primalist's synergy",                   PrimalistSynergy },
	{ "protection of the warder",              ProtectionWarder },
	{ "raven's claw",                          RavensClaw },
	{ "roar of thunder",                       RoarThunder },
	{ "roar of thunder weakness",              RoarThunder },
	{ "roaring strike",                        RoaringStrike },
	{ "spire of the savage lord",              Spire },
	{ "apex of the savage lord",               Spire },
	{ "companion's savagery",                  Spire },
	{ "bestial bloodrage",                     Bloodrage },
	{ "warder's gift",                         WarderGift },
	{ "warder's benevolence",                  WarderBenevolence },
	{ "warder's shielding",                    WarderShielding },
	{ "mend companion",                        MendCompanion },
	{ "sha's reprisal",                        ShaReprisal },
};

bool Contains(std::string_view s, std::string_view part)
{
	return s.find(part) != std::string_view::npos;
}

int BySpell(const eqlib::EQ_Spell* sp, std::string_view base)
{
	const std::string_view extra(sp->Extra, strnlen(sp->Extra, sizeof(sp->Extra)));
	if (StartsWith(extra, "PCPetBst"))
		return Warder;
	if (StartsWith(extra, "PCSwmBst"))
	{
		if (Contains(base, "at the moon")) return Moon;
		if (base == "bestial empathy")     return Empathy;
		return FeralCall;
	}
	if (Contains(base, " at the moon"))
		return Moon;
	if (EndsWith(base, "'s call"))
		return FeralCall;

	if (StartsWith(base, "savage ") || base == "bestial fury discipline")
		return Savage;
	if (StartsWith(base, "bestial ") || StartsWith(base, "warder's "))
		return Vivisection;
	if (base == "protective spirit discipline")
		return ProtectiveSpirit;

	if (EndsWith(base, " poison strike") || EndsWith(base, " disease strike") || EndsWith(base, " ice strike"))
		return Maelstrom;

	if (StartsWith(base, "companion's aid") || base == "friendly pet heal")
		return ProcHeal;

	if (base == "sha's lethargy" || base == "sha's advantage" || base == "sha's revenge" || base == "sha's legacy")
		return ShaSlow;

	if (StartsWith(base, "spirit of ") && base != "spirit of wolf" && base != "spirit of monkey"
		&& base != "spirit of ox" && base != "spirit of bear" && base != "spirit of the shrew")
		return SpiritBuff;

	if (StartsWith(base, "growl of ")) return Growl;
	if (StartsWith(base, "healing of ")) return Salve;
	if (StartsWith(base, "focus of ") || base == "feral vigor") return Focus;
	if (base == "spiritual vigor" || base == "spiritual vitality") return Vivacity;
	if (base == "spiritual dominion" || base == "spiritual ascendance") return Epiphany;
	if (base == "feral guard") return Rampart;
	if (base == "festering malady") return Endemic;
	if (base == "arag's celerity" || EndsWith(base, " velocity")) return Velocity;
	if (base == "rake") return Assault;
	if (base == "savagery" || base == "sha's ferocity" || StartsWith(base, "ferocity")) return Ferocity;
	if (EndsWith(base, " blood") || base == "scorpion venom") return Blood;
	if (EndsWith(base, " mending")) return Mending;
	if (StartsWith(base, "ancient: ") || EndsWith(base, " spear") || EndsWith(base, " shard")
		|| base == "trushar's frost" || base == "blizzard blast")
		return Lance;
	return -1;
}

} // namespace

const ClassCatalogue& BeastlordCatalogue()
{
	static const ClassCatalogue c{ "Beastlord", eqlib::Beastlord, kLines, 2, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
