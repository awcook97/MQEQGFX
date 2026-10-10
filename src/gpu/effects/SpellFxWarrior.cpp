#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

namespace eqgfx::spellfx {

namespace {

constexpr Pair kWarSteel { { 0.94f, 0.96f, 1.00f, 0.95f }, { 0.48f, 0.54f, 0.64f, 0.90f } };
constexpr Pair kGunmetal { { 0.62f, 0.64f, 0.70f, 0.95f }, { 0.24f, 0.26f, 0.30f, 0.90f } };
constexpr Pair kBronze   { { 1.00f, 0.82f, 0.52f, 0.95f }, { 0.62f, 0.38f, 0.14f, 0.90f } };
constexpr Pair kRage     { { 1.00f, 0.60f, 0.22f, 0.95f }, { 0.72f, 0.02f, 0.02f, 0.90f } };
constexpr Pair kCrimson  { { 1.00f, 0.84f, 0.42f, 0.95f }, { 0.72f, 0.06f, 0.10f, 0.90f } };
constexpr Pair kJeer     { { 1.00f, 0.95f, 0.50f, 0.95f }, { 0.95f, 0.38f, 0.10f, 0.85f } };
constexpr Pair kWarRune  { { 0.82f, 0.94f, 1.00f, 0.95f }, { 0.40f, 0.55f, 0.95f, 0.85f } };
constexpr Pair kVigor    { { 0.90f, 1.00f, 0.82f, 0.95f }, { 0.95f, 0.66f, 0.28f, 0.85f } };
constexpr Pair kDusk     { { 0.82f, 0.78f, 1.00f, 0.95f }, { 0.26f, 0.20f, 0.56f, 0.85f } };

constexpr uint32_t GA_Emerge = GlyphAnim_Rise | GlyphAnim_Pop;

LOOK(kWarRunes, "armor of runes",
	F(IM, FxForm_Rune, 1.6f, 0, 80, 7, 3, 21, 0, FIX, kWarRune, 3.0f, 0.05f),
	F(IM, FxForm_Rune, 0.9f, 4.0f, 70, 5, 2, 15, 2, FIX, kWarRune, 2.6f, 0.05f, ADD, 1, 0.3f),
	G(IM, Glyph_Shield, 0.35f, 4.5f, 0, 5, GA_Orbit, 1.3f, FIX, kWarRune, 3.0f),
	Persist(F(AU, FxForm_Ring, 1.0f, 3.5f, 40, 0, 0.6f, 1.0f, 0.3f, FIX, kWarRune, 0, 0.04f, COMET)));

LOOK(kWarStand, "stand",
	F(IM, FxForm_Cracks, 2.4f, 0, 56, 7, 1.1f, 0, 0, FIX, kStone, 2.6f, 0.07f, OVER),
	G(IM, Glyph_Tower, 1.1f, 8.0f, 4, 1, GA_Emerge, 0, FIX, kWarSteel, 3.0f),
	F(IM, FxForm_Lattice, 1.4f, 6.5f, 80, 10, 0.4f, 0, 0, FIX, kWarSteel, 3.0f, 0.06f),
	F(IM, FxForm_Burst, 2.6f, 0.2f, 64, 2, 0.2f, 0, 0, FIX, kBronze, 1.0f, 0.10f, OVER));

LOOK(kWarDefense, "defense",
	G(IM, Glyph_Shield, 0.6f, 3.6f, 3, 4, GA_PopSpin, 1.2f, FIX, kBronze, 2.6f),
	F(IM, FxForm_Crystal, 1.0f, 5.0f, 60, 12, 0, 0.6f, 0, FIX, kGunmetal, 2.6f, 0.06f, OVER),
	F(IM, FxForm_Ring, 1.4f, 0.3f, 48, 0, 0.8f, 1.0f, 0.4f, FIX, kBronze, 2.6f, 0.05f, COMET));

LOOK(kWarStormDefense, "storm defense",
	F(IM, FxForm_Lightning, 1, 30.0f, 48, 2, 0.6f, 8.0f, 1.0f, FIX, kShock, 1.0f, 0.10f),
	G(IM, Glyph_Shield, 0.8f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kShock, 3.0f),
	G(IM, Glyph_Lightning, 0.3f, 4.0f, 0, 4, GA_Orbit, 1.3f, FIX, kShock, 3.0f),
	Persist(F(AU, FxForm_Ring, 1.0f, 4.0f, 40, 0, 2.0f, 1.0f, 0.6f, FIX, kShock, 0, 0.04f, COMET)));

LOOK(kWarShieldLine, "shield",
	G(IM, Glyph_Shield, 1.2f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kWarSteel, 3.0f),
	F(IM, FxForm_Dome, 1.9f, 7.2f, 112, 10, 4, 0, 0, FIX, kDusk, 3.0f, 0.04f),
	F(IM, FxForm_Lattice, 1.8f, 6.8f, 96, 12, 0.5f, 0, 0, FIX, kWarSteel, 3.0f, 0.04f),
	Persist(F(AU, FxForm_Helix, 1.1f, 6.0f, 96, 2, 1.0f, 0, 0.3f, FIX, kDusk, 0, 0.05f, COMET)));

LOOK(kWarShieldTrigger, "shielding trigger",
	G(IM, Glyph_Shield, 0.45f, 5.5f, 4, 1, GA_PopAscend, 0, FIX, kDusk, 1.2f),
	F(IM, FxForm_Ring, 1.1f, 3.5f, 32, 0, 2.0f, 1.0f, 0.8f, FIX, kWarSteel, 1.0f, 0.04f, COMET));

LOOK(kWarShieldReflect, "shield reflect",
	G(IM, Glyph_Shield, 1.1f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kWarSteel, 2.0f),
	F(IM, FxForm_Dome, 1.6f, 6.5f, 96, 8, 3, 0, 0, FIX, kSilver, 1.8f, 0.04f),
	F(IM, FxForm_Burst, 2.0f, 3.5f, 64, 2, 0, 14, 0, FIX, kWhite, 0.8f, 0.07f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Spire, 0.8f, 4.0f, 64, 12, 2.0f, 2.4f, 0, FIX, kSilver, 0.8f, 0.06f));

LOOK(kWarBulwark, "warrior's bulwark",
	F(TR, FxForm_Bolt, 1, 0, 64, 2, 0.2f, 0.3f, 0.4f, FIX, kCrimson, 0.5f, 0.12f),
	G(RE, Glyph_Tower, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kWarSteel, 2.6f),
	F(RE, FxForm_Dome, 1.7f, 6.8f, 96, 8, 3, 0, 0, FIX, kWarSteel, 2.4f, 0.04f),
	F(IM, FxForm_Burst, 1.8f, 5.0f, 48, 1, 0, 10, 0, FIX, kCrimson, 0.7f, 0.08f));

LOOK(kWarNoBleed, "no time to bleed",
	F(IM, FxForm_Crystal, 1.0f, 5.5f, 72, 14, 0, 0.5f, 0, FIX, kStone, 2.8f, 0.06f, OVER),
	G(IM, Glyph_Heart, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kBlood, 2.8f),
	F(IM, FxForm_Bloom, 1.4f, 4.0f, 40, 3, 0, 0, 0, FIX, kBlood, 2.0f, 0.05f, ADD | FxFlag_Reverse));

LOOK(kWarDefensive, "defensive",
	G(IM, Glyph_Shield, 1.1f, 7.4f, 4, 1, GA_Emerge, 0, FIX, kIron, 3.0f),
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kIron, 3.0f, 0.04f),
	F(IM, FxForm_Burst, 2.2f, 0.2f, 48, 2, 0.2f, 0, 0, FIX, kIron, 1.0f, 0.10f, OVER));

LOOK(kWarFortitude, "fortitude",
	G(IM, Glyph_Helm, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kBronze, 3.0f),
	F(IM, FxForm_Lattice, 1.5f, 7.0f, 96, 12, 0.3f, 0, 0, FIX, kBronze, 3.0f, 0.05f),
	F(IM, FxForm_Spire, 0.8f, 7.0f, 96, 16, 0.8f, 2.6f, 0, FIX, kGold, 1.6f, 0.06f));

LOOK(kWarEvasive, "evasive",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 60, 10, 2.5f, 0, 0, FIX, kGhost, 2.6f),
	G(IM, Glyph_Feather, 0.4f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kFeather, 2.6f),
	F(IM, FxForm_Ring, 1.3f, 3.0f, 40, 0, -2.0f, 1.0f, 0.8f, FIX, kGhost, 2.6f, 0.05f, COMET));

LOOK(kWarResistant, "resistant",
	G(IM, Glyph_Gem, 0.8f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kPrism, 2.6f),
	F(IM, FxForm_Dome, 1.7f, 6.6f, 88, 6, 2, 0, 0, FIX, kPrism, 2.4f, 0.03f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.12f, 2, 0, FIX, kPrism, 2.2f));

LOOK(kWarFearless, "fearless",
	G(IM, Glyph_Skull, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kGrey, 1.6f, 0, OVER),
	G(IM, Glyph_Fist, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.6f, 0.5f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 64, 2, 1.0f, 12, 0, FIX, kGold, 1.0f, 0.09f, ADD, 1, 0.5f));

LOOK(kWarWind, "wind",
	G(IM, Glyph_Tornado, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kAir, 2.4f),
	F(IM, FxForm_Vortex, 1.2f, 6.0f, 72, 4, 1.2f, 2.0f, 3.0f, FIX, kVigor, 2.4f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 40, 20, 0.12f, 0, 0, FIX, kVigor, 2.4f));

LOOK(kWarBreather, "breather",
	G(IM, Glyph_Moon, 0.7f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kVigor, 3.0f),
	F(IM, FxForm_Bubbles, 1.0f, 6.5f, 48, 8, 0.18f, 0, 0, FIX, kVigor, 3.0f, 0.035f),
	F(IM, FxForm_Bloom, 1.6f, 1.0f, 40, 2, 0, 0, 0, FIX, kVigor, 3.0f, 0.04f));

LOOK(kWarInnerRejuv, "inner rejuvenation",
	G(IM, Glyph_Heart, 0.8f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 2.6f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 72, 3, 1.2f, 0, 0.5f, FIX, kVigor, 2.6f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.1f, 6.5f, 32, 16, 0.10f, 1, 0, FIX, kHeal, 2.2f));

LOOK(kWarHealingWill, "healing will",
	G(IM, Glyph_Hand, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kHeal, 2.6f),
	F(IM, FxForm_Flame, 0.8f, 7.0f, 64, 16, 0.1f, 2.2f, 0, FIX, kHeal, 2.4f, 0.06f),
	F(IM, FxForm_Orbit, 1.2f, 6.0f, 40, 8, 0, 0, 0, FIX, kHealGold, 2.4f, 0.04f, ADD, 2.0f));

LOOK(kWarOnslaught, "onslaught",
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.8f, 1.3f, 0, FIX, kFlame2, 3.0f, 0.10f),
	G(IM, Glyph_Sword, 0.5f, 4.0f, 3, 4, GA_PopSpin, 1.3f, FIX, kRage, 3.0f),
	F(IM, FxForm_Spire, 0.6f, 6.0f, 96, 16, 1.5f, 2.8f, 0, FIX, kRage, 1.4f, 0.07f),
	F(IM, FxForm_Burst, 2.6f, 0.3f, 72, 2, 0.4f, 16, 0, FIX, kCrimson, 1.0f, 0.11f));

LOOK(kWarOffensive, "offensive",
	F(IM, FxForm_Flame, 1.05f, 6.0f, 112, 28, 0.8f, 1.1f, 0, FIX, kRage, 3.0f, 0.09f),
	G(IM, Glyph_Fist, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kCrimson, 3.0f),
	G(IM, Glyph_ArrowUp, 0.3f, 4.5f, 0, 4, GA_Orbit | GlyphAnim_Ascend, 1.2f, FIX, kRage, 2.4f));

LOOK(kWarThrough, "punch through",
	G(IM, Glyph_Sword, 0.5f, 4.0f, 3, 3, GA_PopSpin, 1.0f, FIX, kGunmetal, 2.4f),
	F(IM, FxForm_Spire, 0.4f, 4.0f, 96, 12, 0.0f, 3.2f, 0, FIX, kWarSteel, 1.4f, 0.05f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 64, 2, 2.0f, 0, 0.5f, FIX, kGunmetal, 2.2f, 0.05f, COMET));

LOOK(kWarWeapon, "weapon affiliation",
	G(IM, Glyph_Sword, 0.8f, 6.0f, 4, 1, GA_PopBob, 0, FIX, kWarSteel, 2.6f),
	G(IM, Glyph_Link, 0.3f, 4.2f, 0, 4, GA_Orbit, 1.1f, FIX, kGold, 2.6f),
	F(IM, FxForm_Helix, 0.9f, 6.0f, 72, 3, 2.0f, 0, 0.5f, FIX, kGold, 2.4f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.0f, 5.0f, 24, 12, 0.10f, 2, 0, FIX, kWhite, 2.0f));

LOOK(kWarFlashAnger, "flash of anger",
	F(IM, FxForm_Flame, 1.0f, 5.5f, 96, 24, 0.9f, 0.8f, 0, FIX, kRage, 1.2f, 0.07f),
	G(IM, Glyph_Sword, 0.5f, 4.0f, 3, 5, GA_Orbit, 1.3f, FIX, kWarSteel, 3.0f),
	F(IM, FxForm_Burst, 2.0f, 4.0f, 48, 1, 0, 14, 0, FIX, kRage, 0.5f, 0.08f));

LOOK(kWarReprisal, "determined reprisal",
	G(IM, Glyph_Sword, 0.9f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kCrimson, 2.6f),
	F(IM, FxForm_Burst, 2.2f, 3.5f, 64, 2, 0, 16, 0, FIX, kRage, 1.0f, 0.08f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Rune, 1.2f, 3.5f, 60, 6, 1, 12, 2, FIX, kCrimson, 2.6f, 0.05f));

LOOK(kWarCharge, "charge",
	F(IM, FxForm_Wind, 1.5f, 4.0f, 72, 14, 3.5f, 0, 0, FIX, kRage, 1.6f),
	G(IM, Glyph_Sword, 0.9f, 7.4f, 1, 1, GA_PopRoll, 0, FIX, kWarSteel, 1.8f),
	F(IM, FxForm_Cracks, 1.8f, 0, 40, 5, 1.2f, 0, 0, FIX, kEarth, 1.8f, 0.06f, OVER));

LOOK(kWarMightyStrike, "mighty strike",
	G(IM, Glyph_Hammer, 1.0f, 8.2f, 4, 1, GA_PopRoll, 0, FIX, kWarSteel, 2.4f),
	G(IM, Glyph_Star, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.2f, FIX, kStar, 2.4f),
	F(IM, FxForm_Burst, 2.4f, 1.0f, 64, 2, 1.0f, 16, 0, FIX, kStar, 1.0f, 0.09f));

LOOK(kWarFellstrike, "fellstrike",
	G(IM, Glyph_Claw, 0.9f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kBlood, 2.4f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.9f, 0.8f, 0, FIX, kBlood, 2.6f, 0.07f),
	F(IM, FxForm_Sparkle, 1.2f, 5.0f, 32, 16, 0.10f, 0, 0, FIX, kRage, 2.2f));

LOOK(kWarFurious, "furious",
	G(IM, Glyph_Sword, 0.6f, 4.5f, 3, 4, GA_RingRise, 1.4f, FIX, kRage, 2.6f),
	F(IM, FxForm_Spire, 0.8f, 5.0f, 96, 16, 3.0f, 3.0f, 0, FIX, kRage, 1.0f, 0.07f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Ring, 1.4f, 4.0f, 48, 0, 3.0f, 1.0f, 1.0f, FIX, kWarSteel, 2.4f, 0.05f, COMET));

LOOK(kWarPrecision, "precision",
	G(IM, Glyph_Eye, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kWarSteel, 2.6f),
	F(IM, FxForm_Rune, 1.2f, 4.0f, 60, 4, 1, 16, 2, FIX, kWarSteel, 2.6f, 0.04f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 24, 12, 0.10f, 2, 0, FIX, kWhite, 2.0f));

LOOK(kWarSpiritRage, "spirit of rage",
	G(IM, Glyph_Spirit, 0.9f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kRage, 2.6f),
	F(IM, FxForm_Vortex, 1.2f, 6.0f, 72, 4, 1.2f, 2.0f, 4.0f, FIX, kRage, 2.6f, 0.05f),
	F(IM, FxForm_Burst, 2.0f, 1.0f, 48, 2, 1.0f, 12, 0, FIX, kCrimson, 1.0f, 0.08f));

LOOK(kWarBlades, "blades",
	G(RE, Glyph_Sword, 0.7f, 3.5f, 3, 6, GA_PopSpin, 1.8f, FIX, kWarSteel, 1.4f),
	F(RE, FxForm_Ring, 1.0f, 3.2f, 64, 0, 4.0f, 1.0f, 1.0f, FIX, kWarSteel, 1.2f, 0.08f, COMET, 1, 0, AE),
	F(RE, FxForm_Ring, 0.7f, 3.8f, 64, 0, -4.0f, 1.0f, 1.0f, FIX, kSilver, 1.2f, 0.06f, COMET, 1, 0.15f, AE),
	F(RE, FxForm_Wind, 1.5f, 4.0f, 60, 12, 2.5f, 0, 0, FIX, kWarSteel, 1.0f),
	F(IM, FxForm_Sparkle, 1.0f, 5.0f, 24, 12, 0.10f, 0, 0, FIX, kWhite, 0.8f));

LOOK(kWarThroatJab, "throat jab",
	F(TR, FxForm_Bolt, 1, 0, 64, 2, 0.1f, 0.2f, 0.2f, FIX, kWarSteel, 0.4f, 0.10f),
	G(IM, Glyph_Fist, 0.6f, 5.2f, 4, 1, GA_PopRoll, 0, FIX, kBronze, 1.0f),
	G(IM, Glyph_Hourglass, 0.4f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kGrey, 2.4f, 0.2f),
	F(IM, FxForm_Burst, 1.2f, 5.0f, 32, 1, 0, 8, 0, FIX, kWhite, 0.5f, 0.07f));

LOOK(kWarStrike, "opportunistic strike",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.2f, 0.3f, 0.6f, FIX, kWarSteel, 0.5f, 0.14f),
	G(IM, Glyph_Sword, 1.3f, 9.0f, 4, 1, GA_Drop, 0, FIX, kWarSteel, 1.4f),
	F(IM, FxForm_Burst, 2.4f, 1.0f, 72, 2, 0.6f, 16, 0, FIX, kBlood, 0.9f, 0.10f, ADD, 1, 0.3f),
	F(IM, FxForm_Cracks, 1.6f, 0, 40, 5, 1.2f, 0, 0, FIX, kBlood, 1.4f, 0.05f, ADD, 1, 0.3f));

LOOK(kWarVitalStrike, "vital strike",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.3f, 0.35f, 0.8f, FIX, kRage, 0.6f, 0.15f),
	G(IM, Glyph_Sword, 1.0f, 6.0f, 4, 1, GA_PopRoll, 0, FIX, kWarSteel, 1.2f),
	G(IM, Glyph_Heart, 0.6f, 6.0f, 4, 1, GA_PopAscend, 0, FIX, kBlood, 1.6f, 0.2f),
	F(IM, FxForm_Spire, 0.6f, 5.0f, 64, 12, 2.0f, 2.5f, 0, FIX, kBlood, 0.8f, 0.07f, ADD, 1, 0.2f));

LOOK(kWarKnuckle, "knuckle",
	G(IM, Glyph_Fist, 1.0f, 6.0f, 4, 1, GA_PopRoll, 0, FIX, kBronze, 1.2f),
	F(IM, FxForm_Burst, 2.0f, 5.0f, 48, 2, 0.2f, 12, 0, FIX, kBronze, 0.7f, 0.09f),
	G(IM, Glyph_ArrowDown, 0.35f, 5.0f, 0, 3, GA_Orbit | GlyphAnim_Descend, 1.1f, FIX, kGrey, 2.4f, 0.2f));

LOOK(kWarShieldBash, "shield bash",
	F(RE, FxForm_Dome, 1.5f, 6.2f, 72, 6, 2, 0, 0, FIX, kBronze, 1.6f, 0.03f),
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.2f, 0.3f, 0.3f, FIX, kBronze, 0.5f, 0.18f),
	G(IM, Glyph_Shield, 1.2f, 6.0f, 4, 1, GA_PopRoll, 0, FIX, kBronze, 1.2f),
	F(IM, FxForm_Burst, 2.4f, 4.0f, 72, 2, 0.3f, 14, 0, FIX, kWarSteel, 0.8f, 0.10f),
	Stars(0.25f));

LOOK(kWarTaunt, "insult",
	G(RE, Glyph_Mouth, 0.7f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kJeer, 1.4f),
	F(RE, FxForm_Bloom, 1.2f, 6.5f, 40, 3, 0, 0, 0, FIX, kJeer, 1.2f, 0.05f),
	F(TR, FxForm_Drain, 1, 0, 48, 2, 0.6f, 10, 0, FIX, kJeer, 1.0f, 0.06f),
	G(IM, Glyph_Mouth, 0.8f, 8.0f, 4, 1, GA_Drop, 0, FIX, kBlood, 1.8f, 0.2f),
	F(IM, FxForm_Burst, 1.8f, 7.0f, 40, 1, 0, 10, 0, FIX, kJeer, 0.6f, 0.07f, ADD, 1, 0.2f));

LOOK(kWarRoar, "roar",
	G(RE, Glyph_Horn, 0.9f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kBronze, 1.6f),
	F(RE, FxForm_Bloom, 2.4f, 6.0f, 64, 4, 0, 0, 0, FIX, kRage, 1.4f, 0.06f),
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.4f, 0.3f, FIX, kRage, 0.7f, 0.14f),
	F(IM, FxForm_Burst, 2.2f, 5.0f, 64, 2, 0.4f, 12, 0, FIX, kRage, 0.9f, 0.09f));

LOOK(kWarShout, "shout",
	G(RE, Glyph_Mouth, 0.9f, 7.2f, 4, 1, GA_PopRoll, 0, FIX, kRage, 1.4f),
	F(RE, FxForm_Bloom, 1.6f, 7.0f, 48, 4, 0, 0, 0, FIX, kJeer, 1.0f, 0.06f),
	F(TR, FxForm_Bolt, 1, 0, 96, 5, 0.6f, 0.5f, 0.2f, FIX, kJeer, 0.6f, 0.18f),
	F(IM, FxForm_Bloom, 2.4f, 5.0f, 48, 3, 0, 0, 0, FIX, kRage, 1.2f, 0.06f, ADD | FxFlag_Reverse),
	G(IM, Glyph_ArrowDown, 0.5f, 8.0f, 4, 1, GA_Drop, 0, FIX, kJeer, 1.8f, 0.2f));

LOOK(kWarRally, "rally cry",
	G(RE, Glyph_Banner, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kCrimson, 2.0f),
	G(RE, Glyph_Horn, 0.6f, 5.0f, 4, 1, GA_PopBob, 0, FIX, kBronze, 1.6f, 0.2f),
	F(RE, FxForm_Bloom, 1.0f, 5.0f, 96, 4, 0, 0, 0, FIX, kCrimson, 1.6f, 0.06f, ADD, 1.2f, 0, AE),
	G(IM, Glyph_Link, 0.35f, 0.8f, 3, 4, GA_PopSpin, 1.0f, FIX, kGunmetal, 2.0f));

LOOK(kWarAttention, "attention",
	F(RE, FxForm_Crystal, 0.9f, 5.0f, 40, 8, 0, 0.5f, 0, FIX, kWarSteel, 1.6f, 0.05f, OVER),
	F(TR, FxForm_Drain, 1, 0, 48, 2, 0.4f, 10, 0, FIX, kCrimson, 1.6f, 0.06f),
	G(IM, Glyph_Eye, 0.9f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kCrimson, 3.0f),
	F(IM, FxForm_Ring, 1.2f, 0.4f, 48, 0, 1.0f, 1.0f, 0.5f, FIX, kCrimson, 3.0f, 0.05f, COMET));

LOOK(kWarTongue, "barbed tongue",
	G(IM, Glyph_Mouth, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kBlood, 2.4f),
	F(IM, FxForm_Thorns, 1.0f, 4.0f, 96, 4, 0.8f, 5, 0, FIX, kBlood, 2.4f, 0.05f),
	G(IM, Glyph_Fang, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kBone, 2.4f));

LOOK(kWarAttraction, "forceful attraction",
	G(RE, Glyph_Shackle, 0.9f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kWarSteel, 2.0f),
	F(RE, FxForm_Vortex, 1.0f, 4.0f, 96, 6, 1.4f, 2.0f, -4.0f, FIX, kGunmetal, 2.2f, 0.06f, ADD, 1, 0, AE),
	F(RE, FxForm_Sparkle, 1.0f, 5.0f, 60, 30, 0.12f, 2, 0, FIX, kWarSteel, 2.2f, 0.06f, ADD, 1, 0, AE),
	F(IM, FxForm_Wind, 1.2f, 4.0f, 48, 10, 2.5f, 0, 0, FIX, kGunmetal, 1.6f));

LOOK(kWarPhantom, "phantom aggressor",
	F(RE, FxForm_Rune, 1.6f, 0, 60, 6, 1, 12, 0, FIX, kDusk, 2.0f),
	G(IM, Glyph_Helm, 1.0f, 6.0f, 4, 1, GA_Emerge, 0, FIX, kGhost, 3.0f),
	G(IM, Glyph_Sword, 0.6f, 4.0f, 3, 3, GA_Orbit, 1.4f, FIX, kGhost, 3.0f),
	F(IM, FxForm_Flame, 0.9f, 7.0f, 72, 18, 0.2f, 1.2f, 0, FIX, kGhost, 3.0f, 0.05f));

LOOK(kWarWadeIn, "wade into battle",
	G(IM, Glyph_Horn, 0.4f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kBronze, 3.0f),
	F(IM, FxForm_Wind, 1.4f, 5.0f, 72, 12, 3.0f, 0, 0, FIX, kWarSteel, 2.0f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 2.2f, 0.06f, OVER),
	F(IM, FxForm_Bloom, 2.2f, 5.0f, 48, 3, 0, 0, 0, FIX, kJeer, 1.6f, 0.05f));

LOOK(kWarExpanse, "expanse",
	G(IM, Glyph_Cross, 0.4f, 5.0f, 0, 4, GA_Orbit, 1.2f, FIX, kHeal, 3.0f),
	F(IM, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kCrimson, 2.4f, 0.05f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Ring, 1.0f, 0.4f, 64, 0, 0.6f, 1.0f, 0.3f, FIX, kCrimson, 3.0f, 0.05f, COMET, 1, 0, AE));

LOOK(kWarHealPrecision, "heal precision",
	G(IM, Glyph_Cross, 0.7f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kHealGold, 3.0f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 72, 3, 1.4f, 0, 0.5f, FIX, kCrimson, 3.0f, 0.05f, COMET),
	G(IM, Glyph_Eye, 0.35f, 4.5f, 0, 2, GA_Orbit, 1.2f, FIX, kCrimson, 3.0f));

LOOK(kWarField, "field armorer",
	G(RE, Glyph_Anvil, 0.9f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kForge, 2.2f),
	F(RE, FxForm_Rune, 1.0f, 0, 96, 6, 1, 18, 0, FIX, kBronze, 2.4f, 0.05f, ADD, 1, 0, AE),
	F(RE, FxForm_Sparkle, 1.0f, 5.0f, 60, 30, 0.12f, 0, 0, FIX, kForge, 2.0f, 0.06f, ADD, 1, 0.2f, AE),
	G(IM, Glyph_Helm, 0.5f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kWarSteel, 2.0f));

LOOK(kWarAura, "champion's aura",
	G(RE, Glyph_Banner, 1.1f, 8.5f, 4, 1, GA_Emerge, 0, FIX, kCrimson, 3.0f),
	F(RE, FxForm_Rune, 1.0f, 0, 96, 8, 3, 24, 0, FIX, kCrimson, 3.0f, 0.05f, ADD, 1, 0, AE),
	Persist(F(AU, FxForm_Ring, 1.6f, 0.3f, 48, 0, 0.4f, 1.0f, 0.2f, FIX, kCrimson, 0, 0.05f, COMET)));

LOOK(kWarVoice, "commanding voice",
	G(RE, Glyph_Horn, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.2f),
	F(RE, FxForm_Bloom, 1.0f, 5.0f, 96, 4, 0, 0, 0, FIX, kGold, 2.0f, 0.05f, ADD, 1, 0, AE),
	G(IM, Glyph_Feather, 0.35f, 5.0f, 0, 2, GA_Orbit, 1.1f, FIX, kFeather, 2.4f));

LOOK(kAaAreaTaunt, "area taunt",
	G(RE, Glyph_Horn, 1.1f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kCrimson, 2.0f),
	F(RE, FxForm_Burst, 1.0f, 4.0f, 120, 3, 0.4f, 20, 0, FIX, kRage, 1.2f, 0.10f, ADD, 1, 0, AE),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 4, 0, 0, 0, FIX, kCrimson, 1.6f, 0.06f, ADD, 1, 0.2f, AE),
	G(IM, Glyph_Mouth, 0.5f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kJeer, 1.6f));

LOOK(kAaBattleLeap, "battle leap",
	F(RE, FxForm_Wind, 1.4f, 4.0f, 60, 12, 3.0f, 0, 0, FIX, kWarSteel, 1.0f),
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.2f, 0.3f, 3.0f, FIX, kBronze, 0.7f, 0.16f),
	F(IM, FxForm_Cracks, 2.4f, 0, 56, 7, 1.2f, 0, 0, FIX, kEarth, 2.0f, 0.07f, OVER, 1, 0.3f),
	F(IM, FxForm_Burst, 3.0f, 0.2f, 72, 2, 0.3f, 14, 0, FIX, kBronze, 1.0f, 0.10f, ADD, 1, 0.3f));

LOOK(kAaFuriousLeap, "furious leap",
	F(RE, FxForm_Wind, 1.5f, 4.0f, 72, 14, 3.5f, 0, 0, FIX, kRage, 1.2f),
	F(RE, FxForm_Flame, 1.0f, 5.0f, 64, 16, 0.9f, 0.6f, 0, FIX, kRage, 1.0f, 0.07f),
	F(RE, FxForm_Burst, 2.6f, 0.2f, 64, 2, 0.3f, 12, 0, FIX, kRage, 0.9f, 0.10f),
	G(RE, Glyph_ArrowUp, 0.8f, 7.0f, 1, 1, GA_PopAscend, 0, FIX, kRage, 1.4f));

LOOK(kAaBladeGuardian, "blade guardian",
	G(IM, Glyph_Sword, 0.55f, 4.0f, 3, 6, GA_Orbit, 1.5f, FIX, kWarSteel, 3.0f),
	F(IM, FxForm_Ring, 1.5f, 4.0f, 64, 0, 3.0f, 1.0f, 1.0f, FIX, kSilver, 3.0f, 0.06f, COMET),
	F(IM, FxForm_Ring, 1.3f, 2.5f, 64, 0, -3.0f, 1.0f, 1.0f, FIX, kWarSteel, 3.0f, 0.05f, COMET));

LOOK(kAaBlastOfAnger, "blast of anger",
	F(TR, FxForm_Bolt, 1, 0, 128, 5, 0.6f, 0.5f, 0.5f, FIX, kRage, 0.7f, 0.22f),
	G(IM, Glyph_Fist, 1.1f, 8.0f, 4, 1, GA_Drop, 0, FIX, kRage, 1.6f),
	F(IM, FxForm_Burst, 2.8f, 3.0f, 96, 3, 1.0f, 18, 0, FIX, kFlame2, 1.0f, 0.12f),
	F(IM, FxForm_Flame, 0.9f, 7.0f, 72, 18, 0.2f, 1.4f, 0, FIX, kRage, 1.2f, 0.07f));

LOOK(kAaBraceImpact, "brace for impact",
	G(IM, Glyph_Shield, 1.3f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kBronze, 3.0f),
	F(IM, FxForm_Cracks, 2.2f, 0, 48, 6, 1.0f, 0, 0, FIX, kStone, 2.6f, 0.06f, OVER),
	F(IM, FxForm_Dome, 1.8f, 6.8f, 112, 8, 3, 0, 0, FIX, kBronze, 3.0f, 0.05f),
	F(IM, FxForm_Burst, 2.4f, 3.0f, 64, 2, 0, 12, 0, FIX, kBronze, 0.8f, 0.08f, ADD | FxFlag_Reverse));

LOOK(kAaCallChallenge, "call of challenge",
	G(RE, Glyph_Horn, 0.7f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kCrimson, 1.4f),
	G(IM, Glyph_Link, 0.35f, 0.6f, 3, 5, GA_PopSpin, 1.0f, FIX, kGunmetal, 3.0f),
	F(IM, FxForm_Ring, 1.0f, 0.6f, 40, 0, 0.3f, 1.0f, 0, FIX, kCrimson, 3.0f, 0.04f));

LOOK(kAaGrapple, "grappling strike",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 0.4f, 8, 0, FIX, kGunmetal, 0.8f, 0.07f),
	G(IM, Glyph_Hand, 0.9f, 6.0f, 4, 1, GA_PopRoll, 0, FIX, kBronze, 1.2f),
	F(IM, FxForm_Wind, 1.2f, 4.0f, 48, 10, 2.5f, 0, 0, FIX, kGunmetal, 1.0f),
	F(IM, FxForm_Burst, 1.8f, 4.0f, 40, 1, 0, 10, 0, FIX, kWarSteel, 0.6f, 0.08f));

LOOK(kAaGutPunch, "gut punch",
	G(IM, Glyph_Fist, 0.9f, 4.5f, 4, 1, GA_PopRoll, 0, FIX, kBronze, 1.0f),
	F(IM, FxForm_Burst, 1.8f, 4.0f, 48, 2, 0, 14, 0, FIX, kCrimson, 0.6f, 0.09f),
	G(IM, Glyph_Star, 0.25f, 6.5f, 0, 3, GA_PopSpin, 0.8f, FIX, kStar, 1.6f, 0.15f));

LOOK(kAaKneeStrike, "knee strike",
	F(IM, FxForm_Burst, 1.6f, 1.5f, 48, 2, 0, 12, 0, FIX, kBronze, 0.6f, 0.09f),
	G(IM, Glyph_ArrowDown, 0.6f, 3.0f, 4, 1, GA_Drop, 0, FIX, kGunmetal, 1.6f),
	F(IM, FxForm_Cracks, 1.2f, 0, 32, 4, 1.0f, 0, 0, FIX, kEarth, 1.4f, 0.05f, OVER));

LOOK(kAaHowl, "howl of the warlord",
	G(RE, Glyph_Wolf, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGhost, 2.4f),
	F(RE, FxForm_Bloom, 2.4f, 6.0f, 64, 4, 0, 0, 0, FIX, kGhost, 1.8f, 0.05f),
	F(RE, FxForm_Vortex, 1.4f, 6.0f, 72, 4, 1.2f, 2.0f, -3.0f, FIX, kDusk, 2.2f, 0.05f));

LOOK(kAaImperator, "imperator's command",
	G(IM, Glyph_Crown, 0.9f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kCrimson, 3.0f),
	F(IM, FxForm_Rune, 1.8f, 0, 80, 8, 3, 16, 0, FIX, kCrimson, 3.0f, 0.05f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 64, 2.5f, 2.5f, 0, 0.8f, FIX, kGold, 2.6f, 0.05f, COMET));

LOOK(kAaMageHunter, "mark of the mage hunter",
	G(IM, Glyph_Pentagram, 0.8f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kArcane, 2.6f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kArcane, 3.0f, 0.04f),
	F(IM, FxForm_Sparkle, 1.4f, 7.0f, 40, 20, 0.12f, 2, 0, FIX, kMagic, 2.6f));

LOOK(kAaPressAttack, "press the attack",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.3f, 0.3f, 0.2f, FIX, kWarSteel, 0.4f, 0.20f),
	G(IM, Glyph_Shield, 1.0f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kWarSteel, 1.0f),
	F(IM, FxForm_Wind, 1.4f, 4.0f, 60, 12, 3.0f, 0, 0, FIX, kWarSteel, 1.2f),
	F(IM, FxForm_Burst, 2.0f, 4.0f, 48, 2, 0, 12, 0, FIX, kWhite, 0.6f, 0.09f));

LOOK(kAaProjection, "projection of fury",
	G(RE, Glyph_Helm, 1.2f, 6.5f, 4, 1, GA_Emerge, 0, FIX, kRage, 3.5f),
	F(RE, FxForm_Flame, 1.0f, 7.0f, 80, 20, 0.2f, 1.2f, 0, FIX, kRage, 3.5f, 0.06f),
	F(RE, FxForm_Rune, 2.4f, 0, 60, 6, 1, 12, 0, FIX, kCrimson, 3.5f),
	G(RE, Glyph_Fist, 0.35f, 4.0f, 0, 2, GA_Orbit, 1.3f, FIX, kCrimson, 3.5f, 0.4f));

LOOK(kAaRallosZek, "rage of rallos zek",
	F(IM, FxForm_Flame, 1.2f, 9.0f, 128, 32, 0.6f, 2.0f, 0, FIX, kFlame2, 3.0f, 0.10f),
	G(IM, Glyph_Fist, 1.4f, 10.0f, 4, 1, GA_PopBob, 0, FIX, kBlood, 3.0f),
	F(IM, FxForm_Lightning, 1, 36.0f, 48, 2, 0.8f, 10.0f, 1.0f, FIX, kRage, 1.0f, 0.10f),
	F(IM, FxForm_Burst, 3.2f, 0.3f, 96, 3, 0.4f, 20, 0, FIX, kRage, 1.2f, 0.12f));

LOOK(kAaRampage, "rampage",
	F(IM, FxForm_Crystal, 1.1f, 5.5f, 72, 14, 0, 0.8f, 0, FIX, kStone, 3.0f, 0.07f, OVER),
	G(IM, Glyph_Fist, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kRage, 3.0f),
	F(IM, FxForm_Burst, 2.6f, 0.3f, 72, 2, 0.4f, 16, 0, FIX, kRage, 1.0f, 0.10f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.2f, 0, 0, FIX, kRage, 2.0f, 0.06f));

LOOK(kAaGlory, "resplendent glory",
	G(IM, Glyph_Sun, 1.0f, 8.4f, 4, 1, GA_PopRoll, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Spire, 0.8f, 8.0f, 96, 16, 0.8f, 2.6f, 0, FIX, kHealGold, 2.4f, 0.06f),
	F(IM, FxForm_Sparkle, 1.4f, 8.0f, 48, 24, 0.12f, 1, 0, FIX, kHeal, 3.0f));

LOOK(kAaSpire, "spire of the warlord",
	F(IM, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kCrimson, 2.6f, 0.10f),
	G(IM, Glyph_Sword, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kWarSteel, 2.6f),
	F(IM, FxForm_Burst, 2.4f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kRage, 1.4f, 0.10f, ADD, 1, 0.35f),
	F(IM, FxForm_Rune, 2.0f, 0, 70, 6, 1, 12, 0, FIX, kCrimson, 3.0f, 0.05f, ADD, 1, 0.35f));

LOOK(kAaVehement, "vehement rage",
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.8f, 1.4f, 0, FIX, kBlood, 3.0f, 0.10f),
	F(IM, FxForm_Vortex, 1.3f, 7.0f, 96, 5, 1.2f, 2.5f, 5.0f, FIX, kRage, 3.0f, 0.06f),
	G(IM, Glyph_Fist, 0.4f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kBlood, 3.0f));

LOOK(kAaWarCry, "war cry",
	G(RE, Glyph_Horn, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.2f),
	F(RE, FxForm_Bloom, 1.0f, 5.0f, 96, 5, 0, 0, 0, FIX, kCrimson, 2.0f, 0.06f, ADD, 1.4f, 0, AE),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 1.5f, 20, 0, FIX, kGold, 1.4f, 0.10f, ADD, 1, 0.2f, AE),
	G(IM, Glyph_ArrowUp, 0.35f, 5.0f, 0, 3, GA_Orbit | GlyphAnim_Ascend, 1.2f, FIX, kGold, 2.2f));

LOOK(kAaBravery, "warlord's bravery",
	G(IM, Glyph_Helm, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kCrimson, 3.0f, 0.04f),
	F(IM, FxForm_Ring, 1.8f, 0.3f, 48, 0, 1.0f, 1.0f, 0.5f, FIX, kGold, 3.0f, 0.05f, COMET));

LOOK(kAaFury, "warlord's fury",
	F(RE, FxForm_Flame, 1.0f, 6.0f, 96, 24, 0.8f, 1.0f, 0, FIX, kCrimson, 2.4f, 0.08f),
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.4f, 0.4f, 0.8f, FIX, kCrimson, 0.7f, 0.16f),
	G(IM, Glyph_Banner, 0.9f, 8.0f, 4, 1, GA_Drop, 0, FIX, kRage, 2.4f),
	F(IM, FxForm_Burst, 2.2f, 0.4f, 64, 2, 0.6f, 12, 0, FIX, kRage, 0.9f, 0.09f, ADD, 1, 0.35f));

LOOK(kAaGrasp, "warlord's grasp",
	F(TR, FxForm_Drain, 1, 0, 64, 3, 0.8f, 12, 0, FIX, kCrimson, 1.4f, 0.07f),
	G(IM, Glyph_Shackle, 0.8f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kGunmetal, 2.4f),
	F(IM, FxForm_Vortex, 1.0f, 4.0f, 64, 4, 1.2f, 2.0f, -3.0f, FIX, kCrimson, 2.0f, 0.05f));

LOOK(kAaResurgence, "warlord's resurgence",
	G(IM, Glyph_Heart, 1.1f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kCrimson, 2.6f),
	F(IM, FxForm_Flame, 0.9f, 9.0f, 80, 20, 0.1f, 3.0f, 0, FIX, kVigor, 2.4f, 0.08f),
	F(IM, FxForm_Burst, 2.4f, 0.5f, 72, 2, 2.0f, 16, 0, FIX, kHeal, 1.2f, 0.09f));

LOOK(kAaTenacity, "warlord's tenacity",
	G(IM, Glyph_Heart, 0.5f, 4.5f, 3, 3, GA_RingRise, 1.3f, FIX, kBlood, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 7.0f, 96, 3, 2.0f, 0, 0.6f, FIX, kCrimson, 2.8f, 0.06f, COMET),
	F(IM, FxForm_Helix, 1.1f, 7.0f, 96, 3, -2.0f, 0, 0.6f, FIX, kGold, 2.8f, 0.05f, COMET),
	F(IM, FxForm_Burst, 2.4f, 0.5f, 64, 2, 1.5f, 12, 0, FIX, kCrimson, 1.0f, 0.09f));

LOOK(kAaHeroicBlade, "heroic blade",
	G(IM, Glyph_Sword, 1.0f, 8.0f, 4, 1, GA_Drop, 0, FIX, kGold, 0.9f),
	G(IM, Glyph_Sword, 1.0f, 8.0f, 4, 1, GA_Drop, 0, FIX, kWarSteel, 0.9f, 0.3f),
	G(IM, Glyph_Sword, 1.2f, 8.0f, 4, 1, GA_Drop, 0, FIX, kGold, 1.2f, 0.6f),
	F(IM, FxForm_Burst, 2.6f, 1.0f, 72, 3, 0.6f, 18, 0, FIX, kGold, 1.0f, 0.10f, ADD, 1, 0.6f));

LOOK(kAaProvocation, "glyph of inspired provocation",
	G(RE, Glyph_Gem, 1.0f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kCrimson, 3.0f),
	F(RE, FxForm_Rune, 2.0f, 0, 70, 6, 1, 12, 0, FIX, kCrimson, 3.0f),
	G(RE, Glyph_Mouth, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kJeer, 3.0f));

enum Line : int
{
	Runes, Stand, Defense, StormDefense, ShieldLine, ShieldTrigger, ShieldReflect, Bulwark, NoBleed,
	Defensive, Fortitude, Evasive, Resistant, Fearless, Wind, Breather, InnerRejuv, HealingWill,
	Onslaught, Offensive, Through, Weapon, FlashAnger, Reprisal, Charge, MightyStrike, Fellstrike,
	Furious, Precision, SpiritRage, Blades, ThroatJab, Strike, VitalStrike, Knuckle, ShieldBash,
	Taunt, Roar, Shout, Rally, Attention, Tongue, Attraction, Phantom, WadeIn, Expanse, HealPrecision,
	Field, Aura, Voice,
	AreaTaunt, BattleLeap, FuriousLeap, BladeGuardian, BlastOfAnger, BraceImpact, CallChallenge,
	Grapple, GutPunch, KneeStrike, Howl, Imperator, MageHunter, PressAttack, Projection, RallosZek,
	Rampage, Glory, Spire, Vehement, WarCry, Bravery, Fury, Grasp, Resurgence, Tenacity, HeroicBlade,
	Provocation,
};

const LineLook kLines[] = {
	{ "Armor of Runes (rune)",            &kWarRunes },
	{ "Stand (discipline)",               &kWarStand },
	{ "Defense (armor class)",            &kWarDefense },
	{ "Storm Defense (discipline)",       &kWarStormDefense },
	{ "Shield (mitigation, hate)",        &kWarShieldLine },
	{ "Shielding trigger",                &kWarShieldTrigger },
	{ "Shield Reflect",                   &kWarShieldReflect },
	{ "Warrior's Bulwark (hate, rune)",   &kWarBulwark },
	{ "No Time to Bleed (rune)",          &kWarNoBleed },
	{ "Defensive Discipline",             &kWarDefensive },
	{ "Fortitude Discipline",             &kWarFortitude },
	{ "Evasive Discipline",               &kWarEvasive },
	{ "Resistant Discipline",             &kWarResistant },
	{ "Fearless Discipline",              &kWarFearless },
	{ "Wind (endurance)",                 &kWarWind },
	{ "Breather (endurance)",             &kWarBreather },
	{ "Inner Rejuvenation (self heal)",   &kWarInnerRejuv },
	{ "Healing Will (regeneration)",      &kWarHealingWill },
	{ "Onslaught (discipline)",           &kWarOnslaught },
	{ "Offensive (discipline)",           &kWarOffensive },
	{ "Punch Through (accuracy)",         &kWarThrough },
	{ "Weapon Affiliation (procs)",       &kWarWeapon },
	{ "Flash of Anger (parry)",           &kWarFlashAnger },
	{ "Determined Reprisal (riposte)",    &kWarReprisal },
	{ "Charge Discipline",                &kWarCharge },
	{ "Mighty Strike Discipline",         &kWarMightyStrike },
	{ "Fellstrike Discipline",            &kWarFellstrike },
	{ "Furious Discipline",               &kWarFurious },
	{ "Precision Discipline",             &kWarPrecision },
	{ "Spirit of Rage Discipline",        &kWarSpiritRage },
	{ "Blades (area strike)",             &kWarBlades },
	{ "Throat Jab (strike)",              &kWarThroatJab },
	{ "Opportunistic Strike (finisher)",  &kWarStrike },
	{ "Vital Strike (finisher)",          &kWarVitalStrike },
	{ "Knuckle (strike, debuff)",         &kWarKnuckle },
	{ "Shield Topple (strike)",           &kWarShieldBash },
	{ "Insult (taunt)",                   &kWarTaunt },
	{ "Roar / Bellow (hate)",             &kWarRoar },
	{ "Shout (hate, debuff)",             &kWarShout },
	{ "Rally-Cry (area)",                 &kWarRally },
	{ "Attention (hate lock)",            &kWarAttention },
	{ "Tongue (taunt damage)",            &kWarTongue },
	{ "Forceful Attraction (pull)",       &kWarAttraction },
	{ "Phantom Aggressor",                &kWarPhantom },
	{ "Wade into Battle (area hate)",     &kWarWadeIn },
	{ "Expanse (heal to area hate)",      &kWarExpanse },
	{ "Precision (heal to hate)",         &kWarHealPrecision },
	{ "Field (group armor class)",        &kWarField },
	{ "Champion's Aura",                  &kWarAura },
	{ "Commanding Voice (group)",         &kWarVoice },

	{ "AA: Area Taunt",                   &kAaAreaTaunt },
	{ "AA: Battle Leap",                  &kAaBattleLeap },
	{ "AA: Furious Leap",                 &kAaFuriousLeap },
	{ "AA: Blade Guardian",               &kAaBladeGuardian },
	{ "AA: Blast of Anger",               &kAaBlastOfAnger },
	{ "AA: Brace for Impact",             &kAaBraceImpact },
	{ "AA: Call of Challenge",            &kAaCallChallenge },
	{ "AA: Grappling Strike",             &kAaGrapple },
	{ "AA: Gut Punch",                    &kAaGutPunch },
	{ "AA: Knee Strike",                  &kAaKneeStrike },
	{ "AA: Howl of the Warlord",          &kAaHowl },
	{ "AA: Imperator's Command",          &kAaImperator },
	{ "AA: Mark of the Mage Hunter",      &kAaMageHunter },
	{ "AA: Press the Attack",             &kAaPressAttack },
	{ "AA: Projection of Fury",           &kAaProjection },
	{ "AA: Rage of Rallos Zek",           &kAaRallosZek },
	{ "AA: Rampage",                      &kAaRampage },
	{ "AA: Resplendent Glory",            &kAaGlory },
	{ "AA: Spire of the Warlord",         &kAaSpire },
	{ "AA: Vehement Rage",                &kAaVehement },
	{ "AA: War Cry",                      &kAaWarCry },
	{ "AA: Warlord's Bravery",            &kAaBravery },
	{ "AA: Warlord's Fury",               &kAaFury },
	{ "AA: Warlord's Grasp",              &kAaGrasp },
	{ "AA: Warlord's Resurgence",         &kAaResurgence },
	{ "AA: Warlord's Tenacity",           &kAaTenacity },
	{ "AA: Wars Sheol's Heroic Blade",    &kAaHeroicBlade },
	{ "AA: Glyph of Inspired Provocation", &kAaProvocation },
};

const GroupLine kNumbered[] = {
	{ 0, Runes },        { 2, Stand },        { 4, Blades },       { 5, Taunt },        { 7, Roar },
	{ 8, ShieldReflect }, { 9, Strike },      { 10, Knuckle },     { 11, Through },     { 12, ShieldBash },
	{ 13, Rally },       { 14, Field },       { 15, Defense },     { 16, Weapon },      { 17, VitalStrike },
	{ 18, Attraction },  { 19, FlashAnger },  { 21, Attention },   { 22, Tongue },      { 23, Offensive },
	{ 24, Shout },       { 25, Bulwark },     { 26, StormDefense }, { 27, NoBleed },    { 28, Phantom },
	{ 29, Reprisal },    { 30, WadeIn },      { 31, ShieldLine },  { 32, ShieldTrigger }, { 33, Expanse },
	{ 34, HealPrecision },
};

const GroupLine kModern[] = {
	{ 40, Breather },    { 50, Runes },       { 70, Stand },       { 90, Blades },      { 100, Taunt },
	{ 110, Roar },       { 130, Strike },     { 160, ShieldBash }, { 180, Field },      { 190, Defense },
	{ 250, Attention },  { 260, Tongue },     { 280, Shout },      { 290, Bulwark },    { 310, NoBleed },
	{ 350, ShieldLine }, { 351, ShieldTrigger }, { 360, Expanse }, { 370, HealPrecision },
};

const GroupLine kOther[] = {
	{ 16000, Runes },    { 16002, Stand },    { 16003, ThroatJab }, { 16004, Blades },   { 16005, Taunt },
	{ 16008, Wind },     { 16009, Onslaught }, { 6192, Onslaught }, { 16909, Onslaught }, { 6173, Roar },
	{ 8467, Taunt },     { 36725, Blades },   { 36750, Blades },   { 3415, InnerRejuv },
	{ 3105, Wind },      { 3205, Wind },      { 3305, Wind },      { 3405, Wind },      { 3505, Wind },
	{ 3311, Breather },  { 3411, Breather },  { 3511, Breather },  { 3611, Breather },  { 3737, Breather },
	{ 3837, Breather },  { 3937, Breather },
};

const NamedLine kAbilities[] = {
	{ "defensive discipline",             Defensive },
	{ "stonewall discipline",             Stand },
	{ "fortitude discipline",             Fortitude },
	{ "evasive discipline",               Evasive },
	{ "shocking defense discipline",      StormDefense },
	{ "aura of runes discipline",         Runes },
	{ "aggressive discipline",            Offensive },
	{ "charge discipline",                Charge },
	{ "mighty strike discipline",         MightyStrike },
	{ "fellstrike discipline",            Fellstrike },
	{ "furious discipline",               Furious },
	{ "precision discipline",             Precision },
	{ "spirit of rage discipline",        SpiritRage },
	{ "provoke",                          Taunt },
	{ "berate",                           Taunt },
	{ "incite",                           Taunt },
	{ "mercenary taunt",                  Taunt },
	{ "bellow",                           Roar },
	{ "bellow of the mastruq",            Roar },
	{ "ancient: chaos cry",               Roar },
	{ "myrmidon's aura",                  Aura },
	{ "champion's aura",                  Aura },
	{ "commanding voice",                 Voice },

	{ "area taunt",                       AreaTaunt },
	{ "mercenary area taunt",             AreaTaunt },
	{ "battle leap",                      BattleLeap },
	{ "battle leap warcry",               BattleLeap },
	{ "furious leap",                     FuriousLeap },
	{ "blade guardian",                   BladeGuardian },
	{ "blade whirl",                      BladeGuardian },
	{ "blast of anger",                   BlastOfAnger },
	{ "brace for impact",                 BraceImpact },
	{ "impact guard",                     BraceImpact },
	{ "call of challenge",                CallChallenge },
	{ "call of challenge snare",          CallChallenge },
	{ "grappling strike",                 Grapple },
	{ "gut punch",                        GutPunch },
	{ "gut punch strike",                 GutPunch },
	{ "knee strike",                      KneeStrike },
	{ "knee strike snare",                KneeStrike },
	{ "howl of the warlord",              Howl },
	{ "imperator's command",              Imperator },
	{ "mark of the mage hunter",          MageHunter },
	{ "press the attack",                 PressAttack },
	{ "projection of fury",               Projection },
	{ "rage of rallos zek",               RallosZek },
	{ "rampage",                          Rampage },
	{ "resplendent glory",                Glory },
	{ "spire of the warlord",             Spire },
	{ "vehement rage",                    Vehement },
	{ "war cry",                          WarCry },
	{ "warlord's bravery",                Bravery },
	{ "warlord's fury",                   Fury },
	{ "warlord's greater fury",           Fury },
	{ "warlord's lesser fury",            Fury },
	{ "warlord's grasp",                  Grasp },
	{ "warlord's grasp taunt",            Grasp },
	{ "warlord's resurgence",             Resurgence },
	{ "warlord's tenacity",               Tenacity },
	{ "heroic blade",                     HeroicBlade },
	{ "glyph of inspired provocation",    Provocation },
};

int BySpell(const eqlib::EQ_Spell*, std::string_view base)
{
	if (base == "resistant discipline")
		return Resistant;
	if (base == "fearless discipline")
		return Fearless;
	if (base == "healing will discipline" || base == "focused will discipline")
		return HealingWill;
	return -1;
}

} // namespace

const ClassCatalogue& WarriorCatalogue()
{
	static const ClassCatalogue c{ "Warrior", eqlib::Warrior, kLines, 15, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
