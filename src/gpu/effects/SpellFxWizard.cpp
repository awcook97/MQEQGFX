#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

#include <initializer_list>

namespace eqgfx::spellfx {

namespace {

constexpr Pair kEther    { { 0.95f, 0.90f, 1.00f, 0.95f }, { 0.55f, 0.35f, 1.00f, 0.85f } };
constexpr Pair kRime     { { 1.00f, 1.00f, 1.00f, 0.95f }, { 0.55f, 0.85f, 1.00f, 0.85f } };
constexpr Pair kFrost    { { 0.75f, 0.92f, 1.00f, 0.95f }, { 0.15f, 0.45f, 0.95f, 0.85f } };
constexpr Pair kCrystal  { { 0.90f, 0.98f, 1.00f, 0.95f }, { 0.55f, 0.70f, 1.00f, 0.85f } };
constexpr Pair kWild     { { 0.40f, 1.00f, 0.70f, 0.95f }, { 1.00f, 0.30f, 0.80f, 0.85f } };
constexpr Pair kChaos    { { 1.00f, 0.55f, 0.20f, 0.95f }, { 0.60f, 0.10f, 0.70f, 0.85f } };
constexpr Pair kPlasma   { { 0.85f, 1.00f, 1.00f, 1.00f }, { 0.55f, 0.40f, 1.00f, 0.90f } };
constexpr Pair kVoid     { { 0.55f, 0.35f, 0.95f, 0.90f }, { 0.05f, 0.02f, 0.15f, 0.85f } };
constexpr Pair kSolar    { { 1.00f, 0.98f, 0.75f, 1.00f }, { 1.00f, 0.55f, 0.10f, 0.90f } };
constexpr Pair kCinder   { { 1.00f, 0.60f, 0.15f, 1.00f }, { 0.45f, 0.05f, 0.00f, 0.90f } };
constexpr Pair kStorm    { { 0.90f, 0.95f, 1.00f, 1.00f }, { 0.30f, 0.35f, 0.85f, 0.90f } };
constexpr Pair kMist     { { 0.80f, 0.85f, 0.90f, 0.70f }, { 0.40f, 0.45f, 0.55f, 0.50f } };
constexpr Pair kXuzl     { { 0.85f, 0.75f, 1.00f, 1.00f }, { 0.35f, 0.10f, 0.60f, 0.90f } };
constexpr Pair kFamiliar { { 0.80f, 0.95f, 1.00f, 0.90f }, { 0.60f, 0.40f, 1.00f, 0.80f } };

constexpr uint32_t GA_Emerge = GlyphAnim_Rise | GlyphAnim_Pop;

LOOK(kWizEthereal, "ethereal fire",
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 2, 0.1f, 1.0f, 1.0f, FIX, kEther, 1.0f, 0.30f),
	F(IM, FxForm_Flame, 1.2f, 7.5f, 112, 28, 0.6f, 1.8f, 0, FIX, kSolar, 1.8f, 0.10f, ADD, 1, 0.2f),
	F(IM, FxForm_Burst, 3.0f, 1.0f, 96, 3, 2.0f, 20, 0, FIX, kEther, 1.0f, 0.14f, ADD, 1, 0.2f),
	F(IM, FxForm_Sparkle, 1.6f, 9.0f, 48, 24, 0.14f, 0, 0, FIX, kWhite, 2.0f, 0.06f, ADD, 1, 0.3f));

LOOK(kWizTears, "tears of fire",
	G(IM, Glyph_Drop, 0.35f, 10.0f, 4, 6, GA_Rain, 1.2f, FIX, kFire, 1.8f),
	F(IM, FxForm_Flame, 1.0f, 5.0f, 80, 20, 0.7f, 1.0f, 0, FIX, kFire, 1.6f, 0.08f, ADD, 1, 0.4f),
	F(IM, FxForm_Burst, 2.2f, 0.4f, 72, 2, 0.6f, 12, 0, FIX, kCinder, 0.9f, 0.10f, ADD, 1, 0.4f));

LOOK(kWizRingFire, "ring of fire",
	F(RE, FxForm_Ring, 1.0f, 0.4f, 96, 0, 1.0f, 1.0f, 0.4f, FIX, kFire, 1.6f, 0.12f, COMET, 1, 0, AE),
	F(RE, FxForm_Flame, 1.0f, 3.5f, 128, 32, 0.1f, 1.0f, 0, FIX, kFire, 1.8f, 0.09f, ADD, 1, 0.1f, AE),
	F(RE, FxForm_Burst, 1.0f, 0.3f, 96, 2, 0.8f, 0, 0, FIX, kCinder, 1.2f, 0.10f, ADD, 1, 0.2f, AE));

LOOK(kWizLureFire, "fire lure",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.7f, 0.5f, 1.0f, FIX, kFire, 0.8f, 0.18f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.8f, 1.0f, 0, FIX, kFire, 1.6f, 0.08f),
	G(IM, Glyph_ArrowDown, 0.35f, 6.0f, 0, 3, GA_Orbit, 1.2f, FIX, kCinder, 2.0f));

LOOK(kWizEruption, "volcanic eruption",
	F(IM, FxForm_Rain, 1.0f, 25.0f, 80, 70, 0, 0.35f, 0, FIX, kCinder, 3.0f, 0.10f, ADD, 1, 0, AE),
	G(IM, Glyph_Flame, 0.4f, 12.0f, 4, 6, GA_Rain, 3.0f, FIX, kFire, 2.4f),
	F(IM, FxForm_Cracks, 1.0f, 0, 64, 8, 1.4f, 0, 0, FIX, kCinder, 2.6f, 0.08f, ADD, 1, 0.8f, AE));

LOOK(kWizPyroblade, "pyroblade",
	G(IM, Glyph_Sword, 1.2f, 8.0f, 4, 1, GA_Drop, 0, FIX, kSolar, 1.6f),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 96, 24, 0.8f, 1.2f, 0, FIX, kFire, 1.6f, 0.09f, ADD, 1, 0.4f),
	F(IM, FxForm_Burst, 2.4f, 2.0f, 72, 1, 0.5f, 16, 0, FIX, kSolar, 0.8f, 0.10f, ADD, 1, 0.4f));

LOOK(kWizFlash, "flashfire",
	F(IM, FxForm_Burst, 2.6f, 3.0f, 96, 3, 3.0f, 24, 0, FIX, kSolar, 0.6f, 0.14f, ADD, 1.6f),
	F(IM, FxForm_Flame, 1.1f, 6.0f, 96, 24, 0.9f, 1.3f, 0, FIX, kFire, 1.0f, 0.09f, ADD, 1.6f));

LOOK(kWizFire, "named fire",
	F(TR, FxForm_Bolt, 1, 0, 128, 5, 0.6f, 0.5f, 1.4f, FIX, kFire, 0.8f, 0.26f),
	F(IM, FxForm_Flame, 1.3f, 8.0f, 128, 32, 0.6f, 1.8f, 0, FIX, kFire, 2.0f, 0.11f),
	G(IM, Glyph_Flame, 1.0f, 8.5f, 4, 1, GA_PopAscend, 0, FIX, kSolar, 1.8f, 0.2f),
	F(IM, FxForm_Burst, 3.0f, 1.0f, 96, 3, 1.5f, 20, 0, FIX, kCinder, 1.0f, 0.14f));

LOOK(kWizChaos, "chaos fire",
	F(IM, FxForm_Lightning, 1, 36.0f, 64, 3, 0.9f, 10.0f, 1.0f, FIX, kChaos, 0.8f, 0.14f),
	F(IM, FxForm_Flame, 1.1f, 6.5f, 96, 24, 0.7f, 1.4f, 0, FIX, kChaos, 1.6f, 0.09f, ADD, 1, 0.25f),
	G(IM, Glyph_Spiral, 0.8f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kChaos, 1.6f, 0.25f));

LOOK(kWizChaoticWildfire, "chaotic wildfire",
	F(IM, FxForm_Flame, 1.2f, 7.0f, 112, 28, 0.6f, 1.6f, 0, FIX, kWild, 1.8f, 0.10f),
	F(IM, FxForm_Burst, 2.8f, 1.5f, 96, 3, 1.5f, 18, 0, FIX, kFire, 1.0f, 0.12f),
	F(IM, FxForm_Sparkle, 1.5f, 8.0f, 48, 24, 0.14f, 0, 0, FIX, kWild, 1.8f));

LOOK(kWizBeam, "fire beam",
	F(RE, FxForm_Lightning, 1, 0, 96, 5, 0.15f, 24.0f, 0, FIX, kSolar, 1.0f, 0.30f),
	F(RE, FxForm_Burst, 1.2f, 3.0f, 48, 1, 0.2f, 12, 0, FIX, kFire, 0.8f, 0.08f),
	F(IM, FxForm_Flame, 1.0f, 5.0f, 80, 20, 0.8f, 1.0f, 0, FIX, kFire, 1.4f, 0.08f));

LOOK(kWizSelfImmolate, "self-immolation",
	F(RE, FxForm_Flame, 1.3f, 9.0f, 128, 32, 0.5f, 2.2f, 0, FIX, kSolar, 1.8f, 0.12f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 2.0f, 24, 0, FIX, kFire, 1.4f, 0.14f, ADD, 1, 0.3f, AE),
	G(RE, Glyph_Sun, 1.2f, 9.0f, 4, 1, GA_PopRoll, 0, FIX, kSolar, 1.6f));

LOOK(kWizSplash, "splash of brimstone",
	G(IM, Glyph_Rock, 0.9f, 10.0f, 4, 1, GA_Drop, 0, FIX, kCinder, 1.4f),
	F(IM, FxForm_Burst, 1.0f, 1.0f, 96, 2, 2.0f, 20, 0, FIX, kCinder, 1.2f, 0.10f, ADD, 1, 0.45f, AE),
	F(IM, FxForm_Cracks, 1.0f, 0, 64, 8, 1.2f, 0, 0, FIX, kFire, 2.0f, 0.07f, ADD, 1, 0.45f, AE));

LOOK(kWizFirePillar, "pillar of flame",
	F(IM, FxForm_Flame, 0.7f, 16.0f, 112, 16, 0.0f, 5.0f, 0, FIX, kFire, 2.0f, 0.12f),
	F(IM, FxForm_Spire, 1.0f, 14.0f, 96, 16, 0.6f, 3.0f, 0, FIX, kSolar, 1.6f, 0.06f),
	F(IM, FxForm_Burst, 2.4f, 0.3f, 72, 2, 0.4f, 12, 0, FIX, kCinder, 1.0f, 0.10f));

LOOK(kWizFireVortex, "fire vortex",
	F(IM, FxForm_Vortex, 1.3f, 6.0f, 96, 5, 1.2f, 2.5f, 5.0f, FIX, kFire, 3.0f, 0.07f),
	G(IM, Glyph_Flame, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kCinder, 3.0f));

LOOK(kWizClawFire, "claw of flame",
	G(IM, Glyph_Claw, 1.1f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kFire, 1.6f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.8f, 1.0f, 0, FIX, kCinder, 1.6f, 0.08f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 64, 1, 0.3f, 14, 0, FIX, kFire, 0.8f, 0.09f));

LOOK(kWizPermBlaze, "permeating blaze",
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 48, 24, 0.12f, 0, 0, FIX, kFire, 3.0f),
	F(IM, FxForm_Bloom, 1.6f, 1.5f, 48, 3, 0, 0, 0, FIX, kCinder, 2.4f, 0.05f),
	G(IM, Glyph_Flame, 0.6f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kFire, 3.0f));

LOOK(kWizLureCold, "cold lure",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.7f, 0.5f, 1.0f, FIX, kRime, 0.8f, 0.18f),
	F(IM, FxForm_Crystal, 1.1f, 5.0f, 60, 12, 0, 0.8f, 0, FIX, kFrost, 1.8f, 0.06f),
	G(IM, Glyph_ArrowDown, 0.35f, 6.0f, 0, 3, GA_Orbit, 1.2f, FIX, kRime, 2.0f));

LOOK(kWizSnap, "cold snap",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.4f, 0.5f, 0.8f, FIX, kCold, 0.7f, 0.18f),
	F(IM, FxForm_Crystal, 1.2f, 5.5f, 60, 12, 0, 1.0f, 0, FIX, kCold, 1.6f, 0.06f),
	G(IM, Glyph_Snowflake, 0.9f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kRime, 1.2f));

LOOK(kWizCascade, "ice cascade",
	F(IM, FxForm_Rain, 1.0f, 25.0f, 80, 70, 0, 0.1f, 0, FIX, kRime, 3.0f, 0.08f, ADD, 1, 0, AE),
	G(IM, Glyph_Snowflake, 0.4f, 12.0f, 4, 6, GA_Rain, 3.0f, FIX, kCold, 2.4f),
	F(IM, FxForm_Crystal, 1.0f, 3.0f, 96, 16, 1, 0.8f, 0, FIX, kFrost, 2.6f, 0.06f, ADD, 1, 0.8f, AE));

LOOK(kWizWildiceCascade, "wildice cascade",
	F(IM, FxForm_Rain, 1.0f, 25.0f, 80, 70, 0, 0.2f, 0, FIX, kWild, 3.0f, 0.08f, ADD, 1, 0, AE),
	G(IM, Glyph_Snowflake, 0.4f, 12.0f, 4, 6, GA_Rain, 3.0f, FIX, kWild, 2.4f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 96, 40, 0.12f, 1, 0, FIX, kRime, 2.6f, 0.06f, ADD, 1, 0.6f, AE));

LOOK(kWizClaw, "claw of ice",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.2f, 0.7f, 0.5f, FIX, kRime, 0.7f, 0.16f),
	G(IM, Glyph_Claw, 1.1f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kCold, 1.6f),
	F(IM, FxForm_Crystal, 1.2f, 5.0f, 60, 12, 0, 1.2f, 0, FIX, kFrost, 1.8f, 0.07f));

LOOK(kWizTorrent, "icicle torrent",
	F(TR, FxForm_Bolt, 1, 0, 128, 6, 0.9f, 0.6f, 1.2f, FIX, kFrost, 0.8f, 0.20f),
	F(IM, FxForm_Crystal, 1.4f, 6.0f, 80, 16, 0, 1.4f, 0, FIX, kRime, 2.0f, 0.07f),
	F(IM, FxForm_Burst, 2.8f, 1.5f, 96, 3, 1.5f, 20, 0, FIX, kCold, 1.0f, 0.12f));

LOOK(kWizFunnel, "funnel of frost",
	F(RE, FxForm_Lightning, 1, 0, 64, 3, 0.2f, 18.0f, 0, FIX, kRime, 0.9f, 0.20f),
	F(IM, FxForm_Vortex, 1.4f, 7.0f, 96, 5, 0.8f, 3.0f, -5.0f, FIX, kFrost, 1.8f, 0.07f),
	F(IM, FxForm_Crystal, 1.1f, 4.0f, 48, 10, 0, 0.8f, 0, FIX, kCold, 1.8f, 0.06f));

LOOK(kWizIceComet, "ice comet",
	F(TR, FxForm_Bolt, 1, 0, 128, 5, 0.4f, 0.7f, 8.0f, FIX, kRime, 1.0f, 0.28f),
	G(IM, Glyph_Snowflake, 1.2f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kCold, 0.9f),
	F(IM, FxForm_Crystal, 2.4f, 4.0f, 96, 16, 1, 1.2f, 0, FIX, kFrost, 2.4f, 0.08f),
	F(IM, FxForm_Burst, 3.2f, 0.5f, 96, 3, 0.8f, 20, 0, FIX, kRime, 1.0f, 0.14f));

LOOK(kWizFreeze, "mindfreeze",
	G(IM, Glyph_Snowflake, 0.6f, 8.2f, 4, 1, GA_PopRoll, 0, FIX, kRime, 2.0f),
	G(IM, Glyph_Spiral, 0.3f, 7.0f, 0, 3, GA_Orbit, 1.0f, FIX, kFrost, 2.0f),
	F(IM, FxForm_Crystal, 0.6f, 7.5f, 40, 8, 0, 0.5f, 0, FIX, kCold, 1.8f, 0.05f));

LOOK(kWizCrystalCircle, "crystallizing circle",
	F(RE, FxForm_Crystal, 1.0f, 3.0f, 112, 20, 1, 1.0f, 0, FIX, kCrystal, 2.2f, 0.07f, ADD, 1, 0, AE),
	F(RE, FxForm_Ring, 1.0f, 0.3f, 96, 0, 0.8f, 1.0f, 0.3f, FIX, kRime, 1.8f, 0.10f, COMET, 1, 0, AE),
	F(RE, FxForm_Wind, 1.4f, 4.0f, 48, 10, 2.5f, 0, 0, FIX, kRime, 1.2f));

LOOK(kWizFrostPillar, "pillar of ice",
	F(IM, FxForm_Crystal, 1.4f, 12.0f, 96, 10, 1, 3.0f, 0, FIX, kFrost, 2.2f, 0.09f),
	F(IM, FxForm_Spire, 1.0f, 14.0f, 96, 16, 0.6f, 3.0f, 0, FIX, kRime, 1.6f, 0.06f),
	F(IM, FxForm_Burst, 2.4f, 0.3f, 72, 2, 0.4f, 12, 0, FIX, kCold, 1.0f, 0.10f));

LOOK(kWizIceVortex, "ice vortex",
	F(IM, FxForm_Vortex, 1.3f, 6.0f, 96, 5, 1.2f, 2.5f, -5.0f, FIX, kCold, 3.0f, 0.07f),
	G(IM, Glyph_Snowflake, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kRime, 3.0f));

LOOK(kWizIceSlab, "ice slab",
	G(IM, Glyph_Rock, 1.6f, 10.0f, 4, 1, GA_Drop, 0, FIX, kRime, 1.6f),
	F(IM, FxForm_Cracks, 2.8f, 0, 64, 8, 1.2f, 0, 0, FIX, kFrost, 2.4f, 0.08f, ADD, 1, 0.45f),
	F(IM, FxForm_Burst, 3.0f, 0.4f, 96, 3, 0.6f, 20, 0, FIX, kCrystal, 1.0f, 0.12f, ADD, 1, 0.45f));

LOOK(kWizFrostbound, "frostbound",
	G(IM, Glyph_Link, 0.35f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kRime, 3.0f),
	F(IM, FxForm_Crystal, 1.0f, 4.0f, 48, 8, 0, 0.8f, 0, FIX, kFrost, 3.0f, 0.06f),
	F(IM, FxForm_Rune, 1.6f, 0, 60, 6, 1, 12, 0, FIX, kCold, 3.0f, 0.05f));

LOOK(kWizPermIce, "permeating ice",
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 48, 24, 0.12f, 1, 0, FIX, kRime, 3.0f),
	F(IM, FxForm_Bloom, 1.6f, 1.5f, 48, 3, 0, 0, 0, FIX, kFrost, 2.4f, 0.05f),
	G(IM, Glyph_Snowflake, 0.6f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kCold, 3.0f));

LOOK(kWizLureShock, "lightning lure",
	F(IM, FxForm_Lightning, 1, 0, 64, 2, 0.6f, 16.0f, 0, FIX, kShock, 0.8f, 0.14f),
	G(IM, Glyph_ArrowDown, 0.35f, 6.0f, 0, 3, GA_Orbit, 1.2f, FIX, kStorm, 2.0f));

LOOK(kWizCloudburst, "cloudburst",
	F(IM, FxForm_Rain, 1.4f, 20.0f, 48, 40, 0, 0.1f, 0, FIX, kStorm, 1.6f, 0.05f),
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 3, 0.8f, 6.0f, 1.0f, FIX, kShock, 1.0f, 0.18f, ADD, 1, 0.3f),
	F(IM, FxForm_Burst, 2.6f, 0.4f, 72, 2, 0.4f, 14, 0, FIX, kStorm, 0.8f, 0.10f, ADD, 1, 0.3f));

LOOK(kWizLightningBolt, "lightning",
	F(IM, FxForm_Lightning, 1, 0, 64, 3, 0.9f, 14.0f, 0, FIX, kShock, 0.8f, 0.16f),
	G(IM, Glyph_Lightning, 0.8f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kShock, 1.2f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 64, 1, 0.3f, 14, 0, FIX, kStorm, 0.7f, 0.08f));

LOOK(kWizLightningStorm, "ball lightning",
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 1.2f, 0.4f, 1.5f, FIX, kShock, 1.0f, 0.18f),
	F(IM, FxForm_Orbit, 1.4f, 6.0f, 64, 14, 0, 0, 0, FIX, kShock, 1.8f, 0.05f, ADD, 3.0f),
	G(IM, Glyph_Lightning, 0.35f, 5.0f, 0, 4, GA_Orbit, 1.4f, FIX, kStorm, 1.8f));

LOOK(kWizLeap, "leap of lightning",
	F(RE, FxForm_Wind, 1.4f, 6.0f, 64, 14, 3.0f, 0, 0, FIX, kStorm, 1.0f, 0.05f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 2, 1.0f, 16, 0, FIX, kShock, 1.0f, 0.10f, ADD, 1, 0, AE),
	G(RE, Glyph_Lightning, 0.9f, 6.5f, 4, 1, GA_PopAscend, 0, FIX, kShock, 1.2f));

LOOK(kWizThunderRing, "circle of thunder",
	F(RE, FxForm_Ring, 1.0f, 0.5f, 96, 0, 1.0f, 1.0f, 0.5f, FIX, kShock, 1.4f, 0.10f, COMET, 1, 0, AE),
	F(RE, FxForm_Lightning, 1, 30.0f, 64, 3, 0.9f, 8.0f, 1.0f, FIX, kStorm, 1.0f, 0.14f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kStorm, 1.4f, 0.06f, ADD, 1, 0.1f, AE));

LOOK(kWizConcussive, "concussive",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.4f, 0.6f, FIX, kMagic, 0.7f, 0.16f),
	F(IM, FxForm_Bloom, 1.8f, 3.0f, 48, 3, 0, 0, 0, FIX, kMagic, 1.2f, 0.06f, ADD | FxFlag_Reverse),
	G(IM, Glyph_Hand, 0.6f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kMagic, 1.2f));

LOOK(kWizTela, "tela",
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 0.5f, 0.5f, 1.2f, FIX, kMagic, 0.8f, 0.20f),
	F(IM, FxForm_Rune, 1.4f, 3.4f, 60, 5, 2, 8, 1, FIX, kMagic, 1.4f, 0.05f),
	F(IM, FxForm_Burst, 2.4f, 2.0f, 72, 2, 1.2f, 14, 0, FIX, kEther, 0.8f, 0.10f));

LOOK(kWizKatel, "katel",
	F(TR, FxForm_Bolt, 1, 0, 96, 2, 0.8f, 0.4f, 1.0f, FIX, kPlasma, 0.7f, 0.14f),
	F(TR, FxForm_Bolt, 1, 0, 96, 2, 0.8f, 0.4f, -1.0f, FIX, kMagic, 0.7f, 0.14f, ADD, 1, 0.15f),
	F(IM, FxForm_Burst, 2.0f, 2.5f, 64, 2, 1.0f, 12, 0, FIX, kPlasma, 0.8f, 0.09f));

LOOK(kWizWoven, "thricewoven",
	F(TR, FxForm_Bolt, 1, 0, 96, 2, 0.9f, 0.4f, 1.2f, FIX, kFire, 0.8f, 0.12f),
	F(TR, FxForm_Bolt, 1, 0, 96, 2, 0.9f, 0.4f, 0.4f, FIX, kCold, 0.8f, 0.12f),
	F(TR, FxForm_Bolt, 1, 0, 96, 2, 0.9f, 0.4f, -0.4f, FIX, kMagic, 0.8f, 0.12f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 96, 3, 1.5f, 0, 0.5f, FIX, kPrism, 1.6f, 0.05f, COMET));

LOOK(kWizWildmagic, "wildmagic",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 1.0f, 0.4f, 1.6f, FIX, kWild, 0.8f, 0.18f),
	F(IM, FxForm_Sparkle, 1.4f, 7.0f, 48, 24, 0.14f, 0, 0, FIX, kWild, 1.6f),
	G(IM, Glyph_Gem, 0.7f, 7.2f, 4, 1, GA_PopRoll, 0, FIX, kWild, 1.4f));

LOOK(kWizPureWild, "pure wildmagic",
	F(IM, FxForm_Rune, 2.4f, 0, 80, 7, 3, 21, 0, FIX, kWild, 2.2f, 0.06f),
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 3, 1.0f, 8.0f, 1.0f, FIX, kWild, 1.0f, 0.18f, ADD, 1, 0.3f),
	F(IM, FxForm_Burst, 3.2f, 1.5f, 96, 3, 1.5f, 20, 0, FIX, kPrism, 1.0f, 0.14f, ADD, 1, 0.3f));

LOOK(kWizHavoc, "havoc",
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 5, 2, 10, 1, FIX, kChaos, 2.4f, 0.05f),
	G(IM, Glyph_ArrowDown, 0.3f, 6.5f, 0, 4, GA_Orbit, 1.2f, FIX, kChaos, 2.6f),
	G(IM, Glyph_Spiral, 0.6f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kVoid, 2.4f));

LOOK(kWizManaVortex, "mana vortex",
	F(IM, FxForm_Vortex, 1.3f, 6.0f, 96, 5, 1.2f, 2.5f, 5.0f, FIX, kArcane, 3.0f, 0.07f),
	G(IM, Glyph_Gem, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kMana, 3.0f));

LOOK(kWizMaelstrom, "maelstrom of mana",
	F(IM, FxForm_Vortex, 1.0f, 8.0f, 128, 7, 1.0f, 3.5f, 6.0f, FIX, kMana, 2.4f, 0.07f, ADD, 1, 0, AE),
	F(IM, FxForm_Sparkle, 1.0f, 8.0f, 96, 40, 0.12f, 2, 0, FIX, kArcane, 2.4f, 0.06f, ADD, 1, 0, AE),
	G(IM, Glyph_Drop, 0.6f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kMana, 2.0f));

LOOK(kWizEtherealBarrage, "ethereal barrage",
	F(TR, FxForm_Bolt, 1, 0, 128, 6, 1.0f, 0.3f, 2.0f, FIX, kEther, 0.9f, 0.14f),
	F(IM, FxForm_Burst, 1.8f, 2.0f, 48, 1, 1.0f, 10, 0, FIX, kEther, 0.5f, 0.08f),
	F(IM, FxForm_Burst, 2.0f, 4.0f, 48, 1, 1.0f, 10, 0, FIX, kPlasma, 0.5f, 0.08f, ADD, 1, 0.2f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 48, 1, 1.0f, 10, 0, FIX, kEther, 0.5f, 0.08f, ADD, 1, 0.4f));

LOOK(kWizClawArcane, "arcane claw",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.2f, 0.7f, 0.5f, FIX, kMagic, 0.7f, 0.16f),
	G(IM, Glyph_Claw, 1.1f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kMagic, 1.6f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 40, 20, 0.12f, 0, 0, FIX, kEther, 1.6f));

LOOK(kWizWeave, "ethereal weave",
	F(IM, FxForm_Helix, 1.1f, 6.5f, 112, 3, 2.0f, 0, 0.6f, FIX, kEther, 2.2f, 0.06f, COMET),
	F(IM, FxForm_Rune, 1.6f, 0, 70, 6, 1, 12, 0, FIX, kPrism, 2.2f, 0.05f),
	F(IM, FxForm_Burst, 2.4f, 3.0f, 64, 2, 1.0f, 14, 0, FIX, kEther, 0.8f, 0.09f, ADD, 1, 0.5f));

LOOK(kWizComposite, "composite fire",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.7f, 0.4f, 1.0f, FIX, kFire, 0.8f, 0.16f),
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.7f, 0.4f, -1.0f, FIX, kCold, 0.8f, 0.16f),
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 1, FIX, kPrism, 1.6f, 0.05f),
	F(IM, FxForm_Burst, 2.6f, 1.5f, 96, 2, 1.5f, 16, 0, FIX, kPrism, 0.9f, 0.12f));

LOOK(kWizPermEther, "permeating ether",
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 48, 24, 0.12f, 2, 0, FIX, kEther, 3.0f),
	F(IM, FxForm_Bloom, 1.6f, 1.5f, 48, 3, 0, 0, 0, FIX, kMagic, 2.4f, 0.05f),
	G(IM, Glyph_Gem, 0.6f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kEther, 3.0f));

LOOK(kWizImpel, "impel",
	F(IM, FxForm_Wind, 1.4f, 6.0f, 72, 16, 3.5f, 0, 0, FIX, kAir, 1.2f, 0.05f, ADD, 2.0f),
	F(IM, FxForm_Burst, 2.4f, 3.0f, 64, 2, 0.2f, 12, 0, FIX, kMagic, 0.8f, 0.09f, ADD | FxFlag_Reverse),
	G(IM, Glyph_Hand, 0.9f, 6.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kMagic, 0.9f));

LOOK(kWizStun, "stun",
	F(TR, FxForm_Bolt, 1, 0, 64, 2, 0.2f, 0.4f, 0.4f, FIX, kArcane, 0.6f, 0.14f),
	F(IM, FxForm_Burst, 2.0f, 6.5f, 48, 1, 0, 12, 0, FIX, kArcane, 0.6f, 0.08f),
	Stars(0.2f));

LOOK(kWizGravity, "gravity flux",
	F(IM, FxForm_Burst, 2.4f, 0.3f, 72, 2, 4.0f, 16, 0, FIX, kVoid, 1.2f, 0.10f),
	G(IM, Glyph_ArrowUp, 0.4f, 3.0f, 0, 4, GA_PopAscend, 1.4f, FIX, kEther, 1.6f),
	F(IM, FxForm_Vortex, 1.2f, 6.0f, 64, 4, 0.8f, 2.0f, 4.0f, FIX, kVoid, 1.4f, 0.05f));

LOOK(kWizVortexBurst, "vortex burst",
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.0f, 2.0f, 6.0f, PRI, kWhite, 1.0f, 0.06f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 64, 2, 1.2f, 14, 0, PRI, kWhite, 0.7f, 0.09f));

LOOK(kWizSpiral, "spiral",
	F(IM, FxForm_Helix, 1.2f, 7.0f, 96, 2, 2.5f, 0, 0.6f, PRI, kWhite, 1.4f, 0.07f, COMET),
	F(IM, FxForm_Burst, 1.0f, 0.4f, 96, 2, 0.8f, 16, 0, PRI, kWhite, 1.0f, 0.09f, ADD, 1, 0.2f, AE));

LOOK(kWizCrystalShield, "crystalwing shield",
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kCrystal, 2.4f, 0.04f),
	G(IM, Glyph_Snowflake, 0.35f, 4.0f, 0, 4, GA_Orbit, 1.5f, FIX, kCrystal, 2.4f),
	F(IM, FxForm_Crystal, 1.3f, 2.5f, 40, 8, 1, 0.6f, 0, FIX, kRime, 2.2f, 0.05f));

LOOK(kWizArmor, "crystalwing armor",
	G(IM, Glyph_Helm, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kCrystal, 2.4f),
	F(IM, FxForm_Crystal, 1.1f, 5.5f, 60, 12, 0, 0.6f, 0, FIX, kCrystal, 2.4f, 0.05f),
	F(IM, FxForm_Ring, 1.3f, 3.0f, 48, 0, 1.4f, 1.0f, 0.5f, FIX, kRime, 2.4f, 0.05f, COMET));

LOOK(kWizHusk, "husk",
	Persist(F(AU, FxForm_Ring, 1.2f, 3.0f, 48, 0, 1.2f, 1.0f, 0.4f, FIX, kCrystal, 0, 0.04f, COMET)),
	F(IM, FxForm_Dome, 1.6f, 6.4f, 88, 6, 2, 0, 0, FIX, kCrystal, 2.4f, 0.04f),
	G(IM, Glyph_Gem, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kCrystal, 2.4f));

LOOK(kWizAegis, "calrena's aegis",
	F(IM, FxForm_Dome, 1.8f, 7.0f, 112, 8, 3, 0, 0, FIX, kSilver, 2.6f, 0.04f),
	G(IM, Glyph_Shield, 0.4f, 4.0f, 3, 3, GA_Orbit, 1.5f, FIX, kSilver, 2.6f),
	F(IM, FxForm_Rune, 1.8f, 0, 60, 6, 1, 12, 0, FIX, kArcane, 2.6f, 0.05f));

LOOK(kWizMistGuard, "mist guard",
	Persist(F(AU, FxForm_Wind, 1.2f, 6.0f, 48, 10, 2.0f, 0, 0, FIX, kMist, 0, 0.05f, OVER, 0.6f)),
	F(IM, FxForm_Bloom, 1.8f, 2.0f, 48, 3, 0, 0, 0, FIX, kMist, 2.4f, 0.06f, OVER),
	G(IM, Glyph_Link, 0.3f, 4.0f, 0, 4, GA_Orbit, 1.3f, FIX, kMist, 2.4f, 0, OVER));

LOOK(kWizMistGrasp, "mist grasp",
	F(IM, FxForm_Vines, 1.0f, 3.5f, 64, 4, 1.2f, 0, 1, FIX, kMist, 2.2f, 0.05f, OVER),
	G(IM, Glyph_Shackle, 0.6f, 2.0f, 4, 1, GA_PopBob, 0, FIX, kMist, 2.2f, 0, OVER));

LOOK(kWizShielding, "shielding",
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kArcane, 2.2f, 0.04f),
	G(IM, Glyph_Shield, 0.8f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kEther, 2.2f));

LOOK(kWizShieldFate, "shield of fate",
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kEther, 2.6f, 0.04f),
	G(IM, Glyph_Hourglass, 0.7f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kEther, 2.6f),
	F(IM, FxForm_Sparkle, 1.4f, 7.0f, 32, 16, 0.10f, 1, 0, FIX, kWhite, 2.4f));

LOOK(kWizResist, "elemental barrier",
	F(IM, FxForm_Rune, 1.6f, 0, 60, 3, 1, 9, 0, FIX, kPrism, 2.4f, 0.05f),
	G(IM, Glyph_Flame, 0.35f, 5.0f, 0, 1, GA_Orbit, 1.3f, FIX, kFire, 2.4f),
	G(IM, Glyph_Snowflake, 0.35f, 5.5f, 0, 1, GA_Orbit, 1.3f, FIX, kCold, 2.4f, 0.15f),
	G(IM, Glyph_Lightning, 0.35f, 6.0f, 0, 1, GA_Orbit, 1.3f, FIX, kShock, 2.4f, 0.3f));

LOOK(kWizMirror, "mirror",
	G(IM, Glyph_Shield, 0.9f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kCrystal, 2.2f),
	F(IM, FxForm_Polygon, 1.4f, 4.0f, 48, 6, 0, 0, 0, FIX, kSilver, 2.0f, 0.05f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0, 12, 0, FIX, kCrystal, 1.0f, 0.06f, ADD | FxFlag_Reverse));

LOOK(kWizTwincast, "twincast",
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 2, FIX, kEther, 2.6f, 0.05f),
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 2, FIX, kCold, 2.6f, 0.05f, ADD, -1.0f),
	G(IM, Glyph_Book, 0.5f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kEther, 2.6f));

LOOK(kWizHarvest, "harvest",
	F(RE, FxForm_Sparkle, 1.8f, 8.0f, 60, 30, 0.12f, 2, 0, FIX, kMana, 2.4f),
	F(RE, FxForm_Vortex, 1.4f, 6.0f, 64, 4, 1.4f, 2.0f, -3.0f, FIX, kMana, 2.2f, 0.05f),
	G(RE, Glyph_Drop, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kMana, 2.2f));

LOOK(kWizGambit, "gambit",
	G(RE, Glyph_Coin, 0.7f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kGold, 2.2f),
	G(RE, Glyph_Drop, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kMana, 2.4f),
	F(RE, FxForm_Ring, 1.2f, 3.0f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kMana, 2.2f, 0.05f, COMET));

LOOK(kWizPhaseMarch, "phase march",
	F(RE, FxForm_Wind, 1.4f, 6.0f, 60, 12, 3.0f, 0, 0, FIX, kEther, 1.8f, 0.05f),
	F(RE, FxForm_Sparkle, 1.4f, 6.0f, 40, 20, 0.12f, 0, 0, FIX, kEther, 1.8f),
	F(RE, FxForm_Bloom, 1.6f, 0.5f, 40, 2, 0, 0, 0, FIX, kVoid, 1.6f, 0.05f));

LOOK(kWizSummonItem, "summon item",
	F(IM, FxForm_Rune, 1.6f, 0, 60, 5, 2, 8, 0, FIX, kEther, 2.0f),
	G(IM, Glyph_Gem, 0.5f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kFire, 1.8f, 0.4f));

LOOK(kWizFamiliar, "familiar",
	G(RE, Glyph_Spirit, 0.9f, 6.5f, 4, 1, GA_Emerge, 0, FIX, kFamiliar, 3.0f),
	F(RE, FxForm_Orbit, 1.3f, 6.0f, 48, 10, 0, 0, 0, FIX, kFamiliar, 3.0f, 0.04f, ADD, 2.0f),
	F(RE, FxForm_Rune, 1.8f, 0, 60, 5, 2, 10, 0, FIX, kFamiliar, 3.0f, 0.05f));

LOOK(kWizZomm, "eye of zomm",
	G(RE, Glyph_Eye, 1.0f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kArcane, 3.0f),
	F(RE, FxForm_Bloom, 1.8f, 7.0f, 40, 3, 0, 0, 0, FIX, kArcane, 2.4f, 0.04f));

LOOK(kWizEvac, "evacuate",
	F(RE, FxForm_Vortex, 1.0f, 9.0f, 128, 7, 0.8f, 3.5f, 6.0f, FIX, kEther, 3.0f, 0.07f, ADD, 1, 0, AE),
	G(RE, Glyph_Door, 1.2f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kEther, 3.0f),
	F(RE, FxForm_Sparkle, 1.0f, 8.0f, 96, 40, 0.12f, 0, 0, FIX, kWhite, 3.0f, 0.06f, ADD, 1, 0.3f, AE));

LOOK(kWizBlink, "blink",
	F(RE, FxForm_Bloom, 1.6f, 3.0f, 48, 3, 0, 0, 0, FIX, kVoid, 1.0f, 0.06f, ADD | FxFlag_Reverse),
	F(RE, FxForm_Sparkle, 1.4f, 6.0f, 40, 20, 0.12f, 2, 0, FIX, kEther, 1.0f));

LOOK(kWizTeleport, "teleport",
	G(IM, Glyph_Door, 1.0f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kEther, 3.0f),
	F(IM, FxForm_Vortex, 1.4f, 7.0f, 96, 5, 0.8f, 3.0f, 5.0f, FIX, kEther, 3.0f, 0.06f),
	F(IM, FxForm_Sparkle, 1.4f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kWhite, 3.0f));

LOOK(kWizGate, "gate",
	F(RE, FxForm_Rune, 1.8f, 0, 70, 5, 2, 10, 0, FIX, kAir, 3.0f, 0.05f),
	G(RE, Glyph_Door, 1.0f, 6.5f, 4, 1, GA_Emerge, 0, FIX, kAir, 3.0f),
	F(RE, FxForm_Spire, 0.8f, 8.0f, 64, 12, 0.6f, 2.0f, 0, FIX, kAir, 2.4f, 0.05f));

LOOK(kWizPortal, "portal",
	F(RE, FxForm_Rune, 1.0f, 0, 96, 8, 3, 24, 0, FIX, kEther, 4.0f, 0.07f, ADD, 1, 0, AE),
	G(RE, Glyph_Door, 1.6f, 8.0f, 4, 1, GA_Emerge, 0, FIX, kEther, 4.0f),
	F(RE, FxForm_Vortex, 2.0f, 10.0f, 128, 6, 0.6f, 3.0f, 5.0f, FIX, kEther, 4.0f, 0.07f));

LOOK(kWizRoot, "root",
	G(IM, Glyph_Shackle, 0.7f, 1.5f, 4, 1, GA_PopBob, 0, FIX, kArcane, 2.4f),
	F(IM, FxForm_Crystal, 1.0f, 1.8f, 40, 8, 1, 0.5f, 0, FIX, kRime, 2.4f, 0.05f),
	F(IM, FxForm_Ring, 1.2f, 0.3f, 40, 0, 0.4f, 1.0f, 0.3f, FIX, kArcane, 2.4f, 0.05f, COMET));

LOOK(kWizSnare, "snare",
	G(IM, Glyph_Link, 0.3f, 1.2f, 0, 4, GA_Orbit, 1.2f, FIX, kArcane, 2.4f),
	F(IM, FxForm_Ring, 1.1f, 0.5f, 40, 0, 0.2f, 1.0f, 0.2f, FIX, kMagic, 2.4f, 0.05f, COMET));

LOOK(kAaManaBurn, "mana burn",
	F(TR, FxForm_Drain, 1, 0, 64, 4, 1.4f, 16, 0, FIX, kMana, 1.2f, 0.08f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Flame, 1.3f, 8.0f, 128, 32, 0.6f, 1.8f, 0, FIX, kMana, 2.0f, 0.11f),
	F(IM, FxForm_Burst, 3.4f, 1.5f, 120, 3, 2.0f, 24, 0, FIX, kArcane, 1.2f, 0.14f),
	G(IM, Glyph_Drop, 1.0f, 8.5f, 4, 1, GA_PopAscend, 0, FIX, kMana, 1.6f));

LOOK(kAaHarvestDruzzil, "harvest of druzzil",
	F(RE, FxForm_Sparkle, 2.4f, 9.0f, 80, 40, 0.14f, 2, 0, FIX, kMana, 3.0f),
	F(RE, FxForm_Vortex, 2.0f, 9.0f, 112, 6, 1.4f, 3.0f, -4.0f, FIX, kArcane, 3.0f, 0.07f),
	G(RE, Glyph_Book, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kMana, 3.0f));

LOOK(kAaFamiliar, "improved familiar",
	G(RE, Glyph_Spirit, 1.1f, 7.0f, 4, 1, GA_Emerge, 0, FIX, kFamiliar, 4.0f),
	G(RE, Glyph_Eye, 0.35f, 6.0f, 0, 3, GA_Orbit, 1.4f, FIX, kFamiliar, 3.5f),
	F(RE, FxForm_Rune, 2.2f, 0, 80, 7, 3, 14, 0, FIX, kFamiliar, 4.0f, 0.05f));

LOOK(kAaCallXuzl, "call of xuzl",
	G(IM, Glyph_Sword, 0.9f, 3.0f, 3, 3, GA_RingRise, 1.8f, FIX, kXuzl, 4.0f),
	F(IM, FxForm_Rune, 2.2f, 0, 80, 6, 1, 12, 0, FIX, kXuzl, 4.0f, 0.06f),
	F(IM, FxForm_Vortex, 1.6f, 6.0f, 96, 5, 1.0f, 2.5f, 4.0f, FIX, kXuzl, 3.0f, 0.06f));

LOOK(kAaXuzlStrike, "xuzl's strike",
	G(IM, Glyph_Sword, 1.3f, 9.0f, 4, 1, GA_Drop, 0, FIX, kXuzl, 1.4f),
	F(IM, FxForm_Burst, 2.6f, 0.5f, 96, 2, 0.8f, 18, 0, FIX, kXuzl, 1.0f, 0.12f, ADD, 1, 0.4f));

LOOK(kAaExodus, "exodus",
	F(RE, FxForm_Vortex, 2.2f, 12.0f, 128, 8, 0.6f, 4.0f, 7.0f, FIX, kWhite, 3.0f, 0.08f),
	F(RE, FxForm_Spire, 1.0f, 12.0f, 96, 16, 0.5f, 3.0f, 0, FIX, kEther, 2.4f, 0.06f),
	G(RE, Glyph_Door, 1.4f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kWhite, 3.0f));

LOOK(kAaAbscond, "abscond",
	F(RE, FxForm_Bloom, 2.0f, 3.0f, 64, 4, 0, 0, 0, FIX, kVoid, 1.4f, 0.07f, ADD | FxFlag_Reverse),
	G(RE, Glyph_Door, 0.9f, 6.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kVoid, 1.4f),
	F(RE, FxForm_Wind, 1.4f, 6.0f, 60, 14, 3.0f, 0, 0, FIX, kEther, 1.2f, 0.05f));

LOOK(kAaHoleInSpace, "hole in space",
	F(IM, FxForm_Vortex, 1.0f, 3.0f, 128, 8, 2.0f, 4.0f, -6.0f, FIX, kVoid, 3.0f, 0.08f, OVER, 1, 0, AE),
	F(IM, FxForm_Sparkle, 1.0f, 5.0f, 96, 40, 0.12f, 2, 0, FIX, kEther, 3.0f, 0.06f, ADD, 1, 0, AE),
	G(IM, Glyph_Spiral, 1.2f, 1.0f, 2, 1, GA_PopRoll, 0, FIX, kVoid, 3.0f, 0, OVER));

LOOK(kAaNexus, "the nexus",
	F(RE, FxForm_Lattice, 1.4f, 8.0f, 96, 12, 0.3f, 0, 0, FIX, kEther, 3.0f, 0.06f),
	G(RE, Glyph_Door, 1.2f, 7.0f, 4, 1, GA_Emerge, 0, FIX, kGold, 3.0f),
	F(RE, FxForm_Rune, 2.0f, 0, 80, 8, 3, 16, 0, FIX, kEther, 3.0f, 0.05f));

LOOK(kAaArcaneDestruction, "arcane destruction",
	G(RE, Glyph_Star, 1.0f, 8.4f, 4, 1, GA_PopRoll, 0, FIX, kEther, 3.5f),
	F(RE, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.9f, 1.2f, 0, FIX, kEther, 3.5f, 0.09f),
	F(RE, FxForm_Rune, 2.0f, 0, 70, 5, 2, 10, 0, FIX, kFire, 3.5f, 0.05f));

LOOK(kAaArcaneFury, "arcane fury",
	G(RE, Glyph_Fist, 0.9f, 8.2f, 4, 1, GA_PopAscend, 0, FIX, kArcane, 3.0f),
	F(RE, FxForm_Burst, 2.4f, 3.0f, 72, 2, 1.5f, 16, 0, FIX, kArcane, 1.2f, 0.10f),
	F(RE, FxForm_Orbit, 1.3f, 6.0f, 48, 10, 0, 0, 0, FIX, kPlasma, 3.0f, 0.04f, ADD, 3.0f));

LOOK(kAaArcaneFusion, "arcane fusion",
	F(RE, FxForm_Drain, 1, 0, 48, 3, 1.0f, 12, 0, FIX, kArcane, 1.6f, 0.07f),
	G(RE, Glyph_Gem, 0.5f, 5.0f, 3, 2, GA_RingRise, 1.4f, FIX, kPlasma, 2.4f),
	F(RE, FxForm_Helix, 1.0f, 6.0f, 72, 2, 1.5f, 0, 0.5f, FIX, kArcane, 2.4f, 0.05f, COMET));

LOOK(kAaArcaneOverkill, "arcane overkill",
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 4, 0.8f, 8.0f, 1.0f, FIX, kPlasma, 1.0f, 0.20f),
	F(IM, FxForm_Burst, 3.4f, 1.0f, 120, 3, 1.5f, 24, 0, FIX, kArcane, 1.2f, 0.14f, ADD, 1, 0.2f),
	G(IM, Glyph_Skull, 0.8f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kPlasma, 1.6f, 0.2f));

LOOK(kAaArcaneWhisper, "arcane whisper",
	G(IM, Glyph_Mouth, 0.6f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kEther, 2.0f),
	F(IM, FxForm_Drain, 1, 0, 32, 2, 0.8f, 10, 0, FIX, kEther, 1.6f, 0.06f, ADD | FxFlag_Reverse));

LOOK(kAaForceFlame, "force of flame",
	F(TR, FxForm_Bolt, 1, 0, 128, 6, 0.8f, 0.6f, 1.2f, FIX, kSolar, 0.8f, 0.28f),
	F(IM, FxForm_Flame, 1.4f, 8.0f, 128, 32, 0.5f, 2.0f, 0, FIX, kFire, 1.8f, 0.12f),
	F(IM, FxForm_Burst, 3.4f, 1.0f, 120, 3, 2.0f, 24, 0, FIX, kSolar, 1.0f, 0.16f));

LOOK(kAaForceIce, "force of ice",
	F(TR, FxForm_Bolt, 1, 0, 128, 6, 0.8f, 0.6f, 1.2f, FIX, kRime, 0.8f, 0.28f),
	F(IM, FxForm_Crystal, 2.0f, 6.0f, 96, 16, 1, 1.6f, 0, FIX, kFrost, 2.2f, 0.08f),
	F(IM, FxForm_Burst, 3.4f, 1.0f, 120, 3, 2.0f, 24, 0, FIX, kCold, 1.0f, 0.16f));

LOOK(kAaForceWill, "force of will",
	F(TR, FxForm_Bolt, 1, 0, 128, 6, 0.8f, 0.6f, 1.2f, FIX, kMagic, 0.8f, 0.28f),
	F(IM, FxForm_Rune, 1.8f, 3.4f, 80, 7, 3, 14, 1, FIX, kMagic, 1.6f, 0.06f),
	F(IM, FxForm_Burst, 3.4f, 1.0f, 120, 3, 2.0f, 24, 0, FIX, kEther, 1.0f, 0.16f),
	Stars(0.3f));

LOOK(kAaFuryGods, "fury of the gods",
	F(IM, FxForm_Lightning, 1, 48.0f, 96, 5, 1.0f, 10.0f, 1.0f, FIX, kSolar, 1.2f, 0.24f),
	G(IM, Glyph_Crown, 1.0f, 9.0f, 4, 1, GA_Drop, 0, FIX, kGold, 1.8f),
	F(IM, FxForm_Burst, 3.6f, 1.0f, 120, 3, 2.5f, 24, 0, FIX, kPrism, 1.2f, 0.14f, ADD, 1, 0.3f));

LOOK(kAaFrenziedDevastation, "frenzied devastation",
	F(IM, FxForm_Flame, 1.2f, 7.5f, 112, 28, 0.8f, 1.5f, 0, FIX, kChaos, 3.5f, 0.10f),
	G(IM, Glyph_Lightning, 0.5f, 5.0f, 3, 3, GA_Orbit, 1.4f, FIX, kSolar, 3.5f),
	F(IM, FxForm_Rune, 2.0f, 0, 70, 6, 1, 12, 0, FIX, kChaos, 3.5f, 0.05f));

LOOK(kAaMindCrash, "mind crash",
	G(IM, Glyph_Spiral, 0.9f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kMagic, 1.8f),
	F(IM, FxForm_Bloom, 1.8f, 7.0f, 48, 3, 0, 0, 0, FIX, kMagic, 1.4f, 0.05f, ADD | FxFlag_Reverse),
	G(IM, Glyph_ArrowDown, 0.3f, 6.0f, 0, 3, GA_Orbit, 1.2f, FIX, kEther, 1.8f));

LOOK(kAaAtolShackles, "atol's shackles",
	G(IM, Glyph_Shackle, 0.8f, 1.5f, 4, 1, GA_PopBob, 0, FIX, kArcane, 2.8f),
	G(IM, Glyph_Link, 0.3f, 1.0f, 0, 5, GA_Orbit, 1.3f, FIX, kMagic, 2.8f),
	F(IM, FxForm_Rune, 1.6f, 0, 60, 5, 2, 10, 0, FIX, kArcane, 2.8f, 0.05f));

LOOK(kAaFrostShackles, "frost shackles",
	G(IM, Glyph_Shackle, 0.8f, 1.5f, 4, 1, GA_PopBob, 0, FIX, kRime, 2.8f),
	F(IM, FxForm_Crystal, 1.3f, 2.0f, 60, 12, 1, 0.6f, 0, FIX, kFrost, 2.8f, 0.05f));

LOOK(kAaStrongRoot, "strong root",
	F(IM, FxForm_Crystal, 1.4f, 3.0f, 80, 14, 1, 1.0f, 0, FIX, kCrystal, 3.0f, 0.07f),
	G(IM, Glyph_Shackle, 0.9f, 1.8f, 4, 1, GA_Emerge, 0, FIX, kCrystal, 3.0f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kFrost, 3.0f, 0.06f));

LOOK(kAaBeamDisplacement, "beam of displacement",
	F(RE, FxForm_Lightning, 1, 0, 96, 4, 0.3f, 20.0f, 0, FIX, kVoid, 1.0f, 0.26f, OVER),
	F(RE, FxForm_Lightning, 1, 0, 64, 2, 0.1f, 20.0f, 0, FIX, kEther, 1.0f, 0.10f),
	F(IM, FxForm_Bloom, 1.6f, 3.0f, 48, 3, 0, 0, 0, FIX, kVoid, 1.2f, 0.05f));

LOOK(kAaConcussion, "concussion",
	F(IM, FxForm_Burst, 2.2f, 3.5f, 72, 2, 0.4f, 16, 0, FIX, kMagic, 0.8f, 0.10f, ADD | FxFlag_Reverse),
	G(IM, Glyph_Hammer, 0.8f, 8.0f, 4, 1, GA_Drop, 0, FIX, kEther, 1.2f));

LOOK(kAaSilentFire, "silent fire",
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.9f, 0.8f, 0, FIX, kFire, 1.6f, 0.06f, OVER),
	G(IM, Glyph_Mouth, 0.5f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kGrey, 1.6f),
	F(IM, FxForm_Bloom, 1.4f, 7.0f, 32, 2, 0, 0, 0, FIX, kGrey, 1.2f, 0.04f, ADD | FxFlag_Reverse));

LOOK(kAaSpireArcanum, "spire of arcanum",
	F(RE, FxForm_Spire, 1.0f, 16.0f, 96, 16, 0.6f, 4.0f, 0, FIX, kEther, 2.6f, 0.08f),
	G(RE, Glyph_Book, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kEther, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kArcane, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaLowerElement, "lower element",
	G(IM, Glyph_ArrowDown, 0.7f, 8.0f, 4, 1, GA_Drop, 0, FIX, kPrism, 2.2f),
	G(IM, Glyph_Flame, 0.3f, 5.0f, 0, 1, GA_Orbit, 1.2f, FIX, kFire, 2.2f),
	G(IM, Glyph_Snowflake, 0.3f, 5.0f, 0, 1, GA_Orbit, 1.2f, FIX, kCold, 2.2f, 0.2f));

LOOK(kAaWardDestruction, "ward of destruction",
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kChaos, 3.0f, 0.05f),
	G(IM, Glyph_Shield, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kFire, 3.0f),
	F(IM, FxForm_Sparkle, 1.4f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kChaos, 3.0f));

LOOK(kAaPyromancy, "pyromancy",
	F(IM, FxForm_Flame, 1.1f, 6.5f, 96, 24, 0.8f, 1.3f, 0, FIX, kSolar, 1.4f, 0.09f),
	G(IM, Glyph_Flame, 0.8f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kFire, 1.2f));

LOOK(kAaCryomancy, "cryomancy",
	F(IM, FxForm_Crystal, 1.2f, 5.5f, 72, 14, 0, 1.2f, 0, FIX, kRime, 1.6f, 0.06f),
	G(IM, Glyph_Snowflake, 0.8f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kCold, 1.2f));

LOOK(kAaArcomancy, "arcomancy",
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 1, FIX, kEther, 1.4f, 0.05f),
	G(IM, Glyph_Gem, 0.7f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kPlasma, 1.2f));

LOOK(kAaDimensionalShield, "dimensional shield",
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kVoid, 2.6f, 0.05f, OVER),
	F(IM, FxForm_Sparkle, 1.5f, 7.0f, 40, 20, 0.12f, 2, 0, FIX, kEther, 2.6f));

LOOK(kAaDimensionalJolt, "dimensional jolt",
	F(IM, FxForm_Bloom, 1.6f, 4.0f, 48, 3, 0, 0, 0, FIX, kVoid, 1.0f, 0.06f, ADD | FxFlag_Reverse),
	G(IM, Glyph_Lightning, 0.7f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kEther, 1.2f),
	G(IM, Glyph_ArrowDown, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kVoid, 1.4f));

LOOK(kAaEtherealistUnity, "etherealist's unity",
	G(IM, Glyph_Gem, 0.4f, 4.5f, 3, 4, GA_Orbit, 1.4f, FIX, kEther, 3.0f),
	F(IM, FxForm_Dome, 1.7f, 6.6f, 88, 6, 2, 0, 0, FIX, kEther, 2.6f, 0.04f),
	F(IM, FxForm_Burst, 1.8f, 2.0f, 48, 1, 1.5f, 12, 0, FIX, kArcane, 0.9f, 0.07f));

LOOK(kAaEvokerSynergy, "evoker's synergy",
	F(IM, FxForm_Helix, 1.1f, 6.5f, 96, 2, 1.8f, 0, 0.6f, FIX, kWild, 2.6f, 0.05f, COMET),
	G(IM, Glyph_Link, 0.5f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kEther, 2.6f));

LOOK(kAaGiftArcana, "gift of arcana",
	G(IM, Glyph_Book, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kGold, 2.2f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kEther, 2.2f));

enum Line : int
{
	Ethereal, Tears, RingFire, LureFire, Eruption, Pyroblade, Flash, Fire, Chaos, ChaoticWildfire, Beam,
	SelfImmolate, Splash, FirePillar, FireVortex, ClawFire, PermBlaze, LureCold, Snap, Cascade,
	WildiceCascade, Claw, Torrent, Funnel, IceComet, Freeze, CrystalCircle, FrostPillar, IceVortex,
	IceSlab, Frostbound, PermIce, LureShock, Cloudburst, LightningBolt, LightningStorm, Leap, ThunderRing,
	Concussive, Tela, Katel, Woven, Wildmagic, PureWild, Havoc, ManaVortex, Maelstrom, EtherealBarrage,
	ClawArcane, Weave, Composite, PermEther, Impel, Stun, Gravity, VortexBurst, Spiral, CrystalShield,
	Armor, Husk, Aegis, MistGuard, MistGrasp, Shielding, ShieldFate, Resist, Mirror, Twincast, Harvest,
	Gambit, PhaseMarch, SummonItem, Familiar, Zomm, Evac, Blink, Teleport, Gate, Portal, Root, Snare,
	ManaBurn, HarvestDruzzil, ImprovedFamiliar, CallXuzl, XuzlStrike, Exodus, Abscond, HoleInSpace, Nexus,
	ArcaneDestruction, ArcaneFury, ArcaneFusion, ArcaneOverkill, ArcaneWhisper, ForceFlame, ForceIce,
	ForceWill, FuryGods, FrenziedDevastation, MindCrash, AtolShackles, FrostShackles, StrongRoot,
	BeamDisplacement, Concussion, SilentFire, SpireArcanum, LowerElement, WardDestruction, Pyromancy,
	Cryomancy, Arcomancy, DimensionalShield, DimensionalJolt, EtherealistUnity, EvokerSynergy, GiftArcana,
};

const LineLook kLines[] = {
	{ "Ethereal (fire nuke)",             &kWizEthereal },
	{ "Tears (fire nuke)",                &kWizTears },
	{ "Ring / Presence (fire area)",      &kWizRingFire },
	{ "Fire lure (nuke, debuff)",         &kWizLureFire },
	{ "Eruption (fire rain)",             &kWizEruption },
	{ "Pyroblade (fire nuke)",            &kWizPyroblade },
	{ "Flashfire (fire nuke)",            &kWizFlash },
	{ "Named fire (nuke)",                &kWizFire },
	{ "Chaos (fire nuke)",                &kWizChaos },
	{ "Chaotic Wildfire (nuke)",          &kWizChaoticWildfire },
	{ "Beam (fire, frontal)",             &kWizBeam },
	{ "Self-Immolation (area)",           &kWizSelfImmolate },
	{ "Splash (fire, target area)",       &kWizSplash },
	{ "Fire pillar (nuke)",               &kWizFirePillar },
	{ "Fire vortex (debuff)",             &kWizFireVortex },
	{ "Claw of flame (nuke)",             &kWizClawFire },
	{ "Permeating Blaze",                 &kWizPermBlaze },
	{ "Cold lure (nuke, debuff)",         &kWizLureCold },
	{ "Snap (cold nuke)",                 &kWizSnap },
	{ "Cascade (cold rain)",              &kWizCascade },
	{ "Wildice Cascade (cold rain)",      &kWizWildiceCascade },
	{ "Claw (cold nuke)",                 &kWizClaw },
	{ "Torrent (cold nuke)",              &kWizTorrent },
	{ "Funnel (cold beam)",               &kWizFunnel },
	{ "Ice comet (nuke)",                 &kWizIceComet },
	{ "Mindfreeze (cold, hate)",          &kWizFreeze },
	{ "Crystallizing Circle (cold area)", &kWizCrystalCircle },
	{ "Ice pillar (nuke)",                &kWizFrostPillar },
	{ "Ice vortex (debuff)",              &kWizIceVortex },
	{ "Ice block (nuke)",                 &kWizIceSlab },
	{ "Frostbound (alliance)",            &kWizFrostbound },
	{ "Permeating Ice",                   &kWizPermIce },
	{ "Lightning lure (nuke, debuff)",    &kWizLureShock },
	{ "Cloudburst (lightning nuke)",      &kWizCloudburst },
	{ "Lightning (nuke)",                 &kWizLightningBolt },
	{ "Ball lightning (nuke)",            &kWizLightningStorm },
	{ "Leap (lightning, jump back)",      &kWizLeap },
	{ "Circle of thunder (area)",         &kWizThunderRing },
	{ "Concussive (hate reduction)",      &kWizConcussive },
	{ "Tela (magic nuke)",                &kWizTela },
	{ "Katel (magic nuke)",               &kWizKatel },
	{ "Thricewoven (nuke)",               &kWizWoven },
	{ "Wildmagic (nuke)",                 &kWizWildmagic },
	{ "Pure Wildmagic (nuke)",            &kWizPureWild },
	{ "Havoc (debuff)",                   &kWizHavoc },
	{ "Mana vortex (debuff)",             &kWizManaVortex },
	{ "Maelstrom (mana area)",            &kWizMaelstrom },
	{ "Ethereal Barrage (magic nuke)",    &kWizEtherealBarrage },
	{ "Arcane claw (nuke)",               &kWizClawArcane },
	{ "Ethereal Weave",                   &kWizWeave },
	{ "Composite fire (nuke)",            &kWizComposite },
	{ "Permeating Ether",                 &kWizPermEther },
	{ "Impel (knockback)",                &kWizImpel },
	{ "Stun",                             &kWizStun },
	{ "Gravity (levitate, knock up)",     &kWizGravity },
	{ "vortex burst",                     &kWizVortexBurst, kFromSpell },
	{ "spiral proc",                      &kWizSpiral, kFromSpell },
	{ "Crystalwing shield",               &kWizCrystalShield },
	{ "Crystalwing armor",                &kWizArmor },
	{ "Husk / Ward (rune)",               &kWizHusk },
	{ "Aegis of Calrena (rune)",          &kWizAegis },
	{ "Mist Guard (defensive proc)",      &kWizMistGuard },
	{ "Mist Grasp (root proc)",           &kWizMistGrasp },
	{ "Shielding (hit points)",           &kWizShielding },
	{ "Shield of Fate",                   &kWizShieldFate },
	{ "Elemental barrier (resists)",      &kWizResist },
	{ "Mirror (reflect)",                 &kWizMirror },
	{ "Twincast",                         &kWizTwincast },
	{ "Harvest (mana)",                   &kWizHarvest },
	{ "Gambit (mana)",                    &kWizGambit },
	{ "Phase March",                      &kWizPhaseMarch },
	{ "Summon item",                      &kWizSummonItem },
	{ "Familiar",                         &kWizFamiliar },
	{ "Eye / Aspect of Zomm",             &kWizZomm },
	{ "Evacuate",                         &kWizEvac },
	{ "Blink (short teleport)",           &kWizBlink },
	{ "Teleport / Translocate",           &kWizTeleport },
	{ "Gate",                             &kWizGate },
	{ "Portal (group teleport)",          &kWizPortal },
	{ "Root",                             &kWizRoot },
	{ "Snare",                            &kWizSnare },

	{ "AA: Mana Burn",                    &kAaManaBurn },
	{ "AA: Harvest of Druzzil",           &kAaHarvestDruzzil },
	{ "AA: Improved Familiar",            &kAaFamiliar },
	{ "AA: Call of Xuzl",                 &kAaCallXuzl },
	{ "AA: Xuzl's Strike",                &kAaXuzlStrike },
	{ "AA: Exodus",                       &kAaExodus },
	{ "AA: Abscond",                      &kAaAbscond },
	{ "AA: Hole In Space",                &kAaHoleInSpace },
	{ "AA: The Nexus",                    &kAaNexus },
	{ "AA: Arcane Destruction",           &kAaArcaneDestruction },
	{ "AA: Arcane Fury",                  &kAaArcaneFury },
	{ "AA: Arcane Fusion",                &kAaArcaneFusion },
	{ "AA: Arcane Overkill",              &kAaArcaneOverkill },
	{ "AA: Arcane Whisper",               &kAaArcaneWhisper },
	{ "AA: Force of Flame",               &kAaForceFlame },
	{ "AA: Force of Ice",                 &kAaForceIce },
	{ "AA: Force of Will",                &kAaForceWill },
	{ "AA: Fury of the Gods",             &kAaFuryGods },
	{ "AA: Frenzied Devastation",         &kAaFrenziedDevastation },
	{ "AA: Mind Crash",                   &kAaMindCrash },
	{ "AA: Atol's Shackles",              &kAaAtolShackles },
	{ "AA: Frost Shackles",               &kAaFrostShackles },
	{ "AA: Strong Root",                  &kAaStrongRoot },
	{ "AA: Beam of Displacement",         &kAaBeamDisplacement },
	{ "AA: Concussion",                   &kAaConcussion },
	{ "AA: Silent Fire",                  &kAaSilentFire },
	{ "AA: Spire of Arcanum",             &kAaSpireArcanum },
	{ "AA: Lower Element",                &kAaLowerElement },
	{ "AA: Ward of Destruction",          &kAaWardDestruction },
	{ "AA: Pyromancy",                    &kAaPyromancy },
	{ "AA: Cryomancy",                    &kAaCryomancy },
	{ "AA: Arcomancy",                    &kAaArcomancy },
	{ "AA: Dimensional Shield",           &kAaDimensionalShield },
	{ "AA: Dimensional Jolt",             &kAaDimensionalJolt },
	{ "AA: Etherealist's Unity",          &kAaEtherealistUnity },
	{ "AA: Evoker's Synergy",             &kAaEvokerSynergy },
	{ "AA: Gift of Arcana",               &kAaGiftArcana },
};

const GroupLine kNumbered[] = {
	{ 0, Ethereal },    { 1, LureCold },    { 2, CrystalShield }, { 3, Tears },     { 4, Concussive },
	{ 5, RingFire },    { 6, Harvest },     { 8, Cloudburst },  { 10, LureFire },    { 11, Armor },
	{ 12, LightningBolt }, { 13, LightningStorm }, { 14, Leap }, { 15, Snap },       { 16, Husk },
	{ 17, Eruption },   { 18, Cascade },    { 19, Pyroblade },  { 20, Flash },       { 21, Claw },
	{ 22, Tela },       { 23, Aegis },      { 24, Torrent },    { 25, Fire },        { 26, MistGuard },
	{ 27, Woven },      { 28, Chaos },      { 29, Beam },       { 30, Wildmagic },   { 31, Blink },
	{ 32, Funnel },     { 33, Zomm },       { 34, IceComet },   { 35, PhaseMarch },  { 37, Freeze },
	{ 39, CrystalCircle }, { 40, Havoc },   { 41, Splash },     { 42, FireVortex },  { 43, FrostPillar },
	{ 44, ManaVortex }, { 45, FirePillar }, { 46, IceVortex },  { 47, Maelstrom },   { 48, Freeze },
	{ 49, Katel },      { 50, EtherealBarrage }, { 51, Snare }, { 52, ClawArcane },  { 54, IceSlab },
	{ 55, Weave },      { 56, Evac },       { 57, PermBlaze },  { 58, PermIce },     { 59, PermEther },
	{ 60, Gambit },     { 62, Composite },  { 64, Frostbound },
};

const GroupLine kModern[] = {
	{ 10, Ethereal },   { 20, LureCold },   { 40, Tears },      { 50, Concussive },  { 60, RingFire },
	{ 70, Harvest },    { 80, Cloudburst }, { 90, LureFire },   { 100, Armor },      { 110, LightningBolt },
	{ 120, LightningStorm }, { 130, Leap }, { 140, Snap },      { 150, Husk },       { 160, Eruption },
	{ 170, Cascade },   { 180, Pyroblade }, { 190, Flash },     { 200, Claw },       { 210, Tela },
	{ 220, Aegis },     { 230, Torrent },   { 240, Fire },      { 250, MistGuard },  { 260, Woven },
	{ 270, Chaos },     { 280, Beam },      { 290, Wildmagic }, { 291, ChaoticWildfire }, { 292, WildiceCascade },
	{ 293, PureWild },  { 310, Funnel },    { 330, IceComet },  { 360, SelfImmolate }, { 410, FrostPillar },
	{ 420, ManaVortex }, { 430, FirePillar }, { 460, Freeze },  { 470, Katel },      { 480, EtherealBarrage },
	{ 500, ClawArcane }, { 510, ClawFire }, { 520, IceSlab },   { 530, Weave },      { 580, Gambit },
	{ 590, Composite }, { 600, Frostbound },
};

const GroupLine kOther[] = {
	{ 12000, LureCold },  { 12001, CrystalShield }, { 12002, Tears },   { 12004, Concussive }, { 12006, RingFire },
	{ 12007, Harvest },   { 12009, Cloudburst }, { 12011, Impel },      { 12012, LureFire },   { 12013, Armor },
	{ 12014, LightningBolt }, { 12015, LightningStorm }, { 12016, Leap }, { 12017, Snap },   { 12019, Husk },
	{ 12020, Eruption },  { 12022, Cascade },   { 12023, Flash },       { 12025, Claw },       { 12026, Tela },
	{ 12027, Aegis },     { 12028, Torrent },   { 12029, Fire },        { 12030, MistGuard },  { 12034, Ethereal },
	{ 12035, Woven },     { 12036, Chaos },     { 12037, Beam },        { 12038, Wildmagic },  { 12039, PureWild },
	{ 16136, Harvest },   { 16138, Wildmagic }, { 16253, Tears },
	{ 16236, PureWild },  { 16336, PureWild },  { 16436, PureWild },    { 16536, PureWild },   { 16636, PureWild },
	{ 16736, PureWild },  { 16836, PureWild },  { 16936, PureWild },
	{ 16238, SelfImmolate }, { 16338, SelfImmolate }, { 16438, SelfImmolate }, { 16538, SelfImmolate },
	{ 16638, SelfImmolate }, { 16838, SelfImmolate },
	{ 16553, ClawFire },  { 16653, ClawFire },  { 16753, ClawFire },    { 16853, ClawFire },   { 16953, ClawFire },
	{ 3243, Teleport },   { 16886, Teleport },  { 1599, Mirror },       { 7228, Twincast },
	{ 6202, Shielding },  { 6302, Shielding },  { 6402, Shielding },    { 6502, Shielding },   { 6602, Shielding },
	{ 6702, Shielding },  { 6802, Shielding },  { 6902, Shielding },    { 125140030, Shielding },
	{ 126140030, Shielding },
	{ 9554, ShieldFate }, { 9654, ShieldFate }, { 9754, ShieldFate },   { 9854, ShieldFate },  { 9954, ShieldFate },
	{ 125110480, ShieldFate }, { 126110480, ShieldFate },
};

const NamedLine kAbilities[] = {
	{ "mana burn",                    ManaBurn },
	{ "harvest of druzzil",           HarvestDruzzil },
	{ "improved familiar",            ImprovedFamiliar },
	{ "summon improved familiar",     ImprovedFamiliar },
	{ "call of xuzl",                 CallXuzl },
	{ "xuzl's strike",                XuzlStrike },
	{ "exodus",                       Exodus },
	{ "abscond",                      Abscond },
	{ "hole in space",                HoleInSpace },
	{ "a hole in space",              HoleInSpace },
	{ "the nexus",                    Nexus },
	{ "arcane destruction",           ArcaneDestruction },
	{ "arcane fury",                  ArcaneFury },
	{ "arcane fusion",                ArcaneFusion },
	{ "arcane overkill",              ArcaneOverkill },
	{ "arcane whisper",               ArcaneWhisper },
	{ "force of flame",               ForceFlame },
	{ "force of ice",                 ForceIce },
	{ "force of will",                ForceWill },
	{ "fury of the gods",             FuryGods },
	{ "frenzied devastation",         FrenziedDevastation },
	{ "mind crash",                   MindCrash },
	{ "atol's shackles",              AtolShackles },
	{ "frost shackles",               FrostShackles },
	{ "strong root",                  StrongRoot },
	{ "beam of displacement",         BeamDisplacement },
	{ "concussion",                   Concussion },
	{ "silent fire",                  SilentFire },
	{ "spire of arcanum",             SpireArcanum },
	{ "lower element",                LowerElement },
	{ "ward of destruction",          WardDestruction },
	{ "pyromancy",                    Pyromancy },
	{ "cryomancy",                    Cryomancy },
	{ "arcomancy",                    Arcomancy },
	{ "dimensional shield",           DimensionalShield },
	{ "dimensional jolt",             DimensionalJolt },
	{ "etherealist's unity",          EtherealistUnity },
	{ "evoker's synergy",             EvokerSynergy },
	{ "gift of arcana",               GiftArcana },
	{ "gift of fire",                 GiftArcana },
	{ "gift of ice",                  GiftArcana },
	{ "improved twincast",            Twincast },
};

bool Contains(std::string_view s, std::string_view part)
{
	return s.find(part) != std::string_view::npos;
}

bool AnyOf(std::string_view base, std::initializer_list<std::string_view> names)
{
	for (std::string_view n : names)
		if (base == n)
			return true;
	return false;
}

int BySpell(const eqlib::EQ_Spell*, std::string_view base)
{
	if (EndsWith(base, " vortex effect"))
		return VortexBurst;
	if (EndsWith(base, "freeze effect"))
		return Freeze;
	if (EndsWith(base, "mist grasp"))
		return MistGrasp;
	if (AnyOf(base, { "fire spiral", "power spiral", "ice spiral" }) || EndsWith(base, " spiral of al`kabor"))
		return Spiral;

	if (EndsWith(base, " gate") || base == "gate")
		return Gate;
	if (EndsWith(base, " portal") || StartsWith(base, "evacuate: ") || StartsWith(base, "alter plane: "))
		return Portal;
	if (StartsWith(base, "translocate") || StartsWith(base, "teleport ") || EndsWith(base, "'s relocation"))
		return Teleport;
	if (AnyOf(base, { "evacuate", "lesser evacuate", "decession", "greater decession" }))
		return Evac;
	if (AnyOf(base, { "shadow step", "bounce", "phase walk", "yonder" }))
		return Blink;
	if (Contains(base, "familiar"))
		return Familiar;
	if (base == "eye of zomm")
		return Zomm;
	if ((StartsWith(base, "focus ") && EndsWith(base, "empowering essence")) || Contains(base, "imbue fire opal"))
		return SummonItem;

	if (AnyOf(base, { "resistant skin", "elemental shield", "elemental armor", "elemental cloak",
			"resistant armor", "elemental barrier" }))
		return Resist;
	if (Contains(base, "shielding") || AnyOf(base, { "shield of the magi", "shield of the arcane",
			"force shield", "shield of maelin", "ether shield" }))
		return Shielding;
	if (EndsWith(base, "skin") || AnyOf(base, { "magi ward", "mana ward", "ether ward", "fire rune" }))
		return Husk;
	if (EndsWith(base, " of calrena"))
		return Aegis;
	if (base == "reflect")
		return Mirror;
	if (base == "harvest")
		return Harvest;
	if (base == "mana weave")
		return Woven;

	if (AnyOf(base, { "root", "immobilize", "paralyzing earth", "fetter", "greater fetter", "bonds of force",
			"elnerick's entombment of ice" }))
		return Root;
	if (AnyOf(base, { "atol's spectral shackles", "clinging frost" }))
		return Snare;
	if (AnyOf(base, { "gravity flux", "invert gravity" }))
		return Gravity;
	if (AnyOf(base, { "tishan's clash", "markar's clash", "tishan's discord", "markar's discord", "force snap",
			"force strike", "cast force" }))
		return Stun;
	if (base == "ancient: greater concussion")
		return Concussive;

	if (StartsWith(base, "lure of ") || base == "enticement of flame")
	{
		if (Contains(base, "frost") || Contains(base, "ice"))
			return LureCold;
		if (Contains(base, "lightning") || Contains(base, "thunder"))
			return LureShock;
		return LureFire;
	}
	if (StartsWith(base, "tears of "))
		return Tears;

	if (AnyOf(base, { "firestorm", "lava storm", "meteor storm" }))
		return Eruption;
	if (AnyOf(base, { "frost storm", "gelid rains" }))
		return Cascade;
	if (AnyOf(base, { "lightning storm", "pillar of lightning", "column of lightning" }))
		return Cloudburst;
	if (AnyOf(base, { "circle of thunder", "jyll's static pulse", "energy storm" }))
		return ThunderRing;
	if (AnyOf(base, { "circle of fire", "jyll's wave of heat", "supernova", "coronal blast" }))
		return RingFire;
	if (AnyOf(base, { "jyll's zephyr of ice", "winds of gelid" }))
		return CrystalCircle;
	if (AnyOf(base, { "pillar of fire", "pillar of flame" }))
		return FirePillar;
	if (AnyOf(base, { "pillar of frost", "column of frost" }))
		return FrostPillar;

	if (AnyOf(base, { "ancient: strike of chaos", "chaos flame" }))
		return Chaos;
	if (base == "ether flame")
		return Ethereal;
	if (AnyOf(base, { "flaming sword of xuzl", "ethantis's burning blade" }))
		return Pyroblade;
	if (AnyOf(base, { "sunstrike", "strike of solusek", "white fire", "corona flare", "ancient: core fire",
			"spark of fire", "firebane", "flame shock", "inferno shock", "fire bolt", "shock of fire",
			"fingers of fire", "conflagration", "inferno of al`kabor", "draught of fire", "draught of ro",
			"porlos' fury" }))
		return Fire;

	if (AnyOf(base, { "ice comet", "gelidin comet", "ice meteor" }))
		return IceComet;
	if (base == "ice block")
		return IceSlab;
	if (AnyOf(base, { "claw of frost", "claw of vox" }))
		return Claw;
	if (AnyOf(base, { "ice spear of solist", "solist's frozen sword", "ancient: spear of gelaqua",
			"ancient: destruction of ice", "wrath of al`kabor", "hsagra's wrath", "draught of ice",
			"draught of e`ci", "iceflame of e`ci" }))
		return Torrent;
	if (AnyOf(base, { "frost bolt", "blast of cold", "numbing cold", "icestrike", "shock of ice", "frost shock",
			"ice shock", "icebane", "spark of ice", "black ice", "frozen harpoon" }))
		return Snap;

	if (AnyOf(base, { "shock of lightning", "lightning bolt", "project lightning", "lightning shock",
			"thunder strike", "thunderbolt", "thunderclap", "agnarr's thunder", "spark of lightning",
			"spark of thunder", "lightningbane", "elnerick's electrical rending", "voltaic draught",
			"draught of thunder", "draught of lightning" }))
		return LightningBolt;
	if (AnyOf(base, { "telaka", "telekara", "telekin", "thundaka", "shock of magic", "disintegrate",
			"garrison's superior sundering", "garrison's mighty mana shock", "draught of jiva",
			"ether shard strike", "retribution of al`kabor", "vengeance of al`kabor", "rend" }))
		return Tela;
	return -1;
}

} // namespace

const ClassCatalogue& WizardCatalogue()
{
	static const ClassCatalogue c{ "Wizard", eqlib::Wizard, kLines, 16, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
