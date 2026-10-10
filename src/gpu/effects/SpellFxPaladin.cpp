#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

namespace eqgfx::spellfx {

namespace {
LOOK(kPalTouch, "touch",
	G(IM, Glyph_Hand, 1.1f, 8.5f, 4, 1, GA_Drop, 0, FIX, kHoly, 2.4f),
	F(IM, FxForm_Flame, 0.8f, 9.0f, 72, 18, 0.1f, 2.8f, 0, FIX, kHoly, 2.2f, 0.07f, ADD, 1, 0.45f),
	F(IM, FxForm_Burst, 2.4f, 0.5f, 72, 2, 2.0f, 16, 0, FIX, kGold, 1.2f, 0.08f, ADD, 1, 0.45f),
	F(IM, FxForm_Sparkle, 1.3f, 8.0f, 48, 24, 0.12f, 1, 0, FIX, kWhite, 2.4f, 0.06f, ADD, 1, 0.3f),
	F(IM, FxForm_Ring, 0.9f, 7.6f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kGold, 2.6f, 0.05f, COMET, 1, 0.5f));

LOOK(kPalLight, "light",
	F(IM, FxForm_Lightning, 1, 36.0f, 48, 1, 0.05f, 0.5f, 1.0f, FIX, kHoly, 1.2f, 0.35f),
	G(IM, Glyph_Lantern, 0.7f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.2f),
	F(IM, FxForm_Sparkle, 1.0f, 10.0f, 48, 24, 0.12f, 1, 0, FIX, kHoly, 2.2f),
	F(IM, FxForm_Bloom, 1.5f, 1.0f, 40, 2, 0, 0, 0, FIX, kGold, 1.8f, 0.05f));

LOOK(kPalBurst, "burst",
	G(RE, Glyph_Sun, 1.3f, 9.0f, 4, 1, GA_PopRoll, 0, FIX, kGold, 2.0f),
	F(RE, FxForm_Spire, 0.8f, 8.5f, 96, 16, 0.8f, 2.6f, 0, FIX, kGold, 1.8f, 0.06f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 2.5f, 20, 0, FIX, kHoly, 1.6f, 0.10f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Sparkle, 1.2f, 8.0f, 40, 20, 0.12f, 1, 0, FIX, kGold, 2.0f),
	F(IM, FxForm_Ring, 1.1f, 1.0f, 40, 0, 1.5f, 1.0f, 0.5f, FIX, kHoly, 1.6f, 0.05f, COMET));

LOOK(kPalWave, "wave",
	G(RE, Glyph_Wave, 0.9f, 1.2f, 3, 8, GA_PopSpin, 2.6f, FIX, kHealGold, 2.0f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 4, 0, 0, 0, FIX, kHeal, 2.2f, 0.07f, ADD, 1.4f, 0, AE),
	F(RE, FxForm_Burst, 1.0f, 0.3f, 96, 2, 1.2f, 0, 0, FIX, kHoly, 1.4f, 0.09f, ADD, 1, 0.1f, AE),
	F(IM, FxForm_Flame, 0.7f, 6.5f, 48, 12, 0.1f, 2.2f, 0, FIX, kHeal, 1.4f, 0.05f));

LOOK(kPalAurora, "aurora",
	F(RE, FxForm_Lattice, 0.5f, 9.0f, 120, 24, 0.3f, 0, 0, FIX, kAurora, 2.8f, 0.09f, ADD, 1, 0, AE),
	F(RE, FxForm_Sparkle, 0.5f, 10.0f, 80, 40, 0.14f, 1, 0, FIX, kAurora, 2.8f, 0.06f, ADD, 1, 0, AE),
	F(RE, FxForm_Ring, 0.5f, 9.0f, 64, 0, 0.4f, 1.0f, 0.2f, FIX, kAurora, 2.8f, 0.06f, COMET, 1, 0, AE),
	F(IM, FxForm_Ring, 1.0f, 5.0f, 40, 0, 1.4f, 1.0f, 0.5f, FIX, kAurora, 2.0f, 0.05f, COMET));

LOOK(kPalCleansing, "cleansing",
	F(IM, FxForm_Rain, 0.9f, 11.0f, 40, 30, 0, 0.15f, 0, FIX, kHoly, 2.4f, 0.05f),
	G(IM, Glyph_Drop, 0.35f, 5.5f, 0, 3, GA_Orbit, 1.2f, FIX, kTear, 3.0f),
	F(AU, FxForm_Helix, 1.0f, 6.5f, 72, 3, 1.5f, 0, 0.6f, FIX, kHeal, 0, 0.05f, COMET));

LOOK(kPalHealing, "healing",
	G(IM, Glyph_Cross, 0.3f, 4.5f, 0, 3, GA_Orbit, 1.3f, FIX, kHeal, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 72, 3, 1.2f, 0, 0.5f, FIX, kHealGold, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.1f, 6.5f, 32, 16, 0.10f, 0, 0, FIX, kHeal, 2.4f));

LOOK(kPalPenitence, "penitence",
	G(RE, Glyph_Drop, 0.3f, 9.0f, 4, 6, GA_Rain, 1.6f, FIX, kTear, 2.4f),
	F(RE, FxForm_Bloom, 0.6f, 1.0f, 96, 3, 0, 0, 0, FIX, kTear, 2.4f, 0.05f, ADD, 1, 0.3f, AE),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kTear, 2.0f));

LOOK(kPalSplash, "splash",
	G(IM, Glyph_Drop, 1.0f, 9.0f, 4, 1, GA_Drop, 0, FIX, kTear, 1.4f),
	F(IM, FxForm_Burst, 1.0f, 1.5f, 96, 2, 2.5f, 24, 0, FIX, kWater, 1.2f, 0.08f, ADD, 1, 0.45f, AE),
	F(IM, FxForm_Bubbles, 0.6f, 5.0f, 96, 14, 0.25f, 0, 0, FIX, kWater, 2.0f, 0.035f, ADD, 1, 0.45f, AE),
	F(IM, FxForm_Sparkle, 0.6f, 8.0f, 60, 30, 0.12f, 0, 0, FIX, kHoly, 2.0f, 0.06f, ADD, 1, 0.5f, AE));

LOOK(kPalProtective, "protective",
	G(IM, Glyph_Shield, 1.3f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kGold, 2.4f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 112, 8, 3, 0, 0, FIX, kHoly, 2.4f, 0.04f),
	F(IM, FxForm_Ring, 1.8f, 0.4f, 48, 0, 1.2f, 1.0f, 0.5f, FIX, kGold, 2.4f, 0.05f, COMET));

LOOK(kPalPreservation, "preservation",
	G(RE, Glyph_Hourglass, 0.9f, 8.4f, 4, 1, GA_PopRoll, 0, FIX, kAmber, 2.6f),
	F(RE, FxForm_Crystal, 1.1f, 5.5f, 60, 12, 0, 0.6f, 0, FIX, kAmber, 2.8f, 0.05f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 12, 0, FIX, kAmber, 1.4f, 0.09f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 2, 0, FIX, kAmber, 2.2f));

LOOK(kPalBlessing, "blessing",
	G(IM, Glyph_Chalice, 0.8f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.6f),
	F(IM, FxForm_Rain, 0.5f, 9.0f, 24, 20, 0, 0.1f, 0, FIX, kGold, 2.2f, 0.05f, ADD, 1, 0.3f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kHoly, 2.4f));

LOOK(kPalConsecrate, "consecrate",
	G(RE, Glyph_Cross, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kWhite, 2.4f),
	F(RE, FxForm_Rune, 0.9f, 0, 80, 8, 3, 16, 0, FIX, kWhite, 2.4f, 0.05f, ADD, 1, 0, AE),
	F(RE, FxForm_Flame, 0.9f, 1.4f, 128, 32, 0.1f, 0.6f, 0, FIX, kHolyFire, 2.2f, 0.06f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Bubbles, 0.9f, 6.0f, 48, 6, 0.18f, 0, 0, FIX, kWater, 2.0f, 0.035f));

LOOK(kPalPurity, "purity",
	G(IM, Glyph_Drop, 0.6f, 8.5f, 4, 1, GA_Drop, 0, FIX, kWhite, 2.0f),
	F(IM, FxForm_Bloom, 1.4f, 1.0f, 40, 3, 0, 0, 0, FIX, kWater, 2.0f, 0.05f, ADD, 1, 0.35f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kWhite, 2.0f, 0.06f, ADD, 1, 0.35f));

LOOK(kPalSymbol, "symbol",
	G(IM, Glyph_FleurDeLis, 1.6f, 0.0f, 2, 1, GlyphAnim_Pop, 0, FIX, kGold, 3.0f),
	G(IM, Glyph_FleurDeLis, 0.6f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kHoly, 2.4f, 0.3f),
	F(IM, FxForm_Rune, 2.2f, 0, 70, 8, 3, 16, 0, FIX, kGold, 3.0f, 0.04f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 64, 2.5f, 3.0f, 0, 1.0f, FIX, kHoly, 2.0f, 0.05f, COMET));

LOOK(kPalArmorFaith, "armor of faith",
	F(IM, FxForm_Crystal, 1.0f, 5.5f, 60, 12, 0, 0.7f, 0, FIX, kGold, 2.6f, 0.06f),
	G(IM, Glyph_Helm, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kGold, 2.6f),
	F(IM, FxForm_Dome, 1.6f, 6.4f, 88, 6, 2, 0, 0, FIX, kGold, 2.0f, 0.03f),
	F(IM, FxForm_Ring, 1.2f, 0.3f, 40, 0, 0.8f, 1.0f, 0.4f, FIX, kGold, 2.4f, 0.05f, COMET));

LOOK(kPalArmorProc, "armor of zeal",
	G(IM, Glyph_Sword, 0.55f, 3.5f, 3, 3, GA_Orbit, 1.4f, FIX, kSilver, 3.2f),
	F(IM, FxForm_Rune, 1.3f, 3.2f, 60, 6, 1, 12, 2, FIX, kGold, 3.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kGold, 2.4f));

LOOK(kPalBrell, "brell's bulwark",
	F(RE, FxForm_Crystal, 2.0f, 3.5f, 60, 12, 1, 0.9f, 0, FIX, kStone, 3.0f, 0.08f, OVER),
	G(RE, Glyph_Tower, 1.1f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.6f),
	F(RE, FxForm_Cracks, 2.6f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 2.5f, 0.06f, OVER),
	G(IM, Glyph_Rock, 0.35f, 1.0f, 3, 5, GlyphAnim_Rise | GlyphAnim_Spin | GlyphAnim_Bob, 1.4f, FIX, kStone, 2.5f, 0, OVER));

LOOK(kPalKeeper, "keeper",
	F(IM, FxForm_Wings, 1.0f, 4.3f, 58, 7, 0.4f, 3.0f, 0, FIX, kFeather, 3.0f, 0.06f),
	G(IM, Glyph_Shield, 0.6f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kGold, 2.2f, 0.2f),
	Persist(F(AU, FxForm_Ring, 0.55f, 7.8f, 32, 0, 1.0f, 1.0f, 0.5f, FIX, kGold, 0, 0.05f, COMET)));

LOOK(kPalHandKeeper, "hand of the keeper",
	G(RE, Glyph_Hand, 0.6f, 5.0f, 3, 6, GA_RingRise, 2.4f, FIX, kGold, 3.0f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.5f, 16, 0, FIX, kHoly, 1.4f, 0.09f, ADD, 1, 0, AE),
	F(IM, FxForm_Wings, 1.0f, 4.3f, 58, 6, 0.5f, 2.6f, 0, FIX, kFeather, 2.6f, 0.05f),
	Persist(F(AU, FxForm_Ring, 0.55f, 7.8f, 32, 0, 1.0f, 1.0f, 0.5f, FIX, kGold, 0, 0.05f, COMET)));

LOOK(kPalGird, "gird",
	G(IM, Glyph_Shield, 0.5f, 3.5f, 3, 4, GA_Orbit, 1.3f, FIX, kGold, 3.0f),
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kWhite, 2.2f, 0.03f));

LOOK(kPalMantle, "mantle",
	Persist(F(AU, FxForm_Helix, 1.05f, 6.0f, 96, 2, 1.0f, 0, 0.3f, FIX, kRoyal, 0, 0.06f, COMET)),
	F(IM, FxForm_Helix, 1.15f, 6.0f, 96, 2, -1.0f, 0, 0.3f, FIX, kGold, 3.0f, 0.05f, COMET),
	G(IM, Glyph_Crown, 0.55f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f));

LOOK(kPalAffirmation, "affirmation",
	F(IM, FxForm_Ring, 0.55f, 7.9f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kGold, 3.0f, 0.06f, COMET),
	G(IM, Glyph_Cross, 0.25f, 5.0f, 0, 4, GA_PopAscend | GlyphAnim_Spin, 1.1f, FIX, kHoly, 2.4f),
	F(IM, FxForm_Sparkle, 1.0f, 8.0f, 24, 12, 0.12f, 0, 0, FIX, kGold, 2.4f));

LOOK(kPalAureate, "aureate",
	G(IM, Glyph_Crown, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.8f),
	F(IM, FxForm_Orbit, 1.3f, 6.5f, 60, 12, 0, 0, 0, FIX, kGold, 2.8f, 0.05f, ADD, 2.5f),
	F(IM, FxForm_Burst, 2.0f, 1.5f, 48, 1, 1.5f, 12, 0, FIX, kGold, 0.9f, 0.08f));

LOOK(kPalFury, "fury",
	F(IM, FxForm_Flame, 1.0f, 5.8f, 96, 24, 0.9f, 0.9f, 0, FIX, kHolyFire, 3.0f, 0.08f),
	G(IM, Glyph_Sword, 0.8f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kGold, 2.4f),
	F(IM, FxForm_Burst, 2.0f, 1.5f, 48, 1, 1.5f, 12, 0, FIX, kGold, 0.8f, 0.08f));

LOOK(kPalRighteous, "righteous",
	F(IM, FxForm_Flame, 1.0f, 5.8f, 96, 24, 0.9f, 0.9f, 0, FIX, kZeal, 3.0f, 0.08f),
	G(IM, Glyph_Fist, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kZeal, 2.4f),
	F(IM, FxForm_Burst, 2.2f, 0.3f, 64, 2, 0.3f, 14, 0, FIX, kZeal, 0.9f, 0.09f));

LOOK(kPalPenitent, "penitent",
	G(IM, Glyph_Scales, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kHealGold, 3.0f),
	F(IM, FxForm_Orbit, 1.3f, 6.0f, 48, 10, 0, 0, 0, FIX, kHeal, 3.0f, 0.04f, ADD, 2.0f));

LOOK(kPalReflexive, "reflexive",
	G(IM, Glyph_Shield, 0.9f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kSilver, 2.0f),
	F(IM, FxForm_Burst, 2.0f, 3.0f, 64, 2, 0, 12, 0, FIX, kWhite, 1.0f, 0.07f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Sparkle, 1.3f, 6.0f, 32, 16, 0.12f, 2, 0, FIX, kSilver, 1.8f));

LOOK(kPalStance, "stance",
	F(IM, FxForm_Cracks, 2.2f, 0, 48, 6, 1.2f, 0, 0, FIX, kEarth, 2.5f, 0.07f, OVER),
	G(IM, Glyph_Shield, 0.9f, 7.0f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kIron, 2.6f),
	F(IM, FxForm_Lattice, 1.3f, 6.0f, 64, 8, 0.4f, 0, 0, FIX, kIron, 2.4f, 0.05f),
	F(IM, FxForm_Burst, 2.5f, 0.2f, 64, 2, 0.2f, 0, 0, FIX, kIron, 1.0f, 0.10f, OVER));

LOOK(kPalSteel, "steel",
	G(IM, Glyph_Sword, 0.55f, 3.0f, 3, 4, GA_Orbit, 1.2f, FIX, kIron, 2.6f),
	F(IM, FxForm_Wind, 1.2f, 4.0f, 60, 10, 2.0f, 0, 0, FIX, kIron, 2.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.3f, 6.0f, 32, 16, 0.10f, 0, 0, FIX, kWhite, 2.0f));

LOOK(kPalGuardian, "guardian",
	Persist(F(AU, FxForm_Wings, 1.0f, 4.4f, 70, 10, 0.3f, 4.0f, 0, FIX, kFeather, 1.2f, 0.07f, HOLD)),
	F(IM, FxForm_Dome, 1.9f, 7.2f, 112, 8, 3, 0, 0, FIX, kGold, 3.5f, 0.04f),
	G(IM, Glyph_Shield, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.5f));

LOOK(kPalValiant, "valiant",
	G(IM, Glyph_Banner, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Lattice, 1.3f, 6.0f, 64, 6, 0.5f, 0, 0, FIX, kGold, 2.6f, 0.05f),
	F(IM, FxForm_Burst, 2.0f, 1.0f, 48, 1, 1.0f, 12, 0, FIX, kGold, 0.9f, 0.08f));

LOOK(kPalPureforge, "pureforge",
	G(IM, Glyph_Anvil, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kForge, 2.8f),
	F(IM, FxForm_Flame, 1.05f, 5.5f, 96, 24, 0.8f, 0.8f, 0, FIX, kForge, 3.0f, 0.08f),
	F(IM, FxForm_Sparkle, 1.5f, 6.0f, 48, 24, 0.10f, 0, 0, FIX, kForge, 3.0f));

LOOK(kPalBlunted, "blunted blade",
	G(IM, Glyph_Sword, 0.9f, 7.5f, 4, 1, GA_PopAscend, 0, FIX, kGrey, 2.4f, 0, OVER),
	F(IM, FxForm_Dome, 1.5f, 6.0f, 72, 6, 2, 0, 0, FIX, kGrey, 2.0f, 0.03f));

LOOK(kPalBreather, "breather",
	F(IM, FxForm_Bubbles, 1.0f, 6.5f, 48, 8, 0.18f, 0, 0, FIX, kAir, 3.0f, 0.035f),
	G(IM, Glyph_Feather, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kFeather, 3.0f),
	F(IM, FxForm_Bloom, 1.6f, 1.0f, 40, 2, 0, 0, 0, FIX, kAir, 3.0f, 0.04f));

LOOK(kPalForce, "force",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.3f, 0.25f, 0.8f, FIX, kHoly, 0.6f, 0.16f),
	F(IM, FxForm_Rune, 1.4f, 3.2f, 70, 6, 1, 12, 1, FIX, kHoly, 1.2f, 0.06f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0.3f, 12, 0, FIX, kWhite, 0.6f, 0.08f),
	Stars(0.2f));

LOOK(kPalForceSea, "force of the sea",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.3f, 1.0f, FIX, kSea, 0.8f, 0.16f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 72, 4, 1.4f, 2.0f, 4.0f, FIX, kSea, 1.6f, 0.06f),
	F(IM, FxForm_Bubbles, 1.1f, 6.0f, 64, 10, 0.2f, 0, 0, FIX, kWater, 2.0f, 0.035f),
	Stars(0.25f));

LOOK(kPalForceAvowed, "force of the oath",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.35f, 1.0f, FIX, kGold, 0.8f, 0.18f),
	G(IM, Glyph_FleurDeLis, 1.0f, 6.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kGold, 1.4f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 64, 2, 0.4f, 12, 0, FIX, kHoly, 0.8f, 0.09f),
	Stars(0.2f));

LOOK(kPalForge, "force of the forge",
	G(IM, Glyph_Anvil, 1.1f, 8.0f, 4, 1, GA_Drop, 0, FIX, kForge, 1.8f),
	F(IM, FxForm_Sparkle, 1.5f, 4.0f, 60, 30, 0.12f, 0, 0, FIX, kForge, 1.6f, 0.06f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.4f, 1.0f, 72, 2, 0.8f, 16, 0, FIX, kForge, 0.9f, 0.10f, ADD, 1, 0.35f),
	Stars(0.4f));

LOOK(kPalEclipse, "eclipse",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.4f, 1.2f, FIX, kRoyal, 0.8f, 0.16f),
	G(IM, Glyph_Sun, 0.8f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kGold, 2.4f),
	G(IM, Glyph_Moon, 0.8f, 8.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kCold, 2.4f, 0.3f),
	F(IM, FxForm_Ring, 1.4f, 4.0f, 48, 0, 1.2f, 1.0f, 0.5f, FIX, kRoyal, 2.2f, 0.06f, COMET),
	Stars(0.3f));

LOOK(kPalLesson, "lesson",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.35f, 1.2f, FIX, kHoly, 0.8f, 0.15f),
	G(IM, Glyph_Book, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.2f),
	F(IM, FxForm_Rune, 1.4f, 3.5f, 70, 5, 2, 20, 2, FIX, kGold, 1.6f, 0.05f),
	Stars(0.2f));

LOOK(kPalCrushSea, "crush of the sea",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.3f, 1.5f, FIX, kSea, 0.8f, 0.16f),
	G(IM, Glyph_Wave, 1.4f, 8.0f, 4, 1, GA_Drop, 0, FIX, kSea, 1.6f),
	F(IM, FxForm_Burst, 2.6f, 2.0f, 96, 3, 3.0f, 16, 0, FIX, kSea, 1.0f, 0.10f, ADD, 1, 0.35f),
	F(IM, FxForm_Bubbles, 1.3f, 5.0f, 64, 10, 0.25f, 0, 0, FIX, kWater, 1.8f, 0.035f, ADD, 1, 0.35f));

LOOK(kPalCrush, "crush",
	G(IM, Glyph_Hammer, 1.3f, 8.5f, 4, 1, GA_Drop, 0, FIX, kGold, 1.6f),
	F(IM, FxForm_Burst, 2.8f, 0.4f, 96, 3, 0.6f, 20, 0, FIX, kHoly, 1.0f, 0.12f, ADD, 1, 0.35f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kGold, 1.6f, 0.06f, ADD, 1, 0.35f),
	F(RE, FxForm_Sparkle, 1.4f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kHeal, 1.8f, 0.06f, ADD, 1, 0.5f));

LOOK(kPalBrilliant, "brilliant",
	F(IM, FxForm_Lightning, 1, 38.0f, 64, 3, 0.4f, 10.0f, 1.0f, FIX, kHoly, 0.8f, 0.12f),
	G(IM, Glyph_Star, 1.0f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kStar, 1.6f),
	F(IM, FxForm_Spire, 0.8f, 3.5f, 96, 16, 2.0f, 3.5f, 0, FIX, kGold, 1.0f, 0.07f));

LOOK(kPalHymn, "hymn",
	G(RE, Glyph_Note, 0.3f, 6.0f, 0, 3, GA_Orbit | GlyphAnim_Ascend, 1.3f, FIX, kGold, 1.6f),
	G(IM, Glyph_Note, 0.4f, 9.0f, 4, 5, GA_Rain, 1.4f, FIX, kGold, 2.2f),
	F(IM, FxForm_Flame, 0.7f, 11.0f, 80, 20, 0.1f, 3.2f, 0, FIX, kHoly, 1.4f, 0.09f, ADD, 1, 0.45f),
	F(IM, FxForm_Burst, 2.2f, 0.4f, 64, 2, 1.0f, 12, 0, FIX, kGold, 1.0f, 0.09f, ADD, 1, 0.45f));

LOOK(kPalDoctrine, "doctrine",
	G(IM, Glyph_Book, 1.3f, 9.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.2f),
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 2, 0.5f, 12.0f, 1.0f, FIX, kHoly, 0.8f, 0.12f, ADD, 1, 0.3f),
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 2, 0.6f, 12.0f, 1.0f, FIX, kHoly, 0.7f, 0.10f, ADD, 1, 0.6f),
	F(IM, FxForm_Flame, 0.8f, 9.0f, 80, 20, 0.1f, 3.0f, 0, FIX, kHoly, 1.4f, 0.08f, ADD, 1, 0.6f),
	F(IM, FxForm_Burst, 2.6f, 0.4f, 96, 3, 1.0f, 20, 0, FIX, kGold, 1.1f, 0.10f, ADD, 1, 0.6f));

LOOK(kPalChastise, "chastise",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.4f, 0.3f, 1.2f, FIX, kHoly, 0.8f, 0.16f),
	G(IM, Glyph_Scales, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.2f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 72, 2, 1.0f, 12, 0, FIX, kHoly, 0.9f, 0.09f),
	F(IM, FxForm_Drain, 1, 0, 48, 3, 1.2f, 12, 0, FIX, kHeal, 1.8f, 0.08f, ADD, 1, 0.3f));

LOOK(kPalUndead, "bane of the undead",
	G(IM, Glyph_Skull, 0.8f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kBone, 1.6f, 0, OVER),
	F(IM, FxForm_Flame, 0.8f, 11.0f, 80, 20, 0.1f, 3.2f, 0, FIX, kHoly, 1.4f, 0.09f, ADD, 1, 0.3f),
	F(IM, FxForm_Burst, 2.4f, 1.0f, 72, 2, 1.0f, 16, 0, FIX, kHoly, 1.0f, 0.09f, ADD, 1, 0.3f));

LOOK(kPalHonor, "challenge for honor",
	G(RE, Glyph_Horn, 0.8f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 1.6f),
	F(RE, FxForm_Bloom, 2.0f, 5.0f, 48, 3, 0, 0, 0, FIX, kGold, 1.6f, 0.05f),
	G(IM, Glyph_Banner, 0.9f, 8.0f, 4, 1, GA_Drop, 0, FIX, kZeal, 2.4f),
	F(IM, FxForm_Burst, 2.2f, 0.4f, 64, 2, 0.6f, 10, 0, FIX, kBlood, 0.9f, 0.09f, ADD, 1, 0.35f));

LOOK(kPalAudacity, "audacity",
	F(RE, FxForm_Bloom, 2.2f, 4.0f, 48, 3, 0, 0, 0, FIX, kZeal, 1.4f, 0.06f),
	G(IM, Glyph_Fist, 1.0f, 8.0f, 4, 1, GA_Drop, 0, FIX, kBlood, 1.8f),
	F(IM, FxForm_Burst, 2.6f, 0.3f, 72, 2, 0.4f, 16, 0, FIX, kBlood, 1.0f, 0.10f, ADD, 1, 0.35f));

LOOK(kPalWords, "words",
	G(IM, Glyph_Dove, 0.7f, 6.0f, 4, 2, GA_PopAscend | GlyphAnim_Spin, 0.8f, FIX, kWhite, 2.6f),
	F(IM, FxForm_Bloom, 1.8f, 2.0f, 40, 2, 0, 0, 0, FIX, kFeather, 2.4f, 0.04f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 24, 12, 0.14f, 0, 0, FIX, kFeather, 2.4f));

LOOK(kPalMarkHero, "mark of the hero",
	G(IM, Glyph_Banner, 0.8f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kGhost, 4.0f),
	F(IM, FxForm_Ring, 1.3f, 0.3f, 40, 0, 0.6f, 1.0f, 0.3f, FIX, kGold, 4.0f, 0.05f, COMET),
	F(IM, FxForm_Orbit, 1.4f, 6.0f, 48, 10, 0, 0, 0, FIX, kHeal, 4.0f, 0.04f, ADD, 2.0f));

LOOK(kPalMarkBrand, "brand",
	G(IM, Glyph_Sword, 1.1f, 9.0f, 4, 1, GA_Drop, 0, FIX, kSilver, 3.0f),
	F(IM, FxForm_Rune, 1.8f, 0, 60, 6, 1, 12, 0, FIX, kGold, 4.0f, 0.05f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 1.8f, 0.4f, 48, 1, 0.5f, 8, 0, FIX, kGold, 0.8f, 0.08f, ADD, 1, 0.35f));

LOOK(kPalShackle, "shackle",
	G(IM, Glyph_Shackle, 0.6f, 0.8f, 3, 2, GlyphAnim_Pop, 0.45f, FIX, kGold, 4.0f),
	F(AU, FxForm_Vines, 1.0f, 2.0f, 120, 6, 0.4f, 0, 1.0f, FIX, kGoldVine, 5.0f, 0.07f),
	F(IM, FxForm_Rune, 1.8f, 0, 60, 6, 1, 12, 0, FIX, kGold, 3.0f));

LOOK(kPalFallen, "penitence for the fallen",
	G(IM, Glyph_Ankh, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kTear, 2.6f),
	F(IM, FxForm_Sparkle, 1.2f, 8.0f, 32, 16, 0.12f, 1, 0, FIX, kTear, 2.4f));

LOOK(kPalProcHeal, "healing proc",
	G(IM, Glyph_Cross, 0.35f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 1.4f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 20, 10, 0.10f, 0, 0, FIX, kHeal, 1.2f));

LOOK(kPalProcStrike, "holy strike",
	G(IM, Glyph_Sword, 0.7f, 4.0f, 4, 1, GA_PopRoll, 0, FIX, kGold, 0.8f),
	F(IM, FxForm_Burst, 1.4f, 3.5f, 32, 1, 0, 8, 0, FIX, kGold, 0.5f, 0.07f));

LOOK(kPalProcGlint, "triggered effect",
	F(IM, FxForm_Sparkle, 1.0f, 6.5f, 24, 12, 0.10f, 0, 0, FIX, kGold, 1.0f),
	F(IM, FxForm_Ring, 1.0f, 3.0f, 32, 0, 2.0f, 1.0f, 0.8f, FIX, kGold, 1.0f, 0.04f, COMET));

LOOK(kAaLayOnHands, "lay on hands",
	G(IM, Glyph_Hand, 1.8f, 11.0f, 4, 1, GA_Drop, 0, FIX, kHoly, 3.0f),
	F(IM, FxForm_Flame, 1.0f, 14.0f, 120, 30, 0.1f, 4.0f, 0, FIX, kHoly, 3.0f, 0.10f, ADD, 1, 0.45f),
	F(IM, FxForm_Burst, 3.5f, 0.5f, 96, 3, 3.0f, 24, 0, FIX, kGold, 1.6f, 0.12f, ADD, 1, 0.45f),
	F(IM, FxForm_Rune, 3.0f, 0, 80, 8, 3, 24, 0, FIX, kGold, 3.0f, 0.05f, ADD, 1, 0.45f),
	F(IM, FxForm_Sparkle, 1.6f, 12.0f, 80, 40, 0.14f, 1, 0, FIX, kWhite, 3.0f, 0.06f, ADD, 1, 0.4f),
	F(IM, FxForm_Wings, 1.0f, 4.4f, 70, 10, 0.3f, 4.2f, 0, FIX, kFeather, 3.0f, 0.06f, ADD, 1, 0.5f));

LOOK(kAaJudgment, "inquisitor's judgment",
	G(IM, Glyph_Scales, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kGold, 2.0f),
	F(IM, FxForm_Lightning, 1, 38.0f, 64, 2, 0.5f, 12.0f, 1.0f, FIX, kHoly, 0.8f, 0.12f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.4f, 0.5f, 72, 2, 1.0f, 16, 0, FIX, kGold, 1.0f, 0.10f, ADD, 1, 0.35f));

LOOK(kAaBeacon, "beacon of the righteous",
	G(RE, Glyph_Lantern, 1.3f, 9.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.4f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 1.0f, 24, 0, FIX, kGold, 1.4f, 0.12f, ADD, 1, 0.2f, AE),
	F(RE, FxForm_Bloom, 1.0f, 2.0f, 96, 3, 0, 0, 0, FIX, kHoly, 2.0f, 0.06f, ADD, 1, 0.2f, AE),
	G(RE, Glyph_Star, 0.3f, 7.0f, 0, 5, GA_PopSpin, 1.2f, FIX, kStar, 2.0f, 0.3f));

LOOK(kAaLodestar, "hallowed lodestar",
	G(RE, Glyph_Star, 1.4f, 10.0f, 4, 1, GA_PopRoll, 0, FIX, kStar, 2.4f),
	F(RE, FxForm_Spire, 0.1f, 1.0f, 120, 24, 0.5f, 8.0f, 0, FIX, kGold, 1.6f, 0.08f, ADD, 1, 0.2f, AE),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 0.6f, 0, 0, FIX, kStar, 1.2f, 0.10f, ADD, 1, 0.2f, AE));

LOOK(kAaPurification, "purification",
	G(IM, Glyph_Dove, 0.8f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kWhite, 2.4f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 64, 2, 2.0f, 12, 0, FIX, kWhite, 1.0f, 0.06f),
	F(IM, FxForm_Sparkle, 1.3f, 8.0f, 40, 20, 0.12f, 0, 0, FIX, kWhite, 2.0f));

LOOK(kAaChampionsOath, "champion's oath",
	G(IM, Glyph_Banner, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	G(IM, Glyph_Sword, 0.5f, 3.5f, 3, 3, GA_Orbit, 1.4f, FIX, kSilver, 3.0f),
	F(IM, FxForm_Rune, 1.8f, 0, 60, 6, 1, 12, 0, FIX, kGold, 3.0f));

LOOK(kAaCloakOfLight, "cloak of light",
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kLight, 2.5f, 0.05f),
	G(IM, Glyph_Sun, 0.5f, 7.5f, 4, 1, GA_PopAscend, 0, FIX, kLight, 2.2f),
	F(IM, FxForm_Dome, 1.8f, 6.5f, 96, 6, 2, 0, 0, FIX, kLight, 2.0f, 0.03f));

LOOK(kAaPersecution, "disruptive persecution",
	F(IM, FxForm_Lightning, 1, 0, 48, 2, 0.8f, 16.0f, 0, FIX, kHoly, 0.6f, 0.10f),
	G(IM, Glyph_Hand, 0.9f, 6.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kGold, 1.4f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0.3f, 12, 0, FIX, kGold, 0.7f, 0.08f));

LOOK(kAaDivineCall, "divine call",
	G(RE, Glyph_Horn, 0.8f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 1.8f),
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.0f, 16, 0, FIX, kGold, 1.6f, 0.08f),
	G(IM, Glyph_Hand, 0.8f, 7.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kGold, 1.6f));

LOOK(kAaProtectorsUnity, "divine protector's unity",
	G(IM, Glyph_Shield, 0.55f, 4.0f, 3, 4, GA_Orbit, 1.4f, FIX, kGold, 3.0f),
	G(IM, Glyph_Crown, 0.6f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kHoly, 2.6f, 0.04f));

LOOK(kAaSteed, "holy steed",
	G(RE, Glyph_Steed, 1.2f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.6f),
	F(RE, FxForm_Rune, 2.8f, 0, 60, 6, 1, 8, 0, FIX, kGold, 2.6f),
	F(RE, FxForm_Wind, 1.5f, 3.0f, 60, 10, 2.5f, 0, 0, FIX, kHoly, 1.8f),
	F(RE, FxForm_Burst, 3.5f, 0, 64, 2, 0.5f, 0, 0, FIX, kGold, 1.2f, 0.10f, ADD, 1, 0.3f));

LOOK(kAaDivineStun, "divine stun",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.3f, 0.25f, 0.6f, FIX, kGold, 0.6f, 0.18f),
	G(IM, Glyph_Hand, 0.9f, 6.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kGold, 1.2f),
	F(IM, FxForm_Burst, 2.2f, 2.5f, 64, 2, 0.4f, 12, 0, FIX, kHoly, 0.8f, 0.09f),
	Stars(0.2f));

LOOK(kAaGiftOfLife, "gift of life",
	G(RE, Glyph_Heart, 1.2f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kHealGold, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 2.5f, 24, 0, FIX, kHeal, 1.8f, 0.12f, ADD, 1, 0.2f, AE),
	F(RE, FxForm_Rune, 2.8f, 0, 80, 8, 3, 16, 0, FIX, kHealGold, 2.4f),
	F(IM, FxForm_Sparkle, 1.2f, 8.0f, 48, 24, 0.12f, 0, 0, FIX, kHeal, 2.2f));

LOOK(kAaGiftOfRes, "gift of resurrection",
	G(IM, Glyph_Ankh, 1.4f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 4.0f, 0.3f),
	F(IM, FxForm_Flame, 0.9f, 10.0f, 96, 24, 0.15f, 3.0f, 0, FIX, kDivine, 4.0f, 0.08f),
	F(IM, FxForm_Wings, 1.0f, 4.4f, 70, 10, 0.3f, 4.2f, 0, FIX, kFeather, 4.0f, 0.06f));

LOOK(kAaHaltTheDead, "halt the dead",
	G(IM, Glyph_Skull, 0.7f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kBone, 4.0f, 0, OVER),
	G(IM, Glyph_Link, 0.35f, 0.6f, 3, 6, GA_PopSpin, 1.1f, FIX, kGold, 4.0f),
	F(IM, FxForm_Ring, 1.1f, 0.6f, 40, 0, 0.3f, 1.0f, 0, FIX, kGold, 4.0f, 0.04f));

LOOK(kAaHandOfPiety, "hand of piety",
	G(RE, Glyph_Hand, 0.7f, 5.0f, 3, 6, GA_RingRise, 2.4f, FIX, kHealGold, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 2.0f, 16, 0, FIX, kHeal, 1.8f, 0.10f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Flame, 0.75f, 8.0f, 72, 18, 0.1f, 2.6f, 0, FIX, kHeal, 1.8f, 0.07f));

LOOK(kAaHandOfTunare, "hand of tunare",
	G(IM, Glyph_Tree, 1.2f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kNature, 3.0f),
	G(IM, Glyph_Leaf, 0.3f, 4.5f, 0, 4, GA_Orbit, 1.4f, FIX, kNature, 3.0f),
	F(IM, FxForm_Vines, 1.0f, 3.5f, 96, 5, 0.8f, 0, 0, FIX, kWood, 3.0f, 0.06f));

LOOK(kAaHeroicLeap, "heroic leap",
	F(RE, FxForm_Wind, 1.5f, 4.0f, 60, 12, 3.0f, 0, 0, FIX, kGold, 1.2f),
	F(RE, FxForm_Burst, 3.0f, 0.2f, 72, 2, 0.3f, 12, 0, FIX, kGold, 1.0f, 0.10f),
	F(RE, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 2.0f, 0.06f, OVER));

LOOK(kAaLeapOfFaith, "leap of faith",
	G(RE, Glyph_Feather, 0.6f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kFeather, 1.8f),
	F(RE, FxForm_Wind, 1.4f, 4.0f, 72, 12, 3.0f, 0, 0, FIX, kHoly, 1.4f));

LOOK(kAaMarrsGift, "marr's gift",
	G(IM, Glyph_Chalice, 0.9f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kMana, 2.8f),
	F(IM, FxForm_Flame, 0.8f, 9.0f, 72, 18, 0.1f, 2.6f, 0, FIX, kHealGold, 2.2f, 0.07f),
	F(IM, FxForm_Sparkle, 1.3f, 8.0f, 40, 20, 0.12f, 0, 0, FIX, kMana, 2.6f));

LOOK(kAaMarrsSalvation, "marr's salvation",
	G(RE, Glyph_Dove, 0.8f, 6.5f, 4, 3, GA_PopAscend | GlyphAnim_Spin, 1.2f, FIX, kWhite, 3.0f),
	F(RE, FxForm_Bloom, 2.5f, 2.0f, 48, 3, 0, 0, 0, FIX, kFeather, 2.4f, 0.05f, ADD | FxFlag_Reverse));

LOOK(kAaProjection, "projection of piety",
	G(RE, Glyph_Helm, 1.2f, 6.5f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kGhost, 3.5f),
	F(RE, FxForm_Flame, 1.0f, 7.0f, 80, 20, 0.2f, 1.2f, 0, FIX, kGhost, 3.5f, 0.06f),
	F(RE, FxForm_Rune, 2.4f, 0, 60, 6, 1, 12, 0, FIX, kWhite, 3.5f));

LOOK(kAaSpeed, "speed of the savior",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 12, 3.5f, 0, 0, FIX, kGold, 3.0f, 0.05f),
	G(IM, Glyph_Feather, 0.45f, 6.8f, 0, 1, GA_PopAscend, 0, FIX, kGold, 1.6f));

LOOK(kAaSpire, "spire of chivalry",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kHoly, 2.6f, 0.10f),
	G(RE, Glyph_Sword, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kSilver, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kGold, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaThunder, "thunder of karana",
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 2, 1.0f, 14.0f, 1.0f, FIX, kShock, 0.8f, 0.12f),
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 2, 1.2f, 14.0f, 1.0f, FIX, kShock, 0.7f, 0.10f, ADD, 1, 0.4f),
	F(IM, FxForm_Rain, 1.6f, 18.0f, 60, 60, 0, 0.3f, 0, FIX, kWater, 2.4f, 0.05f),
	G(IM, Glyph_Lightning, 0.7f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kShock, 2.4f));

LOOK(kAaValorousRage, "valorous rage",
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.8f, 1.2f, 0, FIX, kFlame2, 3.0f, 0.10f),
	G(IM, Glyph_Fist, 1.0f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kBlood, 2.6f),
	F(IM, FxForm_Burst, 2.8f, 0.3f, 72, 2, 0.4f, 16, 0, FIX, kBlood, 1.0f, 0.12f));

LOOK(kAaVanquish, "vanquish the fallen",
	G(IM, Glyph_Skull, 1.0f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kBone, 1.4f, 0, OVER),
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 3, 0.4f, 12.0f, 1.0f, FIX, kHoly, 0.9f, 0.14f, ADD, 1, 0.3f),
	F(IM, FxForm_Flame, 1.0f, 14.0f, 112, 28, 0.1f, 4.0f, 0, FIX, kHoly, 1.8f, 0.10f, ADD, 1, 0.3f),
	F(IM, FxForm_Burst, 3.4f, 0.4f, 120, 3, 1.0f, 24, 0, FIX, kGold, 1.2f, 0.12f, ADD, 1, 0.3f));

LOOK(kAaAegis, "aegis of the faithful",
	G(IM, Glyph_Shield, 1.0f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Dome, 1.9f, 7.0f, 112, 8, 3, 0, 0, FIX, kGold, 3.0f, 0.05f),
	F(IM, FxForm_Ring, 1.9f, 0.3f, 48, 0, 1.0f, 1.0f, 0.5f, FIX, kHoly, 3.0f, 0.05f, COMET));

enum Line : int
{
	Touch, Light, Burst, Wave, Aurora, Cleansing, Healing, Penitence, Splash, Protective,
	Preservation, Blessing, Consecrate, Purity, Symbol, ArmorFaith, ArmorProc, Brell, Keeper,
	HandKeeper, Gird, Mantle, Affirmation, Aureate, Fury, Righteous, Penitent, Reflexive, Stance,
	Steel, Guardian, Valiant, Pureforge, Blunted, Breather, Force, ForceSea, ForceAvowed, Forge,
	Eclipse, Lesson, CrushSea, Crush, Brilliant, Hymn, Doctrine, Chastise, Undead, Honor, Audacity,
	Words, MarkHero, MarkBrand, Shackle, Fallen, ProcHeal, ProcStrike, ProcGlint,
	LayOnHands, Judgment, Beacon, Lodestar, Purification, ChampionsOath, CloakOfLight, Persecution,
	DivineCall, ProtectorsUnity, Steed, DivineStun, GiftOfLife, GiftOfRes, HaltTheDead, HandOfPiety,
	HandOfTunare, HeroicLeap, LeapOfFaith, MarrsGift, MarrsSalvation, Projection, Speed, Spire,
	Thunder, ValorousRage, Vanquish, Aegis,
};

const LineLook kLines[] = {
	{ "Touch (single heal)",          &kPalTouch },
	{ "Light (single heal)",          &kPalLight },
	{ "Burst (group heal)",           &kPalBurst },
	{ "Wave (group heal)",            &kPalWave },
	{ "Aurora (group heal)",          &kPalAurora },
	{ "Cleansing (heal over time)",   &kPalCleansing },
	{ "Healing (heal over time)",     &kPalHealing },
	{ "Penitence (self heal)",        &kPalPenitence },
	{ "Splash (area heal, cure)",     &kPalSplash },
	{ "Protective (heal, trigger)",   &kPalProtective },
	{ "Preservation (group ward)",    &kPalPreservation },
	{ "Blessing (heal trigger)",      &kPalBlessing },
	{ "Consecrate (group cure)",      &kPalConsecrate },
	{ "Purity (cure)",                &kPalPurity },
	{ "Symbol (hit points)",          &kPalSymbol },
	{ "Armor of Faith (self)",        &kPalArmorFaith },
	{ "Armor (proc)",                 &kPalArmorProc },
	{ "Brell's (group hit points)",   &kPalBrell },
	{ "Keeper (hit points)",          &kPalKeeper },
	{ "Hand of the Keeper (group)",   &kPalHandKeeper },
	{ "Gird (mitigation)",            &kPalGird },
	{ "Mantle (heal proc)",           &kPalMantle },
	{ "Affirmation (triggers)",       &kPalAffirmation },
	{ "Aureate / Alliance",           &kPalAureate },
	{ "Fury (weapon proc)",           &kPalFury },
	{ "Righteous (weapon proc)",      &kPalRighteous },
	{ "Penitent (heal proc)",         &kPalPenitent },
	{ "Reflexive (heal proc)",        &kPalReflexive },
	{ "Stance (discipline)",          &kPalStance },
	{ "Steel (discipline)",           &kPalSteel },
	{ "Guardian (discipline)",        &kPalGuardian },
	{ "Valiant (discipline)",         &kPalValiant },
	{ "Pureforge (discipline)",       &kPalPureforge },
	{ "Blunted (discipline)",         &kPalBlunted },
	{ "Breather (discipline)",        &kPalBreather },
	{ "Force (stun)",                 &kPalForce },
	{ "Force of the Sea (stun)",      &kPalForceSea },
	{ "Force of the Oath (stun)",     &kPalForceAvowed },
	{ "Force of the Forge (stun)",    &kPalForge },
	{ "Reciprocal Force (stun)",      &kPalEclipse },
	{ "Lesson (stun, nuke)",          &kPalLesson },
	{ "Crush of the Sea (nuke)",      &kPalCrushSea },
	{ "Crush (nuke, group heal)",     &kPalCrush },
	{ "Brilliant (nuke, heal)",       &kPalBrilliant },
	{ "Hymn (undead nuke)",           &kPalHymn },
	{ "Doctrine (undead nuke)",       &kPalDoctrine },
	{ "Chastise (nuke, heal)",        &kPalChastise },
	{ "Bane of the Undead",           &kPalUndead },
	{ "Honor (hate)",                 &kPalHonor },
	{ "Audacity (hate)",              &kPalAudacity },
	{ "Words (hate reduction)",       &kPalWords },
	{ "Mark of the Hero (debuff)",    &kPalMarkHero },
	{ "Brand (debuff)",               &kPalMarkBrand },
	{ "Shackle (root)",               &kPalShackle },
	{ "Penitence for the Fallen",     &kPalFallen },
	{ "healing proc",                 &kPalProcHeal },
	{ "holy strike proc",             &kPalProcStrike },
	{ "triggered effect",             &kPalProcGlint },

	{ "AA: Lay on Hands",             &kAaLayOnHands },
	{ "AA: Inquisitor's Judgment",    &kAaJudgment },
	{ "AA: Beacon of the Righteous",  &kAaBeacon },
	{ "AA: Hallowed Lodestar",        &kAaLodestar },
	{ "AA: Purification",             &kAaPurification },
	{ "AA: Champion's Oath",          &kAaChampionsOath },
	{ "AA: Cloak of Light",           &kAaCloakOfLight },
	{ "AA: Disruptive Persecution",   &kAaPersecution },
	{ "AA: Divine Call",              &kAaDivineCall },
	{ "AA: Divine Protector's Unity", &kAaProtectorsUnity },
	{ "AA: Steeds",                   &kAaSteed },
	{ "AA: Divine Stun",              &kAaDivineStun },
	{ "AA: Gift of Life",             &kAaGiftOfLife },
	{ "AA: Gift of Resurrection",     &kAaGiftOfRes },
	{ "AA: Halt the Dead",            &kAaHaltTheDead },
	{ "AA: Hand of Piety",            &kAaHandOfPiety },
	{ "AA: Hand of Tunare",           &kAaHandOfTunare },
	{ "AA: Heroic Leap",              &kAaHeroicLeap },
	{ "AA: Leap of Faith",            &kAaLeapOfFaith },
	{ "AA: Marr's Gift",              &kAaMarrsGift },
	{ "AA: Marr's Salvation",         &kAaMarrsSalvation },
	{ "AA: Projection of Piety",      &kAaProjection },
	{ "AA: Speed of the Savior",      &kAaSpeed },
	{ "AA: Spire of Chivalry",        &kAaSpire },
	{ "AA: Thunder of Karana",        &kAaThunder },
	{ "AA: Valorous Rage",            &kAaValorousRage },
	{ "AA: Vanquish the Fallen",      &kAaVanquish },
	{ "AA: Aegis of the Faithful",    &kAaAegis },
};

const GroupLine kNumbered[] = {
	{ 0, Touch },       { 1, Force },        { 2, Burst },       { 3, Symbol },     { 4, Consecrate },
	{ 5, Honor },       { 7, Light },        { 8, Lesson },      { 9, Symbol },     { 10, ArmorFaith },
	{ 11, Cleansing },  { 12, Wave },        { 13, Brell },      { 14, Keeper },    { 17, ForceSea },
	{ 18, ArmorProc },  { 19, Hymn },        { 20, Preservation }, { 21, MarkHero }, { 22, Aurora },
	{ 23, Fury },       { 24, Purity },      { 25, Gird },       { 26, Stance },    { 27, CrushSea },
	{ 28, Crush },      { 29, MarkBrand },   { 30, Brilliant },  { 31, Mantle },    { 32, Splash },
	{ 33, Protective }, { 34, Words },       { 35, Chastise },   { 36, Righteous }, { 38, Penitence },
	{ 39, Pureforge },  { 41, Audacity },    { 42, Affirmation }, { 44, HandKeeper }, { 45, Steel },
	{ 46, ForceAvowed }, { 47, Guardian },   { 48, Reflexive },  { 49, Doctrine },  { 50, Valiant },
	{ 51, Forge },      { 52, Penitent },    { 53, Blunted },    { 54, Eclipse },   { 57, Blessing },
	{ 58, Aureate },    { 59, Healing },
};

const GroupLine kModern[] = {
	{ 10, Touch },      { 20, Force },       { 30, Burst },      { 40, Symbol },    { 60, Honor },
	{ 70, Light },      { 80, Lesson },      { 100, ArmorFaith }, { 110, Cleansing }, { 120, Wave },
	{ 130, Brell },     { 140, Keeper },     { 150, ForceSea },  { 160, ArmorProc }, { 170, Hymn },
	{ 180, Preservation }, { 190, MarkHero }, { 200, Aurora },   { 210, Fury },     { 230, Gird },
	{ 240, Stance },    { 250, CrushSea },   { 260, Crush },     { 270, MarkBrand }, { 280, Brilliant },
	{ 290, Mantle },    { 300, Splash },     { 310, Protective }, { 320, Words },   { 330, Chastise },
	{ 340, Righteous }, { 350, Penitence },  { 370, Audacity },  { 380, Affirmation }, { 390, HandKeeper },
	{ 400, Steel },     { 410, ForceAvowed }, { 411, Healing },  { 430, Reflexive }, { 440, Doctrine },
	{ 450, Valiant },   { 451, ProcHeal },   { 460, Forge },     { 470, Penitent }, { 490, Eclipse },
	{ 500, Blessing },  { 510, Aureate },
};

const GroupLine kOther[] = {
	{ 4027, Touch },      { 4028, Force },      { 4029, Burst },     { 5002, Consecrate }, { 5003, Honor },
	{ 5007, Light },      { 5008, Lesson },     { 5009, Symbol },    { 5011, ArmorFaith }, { 5012, Cleansing },
	{ 5014, Wave },       { 5015, Brell },      { 5016, Keeper },    { 5017, Hymn },      { 5020, Hymn },
	{ 5021, ForceSea },   { 5022, ArmorProc },  { 5023, Hymn },      { 4351, Undead },    { 4451, Undead },
	{ 4551, Undead },     { 3246, Shackle },    { 4566, Shackle },   { 3311, Breather },  { 3411, Breather },
	{ 3511, Breather },   { 3611, Breather },   { 3737, Breather },  { 3837, Breather },  { 3937, Breather },
	{ 2403111, Fallen },  { 19, ProcStrike },   { 40099, Blunted },
	{ 121030451, ProcHeal }, { 122030451, ProcHeal }, { 123030451, ProcHeal }, { 124030451, ProcHeal },
};

const NamedLine kAbilities[] = {
	{ "lay on hands",                            LayOnHands },
	{ "inquisitor's judgment",                   Judgment },
	{ "beacon of the righteous",                 Beacon },
	{ "hallowed lodestar",                       Lodestar },
	{ "blessing of purification",                Purification },
	{ "purification",                            Purification },
	{ "champion's oath",                         ChampionsOath },
	{ "cloak of light",                          CloakOfLight },
	{ "group perfected invisibility to undead",  CloakOfLight },
	{ "disruptive persecution",                  Persecution },
	{ "force of disruption",                     Persecution },
	{ "divine call",                             DivineCall },
	{ "divine protector's unity",                ProtectorsUnity },
	{ "divine steed",                            Steed },
	{ "holy steed",                              Steed },
	{ "valiant steed",                           Steed },
	{ "hallowed steed",                          Steed },
	{ "holy warhorse",                           Steed },
	{ "divine stun",                             DivineStun },
	{ "gift of life",                            GiftOfLife },
	{ "gift of resurrection",                    GiftOfRes },
	{ "halt the dead",                           HaltTheDead },
	{ "hand of piety",                           HandOfPiety },
	{ "hand of tunare",                          HandOfTunare },
	{ "heroic leap",                             HeroicLeap },
	{ "leap of faith",                           LeapOfFaith },
	{ "marr's gift",                             MarrsGift },
	{ "marr's salvation",                        MarrsSalvation },
	{ "projection of piety",                     Projection },
	{ "speed of the savior",                     Speed },
	{ "spire of chivalry",                       Spire },
	{ "thunder of karana",                       Thunder },
	{ "valorous rage",                           ValorousRage },
	{ "vanquish the fallen",                     Vanquish },
	{ "aegis of the faithful",                   Aegis },
};

int BySpell(const eqlib::EQ_Spell*, std::string_view base)
{
	if (EndsWith(base, " heal") || StartsWith(base, "penitent mending") || StartsWith(base, "penitent reward")
		|| StartsWith(base, "valiant healing") || StartsWith(base, "steely "))
		return ProcHeal;
	if (EndsWith(base, " strike"))
		return ProcStrike;
	if (EndsWith(base, " trigger") || EndsWith(base, " effect") || EndsWith(base, " reflection")
		|| EndsWith(base, " guard"))
		return ProcGlint;
	return -1;
}

} // namespace

const ClassCatalogue& PaladinCatalogue()
{
	static const ClassCatalogue c{ "Paladin", eqlib::Paladin, kLines, 10, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
