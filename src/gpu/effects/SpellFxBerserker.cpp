#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

namespace eqgfx::spellfx {

namespace {

constexpr Pair kBerRage    { { 1.00f, 0.45f, 0.15f, 0.95f }, { 0.70f, 0.02f, 0.02f, 0.90f } };
constexpr Pair kBerBlood   { { 0.92f, 0.12f, 0.10f, 0.95f }, { 0.32f, 0.00f, 0.02f, 0.90f } };
constexpr Pair kBerAxe     { { 0.95f, 0.96f, 1.00f, 0.95f }, { 0.75f, 0.10f, 0.10f, 0.90f } };
constexpr Pair kBerFury    { { 1.00f, 0.90f, 0.40f, 1.00f }, { 1.00f, 0.30f, 0.05f, 0.90f } };
constexpr Pair kBerWar     { { 1.00f, 0.78f, 0.45f, 0.95f }, { 0.60f, 0.30f, 0.10f, 0.90f } };
constexpr Pair kBerChaos   { { 1.00f, 0.30f, 0.55f, 0.95f }, { 0.40f, 0.05f, 0.35f, 0.85f } };
constexpr Pair kBerPhantom { { 1.00f, 0.72f, 0.72f, 0.60f }, { 0.60f, 0.10f, 0.20f, 0.30f } };
constexpr Pair kBerJugg    { { 0.85f, 0.85f, 0.90f, 0.95f }, { 0.90f, 0.30f, 0.08f, 0.90f } };
constexpr Pair kBerHide    { { 0.80f, 0.58f, 0.36f, 0.95f }, { 0.42f, 0.22f, 0.10f, 0.90f } };
constexpr Pair kBerEndur   { { 1.00f, 0.95f, 0.70f, 0.90f }, { 0.95f, 0.50f, 0.20f, 0.75f } };
constexpr Pair kBerDark    { { 0.55f, 0.05f, 0.05f, 0.90f }, { 0.10f, 0.00f, 0.00f, 0.85f } };

constexpr uint32_t DARK = OVER;

LOOK(kBerTendon, "tendon",
	G(IM, Glyph_Scythe, 0.7f, 1.2f, 1, 1, GA_PopRoll, 0, FIX, kBerAxe, 0.9f),
	F(IM, FxForm_Burst, 1.4f, 0.8f, 48, 1, 0.2f, 10, 0, FIX, kBerBlood, 0.7f, 0.08f),
	G(IM, Glyph_Drop, 0.25f, 1.5f, 0, 4, GA_Drop, 0.9f, FIX, kBerBlood, 2.0f, 0.2f),
	G(IM, Glyph_Link, 0.3f, 0.5f, 3, 3, GA_PopSpin, 0.8f, FIX, kIron, 2.4f, 0.2f));

LOOK(kBerTemple, "temple",
	G(IM, Glyph_Scythe, 0.9f, 8.6f, 1, 1, GA_Drop, 0, FIX, kBerAxe, 0.9f),
	G(IM, Glyph_Skull, 0.8f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kBone, 1.2f, 0.2f, OVER),
	F(IM, FxForm_Burst, 1.8f, 6.5f, 48, 1, 0.2f, 12, 0, FIX, kBerFury, 0.6f, 0.08f, ADD, 1, 0.25f),
	Stars(0.3f));

LOOK(kBerJarring, "jarring",
	G(IM, Glyph_Fist, 0.8f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kBerWar, 1.0f),
	F(IM, FxForm_Burst, 2.0f, 4.0f, 48, 2, 0.3f, 8, 0, FIX, kBerWar, 0.7f, 0.08f),
	G(IM, Glyph_ArrowDown, 0.5f, 8.0f, 4, 1, GA_Drop, 0, FIX, kGrey, 1.6f, 0.3f, OVER),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 24, 12, 0.10f, 1, 0, FIX, kGrey, 1.4f, 0.06f, OVER, 1, 0.3f));

LOOK(kBerFrenzy, "frenzy",
	G(IM, Glyph_Claw, 0.9f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kBerBlood, 0.9f),
	G(IM, Glyph_Scythe, 0.5f, 4.5f, 1, 3, GA_PopSpin | GlyphAnim_Roll, 0.9f, FIX, kBerAxe, 1.0f, 0.1f),
	F(IM, FxForm_Burst, 1.8f, 4.0f, 48, 2, 0.3f, 12, 0, FIX, kBerRage, 0.7f, 0.08f, ADD, 1, 0.2f));

LOOK(kBerOverpowering, "overpowering frenzy",
	G(IM, Glyph_Claw, 1.2f, 6.0f, 4, 1, GA_PopRoll, 0, FIX, kBerRage, 1.2f),
	F(IM, FxForm_Spire, 0.6f, 4.0f, 96, 16, 2.0f, 3.0f, 0, FIX, kBerBlood, 0.9f, 0.07f),
	F(IM, FxForm_Burst, 2.4f, 3.0f, 72, 3, 0.4f, 16, 0, FIX, kBerFury, 0.8f, 0.10f, ADD, 1, 0.2f),
	G(IM, Glyph_Drop, 0.25f, 4.0f, 0, 6, GA_Rain, 1.2f, FIX, kBerBlood, 1.6f, 0.2f));

LOOK(kBerAxeGods, "axe of the gods",
	G(IM, Glyph_Scythe, 1.5f, 9.0f, 1, 1, GA_Drop, 0, FIX, kBerAxe, 1.6f),
	F(IM, FxForm_Lightning, 1, 36.0f, 48, 2, 0.4f, 10.0f, 1.0f, FIX, kBerFury, 0.8f, 0.12f, ADD, 1, 0.2f),
	F(IM, FxForm_Cracks, 2.2f, 0, 48, 6, 1.1f, 0, 0, FIX, kBerRage, 1.8f, 0.07f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.6f, 0.4f, 72, 2, 0.6f, 16, 0, FIX, kBerFury, 0.9f, 0.10f, ADD, 1, 0.35f),
	Stars(0.4f));

LOOK(kBerAxeAeons, "axe of the aeons",
	G(IM, Glyph_Scythe, 1.3f, 9.0f, 1, 1, GA_Drop, 0, FIX, kBerJugg, 1.6f),
	F(IM, FxForm_Rune, 1.6f, 0, 60, 6, 1, 12, 0, FIX, kBerFury, 1.8f, 0.05f, ADD, 1, 0.35f),
	F(IM, FxForm_Spire, 0.6f, 6.0f, 96, 16, 1.0f, 3.0f, 0, FIX, kBerRage, 1.2f, 0.07f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.4f, 0.5f, 72, 2, 0.8f, 16, 0, FIX, kBerBlood, 0.9f, 0.10f, ADD, 1, 0.35f));

LOOK(kBerArc, "arcblade",
	F(IM, FxForm_Ring, 1.6f, 4.0f, 48, 0, 4.0f, 1.0f, 0.1f, FIX, kBerAxe, 0.6f, 0.10f, COMET),
	G(IM, Glyph_Scythe, 0.9f, 4.5f, 1, 1, GA_PopRoll, 0, FIX, kBerAxe, 0.8f),
	F(IM, FxForm_Burst, 1.0f, 0.5f, 72, 1, 0.3f, 16, 0, FIX, kBerRage, 0.8f, 0.09f, ADD, 1, 0.15f, AE));

LOOK(kBerInsult, "insult",
	G(IM, Glyph_Hand, 0.8f, 6.8f, 4, 1, GA_PopRoll, 0, FIX, kBerWar, 0.9f),
	G(IM, Glyph_Mouth, 0.6f, 8.2f, 4, 1, GA_PopAscend, 0, FIX, kBerRage, 1.6f, 0.2f),
	F(IM, FxForm_Burst, 1.6f, 6.0f, 40, 1, 0.2f, 10, 0, FIX, kBerRage, 0.6f, 0.08f));

LOOK(kBerSapping, "sapping strikes",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.2f, 12, 0, FIX, kBerEndur, 1.8f, 0.08f),
	G(IM, Glyph_Claw, 0.7f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kBerBlood, 1.0f),
	F(IM, FxForm_Vortex, 1.0f, 4.5f, 64, 4, 1.0f, 2.0f, -3.0f, FIX, kBerEndur, 1.6f, 0.05f));

LOOK(kBerAxeThrow, "axe throw",
	F(TR, FxForm_Bolt, 1, 0, 64, 2, 0.5f, 0.2f, 0.9f, FIX, kBerAxe, 0.6f, 0.14f),
	G(IM, Glyph_Scythe, 0.8f, 4.5f, 4, 1, GA_PopRoll, 0, FIX, kBerAxe, 0.9f),
	F(IM, FxForm_Burst, 1.6f, 4.0f, 48, 1, 0.2f, 10, 0, FIX, kBerBlood, 0.6f, 0.08f));

LOOK(kBerBloodAxe, "blood axe",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.35f, 0.3f, 1.4f, FIX, kBerBlood, 0.7f, 0.16f),
	G(IM, Glyph_Scythe, 0.9f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kBerBlood, 1.0f),
	F(IM, FxForm_Burst, 2.2f, 4.0f, 64, 2, 0.4f, 14, 0, FIX, kBerBlood, 0.8f, 0.09f),
	G(IM, Glyph_Drop, 0.25f, 5.0f, 0, 5, GA_Rain, 1.0f, FIX, kBerBlood, 1.8f, 0.2f));

LOOK(kBerVolley, "volley",
	G(RE, Glyph_Scythe, 0.5f, 5.0f, 1, 6, GA_PopSpin | GlyphAnim_Roll, 1.8f, FIX, kBerAxe, 1.2f),
	F(RE, FxForm_Ring, 1.0f, 4.0f, 64, 0, 3.0f, 1.0f, 0.2f, FIX, kBerAxe, 1.0f, 0.06f, COMET, 1, 0, AE),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 2, 0.6f, 20, 0, FIX, kBerBlood, 1.0f, 0.09f, ADD, 1, 0.3f, AE),
	G(IM, Glyph_Scythe, 0.6f, 4.5f, 4, 1, GA_PopRoll, 0, FIX, kBerAxe, 0.8f));

LOOK(kBerVicious, "vicious spiral",
	F(RE, FxForm_Vortex, 1.6f, 4.0f, 96, 5, 0.6f, 2.0f, 6.0f, FIX, kBerAxe, 1.4f, 0.07f),
	G(RE, Glyph_Scythe, 0.6f, 3.5f, 1, 4, GA_PopSpin | GlyphAnim_Roll, 1.6f, FIX, kBerAxe, 1.4f),
	F(RE, FxForm_Burst, 1.0f, 0.3f, 96, 2, 0.3f, 20, 0, FIX, kBerBlood, 1.0f, 0.09f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Sparkle, 1.0f, 5.0f, 24, 12, 0.10f, 1, 0, FIX, kBerBlood, 1.0f));

LOOK(kBerSummonAxe, "summon axes",
	F(RE, FxForm_Rune, 1.2f, 0, 60, 6, 1, 12, 0, FIX, kBerWar, 2.0f, 0.05f),
	G(RE, Glyph_Scythe, 0.8f, 5.5f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kBerAxe, 2.2f),
	F(RE, FxForm_Sparkle, 1.0f, 6.0f, 24, 12, 0.10f, 2, 0, FIX, kBerWar, 2.0f));

LOOK(kBerScream, "scream",
	G(RE, Glyph_Mouth, 0.9f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kBerRage, 1.6f),
	F(RE, FxForm_Bloom, 2.2f, 6.0f, 48, 3, 0, 0, 0, FIX, kBerRage, 1.4f, 0.06f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kBerBlood, 1.6f, 0.05f, ADD, 1.4f, 0.1f, AE),
	G(IM, Glyph_Spiral, 0.5f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kBerChaos, 1.4f, 0.2f));

LOOK(kBerWarCry, "war cry",
	G(RE, Glyph_Horn, 0.9f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kBerWar, 2.0f),
	G(RE, Glyph_Mouth, 0.35f, 6.0f, 0, 4, GA_RingRise, 1.8f, FIX, kBerWar, 2.0f, 0.2f),
	F(RE, FxForm_Bloom, 1.0f, 4.0f, 96, 4, 0, 0, 0, FIX, kBerWar, 2.0f, 0.07f, ADD, 1.4f, 0, AE),
	G(IM, Glyph_ArrowUp, 0.4f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kBerRage, 1.6f));

LOOK(kBerCryHavoc, "cry havoc",
	G(RE, Glyph_Horn, 1.0f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kBerRage, 2.2f),
	F(RE, FxForm_Bloom, 2.4f, 6.0f, 48, 3, 0, 0, 0, FIX, kBerRage, 1.6f, 0.06f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 3, 1.2f, 20, 0, FIX, kBerFury, 1.4f, 0.10f, ADD, 1, 0.2f, AE),
	G(IM, Glyph_Fist, 0.5f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kBerFury, 1.8f));

LOOK(kBerCryCarnage, "cry carnage",
	G(RE, Glyph_Horn, 0.9f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kBerWar, 1.8f),
	F(RE, FxForm_Bloom, 1.0f, 3.0f, 96, 4, 0, 0, 0, FIX, kBerBlood, 2.0f, 0.07f, ADD, 1.4f, 0, AE),
	G(RE, Glyph_Skull, 0.35f, 6.0f, 0, 5, GA_RingRise, 1.8f, FIX, kBerBlood, 2.2f, 0.2f),
	F(IM, FxForm_Flame, 0.9f, 5.0f, 72, 18, 0.9f, 0.8f, 0, FIX, kBerRage, 2.0f, 0.06f));

LOOK(kBerShared, "shared bloodlust",
	G(RE, Glyph_Fang, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kBerBlood, 2.0f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kBerBlood, 2.0f, 0.07f, ADD, 1.2f, 0, AE),
	F(IM, FxForm_Flame, 0.9f, 5.0f, 72, 18, 0.9f, 0.8f, 0, FIX, kBerRage, 2.2f, 0.06f),
	Persist(F(AU, FxForm_Orbit, 1.1f, 5.0f, 40, 8, 0, 0, 0, FIX, kBerBlood, 0, 0.04f, ADD, 2.0f)));

LOOK(kBerAlliance, "alliance",
	G(IM, Glyph_Link, 0.4f, 5.0f, 3, 6, GA_PopSpin, 1.2f, FIX, kBerJugg, 3.0f),
	G(IM, Glyph_Banner, 0.8f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kBerBlood, 3.0f),
	F(IM, FxForm_Ring, 1.2f, 0.3f, 40, 0, 0.8f, 1.0f, 0.4f, FIX, kBerRage, 3.0f, 0.05f, COMET));

LOOK(kBerRageAura, "rage aura",
	F(RE, FxForm_Rune, 1.0f, 0, 80, 8, 3, 16, 0, FIX, kBerBlood, 2.6f, 0.05f, ADD, 1, 0, AE),
	F(RE, FxForm_Flame, 0.9f, 1.4f, 128, 32, 0.1f, 0.6f, 0, FIX, kBerRage, 2.4f, 0.06f, ADD, 1, 0.2f, AE),
	Persist(F(AU, FxForm_Ring, 1.2f, 0.3f, 40, 0, 0.8f, 1.0f, 0.4f, FIX, kBerBlood, 0, 0.05f, COMET)));

LOOK(kBerBerserking, "berserking",
	F(IM, FxForm_Flame, 1.1f, 7.0f, 112, 28, 0.8f, 1.4f, 0, FIX, kBerRage, 3.0f, 0.10f),
	G(IM, Glyph_Fang, 0.9f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kBerBlood, 2.8f),
	F(IM, FxForm_Burst, 2.8f, 0.3f, 72, 2, 0.4f, 16, 0, FIX, kBerBlood, 1.0f, 0.12f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kBerDark, 2.4f, 0.06f, DARK));

LOOK(kBerCleaving, "cleaving",
	F(IM, FxForm_Spire, 0.8f, 6.0f, 96, 16, 0.8f, 2.4f, 0, FIX, kBerRage, 2.0f, 0.06f),
	G(IM, Glyph_Scythe, 0.6f, 4.5f, 3, 2, GA_Orbit, 1.3f, FIX, kBerAxe, 2.6f),
	F(IM, FxForm_Ring, 1.4f, 3.5f, 48, 0, 2.5f, 1.0f, 0.5f, FIX, kBerAxe, 2.4f, 0.06f, COMET));

LOOK(kBerFlurry, "flurry",
	G(IM, Glyph_Sword, 0.5f, 4.0f, 3, 5, GA_Orbit | GlyphAnim_Roll, 1.2f, FIX, kBerAxe, 3.0f),
	F(IM, FxForm_Wind, 1.2f, 4.5f, 60, 10, 2.5f, 0, 0, FIX, kBerRage, 2.4f, 0.05f),
	F(IM, FxForm_Orbit, 1.3f, 4.5f, 60, 12, 0, 0, 0, FIX, kBerFury, 3.0f, 0.05f, ADD, 3.0f));

LOOK(kBerSundering, "sundering",
	G(IM, Glyph_Scythe, 1.1f, 8.0f, 1, 1, GA_PopBob, 0, FIX, kBerAxe, 2.8f),
	G(IM, Glyph_Shield, 0.4f, 3.0f, 3, 4, GA_Rain, 1.2f, FIX, kGrey, 2.4f, 0.2f, OVER),
	F(IM, FxForm_Cracks, 2.4f, 0, 48, 8, 1.3f, 0, 0, FIX, kBerRage, 2.6f, 0.07f),
	F(IM, FxForm_Flame, 1.0f, 5.0f, 96, 24, 0.8f, 0.8f, 0, FIX, kBerBlood, 2.8f, 0.07f));

LOOK(kBerCapricious, "capricious rage",
	G(IM, Glyph_Spiral, 0.9f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kBerChaos, 2.6f),
	F(IM, FxForm_Flame, 1.05f, 5.8f, 96, 24, 0.9f, 1.0f, 0, FIX, kBerChaos, 3.0f, 0.08f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kBerRage, 2.6f));

LOOK(kBerAugmented, "augmented frenzy",
	Persist(F(AU, FxForm_Flame, 0.9f, 1.8f, 64, 16, 0.2f, 0.5f, 0, FIX, kBerRage, 0, 0.05f)),
	G(IM, Glyph_ArrowUp, 0.4f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kBerFury, 2.4f),
	F(IM, FxForm_Burst, 2.0f, 1.5f, 48, 1, 1.5f, 12, 0, FIX, kBerRage, 0.9f, 0.08f));

LOOK(kBerFestering, "festering rage",
	F(IM, FxForm_Bubbles, 0.9f, 5.5f, 48, 8, 0.2f, 0, 0, FIX, kBerBlood, 2.6f, 0.035f),
	G(IM, Glyph_Flame, 0.6f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kBerRage, 2.6f),
	F(IM, FxForm_Flame, 0.95f, 4.5f, 72, 18, 0.9f, 0.7f, 0, FIX, kBerBlood, 2.8f, 0.06f));

LOOK(kBerRageForm, "rage form",
	G(IM, Glyph_Sun, 0.7f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kBerFury, 2.6f),
	G(IM, Glyph_Moon, 0.7f, 8.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kBerBlood, 2.6f, 0.3f),
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.8f, 1.2f, 0, FIX, kBerRage, 3.0f, 0.09f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 96, 2, 1.0f, 0, 0.3f, FIX, kBerChaos, 3.0f, 0.05f, COMET));

LOOK(kBerMangling, "mangling",
	G(IM, Glyph_Claw, 1.1f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kBerBlood, 2.8f),
	G(IM, Glyph_Scythe, 0.45f, 4.0f, 3, 4, GA_Orbit | GlyphAnim_Roll, 1.3f, FIX, kBerAxe, 2.8f),
	F(IM, FxForm_Flame, 1.0f, 5.0f, 96, 24, 0.8f, 0.8f, 0, FIX, kBerBlood, 3.0f, 0.07f));

LOOK(kBerDisconcerting, "disconcerting",
	G(IM, Glyph_Eye, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kBerChaos, 2.6f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, 3.0f, FIX, kBerChaos, 2.6f, 0.05f),
	G(IM, Glyph_Spiral, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kBerRage, 2.6f));

LOOK(kBerPhantomLook, "phantom assailant",
	G(IM, Glyph_Spirit, 1.2f, 6.5f, 4, 1, GA_PopAscend, 0, FIX, kBerPhantom, 2.2f),
	G(IM, Glyph_Scythe, 0.8f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kBerPhantom, 1.4f, 0.3f),
	F(IM, FxForm_Flame, 1.0f, 7.0f, 80, 20, 0.2f, 1.2f, 0, FIX, kBerPhantom, 2.0f, 0.06f),
	F(IM, FxForm_Burst, 2.0f, 4.0f, 48, 1, 0.3f, 12, 0, FIX, kBerBlood, 0.7f, 0.08f, ADD, 1, 0.4f));

LOOK(kBerBlindRage, "blind rage",
	G(IM, Glyph_Eye, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kBerDark, 2.6f, 0, DARK),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 96, 24, 0.9f, 1.0f, 0, FIX, kBerBlood, 3.0f, 0.08f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.12f, 0, 0, FIX, kBerRage, 2.4f));

LOOK(kBerInspiredAnger, "inspired anger",
	G(IM, Glyph_Flame, 0.8f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kBerRage, 2.4f),
	F(IM, FxForm_Spire, 0.8f, 6.0f, 96, 16, 0.8f, 2.4f, 0, FIX, kBerFury, 2.0f, 0.06f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.10f, 1, 0, FIX, kBerFury, 2.0f));

LOOK(kBerReckless, "reckless",
	G(IM, Glyph_Skull, 0.7f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kBerRage, 2.6f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 96, 24, 0.8f, 0.9f, 0, FIX, kFlame2, 2.8f, 0.08f),
	F(IM, FxForm_Burst, 2.2f, 0.3f, 64, 2, 0.3f, 14, 0, FIX, kBerBlood, 0.9f, 0.09f));

LOOK(kBerFocusedFury, "focused fury",
	G(IM, Glyph_Eye, 0.6f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kBerFury, 2.4f),
	F(IM, FxForm_Vortex, 1.0f, 6.0f, 64, 4, 1.2f, 2.0f, -4.0f, FIX, kBerFury, 2.4f, 0.05f),
	F(IM, FxForm_Sparkle, 1.0f, 7.0f, 32, 16, 0.10f, 2, 0, FIX, kBerRage, 2.2f));

LOOK(kBerBattleFocus, "battle focus",
	G(IM, Glyph_Sword, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kBerAxe, 2.4f),
	F(IM, FxForm_Rune, 1.4f, 0, 60, 6, 1, 12, 0, FIX, kBerWar, 2.4f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.10f, 2, 0, FIX, kBerWar, 2.2f));

LOOK(kBerBurningRage, "burning rage",
	F(IM, FxForm_Flame, 1.1f, 7.0f, 112, 28, 0.9f, 1.4f, 0, FIX, kFire, 3.0f, 0.09f),
	G(IM, Glyph_Flame, 0.4f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kBerFury, 3.0f),
	F(IM, FxForm_Sparkle, 1.3f, 8.0f, 40, 20, 0.12f, 0, 0, FIX, kFire, 2.4f));

LOOK(kBerBloodthirst, "bloodthirst",
	G(IM, Glyph_Fang, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kBerBlood, 2.4f),
	F(IM, FxForm_Vortex, 1.1f, 4.5f, 64, 4, 1.0f, 2.0f, -3.0f, FIX, kBerBlood, 2.2f, 0.05f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 32, 16, 0.12f, 2, 0, FIX, kBerBlood, 2.0f));

LOOK(kBerBloodletting, "bloodletting",
	G(IM, Glyph_Scythe, 0.7f, 5.5f, 4, 1, GA_PopRoll, 0, FIX, kBerAxe, 1.0f),
	F(IM, FxForm_Rain, 0.8f, 5.0f, 32, 16, 0, 0.1f, 0, FIX, kBerBlood, 2.2f, 0.05f, ADD, 1, 0.2f),
	F(IM, FxForm_Bloom, 1.4f, 0.2f, 40, 2, 0, 0, 0, FIX, kBerDark, 2.4f, 0.06f, DARK, 1, 0.4f),
	G(IM, Glyph_Drop, 0.3f, 4.5f, 0, 3, GA_Orbit, 1.1f, FIX, kBerBlood, 2.4f, 0.3f));

LOOK(kBerRetaliation, "retaliation",
	G(IM, Glyph_Scythe, 0.5f, 4.0f, 3, 4, GA_Orbit, 1.3f, FIX, kBerAxe, 3.0f),
	F(IM, FxForm_Burst, 2.0f, 3.0f, 64, 2, 0, 12, 0, FIX, kBerRage, 1.0f, 0.07f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Ring, 1.3f, 4.0f, 40, 0, -1.5f, 1.0f, 0.5f, FIX, kBerBlood, 2.4f, 0.05f, COMET));

LOOK(kBerConditioned, "conditioned retaliation",
	G(IM, Glyph_Shield, 0.8f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kBerJugg, 2.0f),
	G(IM, Glyph_Scythe, 0.45f, 5.0f, 3, 3, GA_PopSpin, 1.0f, FIX, kBerAxe, 2.2f, 0.2f),
	F(IM, FxForm_Rune, 1.3f, 3.0f, 60, 6, 1, 12, 2, FIX, kBerRage, 2.4f, 0.05f));

LOOK(kBerUnyielding, "unyielding will",
	G(IM, Glyph_Helm, 0.9f, 7.6f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kBerJugg, 2.8f),
	F(IM, FxForm_Lattice, 1.3f, 6.0f, 64, 8, 0.4f, 0, 0, FIX, kBerJugg, 2.6f, 0.05f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 2.4f, 0.06f, OVER));

LOOK(kBerFrenziedResolve, "frenzied resolve",
	G(IM, Glyph_Fist, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kBerRage, 2.8f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 96, 24, 0.8f, 0.9f, 0, FIX, kBerRage, 3.0f, 0.07f),
	F(IM, FxForm_Dome, 1.6f, 6.4f, 88, 6, 2, 0, 0, FIX, kBerJugg, 2.2f, 0.03f));

LOOK(kBerIndomitable, "indomitable",
	G(IM, Glyph_Tower, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kBerJugg, 3.0f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 112, 8, 3, 0, 0, FIX, kBerJugg, 2.6f, 0.04f),
	F(IM, FxForm_Flame, 1.0f, 3.0f, 72, 18, 0.2f, 0.6f, 0, FIX, kBerRage, 2.6f, 0.06f));

LOOK(kBerWind, "wind",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 12, 2.5f, 0, 0, FIX, kBerEndur, 2.4f, 0.05f),
	G(IM, Glyph_Tornado, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kBerEndur, 2.0f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.10f, 1, 0, FIX, kBerEndur, 2.0f));

LOOK(kBerRejuvenation, "rejuvenation",
	F(IM, FxForm_Helix, 1.0f, 6.0f, 72, 3, 1.2f, 0, 0.5f, FIX, kBerEndur, 2.6f, 0.05f, COMET),
	G(IM, Glyph_Heart, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kBerRage, 2.0f),
	F(IM, FxForm_Sparkle, 1.1f, 6.5f, 32, 16, 0.10f, 0, 0, FIX, kHeal, 2.2f));

LOOK(kBerSprint, "sprint",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 12, 3.5f, 0, 0, FIX, kBerEndur, 2.6f, 0.05f),
	G(IM, Glyph_Feather, 0.45f, 6.8f, 0, 1, GA_PopAscend, 0, FIX, kBerEndur, 1.6f),
	F(IM, FxForm_Cracks, 1.4f, 0, 32, 4, 0.8f, 0, 0, FIX, kEarth, 1.6f, 0.05f, OVER));

LOOK(kBerBreather, "breather",
	F(IM, FxForm_Flame, 0.9f, 3.0f, 72, 18, 0.3f, 0.5f, 0, FIX, kBerDark, 3.0f, 0.05f, DARK),
	G(IM, Glyph_Heart, 0.6f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kBerRage, 3.0f),
	F(IM, FxForm_Bubbles, 1.0f, 6.0f, 48, 8, 0.18f, 0, 0, FIX, kBerEndur, 3.0f, 0.035f),
	F(IM, FxForm_Bloom, 1.6f, 0.3f, 40, 2, 0, 0, 0, FIX, kBerBlood, 3.0f, 0.04f, ADD | FxFlag_Reverse));

LOOK(kBerDecapitation, "decapitation",
	G(IM, Glyph_Scythe, 1.2f, 7.5f, 1, 1, GA_PopRoll, 0, FIX, kBerAxe, 0.8f),
	G(IM, Glyph_Skull, 0.8f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kBone, 1.8f, 0.2f, OVER),
	F(IM, FxForm_Burst, 2.6f, 6.0f, 72, 2, 0.4f, 18, 0, FIX, kBerBlood, 0.9f, 0.10f, ADD, 1, 0.15f),
	F(IM, FxForm_Rain, 1.0f, 7.0f, 40, 24, 0, 0.1f, 0, FIX, kBerBlood, 1.8f, 0.05f, ADD, 1, 0.2f));

LOOK(kBerAxeProc, "axe proc",
	G(IM, Glyph_Scythe, 0.6f, 4.5f, 4, 1, GA_PopRoll, 0, FIX, kBerAxe, 0.7f),
	F(IM, FxForm_Burst, 1.4f, 4.0f, 32, 1, 0, 8, 0, FIX, kBerBlood, 0.5f, 0.07f));

LOOK(kBerRageProc, "rage proc",
	F(IM, FxForm_Sparkle, 1.0f, 6.5f, 24, 12, 0.10f, 0, 0, FIX, kBerRage, 1.0f),
	F(IM, FxForm_Ring, 1.0f, 3.0f, 32, 0, 2.0f, 1.0f, 0.8f, FIX, kBerBlood, 1.0f, 0.04f, COMET));

LOOK(kAaAgony, "agony of absolution",
	G(IM, Glyph_Skull, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kBerDark, 2.4f, 0, DARK),
	F(IM, FxForm_Crystal, 1.1f, 5.5f, 60, 12, 0, 0.8f, 0, FIX, kBerBlood, 2.4f, 0.06f),
	F(IM, FxForm_Vortex, 1.3f, 6.0f, 96, 5, 1.2f, 2.5f, -4.0f, FIX, kBerBlood, 2.2f, 0.06f));

LOOK(kAaBattleLeap, "battle leap",
	F(RE, FxForm_Wind, 1.5f, 4.0f, 60, 12, 3.0f, 0, 0, FIX, kBerRage, 1.2f),
	F(RE, FxForm_Burst, 3.0f, 0.2f, 72, 3, 0.3f, 16, 0, FIX, kBerBlood, 1.0f, 0.10f, ADD, 1, 0.3f),
	F(RE, FxForm_Cracks, 2.4f, 0, 48, 8, 1.2f, 0, 0, FIX, kEarth, 2.2f, 0.07f, OVER, 1, 0.3f),
	G(RE, Glyph_Horn, 0.7f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kBerWar, 1.8f, 0.4f));

LOOK(kAaBindingAxe, "binding axe",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.5f, 0.3f, 1.0f, FIX, kBerAxe, 0.7f, 0.16f),
	G(IM, Glyph_Scythe, 0.9f, 4.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kBerAxe, 1.2f),
	G(IM, Glyph_Shackle, 0.6f, 0.8f, 3, 2, GlyphAnim_Pop, 0.45f, FIX, kBerJugg, 3.0f, 0.3f),
	F(IM, FxForm_Ring, 1.1f, 0.6f, 40, 0, 0.3f, 1.0f, 0, FIX, kBerBlood, 3.0f, 0.04f));

LOOK(kAaBlindingFury, "blinding fury",
	F(IM, FxForm_Spire, 0.6f, 7.0f, 96, 16, 2.0f, 3.5f, 0, FIX, kBerFury, 1.0f, 0.07f),
	G(IM, Glyph_Eye, 0.9f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kBerFury, 1.6f),
	F(IM, FxForm_Bloom, 1.6f, 6.0f, 40, 3, 0, 0, 0, FIX, kBerDark, 2.0f, 0.06f, DARK, 1, 0.4f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kBerFury, 1.6f));

LOOK(kAaBloodPact, "blood pact",
	G(IM, Glyph_Drop, 0.9f, 8.0f, 4, 1, GA_Drop, 0, FIX, kBerBlood, 2.4f),
	F(IM, FxForm_Rune, 1.8f, 0, 70, 5, 2, 20, 0, FIX, kBerBlood, 3.0f, 0.05f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 72, 2.5f, 3.0f, 0, 1.0f, FIX, kBerRage, 2.4f, 0.05f, COMET));

LOOK(kAaBloodSustenance, "blood sustenance",
	G(IM, Glyph_Heart, 0.9f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kBerBlood, 2.6f),
	F(IM, FxForm_Flame, 0.8f, 8.0f, 72, 18, 0.1f, 2.4f, 0, FIX, kBerBlood, 2.2f, 0.07f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 2, 0, FIX, kHeal, 2.4f));

LOOK(kAaBloodfury, "bloodfury",
	F(IM, FxForm_Flame, 1.0f, 5.5f, 96, 24, 0.9f, 1.0f, 0, FIX, kBerBlood, 2.4f, 0.08f),
	G(IM, Glyph_Fist, 0.7f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kBerBlood, 2.0f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 48, 1, 1.0f, 12, 0, FIX, kBerRage, 0.8f, 0.08f));

LOOK(kAaBraxisHowl, "braxi's howl",
	G(RE, Glyph_Wolf, 1.0f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kBerHide, 2.0f),
	F(RE, FxForm_Bloom, 2.4f, 6.5f, 48, 4, 0, 0, 0, FIX, kBerRage, 1.6f, 0.06f),
	F(TR, FxForm_Bolt, 1, 0, 64, 4, 0.6f, 0.4f, 0.2f, FIX, kBerRage, 0.7f, 0.12f),
	G(IM, Glyph_Mouth, 0.7f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kBerRage, 1.6f));

LOOK(kAaCommunion, "communion of blood",
	G(IM, Glyph_Chalice, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kBerEndur, 2.4f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 72, 3, 1.2f, 0, 0.5f, FIX, kBerBlood, 2.4f, 0.05f, COMET),
	G(IM, Glyph_Drop, 0.3f, 4.5f, 0, 4, GA_Orbit, 1.2f, FIX, kBerBlood, 2.6f));

LOOK(kAaDesperation, "desperation",
	G(IM, Glyph_Skull, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kBerBlood, 2.8f),
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.8f, 1.3f, 0, FIX, kBerBlood, 3.0f, 0.09f),
	F(IM, FxForm_Bloom, 2.0f, 0.3f, 40, 3, 0, 0, 0, FIX, kBerDark, 2.4f, 0.07f, DARK),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kBerRage, 2.6f));

LOOK(kAaDevastating, "devastating assault",
	F(RE, FxForm_Vortex, 2.0f, 6.0f, 112, 6, 0.8f, 2.5f, 7.0f, FIX, kBerAxe, 3.0f, 0.07f),
	G(RE, Glyph_Scythe, 0.7f, 4.0f, 1, 6, GA_PopSpin | GlyphAnim_Roll, 2.0f, FIX, kBerAxe, 3.0f),
	F(RE, FxForm_Wind, 1.6f, 4.0f, 72, 12, 3.0f, 0, 0, FIX, kBerRage, 3.0f, 0.05f),
	F(IM, FxForm_Burst, 1.6f, 4.0f, 40, 1, 0.2f, 10, 0, FIX, kBerBlood, 0.6f, 0.08f));

LOOK(kAaDrawnToBlood, "drawn to blood",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 0.6f, 14, 0, FIX, kBerBlood, 1.4f, 0.08f),
	G(IM, Glyph_Fang, 0.8f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kBerBlood, 1.6f),
	F(IM, FxForm_Ring, 1.2f, 0.4f, 40, 0, 1.5f, 1.0f, 0.5f, FIX, kBerRage, 1.8f, 0.05f, COMET));

LOOK(kAaFocusedRampage, "focused furious rampage",
	G(IM, Glyph_Claw, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kBerRage, 3.0f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 96, 24, 0.8f, 0.9f, 0, FIX, kBerRage, 3.0f, 0.08f),
	F(IM, FxForm_Vortex, 1.1f, 6.0f, 64, 4, 1.2f, 2.0f, -4.0f, FIX, kBerFury, 3.0f, 0.05f),
	G(IM, Glyph_Eye, 0.45f, 9.4f, 4, 1, GA_PopAscend, 0, FIX, kBerFury, 2.0f, 0.3f));

LOOK(kAaFuriousLeap, "furious leap",
	F(RE, FxForm_Wind, 1.5f, 4.5f, 72, 12, 3.5f, 0, 0, FIX, kBerFury, 1.4f),
	G(RE, Glyph_ArrowUp, 0.8f, 6.0f, 4, 1, GA_PopAscend, 0, FIX, kBerFury, 1.4f),
	F(RE, FxForm_Bloom, 2.4f, 0.2f, 48, 3, 0, 0, 0, FIX, kBerRage, 1.4f, 0.07f, ADD, 1, 0.3f));

LOOK(kAaFuriousRampage, "furious rampage",
	G(IM, Glyph_Claw, 0.6f, 4.5f, 3, 4, GA_Orbit, 1.5f, FIX, kBerRage, 3.0f),
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.8f, 1.2f, 0, FIX, kFlame2, 3.0f, 0.09f),
	F(IM, FxForm_Burst, 1.0f, 0.3f, 96, 3, 0.4f, 20, 0, FIX, kBerBlood, 1.2f, 0.10f, ADD, 1, 0, AE));

LOOK(kAaJuggernautSurge, "juggernaut surge",
	G(IM, Glyph_Helm, 1.0f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kBerJugg, 3.0f),
	F(IM, FxForm_Flame, 1.1f, 6.0f, 112, 28, 0.8f, 1.1f, 0, FIX, kBerJugg, 3.0f, 0.09f),
	F(IM, FxForm_Cracks, 2.4f, 0, 48, 8, 1.2f, 0, 0, FIX, kBerRage, 2.6f, 0.07f),
	F(IM, FxForm_Burst, 2.8f, 0.3f, 72, 2, 0.4f, 16, 0, FIX, kBerJugg, 1.0f, 0.12f));

LOOK(kAaJuggernautResolve, "juggernaut's resolve",
	G(IM, Glyph_Tower, 1.0f, 8.2f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kBerJugg, 3.0f),
	F(IM, FxForm_Lattice, 1.4f, 6.5f, 80, 8, 0.4f, 0, 0, FIX, kBerJugg, 3.0f, 0.06f),
	F(IM, FxForm_Crystal, 1.8f, 3.0f, 60, 12, 1, 0.8f, 0, FIX, kStone, 3.0f, 0.08f, OVER));

LOOK(kAaRampage, "rampage",
	G(RE, Glyph_Sword, 0.55f, 4.0f, 3, 5, GA_Orbit | GlyphAnim_Roll, 1.8f, FIX, kBerAxe, 1.8f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 0.4f, 20, 0, FIX, kBerRage, 1.2f, 0.10f, ADD, 1, 0, AE),
	F(IM, FxForm_Sparkle, 1.0f, 5.0f, 24, 12, 0.10f, 1, 0, FIX, kBerBlood, 1.0f));

LOOK(kAaRecklessAbandon, "reckless abandon",
	F(IM, FxForm_Flame, 1.15f, 7.5f, 120, 30, 0.8f, 1.6f, 0, FIX, kBerRage, 3.0f, 0.10f),
	G(IM, Glyph_Skull, 0.4f, 5.0f, 3, 4, GA_Orbit, 1.4f, FIX, kBerBlood, 3.0f),
	F(IM, FxForm_Lightning, 1, 36.0f, 48, 2, 0.5f, 10.0f, 1.0f, FIX, kBerFury, 1.0f, 0.10f),
	F(IM, FxForm_Burst, 3.0f, 0.3f, 96, 3, 0.4f, 20, 0, FIX, kBerBlood, 1.1f, 0.12f));

LOOK(kAaSavageSpirit, "savage spirit",
	G(IM, Glyph_Wolf, 1.3f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kBerHide, 3.0f),
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.8f, 1.2f, 0, FIX, kBerRage, 3.0f, 0.09f),
	G(IM, Glyph_Paw, 0.35f, 0.3f, 2, 6, GA_PopSpin, 1.6f, FIX, kBerHide, 3.0f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kBerFury, 2.6f));

LOOK(kAaSelfPreservation, "self preservation",
	G(IM, Glyph_Shield, 1.0f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kBerJugg, 3.0f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 112, 8, 3, 0, 0, FIX, kBerBlood, 2.6f, 0.04f),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.10f, 0, 0, FIX, kHeal, 2.4f));

LOOK(kAaSilentStrikes, "silent strikes",
	G(IM, Glyph_Hand, 0.8f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kGrey, 2.2f, 0, OVER),
	F(IM, FxForm_Wind, 1.2f, 4.5f, 48, 8, 2.0f, 0, 0, FIX, kGhost, 2.2f, 0.04f),
	F(IM, FxForm_Bloom, 1.6f, 3.0f, 40, 2, 0, 0, 0, FIX, kGrey, 2.0f, 0.04f, ADD | FxFlag_Reverse));

LOOK(kAaSpireJuggernaut, "spire of the juggernaut",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kBerRage, 2.6f, 0.10f),
	G(RE, Glyph_Scythe, 1.2f, 9.0f, 1, 1, GA_Drop, 0, FIX, kBerJugg, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kBerBlood, 1.4f, 0.10f, ADD, 1, 0.35f, AE),
	F(RE, FxForm_Cracks, 2.4f, 0, 48, 8, 1.2f, 0, 0, FIX, kBerRage, 2.4f, 0.07f, ADD, 1, 0.35f));

LOOK(kAaTirelessSprint, "tireless sprint",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 12, 3.5f, 0, 0, FIX, kBerWar, 3.0f, 0.05f),
	G(IM, Glyph_Feather, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kBerEndur, 3.0f),
	F(IM, FxForm_Bloom, 1.6f, 0.2f, 40, 2, 0, 0, 0, FIX, kBerWar, 2.0f, 0.05f));

LOOK(kAaUncannyResilience, "uncanny resilience",
	G(IM, Glyph_Heart, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kBerRage, 2.6f),
	F(IM, FxForm_Crystal, 1.0f, 5.5f, 60, 12, 0, 0.7f, 0, FIX, kBerJugg, 2.6f, 0.06f),
	F(IM, FxForm_Ring, 1.2f, 0.3f, 40, 0, 0.8f, 1.0f, 0.4f, FIX, kBerBlood, 2.6f, 0.05f, COMET));

LOOK(kAaUntamedRage, "untamed rage",
	F(IM, FxForm_Flame, 1.2f, 8.0f, 128, 32, 0.8f, 1.8f, 0, FIX, kFlame2, 3.0f, 0.10f),
	G(IM, Glyph_Fang, 1.1f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kBerBlood, 3.0f),
	F(IM, FxForm_Burst, 3.2f, 0.3f, 96, 3, 0.5f, 24, 0, FIX, kBerRage, 1.2f, 0.12f),
	F(IM, FxForm_Cracks, 2.8f, 0, 64, 8, 1.4f, 0, 0, FIX, kBerDark, 2.8f, 0.07f, DARK));

LOOK(kAaVehementRage, "vehement rage",
	G(IM, Glyph_Fist, 1.0f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kBerFury, 2.6f),
	F(IM, FxForm_Spire, 0.8f, 7.0f, 96, 16, 0.8f, 2.8f, 0, FIX, kBerRage, 2.2f, 0.06f),
	F(IM, FxForm_Orbit, 1.3f, 6.0f, 60, 12, 0, 0, 0, FIX, kBerFury, 2.6f, 0.05f, ADD, 2.5f));

enum Line : int
{
	Tendon, Temple, Jarring, Frenzy, Overpowering, AxeGods, AxeAeons, Arc, Insult, Sapping,
	AxeThrow, BloodAxe, Volley, Vicious, SummonAxe, Scream, WarCry, CryHavoc, CryCarnage, Shared,
	Alliance, RageAura, Berserking, Cleaving, Flurry, Sundering, Capricious, Augmented, Festering,
	RageForm, Mangling, Disconcerting, Phantom, BlindRage, InspiredAnger, Reckless, FocusedFury,
	BattleFocus, BurningRage, Bloodthirst, Bloodletting, Retaliation, Conditioned, UnyieldingWill,
	FrenziedResolve, Indomitable, Wind, Rejuvenation, Sprint, Breather, Decapitation, AxeProc, RageProc,
	Agony, BattleLeap, BindingAxe, BlindingFury, BloodPact, BloodSustenance, Bloodfury, BraxisHowl,
	Communion, Desperation, Devastating, DrawnToBlood, FocusedRampage, FuriousLeap, FuriousRampage,
	JuggernautSurge, JuggernautResolve, Rampage, RecklessAbandon, SavageSpirit, SelfPreservation,
	SilentStrikes, SpireJuggernaut, TirelessSprint, UncannyResilience, UntamedRage, VehementRage,
};

const LineLook kLines[] = {
	{ "Tendon (snare, bleed)",          &kBerTendon },
	{ "Temple (stun strike)",           &kBerTemple },
	{ "Jarring (hate reduction)",       &kBerJarring },
	{ "Frenzy (strike)",                &kBerFrenzy },
	{ "Overpowering Frenzy (strike)",   &kBerOverpowering },
	{ "Axe of Rallos (strike, stun)",   &kBerAxeGods },
	{ "Axe of the Aeons (strike)",      &kBerAxeAeons },
	{ "Arcblade (multi-target)",        &kBerArc },
	{ "Slap in the Face (strike)",      &kBerInsult },
	{ "Sapping Strikes (drain)",        &kBerSapping },
	{ "Axe Throw (thrown)",             &kBerAxeThrow },
	{ "Blood Axe (thrown)",             &kBerBloodAxe },
	{ "Volley (thrown, area)",          &kBerVolley },
	{ "Vicious Spiral (area)",          &kBerVicious },
	{ "Summoned axes",                  &kBerSummonAxe },
	{ "Scream (hate)",                  &kBerScream },
	{ "War Cry (group)",                &kBerWarCry },
	{ "Cry Havoc (group)",              &kBerCryHavoc },
	{ "Cry Carnage (group)",            &kBerCryCarnage },
	{ "Shared Bloodlust (group)",       &kBerShared },
	{ "Alliance (debuff)",              &kBerAlliance },
	{ "Aura of Rage (aura)",            &kBerRageAura },
	{ "Berserking (discipline)",        &kBerBerserking },
	{ "Cleaving (discipline)",          &kBerCleaving },
	{ "Flurry (discipline)",            &kBerFlurry },
	{ "Sundering (discipline)",         &kBerSundering },
	{ "Capricious Rage (discipline)",   &kBerCapricious },
	{ "Augmented Frenzy (self)",        &kBerAugmented },
	{ "Festering Rage (proc)",          &kBerFestering },
	{ "Dichotomic Rage (form)",         &kBerRageForm },
	{ "Mangling (discipline)",          &kBerMangling },
	{ "Disconcerting (discipline)",     &kBerDisconcerting },
	{ "Phantom Assailant",              &kBerPhantomLook },
	{ "Blind Rage (discipline)",        &kBerBlindRage },
	{ "Inspired Anger (discipline)",    &kBerInspiredAnger },
	{ "Reckless (discipline)",          &kBerReckless },
	{ "Focused Fury (discipline)",      &kBerFocusedFury },
	{ "Battle Focus (discipline)",      &kBerBattleFocus },
	{ "Burning Rage (discipline)",      &kBerBurningRage },
	{ "Bloodthirst",                    &kBerBloodthirst },
	{ "Open Wound (bloodletting)",      &kBerBloodletting },
	{ "Retaliation (discipline)",       &kBerRetaliation },
	{ "Conditioned Retaliation",        &kBerConditioned },
	{ "Unyielding Will (discipline)",   &kBerUnyielding },
	{ "Frenzied Resolve (discipline)",  &kBerFrenziedResolve },
	{ "Indomitable (discipline)",       &kBerIndomitable },
	{ "Wind (endurance)",               &kBerWind },
	{ "Inner Rejuvenation",             &kBerRejuvenation },
	{ "Sprint (discipline)",            &kBerSprint },
	{ "Breather (endurance rest)",      &kBerBreather },
	{ "decapitation",                   &kBerDecapitation },
	{ "axe proc",                       &kBerAxeProc },
	{ "rage proc",                      &kBerRageProc },

	{ "AA: Agony of Absolution",        &kAaAgony },
	{ "AA: Battle Leap",                &kAaBattleLeap },
	{ "AA: Binding Axe",                &kAaBindingAxe },
	{ "AA: Blinding Fury",              &kAaBlindingFury },
	{ "AA: Blood Pact",                 &kAaBloodPact },
	{ "AA: Blood Sustenance",           &kAaBloodSustenance },
	{ "AA: Bloodfury",                  &kAaBloodfury },
	{ "AA: Braxi's Howl",               &kAaBraxisHowl },
	{ "AA: Communion of Blood",         &kAaCommunion },
	{ "AA: Desperation",                &kAaDesperation },
	{ "AA: Devastating Assault",        &kAaDevastating },
	{ "AA: Drawn to Blood",             &kAaDrawnToBlood },
	{ "AA: Focused Furious Rampage",    &kAaFocusedRampage },
	{ "AA: Furious Leap",               &kAaFuriousLeap },
	{ "AA: Furious Rampage",            &kAaFuriousRampage },
	{ "AA: Juggernaut Surge",           &kAaJuggernautSurge },
	{ "AA: Juggernaut's Resolve",       &kAaJuggernautResolve },
	{ "AA: Rampage",                    &kAaRampage },
	{ "AA: Reckless Abandon",           &kAaRecklessAbandon },
	{ "AA: Savage Spirit",              &kAaSavageSpirit },
	{ "AA: Self Preservation",          &kAaSelfPreservation },
	{ "AA: Silent Strikes",             &kAaSilentStrikes },
	{ "AA: Spire of the Juggernaut",    &kAaSpireJuggernaut },
	{ "AA: Tireless Sprint",            &kAaTirelessSprint },
	{ "AA: Uncanny Resilience",         &kAaUncannyResilience },
	{ "AA: Untamed Rage",               &kAaUntamedRage },
	{ "AA: Vehement Rage",              &kAaVehementRage },
};

const GroupLine kNumbered[] = {
	{ 0, Tendon },       { 1, Scream },       { 2, Temple },      { 3, Jarring },     { 4, BloodAxe },
	{ 5, Wind },         { 6, Volley },       { 7, Frenzy },      { 8, AxeGods },     { 9, AxeThrow },
	{ 10, Shared },      { 12, Cleaving },    { 13, Insult },     { 14, Flurry },     { 15, Rejuvenation },
	{ 16, Retaliation }, { 17, UnyieldingWill }, { 18, FrenziedResolve }, { 19, Sundering }, { 20, Overpowering },
	{ 21, Capricious },  { 22, Vicious },     { 23, CryCarnage }, { 24, Augmented },  { 25, Bloodletting },
	{ 26, Festering },   { 27, Phantom },     { 28, Conditioned }, { 29, Arc },       { 30, RageForm },
	{ 33, AxeAeons },    { 34, Sapping },     { 35, Disconcerting }, { 36, Alliance }, { 38, Mangling },
};

const GroupLine kModern[] = {
	{ 10, Tendon },      { 30, Temple },      { 40, Jarring },    { 60, Volley },     { 70, Frenzy },
	{ 90, AxeThrow },    { 100, Shared },     { 120, Insult },    { 140, Retaliation }, { 180, Overpowering },
	{ 200, Vicious },    { 220, Augmented },  { 230, Bloodletting }, { 240, Festering }, { 260, Conditioned },
	{ 270, Arc },        { 280, RageForm },   { 281, RageForm },  { 282, RageProc },  { 290, AxeAeons },
	{ 300, Sapping },    { 320, Alliance },
};

const GroupLine kOther[] = {
	{ 14001, Tendon },   { 14002, Scream },   { 14003, Temple },  { 14004, Jarring }, { 14005, Berserking },
	{ 14007, Volley },   { 16008, Wind },     { 35041, BlindRage }, { 38003, CryHavoc },
	{ 3311, Breather },  { 3411, Breather },  { 3511, Breather }, { 3611, Breather }, { 3737, Breather },
	{ 3837, Breather },  { 3937, Breather },  { 125010040, Breather }, { 126010040, Breather },
};

const NamedLine kAbilities[] = {
	{ "agony of absolution",        Agony },
	{ "battle leap",                BattleLeap },
	{ "binding axe",                BindingAxe },
	{ "blinding fury",              BlindingFury },
	{ "blood pact",                 BloodPact },
	{ "blood sustenance",           BloodSustenance },
	{ "bloodfury",                  Bloodfury },
	{ "braxi's howl",               BraxisHowl },
	{ "communion of blood",         Communion },
	{ "desperation",                Desperation },
	{ "devastating assault",        Devastating },
	{ "drawn to blood",             DrawnToBlood },
	{ "focused furious rampage",    FocusedRampage },
	{ "furious leap",               FuriousLeap },
	{ "furious rampage",            FuriousRampage },
	{ "juggernaut surge",           JuggernautSurge },
	{ "juggernaut's resolve",       JuggernautResolve },
	{ "rampage",                    Rampage },
	{ "reckless abandon",           RecklessAbandon },
	{ "savage spirit",              SavageSpirit },
	{ "self preservation",          SelfPreservation },
	{ "silent strikes",             SilentStrikes },
	{ "spire of the juggernaut",    SpireJuggernaut },
	{ "tireless sprint",            TirelessSprint },
	{ "uncanny resilience",         UncannyResilience },
	{ "untamed rage",               UntamedRage },
	{ "vehement rage",              VehementRage },

	{ "decapitation",               Decapitation },
	{ "binding axe strike",         AxeProc },
	{ "frenzied axe",               AxeProc },
	{ "frenzied volley",            AxeProc },
	{ "killing spree",              AxeProc },
	{ "screaming axes",             AxeProc },
	{ "battle leap warcry",         RageProc },
	{ "blood pact strike",          RageProc },
	{ "blinded by fury",            RageProc },
	{ "bloodshield",                RageProc },
	{ "braxi's panic",              RageProc },
	{ "braxi's terror",             RageProc },
	{ "decimator's intimidation",   RageProc },
	{ "fury's synergy",             RageProc },
	{ "juggernaut's weakening",     RageProc },
	{ "lingering wound",            RageProc },
	{ "screaming fury",             RageProc },
	{ "uncontained frenzy",         RageProc },
	{ "untamed rage recursion",     RageProc },
};

const NamedLine kOldDiscs[] = {
	{ "leg strike",                     Tendon },
	{ "leg cut",                        Tendon },
	{ "leg slice",                      Tendon },
	{ "crippling strike",               Tendon },
	{ "head strike",                    Temple },
	{ "head pummel",                    Temple },
	{ "head crush",                     Temple },
	{ "mind strike",                    Temple },
	{ "divertive strike",               Jarring },
	{ "distracting strike",             Jarring },
	{ "confusing strike",               Jarring },
	{ "baffling strike",                Jarring },
	{ "rage volley",                    Volley },
	{ "destroyer's volley",             Volley },
	{ "battle cry",                     WarCry },
	{ "war cry",                        WarCry },
	{ "battle cry of dravel",           WarCry },
	{ "war cry of dravel",              WarCry },
	{ "battle cry of the mastruq",      WarCry },
	{ "ancient: cry of chaos",          WarCry },
	{ "aura of rage",                   RageAura },
	{ "bloodlust aura",                 RageAura },
	{ "cleaving rage discipline",       Cleaving },
	{ "cleaving anger discipline",      Cleaving },
	{ "vengeful flurry discipline",     Flurry },
	{ "unpredictable rage discipline",  Capricious },
	{ "unflinching will discipline",    UnyieldingWill },
	{ "inspired anger discipline",      InspiredAnger },
	{ "reckless discipline",            Reckless },
	{ "focused fury discipline",        FocusedFury },
	{ "battle focus discipline",        BattleFocus },
	{ "burning rage discipline",        BurningRage },
	{ "sprint discipline",              Sprint },
	{ "indomitable discipline",         Indomitable },
	{ "bloodthirst",                    Bloodthirst },
	{ "corroded axe",                   SummonAxe },
	{ "blunt axe",                      SummonAxe },
	{ "steel axe",                      SummonAxe },
	{ "bearded axe",                    SummonAxe },
	{ "mithril axe",                    SummonAxe },
	{ "balanced war axe",               SummonAxe },
	{ "bonesplicer axe",                SummonAxe },
	{ "fleshtear axe",                  SummonAxe },
	{ "cold steel cleaving axe",        SummonAxe },
	{ "mithril bloodaxe",               SummonAxe },
	{ "rage axe",                       SummonAxe },
	{ "bloodseeker's axe",              SummonAxe },
	{ "battlerage axe",                 SummonAxe },
	{ "deathfury axe",                  SummonAxe },
	{ "tainted axe of hatred",          SummonAxe },
};

int BySpell(const eqlib::EQ_Spell*, std::string_view base)
{
	for (const NamedLine& n : kOldDiscs)
		if (base == n.name)
			return n.line;
	if (StartsWith(base, "axe of the "))
		return SummonAxe;
	return -1;
}

} // namespace

const ClassCatalogue& BerserkerCatalogue()
{
	static const ClassCatalogue c{ "Berserker", eqlib::Berserker, kLines, 3, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
