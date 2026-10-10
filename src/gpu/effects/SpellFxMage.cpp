#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

#include <cstring>

namespace eqgfx::spellfx {

namespace {

constexpr Pair kMagma    { { 1.00f, 0.72f, 0.20f, 1.00f }, { 0.55f, 0.08f, 0.00f, 0.90f } };
constexpr Pair kSand     { { 0.95f, 0.82f, 0.55f, 0.95f }, { 0.60f, 0.45f, 0.20f, 0.85f } };
constexpr Pair kIceflame { { 0.70f, 0.95f, 1.00f, 0.95f }, { 1.00f, 0.40f, 0.10f, 0.85f } };
constexpr Pair kGarg     { { 0.62f, 0.60f, 0.56f, 0.95f }, { 0.30f, 0.28f, 0.26f, 0.90f } };
constexpr Pair kRunic    { { 0.60f, 0.85f, 1.00f, 0.95f }, { 0.35f, 0.20f, 0.95f, 0.85f } };
constexpr Pair kMonster  { { 0.80f, 0.45f, 1.00f, 0.95f }, { 0.20f, 0.05f, 0.30f, 0.85f } };
constexpr Pair kBlades   { { 1.00f, 1.00f, 1.00f, 1.00f }, { 0.60f, 0.70f, 0.85f, 0.90f } };

constexpr uint32_t GA_Emerge = GlyphAnim_Rise | GlyphAnim_Pop;

LOOK(kMagPetAir, "air elemental",
	F(RE, FxForm_Vortex, 2.2f, 10.0f, 128, 6, 1.6f, 3.0f, 5.0f, FIX, kAir, 4.0f, 0.08f),
	F(RE, FxForm_Wind, 1.6f, 6.0f, 72, 14, 3.0f, 0, 0, FIX, kAir, 3.5f, 0.05f),
	G(RE, Glyph_Tornado, 1.0f, 8.5f, 4, 1, GA_Emerge, 0, FIX, kAir, 3.5f, 0.4f),
	F(RE, FxForm_Sparkle, 2.4f, 9.0f, 60, 30, 0.14f, 2, 0, FIX, kWhite, 3.5f),
	F(RE, FxForm_Rune, 3.0f, 0, 70, 6, 1, 12, 0, FIX, kAir, 4.0f, 0.05f));

LOOK(kMagPetWater, "water elemental",
	F(RE, FxForm_Vortex, 1.8f, 8.0f, 112, 5, 1.2f, 2.5f, -4.0f, FIX, kWater, 4.0f, 0.08f),
	F(RE, FxForm_Bubbles, 2.0f, 8.0f, 112, 14, 0.3f, 0, 0, FIX, kWater, 4.0f, 0.04f),
	F(RE, FxForm_Rain, 2.4f, 10.0f, 60, 50, 0, 0.15f, 0, FIX, kWater, 2.0f, 0.06f, ADD, 1, 1.5f),
	G(RE, Glyph_Wave, 1.1f, 8.5f, 4, 1, GA_Emerge, 0, FIX, kWater, 3.5f, 0.4f),
	F(RE, FxForm_Bloom, 3.0f, 1.0f, 64, 3, 0, 0, 0, FIX, kWater, 3.0f, 0.06f));

LOOK(kMagPetFire, "fire elemental",
	F(RE, FxForm_Flame, 1.6f, 10.0f, 128, 32, 0.4f, 2.8f, 0, FIX, kFire, 4.0f, 0.12f),
	F(RE, FxForm_Burst, 4.0f, 0, 96, 3, 1.5f, 20, 0, FIX, kFire, 1.4f, 0.12f, ADD, 1, 0.3f),
	F(RE, FxForm_Sparkle, 2.4f, 10.0f, 60, 30, 0.14f, 0, 0, FIX, kMagma, 4.0f),
	G(RE, Glyph_Flame, 1.1f, 9.5f, 4, 1, GA_Emerge, 0, FIX, kFire, 3.5f, 0.4f),
	F(RE, FxForm_Cracks, 3.0f, 0, 48, 6, 1.2f, 0, 0, FIX, kMagma, 4.0f, 0.08f));

LOOK(kMagPetEarth, "earth elemental",
	F(RE, FxForm_Cracks, 4.0f, 0, 64, 8, 1.4f, 0, 0, FIX, kEarth, 4.5f, 0.10f, OVER),
	F(RE, FxForm_Crystal, 2.4f, 4.5f, 60, 12, 1, 1.2f, 0, FIX, kStone, 4.0f, 0.08f, OVER),
	G(RE, Glyph_Rock, 0.6f, 1.5f, 3, 6, GlyphAnim_Rise | GlyphAnim_Spin | GlyphAnim_Bob, 2.6f, FIX, kEarth, 4.0f, 0, OVER),
	F(RE, FxForm_Burst, 4.0f, 0, 64, 2, 0.5f, 0, 0, FIX, kSand, 1.4f, 0.12f, OVER),
	G(RE, Glyph_Rock, 1.4f, 5.0f, 4, 1, GA_Emerge, 0, FIX, kStone, 4.0f, 0.4f, OVER));

LOOK(kMagPetMonster, "monster summoning",
	G(RE, Glyph_Pentagram, 2.4f, 0.0f, 2, 1, GA_PopRoll, 0, FIX, kMonster, 4.0f),
	F(RE, FxForm_Vortex, 1.8f, 6.0f, 96, 5, 1.2f, 2.5f, -4.0f, FIX, kMonster, 4.0f, 0.07f, OVER),
	G(RE, Glyph_Claw, 1.1f, 5.0f, 4, 1, GA_Emerge, 0, FIX, kMonster, 3.5f, 0.5f),
	G(RE, Glyph_Eye, 0.4f, 7.5f, 0, 4, GA_Orbit, 1.6f, FIX, kMonster, 3.5f));

LOOK(kMagPetEpic, "manifest elements",
	F(RE, FxForm_Flame, 1.2f, 7.0f, 96, 24, 0.4f, 2.0f, 0, FIX, kFire, 4.0f, 0.09f),
	F(RE, FxForm_Bubbles, 1.6f, 7.0f, 64, 10, 0.25f, 0, 0, FIX, kWater, 4.0f, 0.04f),
	F(RE, FxForm_Wind, 1.6f, 6.0f, 60, 12, 3.0f, 0, 0, FIX, kAir, 4.0f),
	F(RE, FxForm_Crystal, 2.2f, 3.5f, 40, 8, 1, 1.0f, 0, FIX, kStone, 4.0f, 0.07f, OVER),
	F(RE, FxForm_Rune, 3.2f, 0, 80, 8, 3, 16, 0, FIX, kPrism, 4.0f, 0.05f));

LOOK(kMagServant, "gargoyle servant",
	G(IM, Glyph_Bat, 1.2f, 8.0f, 4, 1, GA_Drop, 0, FIX, kGarg, 2.4f, 0, OVER),
	F(IM, FxForm_Cracks, 2.4f, 0, 48, 6, 1.2f, 0, 0, FIX, kMagma, 3.0f, 0.07f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.4f, 0.3f, 64, 2, 0.3f, 12, 0, FIX, kGarg, 1.0f, 0.10f, OVER, 1, 0.35f));

LOOK(kMagRemoteServant, "remote servant",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.4f, 3.0f, FIX, kGarg, 0.8f, 0.18f, OVER),
	G(IM, Glyph_Bat, 1.2f, 8.0f, 4, 1, GA_Drop, 0, FIX, kGarg, 2.4f, 0, OVER),
	F(IM, FxForm_Cracks, 2.4f, 0, 48, 6, 1.2f, 0, 0, FIX, kMagma, 3.0f, 0.07f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.4f, 0.3f, 64, 2, 0.3f, 12, 0, FIX, kGarg, 1.0f, 0.10f, OVER, 1, 0.35f));

LOOK(kMagSummonServant, "runic servant",
	F(IM, FxForm_Rune, 1.6f, 3.2f, 70, 6, 1, 16, 1, FIX, kRunic, 3.0f, 0.06f),
	G(IM, Glyph_Sword, 1.0f, 3.0f, 4, 1, GA_Emerge, 0, FIX, kRunic, 3.0f, 0.3f),
	F(IM, FxForm_Sparkle, 1.4f, 6.0f, 40, 20, 0.12f, 2, 0, FIX, kRunic, 3.0f));

LOOK(kMagSummonMinion, "runic minion",
	F(IM, FxForm_Rune, 1.8f, 0, 70, 5, 2, 12, 0, FIX, kRunic, 3.0f, 0.06f),
	G(IM, Glyph_Gem, 0.6f, 3.0f, 3, 3, GA_RingRise, 1.4f, FIX, kRunic, 3.0f),
	F(IM, FxForm_Vortex, 1.2f, 4.0f, 64, 4, 1.2f, 2.0f, 3.0f, FIX, kRunic, 2.5f, 0.05f));

LOOK(kMagBodyguards, "earthen bodyguards",
	F(RE, FxForm_Crystal, 1.8f, 4.0f, 50, 10, 1, 1.0f, 0, FIX, kStone, 3.5f, 0.08f, OVER),
	G(RE, Glyph_Shield, 0.8f, 3.5f, 3, 3, GA_RingRise, 2.2f, FIX, kStone, 3.5f, 0, OVER),
	F(RE, FxForm_Cracks, 2.6f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 3.5f, 0.07f, OVER));

LOOK(kMagBodyguardProc, "bodyguard",
	G(IM, Glyph_Shield, 0.4f, 4.0f, 3, 3, GA_Orbit, 1.4f, FIX, kStone, 3.0f, 0, OVER),
	F(IM, FxForm_Dome, 1.6f, 6.4f, 88, 6, 2, 0, 0, FIX, kStone, 2.4f, 0.04f, OVER));

LOOK(kMagRageZomm, "rage of zomm",
	G(IM, Glyph_Eye, 0.6f, 5.0f, 3, 3, GA_RingRise, 1.8f, FIX, kMonster, 3.0f),
	F(IM, FxForm_Vortex, 1.3f, 4.0f, 64, 4, 1.2f, 2.0f, -3.0f, FIX, kMonster, 3.0f, 0.05f, OVER));

LOOK(kMagDecoy, "decoy",
	G(RE, Glyph_Mask, 0.9f, 6.5f, 4, 1, GA_Emerge, 0, FIX, kPrism, 3.0f),
	F(RE, FxForm_Bloom, 2.0f, 3.0f, 48, 3, 0, 0, 0, FIX, kPrism, 2.4f, 0.05f),
	G(RE, Glyph_Note, 0.35f, 6.0f, 0, 4, GA_Orbit, 1.4f, FIX, kPrism, 2.4f));

LOOK(kMagBurnout, "burnout",
	F(IM, FxForm_Flame, 1.0f, 5.5f, 72, 18, 0.8f, 0.9f, 0, FIX, kMagma, 2.6f, 0.07f),
	G(IM, Glyph_Lightning, 0.6f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kStar, 2.0f));

LOOK(kMagPetStance, "stance",
	G(IM, Glyph_Shield, 0.7f, 7.2f, 4, 1, GA_Emerge, 0, FIX, kStone, 2.6f, 0, OVER),
	F(IM, FxForm_Cracks, 1.8f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 2.4f, 0.06f, OVER));

LOOK(kMagRenewal, "renewal",
	G(IM, Glyph_Cross, 0.5f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 2.2f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 64, 2, 1.5f, 0, 0.5f, FIX, kHeal, 2.2f, 0.05f, COMET));

LOOK(kMagIceflame, "iceflame",
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.8f, 0.9f, 0, FIX, kIceflame, 3.0f, 0.07f),
	G(IM, Glyph_Snowflake, 0.35f, 5.5f, 0, 2, GA_Orbit, 1.2f, FIX, kCold, 3.0f),
	G(IM, Glyph_Flame, 0.35f, 5.5f, 0, 2, GA_Orbit, 1.2f, FIX, kFire, 3.0f));

LOOK(kMagAuspice, "auspice",
	F(IM, FxForm_Dome, 1.6f, 6.4f, 88, 6, 2, 0, 0, FIX, kRunic, 2.6f, 0.04f),
	G(IM, Glyph_Shield, 0.5f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kRunic, 2.6f));

LOOK(kMagPromised, "promised",
	G(IM, Glyph_Hourglass, 0.6f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kHeal, 3.0f),
	F(IM, FxForm_Ring, 1.2f, 3.0f, 40, 0, 1.0f, 1.0f, 0.4f, FIX, kHeal, 3.0f, 0.05f, COMET));

LOOK(kMagThaumatize, "thaumatize",
	G(IM, Glyph_Pentagram, 1.4f, 0.0f, 2, 1, GA_PopRoll, 0, FIX, kArcane, 2.4f),
	F(IM, FxForm_Burst, 1.8f, 2.0f, 48, 1, 1.5f, 12, 0, FIX, kArcane, 1.0f, 0.07f));

LOOK(kMagCompanion, "companion",
	G(IM, Glyph_Sun, 0.6f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kFire, 2.4f),
	G(IM, Glyph_Moon, 0.6f, 7.8f, 4, 1, GlyphAnim_Pop, 0, FIX, kCold, 2.4f, 0.3f),
	F(IM, FxForm_Ring, 1.2f, 3.0f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kPrism, 2.4f, 0.05f, COMET));

LOOK(kMagCoat, "magma coat",
	Persist(F(AU, FxForm_Flame, 1.10f, 5.8f, 120, 30, 0.9f, 1.0f, 0, FIX, kMagma, 0, 0.09f)),
	Persist(F(AU, FxForm_Sparkle, 1.30f, 7.0f, 24, 12, 0.10f, 0, 0, FIX, kFire, 0)),
	F(IM, FxForm_Burst, 3.0f, 0, 48, 1, 1.0f, 12, 0, FIX, kMagma, 0.8f, 0.10f));

LOOK(kMagCircleCoat, "circle of coats",
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 1.5f, 20, 0, FIX, kMagma, 1.4f, 0.12f, ADD, 1, 0, AE),
	Persist(F(AU, FxForm_Flame, 1.10f, 5.8f, 120, 30, 0.9f, 1.0f, 0, FIX, kMagma, 0, 0.09f)),
	Persist(F(AU, FxForm_Sparkle, 1.30f, 7.0f, 24, 12, 0.10f, 0, 0, FIX, kFire, 0)));

LOOK(kMagSkin, "searing skin",
	Persist(F(AU, FxForm_Ring, 1.1f, 3.0f, 40, 0, 1.5f, 1.0f, 0.6f, FIX, kFire, 0, 0.05f, COMET)),
	Persist(F(AU, FxForm_Orbit, 1.2f, 6.0f, 40, 10, 0, 0, 0, FIX, kMagma, 0, 0.04f, ADD, 2.0f)),
	F(IM, FxForm_Flame, 1.0f, 5.0f, 64, 16, 0.8f, 0.8f, 0, FIX, kFire, 1.4f, 0.07f));

LOOK(kMagFlames, "hungry flames",
	Persist(F(AU, FxForm_Flame, 1.1f, 5.5f, 96, 24, 0.9f, 0.9f, 0, FIX, kFlame2, 0, 0.08f)),
	G(IM, Glyph_Fang, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kFire, 2.2f));

LOOK(kMagVeil, "veil",
	F(IM, FxForm_Flame, 1.05f, 5.8f, 96, 24, 0.9f, 0.9f, 0, FIX, kFlame2, 3.0f, 0.08f, OVER),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.10f, 0, 0, FIX, kMagma, 3.0f));

LOOK(kMagShield, "shield",
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kArcane, 2.4f, 0.04f),
	G(IM, Glyph_Shield, 0.8f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kArcane, 2.4f),
	F(IM, FxForm_Rune, 1.8f, 0, 60, 6, 1, 12, 0, FIX, kArcane, 2.4f));

LOOK(kMagGuardian, "guardian",
	G(IM, Glyph_Gem, 0.7f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kArcane, 3.0f),
	F(IM, FxForm_Orbit, 1.3f, 6.0f, 48, 10, 0, 0, 0, FIX, kArcane, 3.0f, 0.04f, ADD, 2.0f));

LOOK(kMagShieldFate, "shield of fate",
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kPrism, 2.6f, 0.04f),
	G(IM, Glyph_Hourglass, 0.7f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kPrism, 2.6f));

LOOK(kMagMirror, "mirror",
	G(IM, Glyph_Shield, 0.9f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kSilver, 2.2f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0, 12, 0, FIX, kSilver, 1.0f, 0.06f, ADD | FxFlag_Reverse));

LOOK(kMagTwincast, "twincast",
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 2, FIX, kArcane, 2.6f, 0.05f),
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 2, FIX, kFire, 2.6f, 0.05f, ADD, -1.0f),
	G(IM, Glyph_Gem, 0.5f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kPrism, 2.6f));

LOOK(kMagChaoticBuff, "chaotic boon",
	G(IM, Glyph_Gem, 0.7f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kPrism, 2.6f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 48, 24, 0.12f, 0, 0, FIX, kPrism, 2.6f));

LOOK(kMagDistillect, "distillect",
	F(RE, FxForm_Rune, 2.6f, 0, 80, 8, 3, 24, 0, FIX, kArcane, 3.0f, 0.05f),
	G(RE, Glyph_Gem, 0.35f, 1.0f, 3, 6, GA_PopSpin, 2.4f, FIX, kArcane, 3.0f));

LOOK(kMagGather, "gather",
	F(IM, FxForm_Sparkle, 1.6f, 7.0f, 48, 24, 0.12f, 2, 0, FIX, kMana, 2.4f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, 3.0f, FIX, kMana, 2.2f, 0.05f),
	G(IM, Glyph_Gem, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kMana, 2.2f));

LOOK(kMagFirebound, "firebound",
	G(IM, Glyph_Flame, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kFire, 2.6f),
	G(IM, Glyph_Link, 0.3f, 4.5f, 0, 4, GA_Orbit, 1.2f, FIX, kMagma, 2.6f));

LOOK(kMagSummonItem, "summon item",
	F(IM, FxForm_Rune, 1.6f, 0, 60, 5, 2, 8, 0, FIX, kArcane, 2.0f),
	F(IM, FxForm_Sparkle, 1.4f, 5.0f, 40, 20, 0.12f, 2, 0, FIX, kGold, 2.0f),
	G(IM, Glyph_Gem, 0.5f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kGold, 1.8f, 0.4f));

LOOK(kMagCallHeroes, "call of the heroes",
	G(RE, Glyph_Door, 1.3f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 4.0f),
	F(RE, FxForm_Vortex, 2.0f, 8.0f, 96, 6, 0.6f, 3.0f, 5.0f, FIX, kGold, 4.0f, 0.07f));

LOOK(kMagEye, "eye of zomm",
	G(IM, Glyph_Eye, 1.1f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kMonster, 3.0f),
	F(IM, FxForm_Rune, 2.0f, 0, 60, 6, 1, 0, 0, FIX, kMonster, 3.0f));

LOOK(kMagBolt, "fire bolt",
	F(TR, FxForm_Bolt, 1, 0, 128, 5, 0.6f, 0.45f, 1.8f, FIX, kFire, 0.8f, 0.24f),
	F(IM, FxForm_Burst, 2.8f, 1.2f, 96, 3, 2.0f, 16, 0, FIX, kFire, 0.9f, 0.14f),
	F(IM, FxForm_Flame, 1.2f, 6.0f, 96, 24, 0.8f, 1.1f, 0, FIX, kFire, 1.4f, 0.09f),
	F(IM, FxForm_Sparkle, 1.6f, 6.0f, 40, 20, 0.12f, 1, 0, FIX, kMagma, 1.8f));

LOOK(kMagMeteor, "meteor",
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 0.5f, 0.5f, 6.0f, FIX, kMagma, 0.9f, 0.26f),
	G(IM, Glyph_Rock, 0.9f, 6.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kMagma, 0.8f),
	F(IM, FxForm_Cracks, 2.6f, 0, 64, 8, 1.4f, 0, 0, FIX, kMagma, 2.4f, 0.08f),
	F(IM, FxForm_Burst, 3.2f, 0.5f, 96, 3, 0.8f, 20, 0, FIX, kFire, 1.0f, 0.14f));

LOOK(kMagSpear, "spear of magma",
	F(TR, FxForm_Bolt, 1, 0, 128, 2, 0.1f, 1.0f, 0.3f, FIX, kMagma, 0.7f, 0.16f),
	F(IM, FxForm_Burst, 3.0f, 0.6f, 96, 3, 0.8f, 20, 0, FIX, kMagma, 1.0f, 0.14f),
	F(IM, FxForm_Cracks, 2.4f, 0, 64, 8, 1.2f, 0, 0, FIX, kMagma, 2.2f, 0.07f),
	F(IM, FxForm_Flame, 1.1f, 5.0f, 80, 20, 0.6f, 0.9f, 0, FIX, kMagma, 1.6f, 0.08f));

LOOK(kMagShockSteel, "shock of steel",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.3f, 0.4f, 0.8f, FIX, kBlades, 0.7f, 0.12f),
	G(IM, Glyph_Sword, 0.7f, 7.0f, 4, 4, GA_Rain, 1.0f, FIX, kBlades, 1.4f),
	F(IM, FxForm_Burst, 2.0f, 3.0f, 48, 1, 0.3f, 16, 0, FIX, kSilver, 0.7f, 0.08f, ADD, 1, 0.35f));

LOOK(kMagMany, "of many",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.5f, 0.35f, 1.2f, FIX, kArcane, 0.8f, 0.16f),
	F(IM, FxForm_Burst, 2.0f, 2.0f, 48, 1, 1.5f, 10, 0, FIX, kArcane, 0.6f, 0.08f),
	F(IM, FxForm_Burst, 2.4f, 2.5f, 48, 1, 1.5f, 12, 0, FIX, kMagic, 0.6f, 0.09f, ADD, 1, 0.3f),
	F(IM, FxForm_Burst, 2.8f, 3.0f, 64, 2, 1.5f, 16, 0, FIX, kPrism, 0.7f, 0.10f, ADD, 1, 0.6f));

LOOK(kMagSands, "scorching sands",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.8f, 0.5f, 1.0f, FIX, kSand, 0.8f, 0.20f),
	F(IM, FxForm_Vortex, 1.3f, 5.0f, 96, 5, 1.2f, 2.5f, 5.0f, FIX, kSand, 1.8f, 0.06f),
	F(IM, FxForm_Flame, 1.0f, 2.5f, 64, 16, 0.3f, 0.6f, 0, FIX, kFire, 1.4f, 0.06f));

LOOK(kMagSandStrike, "strike of sand",
	F(TR, FxForm_Bolt, 1, 0, 96, 5, 0.9f, 0.6f, 1.0f, FIX, kSand, 0.8f, 0.22f, OVER),
	F(IM, FxForm_Burst, 2.6f, 0.4f, 72, 2, 0.4f, 12, 0, FIX, kSand, 1.0f, 0.12f, OVER),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 1.8f, 0.06f, OVER));

LOOK(kMagFickle, "fickle fire",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.8f, 0.4f, 2.0f, FIX, kFire, 0.8f, 0.18f),
	F(IM, FxForm_Burst, 2.4f, 1.5f, 72, 2, 1.5f, 12, 0, FIX, kPrism, 0.9f, 0.10f),
	F(IM, FxForm_Sparkle, 1.4f, 6.0f, 40, 20, 0.12f, 0, 0, FIX, kFire, 1.4f));

LOOK(kMagChaoticFire, "chaotic fire",
	F(IM, FxForm_Lightning, 1, 36.0f, 64, 3, 0.8f, 12.0f, 1.0f, FIX, kFire, 0.8f, 0.14f),
	F(IM, FxForm_Burst, 3.0f, 1.0f, 96, 3, 1.5f, 20, 0, FIX, kPrism, 1.0f, 0.12f, ADD, 1, 0.3f),
	F(IM, FxForm_Flame, 1.1f, 7.0f, 96, 24, 0.5f, 1.6f, 0, FIX, kFire, 1.6f, 0.09f, ADD, 1, 0.3f));

LOOK(kMagRainMolten, "rain of molten rock",
	F(IM, FxForm_Rain, 1.0f, 25.0f, 80, 80, 0, 0.3f, 0, FIX, kMagma, 3.0f, 0.10f, ADD, 1, 0, AE),
	G(IM, Glyph_Rock, 0.4f, 12.0f, 4, 6, GA_Rain, 3.0f, FIX, kMagma, 2.4f),
	F(IM, FxForm_Burst, 1.0f, 0.3f, 96, 3, 0.5f, 0, 0, FIX, kFire, 1.2f, 0.10f, ADD, 1, 1.2f, AE));

LOOK(kMagRainBlades, "rain of blades",
	F(IM, FxForm_Rain, 1.0f, 25.0f, 80, 60, 0, 0.1f, 0, FIX, kBlades, 3.0f, 0.06f, ADD, 1, 0, AE),
	G(IM, Glyph_Sword, 0.45f, 12.0f, 4, 6, GA_Rain, 3.0f, FIX, kBlades, 2.4f),
	F(IM, FxForm_Burst, 1.0f, 0.3f, 96, 2, 0.3f, 24, 0, FIX, kSilver, 1.0f, 0.08f, ADD, 1, 1.2f, AE));

LOOK(kMagSplash, "splash of lava",
	G(IM, Glyph_Drop, 1.0f, 9.0f, 4, 1, GA_Drop, 0, FIX, kMagma, 1.4f),
	F(IM, FxForm_Burst, 1.0f, 1.0f, 96, 2, 2.0f, 20, 0, FIX, kMagma, 1.2f, 0.10f, ADD, 1, 0.45f, AE),
	F(IM, FxForm_Bubbles, 0.6f, 4.0f, 96, 14, 0.25f, 0, 0, FIX, kFire, 2.0f, 0.04f, ADD, 1, 0.45f, AE));

LOOK(kMagBlast, "scalding blast",
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 1.5f, 24, 0, FIX, kFire, 1.4f, 0.14f, ADD, 1, 0, AE),
	F(RE, FxForm_Flame, 0.8f, 3.0f, 128, 32, 0.15f, 1.0f, 0, FIX, kMagma, 1.8f, 0.09f, ADD, 1, 0.1f, AE),
	F(RE, FxForm_Wind, 1.4f, 4.0f, 60, 12, 2.5f, 0, 0, FIX, kFire, 1.4f));

LOOK(kMagBeamMolten, "beam of molten rock",
	F(IM, FxForm_Lightning, 1, 0, 64, 4, 0.3f, 20.0f, 0, FIX, kMagma, 0.9f, 0.24f),
	F(IM, FxForm_Burst, 1.5f, 1.0f, 48, 1, 0.5f, 8, 0, FIX, kFire, 0.6f));

LOOK(kMagBeamBlades, "beam of blades",
	F(IM, FxForm_Lightning, 1, 0, 64, 3, 0.6f, 24.0f, 0, FIX, kBlades, 0.8f, 0.16f),
	G(RE, Glyph_Sword, 0.4f, 5.0f, 0, 5, GA_Orbit, 1.2f, FIX, kBlades, 1.2f));

LOOK(kMagSymbiosis, "symbiosis",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.2f, 12, 0, FIX, kArcane, 1.8f, 0.08f),
	F(IM, FxForm_Burst, 1.8f, 2.0f, 48, 1, 1.0f, 10, 0, FIX, kArcane, 0.8f, 0.08f));

LOOK(kMagSurge, "surge",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.4f, 1.0f, FIX, kCorrupt, 0.8f, 0.16f, OVER),
	F(IM, FxForm_Bloom, 1.8f, 2.0f, 48, 3, 0, 0, 0, FIX, kCorrupt, 2.0f, 0.05f, OVER));

LOOK(kMagUnnatural, "annihilate the unnatural",
	G(IM, Glyph_Pentagram, 1.2f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kWhite, 1.0f),
	F(IM, FxForm_Lightning, 1, 36.0f, 48, 2, 0.5f, 12.0f, 1.0f, FIX, kWhite, 0.7f, 0.12f, ADD, 1, 0.3f),
	F(IM, FxForm_Burst, 2.4f, 2.0f, 72, 2, 0, 16, 0, FIX, kArcane, 0.9f, 0.10f, ADD | FxFlag_Reverse, 1, 0.3f));

LOOK(kMagMalo, "malo",
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 5, 2, 8, 1, FIX, kPrism, 2.4f, 0.05f),
	G(IM, Glyph_ArrowDown, 0.3f, 6.5f, 0, 4, GA_Orbit, 1.2f, FIX, kPrism, 2.6f));

LOOK(kMagProcHeal, "pet mending",
	G(IM, Glyph_Cross, 0.3f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 1.4f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 20, 10, 0.10f, 0, 0, FIX, kHeal, 1.2f));

LOOK(kAaHost, "host of the elements",
	G(IM, Glyph_Flame, 0.6f, 3.5f, 3, 2, GA_RingRise, 2.2f, FIX, kFire, 4.0f),
	G(IM, Glyph_Wave, 0.6f, 3.5f, 3, 2, GA_RingRise, 2.2f, FIX, kWater, 4.0f),
	G(IM, Glyph_Tornado, 0.6f, 3.5f, 3, 2, GA_RingRise, 2.2f, FIX, kAir, 4.0f),
	G(IM, Glyph_Rock, 0.6f, 3.5f, 3, 2, GA_RingRise, 2.2f, FIX, kStone, 4.0f, 0, OVER),
	F(IM, FxForm_Rune, 2.6f, 0, 80, 8, 3, 16, 0, FIX, kPrism, 4.0f, 0.05f));

LOOK(kAaServantRo, "servant of ro",
	F(IM, FxForm_Flame, 1.2f, 9.0f, 112, 28, 0.3f, 2.4f, 0, FIX, kFire, 3.5f, 0.10f),
	G(IM, Glyph_Sun, 1.0f, 8.0f, 4, 1, GA_Emerge, 0, FIX, kStar, 3.5f, 0.3f),
	F(IM, FxForm_Burst, 3.0f, 0.4f, 96, 3, 1.0f, 20, 0, FIX, kFire, 1.2f, 0.12f));

LOOK(kAaNecessity, "companion of necessity",
	G(RE, Glyph_Spirit, 1.0f, 3.0f, 4, 1, GA_Emerge, 0, FIX, kPrism, 3.0f),
	F(RE, FxForm_Bloom, 2.0f, 2.0f, 48, 3, 0, 0, 0, FIX, kPrism, 2.4f, 0.05f));

LOOK(kAaHeartSkyfire, "heart of skyfire",
	G(IM, Glyph_Heart, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kFire, 4.0f),
	F(IM, FxForm_Flame, 1.05f, 6.0f, 112, 28, 0.9f, 1.0f, 0, FIX, kFire, 4.0f, 0.09f),
	F(IM, FxForm_Sparkle, 1.4f, 9.0f, 48, 24, 0.14f, 0, 0, FIX, kStar, 4.0f));

LOOK(kAaHeartFroststone, "heart of froststone",
	G(IM, Glyph_Heart, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kCold, 4.0f),
	F(IM, FxForm_Crystal, 1.2f, 5.5f, 60, 12, 0, 1.0f, 0, FIX, kCold, 4.0f, 0.06f),
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kStone, 3.0f, 0.04f));

LOOK(kAaForceElements, "force of elements",
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 0.6f, 0.5f, 1.5f, FIX, kPrism, 0.8f, 0.24f),
	F(IM, FxForm_Burst, 3.0f, 1.5f, 96, 3, 2.0f, 20, 0, FIX, kPrism, 1.0f, 0.14f),
	G(IM, Glyph_Flame, 0.4f, 5.0f, 3, 2, GA_RingRise, 1.4f, FIX, kFire, 1.6f),
	G(IM, Glyph_Snowflake, 0.4f, 5.0f, 3, 2, GA_RingRise, 1.4f, FIX, kCold, 1.6f));

LOOK(kAaFrenziedBurnout, "frenzied burnout",
	F(IM, FxForm_Flame, 1.2f, 7.0f, 112, 28, 0.8f, 1.4f, 0, FIX, kMagma, 3.5f, 0.11f),
	G(IM, Glyph_Lightning, 0.5f, 5.0f, 3, 3, GA_Orbit, 1.4f, FIX, kStar, 3.5f));

LOOK(kAaThaumaturgeFocus, "thaumaturge's focus",
	G(IM, Glyph_Eye, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kArcane, 3.0f),
	F(IM, FxForm_Rune, 1.6f, 3.2f, 70, 8, 3, 16, 2, FIX, kArcane, 3.0f, 0.05f));

LOOK(kAaThaumaturgeUnity, "thaumaturge's unity",
	G(IM, Glyph_Gem, 0.4f, 4.5f, 3, 4, GA_Orbit, 1.4f, FIX, kPrism, 3.0f),
	F(IM, FxForm_Burst, 1.8f, 2.0f, 48, 1, 1.5f, 12, 0, FIX, kArcane, 0.9f, 0.07f));

LOOK(kAaShieldElements, "shield of the elements",
	F(IM, FxForm_Dome, 2.0f, 7.4f, 128, 10, 4, 0, 0, FIX, kPrism, 3.5f, 0.06f),
	G(IM, Glyph_Shield, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kPrism, 3.5f));

LOOK(kAaTurnSummoned, "turn summoned",
	G(IM, Glyph_Pentagram, 1.8f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kWhite, 1.6f),
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 3, 0.4f, 12.0f, 1.0f, FIX, kWhite, 0.9f, 0.16f, ADD, 1, 0.4f),
	F(IM, FxForm_Burst, 3.4f, 1.0f, 120, 3, 1.0f, 24, 0, FIX, kArcane, 1.2f, 0.12f, ADD, 1, 0.4f));

LOOK(kAaSpireElements, "spire of the elements",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kPrism, 2.6f, 0.10f),
	G(RE, Glyph_Gem, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kPrism, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kPrism, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaConversion, "elemental conversion",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.2f, 16, 0, FIX, kMana, 2.0f, 0.08f),
	G(IM, Glyph_Gem, 0.6f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kMana, 2.0f));

LOOK(kAaArrow, "elemental arrow",
	F(TR, FxForm_Bolt, 1, 0, 128, 2, 0.1f, 1.0f, 0.4f, FIX, kPrism, 0.7f, 0.14f),
	F(IM, FxForm_Rune, 1.4f, 3.4f, 60, 5, 2, 8, 1, FIX, kPrism, 2.0f, 0.05f));

LOOK(kAaTalon, "virulent talon",
	G(IM, Glyph_Claw, 0.8f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kPoison, 2.4f),
	F(IM, FxForm_Bubbles, 1.1f, 5.5f, 48, 8, 0.2f, 0, 0, FIX, kPoison, 2.4f, 0.04f));

LOOK(kAaWhisper, "arcane whisper",
	G(IM, Glyph_Mouth, 0.6f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kArcane, 2.0f),
	F(IM, FxForm_Bloom, 1.6f, 2.0f, 40, 2, 0, 0, 0, FIX, kArcane, 2.0f, 0.04f, ADD | FxFlag_Reverse));

LOOK(kAaPyromancy, "pyromancy",
	G(IM, Glyph_Flame, 0.8f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kFire, 1.2f),
	F(IM, FxForm_Burst, 2.2f, 2.0f, 64, 2, 1.5f, 12, 0, FIX, kFire, 0.8f, 0.10f));

LOOK(kAaDimensionalShift, "dimensional shift",
	F(RE, FxForm_Vortex, 1.6f, 6.0f, 96, 5, 0.8f, 2.5f, 5.0f, FIX, kArcane, 1.2f, 0.06f),
	F(RE, FxForm_Sparkle, 1.6f, 6.0f, 40, 20, 0.12f, 2, 0, FIX, kArcane, 1.2f));

LOOK(kAaInstability, "dimensional instability",
	F(IM, FxForm_Vortex, 1.3f, 5.0f, 64, 4, 1.2f, 2.0f, -4.0f, FIX, kPrism, 3.0f, 0.05f),
	G(IM, Glyph_Spiral, 0.7f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kPrism, 3.0f));

LOOK(kAaArcaneDestruction, "arcane destruction",
	G(IM, Glyph_Star, 1.0f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kArcane, 3.5f),
	F(IM, FxForm_Flame, 1.05f, 6.0f, 112, 28, 0.9f, 1.0f, 0, FIX, kArcane, 3.5f, 0.09f));

LOOK(kAaDrapeShadows, "drape of shadows",
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kShadow, 2.5f, 0.05f, OVER),
	F(IM, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kShadow, 2.0f, 0.05f, OVER | FxFlag_Reverse));

LOOK(kAaHostShell, "host in the shell",
	G(IM, Glyph_Pentagram, 0.9f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kEarth, 3.0f),
	F(IM, FxForm_Crystal, 1.0f, 5.5f, 60, 10, 1, 0.8f, 0, FIX, kStone, 3.0f, 0.06f, OVER),
	F(IM, FxForm_Dome, 1.6f, 6.4f, 96, 6, 2, 0, 0, FIX, kEarth, 3.0f, 0.04f));

LOOK(kAaSecondWindWard, "second wind ward",
	G(IM, Glyph_Tornado, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kAir, 3.0f),
	F(IM, FxForm_Wind, 1.3f, 6.0f, 64, 14, 1.6f, 0, 0, FIX, kAir, 3.0f, 0.04f),
	F(IM, FxForm_Ring, 1.4f, 3.0f, 48, 0, 2.0f, 1.0f, 0.5f, FIX, kArcane, 3.0f, 0.05f, COMET));

LOOK(kAaModulationShard, "summon modulation shard",
	G(RE, Glyph_Gem, 0.6f, 5.0f, 4, 1, GA_PopRoll, 0, FIX, kMana, 2.6f),
	F(RE, FxForm_Rune, 1.4f, 0, 60, 6, 1, 12, 0, FIX, kMana, 2.6f),
	F(RE, FxForm_Crystal, 0.6f, 4.0f, 40, 6, 1, 0.5f, 0, FIX, kArcane, 2.4f, 0.05f));

LOOK(kAaVelocity, "velocity",
	F(IM, FxForm_Wind, 1.3f, 5.5f, 80, 18, 2.2f, 0, 0, FIX, kMagic, 2.4f, 0.04f),
	G(IM, Glyph_Lightning, 0.6f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kShock, 2.0f),
	F(IM, FxForm_Ring, 1.2f, 0.4f, 48, 0, 3.0f, 1.0f, 1.2f, FIX, kMagic, 2.0f, 0.05f, COMET));

enum Line : int
{
	PetAir, PetWater, PetFire, PetEarth, PetMonster, PetEpic, Servant, RemoteServant, SummonServant,
	SummonMinion, Bodyguards, BodyguardProc, RageZomm, Decoy, Burnout, PetStance, Renewal, Iceflame,
	Auspice, Promised, Thaumatize, Companion, Coat, CircleCoat, Skin, Flames, Veil, Shield, Guardian,
	ShieldFate, Mirror, Twincast, ChaoticBuff, Distillect, Gather, Firebound, SummonItem, CallHeroes,
	Eye, Bolt, Meteor, Spear, ShockSteel, Many, Sands, SandStrike, Fickle, ChaoticFire, RainMolten,
	RainBlades, Splash, Blast, BeamMolten, BeamBlades, Symbiosis, Surge, Unnatural, Malo, ProcHeal,
	Host, ServantRo, Necessity, HeartSkyfire, HeartFroststone, ForceElements, FrenziedBurnout,
	ThaumaturgeFocus, ThaumaturgeUnity, ShieldElements, TurnSummoned, SpireElements, Conversion, Arrow,
	Talon, Whisper, Pyromancy, DimensionalShift, Instability, ArcaneDestruction, DrapeShadows,
	HostShell, SecondWindWard, ModulationShard, Velocity,
};

const LineLook kLines[] = {
	{ "Air elemental (pet)",              &kMagPetAir },
	{ "Water elemental (pet)",            &kMagPetWater },
	{ "Fire elemental (pet)",             &kMagPetFire },
	{ "Earth elemental (pet)",            &kMagPetEarth },
	{ "Monster Summoning (pet)",          &kMagPetMonster },
	{ "Manifest Elements (pet)",          &kMagPetEpic },
	{ "Servant (gargoyle swarm)",         &kMagServant },
	{ "Remote Servant (gargoyle swarm)",  &kMagRemoteServant },
	{ "Summon Servant (runic swarm)",     &kMagSummonServant },
	{ "Summon Minion (runic swarm)",      &kMagSummonMinion },
	{ "Bodyguards (earthen swarm)",       &kMagBodyguards },
	{ "Bodyguard (defensive proc)",       &kMagBodyguardProc },
	{ "Rage of Zomm (swarm)",             &kMagRageZomm },
	{ "Deafening Decoy (swarm)",          &kMagDecoy },
	{ "Burnout (pet haste)",              &kMagBurnout },
	{ "Stance (pet mitigation)",          &kMagPetStance },
	{ "Renewal (pet heal)",               &kMagRenewal },
	{ "Iceflame (pet proc)",              &kMagIceflame },
	{ "Auspice / Aegis (pet ward)",       &kMagAuspice },
	{ "Promised (pet heal)",              &kMagPromised },
	{ "Thaumatize Pet",                   &kMagThaumatize },
	{ "Companion (pet)",                  &kMagCompanion },
	{ "Coat (damage shield)",             &kMagCoat },
	{ "Circle (group damage shield)",     &kMagCircleCoat },
	{ "Skin (damage shield)",             &kMagSkin },
	{ "Hungry Flames (damage shield)",    &kMagFlames },
	{ "Veil (defensive proc)",            &kMagVeil },
	{ "Shield (hit points)",              &kMagShield },
	{ "Guardian (regen)",                 &kMagGuardian },
	{ "Shield of Fate",                   &kMagShieldFate },
	{ "Mirror (reflect)",                 &kMagMirror },
	{ "Twincast",                         &kMagTwincast },
	{ "Chaotic (buff)",                   &kMagChaoticBuff },
	{ "Distillect (aura)",                &kMagDistillect },
	{ "Gather (mana)",                    &kMagGather },
	{ "Firebound",                        &kMagFirebound },
	{ "Summon item",                      &kMagSummonItem },
	{ "Call of the Heroes",               &kMagCallHeroes },
	{ "Eye of Zomm",                      &kMagEye },
	{ "Bolt (fire nuke)",                 &kMagBolt },
	{ "Meteor bolt (nuke)",               &kMagMeteor },
	{ "Spear (nuke)",                     &kMagSpear },
	{ "Shock of steel (nuke)",            &kMagShockSteel },
	{ "Of Many (nuke)",                   &kMagMany },
	{ "Sands (fire nuke)",                &kMagSands },
	{ "Strike of Sand (nuke)",            &kMagSandStrike },
	{ "Fickle (nuke)",                    &kMagFickle },
	{ "Chaotic Fire (nuke)",              &kMagChaoticFire },
	{ "Rain of molten rock",              &kMagRainMolten },
	{ "Rain of blades",                   &kMagRainBlades },
	{ "Splash of lava",                   &kMagSplash },
	{ "Blast (area)",                     &kMagBlast },
	{ "Beam of molten rock",              &kMagBeamMolten },
	{ "Beam of blades",                   &kMagBeamBlades },
	{ "Symbiosis",                        &kMagSymbiosis },
	{ "Surge",                            &kMagSurge },
	{ "Unnatural (anti-summoned)",        &kMagUnnatural },
	{ "Malosini (resist debuff)",         &kMagMalo },
	{ "pet mending proc",                 &kMagProcHeal },

	{ "AA: Host of the Elements",         &kAaHost },
	{ "AA: Servant of Ro",                &kAaServantRo },
	{ "AA: Companion of Necessity",       &kAaNecessity },
	{ "AA: Heart of Skyfire",             &kAaHeartSkyfire },
	{ "AA: Heart of Froststone",          &kAaHeartFroststone },
	{ "AA: Force of Elements",            &kAaForceElements },
	{ "AA: Frenzied Burnout",             &kAaFrenziedBurnout },
	{ "AA: Thaumaturge's Focus",          &kAaThaumaturgeFocus },
	{ "AA: Thaumaturge's Unity",          &kAaThaumaturgeUnity },
	{ "AA: Shield of the Elements",       &kAaShieldElements },
	{ "AA: Turn Summoned",                &kAaTurnSummoned },
	{ "AA: Spire of the Elements",        &kAaSpireElements },
	{ "AA: Elemental Conversion",         &kAaConversion },
	{ "AA: Elemental Arrow",              &kAaArrow },
	{ "AA: Virulent Talon",               &kAaTalon },
	{ "AA: Arcane Whisper",               &kAaWhisper },
	{ "AA: Pyromancy",                    &kAaPyromancy },
	{ "AA: Dimensional Shift",            &kAaDimensionalShift },
	{ "AA: Dimensional Instability",      &kAaInstability },
	{ "AA: Arcane Destruction",           &kAaArcaneDestruction },
	{ "AA: Drape of Shadows",             &kAaDrapeShadows },
	{ "AA: Host in the Shell",        &kAaHostShell },
	{ "AA: Second Wind Ward",         &kAaSecondWindWard },
	{ "AA: Summon Modulation Shard",  &kAaModulationShard },
	{ "AA: Velocity",                 &kAaVelocity },
};

const GroupLine kNumbered[] = {
	{ 0, Malo },        { 1, Coat },        { 2, SummonItem },  { 3, Shield },      { 4, Bolt },
	{ 5, SummonItem },  { 6, RainMolten },  { 7, Guardian },    { 8, ShockSteel },  { 9, Skin },
	{ 10, PetStance },  { 11, Burnout },    { 12, Sands },      { 13, Renewal },    { 15, CircleCoat },
	{ 16, CircleCoat },
	{ 17, RainBlades }, { 18, Fickle },     { 19, Symbiosis },  { 20, SummonItem }, { 21, Flames },
	{ 22, BeamMolten }, { 23, Iceflame },   { 25, Servant },    { 26, Many },       { 27, Veil },
	{ 28, Twincast },   { 29, BodyguardProc }, { 30, Distillect }, { 31, ChaoticBuff }, { 32, Thaumatize },
	{ 33, Surge },      { 34, SandStrike }, { 35, Splash },     { 36, Blast },      { 37, Gather },
	{ 38, Spear },      { 39, Bodyguards }, { 40, BeamBlades }, { 41, SummonServant }, { 42, SummonMinion },
	{ 43, Auspice },    { 44, Malo },       { 45, Iceflame },   { 46, Meteor },     { 47, CallHeroes },
	{ 48, SummonItem }, { 50, Companion },  { 52, SummonItem }, { 53, Firebound },  { 54, SummonItem },
	{ 55, ChaoticFire },
};

const GroupLine kModern[] = {
	{ 10, Malo },       { 20, Coat },       { 50, Bolt },       { 60, SummonItem }, { 70, RainMolten },
	{ 80, Guardian },   { 90, ShockSteel }, { 100, Skin },      { 110, PetStance }, { 120, Burnout },
	{ 130, Sands },     { 140, Renewal },   { 160, CircleCoat }, { 170, RainBlades }, { 190, Symbiosis },
	{ 220, BeamMolten }, { 230, Iceflame }, { 240, RemoteServant }, { 250, Servant }, { 260, Many },
	{ 270, Veil },      { 290, BodyguardProc }, { 310, ChaoticBuff }, { 340, SandStrike }, { 360, Blast },
	{ 370, Gather },    { 380, Spear },     { 390, Bodyguards }, { 400, BeamBlades }, { 410, SummonServant },
	{ 420, SummonMinion }, { 430, Auspice }, { 440, Malo },     { 460, Meteor },    { 490, Companion },
	{ 500, SummonItem }, { 510, Firebound }, { 520, SummonItem }, { 530, ChaoticFire },
};

const GroupLine kOther[] = {
	{ 11000, Coat },     { 11001, Shield },   { 11002, Bolt },     { 11004, RainMolten }, { 11005, Guardian },
	{ 11006, ShockSteel }, { 11007, Skin },   { 11009, PetStance }, { 11010, Burnout },  { 11011, Sands },
	{ 11012, Unnatural }, { 11013, Renewal }, { 11014, Auspice },  { 11015, SummonItem }, { 11016, CircleCoat },
	{ 11017, RainBlades }, { 11018, Fickle }, { 11019, Symbiosis }, { 11021, SummonItem }, { 11022, PetStance },
	{ 11023, BeamMolten }, { 11024, Iceflame }, { 11025, Servant }, { 18037, Servant },  { 18040, Fickle },
	{ 29962, Fickle },   { 27925, RageZomm }, { 16133, Eye },      { 3006, Malo },       { 1599, Mirror },
	{ 5491, Renewal },   { 5492, Skin },      { 5496, Meteor },
	{ 5130, Unnatural }, { 5230, Unnatural }, { 5330, Unnatural }, { 5430, Unnatural },  { 5530, Unnatural },
	{ 5630, Unnatural }, { 5730, Unnatural }, { 5830, Unnatural }, { 5930, Unnatural },  { 125060250, Unnatural },
	{ 20115, Unnatural }, { 20116, Unnatural }, { 20248, Unnatural }, { 20663, Unnatural }, { 20664, Unnatural },
	{ 21529, Unnatural }, { 23238, Unnatural }, { 25490, Unnatural },
	{ 6202, Shield },    { 6302, Shield },    { 6402, Shield },    { 6502, Shield },     { 6602, Shield },
	{ 6702, Shield },    { 6802, Shield },    { 6902, Shield },    { 125140030, Shield },
	{ 9124, Auspice },   { 9524, Auspice },   { 9624, Auspice },   { 9724, Auspice },    { 9824, Auspice },
	{ 9924, Auspice },   { 125110190, Auspice },
	{ 9554, ShieldFate }, { 9654, ShieldFate }, { 9754, ShieldFate }, { 9854, ShieldFate }, { 9954, ShieldFate },
	{ 125110480, ShieldFate },
	{ 2105, Promised },  { 2205, Promised },  { 2305, Promised },  { 2405, Promised },   { 2505, Promised },
	{ 2605, Promised },  { 2705, Promised },  { 2805, Promised },  { 2905, Promised },   { 125150060, Promised },
	{ 2706, ProcHeal },  { 2806, ProcHeal },  { 2906, ProcHeal },  { 125150061, ProcHeal },
	{ 120130240, RemoteServant }, { 121130240, RemoteServant }, { 122130240, RemoteServant },
	{ 123130240, RemoteServant }, { 124130240, RemoteServant },
};

const NamedLine kAbilities[] = {
	{ "host of the elements",         Host },
	{ "servant of ro",                ServantRo },
	{ "companion of necessity",       Necessity },
	{ "heart of skyfire",             HeartSkyfire },
	{ "heart of froststone",          HeartFroststone },
	{ "force of elements",            ForceElements },
	{ "frenzied burnout",             FrenziedBurnout },
	{ "thaumaturge's focus",          ThaumaturgeFocus },
	{ "thaumaturge's unity",          ThaumaturgeUnity },
	{ "improved twincast",            Twincast },
	{ "shield of the elements",       ShieldElements },
	{ "turn summoned",                TurnSummoned },
	{ "spire of the elements",        SpireElements },
	{ "gather mana",                  Gather },
	{ "elemental conversion",         Conversion },
	{ "elemental arrow",              Arrow },
	{ "virulent talon",               Talon },
	{ "arcane whisper",               Whisper },
	{ "pyromancy",                    Pyromancy },
	{ "dimensional shift",            DimensionalShift },
	{ "dimensional instability",      Instability },
	{ "arcane destruction",           ArcaneDestruction },
	{ "drape of shadows",             DrapeShadows },
	{ "call of the hero",             CallHeroes },
	{ "host in the shell",          HostShell },
	{ "second wind ward",           SecondWindWard },
	{ "velocity",                   Velocity },
	{ "modulating shard",           ModulationShard },
	{ "small modulating shard",     ModulationShard },
	{ "medium modulating shard",    ModulationShard },
	{ "large modulating shard",     ModulationShard },
	{ "giant modulating shard",     ModulationShard },
	{ "glowing modulating shard",   ModulationShard },
	{ "radiant modulating shard",   ModulationShard },
	{ "dazzling modulating shard",  ModulationShard },
};

bool Contains(std::string_view s, std::string_view part)
{
	return s.find(part) != std::string_view::npos;
}

int BySpell(const eqlib::EQ_Spell* sp, std::string_view base)
{
	const std::string_view extra(sp->Extra, strnlen(sp->Extra, sizeof(sp->Extra)));
	if (StartsWith(extra, "PCPetMag"))
	{
		if (Contains(extra, "ElemAir"))  return PetAir;
		if (Contains(extra, "ElemWat"))  return PetWater;
		if (Contains(extra, "ElemHeat")) return PetFire;
		if (Contains(extra, "ElemErf"))  return PetEarth;
		if (Contains(extra, "MonSum"))   return PetMonster;
		if (Contains(extra, "Epic"))     return PetEpic;
	}
	if (StartsWith(extra, "PCSwmMag"))
	{
		if (Contains(extra, "Garg"))     return Servant;
		if (Contains(extra, "Rune"))     return Contains(base, "minion") ? SummonMinion : SummonServant;
		if (Contains(extra, "Decoy"))    return Decoy;
		if (Contains(extra, "ElemErf"))  return Contains(base, "zomm") ? RageZomm : Bodyguards;
	}
	if (EndsWith(base, " trigger"))
		return ProcHeal;
	if (StartsWith(base, "summon ") || StartsWith(base, "grant ") || StartsWith(base, "wand of ")
		|| StartsWith(base, "rod of "))
		return SummonItem;
	return -1;
}

} // namespace

const ClassCatalogue& MageCatalogue()
{
	static const ClassCatalogue c{ "Magician", eqlib::Mage, kLines, 7, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
