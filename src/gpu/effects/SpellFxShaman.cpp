#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

namespace eqgfx::spellfx {

namespace {

constexpr Pair kSpirit   { { 0.70f, 1.00f, 0.95f, 0.90f }, { 0.20f, 0.70f, 0.60f, 0.80f } };
constexpr Pair kAncestor { { 0.90f, 0.95f, 1.00f, 0.90f }, { 0.50f, 0.60f, 1.00f, 0.75f } };
constexpr Pair kTotem    { { 0.90f, 0.70f, 0.40f, 0.95f }, { 0.50f, 0.30f, 0.15f, 0.90f } };
constexpr Pair kHex      { { 0.80f, 0.40f, 1.00f, 0.90f }, { 0.25f, 0.05f, 0.35f, 0.85f } };
constexpr Pair kSlowing  { { 0.75f, 0.85f, 0.80f, 0.85f }, { 0.35f, 0.45f, 0.42f, 0.70f } };
constexpr Pair kFeral    { { 1.00f, 0.70f, 0.30f, 0.95f }, { 0.70f, 0.20f, 0.05f, 0.85f } };

LOOK(kShmMending, "mending",
	G(IM, Glyph_Spirit, 1.0f, 9.0f, 4, 1, GA_Drop, 0, FIX, kAncestor, 2.2f),
	F(IM, FxForm_Flame, 0.8f, 9.0f, 72, 18, 0.1f, 2.8f, 0, FIX, kSpirit, 2.0f, 0.07f, ADD, 1, 0.4f),
	F(IM, FxForm_Sparkle, 1.3f, 8.0f, 40, 20, 0.12f, 1, 0, FIX, kSpirit, 2.2f, 0.06f, ADD, 1, 0.3f));

LOOK(kShmReckless, "reckless",
	G(IM, Glyph_Heart, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kSpirit, 2.2f),
	F(IM, FxForm_Burst, 2.4f, 1.0f, 72, 2, 2.5f, 16, 0, FIX, kSpirit, 1.2f, 0.09f),
	F(IM, FxForm_Flame, 0.8f, 8.0f, 72, 18, 0.1f, 2.4f, 0, FIX, kHeal, 1.8f, 0.07f));

LOOK(kShmIntervention, "intervention",
	G(RE, Glyph_Spirit, 0.6f, 9.0f, 3, 5, GA_Rain, 2.0f, FIX, kAncestor, 3.0f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 2.0f, 16, 0, FIX, kAncestor, 1.6f, 0.10f, ADD, 1, 0.3f, AE),
	F(IM, FxForm_Flame, 0.75f, 8.0f, 64, 16, 0.1f, 2.4f, 0, FIX, kAncestor, 1.8f, 0.06f));

LOOK(kShmSpirit, "spirit",
	G(IM, Glyph_Spirit, 0.5f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kSpirit, 2.4f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 64, 2, 1.2f, 0, 0.5f, FIX, kSpirit, 2.4f, 0.05f, COMET));

LOOK(kShmHalcyon, "halcyon",
	F(IM, FxForm_Wind, 1.2f, 4.5f, 48, 8, 1.5f, 0, 0, FIX, kSpirit, 2.4f, 0.05f),
	G(IM, Glyph_Leaf, 0.3f, 4.5f, 0, 3, GA_Orbit, 1.3f, FIX, kNature, 3.0f),
	F(AU, FxForm_Helix, 1.0f, 6.0f, 72, 3, 1.2f, 0, 0.5f, FIX, kSpirit, 0, 0.05f, COMET));

LOOK(kShmRenewal, "renewal",
	G(RE, Glyph_Spirit, 0.45f, 5.0f, 3, 6, GA_RingRise, 2.4f, FIX, kSpirit, 3.0f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kSpirit, 3.0f, 0.06f, ADD, 1, 0, AE),
	F(IM, FxForm_Sparkle, 1.1f, 6.5f, 24, 12, 0.10f, 0, 0, FIX, kSpirit, 2.0f));

LOOK(kShmSwell, "spiritual swell",
	F(IM, FxForm_Bloom, 1.0f, 1.5f, 96, 3, 0, 0, 0, FIX, kSpirit, 2.4f, 0.07f, ADD, 1, 0, AE),
	G(IM, Glyph_Spirit, 0.4f, 5.0f, 3, 4, GA_RingRise, 1.6f, FIX, kSpirit, 2.4f));

LOOK(kShmTalismanRegen, "talisman",
	G(RE, Glyph_Totem, 1.0f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kTotem, 2.6f, 0, OVER),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 2, 1.5f, 12, 0, FIX, kSpirit, 1.4f, 0.09f, ADD, 1, 0.2f, AE),
	G(IM, Glyph_Spirit, 0.35f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kSpirit, 1.8f));

LOOK(kShmRecourse, "recourse",
	F(IM, FxForm_Sparkle, 1.1f, 6.0f, 24, 12, 0.10f, 2, 0, FIX, kSpirit, 1.4f),
	F(IM, FxForm_Ring, 1.0f, 3.0f, 32, 0, 2.0f, 1.0f, 0.8f, FIX, kSpirit, 1.2f, 0.04f, COMET));

LOOK(kShmCounterbias, "counterbias",
	G(IM, Glyph_Scales, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSpirit, 2.4f),
	F(IM, FxForm_Ring, 1.2f, 3.0f, 40, 0, 1.0f, 1.0f, 0.4f, FIX, kSpirit, 2.4f, 0.05f, COMET));

LOOK(kShmProcHeal, "spirit mending",
	G(IM, Glyph_Spirit, 0.3f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kSpirit, 1.4f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 20, 10, 0.10f, 0, 0, FIX, kSpirit, 1.2f));

LOOK(kShmBloodCure, "blood of the ancients",
	G(RE, Glyph_Drop, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kWhite, 2.0f),
	F(RE, FxForm_Burst, 1.0f, 1.0f, 96, 2, 1.5f, 16, 0, FIX, kWhite, 1.4f, 0.08f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kWhite, 1.8f));

LOOK(kShmFocusing, "focusing",
	G(IM, Glyph_Totem, 0.8f, 7.5f, 4, 1, GA_PopAscend, 0, FIX, kTotem, 2.6f, 0, OVER),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 64, 2.5f, 3.0f, 0, 1.0f, FIX, kSpirit, 2.2f, 0.05f, COMET),
	F(IM, FxForm_Ring, 1.3f, 0.3f, 40, 0, 0.8f, 1.0f, 0.4f, FIX, kSpirit, 2.4f, 0.05f, COMET));

LOOK(kShmValor, "valor",
	G(IM, Glyph_Spirit, 0.4f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kAncestor, 2.6f),
	F(IM, FxForm_Wind, 1.2f, 4.5f, 48, 8, 1.8f, 0, 0, FIX, kAncestor, 2.2f, 0.05f));

LOOK(kShmUnity, "unity",
	G(IM, Glyph_Totem, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kTotem, 2.8f, 0, OVER),
	G(IM, Glyph_Spirit, 0.35f, 4.5f, 0, 4, GA_Orbit, 1.4f, FIX, kSpirit, 2.8f),
	F(IM, FxForm_Burst, 1.8f, 2.0f, 48, 1, 1.5f, 12, 0, FIX, kSpirit, 0.9f, 0.07f));

LOOK(kShmTalismanBeast, "talisman of the beast",
	G(RE, Glyph_Paw, 0.4f, 1.0f, 3, 4, GA_PopSpin, 2.0f, FIX, kFeral, 2.6f),
	G(RE, Glyph_Claw, 0.8f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kFeral, 2.4f),
	G(IM, Glyph_Claw, 0.4f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kFeral, 1.6f));

LOOK(kShmWard, "ward",
	G(IM, Glyph_Totem, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kTotem, 3.0f, 0, OVER),
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kSpirit, 2.4f, 0.04f),
	Persist(F(AU, FxForm_Ring, 1.3f, 0.3f, 40, 0, 0.6f, 1.0f, 0.3f, FIX, kSpirit, 0, 0.05f, COMET)));

LOOK(kShmForesight, "foresight",
	G(IM, Glyph_Eye, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSpirit, 2.4f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kSpirit, 2.2f));

LOOK(kShmPact, "pact",
	G(IM, Glyph_Drop, 0.5f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kMana, 2.4f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, 3.0f, FIX, kMana, 2.2f, 0.05f),
	F(IM, FxForm_Sparkle, 1.1f, 6.0f, 24, 12, 0.10f, 2, 0, FIX, kBlood, 2.0f));

LOOK(kShmGrowth, "growth",
	F(IM, FxForm_Vines, 1.0f, 5.5f, 96, 5, 1.0f, 0, 0, FIX, kWood, 3.0f, 0.07f, OVER),
	G(IM, Glyph_Tree, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kNature, 3.0f),
	G(IM, Glyph_Leaf, 0.3f, 4.5f, 0, 3, GA_Orbit, 1.4f, FIX, kNature, 3.0f));

LOOK(kShmWatchful, "watchful spirit",
	G(IM, Glyph_Eye, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kAncestor, 3.0f),
	G(IM, Glyph_Spirit, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kAncestor, 3.0f));

LOOK(kShmAncient, "ancient alliance",
	G(IM, Glyph_Totem, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.8f),
	F(IM, FxForm_Bloom, 1.8f, 2.0f, 48, 3, 0, 0, 0, FIX, kSpirit, 2.4f, 0.05f));

LOOK(kShmFaithful, "faithful",
	G(RE, Glyph_Wolf, 1.2f, 3.0f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kSpirit, 4.0f),
	F(RE, FxForm_Rune, 3.0f, 0, 60, 6, 1, 12, 0, FIX, kSpirit, 3.5f),
	F(RE, FxForm_Sparkle, 2.0f, 6.0f, 40, 20, 0.14f, 0, 0, FIX, kSpirit, 3.0f));

LOOK(kShmPack, "pack",
	G(RE, Glyph_Wolf, 0.8f, 3.0f, 3, 4, GA_RingRise, 2.2f, FIX, kSpirit, 3.5f),
	G(RE, Glyph_Paw, 0.3f, 0.3f, 2, 8, GA_PopSpin, 2.6f, FIX, kSpirit, 3.5f));

LOOK(kShmFeralize, "feralize",
	G(IM, Glyph_Claw, 0.8f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kFeral, 2.2f),
	F(IM, FxForm_Flame, 1.0f, 5.0f, 72, 18, 0.8f, 0.8f, 0, FIX, kFeral, 2.4f, 0.07f));

LOOK(kShmAugment, "spirit augmentation",
	G(IM, Glyph_Paw, 0.35f, 4.5f, 0, 3, GA_Orbit, 1.2f, FIX, kFeral, 2.6f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.12f, 0, 0, FIX, kFeral, 2.4f));

LOOK(kShmRoar, "roar",
	G(RE, Glyph_Wolf, 1.2f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kSpirit, 2.0f),
	F(RE, FxForm_Bloom, 1.0f, 2.0f, 96, 3, 0, 0, 0, FIX, kSpirit, 1.8f, 0.07f, ADD, 1, 0, AE),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 0.5f, 16, 0, FIX, kSpirit, 1.2f, 0.10f, ADD, 1, 0, AE));

LOOK(kShmLupine, "lupine spirit",
	G(IM, Glyph_Wolf, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSpirit, 3.0f),
	F(IM, FxForm_Wind, 1.2f, 4.5f, 48, 8, 2.0f, 0, 0, FIX, kSpirit, 2.4f, 0.05f));

LOOK(kShmDrowse, "drowse",
	G(AU, Glyph_Hourglass, 0.9f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kSlowing, 6.0f),
	F(AU, FxForm_Ring, 1.6f, 0.4f, 48, 0, 0.2f, 1.0f, 0.15f, FIX, kSlowing, 6.0f, 0.06f, COMET, 0.3f),
	G(AU, Glyph_Spiral, 1.2f, 0.0f, 2, 1, GA_PopRoll, 0, FIX, kSlowing, 6.0f));

LOOK(kShmLanguor, "languor",
	G(IM, Glyph_Spiral, 0.7f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kSlowing, 3.0f),
	F(IM, FxForm_Ring, 1.4f, 0.4f, 40, 0, 0.2f, 1.0f, 0.15f, FIX, kSlowing, 3.0f, 0.05f, COMET, 0.3f));

LOOK(kShmInsects, "curse of insects",
	G(AU, Glyph_Insect, 0.3f, 5.5f, 0, 8, GA_Orbit, 1.4f, FIX, kDisease, 5.0f, 0, OVER),
	G(AU, Glyph_Hourglass, 0.6f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kSlowing, 5.0f));

LOOK(kShmMalo, "malosini",
	G(IM, Glyph_Totem, 0.8f, 7.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kHex, 2.6f, 0, OVER),
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 5, 2, 8, 1, FIX, kHex, 2.4f, 0.05f),
	G(IM, Glyph_ArrowDown, 0.3f, 6.5f, 0, 3, GA_Orbit, 1.2f, FIX, kHex, 2.6f));

LOOK(kShmHex, "hex",
	G(AU, Glyph_Doll, 0.8f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kHex, 4.0f),
	F(AU, FxForm_Vortex, 1.1f, 4.5f, 64, 4, 1.0f, 2.0f, -2.0f, FIX, kHex, 4.0f, 0.05f, OVER),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.12f, 1, 0, FIX, kHex, 2.2f));

LOOK(kShmCurse, "curse",
	G(IM, Glyph_Doll, 1.0f, 8.0f, 4, 1, GA_Drop, 0, FIX, kHex, 2.4f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 72, 2, 1.0f, 12, 0, FIX, kHex, 1.0f, 0.09f, ADD, 1, 0.35f),
	F(AU, FxForm_Vortex, 1.1f, 4.0f, 48, 3, 1.0f, 1.5f, -2.0f, FIX, kHex, 4.0f, 0.05f, OVER));

LOOK(kShmDecay, "decay",
	G(IM, Glyph_Skull, 0.7f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kDisease, 3.0f, 0, OVER),
	F(IM, FxForm_Cracks, 1.8f, 0, 48, 6, 1.0f, 0, 0, FIX, kDisease, 3.0f, 0.06f, OVER),
	F(IM, FxForm_Bubbles, 1.1f, 5.0f, 48, 6, 0.18f, 0, 0, FIX, kDisease, 3.0f, 0.04f, OVER));

LOOK(kShmSpearVenom, "spear of venom",
	F(TR, FxForm_Bolt, 1, 0, 128, 2, 0.1f, 0.9f, 0.3f, FIX, kPoison, 0.7f, 0.14f),
	G(IM, Glyph_Snake, 0.9f, 6.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kPoison, 1.6f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 72, 2, 1.5f, 14, 0, FIX, kPoison, 0.9f, 0.10f));

LOOK(kShmBreath, "breath",
	F(TR, FxForm_Bolt, 1, 0, 128, 6, 0.8f, 0.8f, 0.5f, FIX, kDisease, 0.9f, 0.28f),
	F(IM, FxForm_Bubbles, 1.3f, 5.5f, 72, 10, 0.25f, 0, 0, FIX, kDisease, 2.2f, 0.04f, OVER),
	G(IM, Glyph_Insect, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.2f, FIX, kDisease, 2.2f));

LOOK(kShmBreathCone, "breath cone",
	F(IM, FxForm_Lightning, 1, 0, 64, 4, 0.6f, 16.0f, 0, FIX, kDisease, 0.9f, 0.24f),
	F(IM, FxForm_Burst, 1.5f, 1.0f, 48, 1, 0.5f, 8, 0, FIX, kDisease, 0.6f));

LOOK(kShmBite, "bite",
	G(IM, Glyph_Wolf, 1.0f, 7.0f, 4, 1, GA_Drop, 0, FIX, kSpirit, 1.6f),
	G(IM, Glyph_Claw, 0.9f, 4.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kFeral, 1.0f, 0.35f),
	F(IM, FxForm_Burst, 2.4f, 1.5f, 72, 2, 1.0f, 14, 0, FIX, kFeral, 0.9f, 0.10f, ADD, 1, 0.35f));

LOOK(kShmIce, "ice",
	G(IM, Glyph_Snowflake, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kCold, 1.6f),
	F(IM, FxForm_Crystal, 1.4f, 4.0f, 60, 12, 1, 1.0f, 0, FIX, kCold, 2.2f, 0.06f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.4f, 1.0f, 72, 2, 1.0f, 12, 0, FIX, kCold, 0.9f, 0.10f, ADD, 1, 0.35f));

LOOK(kShmGift, "frost gift",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.3f, 1.0f, FIX, kCold, 0.8f, 0.14f),
	G(IM, Glyph_Snowflake, 0.8f, 7.5f, 4, 1, GA_PopRoll, 0, FIX, kCold, 1.8f),
	F(IM, FxForm_Crystal, 1.2f, 5.0f, 50, 10, 0, 1.2f, 0, FIX, kCold, 1.8f, 0.05f));

LOOK(kShmBlood, "blood",
	G(IM, Glyph_Drop, 0.3f, 6.0f, 0, 4, GA_Orbit, 1.1f, FIX, kPoison, 3.0f),
	F(IM, FxForm_Bubbles, 1.1f, 5.5f, 64, 8, 0.2f, 0, 0, FIX, kPoison, 3.0f, 0.04f));

LOOK(kShmNectar, "nectar",
	G(IM, Glyph_Drop, 0.35f, 7.0f, 4, 5, GA_Rain, 1.0f, FIX, kPoison, 2.4f),
	F(IM, FxForm_Bubbles, 1.1f, 5.5f, 80, 10, 0.2f, 0, 0, FIX, kPoison, 3.0f, 0.04f),
	F(IM, FxForm_Orbit, 1.3f, 5.0f, 40, 8, 0, 0, 0, FIX, kPoison, 3.0f, 0.04f));

LOOK(kShmVenom, "venom",
	G(AU, Glyph_Snake, 0.6f, 3.5f, 3, 2, GA_PopSpin, 1.0f, FIX, kPoison, 4.0f),
	F(IM, FxForm_Bubbles, 1.1f, 5.0f, 64, 8, 0.2f, 0, 0, FIX, kPoison, 2.6f, 0.04f));

LOOK(kShmMalady, "malady",
	G(IM, Glyph_Insect, 0.3f, 5.0f, 0, 6, GA_Orbit, 1.3f, FIX, kDisease, 3.0f, 0, OVER),
	F(IM, FxForm_Orbit, 1.5f, 6.0f, 72, 14, 0, 0, 0, FIX, kDisease, 3.0f, 0.035f, OVER, 3.5f));

LOOK(kShmPandemic, "pandemic",
	G(IM, Glyph_Insect, 0.35f, 5.5f, 0, 8, GA_Orbit, 1.7f, FIX, kDisease, 3.2f, 0, OVER),
	F(IM, FxForm_Bubbles, 1.4f, 5.5f, 96, 12, 0.25f, 0, 0, FIX, kDisease, 3.2f, 0.04f, OVER),
	F(IM, FxForm_Bloom, 2.0f, 2.0f, 48, 3, 0, 0, 0, FIX, kDisease, 2.4f, 0.05f, OVER));

LOOK(kShmRainVenom, "rain of venom",
	F(IM, FxForm_Rain, 1.0f, 20.0f, 80, 70, 0, 0.3f, 0, FIX, kPoison, 3.0f, 0.08f, ADD, 1, 0, AE),
	F(IM, FxForm_Bubbles, 0.6f, 4.0f, 96, 14, 0.25f, 0, 0, FIX, kPoison, 2.4f, 0.035f, ADD, 1, 1.0f, AE));

LOOK(kShmChaotic, "chaotic venom",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.6f, 0.4f, 2.0f, FIX, kPrism, 0.8f, 0.18f),
	G(IM, Glyph_Snake, 0.9f, 6.5f, 4, 1, GA_PopRoll, 0, FIX, kPrism, 1.8f),
	F(IM, FxForm_Burst, 2.2f, 1.5f, 72, 2, 1.5f, 12, 0, FIX, kPrism, 0.9f, 0.09f));

LOOK(kShmChant, "chant of the vines",
	F(AU, FxForm_Vines, 1.0f, 3.5f, 96, 5, 0.8f, 0, 1.0f, FIX, kWood, 5.0f, 0.07f, OVER),
	F(IM, FxForm_Bubbles, 1.0f, 4.0f, 48, 6, 0.18f, 0, 0, FIX, kPoison, 2.4f, 0.035f));

LOOK(kAaAbundant, "abundant healing",
	G(IM, Glyph_Spirit, 1.0f, 9.0f, 4, 1, GA_Drop, 0, FIX, kSpirit, 2.2f),
	G(IM, Glyph_Heart, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 2.0f, 0.4f),
	F(IM, FxForm_Flame, 0.8f, 9.0f, 72, 18, 0.1f, 2.8f, 0, FIX, kSpirit, 2.0f, 0.07f, ADD, 1, 0.4f));

LOOK(kAaAncestralAid, "ancestral aid",
	G(RE, Glyph_Totem, 1.2f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kTotem, 2.6f, 0, OVER),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 2.0f, 16, 0, FIX, kAncestor, 1.6f, 0.10f, ADD, 1, 0.2f, AE),
	G(IM, Glyph_Spirit, 0.4f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kAncestor, 2.0f));

LOOK(kAaAncestralGuard, "ancestral guard",
	G(IM, Glyph_Totem, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kTotem, 3.0f, 0, OVER),
	F(IM, FxForm_Dome, 1.9f, 7.2f, 112, 8, 3, 0, 0, FIX, kAncestor, 3.0f, 0.05f));

LOOK(kAaCallAncients, "call of the ancients",
	G(RE, Glyph_Totem, 1.4f, 3.5f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kTotem, 4.0f, 0, OVER),
	F(RE, FxForm_Rune, 2.4f, 0, 70, 6, 1, 12, 0, FIX, kSpirit, 4.0f),
	F(RE, FxForm_Helix, 1.4f, 7.0f, 96, 3, 1.2f, 0, 0.5f, FIX, kSpirit, 3.5f, 0.05f, COMET));

LOOK(kAaCallWild, "call of the wild",
	G(IM, Glyph_Wolf, 1.0f, 7.5f, 4, 1, GA_PopAscend, 0, FIX, kSpirit, 3.0f),
	G(IM, Glyph_Ankh, 0.8f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kAncestor, 3.0f, 0.4f),
	F(IM, FxForm_Flame, 0.9f, 10.0f, 80, 20, 0.15f, 3.0f, 0, FIX, kSpirit, 3.0f, 0.07f));

LOOK(kAaCannibalize, "cannibalization",
	G(IM, Glyph_Heart, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kBlood, 2.0f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, 3.0f, FIX, kMana, 2.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.12f, 2, 0, FIX, kMana, 2.0f));

LOOK(kAaShrink, "group shrink",
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0, 12, 0, FIX, kSpirit, 1.0f, 0.06f, ADD | FxFlag_Reverse),
	G(IM, Glyph_ArrowDown, 0.3f, 7.0f, 0, 3, GA_Orbit, 1.1f, FIX, kSpirit, 2.0f));

LOOK(kAaSilentPresence, "silent presence",
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kLight, 2.5f, 0.05f),
	G(IM, Glyph_Spirit, 0.5f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kLight, 2.2f));

LOOK(kAaPureSpirit, "pure spirit",
	G(IM, Glyph_Spirit, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kWhite, 2.2f),
	F(IM, FxForm_Bloom, 1.8f, 1.0f, 40, 3, 0, 0, 0, FIX, kWhite, 2.0f, 0.05f));

LOOK(kAaInconspicuous, "inconspicuous totem",
	G(IM, Glyph_Totem, 0.9f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kLight, 2.4f),
	F(IM, FxForm_Dome, 1.8f, 6.5f, 96, 6, 2, 0, 0, FIX, kLight, 2.0f, 0.03f));

LOOK(kAaMalaise, "malaise",
	G(IM, Glyph_Doll, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kHex, 2.6f),
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 5, 2, 8, 1, FIX, kHex, 2.4f, 0.05f));

LOOK(kAaWindMalaise, "wind of malaise",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 60, 12, 2.0f, 0, 0, FIX, kHex, 2.4f, 0.05f),
	F(IM, FxForm_Bloom, 1.0f, 2.0f, 96, 3, 0, 0, 0, FIX, kHex, 2.4f, 0.06f, ADD, 1, 0, AE));

LOOK(kAaTurgurs, "turgur's swarm",
	G(AU, Glyph_Insect, 0.35f, 5.5f, 0, 10, GA_Orbit, 1.6f, FIX, kHex, 5.0f, 0, OVER),
	G(AU, Glyph_Hourglass, 0.6f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kSlowing, 5.0f));

LOOK(kAaLevitation, "perfected levitation",
	G(IM, Glyph_Feather, 0.35f, 4.0f, 0, 4, GA_Orbit | GlyphAnim_Ascend, 1.3f, FIX, kFeather, 2.4f),
	F(IM, FxForm_Wind, 1.2f, 4.0f, 48, 8, 1.5f, 0, 0, FIX, kAir, 2.0f));

LOOK(kAaPurified, "purified spirits",
	G(IM, Glyph_Spirit, 0.4f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kWhite, 2.4f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 64, 2, 2.0f, 12, 0, FIX, kWhite, 1.0f, 0.06f));

LOOK(kAaSoothsayer, "soothsayer's intervention",
	G(IM, Glyph_Eye, 0.8f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kAncestor, 2.4f),
	G(IM, Glyph_Spirit, 0.5f, 9.0f, 3, 3, GA_Rain, 1.4f, FIX, kAncestor, 2.4f));

LOOK(kAaSpiritCall, "spirit call",
	G(RE, Glyph_Wolf, 0.8f, 3.0f, 3, 3, GA_RingRise, 2.0f, FIX, kSpirit, 3.5f),
	F(RE, FxForm_Vortex, 1.6f, 5.0f, 64, 4, 1.2f, 2.0f, 3.0f, FIX, kSpirit, 3.0f, 0.05f));

LOOK(kAaSpiritLeap, "spirit leap",
	F(RE, FxForm_Wind, 1.4f, 4.0f, 72, 12, 3.0f, 0, 0, FIX, kSpirit, 1.4f),
	G(RE, Glyph_Spirit, 0.6f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kSpirit, 1.8f));

LOOK(kAaUrgency, "spirit of urgency",
	G(RE, Glyph_Spirit, 1.0f, 3.0f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kBlood, 3.0f),
	G(RE, Glyph_Horn, 0.6f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kBlood, 2.4f));

LOOK(kAaSpiritualBlessing, "spiritual blessing",
	G(IM, Glyph_Spirit, 0.35f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kGold, 3.0f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kGold, 2.4f));

LOOK(kAaSpiritualChanneling, "spiritual channeling",
	G(IM, Glyph_Drop, 0.6f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kBlood, 2.4f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, -3.0f, FIX, kBlood, 2.4f, 0.05f));

LOOK(kAaRebuke, "spiritual rebuke",
	F(IM, FxForm_Lightning, 1, 0, 64, 3, 0.4f, 18.0f, 0, FIX, kSpirit, 0.9f, 0.18f),
	F(AU, FxForm_Vines, 1.0f, 2.0f, 96, 6, 0.4f, 0, 1.0f, FIX, kWood, 4.0f, 0.07f, OVER));

LOOK(kAaCelerity, "talisman of celerity",
	G(RE, Glyph_Totem, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kStar, 2.4f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kStar, 1.4f, 0.10f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Wind, 1.2f, 4.0f, 48, 8, 2.5f, 0, 0, FIX, kStar, 1.6f, 0.05f));

LOOK(kAaUnion, "union of spirits",
	G(IM, Glyph_Spirit, 0.4f, 5.0f, 3, 5, GA_RingRise, 1.6f, FIX, kSpirit, 2.6f),
	G(IM, Glyph_Heart, 0.6f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kHeal, 2.6f));

LOOK(kAaParalysis, "virulent paralysis",
	G(IM, Glyph_Snake, 0.8f, 6.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kPoison, 2.0f),
	F(AU, FxForm_Vines, 1.0f, 2.0f, 96, 6, 0.4f, 0, 1.0f, FIX, kPoison, 5.0f, 0.07f, OVER));

LOOK(kAaVisionary, "visionary's unity",
	G(RE, Glyph_Wolf, 0.7f, 3.0f, 3, 5, GA_RingRise, 2.2f, FIX, kSpirit, 3.5f),
	G(RE, Glyph_Eye, 0.8f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kSpirit, 3.0f));

LOOK(kAaSpireAncestors, "spire of ancestors",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kSpirit, 2.6f, 0.10f),
	G(RE, Glyph_Totem, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kTotem, 2.6f, 0, OVER),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kSpirit, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaRabidBear, "rabid bear",
	G(IM, Glyph_Claw, 1.2f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kFeral, 3.0f),
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.8f, 1.2f, 0, FIX, kFeral, 3.0f, 0.10f),
	G(IM, Glyph_Paw, 0.35f, 0.3f, 2, 6, GA_PopSpin, 1.6f, FIX, kFeral, 3.0f));

LOOK(kAaPactWolf, "pact of the wolf",
	G(IM, Glyph_Wolf, 1.0f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kSpirit, 3.0f),
	G(IM, Glyph_Moon, 0.6f, 9.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kAncestor, 3.0f, 0.3f),
	F(IM, FxForm_Wind, 1.2f, 4.0f, 48, 8, 2.0f, 0, 0, FIX, kSpirit, 2.4f, 0.05f));

LOOK(kAaCascade, "cascade of life",
	G(IM, Glyph_Heart, 0.5f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kSpirit, 1.8f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kSpirit, 1.8f));

LOOK(kAaDampen, "dampen resistance",
	F(IM, FxForm_Rune, 1.6f, 3.2f, 60, 6, 1, 8, 1, FIX, kSlowing, 2.4f, 0.05f),
	G(IM, Glyph_ArrowDown, 0.35f, 7.0f, 0, 3, GA_Orbit, 1.2f, FIX, kSlowing, 2.4f));

LOOK(kAaFleetingSpirit, "fleeting spirit",
	G(IM, Glyph_Spirit, 0.9f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 2.6f),
	F(IM, FxForm_Wind, 1.2f, 6.0f, 64, 12, 1.6f, 0, 0, FIX, kGhost, 2.4f, 0.04f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kAir, 2.2f));

LOOK(kAaLanguidBite, "languid bite",
	G(IM, Glyph_Fang, 0.8f, 6.5f, 4, 1, GlyphAnim_Pop | GlyphAnim_Wiggle, 0, FIX, kBlood, 1.8f),
	G(IM, Glyph_Hourglass, 0.5f, 8.2f, 4, 1, GA_PopRoll, 0, FIX, kAmber, 2.2f, 0.2f),
	F(IM, FxForm_Rain, 1.0f, 5.0f, 32, 16, 0.4f, 0, 0, FIX, kBlood, 1.6f, 0.04f));

LOOK(kAaAncestralGuardian, "ancestral guardian spirit",
	G(IM, Glyph_Totem, 1.1f, 6.5f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kAurora, 3.5f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kAurora, 3.0f, 0.05f),
	F(IM, FxForm_Orbit, 1.5f, 5.0f, 48, 6, 0, 0, 0, FIX, kGhost, 3.5f, 0.05f, ADD, 1.2f));

enum Line : int
{
	Mending, Reckless, Intervention, Spirit, Halcyon, Renewal, Swell, TalismanRegen, Recourse,
	Counterbias, ProcHeal, BloodCure, Focusing, Valor, Unity, TalismanBeast, Ward, Foresight, Pact,
	Growth, Watchful, Ancient, Faithful, Pack, Feralize, Augment, Roar, Lupine, Drowse, Languor,
	Insects, Malo, Hex, Curse, Decay, SpearVenom, Breath, BreathCone, Bite, Ice, Gift, Blood, Nectar,
	Venom, Malady, Pandemic, RainVenom, Chaotic, Chant,
	Abundant, AncestralAid, AncestralGuard, CallAncients, CallWild, Cannibalize, Shrink, SilentPresence, PureSpirit, Inconspicuous, Malaise, WindMalaise, Turgurs, Levitation,
	Purified, Soothsayer, SpiritCall, SpiritLeap, Urgency, SpiritualBlessing, SpiritualChanneling,
	Rebuke, Celerity, Union, Paralysis, Visionary, SpireAncestors, RabidBear,
	PactWolf, Cascade, Dampen,
	FleetingSpirit, LanguidBite, AncestralGuardian,
};

const LineLook kLines[] = {
	{ "Mending (single heal)",          &kShmMending },
	{ "Reckless (big heal)",            &kShmReckless },
	{ "Intervention (group heal)",      &kShmIntervention },
	{ "Spirit (heal over time)",        &kShmSpirit },
	{ "Halcyon (heal over time)",       &kShmHalcyon },
	{ "Renewal (group heal over time)", &kShmRenewal },
	{ "Spiritual Swell (area heal)",    &kShmSwell },
	{ "Talisman (group regen)",         &kShmTalismanRegen },
	{ "Recourse (group heal)",          &kShmRecourse },
	{ "Counterbias (triggers)",         &kShmCounterbias },
	{ "spirit mending proc",            &kShmProcHeal },
	{ "Blood (group cure)",             &kShmBloodCure },
	{ "Focusing (hit points)",          &kShmFocusing },
	{ "Spirit of Valor (stamina)",      &kShmValor },
	{ "Unity (buff bundle)",            &kShmUnity },
	{ "Talisman of the beast (proc)",   &kShmTalismanBeast },
	{ "Ward (mitigation)",              &kShmWard },
	{ "Foresight",                      &kShmForesight },
	{ "Pact (health to mana)",          &kShmPact },
	{ "Growth (hit points)",            &kShmGrowth },
	{ "Watchful Spirit",                &kShmWatchful },
	{ "Ancient Alliance",               &kShmAncient },
	{ "Faithful (pet)",                 &kShmFaithful },
	{ "Pack (swarm)",                   &kShmPack },
	{ "Feralize (pet buff)",            &kShmFeralize },
	{ "Spirit Augmentation (pet buff)", &kShmAugment },
	{ "Roar",                           &kShmRoar },
	{ "Lupine Spirit",                  &kShmLupine },
	{ "Drowse (slow)",                  &kShmDrowse },
	{ "Languor (slow proc)",            &kShmLanguor },
	{ "Curse of Insects (slow)",        &kShmInsects },
	{ "Malosini (resist debuff)",       &kShmMalo },
	{ "Hex (curse)",                    &kShmHex },
	{ "Curse (curse damage)",           &kShmCurse },
	{ "Decay (debuff)",                 &kShmDecay },
	{ "Spear of Venom (nuke)",          &kShmSpearVenom },
	{ "Breath (disease nuke)",          &kShmBreath },
	{ "Breath (cone)",                  &kShmBreathCone },
	{ "Bite (nuke)",                    &kShmBite },
	{ "Ice (cold nuke)",                &kShmIce },
	{ "Gift (cold nuke)",               &kShmGift },
	{ "Blood (poison)",                 &kShmBlood },
	{ "Nectar (poison)",                &kShmNectar },
	{ "Venom (poison)",                 &kShmVenom },
	{ "Malady (disease)",               &kShmMalady },
	{ "Pandemic (disease)",             &kShmPandemic },
	{ "Rain of Venom (area poison)",    &kShmRainVenom },
	{ "Chaotic Venom",                  &kShmChaotic },
	{ "Chant (vines)",                  &kShmChant },

	{ "AA: Abundant Healing",           &kAaAbundant },
	{ "AA: Ancestral Aid",              &kAaAncestralAid },
	{ "AA: Ancestral Guard",            &kAaAncestralGuard },
	{ "AA: Call of the Ancients",       &kAaCallAncients },
	{ "AA: Call of the Wild / Rejuvenation", &kAaCallWild },
	{ "AA: Cannibalization",            &kAaCannibalize },
	{ "AA: Group Shrink",               &kAaShrink },
	{ "AA: Silent Presence",            &kAaSilentPresence },
	{ "AA: Improved Pure Spirit",       &kAaPureSpirit },
	{ "AA: Inconspicuous Totem",        &kAaInconspicuous },
	{ "AA: Malaise",                    &kAaMalaise },
	{ "AA: Wind of Malaise",            &kAaWindMalaise },
	{ "AA: Turgur's Swarm",             &kAaTurgurs },
	{ "AA: Perfected Levitation",       &kAaLevitation },
	{ "AA: Purified Spirits",           &kAaPurified },
	{ "AA: Soothsayer's Intervention",  &kAaSoothsayer },
	{ "AA: Spirit Call",                &kAaSpiritCall },
	{ "AA: Spirit Leap",                &kAaSpiritLeap },
	{ "AA: Spirit of Urgency",          &kAaUrgency },
	{ "AA: Spiritual Blessing",         &kAaSpiritualBlessing },
	{ "AA: Spiritual Channeling",       &kAaSpiritualChanneling },
	{ "AA: Spiritual Rebuke",           &kAaRebuke },
	{ "AA: Talisman of Celerity",       &kAaCelerity },
	{ "AA: Union of Spirits",           &kAaUnion },
	{ "AA: Virulent Paralysis",         &kAaParalysis },
	{ "AA: Visionary's Unity",          &kAaVisionary },
	{ "AA: Spire of Ancestors",         &kAaSpireAncestors },
	{ "AA: Rabid Bear",                 &kAaRabidBear },
	{ "AA: Pact of the Wolf",           &kAaPactWolf },
	{ "AA: Cascade of Life",            &kAaCascade },
	{ "AA: Dampen Resistance",          &kAaDampen },
	{ "AA: Fleeting Spirit",  &kAaFleetingSpirit },
	{ "AA: Languid Bite",     &kAaLanguidBite },
	{ "AA: Spirit Guardian",  &kAaAncestralGuardian },
};

const GroupLine kNumbered[] = {
	{ 0, Foresight },   { 1, SpearVenom },  { 2, Spirit },      { 3, TalismanBeast }, { 5, Faithful },
	{ 6, Breath },      { 7, Feralize },    { 8, Mending },     { 9, Focusing },    { 10, Valor },
	{ 12, Pact },       { 13, Bite },       { 14, Valor },      { 15, TalismanRegen }, { 16, Ice },
	{ 17, Hex },        { 19, Drowse },     { 20, Blood },      { 21, Focusing },   { 22, Halcyon },
	{ 23, Unity },      { 24, Unity },      { 25, Nectar },     { 26, Renewal },    { 27, Languor },
	{ 29, Ward },       { 30, Intervention }, { 31, Pack },     { 32, BloodCure },  { 33, Counterbias },
	{ 34, Malo },       { 35, Gift },       { 36, Malo },       { 37, Malady },     { 38, Recourse },
	{ 39, Pandemic },   { 40, Venom },      { 41, Reckless },   { 43, Malady },     { 44, RainVenom },
	{ 45, Insects },    { 46, Malo },       { 47, Curse },      { 48, Decay },      { 49, BreathCone },
	{ 50, Augment },    { 51, Swell },      { 52, Counterbias }, { 53, ProcHeal },  { 55, Roar },
	{ 58, Pandemic },   { 59, Watchful },   { 60, Chaotic },    { 61, Ancient },
};

const GroupLine kModern[] = {
	{ 20, SpearVenom }, { 30, Spirit },     { 40, TalismanBeast }, { 50, Faithful }, { 60, Breath },
	{ 80, Mending },    { 90, Focusing },   { 100, Valor },     { 110, Pact },      { 120, Bite },
	{ 140, TalismanRegen }, { 150, Ice },   { 160, Hex },       { 170, Drowse },    { 180, Blood },
	{ 200, Halcyon },   { 210, Unity },     { 220, Unity },     { 230, Nectar },    { 240, Renewal },
	{ 250, Languor },   { 260, Ward },      { 270, Intervention }, { 280, Pack },   { 300, Counterbias },
	{ 302, ProcHeal },  { 320, Gift },      { 340, Malady },    { 350, Recourse },  { 360, Pandemic },
	{ 370, Venom },     { 380, Reckless },  { 390, Malady },    { 420, Malo },      { 430, Curse },
	{ 440, Decay },     { 450, BreathCone }, { 460, Augment },  { 490, Roar },      { 500, Pandemic },
	{ 510, Watchful },  { 520, Chaotic },   { 530, Ancient },
};

const GroupLine kOther[] = {
	{ 3001, Foresight }, { 3002, SpearVenom }, { 3004, Spirit }, { 3006, Malo },    { 3008, Breath },
	{ 3010, Feralize }, { 3011, Mending },  { 3012, Focusing }, { 3014, Foresight }, { 3015, Pact },
	{ 3016, Bite },     { 3018, Valor },    { 3019, TalismanRegen }, { 3020, Ice }, { 3022, Hex },
	{ 3024, Drowse },   { 3026, Nectar },   { 3027, Focusing }, { 3028, Halcyon },  { 3031, Malo },
	{ 3033, TalismanBeast }, { 3035, Nectar }, { 3036, Renewal }, { 3037, Languor }, { 8503, Renewal },
	{ 7100, Malo },     { 7200, Malo },     { 7300, Malo },     { 7400, Malo },     { 7500, Malo },
	{ 7600, Malo },     { 7700, Malo },     { 7800, Malo },     { 7900, Malo },     { 125130010, Malo },
	{ 5240, Growth },   { 5340, Growth },   { 5440, Growth },   { 5540, Growth },   { 5640, Growth },
	{ 5740, Growth },   { 5840, Growth },   { 5940, Growth },   { 125060350, Growth },
	{ 5122, Chant },    { 5222, Chant },    { 5322, Chant },    { 5422, Chant },    { 5522, Chant },
	{ 5622, Chant },    { 5722, Chant },    { 5922, Chant },    { 1510131, Lupine },
};

const NamedLine kAbilities[] = {
	{ "abundant healing",             Abundant },
	{ "ancestral aid",                AncestralAid },
	{ "ancestral guard",              AncestralGuard },
	{ "call of the ancients",         CallAncients },
	{ "call of the wild",             CallWild },
	{ "rejuvenation of spirit",       CallWild },
	{ "cannibalization",              Cannibalize },
	{ "group shrink",                 Shrink },
	{ "silent presence",              SilentPresence },
	{ "group silent presence",        SilentPresence },
	{ "improved pure spirit",         PureSpirit },
	{ "inconspicuous totem",          Inconspicuous },
	{ "lupine spirit",                Lupine },
	{ "malaise",                      Malaise },
	{ "wind of malaise",              WindMalaise },
	{ "turgur's swarm",               Turgurs },
	{ "turgur's virulent swarm",      Turgurs },
	{ "perfected levitation",         Levitation },
	{ "purified spirits",             Purified },
	{ "soothsayer's intervention",    Soothsayer },
	{ "spirit call",                  SpiritCall },
	{ "spirit leap",                  SpiritLeap },
	{ "spirit of urgency",            Urgency },
	{ "spiritual blessing",           SpiritualBlessing },
	{ "spiritual channeling",         SpiritualChanneling },
	{ "spiritual rebuke",             Rebuke },
	{ "talisman of celerity",         Celerity },
	{ "union of spirits",             Union },
	{ "virulent paralysis",           Paralysis },
	{ "visionary's unity",            Visionary },
	{ "spire of ancestors",           SpireAncestors },
	{ "rabid bear",                   RabidBear },
	{ "pact of the wolf",             PactWolf },
	{ "group pact of the wolf",       PactWolf },
	{ "cascade of life",              Cascade },
	{ "dampen resistance",            Dampen },
	{ "feralize",                     Feralize },
	{ "fleeting spirit (decay)",    FleetingSpirit },
	{ "languid bite",               LanguidBite },
	{ "languid bite bleed",         LanguidBite },
	{ "ancestral guardian spirit",  AncestralGuardian },
};

int BySpell(const eqlib::EQ_Spell*, std::string_view base)
{
	if (EndsWith(base, " counterbias effect"))
		return ProcHeal;
	if (EndsWith(base, " recourse"))
		return Recourse;
	return -1;
}

} // namespace

const ClassCatalogue& ShamanCatalogue()
{
	static const ClassCatalogue c{ "Shaman", eqlib::Shaman, kLines, 14, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
