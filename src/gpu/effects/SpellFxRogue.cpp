#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

namespace eqgfx::spellfx {

namespace {

constexpr Pair kRogBlade    { { 1.00f, 1.00f, 1.00f, 1.00f }, { 0.62f, 0.68f, 0.78f, 0.90f } };
constexpr Pair kRogSmoke    { { 0.72f, 0.72f, 0.75f, 0.55f }, { 0.28f, 0.28f, 0.32f, 0.30f } };
constexpr Pair kRogShade    { { 0.40f, 0.30f, 0.55f, 0.85f }, { 0.05f, 0.02f, 0.10f, 0.80f } };
constexpr Pair kRogVenom    { { 0.75f, 1.00f, 0.25f, 0.95f }, { 0.10f, 0.45f, 0.05f, 0.85f } };
constexpr Pair kRogToxin    { { 0.60f, 1.00f, 0.75f, 0.95f }, { 0.40f, 0.10f, 0.60f, 0.85f } };
constexpr Pair kRogBlood    { { 0.95f, 0.20f, 0.18f, 0.95f }, { 0.35f, 0.00f, 0.02f, 0.90f } };
constexpr Pair kRogCoin     { { 1.00f, 0.90f, 0.45f, 1.00f }, { 0.70f, 0.50f, 0.10f, 0.90f } };
constexpr Pair kRogGlare    { { 1.00f, 1.00f, 0.95f, 1.00f }, { 1.00f, 0.95f, 0.60f, 0.60f } };
constexpr Pair kRogMidnight { { 0.35f, 0.45f, 0.75f, 0.85f }, { 0.05f, 0.05f, 0.15f, 0.80f } };
constexpr Pair kRogEdge     { { 1.00f, 1.00f, 1.00f, 1.00f }, { 0.95f, 0.25f, 0.20f, 0.90f } };
constexpr Pair kRogChaos    { { 0.95f, 0.45f, 1.00f, 0.95f }, { 0.20f, 0.90f, 0.55f, 0.85f } };

constexpr uint32_t DARK = OVER;

LOOK(kRogBlinding, "blinding",
	G(IM, Glyph_Eye, 0.9f, 7.2f, 4, 1, GlyphAnim_Pop, 0, FIX, kRogGlare, 1.2f),
	F(IM, FxForm_Spire, 0.6f, 6.5f, 96, 16, 1.5f, 3.0f, 0, FIX, kRogGlare, 0.6f, 0.08f),
	F(IM, FxForm_Burst, 2.4f, 6.0f, 72, 2, 0.2f, 20, 0, FIX, kWhite, 0.5f, 0.10f),
	F(IM, FxForm_Sparkle, 1.0f, 7.5f, 32, 16, 0.14f, 1, 0, FIX, kRogGlare, 1.4f, 0.06f, ADD, 1, 0.2f));

LOOK(kRogBleed, "bleed",
	G(IM, Glyph_Claw, 0.9f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kRogEdge, 0.7f),
	G(IM, Glyph_Drop, 0.2f, 5.0f, 0, 5, GA_Rain, 0.9f, FIX, kRogBlood, 2.4f, 0.2f),
	F(AU, FxForm_Rain, 0.6f, 5.0f, 24, 12, 0, 0.05f, 0, FIX, kRogBlood, 4.0f, 0.04f));

LOOK(kRogAmbush, "ambush",
	F(RE, FxForm_Bloom, 1.2f, 0.2f, 48, 2, 0, 0, 0, FIX, kRogShade, 0.8f, 0.07f, DARK | FxFlag_Reverse),
	G(IM, Glyph_Bat, 0.8f, 7.0f, 4, 1, GA_Drop, 0, FIX, kRogShade, 1.0f, 0, DARK),
	G(IM, Glyph_Sword, 1.0f, 8.0f, 4, 1, GA_Drop, 0, FIX, kRogBlade, 1.0f, 0.2f),
	F(IM, FxForm_Burst, 1.8f, 4.0f, 48, 1, 0.1f, 14, 0, FIX, kRogBlade, 0.5f, 0.08f, ADD, 1, 0.35f));

LOOK(kRogJugular, "jugular",
	F(IM, FxForm_Ring, 0.5f, 5.6f, 40, 0, 4.0f, 1.0f, 0, FIX, kRogEdge, 0.4f, 0.06f, COMET),
	F(IM, FxForm_Burst, 1.4f, 5.6f, 40, 1, 0, 10, 0, FIX, kRogBlood, 0.6f, 0.08f, ADD, 1, 0.15f),
	G(IM, Glyph_Drop, 0.25f, 5.6f, 0, 4, GA_Rain, 0.6f, FIX, kRogBlood, 1.8f, 0.2f));

LOOK(kRogDaggerStrike, "daggerstrike",
	G(IM, Glyph_Sword, 0.6f, 4.5f, 3, 2, GA_PopSpin, 0.5f, FIX, kRogBlade, 0.8f),
	F(IM, FxForm_Burst, 1.6f, 4.0f, 48, 1, 0, 12, 0, FIX, kRogBlade, 0.5f, 0.07f),
	F(IM, FxForm_Sparkle, 0.9f, 5.0f, 24, 12, 0.10f, 1, 0, FIX, kSilver, 0.8f));

LOOK(kRogDaggerThrow, "dagger-throw",
	F(TR, FxForm_Bolt, 1, 0, 64, 1, 0.05f, 0.4f, 0.4f, FIX, kRogBlade, 0.5f, 0.10f),
	G(IM, Glyph_Sword, 0.7f, 4.5f, 4, 1, GA_PopRoll, 0, FIX, kRogBlade, 0.8f),
	F(IM, FxForm_Burst, 1.4f, 4.5f, 40, 1, 0, 8, 0, FIX, kIron, 0.4f, 0.07f));

LOOK(kRogAssault, "assault",
	G(IM, Glyph_Sword, 0.55f, 4.5f, 3, 5, GA_PopSpin, 0.9f, FIX, kRogBlade, 1.0f),
	F(IM, FxForm_Spire, 0.4f, 4.5f, 80, 10, 0.2f, 1.6f, 0, FIX, kRogBlade, 0.6f, 0.07f, ADD, 1, 0.2f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 72, 3, 1.0f, 16, 0, FIX, kRogEdge, 0.7f, 0.09f, ADD, 1, 0.3f),
	F(IM, FxForm_Sparkle, 1.2f, 5.0f, 40, 20, 0.12f, 0, 0, FIX, kRogBlood, 1.2f, 0.06f, ADD, 1, 0.4f));

LOOK(kRogPuncture, "puncture",
	F(TR, FxForm_Bolt, 1, 0, 96, 2, 0.1f, 0.6f, 0.2f, FIX, kRogToxin, 0.5f, 0.10f),
	G(IM, Glyph_Fang, 0.7f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kRogToxin, 1.0f),
	F(IM, FxForm_Vortex, 0.8f, 5.5f, 48, 3, 1.0f, 2.0f, 5.0f, FIX, kRogToxin, 1.4f, 0.05f),
	G(IM, Glyph_Spiral, 0.5f, 7.5f, 4, 1, GA_PopRoll, 0, FIX, kRogToxin, 1.6f, 0.3f));

LOOK(kRogHuntersDagger, "shadow-hunter's dagger",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.3f, 0.5f, 0.8f, FIX, kRogShade, 0.6f, 0.12f, DARK),
	F(TR, FxForm_Bolt, 1, 0, 64, 1, 0.05f, 0.3f, 0.8f, FIX, kRogBlade, 0.6f, 0.08f),
	G(IM, Glyph_Sword, 0.9f, 7.5f, 4, 1, GA_Drop, 0, FIX, kRogMidnight, 1.4f),
	F(IM, FxForm_Bloom, 1.6f, 0.5f, 48, 2, 0, 0, 0, FIX, kRogShade, 1.2f, 0.06f, DARK));

LOOK(kRogSneakAttack, "sneak attack",
	F(RE, FxForm_Flame, 0.9f, 4.0f, 48, 12, 0.9f, 0.6f, 0, FIX, kRogShade, 0.8f, 0.07f, DARK),
	G(IM, Glyph_Sword, 0.8f, 4.0f, 4, 1, GA_PopRoll, 0, FIX, kRogBlade, 0.8f),
	F(IM, FxForm_Burst, 1.4f, 4.0f, 40, 1, 0, 8, 0, FIX, kRogBlade, 0.5f, 0.07f));

LOOK(kRogVengeance, "thief's vengeance",
	G(IM, Glyph_Coin, 0.6f, 6.5f, 4, 1, GA_PopAscend, 0, FIX, kRogCoin, 1.2f),
	G(IM, Glyph_Sword, 0.8f, 4.5f, 4, 1, GA_PopRoll, 0, FIX, kRogEdge, 0.8f),
	F(IM, FxForm_Burst, 1.8f, 4.5f, 48, 1, 0.2f, 12, 0, FIX, kRogCoin, 0.6f, 0.08f));

LOOK(kRogAssassinsStrike, "assassin's strike",
	F(IM, FxForm_Bloom, 1.4f, 0.2f, 48, 2, 0, 0, 0, FIX, kRogShade, 1.0f, 0.08f, DARK | FxFlag_Reverse),
	G(IM, Glyph_Skull, 0.7f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kRogShade, 1.6f, 0.2f, DARK),
	F(IM, FxForm_Spire, 0.3f, 5.0f, 64, 8, 0.2f, 2.0f, 0, FIX, kRogEdge, 0.6f, 0.08f));

LOOK(kRogChaosStrike, "chaos strike",
	G(IM, Glyph_Sword, 0.9f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kRogChaos, 1.0f),
	F(IM, FxForm_Lightning, 1, 0, 48, 2, 0.8f, 16.0f, 0, FIX, kRogChaos, 0.6f, 0.10f),
	F(IM, FxForm_Burst, 2.0f, 4.0f, 48, 2, 0.3f, 14, 0, FIX, kRogChaos, 0.6f, 0.08f));

LOOK(kRogDaggerfall, "daggerfall",
	G(IM, Glyph_Sword, 0.4f, 9.0f, 4, 7, GA_Rain, 1.3f, FIX, kRogBlade, 1.4f),
	F(IM, FxForm_Rain, 1.2f, 10.0f, 40, 20, 0, 0.05f, 0, FIX, kSilver, 1.0f, 0.05f),
	F(IM, FxForm_Burst, 2.0f, 0.4f, 64, 2, 0.4f, 12, 0, FIX, kRogBlade, 0.7f, 0.08f, ADD, 1, 0.45f));

LOOK(kRogRazorarc, "razorarc",
	F(RE, FxForm_Ring, 1.8f, 3.5f, 64, 0, 6.0f, 1.0f, 0, FIX, kRogBlade, 0.5f, 0.08f, COMET),
	F(RE, FxForm_Ring, 1.6f, 3.8f, 64, 0, -6.0f, 1.0f, 0, FIX, kRogEdge, 0.5f, 0.06f, COMET, 1, 0.15f),
	F(IM, FxForm_Sparkle, 1.2f, 5.0f, 32, 16, 0.10f, 1, 0, FIX, kSilver, 1.0f));

LOOK(kRogToxicity, "toxicity",
	G(IM, Glyph_Book, 0.7f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kRogVenom, 2.0f),
	G(IM, Glyph_Drop, 0.35f, 5.5f, 0, 3, GA_Orbit, 1.1f, FIX, kRogVenom, 2.4f),
	F(IM, FxForm_Bubbles, 0.8f, 5.0f, 40, 6, 0.15f, 0, 0, FIX, kPoison, 2.2f, 0.035f));

LOOK(kRogProcureSap, "procure sap",
	G(IM, Glyph_Leaf, 0.6f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kWood, 1.8f),
	F(IM, FxForm_Drain, 1, 0, 32, 2, 0.8f, 10, 0, FIX, kRogVenom, 1.6f, 0.07f),
	G(IM, Glyph_Drop, 0.4f, 4.0f, 4, 1, GA_Drop, 0, FIX, kRogVenom, 1.6f, 0.4f));

LOOK(kRogToxicBlade, "toxic blade",
	G(IM, Glyph_Sword, 0.8f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kRogBlade, 2.4f),
	G(IM, Glyph_Snake, 0.5f, 7.2f, 4, 1, GA_PopRoll, 0, FIX, kRogVenom, 2.4f, 0.2f),
	G(IM, Glyph_Drop, 0.2f, 6.0f, 0, 3, GA_Rain, 0.4f, FIX, kRogVenom, 2.2f, 0.4f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 24, 12, 0.10f, 1, 0, FIX, kPoison, 2.0f));

LOOK(kRogPoisonDisc, "venom discipline",
	F(IM, FxForm_Flame, 1.0f, 5.5f, 96, 24, 0.9f, 0.9f, 0, FIX, kRogVenom, 3.0f, 0.07f),
	G(IM, Glyph_Snake, 0.45f, 4.0f, 3, 3, GA_Orbit, 1.3f, FIX, kRogVenom, 3.0f),
	F(IM, FxForm_Bubbles, 1.1f, 6.0f, 48, 8, 0.2f, 0, 0, FIX, kPoison, 3.0f, 0.035f),
	Persist(F(AU, FxForm_Orbit, 1.1f, 5.0f, 32, 8, 0, 0, 0, FIX, kRogVenom, 0, 0.035f, ADD, 1.5f)));

LOOK(kRogAlliance, "poisonous alliance",
	G(IM, Glyph_Link, 0.4f, 6.5f, 4, 2, GA_PopSpin, 0.4f, FIX, kRogToxin, 2.6f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 72, 2, 1.5f, 0, 0.5f, FIX, kRogToxin, 2.6f, 0.05f, COMET),
	F(IM, FxForm_Rune, 1.4f, 0, 60, 5, 2, 10, 0, FIX, kRogVenom, 2.6f, 0.05f));

LOOK(kRogSpikeTrap, "spike trap",
	F(RE, FxForm_Rune, 1.6f, 0, 60, 6, 1, 12, 0, FIX, kRogVenom, 2.4f, 0.05f, ADD, 1, 0, AE),
	F(RE, FxForm_Crystal, 1.2f, 1.5f, 48, 12, 1, 0.5f, 0, FIX, kIron, 2.4f, 0.05f, ADD, 1, 0.3f),
	F(RE, FxForm_Bubbles, 1.0f, 2.5f, 40, 6, 0.15f, 0, 0, FIX, kRogVenom, 2.4f, 0.035f, ADD, 1, 0.4f));

LOOK(kRogPinpoint, "pinpoint",
	G(IM, Glyph_Eye, 0.7f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kRogEdge, 2.4f),
	F(IM, FxForm_Ring, 0.9f, 4.0f, 40, 0, 1.0f, 1.0f, 0, FIX, kRogEdge, 2.4f, 0.04f),
	F(IM, FxForm_Ring, 0.5f, 4.0f, 32, 0, -1.0f, 1.0f, 0, FIX, kRogEdge, 2.4f, 0.04f),
	G(IM, Glyph_ArrowDown, 0.3f, 6.0f, 0, 3, GA_Orbit, 1.1f, FIX, kRogBlood, 2.4f, 0.3f));

LOOK(kRogMark, "mark",
	F(IM, FxForm_Rune, 1.3f, 0, 60, 4, 1, 8, 0, FIX, kRogCoin, 3.0f, 0.05f),
	G(IM, Glyph_Coin, 0.5f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kRogCoin, 3.0f),
	F(IM, FxForm_Sparkle, 0.9f, 7.0f, 20, 10, 0.10f, 1, 0, FIX, kRogCoin, 2.0f));

LOOK(kRogSmokescreen, "smoke",
	F(RE, FxForm_Bubbles, 1.3f, 6.0f, 80, 14, 0.35f, 0, 0, FIX, kRogSmoke, 2.6f, 0.05f, DARK),
	F(RE, FxForm_Vortex, 1.2f, 5.0f, 64, 5, 1.2f, 1.5f, 1.5f, FIX, kRogSmoke, 2.6f, 0.08f, DARK),
	F(RE, FxForm_Bloom, 1.6f, 0.5f, 48, 2, 0, 0, 0, FIX, kGrey, 2.0f, 0.06f, DARK));

LOOK(kRogMisdirection, "misdirection",
	G(RE, Glyph_Mask, 0.8f, 7.2f, 4, 1, GA_PopRoll, 0, FIX, kRogShade, 2.2f, 0, DARK),
	G(RE, Glyph_Spiral, 0.35f, 4.5f, 3, 3, GA_Orbit, 1.2f, FIX, kRogMidnight, 2.2f),
	F(RE, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kRogShade, 1.8f, 0.05f, DARK | FxFlag_Reverse));

LOOK(kRogDistraction, "distraction",
	G(RE, Glyph_Coin, 0.35f, 6.0f, 0, 3, GA_Orbit | GlyphAnim_Ascend, 1.0f, FIX, kRogCoin, 1.8f),
	G(RE, Glyph_Mouth, 0.6f, 7.4f, 4, 1, GlyphAnim_Pop, 0, FIX, kRogShade, 1.4f, 0, DARK));

LOOK(kRogEscape, "escape",
	F(RE, FxForm_Bubbles, 1.1f, 5.0f, 64, 10, 0.3f, 0, 0, FIX, kRogSmoke, 2.0f, 0.05f, DARK),
	G(RE, Glyph_Door, 0.9f, 4.5f, 4, 1, GA_PopAscend, 0, FIX, kRogShade, 1.8f, 0, DARK),
	F(RE, FxForm_Wind, 1.3f, 4.0f, 48, 10, 2.5f, 0, 0, FIX, kRogSmoke, 1.4f, 0.05f, DARK));

LOOK(kRogIndiscernible, "indiscernible",
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kGhost, 2.5f, 0.05f),
	G(IM, Glyph_Moon, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kRogMidnight, 2.4f),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.10f, 1, 0, FIX, kGhost, 2.4f));

LOOK(kRogThiefsVision, "thief's vision",
	G(IM, Glyph_Eye, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kRogCoin, 2.8f),
	F(IM, FxForm_Spire, 0.5f, 7.6f, 48, 8, 0.2f, 1.2f, 0, FIX, kRogCoin, 2.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.0f, 7.5f, 24, 12, 0.10f, 2, 0, FIX, kRogGlare, 2.4f));

LOOK(kRogPhantom, "phantom assassin",
	G(RE, Glyph_Skeleton, 1.1f, 3.0f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kRogShade, 3.0f, 0, DARK),
	F(RE, FxForm_Flame, 0.8f, 6.5f, 72, 18, 0.2f, 1.2f, 0, FIX, kGhost, 3.0f, 0.06f),
	G(RE, Glyph_Sword, 0.4f, 4.0f, 3, 2, GA_Orbit, 0.8f, FIX, kRogBlade, 3.0f, 0.4f),
	F(RE, FxForm_Bloom, 2.0f, 0.3f, 48, 2, 0, 0, 0, FIX, kRogShade, 2.0f, 0.06f, DARK));

LOOK(kRogSpellEvasion, "spell evasion",
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kArcane, 2.4f, 0.03f),
	F(IM, FxForm_Burst, 2.0f, 3.0f, 64, 2, 0, 12, 0, FIX, kMagic, 1.0f, 0.07f, ADD | FxFlag_Reverse),
	G(IM, Glyph_Spiral, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.5f, FIX, kArcane, 2.4f));

LOOK(kRogExecutioner, "executioner",
	F(IM, FxForm_Flame, 1.05f, 5.8f, 96, 24, 0.9f, 0.9f, 0, FIX, kRogBlood, 3.0f, 0.08f),
	G(IM, Glyph_Scythe, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kRogShade, 3.0f, 0, DARK),
	F(IM, FxForm_Burst, 2.2f, 0.3f, 64, 2, 0.3f, 14, 0, FIX, kRogBlood, 0.9f, 0.09f));

LOOK(kRogEdgeDisc, "edge",
	G(IM, Glyph_Sword, 0.5f, 3.5f, 3, 4, GA_Orbit, 1.2f, FIX, kRogEdge, 3.0f),
	F(IM, FxForm_Ring, 1.2f, 3.5f, 64, 0, 3.0f, 1.0f, 0.6f, FIX, kRogEdge, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Spire, 0.7f, 6.0f, 64, 12, 0.6f, 2.0f, 0, FIX, kRogBlade, 1.2f, 0.06f));

LOOK(kRogEradicator, "eradicator",
	G(IM, Glyph_Skull, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kRogBlood, 2.8f),
	G(IM, Glyph_Sword, 0.45f, 3.5f, 3, 3, GA_Orbit, 1.3f, FIX, kRogBlade, 2.8f),
	F(IM, FxForm_Lattice, 1.2f, 6.0f, 64, 6, 0.4f, 0, 0, FIX, kRogBlood, 2.4f, 0.05f));

LOOK(kRogAim, "aim",
	G(IM, Glyph_Eye, 0.6f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kRogBlade, 2.8f),
	F(IM, FxForm_Rune, 1.2f, 4.0f, 60, 4, 1, 16, 1, FIX, kRogBlade, 2.8f, 0.04f),
	G(IM, Glyph_Sword, 0.35f, 4.0f, 0, 3, GA_Orbit, 1.2f, FIX, kSilver, 2.8f));

LOOK(kRogKnifeplay, "knifeplay",
	G(IM, Glyph_Sword, 0.5f, 4.5f, 3, 6, GA_Orbit | GlyphAnim_Roll, 1.1f, FIX, kRogBlade, 3.0f),
	F(IM, FxForm_Orbit, 1.2f, 5.0f, 48, 10, 0, 0, 0, FIX, kSilver, 3.0f, 0.04f, ADD, 3.0f));

LOOK(kRogReflexes, "reflexes",
	F(IM, FxForm_Wind, 1.2f, 4.5f, 48, 8, 2.0f, 0, 0, FIX, kRogMidnight, 2.4f, 0.05f),
	G(IM, Glyph_Lightning, 0.6f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kRogBlade, 2.2f),
	F(IM, FxForm_Ring, 1.1f, 5.0f, 40, 0, 2.5f, 1.0f, 0.8f, FIX, kRogMidnight, 2.2f, 0.04f, COMET));

LOOK(kRogHiddenBlade, "hidden blade",
	G(IM, Glyph_Sword, 0.7f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kRogShade, 2.6f, 0, DARK),
	F(IM, FxForm_Dome, 1.4f, 6.0f, 72, 6, 2, 0, 0, FIX, kRogShade, 2.4f, 0.04f, DARK),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 24, 12, 0.10f, 2, 0, FIX, kRogBlade, 2.2f));

LOOK(kRogWeapons, "weapons",
	G(IM, Glyph_Sword, 0.7f, 5.0f, 3, 2, GA_Orbit, 0.9f, FIX, kRogChaos, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 96, 2, 1.5f, 0, 0.6f, FIX, kRogChaos, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 96, 2, -1.5f, 0, 0.6f, FIX, kRogBlade, 3.0f, 0.05f, COMET));

LOOK(kRogWeaponTrigger, "weapon trigger",
	G(IM, Glyph_Sword, 0.5f, 4.5f, 4, 1, GA_PopRoll, 0, FIX, kRogChaos, 0.7f),
	F(IM, FxForm_Burst, 1.4f, 4.5f, 32, 1, 0, 8, 0, FIX, kRogChaos, 0.5f, 0.07f));

LOOK(kRogSpeed, "speed",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 12, 3.5f, 0, 0, FIX, kRogShade, 3.0f, 0.05f, DARK),
	F(IM, FxForm_Wind, 1.2f, 5.0f, 48, 8, 3.0f, 0, 0, FIX, kRogMidnight, 3.0f, 0.04f),
	G(IM, Glyph_Bat, 0.5f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kRogShade, 1.8f, 0, DARK));

LOOK(kRogSpeedShock, "speed shock",
	F(IM, FxForm_Lightning, 1, 0, 32, 1, 0.6f, 14.0f, 0, FIX, kRogMidnight, 0.4f, 0.08f),
	F(IM, FxForm_Burst, 1.2f, 4.0f, 32, 1, 0, 8, 0, FIX, kRogShade, 0.4f, 0.07f, DARK));

LOOK(kRogFooted, "footed",
	F(IM, FxForm_Wind, 1.1f, 1.5f, 48, 10, 2.5f, 0, 0, FIX, kAir, 2.6f, 0.05f),
	G(IM, Glyph_Feather, 0.3f, 1.0f, 3, 3, GA_Orbit, 1.1f, FIX, kFeather, 2.6f),
	F(IM, FxForm_Ring, 1.0f, 0.2f, 40, 0, 2.0f, 1.0f, 0.6f, FIX, kAir, 2.6f, 0.04f, COMET));

LOOK(kRogLithe, "lithe",
	F(IM, FxForm_Helix, 1.1f, 6.0f, 72, 3, 2.5f, 0, 1.0f, FIX, kRogMidnight, 2.6f, 0.04f, COMET),
	G(IM, Glyph_Feather, 0.5f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kFeather, 2.2f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 24, 12, 0.10f, 0, 0, FIX, kFeather, 2.2f));

LOOK(kRogWeaponBond, "weapon bond",
	G(IM, Glyph_Link, 0.4f, 4.5f, 3, 3, GA_Orbit, 1.0f, FIX, kIron, 2.6f),
	G(IM, Glyph_Sword, 0.6f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kRogBlade, 2.6f),
	F(IM, FxForm_Ring, 1.0f, 4.5f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kIron, 2.4f, 0.05f, COMET));

LOOK(kRogWind, "wind",
	F(IM, FxForm_Wind, 1.2f, 5.0f, 48, 8, 1.8f, 0, 0, FIX, kAir, 2.4f, 0.05f),
	F(IM, FxForm_Vortex, 0.9f, 5.0f, 48, 3, 1.0f, 1.5f, 2.0f, FIX, kAir, 2.2f, 0.04f),
	G(IM, Glyph_Tornado, 0.5f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kAir, 1.8f));

LOOK(kRogBreather, "breather",
	F(IM, FxForm_Bubbles, 1.0f, 6.0f, 40, 6, 0.2f, 0, 0, FIX, kRogSmoke, 3.0f, 0.04f, DARK),
	G(IM, Glyph_Moon, 0.45f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kRogMidnight, 3.0f),
	F(IM, FxForm_Bloom, 1.4f, 1.0f, 40, 2, 0, 0, 0, FIX, kRogMidnight, 3.0f, 0.04f));

LOOK(kRogDuelist, "duelist",
	G(IM, Glyph_Sword, 0.6f, 5.5f, 3, 2, GA_PopSpin, 0.35f, FIX, kRogBlade, 3.0f),
	F(IM, FxForm_Ring, 1.4f, 0.2f, 48, 0, 1.0f, 1.0f, 0.4f, FIX, kRogEdge, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.0f, 5.5f, 20, 10, 0.10f, 0, 0, FIX, kSilver, 2.4f));

LOOK(kRogCounterattack, "counterattack",
	G(IM, Glyph_Sword, 0.8f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kRogBlade, 2.4f),
	F(IM, FxForm_Burst, 2.0f, 3.5f, 64, 2, 0, 12, 0, FIX, kRogBlade, 1.0f, 0.07f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Lattice, 1.2f, 5.5f, 48, 6, 0.3f, 0, 0, FIX, kIron, 2.4f, 0.04f));

LOOK(kRogKinesthetics, "kinesthetics",
	F(IM, FxForm_Orbit, 1.3f, 5.5f, 60, 12, 0, 0, 0, FIX, kRogMidnight, 3.0f, 0.04f, ADD, 3.0f),
	G(IM, Glyph_Fist, 0.6f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kRogBlade, 2.4f),
	F(IM, FxForm_Spire, 0.6f, 5.0f, 48, 8, 0.4f, 1.8f, 0, FIX, kRogMidnight, 1.4f, 0.05f));

LOOK(kRogPrecision, "deadly precision",
	G(IM, Glyph_Eye, 0.7f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kRogBlood, 2.6f),
	F(IM, FxForm_Spire, 0.2f, 7.6f, 48, 4, 0.1f, 2.0f, 0, FIX, kRogEdge, 2.0f, 0.04f),
	F(IM, FxForm_Rune, 1.0f, 3.2f, 40, 4, 1, 4, 1, FIX, kRogEdge, 2.6f, 0.04f));

LOOK(kRogTwistedChance, "twisted chance",
	G(IM, Glyph_Coin, 0.6f, 7.4f, 4, 1, GlyphAnim_Pop | GlyphAnim_Roll | GlyphAnim_Bob, 0, FIX, kRogCoin, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 80, 2, 3.0f, 0, 1.0f, FIX, kRogChaos, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.12f, 1, 0, FIX, kRogCoin, 2.4f));

LOOK(kRogFrenziedStabbing, "frenzied stabbing",
	G(IM, Glyph_Sword, 0.45f, 4.5f, 3, 6, GA_PopSpin, 0.7f, FIX, kRogEdge, 2.4f),
	F(IM, FxForm_Spire, 0.5f, 4.5f, 96, 16, 0.4f, 1.5f, 0, FIX, kRogBlade, 2.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 5.0f, 40, 20, 0.10f, 0, 0, FIX, kRogBlood, 2.0f));

LOOK(kRogFeint, "assassin's feint",
	G(IM, Glyph_Mask, 0.7f, 7.2f, 4, 1, GA_PopRoll, 0, FIX, kRogBlade, 1.6f),
	F(IM, FxForm_Wind, 1.2f, 4.0f, 40, 8, 2.0f, 0, 0, FIX, kRogShade, 1.4f, 0.05f, DARK),
	F(IM, FxForm_Bloom, 1.4f, 4.0f, 32, 2, 0, 0, 0, FIX, kRogBlade, 1.0f, 0.04f));

LOOK(kAaAppraisal, "appraisal",
	G(IM, Glyph_Coin, 0.8f, 7.2f, 4, 1, GA_PopRoll, 0, FIX, kRogCoin, 2.2f),
	G(IM, Glyph_Scales, 0.6f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kRogCoin, 2.2f, 0.2f),
	F(IM, FxForm_Sparkle, 1.0f, 7.0f, 24, 12, 0.10f, 1, 0, FIX, kRogCoin, 2.0f));

LOOK(kAaPremonition, "assassin's premonition",
	G(IM, Glyph_Eye, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kRogShade, 3.0f, 0, DARK),
	F(IM, FxForm_Rune, 1.6f, 0, 70, 6, 1, 12, 0, FIX, kRogMidnight, 3.0f, 0.05f),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.9f, 0.8f, 0, FIX, kRogShade, 3.0f, 0.06f, DARK),
	G(IM, Glyph_Sword, 0.4f, 4.0f, 3, 4, GA_Orbit, 1.4f, FIX, kRogBlade, 3.0f));

LOOK(kAaParry, "assassin's parry",
	G(IM, Glyph_Sword, 0.8f, 5.0f, 3, 2, GlyphAnim_Pop, 0.3f, FIX, kRogBlade, 1.0f),
	F(IM, FxForm_Burst, 1.6f, 4.5f, 40, 1, 0, 10, 0, FIX, kSilver, 0.5f, 0.07f));

LOOK(kAaSynergy, "blackguard's synergy",
	G(IM, Glyph_Sword, 0.6f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kRogShade, 1.6f, 0, DARK),
	F(IM, FxForm_Ring, 1.0f, 4.0f, 40, 0, 2.0f, 1.0f, 0.8f, FIX, kRogMidnight, 1.4f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 24, 12, 0.10f, 0, 0, FIX, kRogMidnight, 1.4f));

LOOK(kAaDisguise, "cunning disguise",
	F(IM, FxForm_Bubbles, 1.0f, 6.5f, 48, 8, 0.25f, 0, 0, FIX, kRogSmoke, 2.0f, 0.05f, DARK),
	G(IM, Glyph_Mask, 0.9f, 7.2f, 4, 1, GA_PopRoll, 0, FIX, kIllusion, 2.4f, 0.2f),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.12f, 1, 0, FIX, kIllusion, 2.0f, 0.06f, ADD, 1, 0.3f));

LOOK(kAaPurgePoison, "purge poison",
	F(IM, FxForm_Drain, 1, 0, 32, 3, 1.0f, 12, 0, FIX, kRogVenom, 1.8f, 0.07f, ADD | FxFlag_Reverse),
	G(IM, Glyph_Snake, 0.7f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kRogVenom, 1.8f),
	F(IM, FxForm_Burst, 1.8f, 3.5f, 48, 2, 1.5f, 10, 0, FIX, kWhite, 0.8f, 0.06f, ADD, 1, 0.3f));

LOOK(kAaEnvenomed, "envenomed blade",
	G(IM, Glyph_Snake, 0.6f, 4.5f, 4, 1, GA_PopRoll, 0, FIX, kRogVenom, 0.8f),
	F(IM, FxForm_Bubbles, 0.7f, 5.0f, 24, 4, 0.12f, 0, 0, FIX, kRogVenom, 1.0f, 0.03f));

LOOK(kAaEscape, "escape",
	F(RE, FxForm_Bubbles, 1.4f, 6.5f, 96, 16, 0.4f, 0, 0, FIX, kRogSmoke, 3.0f, 0.06f, DARK),
	F(RE, FxForm_Vortex, 1.4f, 6.0f, 96, 6, 1.4f, 2.0f, -3.0f, FIX, kRogShade, 2.4f, 0.08f, DARK),
	G(RE, Glyph_Bat, 0.4f, 6.0f, 0, 5, GA_Orbit | GlyphAnim_Ascend, 1.4f, FIX, kRogShade, 2.4f, 0.2f, DARK),
	F(RE, FxForm_Bloom, 2.0f, 0.3f, 48, 3, 0, 0, 0, FIX, kGrey, 2.0f, 0.06f, DARK));

LOOK(kAaShroud, "shroud of stealth",
	F(IM, FxForm_Dome, 1.8f, 6.5f, 96, 8, 3, 0, 0, FIX, kRogShade, 3.0f, 0.06f, DARK),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kRogShade, 2.6f, 0.05f, DARK),
	G(IM, Glyph_Moon, 0.6f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kRogMidnight, 2.4f));

LOOK(kAaSharedShroud, "shared shroud of stealth",
	F(RE, FxForm_Bloom, 1.0f, 0.5f, 96, 3, 0, 0, 0, FIX, kRogShade, 2.6f, 0.07f, DARK, 1, 0, AE),
	F(RE, FxForm_Bubbles, 1.0f, 5.0f, 96, 12, 0.3f, 0, 0, FIX, kRogSmoke, 2.6f, 0.05f, DARK, 1, 0, AE),
	G(RE, Glyph_Moon, 0.4f, 6.5f, 0, 4, GA_Orbit, 1.4f, FIX, kRogMidnight, 2.4f));

LOOK(kAaFlurry, "flurry of daggers",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.3f, 0.6f, FIX, kRogBlade, 0.6f, 0.10f),
	G(IM, Glyph_Sword, 0.4f, 6.0f, 4, 6, GA_Rain, 1.0f, FIX, kRogBlade, 1.2f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 72, 3, 0.4f, 20, 0, FIX, kRogEdge, 0.7f, 0.09f, ADD, 1, 0.3f));

LOOK(kAaRakesRampage, "rake's rampage",
	F(RE, FxForm_Ring, 1.0f, 3.5f, 96, 0, 8.0f, 1.0f, 0, FIX, kRogBlade, 1.0f, 0.10f, COMET, 1, 0, AE),
	F(RE, FxForm_Ring, 0.8f, 3.0f, 96, 0, -8.0f, 1.0f, 0, FIX, kRogEdge, 1.0f, 0.08f, COMET, 1, 0.2f, AE),
	G(RE, Glyph_Sword, 0.5f, 4.0f, 3, 6, GA_PopSpin, 1.6f, FIX, kRogBlade, 1.2f),
	F(RE, FxForm_Sparkle, 0.8f, 4.0f, 60, 30, 0.12f, 0, 0, FIX, kRogBlood, 1.4f, 0.06f, ADD, 1, 0.3f, AE));

LOOK(kAaFocusedRampage, "focused rake's rampage",
	G(IM, Glyph_Sword, 0.5f, 4.5f, 3, 6, GA_PopSpin, 0.8f, FIX, kRogEdge, 1.4f),
	F(IM, FxForm_Vortex, 1.0f, 5.0f, 72, 6, 1.0f, 2.0f, 8.0f, FIX, kRogBlade, 1.4f, 0.06f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 72, 3, 1.0f, 18, 0, FIX, kRogBlood, 0.8f, 0.09f, ADD, 1, 0.3f));

LOOK(kAaRakesWeakening, "rake's weakening",
	G(IM, Glyph_Claw, 0.8f, 5.5f, 4, 1, GA_PopRoll, 0, FIX, kRogEdge, 1.2f),
	G(IM, Glyph_ArrowDown, 0.35f, 6.5f, 0, 3, GA_Orbit, 1.1f, FIX, kRogBlood, 2.4f, 0.2f));

LOOK(kAaIntimidation, "intimidation",
	G(IM, Glyph_Skull, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kRogShade, 2.0f, 0, DARK),
	F(IM, FxForm_Bloom, 2.0f, 4.0f, 48, 3, 0, 0, 0, FIX, kRogShade, 1.6f, 0.06f, DARK),
	F(IM, FxForm_Burst, 1.8f, 5.0f, 40, 1, 0, 10, 0, FIX, kRogBlade, 0.5f, 0.06f));

LOOK(kAaLethality, "lethality",
	G(IM, Glyph_Skull, 0.5f, 5.5f, 4, 1, GA_PopAscend, 0, FIX, kRogBlood, 1.2f),
	F(IM, FxForm_Spire, 0.3f, 5.0f, 48, 6, 0.2f, 1.6f, 0, FIX, kRogBlood, 0.6f, 0.06f));

LOOK(kAaLigamentSlice, "ligament slice",
	G(IM, Glyph_Link, 0.4f, 1.0f, 3, 2, GlyphAnim_Pop, 0.4f, FIX, kRogBlood, 2.0f),
	F(IM, FxForm_Ring, 0.8f, 1.2f, 40, 0, 4.0f, 1.0f, 0, FIX, kRogEdge, 0.5f, 0.06f, COMET),
	F(IM, FxForm_Burst, 1.4f, 1.2f, 40, 1, 0, 10, 0, FIX, kRogBlood, 0.6f, 0.07f, ADD, 1, 0.15f));

LOOK(kAaMassiveStrike, "massive strike",
	G(IM, Glyph_Sword, 1.3f, 9.0f, 4, 1, GA_Drop, 0, FIX, kRogBlade, 1.2f),
	F(IM, FxForm_Burst, 2.6f, 0.4f, 96, 3, 0.6f, 20, 0, FIX, kRogEdge, 0.9f, 0.11f, ADD, 1, 0.3f),
	F(IM, FxForm_Cracks, 1.6f, 0, 40, 6, 1.0f, 0, 0, FIX, kIron, 1.4f, 0.05f, ADD, 1, 0.3f));

LOOK(kAaRoguesFury, "rogue's fury",
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.8f, 1.2f, 0, FIX, kRogEdge, 3.0f, 0.09f),
	G(IM, Glyph_Sword, 0.5f, 4.0f, 3, 4, GA_Orbit, 1.4f, FIX, kRogBlade, 3.0f),
	G(IM, Glyph_Skull, 0.7f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kRogBlood, 3.0f),
	F(IM, FxForm_Burst, 2.8f, 0.3f, 72, 2, 0.4f, 16, 0, FIX, kRogBlood, 1.0f, 0.11f));

LOOK(kAaFlanking, "shadow's flanking",
	F(RE, FxForm_Wind, 1.4f, 4.0f, 60, 12, 3.0f, 0, 0, FIX, kRogShade, 1.4f, 0.05f, DARK),
	G(RE, Glyph_Bat, 0.4f, 4.5f, 3, 2, GA_PopSpin, 1.0f, FIX, kRogShade, 1.6f, 0, DARK),
	F(IM, FxForm_Ring, 1.3f, 4.0f, 48, 0, 3.0f, 1.0f, 0.5f, FIX, kRogMidnight, 2.4f, 0.05f, COMET));

LOOK(kAaSleight, "sleight of hand",
	G(RE, Glyph_Hand, 0.7f, 6.5f, 4, 1, GA_PopRoll, 0, FIX, kRogShade, 1.8f, 0, DARK),
	G(RE, Glyph_Coin, 0.3f, 6.0f, 0, 3, GA_Orbit, 0.9f, FIX, kRogCoin, 1.8f),
	F(RE, FxForm_Bubbles, 1.0f, 5.0f, 40, 6, 0.25f, 0, 0, FIX, kRogSmoke, 1.8f, 0.04f, DARK));

LOOK(kAaSleightJolt, "sleight of hand jolt",
	G(IM, Glyph_Coin, 0.5f, 6.5f, 4, 1, GA_PopAscend, 0, FIX, kRogCoin, 1.0f),
	F(IM, FxForm_Bloom, 1.2f, 4.0f, 32, 2, 0, 0, 0, FIX, kRogSmoke, 1.0f, 0.04f, DARK | FxFlag_Reverse));

LOOK(kAaSpireRake, "spire of the rake",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kRogShade, 2.6f, 0.10f, DARK),
	G(RE, Glyph_Sword, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kRogBlade, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kRogMidnight, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaGetaway, "stealthy getaway",
	F(RE, FxForm_Bubbles, 1.2f, 5.5f, 64, 10, 0.3f, 0, 0, FIX, kRogSmoke, 2.2f, 0.05f, DARK),
	F(RE, FxForm_Wind, 1.4f, 4.5f, 60, 12, 3.5f, 0, 0, FIX, kRogShade, 1.6f, 0.05f, DARK),
	G(RE, Glyph_Feather, 0.5f, 6.0f, 4, 1, GA_PopAscend, 0, FIX, kRogSmoke, 1.6f));

LOOK(kAaStrikeFury, "strike fury",
	G(IM, Glyph_Fist, 0.6f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kRogEdge, 0.8f),
	F(IM, FxForm_Burst, 1.6f, 4.5f, 40, 1, 0.2f, 12, 0, FIX, kRogEdge, 0.5f, 0.07f));

LOOK(kAaTumble, "tumble",
	F(RE, FxForm_Ring, 0.9f, 3.0f, 48, 0, 6.0f, 1.0f, 0, FIX, kRogMidnight, 0.8f, 0.07f, COMET),
	F(RE, FxForm_Wind, 1.2f, 2.5f, 40, 8, 2.5f, 0, 0, FIX, kRogSmoke, 1.0f, 0.05f, DARK),
	F(RE, FxForm_Bloom, 1.4f, 0.2f, 32, 2, 0, 0, 0, FIX, kStone, 1.0f, 0.05f, OVER));

LOOK(kAaTwistedExploit, "twisted exploit",
	G(IM, Glyph_Spiral, 0.8f, 6.5f, 4, 1, GA_PopRoll, 0, FIX, kRogChaos, 1.2f),
	F(IM, FxForm_Vortex, 0.8f, 5.0f, 48, 4, 1.0f, 2.0f, -6.0f, FIX, kRogChaos, 1.0f, 0.05f));

LOOK(kAaTwistedShank, "twisted shank",
	G(IM, Glyph_Sword, 1.0f, 5.0f, 4, 1, GlyphAnim_Pop | GlyphAnim_Roll, 0, FIX, kRogEdge, 1.0f),
	F(IM, FxForm_Vortex, 0.9f, 5.0f, 64, 4, 1.2f, 2.0f, 8.0f, FIX, kRogBlood, 1.0f, 0.06f),
	F(IM, FxForm_Burst, 1.8f, 5.0f, 48, 1, 0, 12, 0, FIX, kRogBlood, 0.6f, 0.08f, ADD, 1, 0.3f),
	G(IM, Glyph_Drop, 0.2f, 5.0f, 0, 4, GA_Rain, 0.7f, FIX, kRogBlood, 1.6f, 0.35f));

LOOK(kAaWhisperStep, "whisper step",
	F(RE, FxForm_Wind, 1.2f, 3.0f, 40, 8, 2.5f, 0, 0, FIX, kGhost, 1.2f, 0.04f),
	G(RE, Glyph_Feather, 0.35f, 0.8f, 3, 3, GA_PopAscend, 0.8f, FIX, kGhost, 1.4f));

enum Line : int
{
	Blinding, Bleed, Ambush, Jugular, DaggerStrike, DaggerThrow, Assault, Puncture, HuntersDagger,
	SneakAttack, Vengeance, AssassinsStrike, ChaosStrike, Daggerfall, Razorarc, Toxicity, ProcureSap,
	ToxicBlade, PoisonDisc, Alliance, SpikeTrap, Pinpoint, Mark, Smoke, Misdirection, Distraction,
	Escape, Indiscernible, ThiefsVision, Phantom, SpellEvasion, Executioner, Edge, Eradicator, Aim,
	Knifeplay, Reflexes, HiddenBlade, Weapons, WeaponTrigger, Speed, SpeedShock, Footed, Lithe,
	WeaponBond, Wind, Breather, Duelist, Counterattack, Kinesthetics, Precision, TwistedChance,
	FrenziedStabbing, Feint,
	Appraisal, Premonition, Parry, Synergy, Disguise, PurgePoison, Envenomed, AaEscape, Shroud,
	SharedShroud, Flurry, RakesRampage, FocusedRampage, RakesWeakening, Intimidation, Lethality,
	LigamentSlice, MassiveStrike, RoguesFury, Flanking, Sleight, SleightJolt, SpireRake, Getaway,
	StrikeFury, Tumble, TwistedExploit, TwistedShank, WhisperStep,
};

const LineLook kLines[] = {
	{ "Blinding (strike, blind)",         &kRogBlinding },
	{ "Bleed (strike, dot)",              &kRogBleed },
	{ "Ambush (opening strike)",          &kRogAmbush },
	{ "Jugular (strike)",                 &kRogJugular },
	{ "Daggerstrike (strike)",            &kRogDaggerStrike },
	{ "Dagger-Throw (strike)",            &kRogDaggerThrow },
	{ "Assault (strike)",                 &kRogAssault },
	{ "Puncture (strike, debuff)",        &kRogPuncture },
	{ "Shadow-Hunter's Dagger",           &kRogHuntersDagger },
	{ "Sneak Attack",                     &kRogSneakAttack },
	{ "Thief's Vengeance",                &kRogVengeance },
	{ "Assassin's Strike",                &kRogAssassinsStrike },
	{ "Chaos Strike",                     &kRogChaosStrike },
	{ "Daggerfall",                       &kRogDaggerfall },
	{ "Razorarc",                         &kRogRazorarc },
	{ "Toxicity (poison craft)",          &kRogToxicity },
	{ "Procure Sap",                      &kRogProcureSap },
	{ "Toxic Blade (poison proc)",        &kRogToxicBlade },
	{ "Venom (discipline)",               &kRogPoisonDisc },
	{ "Poisonous Alliance",               &kRogAlliance },
	{ "Spike Trap (poison)",              &kRogSpikeTrap },
	{ "Pinpoint (debuff)",                &kRogPinpoint },
	{ "Mark (debuff)",                    &kRogMark },
	{ "Smoke (hate reduction)",           &kRogSmokescreen },
	{ "Misdirection (hate reduction)",    &kRogMisdirection },
	{ "Distraction (hate reduction)",     &kRogDistraction },
	{ "Escape (hide)",                    &kRogEscape },
	{ "Indiscernible (discipline)",       &kRogIndiscernible },
	{ "Thief's Vision",                   &kRogThiefsVision },
	{ "Phantom Assassin (swarm)",         &kRogPhantom },
	{ "Spell Evasion (discipline)",       &kRogSpellEvasion },
	{ "Executioner (discipline)",         &kRogExecutioner },
	{ "Edge (discipline)",                &kRogEdgeDisc },
	{ "Eradicator (discipline)",          &kRogEradicator },
	{ "Aim (discipline)",                 &kRogAim },
	{ "Knifeplay (discipline)",           &kRogKnifeplay },
	{ "Reflexes (discipline)",            &kRogReflexes },
	{ "Hidden Blade (discipline)",        &kRogHiddenBlade },
	{ "Weapons (dual proc)",              &kRogWeapons },
	{ "Weapon Trigger",                   &kRogWeaponTrigger },
	{ "Speed (discipline)",               &kRogSpeed },
	{ "Speed Shock",                      &kRogSpeedShock },
	{ "Footed (discipline)",              &kRogFooted },
	{ "Lithe (discipline)",               &kRogLithe },
	{ "Weapon Bond",                      &kRogWeaponBond },
	{ "Wind (endurance)",                 &kRogWind },
	{ "Breather (endurance)",             &kRogBreather },
	{ "Duelist (discipline)",             &kRogDuelist },
	{ "Counterattack (discipline)",       &kRogCounterattack },
	{ "Kinesthetics (discipline)",        &kRogKinesthetics },
	{ "Deadly Precision (discipline)",    &kRogPrecision },
	{ "Twisted Chance (discipline)",      &kRogTwistedChance },
	{ "Frenzied Stabbing (discipline)",   &kRogFrenziedStabbing },
	{ "Assassin's Feint",                 &kRogFeint },

	{ "AA: Appraisal",                    &kAaAppraisal },
	{ "AA: Assassin's Premonition",       &kAaPremonition },
	{ "AA: Assassin's Parry",             &kAaParry },
	{ "AA: Blackguard's Synergy",         &kAaSynergy },
	{ "AA: Cunning Disguise",             &kAaDisguise },
	{ "AA: Purge Poison",                 &kAaPurgePoison },
	{ "AA: Envenomed Blade",              &kAaEnvenomed },
	{ "AA: Escape",                       &kAaEscape },
	{ "AA: Shroud of Stealth",            &kAaShroud },
	{ "AA: Shared Shroud of Stealth",     &kAaSharedShroud },
	{ "AA: Flurry of Daggers",            &kAaFlurry },
	{ "AA: Rake's Rampage",               &kAaRakesRampage },
	{ "AA: Focused Rake's Rampage",       &kAaFocusedRampage },
	{ "AA: Rake's Weakening",             &kAaRakesWeakening },
	{ "AA: Intimidation",                 &kAaIntimidation },
	{ "AA: Lethality",                    &kAaLethality },
	{ "AA: Ligament Slice",               &kAaLigamentSlice },
	{ "AA: Massive Strike",               &kAaMassiveStrike },
	{ "AA: Rogue's Fury",                 &kAaRoguesFury },
	{ "AA: Shadow's Flanking",            &kAaFlanking },
	{ "AA: Sleight of Hand",              &kAaSleight },
	{ "AA: Sleight of Hand Jolt",         &kAaSleightJolt },
	{ "AA: Spire of the Rake",            &kAaSpireRake },
	{ "AA: Stealthy Getaway",             &kAaGetaway },
	{ "AA: Strike Fury",                  &kAaStrikeFury },
	{ "AA: Tumble",                       &kAaTumble },
	{ "AA: Twisted Exploit",              &kAaTwistedExploit },
	{ "AA: Twisted Shank",                &kAaTwistedShank },
	{ "AA: Whisper Step",                 &kAaWhisperStep },
};

const GroupLine kNumbered[] = {
	{ 0, Blinding },    { 1, Bleed },       { 4, Executioner }, { 5, Pinpoint },    { 6, Ambush },
	{ 7, Jugular },     { 9, Smoke },       { 10, DaggerStrike }, { 11, ProcureSap }, { 13, Escape },
	{ 14, DaggerThrow }, { 15, Assault },   { 17, Mark },       { 18, Distraction }, { 19, Footed },
	{ 20, Misdirection }, { 21, Edge },     { 22, ToxicBlade }, { 23, Eradicator }, { 25, Indiscernible },
	{ 26, SpikeTrap },  { 27, Aim },        { 28, PoisonDisc }, { 29, ThiefsVision }, { 30, Knifeplay },
	{ 31, Phantom },    { 32, Reflexes },   { 33, HiddenBlade }, { 34, Weapons },   { 35, WeaponTrigger },
	{ 36, Speed },      { 37, HuntersDagger }, { 38, Alliance }, { 39, Puncture },
};

const GroupLine kModern[] = {
	{ 10, Blinding },   { 20, Bleed },      { 60, Pinpoint },   { 70, Ambush },     { 80, Jugular },
	{ 90, Smoke },      { 120, Escape },    { 130, DaggerThrow }, { 140, Assault }, { 150, Mark },
	{ 180, Misdirection }, { 190, Edge },   { 200, ToxicBlade }, { 220, Indiscernible }, { 240, Aim },
	{ 250, PoisonDisc }, { 300, HiddenBlade }, { 310, Weapons }, { 311, WeaponTrigger }, { 320, Speed },
	{ 330, HuntersDagger }, { 340, Alliance }, { 350, Puncture },
};

const GroupLine kOther[] = {
	{ 12102, Toxicity },  { 12202, Toxicity },  { 12302, Toxicity },  { 13003, Toxicity },  { 12502, SpellEvasion },
	{ 13000, Blinding },  { 13002, SpellEvasion }, { 13004, Lithe },  { 13005, Executioner }, { 13007, Pinpoint },
	{ 6198, Indiscernible }, { 15316, WeaponBond }, { 15416, WeaponBond }, { 15516, WeaponBond },
	{ 16008, Wind },      { 3105, Wind },       { 3205, Wind },       { 3305, Wind },       { 3405, Wind },
	{ 3505, Wind },       { 3311, Breather },   { 3411, Breather },   { 3415, Breather },   { 3511, Breather },
	{ 3611, Breather },   { 3737, Breather },   { 3837, Breather },   { 3937, Breather },
	{ 125010040, Breather }, { 126010040, Breather },
	{ 104659, SneakAttack }, { 104676, Duelist }, { 104685, Vengeance }, { 104686, AssassinsStrike },
	{ 105017, ChaosStrike }, { 105018, ChaosStrike }, { 106174, Daggerfall }, { 108001, ThiefsVision },
	{ 108470, Razorarc },
};

const NamedLine kAbilities[] = {
	{ "appraisal",                   Appraisal },
	{ "assassin's premonition",      Premonition },
	{ "assassin's parry",            Parry },
	{ "blackguard's synergy",        Synergy },
	{ "blackguard's synergy strike", Synergy },
	{ "purge poison",                PurgePoison },
	{ "enhanced purging",            PurgePoison },
	{ "envenomed blade",             Envenomed },
	{ "escape",                      AaEscape },
	{ "evader's shroud of stealth",  Shroud },
	{ "shared shroud of stealth",    SharedShroud },
	{ "flurry of daggers",           Flurry },
	{ "rake's rampage",              RakesRampage },
	{ "focused rake's rampage",      FocusedRampage },
	{ "rake's weakening",            RakesWeakening },
	{ "intimidation",                Intimidation },
	{ "shadow's intimidation",       Intimidation },
	{ "lethality",                   Lethality },
	{ "ligament slice",              LigamentSlice },
	{ "massive strike",              MassiveStrike },
	{ "rogue's fury",                RoguesFury },
	{ "shadow's flanking",           Flanking },
	{ "sleight of hand",             Sleight },
	{ "sleight of hand jolt",        SleightJolt },
	{ "spire of the rake",           SpireRake },
	{ "stealthy getaway",            Getaway },
	{ "strike fury",                 StrikeFury },
	{ "tumble",                      Tumble },
	{ "twisted exploit",             TwistedExploit },
	{ "twisted shank",               TwistedShank },
	{ "whisper step",                WhisperStep },
};

const NamedLine kGroupless[] = {
	{ "deadeye discipline",           Aim },
	{ "deadly aim discipline",        Aim },
	{ "nimble discipline",            Lithe },
	{ "kinesthetics discipline",      Kinesthetics },
	{ "counterattack discipline",     Counterattack },
	{ "blinding speed discipline",    Speed },
	{ "deadly precision discipline",  Precision },
	{ "twisted chance discipline",    TwistedChance },
	{ "frenzied stabbing discipline", FrenziedStabbing },
	{ "assassin's feint",             Feint },
	{ "rogue's ploy",                 Misdirection },
	{ "poison spikes trap",           SpikeTrap },
	{ "poison spurs trap",            SpikeTrap },
};

int BySpell(const eqlib::EQ_Spell*, std::string_view base)
{
	for (const NamedLine& n : kGroupless)
		if (base == n.name)
			return n.line;
	if (EndsWith(base, " speed shock"))
		return SpeedShock;
	if (StartsWith(base, "cunning disguise"))
		return Disguise;
	return -1;
}

} // namespace

const ClassCatalogue& RogueCatalogue()
{
	static const ClassCatalogue c{ "Rogue", eqlib::Rogue, kLines, 12, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
