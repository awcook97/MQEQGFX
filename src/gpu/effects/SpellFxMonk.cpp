#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

namespace eqgfx::spellfx {

namespace {

constexpr Pair kKi      { { 1.00f, 0.90f, 0.55f, 0.95f }, { 1.00f, 0.55f, 0.10f, 0.85f } };
constexpr Pair kSaffron { { 1.00f, 0.80f, 0.40f, 0.95f }, { 0.85f, 0.35f, 0.10f, 0.85f } };
constexpr Pair kChi     { { 0.70f, 1.00f, 0.90f, 0.90f }, { 0.15f, 0.70f, 0.60f, 0.80f } };
constexpr Pair kJade    { { 0.75f, 1.00f, 0.75f, 0.95f }, { 0.10f, 0.55f, 0.35f, 0.85f } };
constexpr Pair kTiger   { { 1.00f, 0.70f, 0.20f, 0.95f }, { 0.35f, 0.12f, 0.02f, 0.90f } };
constexpr Pair kCrane   { { 1.00f, 1.00f, 1.00f, 0.95f }, { 0.90f, 0.15f, 0.15f, 0.85f } };
constexpr Pair kDragon  { { 0.60f, 1.00f, 0.55f, 0.95f }, { 1.00f, 0.75f, 0.15f, 0.85f } };
constexpr Pair kVoid    { { 0.55f, 0.45f, 0.85f, 0.75f }, { 0.05f, 0.02f, 0.15f, 0.70f } };
constexpr Pair kAsh     { { 0.85f, 0.82f, 0.78f, 0.90f }, { 0.35f, 0.32f, 0.30f, 0.80f } };
constexpr Pair kCrimson { { 1.00f, 0.45f, 0.30f, 0.95f }, { 0.60f, 0.02f, 0.05f, 0.90f } };
constexpr Pair kLunar   { { 0.90f, 0.94f, 1.00f, 0.90f }, { 0.45f, 0.50f, 0.85f, 0.75f } };
constexpr Pair kStorm   { { 0.85f, 0.95f, 1.00f, 0.95f }, { 0.30f, 0.35f, 0.60f, 0.85f } };
constexpr Pair kWasp    { { 1.00f, 0.92f, 0.20f, 0.95f }, { 0.20f, 0.15f, 0.05f, 0.90f } };
constexpr Pair kHex     { { 0.85f, 0.45f, 1.00f, 0.90f }, { 0.30f, 0.05f, 0.40f, 0.85f } };

LOOK(kMnkFists, "fists",
	G(IM, Glyph_Claw, 0.9f, 6.0f, 4, 1, GA_PopRoll, 0, FIX, kTiger, 1.2f),
	G(IM, Glyph_Fist, 0.45f, 4.5f, 3, 3, GA_PopSpin, 1.0f, FIX, kKi, 1.4f),
	F(IM, FxForm_Burst, 1.6f, 4.0f, 48, 1, 0.2f, 10, 0, FIX, kKi, 0.4f, 0.08f),
	F(IM, FxForm_Burst, 1.6f, 4.8f, 48, 1, 0.2f, 10, 0, FIX, kTiger, 0.4f, 0.08f, ADD, 1, 0.15f),
	F(IM, FxForm_Burst, 1.6f, 5.6f, 48, 1, 0.2f, 10, 0, FIX, kKi, 0.4f, 0.08f, ADD, 1, 0.3f));

LOOK(kMnkSynergy, "synergy",
	F(IM, FxForm_Burst, 1.8f, 2.5f, 48, 1, 0.3f, 12, 0, FIX, kSaffron, 0.4f, 0.08f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0.3f, 12, 0, FIX, kSaffron, 0.4f, 0.08f, ADD, 1, 0.15f),
	F(IM, FxForm_Burst, 1.8f, 3.5f, 48, 1, 0.3f, 12, 0, FIX, kSaffron, 0.4f, 0.08f, ADD, 1, 0.3f),
	G(IM, Glyph_ArrowDown, 0.35f, 7.0f, 0, 3, GA_Orbit, 1.0f, FIX, kCrimson, 2.4f, 0.35f),
	F(IM, FxForm_Ring, 1.0f, 3.0f, 40, 0, 1.6f, 1.0f, 0.5f, FIX, kKi, 2.4f, 0.05f, COMET, 1, 0.35f));

LOOK(kMnkPhantom, "phantom fists",
	G(IM, Glyph_Fist, 0.5f, 4.5f, 3, 6, GA_Orbit, 1.4f, FIX, kGhost, 2.0f),
	F(IM, FxForm_Orbit, 1.4f, 5.0f, 72, 12, 0, 0, 0, FIX, kGhost, 2.0f, 0.05f, ADD, 3.0f),
	G(IM, Glyph_Spirit, 0.5f, 6.5f, 4, 2, GA_PopAscend | GlyphAnim_Spin, 1.0f, FIX, kLunar, 1.8f, 0.2f),
	F(IM, FxForm_Burst, 2.0f, 4.0f, 64, 2, 0.3f, 16, 0, FIX, kGhost, 0.8f, 0.07f, ADD, 1, 0.3f));

LOOK(kMnkFang, "fang",
	G(IM, Glyph_Fang, 1.0f, 8.5f, 4, 1, GA_Drop, 0, FIX, kDragon, 1.6f),
	F(IM, FxForm_Flame, 0.9f, 7.0f, 96, 24, 0.2f, 1.8f, 0, FIX, kDragon, 1.6f, 0.08f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 64, 2, 0.4f, 14, 0, FIX, kJade, 0.8f, 0.09f, ADD, 1, 0.35f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kGold, 1.6f, 0.06f, ADD, 1, 0.4f));

LOOK(kMnkSting, "sting",
	F(TR, FxForm_Bolt, 1, 0, 64, 2, 0.1f, 0.15f, 0.3f, FIX, kSilver, 0.5f, 0.10f),
	F(TR, FxForm_Bolt, 1, 0, 64, 2, 0.1f, 0.15f, 0.5f, FIX, kSilver, 0.5f, 0.10f, ADD, 1, 0.12f),
	F(TR, FxForm_Bolt, 1, 0, 64, 2, 0.1f, 0.15f, 0.7f, FIX, kSilver, 0.5f, 0.10f, ADD, 1, 0.24f),
	G(IM, Glyph_Insect, 0.7f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kWasp, 1.6f),
	G(IM, Glyph_Star, 0.3f, 4.5f, 3, 3, GA_PopRoll, 0.8f, FIX, kSilver, 1.0f));

LOOK(kMnkShuriken, "shuriken",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.25f, 0.2f, 1.2f, FIX, kSilver, 0.7f, 0.12f),
	G(IM, Glyph_Star, 0.7f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kSilver, 1.0f),
	F(IM, FxForm_Burst, 1.6f, 4.0f, 48, 1, 0.2f, 12, 0, FIX, kWhite, 0.5f, 0.07f));

LOOK(kMnkPrecision, "precision strike",
	F(TR, FxForm_Bolt, 1, 0, 64, 2, 0.1f, 0.3f, 0.4f, FIX, kKi, 0.5f, 0.10f),
	F(TR, FxForm_Bolt, 1, 0, 64, 2, 0.1f, 0.3f, 0.8f, FIX, kSilver, 0.5f, 0.10f, ADD, 1, 0.15f),
	G(IM, Glyph_Eye, 0.8f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kKi, 1.6f),
	F(IM, FxForm_Rune, 1.2f, 4.0f, 60, 4, 1, 16, 1, FIX, kKi, 1.4f, 0.05f),
	G(RE, Glyph_Star, 0.25f, 5.0f, 0, 4, GA_Orbit, 1.0f, FIX, kSilver, 2.0f));

LOOK(kMnkThrough, "through",
	G(IM, Glyph_Shield, 0.9f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kIron, 0.9f, 0, OVER),
	G(IM, Glyph_Fist, 0.9f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kKi, 1.2f, 0.25f),
	F(IM, FxForm_Spire, 0.4f, 4.5f, 64, 12, 1.5f, 2.0f, 0, FIX, kKi, 0.8f, 0.07f, ADD, 1, 0.25f),
	F(IM, FxForm_Crystal, 1.0f, 4.0f, 40, 8, 0, 0.5f, 0, FIX, kIron, 0.8f, 0.05f, ADD | FxFlag_Reverse, 1, 0.25f));

LOOK(kMnkEcho, "echo",
	F(RE, FxForm_Bloom, 1.0f, 5.0f, 40, 2, 0, 0, 0, FIX, kGhost, 1.2f, 0.04f),
	G(IM, Glyph_Spiral, 0.4f, 7.0f, 4, 3, GA_Orbit | GlyphAnim_Roll, 1.0f, FIX, kGhost, 2.4f),
	F(IM, FxForm_Bloom, 1.0f, 5.0f, 96, 4, 0, 0, 0, FIX, kLunar, 2.0f, 0.04f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Sparkle, 1.0f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kGhost, 2.0f));

LOOK(kMnkApparition, "apparition",
	G(IM, Glyph_Spirit, 1.0f, 6.0f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 2.4f),
	F(IM, FxForm_Flame, 0.9f, 6.5f, 72, 18, 0.2f, 1.2f, 0, FIX, kGhost, 2.2f, 0.05f),
	F(IM, FxForm_Sparkle, 1.4f, 7.0f, 40, 20, 0.12f, 2, 0, FIX, kLunar, 2.0f),
	G(IM, Glyph_Mask, 0.5f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kLunar, 2.0f, 0.3f));

LOOK(kMnkCurse, "curse",
	G(IM, Glyph_Doll, 0.9f, 8.0f, 4, 1, GA_Drop, 0, FIX, kHex, 2.6f),
	G(IM, Glyph_Hand, 0.35f, 4.0f, 3, 5, GA_Orbit, 1.2f, FIX, kHex, 3.0f),
	F(IM, FxForm_Rune, 1.6f, 0, 70, 5, 2, 13, 0, FIX, kHex, 3.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.12f, 1, 0, FIX, kShadow, 2.4f));

LOOK(kMnkAlliance, "alliance",
	G(IM, Glyph_Link, 0.35f, 5.0f, 0, 6, GA_Orbit, 1.3f, FIX, kKi, 3.0f),
	F(IM, FxForm_Rune, 1.8f, 0, 70, 6, 1, 12, 0, FIX, kSaffron, 3.0f, 0.05f),
	F(IM, FxForm_Drain, 1, 0, 48, 3, 0.8f, 16, 0, FIX, kKi, 1.8f, 0.07f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 12, 0, FIX, kKi, 1.4f, 0.08f, ADD, 1, 0.2f, AE));

LOOK(kMnkTechnique, "technique",
	G(RE, Glyph_Fist, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kCrimson, 2.4f),
	G(RE, Glyph_Moon, 0.4f, 5.0f, 0, 2, GA_Orbit, 1.1f, FIX, kLunar, 2.4f),
	F(RE, FxForm_Helix, 1.0f, 6.0f, 72, 3, 1.4f, 0, 0.5f, FIX, kCrimson, 2.4f, 0.05f, COMET),
	F(RE, FxForm_Drain, 1, 0, 48, 3, 1.0f, 12, 0, FIX, kCrimson, 1.6f, 0.07f, ADD | FxFlag_Reverse));

LOOK(kMnkPoise, "poise",
	G(IM, Glyph_Paw, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kTiger, 2.8f),
	F(IM, FxForm_Rune, 1.6f, 0, 70, 8, 3, 8, 0, FIX, kTiger, 2.8f, 0.05f),
	G(IM, Glyph_Claw, 0.4f, 4.0f, 3, 3, GA_Orbit, 1.2f, FIX, kKi, 2.8f),
	F(IM, FxForm_Helix, 1.1f, 5.5f, 64, 2, 2.0f, 0, 0.8f, FIX, kDragon, 2.2f, 0.05f, COMET));

LOOK(kMnkCrane, "crane stance",
	G(IM, Glyph_Dove, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kCrane, 2.2f),
	F(IM, FxForm_Wings, 1.0f, 4.4f, 70, 10, 0.8f, 4.0f, 0, FIX, kCrane, 2.2f, 0.06f),
	F(IM, FxForm_Burst, 2.2f, 3.5f, 64, 2, 0.4f, 14, 0, FIX, kCrane, 0.6f, 0.09f, ADD, 1, 0.5f),
	F(IM, FxForm_Burst, 2.4f, 4.0f, 64, 2, 0.4f, 14, 0, FIX, kWhite, 0.6f, 0.09f, ADD, 1, 0.8f));

LOOK(kMnkEarthforce, "earthforce",
	G(IM, Glyph_Rock, 1.0f, 7.5f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kStone, 2.8f, 0, OVER),
	F(IM, FxForm_Crystal, 1.3f, 3.0f, 60, 12, 1, 0.7f, 0, FIX, kStone, 2.8f, 0.07f, OVER),
	F(IM, FxForm_Cracks, 2.4f, 0, 48, 6, 0.8f, 0, 0, FIX, kEarth, 2.6f, 0.06f, OVER),
	F(IM, FxForm_Ring, 1.3f, 0.3f, 40, 0, 0.5f, 1.0f, 0.3f, FIX, kKi, 2.6f, 0.05f, COMET));

LOOK(kMnkHeel, "heel",
	F(IM, FxForm_Ring, 0.9f, 0.8f, 40, 0, 3.0f, 1.0f, 0.8f, FIX, kKi, 3.0f, 0.06f, COMET),
	F(IM, FxForm_Ring, 1.1f, 1.4f, 40, 0, -3.0f, 1.0f, 0.8f, FIX, kSaffron, 3.0f, 0.05f, COMET),
	G(IM, Glyph_Tornado, 0.5f, 1.2f, 3, 2, GA_Orbit, 1.2f, FIX, kKi, 3.0f),
	F(IM, FxForm_Sparkle, 1.2f, 3.0f, 32, 16, 0.10f, 0, 0, FIX, kKi, 2.4f));

LOOK(kMnkForm, "form",
	G(IM, Glyph_Sun, 0.5f, 5.5f, 4, 1, GA_Orbit, 1.2f, FIX, kKi, 3.0f),
	G(IM, Glyph_Moon, 0.5f, 5.5f, 4, 1, GA_Orbit, 1.2f, FIX, kLunar, 3.0f, 0.5f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 72, 2, 1.5f, 0, 0.5f, FIX, kKi, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 72, 2, -1.5f, 0, 0.5f, FIX, kLunar, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Rune, 1.5f, 0, 60, 3, 1, 16, 0, FIX, kWhite, 3.0f, 0.04f));

LOOK(kMnkDrunken, "drunken monkey",
	G(IM, Glyph_Chalice, 0.8f, 7.8f, 4, 1, GA_Roll, 0, FIX, kAmber, 2.8f),
	G(IM, Glyph_Spiral, 0.35f, 5.0f, 0, 3, GA_Orbit | GlyphAnim_Roll, 1.2f, FIX, kSaffron, 2.8f),
	F(IM, FxForm_Orbit, 1.3f, 5.5f, 60, 12, 0, 0, 0, FIX, kAmber, 2.8f, 0.05f, ADD, 1.5f),
	F(IM, FxForm_Bubbles, 0.8f, 6.0f, 48, 8, 0.15f, 0, 0, FIX, kAmber, 2.4f, 0.035f));

LOOK(kMnkPalm, "palm",
	G(IM, Glyph_Hand, 1.0f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kCold, 2.8f),
	F(IM, FxForm_Crystal, 1.0f, 5.0f, 60, 12, 0, 0.6f, 0, FIX, kCold, 2.6f, 0.05f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 40, 20, 0.10f, 0, 0, FIX, kWhite, 2.6f),
	F(IM, FxForm_Burst, 2.0f, 4.0f, 48, 1, 0.3f, 12, 0, FIX, kCold, 0.8f, 0.07f));

LOOK(kMnkIronfist, "ironfist",
	G(IM, Glyph_Fist, 1.0f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kIron, 2.4f),
	F(IM, FxForm_Lattice, 1.1f, 5.5f, 64, 8, 0.4f, 0, 0, FIX, kIron, 2.4f, 0.05f),
	F(IM, FxForm_Sparkle, 1.4f, 5.0f, 48, 24, 0.10f, 0, 0, FIX, kForge, 2.2f),
	F(IM, FxForm_Burst, 2.2f, 4.0f, 48, 1, 0.3f, 12, 0, FIX, kIron, 0.8f, 0.08f, ADD, 1, 0.3f));

LOOK(kMnkEyeStorm, "eye of the storm",
	F(IM, FxForm_Vortex, 1.6f, 7.0f, 96, 5, 1.4f, 2.5f, 4.0f, FIX, kStorm, 3.0f, 0.06f),
	G(IM, Glyph_Eye, 0.8f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kStorm, 3.0f),
	F(IM, FxForm_Lightning, 1, 30.0f, 48, 1, 0.4f, 8.0f, 1.0f, FIX, kShock, 0.8f, 0.10f, ADD, 1, 0.3f),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 60, 10, 2.5f, 0, 0, FIX, kStorm, 2.4f, 0.04f));

LOOK(kMnkReflexes, "reflexes",
	G(IM, Glyph_Hand, 0.5f, 5.0f, 3, 4, GA_Orbit, 1.3f, FIX, kKi, 2.8f),
	F(IM, FxForm_Ring, 1.3f, 5.0f, 48, 0, 2.5f, 1.0f, 0.8f, FIX, kKi, 2.8f, 0.05f, COMET),
	F(IM, FxForm_Ring, 1.3f, 3.0f, 48, 0, -2.5f, 1.0f, 0.8f, FIX, kSaffron, 2.8f, 0.05f, COMET),
	F(IM, FxForm_Burst, 1.6f, 4.0f, 48, 1, 0, 10, 0, FIX, kWhite, 0.6f, 0.06f, ADD | FxFlag_Reverse));

LOOK(kMnkFlinch, "flinch",
	G(IM, Glyph_Shield, 0.9f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kIron, 1.4f),
	G(IM, Glyph_ArrowDown, 0.5f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kGrey, 1.8f, 1.2f, OVER),
	F(IM, FxForm_Dome, 1.5f, 6.0f, 72, 6, 2, 0, 0, FIX, kIron, 1.4f, 0.03f),
	F(IM, FxForm_Burst, 1.8f, 4.0f, 48, 1, 0, 10, 0, FIX, kSilver, 0.6f, 0.07f));

LOOK(kMnkCounter, "counter",
	G(IM, Glyph_Hourglass, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kWater, 3.0f),
	G(IM, Glyph_Hand, 0.4f, 4.5f, 3, 3, GA_Orbit, 1.2f, FIX, kChi, 3.0f),
	F(IM, FxForm_Vortex, 1.3f, 6.0f, 72, 4, 1.2f, 1.5f, -2.0f, FIX, kWater, 3.0f, 0.05f),
	F(IM, FxForm_Burst, 1.8f, 4.0f, 48, 1, 0, 10, 0, FIX, kChi, 0.8f, 0.06f, ADD | FxFlag_Reverse));

LOOK(kMnkVoidBody, "void body",
	F(IM, FxForm_Flame, 1.0f, 6.5f, 80, 20, 0.9f, 0.9f, 0, FIX, kVoid, 3.0f, 0.06f, OVER),
	F(IM, FxForm_Dome, 1.6f, 6.5f, 88, 6, 2, 0, 0, FIX, kVoid, 2.6f, 0.03f, ADD | FxFlag_Reverse),
	G(IM, Glyph_Moon, 0.6f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kLunar, 2.4f),
	F(IM, FxForm_Sparkle, 1.4f, 6.0f, 40, 20, 0.10f, 2, 0, FIX, kVoid, 2.4f));

LOOK(kMnkVoidStep, "void step",
	G(IM, Glyph_Door, 1.1f, 6.0f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kVoid, 2.2f),
	F(IM, FxForm_Wind, 1.3f, 4.5f, 60, 10, 3.0f, 0, 0, FIX, kVoid, 2.0f, 0.05f),
	F(IM, FxForm_Bloom, 1.5f, 0.5f, 40, 2, 0, 0, 0, FIX, kShadow, 2.0f, 0.05f));

LOOK(kMnkDeath, "death",
	G(IM, Glyph_Ankh, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kJade, 3.0f),
	G(IM, Glyph_Hourglass, 0.35f, 4.5f, 0, 3, GA_Orbit, 1.2f, FIX, kAmber, 3.0f),
	F(IM, FxForm_Dome, 1.7f, 7.0f, 96, 8, 3, 0, 0, FIX, kJade, 2.6f, 0.04f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 72, 3, 1.0f, 0, 0.4f, FIX, kHeal, 3.0f, 0.05f, COMET));

LOOK(kMnkProtection, "protection",
	G(IM, Glyph_Hand, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kJade, 2.8f),
	G(IM, Glyph_Leaf, 0.3f, 4.5f, 0, 4, GA_Orbit, 1.3f, FIX, kJade, 2.8f),
	F(IM, FxForm_Ring, 1.3f, 0.3f, 48, 0, 1.0f, 1.0f, 0.5f, FIX, kJade, 2.8f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.10f, 1, 0, FIX, kHeal, 2.4f));

LOOK(kMnkProjection, "projection",
	G(RE, Glyph_Spirit, 1.1f, 6.0f, 4, 1, GA_PopAscend, 0, FIX, kLunar, 3.5f),
	F(RE, FxForm_Flame, 0.9f, 7.0f, 72, 18, 0.2f, 1.4f, 0, FIX, kGhost, 3.5f, 0.05f),
	F(RE, FxForm_Rune, 2.2f, 0, 70, 8, 3, 16, 0, FIX, kLunar, 3.5f, 0.05f),
	F(RE, FxForm_Sparkle, 1.4f, 10.0f, 48, 24, 0.12f, 1, 0, FIX, kStar, 3.0f));

LOOK(kMnkInnerflame, "innerflame",
	F(IM, FxForm_Flame, 1.0f, 6.0f, 96, 24, 0.8f, 1.0f, 0, FIX, kKi, 3.0f, 0.08f),
	G(IM, Glyph_Flame, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kSaffron, 2.6f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 64, 2, 0.8f, 14, 0, FIX, kKi, 0.9f, 0.09f));

LOOK(kMnkBreaths, "breaths",
	F(RE, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kChi, 1.4f, 0.05f, ADD | FxFlag_Reverse),
	F(RE, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kChi, 1.4f, 0.05f, ADD, 1, 1.4f),
	G(RE, Glyph_Leaf, 0.7f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kJade, 2.8f),
	F(RE, FxForm_Helix, 1.0f, 5.5f, 64, 2, 0.6f, 0, 0.3f, FIX, kChi, 2.8f, 0.05f, COMET));

LOOK(kMnkWinds, "wind",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 12, 2.5f, 0, 0, FIX, kAir, 2.4f, 0.05f),
	G(IM, Glyph_Tornado, 0.8f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kAir, 2.2f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 72, 3, 1.6f, 0, 0.6f, FIX, kChi, 2.4f, 0.05f, COMET));

LOOK(kMnkBreather, "rest",
	G(IM, Glyph_Moon, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kLunar, 3.0f),
	F(IM, FxForm_Bloom, 1.4f, 0.5f, 40, 2, 0, 0, 0, FIX, kLunar, 3.0f, 0.04f, ADD, 0.5f),
	F(IM, FxForm_Sparkle, 1.2f, 8.0f, 32, 16, 0.10f, 1, 0, FIX, kStar, 3.0f));

LOOK(kMnkRejuvenation, "inner rejuvenation",
	G(IM, Glyph_Leaf, 0.35f, 4.5f, 0, 3, GA_Orbit, 1.2f, FIX, kJade, 3.0f),
	F(IM, FxForm_Flame, 0.8f, 7.0f, 72, 18, 0.1f, 2.2f, 0, FIX, kJade, 2.2f, 0.06f),
	F(IM, FxForm_Bloom, 1.3f, 0.5f, 40, 2, 0, 0, 0, FIX, kChi, 2.2f, 0.04f));

LOOK(kMnkFocusedWill, "focused will",
	G(IM, Glyph_Hand, 0.5f, 5.0f, 3, 2, GA_PopBob, 0.6f, FIX, kJade, 3.0f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 72, 3, 1.0f, 0, 0.4f, FIX, kHeal, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.1f, 7.0f, 32, 16, 0.10f, 2, 0, FIX, kJade, 2.6f));

LOOK(kMnkResistant, "resistant",
	G(IM, Glyph_Shield, 0.9f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kPrism, 2.6f),
	F(IM, FxForm_Dome, 1.7f, 6.8f, 96, 8, 3, 0, 0, FIX, kPrism, 2.4f, 0.03f));

LOOK(kMnkFearless, "fearless",
	G(IM, Glyph_Heart, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kCrimson, 2.6f),
	F(IM, FxForm_Spire, 0.8f, 7.0f, 64, 12, 0.8f, 2.4f, 0, FIX, kSaffron, 2.0f, 0.06f));

LOOK(kMnkThunderkick, "thunderkick",
	F(IM, FxForm_Lightning, 1, 36.0f, 48, 2, 0.5f, 10.0f, 1.0f, FIX, kShock, 0.8f, 0.12f),
	G(IM, Glyph_Lightning, 0.9f, 7.0f, 4, 1, GA_Drop, 0, FIX, kStorm, 1.4f),
	F(IM, FxForm_Burst, 2.4f, 2.5f, 64, 2, 0.4f, 16, 0, FIX, kShock, 0.8f, 0.10f, ADD, 1, 0.2f));

LOOK(kMnkWhirlwind, "whirlwind",
	F(IM, FxForm_Vortex, 1.4f, 6.0f, 96, 5, 1.2f, 2.0f, 6.0f, FIX, kAir, 3.0f, 0.05f),
	G(IM, Glyph_Hand, 0.4f, 4.0f, 3, 4, GA_Orbit, 1.2f, FIX, kKi, 3.0f));

LOOK(kMnkVoiddance, "voiddance",
	G(IM, Glyph_Moon, 0.35f, 4.5f, 0, 4, GA_Orbit | GlyphAnim_Roll, 1.3f, FIX, kVoid, 3.0f),
	F(IM, FxForm_Orbit, 1.3f, 5.0f, 60, 12, 0, 0, 0, FIX, kVoid, 3.0f, 0.05f, ADD, 2.5f),
	F(IM, FxForm_Wind, 1.2f, 4.0f, 48, 8, 2.0f, 0, 0, FIX, kLunar, 2.4f, 0.04f));

LOOK(kMnkAura, "aura",
	Persist(F(AU, FxForm_Rune, 1.0f, 0, 80, 8, 3, 16, 0, FIX, kSaffron, 0, 0.04f, ADD, 0.5f, 0, AE)),
	G(RE, Glyph_Feather, 0.35f, 5.0f, 0, 5, GA_Orbit, 1.6f, FIX, kFeather, 3.0f),
	F(RE, FxForm_Bloom, 1.0f, 0.5f, 64, 3, 0, 0, 0, FIX, kKi, 2.0f, 0.05f, ADD, 1, 0, AE));

LOOK(kMnkHundredFists, "hundred fists",
	G(IM, Glyph_Fist, 0.45f, 4.5f, 3, 8, GA_Orbit, 1.4f, FIX, kKi, 3.0f),
	F(IM, FxForm_Orbit, 1.3f, 4.5f, 80, 16, 0, 0, 0, FIX, kSaffron, 3.0f, 0.05f, ADD, 4.0f),
	F(IM, FxForm_Burst, 2.0f, 4.0f, 64, 2, 0.3f, 16, 0, FIX, kKi, 0.8f, 0.08f));

LOOK(kMnkSilentfist, "silentfist",
	G(IM, Glyph_Fist, 1.2f, 8.5f, 4, 1, GA_Drop, 0, FIX, kDragon, 1.6f),
	F(IM, FxForm_Flame, 0.9f, 6.0f, 80, 20, 0.2f, 1.6f, 0, FIX, kDragon, 1.4f, 0.07f, ADD, 1, 0.35f),
	F(IM, FxForm_Bloom, 1.8f, 4.0f, 40, 2, 0, 0, 0, FIX, kJade, 1.2f, 0.05f, ADD, 1, 0.35f));

LOOK(kMnkAshenhand, "ashenhand",
	G(IM, Glyph_Hand, 1.2f, 8.5f, 4, 1, GA_Drop, 0, FIX, kAsh, 1.8f, 0, OVER),
	F(IM, FxForm_Burst, 2.4f, 3.0f, 64, 2, 0.4f, 16, 0, FIX, kFire, 0.8f, 0.09f, ADD, 1, 0.35f),
	F(IM, FxForm_Rain, 1.0f, 8.0f, 40, 30, 0, 0.1f, 0, FIX, kAsh, 2.4f, 0.04f, OVER, 0.5f, 0.35f));

LOOK(kMnkPlaneswalk, "planeswalk",
	G(IM, Glyph_Door, 0.8f, 7.5f, 4, 1, GA_PopAscend, 0, FIX, kArcane, 2.2f),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 12, 3.5f, 0, 0, FIX, kArcane, 2.4f, 0.05f));

LOOK(kMnkSpeedFocus, "speed focus",
	G(IM, Glyph_Hourglass, 0.8f, 7.6f, 4, 1, GA_Roll, 0, FIX, kKi, 2.4f),
	G(IM, Glyph_Fist, 0.35f, 4.5f, 3, 2, GA_Orbit, 0.9f, FIX, kSaffron, 2.4f),
	F(IM, FxForm_Ring, 1.0f, 4.5f, 40, 0, 4.0f, 1.0f, 0.8f, FIX, kKi, 2.4f, 0.05f, COMET));

LOOK(kMnkDreamwalk, "dreamwalk",
	G(IM, Glyph_Z, 0.35f, 6.0f, 0, 3, GA_Orbit | GlyphAnim_Ascend, 1.1f, FIX, kIllusion, 2.6f),
	F(IM, FxForm_Dome, 1.7f, 6.8f, 96, 8, 3, 0, 0, FIX, kIllusion, 2.4f, 0.03f),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 60, 10, 3.0f, 0, 0, FIX, kIllusion, 2.2f, 0.05f));

LOOK(kMnkRapidKick, "rapid kick",
	F(IM, FxForm_Burst, 1.4f, 1.5f, 32, 1, 0.1f, 8, 0, FIX, kKi, 0.35f, 0.07f),
	F(IM, FxForm_Burst, 1.4f, 2.0f, 32, 1, 0.1f, 8, 0, FIX, kKi, 0.35f, 0.07f, ADD, 1, 0.12f),
	F(IM, FxForm_Burst, 1.4f, 2.5f, 32, 1, 0.1f, 8, 0, FIX, kKi, 0.35f, 0.07f, ADD, 1, 0.24f),
	F(IM, FxForm_Ring, 1.0f, 1.0f, 40, 0, 5.0f, 1.0f, 0.9f, FIX, kSaffron, 2.4f, 0.05f, COMET));

LOOK(kMnkFistsOfWu, "fists of wu",
	G(RE, Glyph_Fist, 0.6f, 5.0f, 3, 6, GA_RingRise, 2.4f, FIX, kKi, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kSaffron, 1.4f, 0.09f, ADD, 1, 0, AE),
	F(IM, FxForm_Ring, 0.9f, 4.5f, 40, 0, 2.0f, 1.0f, 0.6f, FIX, kKi, 2.4f, 0.05f, COMET));

LOOK(kAaImitateDeath, "imitate death",
	G(IM, Glyph_Skeleton, 1.1f, 0.1f, 2, 1, GlyphAnim_Pop, 0, FIX, kBone, 3.0f, 0, OVER),
	G(IM, Glyph_Spirit, 0.8f, 4.0f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 3.0f, 0.3f),
	F(IM, FxForm_Bloom, 1.6f, 0.3f, 40, 2, 0, 0, 0, FIX, kGhost, 2.4f, 0.04f));

LOOK(kAaFivePointPalm, "five point palm",
	G(IM, Glyph_Hand, 1.2f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kCrimson, 1.4f),
	F(IM, FxForm_Rune, 1.4f, 4.5f, 70, 5, 2, 5, 1, FIX, kCrimson, 1.8f, 0.06f, ADD, 1, 0.3f),
	G(IM, Glyph_Heart, 0.9f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kBlood, 1.2f, 0.8f),
	F(IM, FxForm_Burst, 2.8f, 4.0f, 96, 3, 0.4f, 5, 0, FIX, kBlood, 1.0f, 0.12f, ADD, 1, 0.9f));

LOOK(kAaWaspTouch, "two-finger wasp touch",
	G(IM, Glyph_Insect, 0.9f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kWasp, 2.4f),
	G(IM, Glyph_ArrowDown, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.0f, FIX, kWasp, 2.4f),
	F(IM, FxForm_Burst, 1.4f, 4.5f, 32, 1, 0.1f, 2, 0, FIX, kWasp, 0.6f, 0.07f));

LOOK(kAaZanFi, "zan fi's whistle",
	F(IM, FxForm_Bloom, 1.2f, 6.0f, 64, 3, 0, 0, 0, FIX, kAir, 1.6f, 0.04f),
	G(IM, Glyph_Fist, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kSaffron, 3.0f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 96, 24, 0.9f, 0.8f, 0, FIX, kSaffron, 3.0f, 0.07f),
	F(IM, FxForm_Wind, 1.2f, 4.5f, 60, 10, 2.0f, 0, 0, FIX, kAir, 2.4f, 0.04f));

LOOK(kAaZanFiEcho, "zan fi's echo",
	G(IM, Glyph_Fist, 0.6f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kSaffron, 0.8f),
	F(IM, FxForm_Bloom, 1.2f, 4.5f, 32, 2, 0, 0, 0, FIX, kAir, 0.8f, 0.04f));

LOOK(kAaInfusionThunder, "infusion of thunder",
	F(IM, FxForm_Lightning, 1, 38.0f, 64, 2, 0.6f, 12.0f, 1.0f, FIX, kShock, 0.8f, 0.12f),
	F(IM, FxForm_Lightning, 1, 38.0f, 64, 2, 0.7f, 12.0f, 1.0f, FIX, kShock, 0.7f, 0.10f, ADD, 1, 0.4f),
	G(IM, Glyph_Lightning, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kStorm, 3.0f),
	F(IM, FxForm_Sparkle, 1.3f, 6.0f, 48, 24, 0.12f, 0, 0, FIX, kShock, 3.0f));

LOOK(kAaThunderShock, "thunder shock",
	F(IM, FxForm_Lightning, 1, 20.0f, 32, 1, 0.5f, 10.0f, 1.0f, FIX, kShock, 0.5f, 0.08f),
	F(IM, FxForm_Sparkle, 0.9f, 5.0f, 20, 10, 0.10f, 0, 0, FIX, kShock, 0.8f));

LOOK(kAaDestructiveForce, "destructive force",
	G(RE, Glyph_Fist, 0.7f, 5.0f, 3, 6, GA_RingRise, 2.6f, FIX, kKi, 3.0f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 120, 3, 0.6f, 24, 0, FIX, kKi, 1.4f, 0.12f, ADD, 1, 0.2f, AE),
	F(RE, FxForm_Cracks, 1.0f, 0, 64, 8, 1.0f, 0, 0, FIX, kEarth, 2.4f, 0.06f, OVER, 1, 0.2f, AE),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 96, 24, 0.9f, 0.9f, 0, FIX, kKi, 3.0f, 0.07f));

LOOK(kAaFocusedForce, "focused destructive force",
	G(IM, Glyph_Fist, 1.1f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kKi, 3.0f),
	F(IM, FxForm_Spire, 0.4f, 7.5f, 96, 16, 0.5f, 3.0f, 0, FIX, kKi, 2.4f, 0.07f),
	F(IM, FxForm_Ring, 0.8f, 5.0f, 40, 0, 3.0f, 1.0f, 0.8f, FIX, kSaffron, 3.0f, 0.05f, COMET));

LOOK(kAaDragonForce, "dragon force",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.4f, 0.6f, FIX, kDragon, 0.6f, 0.16f),
	G(IM, Glyph_Fang, 0.9f, 6.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kDragon, 1.4f),
	F(IM, FxForm_Burst, 2.4f, 3.0f, 64, 2, 0.4f, 16, 0, FIX, kJade, 0.8f, 0.10f),
	F(IM, FxForm_Wind, 1.3f, 4.5f, 60, 10, 3.0f, 0, 0, FIX, kDragon, 1.2f, 0.05f));

LOOK(kAaEyeGouge, "eye gouge",
	G(IM, Glyph_Eye, 0.9f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kWhite, 1.6f),
	G(IM, Glyph_Claw, 0.9f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kBlood, 1.2f, 0.2f),
	F(IM, FxForm_Burst, 1.6f, 6.5f, 32, 1, 0, 10, 0, FIX, kBlood, 0.6f, 0.07f, ADD, 1, 0.2f));

LOOK(kAaGrappling, "grappling strike",
	G(IM, Glyph_Hand, 0.9f, 6.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kKi, 1.4f),
	F(IM, FxForm_Drain, 1, 0, 48, 3, 0.6f, 12, 0, FIX, kKi, 1.2f, 0.08f),
	F(IM, FxForm_Burst, 1.8f, 3.5f, 48, 1, 0.2f, 12, 0, FIX, kSaffron, 0.6f, 0.08f));

LOOK(kAaMovingMountains, "moving mountains",
	G(IM, Glyph_Rock, 1.0f, 6.0f, 4, 1, GA_PopRoll, 0, FIX, kStone, 1.8f, 0, OVER),
	F(IM, FxForm_Drain, 1, 0, 64, 4, 1.2f, 16, 0, FIX, kEarth, 1.6f, 0.08f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Cracks, 1.8f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 1.8f, 0.06f, OVER));

LOOK(kAaNeshikaBlink, "neshika's blink",
	F(RE, FxForm_Sparkle, 1.4f, 6.0f, 48, 24, 0.12f, 2, 0, FIX, kChi, 1.0f),
	F(RE, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0, 12, 0, FIX, kChi, 0.6f, 0.07f, ADD | FxFlag_Reverse),
	F(RE, FxForm_Wind, 1.3f, 4.5f, 60, 10, 4.0f, 0, 0, FIX, kChi, 1.0f, 0.05f, ADD, 1, 0.4f));

LOOK(kAaTonPo, "ton po's stance",
	G(IM, Glyph_Fist, 1.0f, 7.8f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kSaffron, 3.0f),
	F(IM, FxForm_Rune, 1.8f, 0, 70, 8, 3, 24, 0, FIX, kSaffron, 3.0f, 0.05f),
	F(IM, FxForm_Lattice, 1.2f, 5.5f, 64, 8, 0.5f, 0, 0, FIX, kKi, 2.6f, 0.05f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 0.8f, 0, 0, FIX, kEarth, 2.4f, 0.06f, OVER));

LOOK(kAaSpireSensei, "spire of the sensei",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kSaffron, 2.6f, 0.10f),
	G(RE, Glyph_Book, 1.1f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kKi, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kKi, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaSenseiGuard, "sensei's guard",
	G(IM, Glyph_Hand, 0.7f, 6.0f, 4, 1, GA_PopAscend, 0, FIX, kSaffron, 1.4f),
	F(IM, FxForm_Dome, 1.5f, 6.5f, 72, 6, 2, 0, 0, FIX, kSaffron, 1.2f, 0.03f));

LOOK(kAaSwiftTails, "swift tails' chant",
	G(IM, Glyph_Spirit, 0.4f, 4.5f, 3, 3, GA_Orbit, 1.3f, FIX, kChi, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 72, 3, 2.0f, 0, 0.6f, FIX, kChi, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Wind, 1.2f, 4.5f, 60, 10, 2.0f, 0, 0, FIX, kAir, 2.4f, 0.04f));

LOOK(kAaDistantStrike, "distant strike",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.3f, 0.5f, 0.4f, FIX, kKi, 0.7f, 0.14f),
	G(IM, Glyph_Fist, 0.8f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kKi, 1.0f),
	F(IM, FxForm_Burst, 1.8f, 4.0f, 48, 1, 0.2f, 12, 0, FIX, kSaffron, 0.6f, 0.08f));

LOOK(kAaMagnanimous, "magnanimous force",
	G(IM, Glyph_Hand, 1.2f, 6.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kWhite, 1.4f),
	F(IM, FxForm_Burst, 2.8f, 3.0f, 96, 3, 0.4f, 20, 0, FIX, kWhite, 0.9f, 0.10f),
	G(IM, Glyph_Spiral, 0.4f, 7.0f, 0, 3, GA_Orbit | GlyphAnim_Roll, 1.0f, FIX, kGhost, 2.2f, 0.3f));

LOOK(kAaIntimidation, "ascendant's intimidation",
	G(RE, Glyph_Shackle, 0.8f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kCrimson, 2.0f),
	F(RE, FxForm_Bloom, 1.0f, 3.0f, 96, 3, 0, 0, 0, FIX, kCrimson, 1.8f, 0.05f, ADD, 1, 0, AE),
	F(IM, FxForm_Ring, 1.0f, 0.6f, 40, 0, 0.3f, 1.0f, 0, FIX, kCrimson, 3.0f, 0.04f));

LOOK(kAaStunningKick, "stunning kick",
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0.3f, 12, 0, FIX, kKi, 0.6f, 0.08f),
	Stars(0.1f));

LOOK(kAaResonantKick, "resonant kick",
	F(IM, FxForm_Bloom, 1.0f, 3.0f, 48, 3, 0, 0, 0, FIX, kAir, 1.0f, 0.04f),
	F(IM, FxForm_Burst, 1.6f, 3.0f, 32, 1, 0.2f, 8, 0, FIX, kKi, 0.5f, 0.07f));

LOOK(kAaThunderfoot, "thunderfoot",
	F(IM, FxForm_Cracks, 1.6f, 0, 48, 6, 1.2f, 0, 0, FIX, kShock, 1.4f, 0.05f),
	F(IM, FxForm_Lightning, 1, 24.0f, 32, 1, 0.5f, 10.0f, 1.0f, FIX, kShock, 0.5f, 0.08f),
	F(IM, FxForm_Burst, 1.8f, 0.5f, 48, 1, 0.2f, 12, 0, FIX, kStorm, 0.6f, 0.08f));

LOOK(kAaSweepingKick, "sweeping kick",
	F(IM, FxForm_Ring, 1.3f, 0.6f, 40, 0, 4.0f, 1.0f, 0.9f, FIX, kKi, 0.8f, 0.06f, COMET),
	G(IM, Glyph_Link, 0.3f, 0.6f, 3, 4, GA_PopSpin, 1.0f, FIX, kIron, 2.4f));

LOOK(kAaFistsOfFury, "fists of fury",
	G(IM, Glyph_Fist, 0.5f, 4.5f, 3, 2, GA_Orbit, 0.9f, FIX, kFlame2, 3.0f),
	F(IM, FxForm_Flame, 0.9f, 5.0f, 80, 20, 0.8f, 0.8f, 0, FIX, kFlame2, 3.0f, 0.07f));

LOOK(kAaMastersSynergy, "master's synergy",
	G(IM, Glyph_ArrowUp, 0.4f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kSaffron, 1.4f),
	F(IM, FxForm_Ring, 0.9f, 4.0f, 32, 0, 2.0f, 1.0f, 0.8f, FIX, kSaffron, 1.2f, 0.04f, COMET));

LOOK(kAaPurifiedProtection, "purified protection",
	G(IM, Glyph_Drop, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kWater, 3.0f),
	F(IM, FxForm_Dome, 1.7f, 6.8f, 96, 8, 3, 0, 0, FIX, kWhite, 2.6f, 0.03f));

LOOK(kAaPurifyBody, "purify body",
	G(IM, Glyph_Leaf, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kJade, 2.4f),
	F(IM, FxForm_Rain, 0.8f, 9.0f, 32, 24, 0, 0.1f, 0, FIX, kChi, 1.8f, 0.05f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 64, 2, 1.5f, 12, 0, FIX, kWhite, 1.0f, 0.06f, ADD, 1, 0.3f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kJade, 2.0f, 0.06f, ADD, 1, 0.3f));

LOOK(kAaVehementRage, "vehement rage",
	F(IM, FxForm_Spire, 0.8f, 7.0f, 96, 16, 0.8f, 2.8f, 0, FIX, kBlood, 2.4f, 0.07f),
	G(IM, Glyph_ArrowUp, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.1f, FIX, kCrimson, 2.6f),
	F(IM, FxForm_Burst, 2.4f, 0.3f, 64, 2, 0.3f, 14, 0, FIX, kBlood, 0.9f, 0.10f));

enum Line : int
{
	Fists, Synergy, Phantom, Fang, Sting, Shuriken, Precision, Through, Echo, Apparition, Curse,
	Alliance, Technique, Poise, Crane, Earthforce, Heel, Form, Drunken, Palm, Ironfist, EyeStorm,
	Reflexes, Flinch, Counter, VoidBody, VoidStep, Death, Protection, Projection, Innerflame, Breaths, Winds,
	Breather, Rejuvenation, FocusedWill, Resistant, Fearless, Thunderkick, Whirlwind, Voiddance, Aura,
	HundredFists, Silentfist, Ashenhand, Planeswalk, SpeedFocus, Dreamwalk, RapidKick, FistsOfWu,
	ImitateDeath, FivePointPalm, WaspTouch, ZanFi, ZanFiEcho, InfusionThunder, ThunderShock,
	DestructiveForce, FocusedForce, DragonForce, EyeGouge, Grappling, MovingMountains, NeshikaBlink,
	TonPo, SpireSensei, SenseiGuard, SwiftTails, DistantStrike, Magnanimous, Intimidation,
	StunningKick, ResonantKick, Thunderfoot, SweepingKick, FistsOfFury, MastersSynergy,
	PurifiedProtection, PurifyBody, VehementRage,
};

const LineLook kLines[] = {
	{ "Fists (tiger claw strikes)",     &kMnkFists },
	{ "Synergy (flying kicks)",         &kMnkSynergy },
	{ "Phantom (phantasmal fists)",     &kMnkPhantom },
	{ "Fang (dragon strike)",           &kMnkFang },
	{ "Sting (shuriken volley)",        &kMnkSting },
	{ "Vigorous Shuriken",              &kMnkShuriken },
	{ "Precision Strike (throwing)",    &kMnkPrecision },
	{ "Through (armor pierce)",         &kMnkThrough },
	{ "Echo (confuse)",                 &kMnkEcho },
	{ "Apparition (distract)",          &kMnkApparition },
	{ "Curse (damage shield debuff)",   &kMnkCurse },
	{ "Alliance / Covenant",            &kMnkAlliance },
	{ "Technique (endurance steal)",    &kMnkTechnique },
	{ "Balance / Poise (stance)",       &kMnkPoise },
	{ "Crane / Heron Stance",           &kMnkCrane },
	{ "Earthforce (stance)",            &kMnkEarthforce },
	{ "Heel (flying kick trance)",      &kMnkHeel },
	{ "Form (two realities)",           &kMnkForm },
	{ "Drunken Monkey Style",           &kMnkDrunken },
	{ "Palm (melee damage)",            &kMnkPalm },
	{ "Ironfist (special attacks)",     &kMnkIronfist },
	{ "Eye of the Storm",               &kMnkEyeStorm },
	{ "Reflexes (block)",               &kMnkReflexes },
	{ "Flinch / Reflex (block)",        &kMnkFlinch },
	{ "Counterforce (slow attackers)",  &kMnkCounter },
	{ "Void Body (avoidance)",          &kMnkVoidBody },
	{ "Void Step (dodge)",              &kMnkVoidStep },
	{ "Death (delay death)",            &kMnkDeath },
	{ "Protection of the Shadewalker",  &kMnkProtection },
	{ "Projection (astral)",            &kMnkProjection },
	{ "Innerflame (discipline)",        &kMnkInnerflame },
	{ "Breaths (endurance)",            &kMnkBreaths },
	{ "Wind (endurance regen)",         &kMnkWinds },
	{ "Rest (endurance)",               &kMnkBreather },
	{ "Inner Rejuvenation",             &kMnkRejuvenation },
	{ "Focused / Healing Will",         &kMnkFocusedWill },
	{ "Resistant (discipline)",         &kMnkResistant },
	{ "Fearless (discipline)",          &kMnkFearless },
	{ "Thunderkick (discipline)",       &kMnkThunderkick },
	{ "Whirlwind (discipline)",         &kMnkWhirlwind },
	{ "Voiddance (discipline)",         &kMnkVoiddance },
	{ "Disciple's / Master's Aura",     &kMnkAura },
	{ "Hundred Fists (discipline)",     &kMnkHundredFists },
	{ "Silentfist (discipline)",        &kMnkSilentfist },
	{ "Ashenhand (discipline)",         &kMnkAshenhand },
	{ "Planeswalk (discipline)",        &kMnkPlaneswalk },
	{ "Speed Focus (discipline)",       &kMnkSpeedFocus },
	{ "Dreamwalk (discipline)",         &kMnkDreamwalk },
	{ "Rapid Kick (discipline)",        &kMnkRapidKick },
	{ "Fists of Wu (group)",            &kMnkFistsOfWu },

	{ "AA: Imitate Death",              &kAaImitateDeath },
	{ "AA: Five Point Palm",            &kAaFivePointPalm },
	{ "AA: Two-Finger Wasp Touch",      &kAaWaspTouch },
	{ "AA: Zan Fi's Whistle",           &kAaZanFi },
	{ "AA: Zan Fi's Echoes (strike)",   &kAaZanFiEcho },
	{ "AA: Infusion of Thunder",        &kAaInfusionThunder },
	{ "AA: Infusion of Thunder Shock",  &kAaThunderShock },
	{ "AA: Destructive Force",          &kAaDestructiveForce },
	{ "AA: Focused Destructive Force",  &kAaFocusedForce },
	{ "AA: Dragon Force",               &kAaDragonForce },
	{ "AA: Eye Gouge",                  &kAaEyeGouge },
	{ "AA: Grappling Strike",           &kAaGrappling },
	{ "AA: Moving Mountains",           &kAaMovingMountains },
	{ "AA: Neshika's Blink",            &kAaNeshikaBlink },
	{ "AA: Ton Po's Stance",            &kAaTonPo },
	{ "AA: Spire of the Sensei",        &kAaSpireSensei },
	{ "AA: Sensei's Guard",             &kAaSenseiGuard },
	{ "AA: Swift Tails' Chant",         &kAaSwiftTails },
	{ "AA: Distant Strike",             &kAaDistantStrike },
	{ "AA: Magnanimous Force",          &kAaMagnanimous },
	{ "AA: Ascendant's Intimidation",   &kAaIntimidation },
	{ "AA: Stunning Kick",              &kAaStunningKick },
	{ "AA: Resonant Kick",              &kAaResonantKick },
	{ "AA: Thunderfoot",                &kAaThunderfoot },
	{ "AA: Sweeping Kick",              &kAaSweepingKick },
	{ "AA: Fists of Fury",              &kAaFistsOfFury },
	{ "AA: Master's Synergy",           &kAaMastersSynergy },
	{ "AA: Purified Protection",        &kAaPurifiedProtection },
	{ "AA: Purify Body",                &kAaPurifyBody },
	{ "AA: Vehement Rage",              &kAaVehementRage },
};

const GroupLine kNumbered[] = {
	{ 0, Echo },        { 2, Synergy },     { 4, Fists },       { 5, Apparition },  { 6, Breaths },
	{ 9, Projection },  { 10, Death },      { 11, Palm },       { 12, Shuriken },   { 13, Flinch },
	{ 14, Drunken },    { 15, Phantom },    { 16, Ironfist },   { 17, VoidBody },   { 18, Heel },
	{ 19, Poise },      { 20, VoidStep },   { 22, Earthforce }, { 23, Reflexes },   { 24, Fang },
	{ 25, Protection }, { 26, Sting },      { 27, Technique },  { 28, Counter },       { 30, Form },
	{ 31, Form },       { 32, Curse },      { 33, Alliance },   { 34, Precision },  { 73, Heel },
};

const GroupLine kModern[] = {
	{ 10, Echo },       { 20, Synergy },    { 30, Fists },      { 40, Apparition }, { 50, Breaths },
	{ 80, Death },      { 130, Phantom },   { 170, Poise },     { 210, Reflexes },  { 220, Fang },
	{ 240, Sting },     { 250, Technique }, { 260, Counter },      { 270, Form },      { 271, Form },
	{ 280, Curse },     { 290, Alliance },  { 300, Precision },
};

const GroupLine kOther[] = {
	{ 15000, Echo },      { 15002, Earthforce }, { 15004, Fists },     { 15005, Apparition }, { 15006, Breaths },
	{ 15007, Ironfist },  { 4690, Earthforce },  { 45120, Innerflame },
	{ 15211, Through },   { 15311, Through },    { 15611, Through },   { 15911, Through },
	{ 16008, Winds },     { 3105, Winds },       { 3205, Winds },      { 3305, Winds },      { 3405, Winds },
	{ 3505, Winds },      { 3415, Rejuvenation },
	{ 3311, Breather },   { 3411, Breather },    { 3511, Breather },   { 3611, Breather },   { 3737, Breather },
	{ 3837, Breather },   { 3937, Breather },    { 125010040, Breather }, { 126010040, Breather },
	{ 8421, Crane },      { 8821, Crane },       { 8521, EyeStorm },
};

const NamedLine kAbilities[] = {
	{ "imitate death",                 ImitateDeath },
	{ "five point palm",               FivePointPalm },
	{ "five point palm focusing",      FivePointPalm },
	{ "two-finger wasp touch",         WaspTouch },
	{ "zan fi's whistle",              ZanFi },
	{ "zan fi's echoes",               ZanFi },
	{ "zan fi's echoes strike",        ZanFiEcho },
	{ "zan fi's echo",                 ZanFiEcho },
	{ "infusion of thunder",           InfusionThunder },
	{ "infusion of thunder shock",     ThunderShock },
	{ "destructive force",             DestructiveForce },
	{ "focused destructive force",     FocusedForce },
	{ "dragon force",                  DragonForce },
	{ "eye gouge",                     EyeGouge },
	{ "grappling strike",              Grappling },
	{ "moving mountains",              MovingMountains },
	{ "neshika's blink",               NeshikaBlink },
	{ "ton po's stance",               TonPo },
	{ "spire of the sensei",           SpireSensei },
	{ "sensei's guard",                SenseiGuard },
	{ "swift tails' chant",            SwiftTails },
	{ "distant strike",                DistantStrike },
	{ "magnanimous force",             Magnanimous },
	{ "ascendant's intimidation",      Intimidation },
	{ "stunning kick",                 StunningKick },
	{ "resonant kick",                 ResonantKick },
	{ "resonant strike",               ResonantKick },
	{ "thunderfoot",                   Thunderfoot },
	{ "sweeping kick",                 SweepingKick },
	{ "fists of fury",                 FistsOfFury },
	{ "master's synergy",              MastersSynergy },
	{ "purified protection",           PurifiedProtection },
	{ "purification",                  PurifyBody },
	{ "vehement rage",                 VehementRage },
};

const NamedLine kUngrouped[] = {
	{ "focused will discipline",       FocusedWill },
	{ "healing will discipline",       FocusedWill },
	{ "resistant discipline",          Resistant },
	{ "fearless discipline",           Fearless },
	{ "stonestance discipline",        Earthforce },
	{ "thunderkick discipline",        Thunderkick },
	{ "whirlwind discipline",          Whirlwind },
	{ "voiddance discipline",          Voiddance },
	{ "disciple's aura",               Aura },
	{ "master's aura",                 Aura },
	{ "hundred fists discipline",      HundredFists },
	{ "silentfist discipline",         Silentfist },
	{ "ashenhand discipline",          Ashenhand },
	{ "planeswalk discipline",         Planeswalk },
	{ "speed focus discipline",        SpeedFocus },
	{ "dreamwalk discipline",          Dreamwalk },
	{ "counterforce discipline",       Counter },
	{ "rapid kick discipline",         RapidKick },
	{ "fists of wu",                   FistsOfWu },
	{ "leopard claw",                  Fang },
	{ "dragon fang",                   Fang },
	{ "phantom zephyr",                Apparition },
	{ "phantom wind",                  Apparition },
	{ "phantom echo",                  Apparition },
	{ "phantom call",                  Apparition },
	{ "phantom shadow",                Apparition },
	{ "phantom cry",                   Apparition },
	{ "ancient: phantom chaos",        Apparition },
};

int BySpell(const eqlib::EQ_Spell*, std::string_view base)
{
	for (const NamedLine& n : kUngrouped)
		if (base == n.name)
			return n.line;
	return -1;
}

} // namespace

const ClassCatalogue& MonkCatalogue()
{
	static const ClassCatalogue c{ "Monk", eqlib::Monk, kLines, 8, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
