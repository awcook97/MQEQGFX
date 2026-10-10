#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

namespace eqgfx::spellfx {

namespace {

constexpr Pair kRngFletch  { { 0.98f, 0.95f, 0.85f, 0.95f }, { 0.55f, 0.40f, 0.22f, 0.90f } };
constexpr Pair kRngHunter  { { 0.72f, 0.88f, 0.40f, 0.95f }, { 0.35f, 0.42f, 0.12f, 0.85f } };
constexpr Pair kRngMist    { { 0.85f, 0.92f, 0.90f, 0.55f }, { 0.50f, 0.60f, 0.62f, 0.25f } };
constexpr Pair kRngDew     { { 0.85f, 1.00f, 0.82f, 0.95f }, { 0.40f, 0.85f, 0.95f, 0.80f } };
constexpr Pair kRngSunlit  { { 1.00f, 0.95f, 0.55f, 0.95f }, { 1.00f, 0.55f, 0.10f, 0.85f } };
constexpr Pair kRngEmber   { { 1.00f, 0.60f, 0.25f, 0.95f }, { 0.30f, 0.25f, 0.22f, 0.85f } };
constexpr Pair kRngWolf    { { 0.85f, 0.88f, 0.95f, 0.90f }, { 0.40f, 0.45f, 0.60f, 0.75f } };
constexpr Pair kRngEagle   { { 1.00f, 0.95f, 0.80f, 0.95f }, { 0.60f, 0.40f, 0.20f, 0.85f } };
constexpr Pair kRngCheetah { { 1.00f, 0.40f, 0.25f, 0.95f }, { 0.85f, 0.62f, 0.10f, 0.85f } };
constexpr Pair kRngBee     { { 1.00f, 0.90f, 0.20f, 0.95f }, { 0.35f, 0.25f, 0.05f, 0.90f } };
constexpr Pair kRngMoss    { { 0.45f, 0.75f, 0.35f, 0.95f }, { 0.15f, 0.30f, 0.10f, 0.90f } };
constexpr Pair kRngDusk    { { 0.55f, 0.60f, 0.80f, 0.85f }, { 0.10f, 0.10f, 0.25f, 0.75f } };
constexpr Pair kRngScale   { { 0.45f, 0.62f, 0.45f, 0.95f }, { 0.08f, 0.12f, 0.10f, 0.90f } };
constexpr Pair kRngBrownie { { 1.00f, 0.85f, 0.95f, 0.95f }, { 0.55f, 0.85f, 0.40f, 0.85f } };
constexpr Pair kRngSky     { { 0.80f, 0.90f, 1.00f, 0.95f }, { 0.35f, 0.40f, 0.85f, 0.80f } };
constexpr Pair kRngTawny   { { 1.00f, 0.80f, 0.45f, 0.95f }, { 0.55f, 0.30f, 0.10f, 0.90f } };

LOOK(kRngBalm, "balm",
	G(IM, Glyph_Leaf, 0.9f, 8.5f, 4, 1, GA_Drop, 0, FIX, kNature, 2.2f),
	F(IM, FxForm_Flame, 0.8f, 8.5f, 72, 18, 0.1f, 2.6f, 0, FIX, kHeal, 2.0f, 0.07f, ADD, 1, 0.4f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kRngDew, 2.2f, 0.06f, ADD, 1, 0.3f));

LOOK(kRngSpring, "spring",
	G(IM, Glyph_Drop, 1.0f, 9.0f, 4, 1, GA_Drop, 0, FIX, kRngDew, 1.6f),
	F(IM, FxForm_Burst, 2.0f, 1.5f, 72, 2, 2.0f, 16, 0, FIX, kWater, 1.2f, 0.08f, ADD, 1, 0.45f),
	F(IM, FxForm_Bubbles, 0.9f, 6.0f, 64, 10, 0.22f, 0, 0, FIX, kRngDew, 2.2f, 0.035f, ADD, 1, 0.45f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 64, 2, 1.2f, 0, 0.5f, FIX, kHeal, 2.4f, 0.05f, COMET, 1, 0.4f));

LOOK(kRngDesperate, "desperate",
	F(IM, FxForm_Rain, 1.0f, 12.0f, 60, 50, 0, 0.1f, 0, FIX, kRngDew, 1.8f, 0.05f),
	G(IM, Glyph_Wave, 1.2f, 8.5f, 4, 1, GA_Drop, 0, FIX, kWater, 1.6f),
	F(IM, FxForm_Burst, 2.6f, 0.4f, 96, 3, 2.5f, 20, 0, FIX, kHeal, 1.2f, 0.10f, ADD, 1, 0.4f),
	F(IM, FxForm_Flame, 0.85f, 10.0f, 80, 20, 0.1f, 3.0f, 0, FIX, kHeal, 1.8f, 0.08f, ADD, 1, 0.45f));

LOOK(kRngLingering, "lingering",
	F(IM, FxForm_Orbit, 1.4f, 6.0f, 48, 10, 0, 0, 0, FIX, kRngMist, 3.0f, 0.05f, ADD, 1.5f),
	G(IM, Glyph_Drop, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kRngDew, 3.0f),
	F(IM, FxForm_Rain, 0.8f, 9.0f, 30, 20, 0.5f, 0, 0, FIX, kRngDew, 2.6f, 0.04f),
	F(AU, FxForm_Helix, 1.0f, 6.0f, 72, 3, 1.2f, 0, 0.5f, FIX, kRngDew, 0, 0.04f, COMET));

LOOK(kRngSummer, "summer's",
	G(RE, Glyph_Sun, 1.2f, 9.0f, 4, 1, GA_PopRoll, 0, FIX, kRngSunlit, 2.2f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kRngDew, 2.2f, 0.07f, ADD, 1, 0.2f, AE),
	F(RE, FxForm_Sparkle, 0.8f, 8.0f, 60, 30, 0.12f, 1, 0, FIX, kRngSunlit, 2.2f, 0.06f, ADD, 1, 0, AE),
	F(IM, FxForm_Flame, 0.8f, 7.0f, 64, 16, 0.2f, 2.0f, 0, FIX, kRngSunlit, 1.6f, 0.07f));

LOOK(kRngSummerBurn, "summer's burn",
	F(IM, FxForm_Flame, 0.9f, 8.0f, 80, 20, 0.3f, 2.2f, 0, FIX, kRngSunlit, 1.4f, 0.09f),
	G(IM, Glyph_Sun, 0.6f, 7.5f, 4, 1, GA_PopAscend, 0, FIX, kFire, 1.4f),
	F(IM, FxForm_Burst, 1.8f, 2.0f, 48, 1, 0.5f, 12, 0, FIX, kFire, 0.7f, 0.08f));

LOOK(kRngSummerSplash, "summer's splash",
	F(IM, FxForm_Burst, 1.0f, 1.0f, 72, 2, 1.5f, 12, 0, FIX, kRngDew, 1.0f, 0.07f, ADD, 1, 0, AE),
	F(IM, FxForm_Bubbles, 0.6f, 5.0f, 64, 10, 0.2f, 0, 0, FIX, kRngDew, 1.6f, 0.035f, ADD, 1, 0, AE),
	G(IM, Glyph_Drop, 0.3f, 6.0f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 1.2f));

LOOK(kRngVigor, "vigor",
	G(IM, Glyph_Paw, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kRngHunter, 2.4f),
	F(IM, FxForm_Helix, 1.05f, 6.0f, 72, 3, 1.0f, 0, 0.5f, FIX, kHeal, 2.6f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.10f, 0, 0, FIX, kNature, 2.4f));

LOOK(kRngWind, "wind",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.5f, 0.4f, 0.6f, FIX, kCold, 0.8f, 0.14f),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 12, 2.5f, 0, 0, FIX, kCold, 1.6f, 0.05f),
	F(IM, FxForm_Crystal, 1.0f, 5.0f, 60, 12, 0, 0.7f, 0, FIX, kCold, 1.8f, 0.05f),
	G(IM, Glyph_Snowflake, 0.35f, 5.5f, 4, 4, GA_Orbit, 1.2f, FIX, kWhite, 1.8f));

LOOK(kRngAsh, "ash",
	G(IM, Glyph_Flame, 1.0f, 8.5f, 4, 1, GA_Drop, 0, FIX, kFire, 1.4f),
	F(IM, FxForm_Rain, 1.2f, 12.0f, 60, 40, 0.3f, 0.2f, 0, FIX, kRngEmber, 2.0f, 0.05f),
	F(IM, FxForm_Flame, 0.9f, 7.0f, 96, 24, 0.4f, 1.8f, 0, FIX, kFire, 1.6f, 0.09f, ADD, 1, 0.35f),
	F(IM, FxForm_Cracks, 1.8f, 0, 48, 6, 1.2f, 0, 0, FIX, kRngEmber, 2.0f, 0.06f, ADD, 1, 0.35f));

LOOK(kRngHeartshot, "heartshot",
	F(TR, FxForm_Bolt, 1, 0, 64, 1, 0, 0.12f, 1.2f, FIX, kRngFletch, 0.6f, 0.10f),
	F(TR, FxForm_Bolt, 1, 0, 64, 3, 0.2f, 0.3f, 1.2f, FIX, kBlood, 0.6f, 0.05f),
	G(IM, Glyph_Heart, 0.7f, 5.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kBlood, 1.2f),
	F(IM, FxForm_Burst, 1.6f, 4.0f, 48, 1, 0.3f, 12, 0, FIX, kBlood, 0.6f, 0.08f),
	F(IM, FxForm_Spire, 0.5f, 4.5f, 64, 12, 1.5f, 2.0f, 0, FIX, kRngFletch, 0.6f, 0.05f));

LOOK(kRngHeartpierce, "heartpierce",
	G(IM, Glyph_Sword, 1.1f, 9.0f, 4, 1, GA_Drop, 0, FIX, kIron, 1.4f),
	G(IM, Glyph_Heart, 0.6f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kBlood, 1.2f, 0.3f),
	F(IM, FxForm_Burst, 2.2f, 0.4f, 64, 2, 0.4f, 14, 0, FIX, kBlood, 0.9f, 0.09f, ADD, 1, 0.3f),
	F(IM, FxForm_Cracks, 1.6f, 0, 40, 5, 1.0f, 0, 0, FIX, kWood, 1.6f, 0.06f, OVER, 1, 0.3f));

LOOK(kRngSwarm, "swarm",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.2f, 12, 0, FIX, kRngBee, 1.2f, 0.06f),
	G(IM, Glyph_Insect, 0.3f, 7.0f, 4, 6, GA_Rain, 1.2f, FIX, kRngBee, 2.4f),
	F(IM, FxForm_Orbit, 1.3f, 6.0f, 72, 16, 0, 0, 0, FIX, kRngBee, 3.0f, 0.04f, ADD, 3.0f));

LOOK(kRngFlusterbolt, "flusterbolt",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.5f, 0.3f, 0.8f, FIX, kAir, 0.7f, 0.14f),
	F(IM, FxForm_Vortex, 1.0f, 5.0f, 72, 4, 1.4f, 2.0f, 5.0f, FIX, kAir, 1.4f, 0.05f),
	G(IM, Glyph_Tornado, 0.8f, 7.5f, 4, 1, GA_PopRoll, 0, FIX, kAir, 1.4f),
	Stars(0.2f));

LOOK(kRngVinelash, "vinelash",
	F(TR, FxForm_Bolt, 1, 0, 96, 2, 0.6f, 0.6f, 0.4f, FIX, kWood, 0.7f, 0.10f),
	F(IM, FxForm_Vines, 1.0f, 5.0f, 96, 5, 1.0f, 0, 0, FIX, kWood, 2.0f, 0.06f),
	F(IM, FxForm_Thorns, 1.0f, 5.0f, 96, 5, 1.0f, 4, 0, FIX, kThorn, 2.0f, 0.04f),
	F(IM, FxForm_Burst, 2.0f, 1.0f, 48, 1, 0.8f, 10, 0, FIX, kNature, 0.8f, 0.08f));

LOOK(kRngEntropy, "nature's entropy",
	G(IM, Glyph_Leaf, 0.4f, 8.0f, 4, 5, GA_Rain, 1.2f, FIX, kAmber, 2.4f),
	F(IM, FxForm_Vines, 1.0f, 4.5f, 96, 5, 0.8f, 0, 1, FIX, kRngMoss, 2.4f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.10f, 1, 0, FIX, kAmber, 2.0f));

LOOK(kRngFocusedArrows, "focused arrows",
	F(TR, FxForm_Bolt, 1, 0, 48, 1, 0, 0.12f, 1.0f, FIX, kRngFletch, 0.6f, 0.09f),
	F(TR, FxForm_Bolt, 1, 0, 48, 1, 0, 0.12f, 1.5f, FIX, kRngFletch, 0.6f, 0.09f, ADD, 1, 0.12f),
	F(TR, FxForm_Bolt, 1, 0, 48, 1, 0, 0.12f, 0.6f, FIX, kRngFletch, 0.6f, 0.09f, ADD, 1, 0.24f),
	F(IM, FxForm_Burst, 1.6f, 4.0f, 48, 2, 0.3f, 12, 0, FIX, kIron, 0.7f, 0.07f),
	F(IM, FxForm_Sparkle, 1.0f, 5.0f, 24, 12, 0.10f, 0, 0, FIX, kRngFletch, 1.0f));

LOOK(kRngStormArrows, "storm of arrows",
	G(IM, Glyph_ArrowDown, 0.45f, 9.0f, 4, 6, GA_Rain, 1.8f, FIX, kRngFletch, 1.6f),
	F(IM, FxForm_Rain, 1.0f, 14.0f, 80, 60, 1.4f, 0.25f, 0, FIX, kRngFletch, 1.6f, 0.04f, ADD, 1, 0, AE),
	F(IM, FxForm_Burst, 1.0f, 0.3f, 96, 2, 0.4f, 20, 0, FIX, kIron, 1.0f, 0.08f, ADD, 1, 0.4f, AE));

LOOK(kRngArcArrows, "arc of arrows",
	F(TR, FxForm_Bolt, 1, 0, 48, 1, 0, 0.12f, 2.0f, FIX, kRngFletch, 0.7f, 0.08f),
	F(TR, FxForm_Bolt, 1, 0, 48, 1, 0, 0.12f, 2.8f, FIX, kRngFletch, 0.7f, 0.08f, ADD, 1, 0.08f),
	F(TR, FxForm_Bolt, 1, 0, 48, 1, 0, 0.12f, 3.6f, FIX, kRngFletch, 0.7f, 0.08f, ADD, 1, 0.16f),
	F(IM, FxForm_Bloom, 1.0f, 0.5f, 72, 2, 0, 0, 0, FIX, kIron, 1.2f, 0.05f, ADD, 1, 0, AE),
	G(IM, Glyph_ArrowDown, 0.4f, 6.0f, 4, 5, GA_Drop, 1.2f, FIX, kRngFletch, 1.2f));

LOOK(kRngAimshot, "aimshot",
	G(IM, Glyph_ArrowUp, 0.9f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kGold, 2.6f),
	F(IM, FxForm_Rune, 1.4f, 5.0f, 60, 4, 2, 12, 1, FIX, kRngFletch, 3.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 2, 0, FIX, kGold, 2.4f));

LOOK(kRngShot, "shot",
	F(TR, FxForm_Bolt, 1, 0, 64, 2, 0.25f, 0.2f, 0.9f, FIX, kBlood, 0.6f, 0.10f),
	F(IM, FxForm_Rune, 1.2f, 4.0f, 60, 4, 2, 8, 1, FIX, kBlood, 1.4f, 0.05f),
	F(IM, FxForm_Ring, 1.2f, 0.3f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kBlood, 1.4f, 0.05f, COMET));

LOOK(kRngCalledShots, "called shots",
	G(IM, Glyph_ArrowUp, 0.35f, 4.5f, 4, 4, GA_Orbit, 1.3f, FIX, kRngFletch, 3.0f),
	F(IM, FxForm_Ring, 1.3f, 4.5f, 48, 0, 1.2f, 1.0f, 0.5f, FIX, kGold, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 24, 12, 0.10f, 0, 0, FIX, kGold, 2.4f));

LOOK(kRngFusillade, "fusillade",
	F(TR, FxForm_Bolt, 1, 0, 96, 2, 0.5f, 0.3f, 1.2f, FIX, kRngFletch, 0.7f, 0.10f),
	F(TR, FxForm_Bolt, 1, 0, 96, 2, 0.5f, 0.3f, 1.2f, FIX, kFire, 0.7f, 0.10f, ADD, 1, 0.1f),
	F(IM, FxForm_Burst, 2.4f, 2.0f, 72, 2, 1.0f, 16, 0, FIX, kFire, 0.9f, 0.09f),
	F(CH, FxForm_Helix, 1.0f, 6.0f, 64, 2, 1.2f, 0, 0.5f, FIX, kHeal, 2.0f, 0.05f, COMET));

LOOK(kRngJoltKick, "jolting kicks",
	F(IM, FxForm_Burst, 1.8f, 1.0f, 48, 1, 0.3f, 10, 0, FIX, kShock, 0.6f, 0.08f),
	F(IM, FxForm_Lightning, 1, 0, 32, 2, 0.8f, 16.0f, 0, FIX, kShock, 0.4f, 0.06f),
	G(IM, Glyph_ArrowDown, 0.6f, 7.5f, 4, 1, GA_Drop, 0, FIX, kRngMist, 1.4f));

LOOK(kRngJoltStrike, "jolting strikes",
	G(IM, Glyph_Sword, 0.8f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kSilver, 0.9f),
	F(IM, FxForm_Lightning, 1, 0, 48, 3, 1.0f, 18.0f, 0, FIX, kShock, 0.5f, 0.07f),
	F(IM, FxForm_Bloom, 1.4f, 3.0f, 40, 2, 0, 0, 0, FIX, kRngMist, 1.2f, 0.05f));

LOOK(kRngEnrageKick, "enraging kicks",
	F(IM, FxForm_Burst, 2.0f, 0.8f, 64, 2, 0.4f, 12, 0, FIX, kZeal, 0.8f, 0.09f),
	G(IM, Glyph_ArrowUp, 0.6f, 7.5f, 4, 1, GA_PopAscend, 0, FIX, kBlood, 1.4f),
	F(IM, FxForm_Cracks, 1.5f, 0, 32, 4, 1.0f, 0, 0, FIX, kEarth, 1.4f, 0.05f, OVER));

LOOK(kRngBlades, "blades",
	G(IM, Glyph_Sword, 0.5f, 3.5f, 3, 3, GA_Orbit, 1.3f, FIX, kSilver, 3.0f),
	F(IM, FxForm_Lightning, 1, 30.0f, 48, 1, 0.5f, 6.0f, 1.0f, FIX, kShock, 0.8f, 0.10f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.10f, 0, 0, FIX, kShock, 2.2f));

LOOK(kRngDevastating, "devastating",
	G(IM, Glyph_Sword, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kIron, 1.4f),
	F(IM, FxForm_Burst, 2.6f, 0.3f, 96, 3, 0.5f, 20, 0, FIX, kIron, 1.0f, 0.11f, ADD, 1, 0.35f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 1.8f, 0.06f, OVER, 1, 0.35f));

LOOK(kRngStormBlades, "storm of blades",
	G(RE, Glyph_Sword, 0.5f, 3.0f, 3, 6, GA_PopSpin, 2.2f, FIX, kSilver, 1.6f),
	F(RE, FxForm_Vortex, 1.0f, 3.5f, 96, 6, 1.4f, 1.5f, 6.0f, FIX, kIron, 1.4f, 0.06f, ADD, 1, 0, AE),
	F(RE, FxForm_Burst, 1.0f, 0.3f, 96, 2, 0.4f, 24, 0, FIX, kSilver, 1.0f, 0.09f, ADD, 1, 0.2f, AE));

LOOK(kRngFocusedBlades, "focused blades",
	G(IM, Glyph_Sword, 0.5f, 4.0f, 3, 4, GA_PopSpin, 1.2f, FIX, kSilver, 1.2f),
	F(IM, FxForm_Spire, 0.6f, 4.0f, 64, 12, 1.5f, 2.0f, 0, FIX, kIron, 0.8f, 0.06f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0.3f, 12, 0, FIX, kSilver, 0.6f, 0.08f, ADD, 1, 0.2f));

LOOK(kRngReflexive, "reflexive spurs",
	F(IM, FxForm_Thorns, 1.0f, 5.0f, 96, 5, 1.2f, 5, 0, FIX, kThorn, 2.6f, 0.05f),
	G(IM, Glyph_Shield, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kThorn, 2.4f),
	F(IM, FxForm_Burst, 2.0f, 3.0f, 64, 2, 0, 12, 0, FIX, kIron, 1.0f, 0.07f, ADD | FxFlag_Reverse));

LOOK(kRngShieldOf, "shield of",
	F(IM, FxForm_Thorns, 1.0f, 5.5f, 96, 6, 1.0f, 5, 0, FIX, kThorn, 2.6f, 0.05f),
	G(IM, Glyph_Shield, 0.9f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kWood, 2.4f),
	F(IM, FxForm_Ring, 1.2f, 0.3f, 40, 0, 0.8f, 1.0f, 0.4f, FIX, kThorn, 2.4f, 0.05f, COMET));

LOOK(kRngCloak, "cloak of",
	F(IM, FxForm_Dome, 1.5f, 6.5f, 96, 8, 2, 0, 0, FIX, kWood, 2.4f, 0.03f),
	F(IM, FxForm_Crystal, 1.1f, 5.5f, 60, 12, 0, 0.5f, 0, FIX, kThorn, 2.6f, 0.05f),
	G(IM, Glyph_Feather, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kRngEagle, 2.6f));

LOOK(kRngSharedCloak, "shared cloak",
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kThorn, 2.2f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Feather, 0.45f, 5.0f, 3, 6, GA_RingRise, 2.4f, FIX, kRngEagle, 2.6f),
	F(IM, FxForm_Crystal, 1.1f, 5.5f, 60, 12, 0, 0.5f, 0, FIX, kThorn, 2.4f, 0.05f));

LOOK(kRngCoat, "coat",
	F(IM, FxForm_Vines, 1.0f, 5.5f, 96, 5, 1.0f, 0, 0, FIX, kWood, 2.6f, 0.06f),
	F(IM, FxForm_Thorns, 1.0f, 5.5f, 96, 5, 1.0f, 3, 0, FIX, kThorn, 2.6f, 0.04f),
	G(IM, Glyph_Leaf, 0.3f, 4.5f, 0, 3, GA_Orbit, 1.3f, FIX, kNature, 2.6f));

LOOK(kRngNettlecoat, "grasping nettlecoat",
	F(IM, FxForm_Vines, 1.1f, 2.5f, 96, 6, 0.6f, 0, 1, FIX, kRngMoss, 3.0f, 0.06f),
	F(IM, FxForm_Thorns, 1.1f, 2.5f, 96, 6, 0.6f, 4, 1, FIX, kThorn, 3.0f, 0.04f),
	G(IM, Glyph_Shackle, 0.5f, 6.8f, 4, 1, GA_PopBob, 0, FIX, kWood, 2.4f));

LOOK(kRngSkin, "skin",
	F(IM, FxForm_Crystal, 1.0f, 5.5f, 60, 14, 0, 0.6f, 0, FIX, kRngScale, 2.6f, 0.06f, OVER),
	G(IM, Glyph_Snake, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kRngScale, 2.4f),
	F(IM, FxForm_Lattice, 1.2f, 6.0f, 64, 8, 0.4f, 0, 0, FIX, kRngMoss, 2.2f, 0.04f));

LOOK(kRngProtection, "protection",
	G(IM, Glyph_Tree, 1.1f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kNature, 2.8f),
	F(IM, FxForm_Dome, 1.7f, 7.0f, 96, 8, 3, 0, 0, FIX, kNature, 2.4f, 0.04f),
	F(IM, FxForm_Ring, 1.7f, 0.3f, 48, 0, 1.2f, 1.0f, 0.5f, FIX, kWood, 2.4f, 0.05f, COMET));

LOOK(kRngStrength, "strength",
	G(IM, Glyph_Fist, 0.8f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kRngHunter, 2.4f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 72, 2, 1.0f, 0, 0.5f, FIX, kWood, 2.6f, 0.05f, COMET),
	F(IM, FxForm_Burst, 2.0f, 1.5f, 48, 1, 1.5f, 12, 0, FIX, kNature, 0.9f, 0.08f));

LOOK(kRngPredator, "predator",
	G(RE, Glyph_Claw, 1.0f, 8.5f, 4, 1, GA_PopRoll, 0, FIX, kRngTawny, 2.2f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kRngTawny, 2.0f, 0.06f, ADD, 1, 0.2f, AE),
	G(IM, Glyph_Fang, 0.5f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kBone, 2.0f),
	F(IM, FxForm_Flame, 0.9f, 5.0f, 64, 16, 0.9f, 0.7f, 0, FIX, kRngTawny, 2.0f, 0.06f));

LOOK(kRngShout, "shout",
	G(RE, Glyph_Horn, 0.9f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kRngHunter, 1.8f),
	F(RE, FxForm_Bloom, 2.2f, 5.0f, 48, 3, 0, 0, 0, FIX, kRngHunter, 1.6f, 0.05f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 0.8f, 16, 0, FIX, kRngTawny, 1.2f, 0.09f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kRngTawny, 2.0f));

LOOK(kRngEyes, "eyes",
	G(IM, Glyph_Eye, 0.9f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kRngEagle, 3.0f),
	F(IM, FxForm_Ring, 0.7f, 8.2f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kGold, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 32, 16, 0.10f, 2, 0, FIX, kRngEagle, 2.4f));

LOOK(kRngSagacity, "skylight sagacity",
	G(IM, Glyph_Moon, 0.8f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kRngSky, 2.6f),
	F(IM, FxForm_Sparkle, 1.4f, 9.0f, 48, 24, 0.14f, 0, 0, FIX, kStar, 2.6f),
	G(IM, Glyph_Feather, 0.3f, 5.5f, 0, 3, GA_Orbit, 1.2f, FIX, kRngSky, 2.6f));

LOOK(kRngDrifting, "drifting",
	F(IM, FxForm_Wind, 1.4f, 5.0f, 60, 10, 1.5f, 0, 0, FIX, kRngMist, 2.2f, 0.05f, OVER),
	F(IM, FxForm_Bloom, 1.6f, 1.0f, 40, 2, 0, 0, 0, FIX, kRngMist, 2.2f, 0.05f, OVER),
	G(IM, Glyph_Feather, 0.6f, 7.5f, 4, 1, GA_Drop, 0, FIX, kRngMist, 2.0f));

LOOK(kRngVeil, "veil",
	F(IM, FxForm_Dome, 1.6f, 6.5f, 96, 6, 2, 0, 0, FIX, kRngDusk, 3.0f, 0.03f),
	G(IM, Glyph_Moon, 0.7f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kRngDusk, 3.0f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kRngDusk, 2.6f));

LOOK(kRngWindBoon, "wind boon",
	G(IM, Glyph_Tornado, 0.8f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kCold, 2.4f),
	F(IM, FxForm_Vortex, 1.2f, 6.0f, 96, 4, 1.4f, 2.0f, 4.0f, FIX, kCold, 2.4f, 0.05f),
	G(IM, Glyph_Snowflake, 0.3f, 4.0f, 0, 3, GA_Orbit, 1.3f, FIX, kWhite, 2.4f));

LOOK(kRngFireBoon, "fire boon",
	G(IM, Glyph_Flame, 0.8f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kFire, 2.4f),
	F(IM, FxForm_Sparkle, 1.3f, 8.0f, 60, 30, 0.12f, 0, 0, FIX, kRngEmber, 2.4f),
	F(IM, FxForm_Flame, 1.0f, 2.5f, 64, 16, 0.2f, 0.6f, 0, FIX, kFire, 2.2f, 0.06f));

LOOK(kRngBurningCloak, "burning cloak",
	F(IM, FxForm_Flame, 1.05f, 6.5f, 112, 28, 0.9f, 1.0f, 0, FIX, kFire, 3.0f, 0.08f),
	G(IM, Glyph_Sun, 0.6f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kFire, 2.2f),
	Persist(F(AU, FxForm_Ring, 0.9f, 0.3f, 40, 0, 1.0f, 1.0f, 0.5f, FIX, kFire, 0, 0.05f, COMET)));

LOOK(kRngHunt, "by the hunt",
	G(IM, Glyph_Wolf, 1.0f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kRngHunter, 3.0f),
	F(IM, FxForm_Flame, 1.0f, 5.8f, 96, 24, 0.9f, 0.9f, 0, FIX, kRngHunter, 3.0f, 0.08f),
	F(IM, FxForm_Burst, 2.0f, 1.0f, 48, 1, 1.0f, 12, 0, FIX, kRngTawny, 0.9f, 0.08f));

LOOK(kRngDiscipline, "stalker's discipline",
	G(IM, Glyph_Claw, 1.1f, 8.4f, 4, 1, GA_PopRoll, 0, FIX, kBlood, 3.0f),
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.8f, 1.2f, 0, FIX, kFlame2, 3.0f, 0.10f),
	F(IM, FxForm_Cracks, 2.2f, 0, 48, 6, 1.2f, 0, 0, FIX, kEarth, 2.5f, 0.07f, OVER),
	F(IM, FxForm_Burst, 2.6f, 0.3f, 72, 2, 0.4f, 16, 0, FIX, kBlood, 1.0f, 0.10f));

LOOK(kRngUnity, "unity",
	G(IM, Glyph_Paw, 0.35f, 5.0f, 3, 5, GA_RingRise, 1.4f, FIX, kRngHunter, 3.0f),
	F(IM, FxForm_Rune, 1.8f, 0, 60, 5, 2, 10, 0, FIX, kNature, 3.0f, 0.05f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 72, 2, 1.0f, 0, 0.5f, FIX, kGoldVine, 2.8f, 0.05f, COMET));

LOOK(kRngEnrichment, "enrichment",
	G(RE, Glyph_Leaf, 0.4f, 5.0f, 3, 6, GA_RingRise, 2.4f, FIX, kGoldVine, 2.6f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kGold, 2.2f, 0.06f, ADD, 1, 0, AE),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kGold, 2.4f));

LOOK(kRngAlliance, "alliance",
	G(IM, Glyph_Link, 0.4f, 7.0f, 4, 3, GA_PopSpin, 0.8f, FIX, kRngHunter, 3.0f),
	F(IM, FxForm_Rune, 1.6f, 0, 60, 3, 1, 9, 0, FIX, kGold, 3.0f, 0.05f),
	F(IM, FxForm_Ring, 1.3f, 5.0f, 40, 0, 1.4f, 1.0f, 0.5f, FIX, kRngHunter, 2.6f, 0.05f, COMET));

LOOK(kRngFalcons, "spirit of falcons",
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kRngEagle, 2.0f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Feather, 0.45f, 5.0f, 3, 6, GA_RingRise, 2.2f, FIX, kFeather, 2.6f),
	F(IM, FxForm_Wings, 1.0f, 4.3f, 58, 6, 1.2f, 2.2f, 0, FIX, kRngEagle, 2.4f, 0.05f),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 60, 10, 3.0f, 0, 0, FIX, kAir, 2.2f, 0.05f));

LOOK(kRngSwarmCall, "swarm of",
	G(RE, Glyph_Insect, 0.35f, 3.0f, 4, 6, GA_RingRise, 1.6f, FIX, kRngBee, 2.0f),
	F(TR, FxForm_Drain, 1, 0, 64, 4, 1.8f, 16, 0, FIX, kRngBee, 1.6f, 0.06f),
	F(IM, FxForm_Orbit, 1.5f, 7.0f, 96, 20, 0, 0, 0, FIX, kRngBee, 3.0f, 0.04f, ADD, 4.0f));

LOOK(kRngSwarmPet, "swarm",
	G(IM, Glyph_Insect, 0.5f, 5.0f, 4, 4, GA_Orbit | GlyphAnim_Ascend, 1.2f, FIX, kRngBee, 2.0f),
	F(IM, FxForm_Orbit, 1.2f, 5.0f, 48, 12, 0, 0, 0, FIX, kRngBee, 2.0f, 0.04f, ADD, 3.5f));

LOOK(kRngSwarmGuard, "guardian swarm",
	F(IM, FxForm_Orbit, 1.4f, 6.0f, 64, 14, 0, 0, 0, FIX, kRngBee, 3.0f, 0.04f, ADD, 2.5f),
	G(IM, Glyph_Shield, 0.5f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kRngBee, 1.6f));

LOOK(kRngHuntProc, "hunt effect",
	G(IM, Glyph_Paw, 0.35f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kRngHunter, 1.2f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 20, 10, 0.10f, 0, 0, FIX, kRngHunter, 1.0f));

LOOK(kRngRoots, "roots",
	G(IM, Glyph_Shackle, 0.6f, 0.8f, 3, 2, GlyphAnim_Pop, 0.45f, FIX, kWood, 4.0f),
	F(AU, FxForm_Vines, 1.0f, 2.0f, 120, 6, 0.4f, 0, 1.0f, FIX, kWood, 5.0f, 0.07f),
	F(IM, FxForm_Cracks, 1.6f, 0, 40, 5, 1.0f, 0, 0, FIX, kEarth, 3.0f, 0.05f, OVER));

LOOK(kRngSnare, "snare",
	G(IM, Glyph_Link, 0.35f, 0.6f, 3, 6, GA_PopSpin, 1.1f, FIX, kNature, 4.0f),
	F(IM, FxForm_Vines, 0.9f, 1.2f, 96, 4, 0.6f, 0, 1, FIX, kRngMoss, 4.0f, 0.05f),
	G(IM, Glyph_Hourglass, 0.5f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kNature, 2.0f));

LOOK(kRngSummoned, "ward summoned",
	G(IM, Glyph_Pentagram, 1.2f, 0.0f, 2, 1, GA_PopRoll, 0, FIX, kNature, 2.4f),
	F(IM, FxForm_Vines, 1.0f, 3.0f, 96, 5, 0.6f, 0, 1, FIX, kWood, 2.4f, 0.06f),
	F(IM, FxForm_Burst, 2.2f, 2.0f, 64, 2, 1.0f, 12, 0, FIX, kNature, 0.9f, 0.09f));

LOOK(kRngBreather, "breather",
	G(IM, Glyph_Leaf, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kNature, 3.0f),
	F(IM, FxForm_Wind, 1.0f, 6.5f, 48, 8, 1.5f, 0, 0, FIX, kAir, 3.0f, 0.04f),
	F(IM, FxForm_Bloom, 1.6f, 1.0f, 40, 2, 0, 0, 0, FIX, kRngMist, 3.0f, 0.04f));

LOOK(kAaAuspice, "auspice of the hunter",
	G(RE, Glyph_Eye, 1.0f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kRngHunter, 2.6f),
	F(RE, FxForm_Rune, 2.6f, 0, 70, 6, 1, 12, 0, FIX, kGold, 2.6f, 0.05f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kRngHunter, 1.4f, 0.09f, ADD, 1, 0.2f, AE),
	G(RE, Glyph_Paw, 0.4f, 4.0f, 3, 6, GA_RingRise, 2.4f, FIX, kRngTawny, 2.6f));

LOOK(kAaBindSight, "bind sight",
	G(IM, Glyph_Eye, 0.9f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kArcane, 2.6f),
	F(IM, FxForm_Rune, 1.2f, 7.5f, 60, 6, 1, 12, 1, FIX, kArcane, 2.6f, 0.04f),
	F(TR, FxForm_Drain, 1, 0, 32, 2, 1.0f, 8, 0, FIX, kArcane, 1.4f, 0.05f));

LOOK(kAaBrownies, "bulwark of the brownies",
	F(IM, FxForm_Wings, 0.6f, 4.3f, 40, 5, 2.5f, 1.4f, 0, FIX, kRngBrownie, 3.0f, 0.04f),
	G(IM, Glyph_Shield, 0.8f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kRngBrownie, 3.0f),
	F(IM, FxForm_Orbit, 1.4f, 6.0f, 48, 10, 0, 0, 0, FIX, kRngBrownie, 3.0f, 0.04f, ADD, 3.0f),
	F(IM, FxForm_Sparkle, 1.4f, 7.0f, 60, 30, 0.12f, 0, 0, FIX, kRngBrownie, 3.0f));

LOOK(kAaCamouflage, "camouflage",
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kRngMoss, 2.5f, 0.05f),
	G(IM, Glyph_Leaf, 0.35f, 5.0f, 0, 5, GA_Orbit, 1.3f, FIX, kNature, 2.4f),
	F(IM, FxForm_Dome, 1.6f, 6.5f, 96, 6, 2, 0, 0, FIX, kRngMoss, 2.0f, 0.03f));

LOOK(kAaSharedCamo, "shared camouflage",
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kRngMoss, 2.4f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Leaf, 0.4f, 4.0f, 3, 8, GA_RingRise, 2.4f, FIX, kNature, 2.6f),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kRngMoss, 2.2f, 0.05f));

LOOK(kAaChameleon, "chameleon's gift",
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kPrism, 2.5f, 0.05f),
	G(IM, Glyph_Eye, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kPrism, 2.2f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kPrism, 2.4f));

LOOK(kAaConvergence, "convergence of spirits",
	G(IM, Glyph_Leaf, 1.0f, 8.5f, 4, 1, GA_Drop, 0, FIX, kHeal, 2.4f),
	F(IM, FxForm_Sparkle, 1.4f, 6.0f, 60, 30, 0.12f, 2, 0, FIX, kHeal, 2.4f),
	G(IM, Glyph_Spirit, 0.4f, 6.0f, 4, 4, GA_PopSpin, 1.4f, FIX, kRngDew, 2.4f),
	F(IM, FxForm_Flame, 0.85f, 9.0f, 80, 20, 0.1f, 2.8f, 0, FIX, kHeal, 2.0f, 0.07f, ADD, 1, 0.4f));

LOOK(kAaCoverTracks, "cover tracks",
	G(IM, Glyph_Paw, 0.35f, 0.0f, 2, 5, GlyphAnim_Pop, 1.2f, FIX, kRngMist, 2.4f),
	F(IM, FxForm_Wind, 1.3f, 2.0f, 48, 8, 2.0f, 0, 0, FIX, kRngMist, 2.4f, 0.05f, OVER),
	F(IM, FxForm_Bloom, 1.8f, 0.3f, 40, 2, 0, 0, 0, FIX, kRngMist, 2.2f, 0.04f, OVER));

LOOK(kAaElementalArrow, "elemental arrow",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.3f, 1.0f, FIX, kPrism, 0.7f, 0.10f),
	F(TR, FxForm_Bolt, 1, 0, 48, 1, 0, 0.12f, 1.0f, FIX, kRngFletch, 0.7f, 0.08f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 64, 2, 0.4f, 14, 0, FIX, kPrism, 0.9f, 0.09f),
	F(IM, FxForm_Sparkle, 1.2f, 5.0f, 32, 16, 0.12f, 0, 0, FIX, kPrism, 1.4f));

LOOK(kAaEmpoweredBlades, "empowered blades",
	G(IM, Glyph_Sword, 0.55f, 3.5f, 3, 3, GA_Orbit, 1.4f, FIX, kGold, 3.0f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.9f, 0.8f, 0, FIX, kShock, 2.6f, 0.06f),
	F(IM, FxForm_Spire, 0.8f, 7.5f, 64, 12, 1.0f, 2.2f, 0, FIX, kGold, 1.6f, 0.05f));

LOOK(kAaEntrap, "entrap",
	F(IM, FxForm_Thorns, 1.0f, 1.6f, 96, 5, 0.5f, 3, 1, FIX, kThorn, 4.0f, 0.05f),
	G(IM, Glyph_Link, 0.35f, 0.8f, 3, 5, GA_PopSpin, 1.0f, FIX, kWood, 4.0f),
	F(IM, FxForm_Ring, 1.0f, 0.4f, 40, 0, 0.3f, 1.0f, 0, FIX, kNature, 4.0f, 0.04f));

LOOK(kAaEntropyNature, "entropy of nature",
	F(IM, FxForm_Vortex, 1.2f, 6.0f, 72, 4, 1.4f, 2.0f, -3.0f, FIX, kDisease, 2.4f, 0.05f),
	G(IM, Glyph_Leaf, 0.35f, 8.0f, 4, 6, GA_Rain, 1.3f, FIX, kDisease, 2.4f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.10f, 1, 0, FIX, kAmber, 2.0f));

LOOK(kAaFlamingArrows, "flaming arrows",
	G(IM, Glyph_ArrowUp, 0.4f, 4.0f, 3, 3, GA_Orbit, 1.3f, FIX, kFire, 3.0f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 96, 24, 0.8f, 0.9f, 0, FIX, kFire, 3.0f, 0.08f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kForge, 2.6f));

LOOK(kAaPoisonArrows, "poison arrows",
	G(IM, Glyph_ArrowUp, 0.4f, 4.0f, 3, 3, GA_Orbit, 1.3f, FIX, kPoison, 3.0f),
	G(IM, Glyph_Snake, 0.7f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kPoison, 2.6f),
	F(IM, FxForm_Bubbles, 1.0f, 6.0f, 64, 10, 0.2f, 0, 0, FIX, kPoison, 2.6f, 0.035f));

LOOK(kAaGladestrider, "gladestrider's leap",
	F(RE, FxForm_Wind, 1.5f, 4.0f, 60, 12, 3.0f, 0, 0, FIX, kNature, 1.2f),
	F(RE, FxForm_Burst, 2.6f, 0.2f, 64, 2, 0.3f, 12, 0, FIX, kWood, 1.0f, 0.09f),
	G(RE, Glyph_Leaf, 0.3f, 1.0f, 0, 6, GA_RingRise, 1.4f, FIX, kNature, 1.6f));

LOOK(kAaGraspSylvan, "grasp of sylvan spirits",
	F(RE, FxForm_Vines, 1.0f, 2.0f, 120, 8, 0.5f, 0, 1.0f, FIX, kWood, 3.0f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Spirit, 0.5f, 4.0f, 4, 5, GA_RingRise, 2.2f, FIX, kNature, 3.0f),
	F(RE, FxForm_Rune, 1.0f, 0, 80, 5, 2, 10, 0, FIX, kNature, 3.0f, 0.05f, ADD, 1, 0, AE));

LOOK(kAaGuardian, "guardian of the forest",
	G(IM, Glyph_Tree, 1.6f, 9.5f, 4, 1, GA_PopBob, 0, FIX, kNature, 3.5f),
	F(IM, FxForm_Vines, 1.1f, 7.0f, 120, 6, 1.2f, 0, 0, FIX, kWood, 3.5f, 0.07f),
	F(IM, FxForm_Flame, 1.1f, 7.0f, 112, 28, 0.8f, 1.2f, 0, FIX, kNature, 3.0f, 0.08f),
	Persist(F(AU, FxForm_Ring, 1.0f, 0.3f, 40, 0, 1.0f, 1.0f, 0.5f, FIX, kNature, 0, 0.05f, COMET)));

LOOK(kAaGroupGuardian, "group guardian of the forest",
	G(RE, Glyph_Tree, 1.3f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kNature, 3.0f),
	F(RE, FxForm_Vines, 1.0f, 3.0f, 120, 8, 0.8f, 0, 1.0f, FIX, kWood, 3.0f, 0.06f, ADD, 1, 0, AE),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 1.5f, 20, 0, FIX, kNature, 1.4f, 0.10f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.8f, 1.0f, 0, FIX, kNature, 2.4f, 0.07f));

LOOK(kAaHarmonious, "harmonious arrow",
	F(TR, FxForm_Bolt, 1, 0, 64, 1, 0, 0.12f, 1.2f, FIX, kRngFletch, 0.6f, 0.09f),
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.4f, 1.2f, FIX, kAurora, 0.6f, 0.06f),
	F(IM, FxForm_Burst, 2.0f, 3.0f, 64, 2, 0.6f, 12, 0, FIX, kAurora, 0.9f, 0.08f),
	F(CH, FxForm_Helix, 1.0f, 6.0f, 64, 2, 1.2f, 0, 0.5f, FIX, kAurora, 2.0f, 0.05f, COMET));

LOOK(kAaImbued, "imbued ferocity",
	G(IM, Glyph_Fang, 1.0f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kBlood, 2.6f),
	F(IM, FxForm_Flame, 1.05f, 6.0f, 96, 24, 0.8f, 1.0f, 0, FIX, kZeal, 3.0f, 0.08f),
	F(IM, FxForm_Burst, 2.4f, 0.3f, 64, 2, 0.4f, 14, 0, FIX, kZeal, 0.9f, 0.09f));

LOOK(kAaOutriderAccuracy, "outrider's accuracy",
	G(IM, Glyph_Eye, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kRngEagle, 3.0f),
	F(IM, FxForm_Rune, 1.6f, 5.0f, 60, 4, 2, 16, 1, FIX, kGold, 3.0f, 0.05f),
	F(IM, FxForm_Spire, 0.8f, 5.0f, 64, 12, 0.8f, 2.0f, 0, FIX, kRngFletch, 1.4f, 0.05f));

LOOK(kAaOutriderEvasion, "outrider's evasion",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 14, 3.5f, 0, 0, FIX, kRngDusk, 2.4f, 0.05f),
	G(IM, Glyph_Feather, 0.35f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kRngDusk, 2.6f),
	F(IM, FxForm_Dome, 1.6f, 6.5f, 72, 6, 2, 0, 0, FIX, kGhost, 2.4f, 0.03f));

LOOK(kAaPackHunt, "pack hunt",
	G(RE, Glyph_Wolf, 0.6f, 4.0f, 3, 4, GA_RingRise, 2.2f, FIX, kRngWolf, 3.0f),
	F(RE, FxForm_Rune, 2.6f, 0, 70, 8, 3, 16, 0, FIX, kRngWolf, 3.0f, 0.05f),
	F(RE, FxForm_Wind, 1.6f, 3.0f, 60, 10, 2.5f, 0, 0, FIX, kRngWolf, 2.0f, 0.05f),
	F(RE, FxForm_Burst, 3.0f, 0, 64, 2, 0.5f, 0, 0, FIX, kRngWolf, 1.2f, 0.09f, ADD, 1, 0.3f));

LOOK(kAaSpiritWolf, "protection of the spirit wolf",
	G(IM, Glyph_Wolf, 1.2f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kGhost, 3.0f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kRngWolf, 2.6f, 0.04f),
	F(IM, FxForm_Orbit, 1.4f, 6.0f, 48, 10, 0, 0, 0, FIX, kRngWolf, 3.0f, 0.04f, ADD, 2.0f));

LOOK(kAaCheetah, "scarlet cheetah fang",
	G(IM, Glyph_Fang, 1.0f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kRngCheetah, 2.6f),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 14, 3.5f, 0, 0, FIX, kRngCheetah, 2.6f, 0.05f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.9f, 0.8f, 0, FIX, kRngCheetah, 2.6f, 0.07f));

LOOK(kAaSilentStrikes, "silent strikes",
	G(IM, Glyph_Sword, 0.8f, 7.5f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 2.4f),
	F(IM, FxForm_Bloom, 2.0f, 3.0f, 48, 2, 0, 0, 0, FIX, kRngDusk, 2.2f, 0.05f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.10f, 1, 0, FIX, kRngDusk, 2.2f));

LOOK(kAaSpire, "spire of the pathfinders",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kNature, 2.6f, 0.10f),
	G(RE, Glyph_ArrowUp, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kRngFletch, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kRngHunter, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaSpiritEagle, "spirit of eagle",
	F(IM, FxForm_Wings, 1.0f, 4.4f, 70, 10, 0.6f, 4.0f, 0, FIX, kRngEagle, 3.0f, 0.06f),
	F(IM, FxForm_Wind, 1.4f, 5.0f, 72, 12, 3.5f, 0, 0, FIX, kAir, 3.0f, 0.05f),
	G(IM, Glyph_Feather, 0.45f, 7.0f, 0, 1, GA_PopAscend, 0, FIX, kRngEagle, 1.8f));

LOOK(kAaUnity, "wildstalker's unity",
	G(IM, Glyph_Leaf, 1.0f, 8.5f, 4, 1, GA_PopRoll, 0, FIX, kGoldVine, 3.0f),
	F(IM, FxForm_Rune, 2.2f, 0, 70, 8, 3, 16, 0, FIX, kNature, 3.0f, 0.04f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 64, 2.5f, 3.0f, 0, 1.0f, FIX, kGoldVine, 2.4f, 0.05f, COMET));

LOOK(kAaHeavyArrow, "heavy arrow",
	F(TR, FxForm_Bolt, 1, 0, 48, 1, 0, 0.15f, 0.8f, FIX, kIron, 0.6f, 0.16f),
	F(IM, FxForm_Burst, 2.0f, 3.5f, 48, 1, 0.2f, 10, 0, FIX, kIron, 0.6f, 0.09f),
	F(IM, FxForm_Cracks, 1.2f, 0, 32, 4, 1.0f, 0, 0, FIX, kEarth, 1.2f, 0.05f, OVER));

LOOK(kAaHuntersFury, "hunter's fury",
	G(IM, Glyph_Claw, 0.5f, 6.0f, 4, 1, GA_PopAscend, 0, FIX, kBlood, 1.0f),
	F(IM, FxForm_Burst, 1.4f, 3.5f, 32, 1, 0, 8, 0, FIX, kRngTawny, 0.5f, 0.07f));

LOOK(kAaSynergy, "outrider's synergy",
	G(IM, Glyph_Star, 0.4f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kRngHunter, 1.2f),
	F(IM, FxForm_Ring, 1.0f, 3.0f, 32, 0, 2.0f, 1.0f, 0.8f, FIX, kRngHunter, 1.0f, 0.04f, COMET));

enum Line : int
{
	Balm, Spring, Desperate, Lingering, Summer, SummerBurn, SummerSplash, Vigor, Wind, Ash,
	Heartshot, Heartpierce, Swarm, Flusterbolt, Vinelash, Entropy, FocusedArrows, StormArrows,
	ArcArrows, Aimshot, Shot, CalledShots, Fusillade, JoltKick, JoltStrike, EnrageKick, Blades,
	Devastating, StormBlades, FocusedBlades, Reflexive, ShieldOf, Cloak, SharedCloak, Coat,
	Nettlecoat, Skin, Protection, Strength, Predator, Shout, Eyes, Sagacity, Drifting, Veil,
	WindBoon, FireBoon, BurningCloak, Hunt, Discipline, Unity, Enrichment, Alliance, Falcons,
	SwarmCall, SwarmPet, SwarmGuard, HuntProc, Roots, Snare, Summoned, Breather,
	AaAuspice, AaBindSight, AaBrownies, AaCamouflage, AaSharedCamo, AaChameleon, AaConvergence,
	AaCoverTracks, AaElementalArrow, AaEmpoweredBlades, AaEntrap, AaEntropyNature, AaFlamingArrows,
	AaPoisonArrows, AaGladestrider, AaGraspSylvan, AaGuardian, AaGroupGuardian, AaHarmonious,
	AaImbued, AaOutriderAccuracy, AaOutriderEvasion, AaPackHunt, AaSpiritWolf, AaCheetah,
	AaSilentStrikes, AaSpire, AaSpiritEagle, AaUnity, AaHeavyArrow, AaHuntersFury, AaSynergy,
};

const LineLook kLines[] = {
	{ "Balm (heal)",                    &kRngBalm },
	{ "Spring (heal)",                  &kRngSpring },
	{ "Desperate (emergency heal)",     &kRngDesperate },
	{ "Lingering (heal over time)",     &kRngLingering },
	{ "Summer's (nuke, group heal)",    &kRngSummer },
	{ "Summer's Burn (trigger)",        &kRngSummerBurn },
	{ "Summer's Splash (trigger)",      &kRngSummerSplash },
	{ "Vigor (regeneration)",           &kRngVigor },
	{ "Wind (cold nuke)",               &kRngWind },
	{ "Ash (fire nuke)",                &kRngAsh },
	{ "Heartshot (arrow nuke)",         &kRngHeartshot },
	{ "Heartpierce (nuke)",             &kRngHeartpierce },
	{ "Swarm (damage over time)",       &kRngSwarm },
	{ "Flusterbolt (stun)",             &kRngFlusterbolt },
	{ "Vinelash",                       &kRngVinelash },
	{ "Nature's Entropy",               &kRngEntropy },
	{ "Focused Arrows (discipline)",    &kRngFocusedArrows },
	{ "Storm of Arrows (area)",         &kRngStormArrows },
	{ "Arc of Arrows (discipline)",     &kRngArcArrows },
	{ "Aimshot (discipline)",           &kRngAimshot },
	{ "Shot (hate arrow)",              &kRngShot },
	{ "Called Shots (discipline)",      &kRngCalledShots },
	{ "Fusillade (arrows, heal)",       &kRngFusillade },
	{ "Jolting Kicks (hate reduction)", &kRngJoltKick },
	{ "Jolting Strikes (hate reduction)", &kRngJoltStrike },
	{ "Enraging Kicks (hate)",          &kRngEnrageKick },
	{ "Blades (weapon proc)",           &kRngBlades },
	{ "Devastating (discipline)",       &kRngDevastating },
	{ "Storm of Blades (discipline)",   &kRngStormBlades },
	{ "Focused Blades (discipline)",    &kRngFocusedBlades },
	{ "Reflexive (damage shield)",      &kRngReflexive },
	{ "Shield of (damage shield)",      &kRngShieldOf },
	{ "Cloak of (damage shield)",       &kRngCloak },
	{ "Shared Cloak (group)",           &kRngSharedCloak },
	{ "Coat (damage shield)",           &kRngCoat },
	{ "Grasping Nettlecoat",            &kRngNettlecoat },
	{ "Skin / Scale (armor)",           &kRngSkin },
	{ "Protection (hit points)",        &kRngProtection },
	{ "Strength (attack)",              &kRngStrength },
	{ "Predator (group attack)",        &kRngPredator },
	{ "Shout (group buff)",             &kRngShout },
	{ "Eyes (archery)",                 &kRngEyes },
	{ "Skylight Sagacity",              &kRngSagacity },
	{ "Drifting (hate reduction)",      &kRngDrifting },
	{ "Veil (discipline)",              &kRngVeil },
	{ "Wind Boon",                      &kRngWindBoon },
	{ "Fire Boon",                      &kRngFireBoon },
	{ "Burning Cloak (fire proc)",      &kRngBurningCloak },
	{ "By the Hunt (proc buff)",        &kRngHunt },
	{ "Stalker's Discipline",           &kRngDiscipline },
	{ "Unity",                          &kRngUnity },
	{ "Enrichment (group)",             &kRngEnrichment },
	{ "Alliance (debuff)",              &kRngAlliance },
	{ "Spirit of Falcons (movement)",   &kRngFalcons },
	{ "Swarm of (swarm pets)",          &kRngSwarmCall },
	{ "swarm pet",                      &kRngSwarmPet },
	{ "guardian swarm proc",            &kRngSwarmGuard },
	{ "hunt proc",                      &kRngHuntProc },
	{ "Roots (root)",                   &kRngRoots },
	{ "Snare",                          &kRngSnare },
	{ "Ward Summoned",                  &kRngSummoned },
	{ "Breather (discipline)",          &kRngBreather },

	{ "AA: Auspice of the Hunter",      &kAaAuspice },
	{ "AA: Bind Sight",                 &kAaBindSight },
	{ "AA: Bulwark of the Brownies",    &kAaBrownies },
	{ "AA: Innate Camouflage",          &kAaCamouflage },
	{ "AA: Shared Camouflage",          &kAaSharedCamo },
	{ "AA: Chameleon's Gift",           &kAaChameleon },
	{ "AA: Convergence of Spirits",     &kAaConvergence },
	{ "AA: Cover Tracks",               &kAaCoverTracks },
	{ "AA: Elemental Arrow",            &kAaElementalArrow },
	{ "AA: Empowered Blades",           &kAaEmpoweredBlades },
	{ "AA: Entrap",                     &kAaEntrap },
	{ "AA: Entropy of Nature",          &kAaEntropyNature },
	{ "AA: Flaming Arrows",             &kAaFlamingArrows },
	{ "AA: Poison Arrows",              &kAaPoisonArrows },
	{ "AA: Gladestrider's Leap",        &kAaGladestrider },
	{ "AA: Grasp of Sylvan Spirits",    &kAaGraspSylvan },
	{ "AA: Guardian of the Forest",     &kAaGuardian },
	{ "AA: Group Guardian of the Forest", &kAaGroupGuardian },
	{ "AA: Harmonious Arrow",           &kAaHarmonious },
	{ "AA: Imbued Ferocity",            &kAaImbued },
	{ "AA: Outrider's Accuracy",        &kAaOutriderAccuracy },
	{ "AA: Outrider's Evasion",         &kAaOutriderEvasion },
	{ "AA: Pack Hunt",                  &kAaPackHunt },
	{ "AA: Protection of the Spirit Wolf", &kAaSpiritWolf },
	{ "AA: Scarlet Cheetah's Fang",     &kAaCheetah },
	{ "AA: Silent Strikes",             &kAaSilentStrikes },
	{ "AA: Spire of the Pathfinders",   &kAaSpire },
	{ "AA: Spirit of Eagles",           &kAaSpiritEagle },
	{ "AA: Wildstalker's Unity",        &kAaUnity },
	{ "AA: Heavy Arrow (proc)",         &kAaHeavyArrow },
	{ "AA: Hunter's Fury (proc)",       &kAaHuntersFury },
	{ "AA: Outrider's Synergy (proc)",  &kAaSynergy },
};

const GroupLine kNumbered[] = {
	{ 0, Balm },         { 1, ShieldOf },     { 2, JoltKick },     { 3, Swarm },        { 4, Spring },
	{ 5, Strength },     { 6, Wind },         { 7, Drifting },     { 8, Predator },     { 9, Ash },
	{ 10, Heartshot },   { 11, Summer },      { 12, Skin },        { 13, Protection },  { 14, Blades },
	{ 16, Eyes },        { 18, Cloak },       { 19, Sagacity },    { 20, Heartpierce }, { 21, JoltStrike },
	{ 22, Aimshot },     { 23, ArcArrows },   { 24, Devastating }, { 25, Shot },         { 26, Coat },
	{ 27, SummerBurn },  { 28, SummerSplash }, { 29, FocusedArrows }, { 30, SwarmCall }, { 31, StormArrows },
	{ 32, CalledShots }, { 33, Flusterbolt }, { 35, Lingering },   { 36, Vigor },       { 37, Desperate },
	{ 38, Hunt },        { 39, Nettlecoat },  { 40, Veil },        { 41, EnrageKick },  { 42, SharedCloak },
	{ 43, Reflexive },   { 44, WindBoon },    { 45, FireBoon },    { 46, StormBlades }, { 47, FocusedBlades },
	{ 48, Unity },       { 49, Shout },       { 50, BurningCloak }, { 51, Discipline }, { 52, Fusillade },
	{ 54, Enrichment },  { 55, Alliance },
};

const GroupLine kModern[] = {
	{ 10, Balm },        { 20, ShieldOf },    { 30, JoltKick },    { 40, Swarm },       { 50, Spring },
	{ 60, Strength },    { 70, Wind },        { 80, Drifting },    { 90, Predator },    { 100, Ash },
	{ 110, Heartshot },  { 120, Summer },     { 121, SummerBurn }, { 122, SummerSplash }, { 130, Skin },
	{ 140, Protection }, { 150, Blades },     { 160, Eyes },       { 170, Cloak },      { 200, JoltStrike },
	{ 220, ArcArrows },  { 230, Devastating }, { 250, Coat },      { 260, FocusedArrows }, { 270, SwarmCall },
	{ 280, StormArrows }, { 290, CalledShots }, { 300, Lingering }, { 310, Vigor },     { 320, Desperate },
	{ 330, Hunt },       { 350, Veil },       { 360, EnrageKick }, { 380, Reflexive },  { 390, WindBoon },
	{ 400, FireBoon },   { 410, StormBlades }, { 420, FocusedBlades }, { 440, Shout },  { 450, BurningCloak },
	{ 460, Discipline }, { 470, Fusillade },  { 480, Enrichment }, { 490, Alliance },   { 530, Shot },
};

const GroupLine kOther[] = {
	{ 4001, Balm },       { 4002, ShieldOf },   { 4004, JoltKick },   { 4005, Swarm },      { 4006, Spring },
	{ 4008, Strength },   { 4010, Wind },       { 4012, Drifting },   { 4013, Predator },   { 4014, Ash },
	{ 4015, Heartshot },  { 4016, Summer },     { 4018, Skin },       { 4019, Protection }, { 4020, Blades },
	{ 4022, Eyes },       { 4024, Hunt },       { 4026, Entropy },    { 2025, Vinelash },   { 5562, Vinelash },
	{ 104506, Aimshot },  { 1506986, Falcons }, { 20115, Summoned },  { 20248, Summoned },  { 21529, Summoned },
	{ 3311, Breather },   { 3411, Breather },   { 3511, Breather },   { 3611, Breather },   { 3737, Breather },
	{ 3837, Breather },   { 3937, Breather },
};

const NamedLine kAbilities[] = {
	{ "auspice of the hunter",                AaAuspice },
	{ "bind sight",                           AaBindSight },
	{ "bulwark of the brownies",              AaBrownies },
	{ "bulwark of the brownies jolt",         AaBrownies },
	{ "brownie's blessing",                   AaBrownies },
	{ "camouflage",                           AaCamouflage },
	{ "perfected camouflage",                 AaCamouflage },
	{ "shared camouflage",                    AaSharedCamo },
	{ "shared perfected camouflage",          AaSharedCamo },
	{ "chameleon's gift",                     AaChameleon },
	{ "chameleon's gift subtlety",            AaChameleon },
	{ "convergence of spirits",               AaConvergence },
	{ "cover tracks",                         AaCoverTracks },
	{ "elemental arrow",                      AaElementalArrow },
	{ "empowered blades",                     AaEmpoweredBlades },
	{ "entrap",                               AaEntrap },
	{ "entropy of nature",                    AaEntropyNature },
	{ "flaming arrows",                       AaFlamingArrows },
	{ "flaming arrows penance",               AaFlamingArrows },
	{ "flames of kesh`yk",                    AaFlamingArrows },
	{ "poison arrows",                        AaPoisonArrows },
	{ "poison arrows penance",                AaPoisonArrows },
	{ "poison of kesh`yk",                    AaPoisonArrows },
	{ "gladestrider's leap",                  AaGladestrider },
	{ "grasp of sylvan spirits",              AaGraspSylvan },
	{ "guardian of the forest",               AaGuardian },
	{ "guardian of the forest form",          AaGuardian },
	{ "group guardian of the forest",         AaGroupGuardian },
	{ "group guardian of the forest form",    AaGroupGuardian },
	{ "harmonious arrow",                     AaHarmonious },
	{ "harmonious arrow harmony",             AaHarmonious },
	{ "imbued ferocity",                      AaImbued },
	{ "imbued ferocity subtlety",             AaImbued },
	{ "outrider's accuracy",                  AaOutriderAccuracy },
	{ "outrider's evasion",                   AaOutriderEvasion },
	{ "pack hunt",                            AaPackHunt },
	{ "protection of the spirit wolf",        AaSpiritWolf },
	{ "scarlet cheetah fang",                 AaCheetah },
	{ "greater scarlet cheetah fang",         AaCheetah },
	{ "scarlet cheetah's twincast",           AaCheetah },
	{ "silent strikes",                       AaSilentStrikes },
	{ "spire of the pathfinders",             AaSpire },
	{ "apex of the pathfinders",              AaSpire },
	{ "pathfinder's subtlety",                AaSpire },
	{ "spirit of eagle",                      AaSpiritEagle },
	{ "wildstalker's unity",                  AaUnity },
	{ "heavy arrow",                          AaHeavyArrow },
	{ "hunter's fury",                        AaHuntersFury },
	{ "fury of the forest",                   AaHuntersFury },
	{ "outrider's synergy",                   AaSynergy },
	{ "outrider's synergy strike",            AaSynergy },
};

const NamedLine kClassic[] = {
	{ "salve",                  Balm },         { "minor healing",          Balm },
	{ "light healing",          Balm },         { "healing",                Balm },
	{ "greater healing",        Balm },         { "chloroblast",            Balm },
	{ "sylvan light",           Balm },         { "sylvan water",           Spring },
	{ "chloroplast",            Vigor },        { "regrowth",               Vigor },
	{ "hunter's vigor",         Vigor },
	{ "icewind",                Wind },         { "frost wind",             Wind },
	{ "frozen wind",            Wind },         { "ancient: north wind",    Wind },
	{ "flame lick",             Ash },          { "burst of fire",          Ash },
	{ "ignite",                 Ash },          { "immolate",               Ash },
	{ "firestrike",             Ash },          { "brushfire",              Ash },
	{ "sylvan burn",            Ash },          { "hearth embers",          Ash },
	{ "scorched earth",         Ash },          { "ancient: burning chaos", Ash },
	{ "flaming arrow",          Heartshot },    { "burning arrow",          Heartshot },
	{ "stinging swarm",         Swarm },        { "swarm of pain",          Swarm },
	{ "drones of doom",         Swarm },        { "fire swarm",             Swarm },
	{ "drifting death",         Swarm },        { "locust swarm",           Swarm },
	{ "hail of arrows",         StormArrows },
	{ "jolt",                   JoltStrike },   { "cinder jolt",            JoltStrike },
	{ "jolting blades",         JoltStrike },
	{ "call of sky",            Blades },       { "call of flame",          Blades },
	{ "call of fire",           Blades },       { "call of earth",          Blades },
	{ "call of ice",            Blades },       { "call of the rathe",      Blades },
	{ "call of lightning",      Blades },
	{ "shield of thistles",     ShieldOf },     { "shield of brambles",     ShieldOf },
	{ "shield of spikes",       ShieldOf },     { "shield of thorns",       ShieldOf },
	{ "shield of briar",        ShieldOf },
	{ "thistlecoat",            Coat },         { "barbcoat",               Coat },
	{ "bramblecoat",            Coat },         { "spikecoat",              Coat },
	{ "thorncoat",              Coat },         { "bladecoat",              Coat },
	{ "briarcoat",              Coat },
	{ "skin like wood",         Skin },         { "skin like rock",         Skin },
	{ "skin like steel",        Skin },         { "skin like diamond",      Skin },
	{ "skin like nature",       Skin },         { "natureskin",             Skin },
	{ "onyx skin",              Skin },
	{ "riftwind's protection",  Protection },   { "warder's protection",    Protection },
	{ "protection of the wild", Protection },   { "ward of the hunter",     Protection },
	{ "strength of earth",      Strength },     { "strength of nature",     Strength },
	{ "storm strength",         Strength },     { "strength of tunare",     Strength },
	{ "strength of the hunter", Strength },
	{ "mark of the predator",   Predator },     { "call of the predator",   Predator },
	{ "spirit of the predator", Predator },     { "howl of the predator",   Predator },
	{ "hawk eye",               Eyes },         { "falcon eye",             Eyes },
	{ "eagle eye",              Eyes },
	{ "nature veil",            Drifting },
	{ "displace summoned",      Summoned },
	{ "tangling weeds",         Snare },        { "snare",                  Snare },
	{ "ensnare",                Snare },
	{ "grasping roots",         Roots },        { "ensnaring roots",        Roots },
	{ "tangle",                 Roots },        { "enveloping roots",       Roots },
	{ "entangle",               Roots },        { "earthen embrace",        Roots },
	{ "earthen roots",          Roots },        { "earthen shackles",       Roots },
	{ "elddar's grasp",         Roots },
};

int BySpell(const eqlib::EQ_Spell*, std::string_view base)
{
	for (const NamedLine& c : kClassic)
		if (base == c.name)
			return c.line;
	if (StartsWith(base, "cloud of "))
		return SwarmGuard;
	if (EndsWith(base, " swarm"))
		return SwarmPet;
	if (StartsWith(base, "summer's "))
	{
		if (EndsWith(base, " burn"))
			return SummerBurn;
		if (EndsWith(base, " splash"))
			return SummerSplash;
	}
	if (EndsWith(base, " effect"))
		return HuntProc;
	return -1;
}

} // namespace

const ClassCatalogue& RangerCatalogue()
{
	static const ClassCatalogue c{ "Ranger", eqlib::Ranger, kLines, 11, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
