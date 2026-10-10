#include "../../MQEQGFX.h"

#include "../../SettingsFile.h"
#include "../../PatternScan.h"
#include "../../Signatures.h"
#include "SpellFxLooks.h"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <deque>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

using namespace eqgfx::gpu;
using namespace eqgfx::spellfx;

namespace {

constexpr float kPi = 3.14159265358979323846f;

LOOK(kFlameShield, "flame shield",
	Persist(F(AU, FxForm_Flame,   1.10f, 5.8f, 120, 30, 0.9f, 1.0f, 0, FIX, kFire, 0, 0.09f)),
	Persist(F(AU, FxForm_Sparkle, 1.30f, 7.0f,  24, 12, 0.10f, 0, 0, FIX,
		Pair{ { 1.0f, 0.65f, 0.2f, 0.9f }, { 1.0f, 0.2f, 0.0f, 0.7f } }, 0)),
	F(IM, FxForm_Burst, 3.0f, 0, 48, 1, 1.0f, 12, 0, FIX, kFire, 0.8f, 0.10f));

LOOK(kThornShield, "thorn shield",
	Persist(F(AU, FxForm_Vines,  1.15f, 5.5f, 120, 5, 1.1f, 0, 0, FIX, kWood, 2.5f, 0.07f, OVER | FxFlag_NoEnvelope)),
	Persist(F(AU, FxForm_Thorns, 1.15f, 5.5f,  30, 5, 1.1f, 6, 0, FIX, kThorn, 2.5f, 0.06f, OVER | FxFlag_NoEnvelope)),
	G(IM, Glyph_Leaf, 0.35f, 3.0f, 0, 5, GA_PopAscend, 1.4f, FIX, kNature, 1.8f));

LOOK(kFrostShield, "frost shield",
	Persist(F(AU, FxForm_Crystal, 1.20f, 5.5f, 50, 10, 0, 1.0f, 0, FIX, kCold, 2.0f, 0.05f, ADD | FxFlag_NoEnvelope)),
	Persist(F(AU, FxForm_Sparkle, 1.40f, 7.0f, 32, 16, 0.08f, 1, 0, FIX, kWhite, 0)),
	G(IM, Glyph_Snowflake, 0.6f, 6.8f, 0, 1, GA_PopAscend, 0, FIX, kCold, 1.6f));

LOOK(kArcaneShield, "arcane shield",
	Persist(F(AU, FxForm_Flame,   1.05f, 5.8f, 72, 18, 0.9f, 0.8f, 0, PRI, kMagic, 0, 0.07f)),
	Persist(F(AU, FxForm_Sparkle, 1.30f, 6.5f, 24, 12, 0.10f, 0, 0, PRI, kMagic, 0)));

LOOK(kVenomShield, "venom shield",
	Persist(F(AU, FxForm_Bubbles, 1.20f, 6.0f, 96, 12, 0.22f, 0, 0, PRI, kPoison, 0, 0.04f)),
	Persist(F(AU, FxForm_Flame,   1.00f, 5.5f, 48, 12, 0.9f, 0.6f, 0, PRI, kPoison, 0, 0.06f)));

LOOK(kWings, "wings",
	Persist(F(AU, FxForm_Wings, 1.0f, 4.3f, 58, 7, 0.5f, 3.2f, 0, FIX, kFeather, 1.2f, 0.06f, ADD | FxFlag_NoEnvelope)),
	F(RE, FxForm_Wind, 1.4f, 5.0f, 60, 10, 2.0f, 0, 0, FIX, kAir, 1.2f),
	F(IM, FxForm_Sparkle, 1.5f, 6.0f, 32, 16, 0.14f, 1, 0, FIX, kFeather, 2.0f));

LOOK(kHaste, "haste",
	F(AU, FxForm_Wind, 1.3f, 5.0f, 72, 12, 2.5f, 0, 0, FIX, kStar, 2.5f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.12f, 0, 0, FIX, kStar, 1.5f),
	G(IM, Glyph_Lightning, 0.5f, 7.0f, 0, 1, GA_PopAscend, 0, FIX, kStar, 1.6f));

LOOK(kRunSpeed, "run speed",
	F(AU, FxForm_Wind, 1.2f, 2.5f, 72, 12, 3.0f, 0, 0, PRI, kNature, 3.0f, 0.05f),
	G(IM, Glyph_Feather, 0.45f, 6.8f, 0, 1, GA_PopAscend, 0, PRI, kNature, 1.6f));

LOOK(kInvis, "invisibility",
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX,
		Pair{ { 1, 1, 1, 0.45f }, { 0.7f, 0.8f, 1.0f, 0.15f } }, 2.5f, 0.05f),
	F(IM, FxForm_Dome, 1.8f, 6.5f, 96, 6, 2, 0, 0, FIX,
		Pair{ { 0.9f, 0.95f, 1.0f, 0.4f }, { 0.6f, 0.7f, 1.0f, 0.1f } }, 2.0f, 0.03f));

LOOK(kVision, "vision",
	G(IM, Glyph_Eye, 0.9f, 7.2f, 4, 1, GA_PopAscend, 0, PRI, kArcane, 2.5f),
	F(IM, FxForm_Sparkle, 1.0f, 7.5f, 24, 12, 0.10f, 0, 0, PRI, kArcane, 2.0f));

LOOK(kIllusionLook, "illusion",
	G(IM, Glyph_Moon, 0.8f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kIllusion, 3.0f),
	F(IM, FxForm_Flame, 1.1f, 6.2f, 96, 24, 0.95f, 0.8f, 0, FIX, kIllusion, 2.2f, 0.06f),
	F(IM, FxForm_Sparkle, 1.6f, 7.0f, 40, 20, 0.14f, 2, 0, FIX, kIllusion, 2.0f));

LOOK(kRaiseDead, "raise dead",
	F(RE, FxForm_Cracks, 3.5f, 0, 48, 6, 1.2f, 0, 0, FIX, kNecro, 4.0f, 0.08f),
	F(RE, FxForm_Rune, 3.2f, 0, 60, 5, 2, 8, 0, FIX, kNecro, 4.5f, 0.06f),
	G(RE, Glyph_Skeleton, 3.0f, 3.0f, 3, 3, GA_RiseWiggle, 2.6f, FIX, kBone, 4.8f, 0.2f, OVER),
	F(RE, FxForm_Sparkle, 2.5f, 6.0f, 40, 20, 0.14f, 0, 0, FIX, kNecro, 4.0f),
	F(RE, FxForm_Vortex, 1.8f, 3.0f, 64, 4, 1.2f, 1.5f, -2.0f, FIX, kShadow, 3.0f, 0.05f, OVER));

LOOK(kFireElemental, "fire elemental",
	F(RE, FxForm_Flame, 1.5f, 8.0f, 128, 32, 0.5f, 2.2f, 0, FIX, kFire, 3.5f, 0.12f),
	F(RE, FxForm_Rune, 3.0f, 0, 60, 6, 1, 8, 0, FIX, kFire, 3.5f),
	F(RE, FxForm_Burst, 4.0f, 0, 96, 3, 1.5f, 0, 0, FIX, kFire, 1.2f, 0.12f, ADD, 1, 0.3f),
	G(RE, Glyph_Flame, 0.9f, 9.0f, 4, 1, GA_PopAscend, 0, FIX, kFire, 2.5f, 0.5f));

LOOK(kWaterElemental, "water elemental",
	F(RE, FxForm_Vortex, 2.0f, 7.0f, 96, 4, 1.4f, 2.5f, 3.0f, FIX, kWater, 3.5f, 0.08f),
	F(RE, FxForm_Bubbles, 2.0f, 7.0f, 112, 14, 0.3f, 0, 0, FIX, kWater, 3.5f, 0.04f),
	F(RE, FxForm_Rune, 3.0f, 0, 60, 6, 1, 8, 0, FIX, kWater, 3.5f),
	G(RE, Glyph_Wave, 0.9f, 8.5f, 4, 1, GA_PopAscend, 0, FIX, kWater, 2.5f, 0.5f));

LOOK(kEarthElemental, "earth elemental",
	F(RE, FxForm_Cracks, 4.0f, 0, 64, 8, 1.4f, 0, 0, FIX, kEarth, 4.0f, 0.10f, OVER),
	F(RE, FxForm_Crystal, 2.4f, 4.0f, 50, 10, 1.0f, 1.0f, 0, FIX, kEarth, 3.5f, 0.07f, OVER),
	G(RE, Glyph_Rock, 0.6f, 1.5f, 3, 5, GlyphAnim_Rise | GlyphAnim_Spin | GlyphAnim_Bob, 2.5f, FIX, kEarth, 4.0f, 0, OVER),
	F(RE, FxForm_Burst, 4.0f, 0, 64, 2, 0.5f, 0, 0, FIX, kEarth, 1.2f, 0.12f, OVER));

LOOK(kAirElemental, "air elemental",
	F(RE, FxForm_Vortex, 2.5f, 9.0f, 120, 5, 1.8f, 3.0f, 4.0f, FIX, kAir, 3.5f, 0.07f),
	F(RE, FxForm_Sparkle, 2.5f, 6.0f, 40, 20, 0.12f, 2, 0, FIX, kAir, 3.0f),
	G(RE, Glyph_Tornado, 0.9f, 10.0f, 4, 1, GA_PopAscend, 0, FIX, kAir, 2.5f, 0.5f));

LOOK(kBeastSpirit, "beast spirit",
	G(RE, Glyph_Paw, 0.55f, 0.0f, 2, 4, GlyphAnim_Pop | GlyphAnim_Spin, 2.2f, PRI, kGold, 3.5f),
	F(RE, FxForm_Rune, 3.0f, 0, 60, 6, 1, 12, 0, PRI, kGold, 3.5f),
	F(RE, FxForm_Sparkle, 2.0f, 6.0f, 40, 20, 0.14f, 0, 0, PRI, kGold, 3.0f));

LOOK(kAnimateWeapon, "animate weapon",
	G(RE, Glyph_Sword, 1.2f, 3.0f, 3, 3, GlyphAnim_Rise | GlyphAnim_Spin, 2.2f, FIX, kIron, 4.0f, 0, OVER),
	F(RE, FxForm_Rune, 3.0f, 0, 60, 6, 1, 8, 0, FIX, kIllusion, 4.0f));

LOOK(kSummonPet, "summoning",
	F(RE, FxForm_Rune, 3.0f, 0, 70, 5, 2, 10, 0, PRI, kMagic, 3.5f),
	G(RE, Glyph_Pentagram, 2.2f, 0.0f, 2, 1, GlyphAnim_Pop, 0, PRI, kMagic, 3.5f),
	F(RE, FxForm_Vortex, 1.6f, 5.0f, 64, 4, 1.4f, 2.0f, 3.0f, PRI, kMagic, 3.0f),
	F(RE, FxForm_Sparkle, 2.0f, 6.0f, 32, 16, 0.12f, 2, 0, PRI, kMagic, 3.0f));

LOOK(kResurrect, "resurrection",
	F(IM, FxForm_Flame, 0.9f, 10.0f, 96, 24, 0.15f, 3.0f, 0, FIX, kDivine, 4.0f, 0.08f),
	F(IM, FxForm_Sparkle, 1.6f, 14.0f, 60, 30, 0.14f, 0, 0, FIX, kGold, 4.0f),
	G(IM, Glyph_Ankh, 1.4f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 4.0f, 0.3f),
	F(IM, FxForm_Rune, 3.0f, 0, 70, 8, 3, 12, 0, FIX, kDivine, 4.0f));

LOOK(kTeleport, "teleport",
	F(RE, FxForm_Vortex, 2.0f, 10.0f, 120, 6, 0.6f, 3.0f, 5.0f, PRI, kArcane, 4.0f, 0.07f),
	F(RE, FxForm_Rune, 3.2f, 0, 70, 8, 3, 12, 0, PRI, kArcane, 4.0f),
	F(RE, FxForm_Sparkle, 1.8f, 10.0f, 50, 25, 0.12f, 0, 0, PRI, kArcane, 4.0f));

LOOK(kBind, "bind affinity",
	F(IM, FxForm_Rune, 2.4f, 0, 60, 6, 1, 6, 0, PRI, kGold, 3.0f),
	G(IM, Glyph_Ankh, 0.7f, 7.0f, 4, 1, GA_PopAscend, 0, PRI, kGold, 2.5f),
	F(IM, FxForm_Sparkle, 1.2f, 8.0f, 30, 15, 0.12f, 0, 0, PRI, kGold, 2.5f));

LOOK(kSummonItem, "summon item",
	F(RE, FxForm_Sparkle, 1.8f, 5.0f, 40, 20, 0.12f, 2, 0, PRI, kGold, 2.0f),
	F(RE, FxForm_Rune, 1.4f, 0, 60, 5, 2, 0, 0, PRI, kGold, 2.0f));

LOOK(kSummonCorpse, "summon corpse",
	F(IM, FxForm_Vortex, 1.6f, 4.0f, 64, 4, 1.2f, 2.0f, -3.0f, FIX, kShadow, 2.5f, 0.05f, OVER),
	G(IM, Glyph_Skull, 0.8f, 6.5f, 4, 1, GA_PopAscend, 0, FIX, kBone, 2.5f, 0, OVER));

LOOK(kMount, "mount",
	F(RE, FxForm_Rune, 2.8f, 0, 60, 6, 1, 8, 0, PRI, kGold, 2.5f),
	F(RE, FxForm_Burst, 3.5f, 0, 64, 2, 0.5f, 0, 0, PRI, kGold, 1.2f, 0.10f),
	F(RE, FxForm_Wind, 1.5f, 3.0f, 60, 10, 2.5f, 0, 0, PRI, kGold, 1.5f));

LOOK(kEye, "scrying eye",
	G(RE, Glyph_Eye, 1.3f, 8.0f, 4, 1, GA_PopBob, 0, PRI, kArcane, 3.0f),
	F(RE, FxForm_Rune, 2.0f, 0, 60, 6, 1, 0, 0, PRI, kArcane, 3.0f));

LOOK(kMez, "mesmerize",
	G(AU, Glyph_Spiral, 0.8f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kIllusion, 6.0f),
	G(AU, Glyph_Z, 0.35f, 7.8f, 0, 1, GlyphAnim_Ascend, 0.5f, FIX, kWhite, 2.0f, 0.2f),
	G(AU, Glyph_Z, 0.45f, 8.1f, 0, 1, GlyphAnim_Ascend, 0.5f, FIX, kWhite, 2.0f, 1.0f),
	G(AU, Glyph_Z, 0.55f, 8.4f, 0, 1, GlyphAnim_Ascend, 0.5f, FIX, kWhite, 2.0f, 1.8f),
	F(AU, FxForm_Ring, 1.0f, 6.9f, 32, 0, 1.2f, 1.0f, 0.3f, FIX, kIllusion, 6.0f, 0.05f, ADD | FxFlag_Comet));

LOOK(kCharm, "charm",
	G(AU, Glyph_Heart, 0.45f, 7.2f, 0, 3, GA_Orbit, 1.1f, FIX, kPink, 6.0f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kPink, 2.0f));

LOOK(kFear, "fear",
	G(AU, Glyph_Skull, 0.45f, 7.3f, 0, 3, GA_Orbit, 1.2f, FIX, kBone, 5.0f, 0, OVER),
	F(IM, FxForm_Bloom, 2.5f, 3.0f, 48, 3, 0, 0, 0, FIX, kShadow, 2.5f, 0.06f, OVER | FxFlag_Reverse));

LOOK(kStun, "stun",
	G(AU, Glyph_Star, 0.32f, 6.9f, 0, 4, GlyphAnim_Pop | GlyphAnim_Spin, 0.9f, FIX, kStar, 3.0f),
	F(IM, FxForm_Burst, 2.0f, 5.5f, 48, 1, 0.5f, 12, 0, FIX, kStar, 0.6f, 0.08f));

LOOK(kRoot, "root",
	F(AU, FxForm_Vines, 1.0f, 1.8f, 120, 6, 0.4f, 0, 1.0f, FIX, kWood, 5.0f, 0.08f, OVER),
	F(AU, FxForm_Thorns, 1.0f, 1.8f, 24, 6, 0.4f, 4, 1.0f, FIX, kThorn, 5.0f, 0.06f, OVER),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 3.0f, 0.06f, OVER));

LOOK(kSnare, "snare",
	G(AU, Glyph_Link, 0.35f, 0.6f, 3, 6, GlyphAnim_Pop | GlyphAnim_Spin, 1.1f, FIX, kIron, 5.0f, 0, OVER),
	F(AU, FxForm_Ring, 1.1f, 0.6f, 40, 0, 0.3f, 1.0f, 0, FIX, kIron, 5.0f, 0.04f, OVER));

LOOK(kSlow, "slow",
	G(AU, Glyph_Hourglass, 0.8f, 7.2f, 4, 1, GA_PopBob, 0, PRI, kGrey, 5.0f),
	F(AU, FxForm_Ring, 1.6f, 0.4f, 48, 0, 0.2f, 1.0f, 0.15f, PRI, kGrey, 5.0f, 0.06f, ADD | FxFlag_Comet, 0.3f));

LOOK(kBlind, "blindness",
	F(AU, FxForm_Dome, 0.7f, 0.9f, 72, 5, 1, 0, 0, FIX, kShadow, 4.0f, 0.06f, OVER),
	G(AU, Glyph_Eye, 0.6f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kShadow, 4.0f, 0, OVER));

LOOK(kSilence, "silence",
	G(AU, Glyph_Note, 0.6f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kGrey, 3.0f),
	F(IM, FxForm_Bloom, 1.5f, 3.0f, 40, 2, 0, 0, 0, FIX, kGrey, 2.0f, 0.05f, ADD | FxFlag_Reverse));

LOOK(kTaunt, "taunt",
	F(IM, FxForm_Burst, 3.0f, 0.5f, 72, 2, 1.0f, 8, 0, FIX, kBlood, 0.9f, 0.10f),
	G(IM, Glyph_ArrowDown, 0.5f, 7.5f, 0, 1, GA_PopAscend, 0, FIX, kBlood, 1.5f));

LOOK(kFeign, "feign death",
	G(IM, Glyph_Skull, 0.6f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kGrey, 2.5f),
	F(IM, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kGrey, 2.0f, 0.05f, ADD | FxFlag_Reverse));

LOOK(kDispel, "dispel",
	F(IM, FxForm_Burst, 2.4f, 4.0f, 72, 2, 1.5f, 8, 0, FIX, kWhite, 0.8f),
	F(IM, FxForm_Sparkle, 1.4f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kWhite, 1.6f));

LOOK(kRune, "rune",
	F(AU, FxForm_Dome, 1.8f, 6.5f, 96, 6, 2, 0, 0, FIX, kIllusion, 3.0f, 0.04f),
	F(IM, FxForm_Rune, 1.6f, 3.2f, 70, 6, 1, 12, 1, FIX, kIllusion, 2.0f),
	G(IM, Glyph_Shield, 0.6f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kIllusion, 2.0f));

LOOK(kArmor, "armor",
	G(IM, Glyph_Shield, 0.9f, 7.2f, 4, 1, GA_PopAscend, 0, PRI, kGold, 2.5f),
	F(IM, FxForm_Dome, 1.6f, 6.2f, 88, 5, 2, 0, 0, PRI, kGold, 2.0f, 0.03f));

LOOK(kResistBuff, "resist ward",
	F(IM, FxForm_Dome, 1.7f, 6.5f, 96, 6, 2, 0, 0, PRI, kWhite, 2.5f, 0.04f),
	F(IM, FxForm_Sparkle, 1.3f, 6.5f, 32, 16, 0.12f, 0, 0, PRI, kWhite, 2.0f));

LOOK(kDivineAura, "divine aura",
	F(IM, FxForm_Dome, 2.0f, 7.0f, 112, 8, 3, 0, 0, FIX, kDivine, 3.0f, 0.06f),
	G(IM, Glyph_Sun, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Flame, 0.9f, 10.0f, 64, 16, 0.15f, 3.0f, 0, FIX, kDivine, 2.5f, 0.06f));

LOOK(kStatBuff, "empower",
	G(IM, Glyph_ArrowUp, 0.35f, 5.5f, 0, 3, GA_PopAscend | GlyphAnim_Spin, 1.3f, PRI, kGold, 2.0f),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.12f, 0, 0, PRI, kGold, 2.0f));

LOOK(kStatDebuff, "weaken",
	G(IM, Glyph_ArrowDown, 0.35f, 7.0f, 0, 3, GA_Orbit, 1.2f, PRI, kShadow, 3.0f),
	F(IM, FxForm_Bloom, 2.0f, 2.0f, 48, 3, 0, 0, 0, PRI, kShadow, 2.5f, 0.05f, ADD | FxFlag_Reverse));

LOOK(kHealFx, "heal",
	F(IM, FxForm_Flame, 0.75f, 8.0f, 72, 18, 0.1f, 2.6f, 0, FIX, kHeal, 1.8f, 0.07f),
	F(IM, FxForm_Sparkle, 1.4f, 9.0f, 40, 20, 0.12f, 1, 0, FIX, kWhite, 1.6f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 64, 32, 0.13f, 0, 0, FIX, kHeal, 2.6f, 0.06f, ADD, 1, 0.3f),
	G(IM, Glyph_Cross, 0.6f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 2.4f, 0.2f),
	F(IM, FxForm_Burst, 2.2f, 0.5f, 72, 2, 2.5f, 8, 0, FIX, kHeal, 1.4f, 0.07f),
	F(IM, FxForm_Ring, 1.3f, 3.0f, 40, 0, 1.4f, 1.0f, 0.5f, FIX, kHeal, 2.4f, 0.05f, ADD | FxFlag_Comet));

LOOK(kBigHeal, "complete heal",
	F(IM, FxForm_Flame, 0.9f, 11.0f, 96, 24, 0.15f, 3.0f, 0, FIX, kDivine, 3.2f, 0.08f),
	F(IM, FxForm_Sparkle, 1.4f, 10.0f, 80, 40, 0.14f, 0, 0, FIX, kGold, 3.2f),
	F(IM, FxForm_Sparkle, 1.6f, 12.0f, 40, 20, 0.12f, 1, 0, FIX, kWhite, 2.4f),
	F(IM, FxForm_Rune, 2.8f, 0, 70, 8, 3, 12, 0, FIX, kGold, 3.2f),
	F(IM, FxForm_Burst, 3.0f, 0.5f, 96, 3, 3.0f, 12, 0, FIX, kGold, 1.8f, 0.10f),
	G(IM, Glyph_Cross, 0.9f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kGold, 3.0f, 0.3f));

LOOK(kRegen, "regeneration",
	Persist(G(AU, Glyph_Leaf, 0.3f, 4.5f, 0, 3, GA_Orbit, 1.4f, FIX, kNature, 0)),
	Persist(F(AU, FxForm_Sparkle, 1.2f, 6.5f, 24, 12, 0.10f, 0, 0, FIX, kHeal, 0)),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kHeal, 2.0f),
	F(IM, FxForm_Ring, 1.3f, 2.5f, 40, 0, 1.0f, 1.0f, 0.3f, FIX, kNature, 2.0f, 0.05f, ADD | FxFlag_Comet));

LOOK(kGroupHeal, "group heal",
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 2.0f, 16, 0, FIX, kHeal, 1.8f, 0.10f, ADD, 1, 0, Size::AERange),
	F(RE, FxForm_Rune, 2.6f, 0, 70, 8, 3, 12, 0, FIX, kHeal, 2.4f),
	F(RE, FxForm_Sparkle, 1.5f, 8.0f, 60, 30, 0.14f, 0, 0, FIX, kHeal, 2.4f),
	G(RE, Glyph_Cross, 0.7f, 7.5f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 2.4f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 48, 24, 0.12f, 0, 0, FIX, kHeal, 2.2f),
	F(IM, FxForm_Burst, 1.8f, 0.5f, 48, 1, 2.0f, 0, 0, FIX, kHeal, 1.2f, 0.06f));

LOOK(kManaRegen, "mana",
	G(AU, Glyph_Drop, 0.35f, 5.5f, 0, 3, GA_Orbit, 1.1f, FIX, kMana, 4.0f),
	F(AU, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.10f, 0, 0, FIX, kMana, 4.0f));

LOOK(kCure, "cure",
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kWhite, 1.6f),
	F(IM, FxForm_Burst, 1.6f, 3.0f, 64, 2, 2.0f, 0, 0, FIX, kWhite, 1.0f, 0.05f),
	G(IM, Glyph_Drop, 0.5f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kWhite, 1.6f));

LOOK(kLifetap, "lifetap",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.5f, 16, 0, FIX, kBlood, 2.0f, 0.10f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.0f, 2.0f, -3.0f, FIX, kBlood, 1.8f, 0.05f),
	F(Stage::Aura, FxForm_Sparkle, 1.0f, 6.0f, 32, 16, 0.12f, 2, 0, FIX, kBlood, 2.0f));

LOOK(kManaDrain, "mana drain",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.5f, 16, 0, FIX, kMana, 2.0f, 0.10f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.0f, 2.0f, -3.0f, FIX, kMana, 1.8f, 0.05f));

LOOK(kFireBolt, "fireball",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.35f, 1.5f, PRI, kFire, 0.8f, 0.20f),
	F(IM, FxForm_Burst, 2.4f, 1.0f, 72, 2, 2.0f, 8, 0, PRI, kFire, 0.8f, 0.12f),
	F(IM, FxForm_Flame, 1.1f, 5.5f, 80, 20, 0.8f, 1.0f, 0, PRI, kFire, 1.4f, 0.08f));

LOOK(kFrostBolt, "frostbolt",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.3f, 1.0f, PRI, kCold, 0.8f, 0.14f),
	F(IM, FxForm_Crystal, 1.2f, 5.0f, 50, 10, 0, 1.2f, 0, PRI, kCold, 1.8f, 0.05f),
	F(IM, FxForm_Sparkle, 1.4f, 6.0f, 32, 16, 0.10f, 1, 0, FIX, kWhite, 1.8f));

LOOK(kVenomBolt, "venom",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.45f, 0.3f, 1.2f, PRI, kPoison, 0.8f, 0.16f),
	F(IM, FxForm_Bubbles, 1.2f, 5.5f, 96, 12, 0.25f, 0, 0, PRI, kPoison, 2.2f, 0.04f));

LOOK(kPlagueBolt, "plague",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.45f, 0.3f, 1.2f, PRI, kDisease, 0.8f, 0.16f),
	F(IM, FxForm_Orbit, 1.4f, 5.5f, 60, 12, 0, 0, 0, PRI, kDisease, 2.4f, 0.04f, OVER, 3.0f),
	F(IM, FxForm_Bubbles, 1.2f, 5.0f, 64, 8, 0.2f, 0, 0, PRI, kDisease, 2.2f, 0.04f, OVER));

LOOK(kShadowBolt, "shadow",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.6f, 0.4f, 1.2f, PRI, kCorrupt, 0.8f, 0.18f, OVER),
	F(IM, FxForm_Vortex, 1.3f, 4.0f, 64, 4, 1.0f, 2.0f, -3.0f, PRI, kCorrupt, 1.8f, 0.06f, OVER));

LOOK(kArcaneBolt, "arcane missile",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.4f, 2.0f, PRI, kMagic, 0.8f, 0.16f),
	F(IM, FxForm_Burst, 2.0f, 1.5f, 72, 2, 2.0f, 12, 0, PRI, kMagic, 0.8f, 0.08f),
	F(IM, FxForm_Sparkle, 1.4f, 6.0f, 32, 16, 0.12f, 0, 0, PRI, kMagic, 1.4f));

LOOK(kPrismBolt, "prismatic",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.6f, 0.4f, 2.0f, FIX, kPrism, 0.8f, 0.18f),
	F(IM, FxForm_Burst, 2.4f, 1.5f, 96, 3, 2.0f, 0, 0, FIX, kPrism, 1.0f, 0.10f));

LOOK(kLightning, "lightning",
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 2, 1.0f, 14.0f, 1.0f, FIX, kShock, 0.7f, 0.12f),
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 2, 1.2f, 14.0f, 1.0f, FIX, kShock, 0.6f, 0.10f, ADD, 1, 0.3f),
	F(IM, FxForm_Burst, 2.0f, 1.0f, 64, 2, 1.0f, 12, 0, FIX, kShock, 0.8f, 0.08f, ADD, 1, 0.1f),
	F(IM, FxForm_Sparkle, 1.2f, 5.0f, 32, 16, 0.12f, 1, 0, FIX, kShock, 1.2f));

LOOK(kSmite, "smite",
	F(IM, FxForm_Flame, 0.8f, 12.0f, 96, 24, 0.1f, 3.5f, 0, FIX, kDivine, 1.4f, 0.10f),
	G(IM, Glyph_Sun, 0.8f, 7.5f, 4, 1, GA_PopAscend, 0, FIX, kGold, 1.6f),
	F(IM, FxForm_Burst, 2.4f, 0.5f, 72, 2, 1.0f, 12, 0, FIX, kGold, 0.9f, 0.10f));

LOOK(kBeam, "beam",
	F(IM, FxForm_Lightning, 1, 0, 64, 3, 0.4f, 20.0f, 0, PRI, kMagic, 0.9f, 0.18f),
	F(IM, FxForm_Burst, 1.5f, 1.0f, 48, 1, 0.5f, 8, 0, PRI, kMagic, 0.6f));

LOOK(kPBAE, "shockwave",
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 1.5f, 24, 0, PRI, kMagic, 1.4f, 0.14f, ADD, 1, 0, Size::AERange),
	F(RE, FxForm_Cracks, 0.8f, 0, 64, 8, 1.0f, 0, 0, ACC, kEarth, 1.8f, 0.06f, ADD, 1, 0, Size::AERange));

LOOK(kFireNova, "fire nova",
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 1.5f, 24, 0, PRI, kFire, 1.4f, 0.14f, ADD, 1, 0, Size::AERange),
	F(RE, FxForm_Flame, 0.8f, 3.5f, 128, 32, 0.15f, 1.2f, 0, PRI, kFire, 1.8f, 0.10f, ADD, 1, 0.1f, Size::AERange));

LOOK(kFrostNova, "frost nova",
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 1.0f, 24, 0, PRI, kCold, 1.4f, 0.12f, ADD, 1, 0, Size::AERange),
	F(RE, FxForm_Crystal, 0.6f, 4.0f, 100, 20, 1.0f, 1.5f, 0, PRI, kCold, 2.2f, 0.06f, ADD, 1, 0.1f, Size::AERange));

LOOK(kQuake, "earthquake",
	F(RE, FxForm_Cracks, 1.0f, 0, 96, 12, 1.6f, 0, 0, FIX, kEarth, 2.5f, 0.10f, OVER, 1, 0, Size::AERange),
	F(RE, FxForm_Burst, 1.0f, 0.2f, 96, 3, 0.3f, 0, 0, FIX, kEarth, 1.5f, 0.12f, OVER, 1, 0, Size::AERange));

LOOK(kRain, "rain",
	F(IM, FxForm_Rain, 1.0f, 25.0f, 80, 80, 0, 0.3f, 0, PRI, kFire, 3.0f, 0.10f, ADD, 1, 0, Size::AERange),
	F(IM, FxForm_Burst, 1.0f, 0.3f, 96, 3, 0.5f, 0, 0, PRI, kFire, 1.2f, 0.10f, ADD, 1, 2.2f, Size::AERange));

LOOK(kBurning, "burning",
	F(IM, FxForm_Flame, 1.0f, 5.0f, 64, 16, 0.8f, 0.8f, 0, PRI, kFire, 2.5f, 0.07f));

LOOK(kPoisoned, "poisoned",
	F(IM, FxForm_Bubbles, 1.1f, 5.5f, 80, 10, 0.2f, 0, 0, PRI, kPoison, 3.0f, 0.04f),
	F(IM, FxForm_Orbit, 1.3f, 5.0f, 40, 8, 0, 0, 0, PRI, kPoison, 3.0f, 0.04f));

LOOK(kDiseased, "diseased",
	F(IM, FxForm_Orbit, 1.5f, 6.0f, 72, 14, 0, 0, 0, FIX, Pair{ { 0.2f, 0.22f, 0.1f, 0.9f }, { 0.1f, 0.1f, 0.05f, 0.9f } },
		3.0f, 0.035f, OVER, 3.5f),
	F(IM, FxForm_Bubbles, 1.1f, 5.0f, 64, 8, 0.2f, 0, 0, PRI, kDisease, 3.0f, 0.04f, OVER));

LOOK(kCorrupted, "corrupted",
	F(IM, FxForm_Vortex, 1.1f, 4.5f, 64, 4, 1.0f, 2.0f, -2.0f, PRI, kCorrupt, 3.0f, 0.05f, OVER),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.12f, 1, 0, PRI, kCorrupt, 3.0f, 0.05f, OVER));

LOOK(kHexed, "hexed",
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 40, 20, 0.12f, 0, 0, PRI, kMagic, 2.5f),
	F(IM, FxForm_Ring, 1.2f, 3.0f, 40, 0, 1.5f, 1.0f, 0.4f, PRI, kMagic, 2.5f, 0.05f, ADD | FxFlag_Comet));

LOOK(kSongLook, "song",
	G(RE, Glyph_Note, 0.4f, 6.0f, 0, 3, GA_Orbit | GlyphAnim_Ascend, 1.4f, FIX, kSong, 2.2f),
	F(RE, FxForm_Bloom, 2.5f, 2.0f, 48, 3, 0, 0, 0, FIX, kSong, 1.8f, 0.05f));

LOOK(kGenericBuff, "blessing",
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 0, 0, PRI, kGold, 2.0f),
	F(IM, FxForm_Ring, 1.3f, 3.0f, 40, 0, 1.0f, 1.0f, 0.3f, PRI, kGold, 2.0f, 0.05f, ADD | FxFlag_Comet));

LOOK(kGenericDebuff, "affliction",
	F(IM, FxForm_Bloom, 2.0f, 2.5f, 48, 3, 0, 0, 0, PRI, kShadow, 2.0f, 0.05f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.12f, 1, 0, PRI, kShadow, 2.0f));

LOOK(kFizzle, "fizzle",
	F(RE, FxForm_Burst, 1.6f, 1.5f, 48, 1, -1.0f, 6, 0, FIX, kGrey, 0.7f, 0.05f),
	F(RE, FxForm_Sparkle, 1.2f, 4.0f, 24, 12, 0.10f, 1, 0, FIX, kGrey, 1.0f));

const SpellLook* const kAllLooks[] = {
	&kFlameShield, &kThornShield, &kFrostShield, &kArcaneShield, &kVenomShield,
	&kWings, &kHaste, &kRunSpeed, &kInvis, &kVision, &kIllusionLook,
	&kRaiseDead, &kFireElemental, &kWaterElemental, &kEarthElemental, &kAirElemental,
	&kBeastSpirit, &kAnimateWeapon, &kSummonPet,
	&kResurrect, &kTeleport, &kBind, &kSummonItem, &kSummonCorpse, &kMount, &kEye,
	&kMez, &kCharm, &kFear, &kStun, &kRoot, &kSnare, &kSlow, &kBlind, &kSilence, &kTaunt,
	&kFeign, &kDispel,
	&kRune, &kArmor, &kResistBuff, &kDivineAura, &kStatBuff, &kStatDebuff,
	&kHealFx, &kBigHeal, &kRegen, &kGroupHeal, &kManaRegen, &kCure,
	&kLifetap, &kManaDrain,
	&kFireBolt, &kFrostBolt, &kVenomBolt, &kPlagueBolt, &kShadowBolt, &kArcaneBolt, &kPrismBolt,
	&kLightning, &kSmite, &kBeam, &kPBAE, &kFireNova, &kFrostNova, &kQuake, &kRain,
	&kBurning, &kPoisoned, &kDiseased, &kCorrupted, &kHexed,
	&kSongLook, &kGenericBuff, &kGenericDebuff, &kFizzle,
};

enum class Charge : uint8_t { Arcane, Divine, Nature, Necro, Fire, Frost, Song, Shadow, Primal };

LOOK(kChargeArcane, "arcane",
	F(CH, FxForm_Rune, 1.9f, 0, 70, 5, 2, 10, 0, PRI, kArcane, 1, 0.05f),
	F(CH, FxForm_Sparkle, 1.4f, 5.0f, 40, 20, 0.10f, 2, 0, PRI, kArcane, 1));
LOOK(kChargeDivine, "divine",
	F(CH, FxForm_Rune, 1.9f, 0, 70, 8, 3, 12, 0, PRI, kDivine, 1, 0.05f),
	F(CH, FxForm_Sparkle, 1.2f, 9.0f, 40, 20, 0.10f, 1, 0, PRI, kDivine, 1));
LOOK(kChargeNature, "nature",
	F(CH, FxForm_Rune, 1.9f, 0, 60, 6, 1, 6, 0, PRI, kNature, 1, 0.05f),
	G(CH, Glyph_Leaf, 0.3f, 4.5f, 0, 3, GA_Orbit, 1.4f, PRI, kNature, 1));
LOOK(kChargeNecro, "necro",
	F(CH, FxForm_Rune, 1.9f, 0, 70, 5, 2, 8, 0, FIX, kNecro, 1, 0.05f),
	G(CH, Glyph_Skull, 0.4f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kNecro, 1),
	F(CH, FxForm_Vortex, 1.2f, 1.5f, 48, 3, 1.0f, 1.5f, -2.0f, FIX, kShadow, 1, 0.04f, OVER));
LOOK(kChargeFire, "fire",
	F(CH, FxForm_Rune, 1.9f, 0, 70, 6, 1, 10, 0, PRI, kFire, 1, 0.05f),
	F(CH, FxForm_Flame, 0.9f, 1.2f, 48, 12, 0.3f, 1.0f, 0, PRI, kFire, 1, 0.06f));
LOOK(kChargeFrost, "frost",
	F(CH, FxForm_Rune, 1.9f, 0, 70, 6, 1, 12, 0, PRI, kCold, 1, 0.05f),
	F(CH, FxForm_Sparkle, 1.4f, 7.0f, 40, 20, 0.08f, 1, 0, FIX, kWhite, 1));
LOOK(kChargeSong, "song",
	G(CH, Glyph_Note, 0.3f, 6.5f, 0, 3, GA_Orbit, 1.3f, FIX, kSong, 1));
LOOK(kChargeShadow, "shadow",
	F(CH, FxForm_Rune, 1.9f, 0, 70, 7, 3, 7, 0, PRI, kCorrupt, 1, 0.05f),
	F(CH, FxForm_Vortex, 1.3f, 2.0f, 48, 3, 1.0f, 1.5f, -2.5f, PRI, kCorrupt, 1, 0.04f, OVER));
LOOK(kChargePrimal, "primal",
	F(CH, FxForm_Rune, 1.9f, 0, 60, 4, 1, 8, 0, PRI, kEarth, 1, 0.05f),
	F(CH, FxForm_Sparkle, 1.4f, 5.0f, 32, 16, 0.10f, 2, 0, PRI, kEarth, 1));

Charge ChargeForClass(int classId)
{
	switch (classId)
	{
	case Shadowknight: return Charge::Necro;
	case Bard:         return Charge::Song;
	case Shaman:       return Charge::Primal;
	case Paladin:      return Charge::Divine;
	case Cleric:       return Charge::Divine;
	case Necromancer:  return Charge::Necro;
	case Druid:        return Charge::Nature;
	case Ranger:       return Charge::Nature;
	case Beastlord:    return Charge::Primal;
	case Wizard:       return Charge::Fire;
	case Rogue:        return Charge::Shadow;
	case Monk:         return Charge::Frost;
	case Berserker:    return Charge::Fire;
	case Warrior:      return Charge::Primal;
	default:           return Charge::Arcane;
	}
}

const SpellLook* ChargeLook(Charge c)
{
	switch (c)
	{
	case Charge::Divine: return &kChargeDivine;
	case Charge::Nature: return &kChargeNature;
	case Charge::Necro:  return &kChargeNecro;
	case Charge::Fire:   return &kChargeFire;
	case Charge::Frost:  return &kChargeFrost;
	case Charge::Song:   return &kChargeSong;
	case Charge::Shadow: return &kChargeShadow;
	case Charge::Primal: return &kChargePrimal;
	default:             return &kChargeArcane;
	}
}

struct Classified
{
	const SpellLook* looks[2] = { nullptr, nullptr };
	int         count = 0;
	Palette     pal { kMagic, kMagic };
	Charge      charge = Charge::Arcane;
	bool        beneficial = false;
	bool        needsTarget = true;
	bool        casterArea = false;
	bool        targetArea = false;
	bool        beam = false;
	bool        dot = false;
	bool        hasPersist = false;
	float       aeRange = 0.0f;
	float       range = 0.0f;
	int         coneStart = 0, coneEnd = 0;
	int         level = 0;
	int         rank = 1;
	std::string why;
};

std::unordered_map<int, Classified> s_classCache;
int s_classCacheOwner = 0;

std::string Lower(const char* s)
{
	std::string out = s ? s : "";
	for (char& c : out)
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	return out;
}

bool Has(const std::string& hay, std::initializer_list<const char*> needles)
{
	for (const char* n : needles)
		if (hay.find(n) != std::string::npos)
			return true;
	return false;
}

Pair ResistPair(int resist)
{
	switch (resist)
	{
	case ResistType_Fire:       return kFire;
	case ResistType_Cold:       return kCold;
	case ResistType_Poison:     return kPoison;
	case ResistType_Disease:    return kDisease;
	case ResistType_Corruption: return kCorrupt;
	case ResistType_Chromatic:
	case ResistType_Prismatic:  return kPrism;
	case ResistType_Physical:   return kEarth;
	default:                    return kMagic;
	}
}

bool CastBy(const EQ_Spell* sp, int cls)
{
	return cls >= 0 && cls <= MAX_CLASSES && sp->ClassLevel[cls] > 0 && sp->ClassLevel[cls] < 255;
}

bool OnlyBards(const EQ_Spell* sp)
{
	if (!CastBy(sp, Bard))
		return false;
	for (int c = 1; c <= MAX_CLASSES; ++c)
		if (c != Bard && CastBy(sp, c))
			return false;
	return true;
}

Charge ChargeFor(const EQ_Spell* sp, bool beneficial)
{
	if (OnlyBards(sp))                                    return Charge::Song;
	if (CastBy(sp, Necromancer) || CastBy(sp, Shadowknight))
		return beneficial ? Charge::Shadow : Charge::Necro;
	if (CastBy(sp, Cleric) || CastBy(sp, Paladin))        return Charge::Divine;
	if (CastBy(sp, Druid) || CastBy(sp, Ranger))          return Charge::Nature;
	if (CastBy(sp, Shaman) || CastBy(sp, Beastlord))      return Charge::Primal;
	if (sp->Resist == ResistType_Fire)                    return Charge::Fire;
	if (sp->Resist == ResistType_Cold)                    return Charge::Frost;
	return Charge::Arcane;
}

Classified ClassifyGeneric(EQ_Spell* sp)
{
	Classified c;
	if (!sp)
	{
		c.looks[0] = &kGenericDebuff;
		c.count = 1;
		c.why = "no spell data";
		return c;
	}

	const std::string name = Lower(sp->Name);
	const std::string extra = Lower(sp->Extra);
	const int tt = sp->TargetType;
	const int resist = sp->Resist;

	c.beneficial = sp->IsBeneficialSpell();
	c.aeRange = sp->AERange;
	c.range = sp->Range;
	c.coneStart = sp->ConeStartAngle;
	c.coneEnd = sp->ConeEndAngle;
	c.charge = ChargeFor(sp, c.beneficial);
	c.pal = Palette{ ResistPair(resist), kEarth };
	for (int cls = 1; cls <= MAX_CLASSES; ++cls)
		if (sp->ClassLevel[cls] > 0 && sp->ClassLevel[cls] < 254 && (c.level == 0 || sp->ClassLevel[cls] < c.level))
			c.level = sp->ClassLevel[cls];
	c.rank = std::clamp(sp->SpellRank, 1, 3);

	c.casterArea = tt == TargetType_PBAE || tt == TargetType_AEPC_v1 || tt == TargetType_Group_v1
		|| tt == TargetType_AEPC_v2 || tt == TargetType_Group_v2 || tt == 36 || tt == 37;
	c.targetArea = tt == TargetType_TargetArea || tt == 20 || tt == 24 || tt == 25 || tt == 45 || tt == 50;
	c.beam = tt == 44 || tt == 42;
	c.needsTarget = !(tt == TargetType_Self || c.casterArea);

	std::map<int, int64_t> fx;
	for (int i = 0; i < sp->GetNumEffects(); ++i)
	{
		const int attrib = sp->GetEffectAttrib(i);
		if (attrib < 0 || attrib == SPA_NOSPELL)
			continue;
		fx.try_emplace(attrib, sp->GetEffectBase(i));
	}
	auto has = [&](int spa) { return fx.count(spa) != 0; };
	auto base = [&](int spa) -> int64_t { auto it = fx.find(spa); return it == fx.end() ? 0 : it->second; };

	const bool duration = sp->DurationCap > 0 || sp->DurationType > 0;

	auto set = [&](const SpellLook& l, const char* why) {
		if (c.count < 2)
			c.looks[c.count++] = &l;
		if (c.why.empty())
			c.why = why;
	};

	if (has(SPA_SUMMON_PET) || has(SPA_CREATE_UNDEAD) || has(SPA_WAKE_DEAD))
	{
		c.needsTarget = false;
		if (has(SPA_WAKE_DEAD) || Has(extra, { "skel", "spectre", "undead", "wraith", "lich", "bone", "zomb", "ghoul", "mummy" })
			|| Has(name, { "skelet", "bone", "undead", "corpse", "dead", "lich", "wraith", "spectr", "cadaver", "zomb", "mummy", "ghoul" })
			|| (CastBy(sp, Necromancer) || CastBy(sp, Shadowknight)))
			set(kRaiseDead, "summons undead");
		else if (Has(extra, { "sumfire", "fire" }) || Has(name, { "fire", "flame", "blaze", "inferno", "ember" }))
			set(kFireElemental, "summons a fire elemental");
		else if (Has(extra, { "sumwater", "water" }) || Has(name, { "water", "tide", "wave", "ocean" }))
			set(kWaterElemental, "summons a water elemental");
		else if (Has(extra, { "sumearth", "earth" }) || Has(name, { "earth", "stone", "rock", "golem" }))
			set(kEarthElemental, "summons an earth elemental");
		else if (Has(extra, { "sumair", "air" }) || Has(name, { "air", "wind", "gale", "storm" }))
			set(kAirElemental, "summons an air elemental");
		else if (CastBy(sp, Beastlord) || CastBy(sp, Shaman) || Has(extra, { "warder", "blpet", "spirit" })
			|| Has(name, { "warder", "spirit of", "companion", "wolf", "bear" }))
			set(kBeastSpirit, "summons a spirit companion");
		else if (CastBy(sp, Enchanter) || Has(name, { "animat", "sword", "blade", "weapon" }))
			set(kAnimateWeapon, "animates a weapon");
		else
			set(kSummonPet, "summons a pet");
		c.charge = c.looks[0] == &kRaiseDead ? Charge::Necro : c.charge;
		return c;
	}

	if (has(SPA_RESURRECT))                          { set(kResurrect, "resurrects"); return c; }
	if (has(SPA_GATE) || has(SPA_TRANSLOCATE) || has(SPA_PORTAL) || has(SPA_EVACUATE)
		|| has(SPA_TELEPORT_CASTERS_BINDPOINT) || has(SPA_GATE_STARTING_CITY) || has(SPA_TRANSLOCATE_TO_ANCHOR)
		|| has(SPA_TELEPORT_TO_ANCHOR))
	{
		c.pal.primary = (CastBy(sp, Druid) || CastBy(sp, Ranger)) ? kNature : kArcane;
		c.needsTarget = false;
		set(kTeleport, "teleports");
		return c;
	}

	if (has(SPA_LEVITATION) && base(SPA_LEVITATION) >= 0) { set(kWings, "levitates"); c.hasPersist = true; }

	if (has(SPA_DAMAGE_SHIELD))
	{
		c.hasPersist = true;
		const bool thorny = Has(name, { "thorn", "thistle", "bramble", "briar", "spike", "barb", "nettle",
			"bristle", "quill", "bark", "vine", "brier" });
		if (thorny || (resist != ResistType_Fire && resist != ResistType_Cold
			&& (CastBy(sp, Druid) || CastBy(sp, Ranger))))
			set(kThornShield, "damage shield, thorns");
		else if (resist == ResistType_Fire || Has(name, { "fire", "flame", "inferno", "blaze", "burn", "magma" }))
			set(kFlameShield, "damage shield, fire");
		else if (resist == ResistType_Cold || Has(name, { "frost", "ice", "cold", "rime" }))
			set(kFrostShield, "damage shield, cold");
		else if (resist == ResistType_Poison || resist == ResistType_Disease)
			set(kVenomShield, "damage shield, venom");
		else
		{
			c.pal.primary = ResistPair(resist);
			set(kArcaneShield, "damage shield, magic");
		}
	}

	if (c.count > 0)
		return c;

	if (has(SPA_ENTHRALL))                           { set(kMez, "mesmerizes"); return c; }
	if (has(SPA_CHARM))                              { set(kCharm, "charms"); return c; }
	if (has(SPA_FEAR) || has(SPA_FEARSTUN))          { set(kFear, "fears"); return c; }
	if (has(SPA_ROOT))                               { set(kRoot, "roots"); return c; }

	const int64_t hp = has(SPA_INSTANT_HP) ? base(SPA_INSTANT_HP) : base(SPA_HP);
	const bool hasHp = has(SPA_HP) || has(SPA_INSTANT_HP);

	const bool undeadNuke = tt == TargetType_TargetUndead || Has(name, { "smite", "exorc", "holy", "sunskin", "sanctif" });

	if (hasHp && hp < 0 && !c.beneficial)
	{
		const bool lightning = Has(name, { "lightning", "shock", "thunder", "storm", "spark", "jolt", "voltaic",
			"static", "bolt of", "electr", "tempest" }) && resist == ResistType_Magic;

		c.dot = duration && !has(SPA_STUN) && !has(SPA_INSTANT_HP);
		if (tt == TargetType_TargetDrain || Has(name, { " tap", "drain", "siphon", "leech", "vampir", "bond of death" }))
		{
			set(kLifetap, "lifetap");
		}
		else if (c.beam)
		{
			c.pal.primary = lightning ? kShock : ResistPair(resist);
			set(kBeam, tt == 44 ? "beam" : "cone");
		}
		else if (c.targetArea)
		{
			c.pal.primary = ResistPair(resist);
			set(kRain, "rains on an area");
		}
		else if (c.casterArea && tt != TargetType_Group_v1 && tt != TargetType_Group_v2)
		{
			if (resist == ResistType_Fire)           set(kFireNova, "fire around the caster");
			else if (resist == ResistType_Cold)      set(kFrostNova, "frost around the caster");
			else if (resist == ResistType_Physical || Has(name, { "quake", "tremor", "earth" }))
				set(kQuake, "shakes the ground");
			else
			{
				c.pal.primary = lightning ? kShock : ResistPair(resist);
				set(kPBAE, "blast around the caster");
			}
		}
		else if (c.dot)
		{
			switch (resist)
			{
			case ResistType_Fire:       set(kBurning, "burns over time"); break;
			case ResistType_Poison:     set(kPoisoned, "poisons over time"); break;
			case ResistType_Disease:    set(kDiseased, "diseases over time"); break;
			case ResistType_Corruption: set(kCorrupted, "corrupts over time"); break;
			default:                    set(kHexed, "damages over time"); break;
			}
		}
		else if (undeadNuke)                     set(kSmite, "smites undead");
		else if (lightning)                      set(kLightning, "calls lightning");
		else
		{
			switch (resist)
			{
			case ResistType_Fire:       set(kFireBolt, "fire bolt"); break;
			case ResistType_Cold:       set(kFrostBolt, "cold bolt"); break;
			case ResistType_Poison:     set(kVenomBolt, "poison bolt"); break;
			case ResistType_Disease:    set(kPlagueBolt, "disease bolt"); break;
			case ResistType_Corruption: set(kShadowBolt, "corruption bolt"); break;
			case ResistType_Chromatic:
			case ResistType_Prismatic:  set(kPrismBolt, "prismatic bolt"); break;
			default:                    set(kArcaneBolt, "magic bolt"); break;
			}
		}

		if (has(SPA_STUN) && c.count < 2)
			set(kStun, "stuns");
		if (OnlyBards(sp) && c.count < 2)
			set(kSongLook, "a song");
		return c;
	}

	if (has(SPA_STUN) || has(SPA_SPIN_STUN))         { set(kStun, "stuns"); return c; }
	if (has(SPA_MANA) && base(SPA_MANA) < 0 && !c.beneficial) { set(kManaDrain, "drains mana"); return c; }

	if (has(SPA_COMPLETEHEAL))                       { set(kBigHeal, "complete heal"); return c; }
	if ((hasHp && hp > 0 && c.beneficial) || (has(SPA_HEALDOT) && c.beneficial))
	{
		if (tt == TargetType_Group_v1 || tt == TargetType_Group_v2 || tt == TargetType_AEPC_v1 || tt == TargetType_AEPC_v2)
			set(kGroupHeal, "group heal");
		else if (has(SPA_HEALDOT) || (duration && !has(SPA_INSTANT_HP)))
			set(kRegen, "heals over time");
		else if (hp > 5000 || Has(name, { "complete", "divine", "celestial", "sacred", "miracle" }))
			set(kBigHeal, "big heal");
		else
			set(kHealFx, "heal");
		if (OnlyBards(sp) && c.count < 2)
			set(kSongLook, "a song");
		return c;
	}

	if (has(SPA_INVULNERABILITY) || has(SPA_DIVINE_INTERVENTION) || has(SPA_DIVINE_SAVE))
		{ set(kDivineAura, "divine protection"); return c; }
	if (has(SPA_STONESKIN) || has(SPA_ABSORB_HIT) || has(SPA_SPELL_GUARD) || has(SPA_MELEE_GUARD)
		|| has(SPA_SHIELD_CASTER) || has(SPA_MANA_ABSORB_PCT_DMG))
		{ set(kRune, "absorbs damage"); return c; }

	if (has(SPA_INVISIBILITY) || has(SPA_INVIS_VS_UNDEAD) || has(SPA_INVIS_VS_ANIMALS))
		{ set(kInvis, "invisibility"); return c; }
	if (has(SPA_CHANGE_FORM) || has(SPA_ILLUSION_OTHERS))  { set(kIllusionLook, "illusion"); return c; }
	if (has(SPA_EYE_OF_ZOMM) || has(SPA_BIND_SIGHT))       { c.needsTarget = false; set(kEye, "scrying"); return c; }
	if (has(SPA_SEE_INVIS) || has(SPA_INFRAVISION) || has(SPA_ULTRAVISION) || has(SPA_MAGNIFICATION))
		{ set(kVision, "sight"); return c; }
	if (has(SPA_BIND_AFFINITY))                            { set(kBind, "binds"); return c; }
	if (has(SPA_FEIGN_DEATH))                              { c.needsTarget = false; set(kFeign, "feigns death"); return c; }
	if (has(SPA_SUMMON_CORPSE))                            { set(kSummonCorpse, "summons a corpse"); return c; }
	if (has(SPA_SUMMON_MOUNT))                             { c.needsTarget = false; set(kMount, "summons a mount"); return c; }
	if (has(SPA_CREATE_ITEM))                              { c.needsTarget = false; set(kSummonItem, "summons an item"); return c; }
	if (has(SPA_DISPEL_MAGIC))                             { set(kDispel, "dispels"); return c; }
	if (has(SPA_BLINDNESS))                                { set(kBlind, "blinds"); return c; }
	if (has(SPA_SILENCE))                                  { set(kSilence, "silences"); return c; }
	if (has(SPA_HATE) || has(SPA_TAUNT) || has(SPA_AE_TAUNT)) { set(kTaunt, "generates hate"); return c; }

	if (has(SPA_HASTE) || has(SPA_BARD_HASTE))
	{
		const int64_t b = has(SPA_HASTE) ? base(SPA_HASTE) : base(SPA_BARD_HASTE);
		if (b < 100 && !c.beneficial) set(kSlow, "slows");
		else                          set(kHaste, "hastes");
		if (OnlyBards(sp) && c.count < 2) set(kSongLook, "a song");
		return c;
	}
	if (has(SPA_MOVEMENT_RATE))
	{
		if (base(SPA_MOVEMENT_RATE) < 0) set(kSnare, "snares");
		else                             set(kRunSpeed, "run speed");
		if (OnlyBards(sp) && c.count < 2) set(kSongLook, "a song");
		return c;
	}

	if (c.beneficial && ((has(SPA_POISON) && base(SPA_POISON) < 0) || (has(SPA_DISEASE) && base(SPA_DISEASE) < 0)
		|| (has(SPA_CURSE) && base(SPA_CURSE) < 0) || (has(SPA_CORRUPTION) && base(SPA_CORRUPTION) < 0)))
		{ set(kCure, "cures"); return c; }

	if (has(SPA_MANA) && base(SPA_MANA) > 0)               { set(kManaRegen, "restores mana"); return c; }

	for (auto [spa, pair] : { std::pair{ SPA_RESIST_FIRE, kFire }, std::pair{ SPA_RESIST_COLD, kCold },
		std::pair{ SPA_RESIST_POISON, kPoison }, std::pair{ SPA_RESIST_DISEASE, kDisease },
		std::pair{ SPA_RESIST_MAGIC, kMagic }, std::pair{ SPA_RESIST_CORRUPTION, kCorrupt } })
	{
		if (!has(spa))
			continue;
		if (base(spa) > 0 && c.beneficial)
		{
			c.pal.primary = has(SPA_RESIST_ALL) ? kPrism : pair;
			set(kResistBuff, "raises resists");
		}
		else
		{
			c.pal.primary = pair;
			set(kStatDebuff, "lowers resists");
		}
		return c;
	}
	if (has(SPA_RESIST_ALL))
	{
		c.pal.primary = kPrism;
		set(base(SPA_RESIST_ALL) > 0 ? kResistBuff : kStatDebuff, "resist all");
		return c;
	}

	if (has(SPA_AC) && base(SPA_AC) > 0 && c.beneficial)   { set(kArmor, "armor"); return c; }

	for (int spa : { SPA_STR, SPA_DEX, SPA_AGI, SPA_STA, SPA_INT, SPA_WIS, SPA_CHA, SPA_ATTACK_POWER, SPA_MAX_HP, SPA_AC })
	{
		if (!has(spa))
			continue;
		if (base(spa) > 0 && c.beneficial) set(kStatBuff, "raises stats");
		else                               set(kStatDebuff, "lowers stats");
		if (OnlyBards(sp) && c.count < 2)  set(kSongLook, "a song");
		return c;
	}

	if (OnlyBards(sp))
	{
		set(kSongLook, "a song");
		return c;
	}

	set(c.beneficial ? kGenericBuff : kGenericDebuff, c.beneficial ? "unclassified buff" : "unclassified debuff");
	if (c.beneficial)
		c.pal.primary = kGold;
	return c;
}

Classified ClassifySpell(EQ_Spell* sp)
{
	Classified c = ClassifyGeneric(sp);
	if (const LineRef ref = FindLineLook(sp, pLocalPlayer ? pLocalPlayer->GetClass() : 0); ref.line)
	{
		const LineLook* line = ref.line;
		c.looks[0] = line->look;
		c.looks[1] = nullptr;
		c.count = 1;
		c.why = std::string(ref.cls) + ": " + line->line;
		if (line->primary.c1.a > 0.0f)
			c.pal.primary = line->primary;
		c.hasPersist = false;
	}
	return c;
}

const Classified& Classify(int spellId)
{
	const int cls = pLocalPlayer ? pLocalPlayer->GetClass() : 0;
	if (cls != s_classCacheOwner)
	{
		s_classCache.clear();
		s_classCacheOwner = cls;
	}

	auto it = s_classCache.find(spellId);
	if (it != s_classCache.end())
		return it->second;

	EQ_Spell* sp = GetSpellByID(spellId);
	Classified c = ClassifySpell(sp);
	for (int i = 0; i < c.count; ++i)
		for (int j = 0; j < c.looks[i]->count; ++j)
			if (c.looks[i]->layers[j].persist)
				c.hasPersist = true;
	return s_classCache.emplace(spellId, std::move(c)).first->second;
}

struct Config
{
	bool  enabled   = true;
	int   scope     = 2;
	float range     = 200.0f;
	bool  charge    = true;
	bool  travel    = true;
	bool  impacts   = true;
	bool  auras     = true;
	int   persist   = 2;
	bool  landings  = true;
	float alpha     = 1.0f;
	float size      = 1.0f;
	float auraTime  = 10.0f;
	float duration  = 1.6f;
	int   maxActive = 400;
	bool  levelScale = true;
	float levelStrength = 1.0f;

	AlwaysOnStyle aura;
};

std::mutex s_cfgMutex;
Config     s_cfg;
constexpr const char* kIni = "SpellFx";

Config Cfg()
{
	std::scoped_lock lock(s_cfgMutex);
	return s_cfg;
}

float ClampF(float v, float lo, float hi, float def)
{
	return std::isfinite(v) ? std::clamp(v, lo, hi) : def;
}

void WriteCfg(const Config& c)
{
	eqgfx::settings::Section sec;
	sec.PutBool("Enabled", c.enabled);
	sec.PutInt("Scope", c.scope);
	sec.PutFloat("Range", c.range);
	sec.PutBool("Charge", c.charge);
	sec.PutBool("Travel", c.travel);
	sec.PutBool("Impacts", c.impacts);
	sec.PutBool("Auras", c.auras);
	sec.PutInt("Persist", c.persist);
	sec.PutBool("Landings", c.landings);
	sec.PutFloat("Alpha", c.alpha);
	sec.PutFloat("Size", c.size);
	sec.PutFloat("AuraTime", c.auraTime);
	sec.PutInt("MaxActive", c.maxActive);
	sec.PutFloat("Duration", c.duration);
	sec.PutFloat("AuraOpacity", c.aura.opacity);
	sec.PutFloat("AuraMotion", c.aura.motion);
	sec.PutFloat("AuraFadeAfter", c.aura.fadeAfter);
	sec.PutFloat("AuraFadeTo", c.aura.fadeTo);
	sec.PutBool("AuraBreathe", c.aura.breathe);
	sec.PutBool("LevelScale", c.levelScale);
	sec.PutFloat("LevelStrength", c.levelStrength);
	eqgfx::settings::json doc = sec.Take();
	for (const char* key : { "AlwaysOnOpacity", "AlwaysOnMotion", "AlwaysOnFadeAfter", "AlwaysOnFadeTo", "AlwaysOnBreathe" })
		if (const eqgfx::settings::json* legacy = eqgfx::settings::Global().Find(kIni, key))
			doc[key] = *legacy;
	eqgfx::settings::Global().SetSection(kIni, std::move(doc));
}

bool  s_cfgDirty = false;
float s_cfgChangedAt = 0.0f;

void SaveCfg(const Config&)
{
	s_cfgDirty = true;
	s_cfgChangedAt = FxClockNow();
}

void FlushCfg(bool force)
{
	if (!s_cfgDirty || (!force && FxClockNow() - s_cfgChangedAt < 1.5f))
		return;
	s_cfgDirty = false;
	WriteCfg(Cfg());
}

void LoadCfg()
{
	Config c;
	c.enabled   = eqgfx::settings::Global().GetBool(kIni, "Enabled", c.enabled);
	c.scope     = std::clamp(eqgfx::settings::Global().GetInt(kIni, "Scope", c.scope), 0, 2);
	c.range     = eqgfx::settings::Global().GetFloat(kIni, "Range", c.range);
	c.charge    = eqgfx::settings::Global().GetBool(kIni, "Charge", c.charge);
	c.travel    = eqgfx::settings::Global().GetBool(kIni, "Travel", c.travel);
	c.impacts   = eqgfx::settings::Global().GetBool(kIni, "Impacts", c.impacts);
	c.auras     = eqgfx::settings::Global().GetBool(kIni, "Auras", c.auras);
	c.persist   = std::clamp(eqgfx::settings::Global().GetInt(kIni, "Persist", c.persist), 0, 2);
	c.landings  = eqgfx::settings::Global().GetBool(kIni, "Landings", c.landings);
	c.alpha     = std::clamp(eqgfx::settings::Global().GetFloat(kIni, "Alpha", c.alpha), 0.05f, 1.0f);
	c.size      = std::clamp(eqgfx::settings::Global().GetFloat(kIni, "Size", c.size), 0.25f, 4.0f);
	c.auraTime  = std::clamp(eqgfx::settings::Global().GetFloat(kIni, "AuraTime", c.auraTime), 1.0f, 60.0f);
	c.maxActive = std::clamp(eqgfx::settings::Global().GetInt(kIni, "MaxActive", c.maxActive), 50, 2000);
	c.duration  = std::clamp(eqgfx::settings::Global().GetFloat(kIni, "Duration", c.duration), 0.5f, 4.0f);
	AlwaysOnStyle& au = c.aura;
	au.opacity   = eqgfx::settings::Global().GetFloat(kIni, "AlwaysOnOpacity", au.opacity);
	au.motion    = eqgfx::settings::Global().GetFloat(kIni, "AlwaysOnMotion", au.motion);
	au.fadeAfter = eqgfx::settings::Global().GetFloat(kIni, "AlwaysOnFadeAfter", au.fadeAfter);
	au.fadeTo    = eqgfx::settings::Global().GetFloat(kIni, "AlwaysOnFadeTo", au.fadeTo);
	au.breathe   = eqgfx::settings::Global().GetBool(kIni, "AlwaysOnBreathe", au.breathe);
	au.opacity   = std::clamp(eqgfx::settings::Global().GetFloat(kIni, "AuraOpacity", au.opacity), 0.05f, 1.0f);
	au.motion    = std::clamp(eqgfx::settings::Global().GetFloat(kIni, "AuraMotion", au.motion), 0.0f, 2.0f);
	au.fadeAfter = std::clamp(eqgfx::settings::Global().GetFloat(kIni, "AuraFadeAfter", au.fadeAfter), 0.0f, 600.0f);
	au.fadeTo    = std::clamp(eqgfx::settings::Global().GetFloat(kIni, "AuraFadeTo", au.fadeTo), 0.0f, 1.0f);
	au.breathe   = eqgfx::settings::Global().GetBool(kIni, "AuraBreathe", au.breathe);
	c.levelScale    = eqgfx::settings::Global().GetBool(kIni, "LevelScale", c.levelScale);
	c.levelStrength = std::clamp(eqgfx::settings::Global().GetFloat(kIni, "LevelStrength", c.levelStrength), 0.0f, 1.0f);
	std::scoped_lock lock(s_cfgMutex);
	s_cfg = c;
}

struct Body
{
	float x = 0, y = 0, z = 0;
	float yaw = 0;
	float scale = 1.0f;
	bool  valid = false;
};

Body BodyOf(int spawnId)
{
	Body b;
	PlayerClient* sp = spawnId > 0 ? GetSpawnByID(spawnId) : nullptr;
	if (!sp)
		return b;
	b.x = sp->X;
	b.y = sp->Y;
	const float h = sp->AvatarHeight > 0.5f ? sp->AvatarHeight : 6.0f;
	const float gap = sp->Z - sp->FloorHeight;
	b.z = (gap >= -1.0f && gap <= h) ? sp->FloorHeight : sp->Z;
	b.yaw = kPi * 0.5f - sp->Heading * (2.0f * kPi / 512.0f);
	b.scale = std::clamp(h / 6.0f, 0.5f, 5.0f);
	b.valid = true;
	return b;
}

struct Active
{
	const Layer* layer = nullptr;
	Palette  pal;
	int      caster = 0;
	int      target = 0;
	int      spellId = 0;
	uint32_t castSerial = 0;
	float    birth = 0;
	float    life = 0;
	float    aeRange = 0;
	float    phase = 0;
	Body     lastA, lastB;
	bool     persistent = false;
	float    fadeStart = -1.0f;
	bool     seen = true;
	float    until = 0;
	float    beamRange = 0;
	float    shownSince = 0;
	int      glyphSlot = -1;
	float    intensity = 0.5f;
};

struct PendingCast
{
	uint32_t serial;
	int      caster;
	int      spellId;
	int      target;
	float    start;
	float    release;
	bool     seenActive;
};

std::vector<Active>      s_active;
std::vector<PendingCast> s_pending;
uint32_t                 s_serial = 0;
int                      s_lastSelfSpell = -1;
uint32_t                 s_lastSelfEta = 0;
float                    s_lastBuffScan = 0.0f;

struct Recent { int spellId, target; float when; };
std::vector<Recent> s_recent;

void PruneRecent(float now)
{
	s_recent.erase(std::remove_if(s_recent.begin(), s_recent.end(),
		[&](const Recent& r) { return now - r.when > 3.0f; }), s_recent.end());
}

std::atomic<uint32_t> s_statActive{ 0 };
std::atomic<uint32_t> s_statCasts{ 0 };

float PhaseFor(uint32_t seed)
{
	uint32_t h = seed * 2654435761u;
	h ^= h >> 15;
	h *= 2246822519u;
	h ^= h >> 13;
	return (h & 0xFFFFFF) / static_cast<float>(0x1000000);
}

bool InScope(int casterId, const Config& cfg)
{
	PlayerClient* me = pLocalPlayer.get();
	PlayerClient* sp = GetSpawnByID(casterId);
	if (!me || !sp)
		return false;
	if (sp == me)
		return true;

	if (GetDistanceSquared3D(me, sp) > cfg.range * cfg.range)
		return false;

	if (cfg.scope >= 2)
		return true;
	if (cfg.scope == 0)
		return false;
	if (pTarget && pTarget == sp)
		return true;
	if (me->PetID == casterId)
		return true;
	return IsInGroup(sp);
}

float StageLife(const Layer& l, const Config& cfg)
{
	if (l.stage == Stage::Aura && l.life <= 0.0f)
		return cfg.auraTime;
	if (l.stage == Stage::Charge || l.stage == Stage::Travel)
		return std::max(l.life, 0.1f);
	return std::max(l.life * cfg.duration, 0.1f);
}

int s_demoLevel = 100;

float IntensityFor(int spellId, int caster, const Config& cfg)
{
	if (!cfg.levelScale)
		return 0.5f;
	int level = s_demoLevel, rank = 1;
	if (spellId > 0)
	{
		const Classified& c = Classify(spellId);
		level = c.level;
		rank = c.rank;
		if (level <= 0)
		{
			PlayerClient* sp = GetSpawnByID(caster);
			level = sp ? sp->Level : 100;
		}
	}
	const float t = std::clamp((level - 1) / 124.0f + 0.04f * (rank - 1), 0.0f, 1.0f);
	return 0.5f + (t - 0.5f) * cfg.levelStrength;
}

void Push(const Layer& l, const Palette& pal, int caster, int target, int spellId, uint32_t serial,
	float birth, float life, float aeRange, bool persistent = false)
{
	const Config cfg = Cfg();
	if (static_cast<int>(s_active.size()) >= cfg.maxActive)
		return;
	Active a;
	a.intensity = IntensityFor(spellId, caster, cfg);
	a.layer = &l;
	a.pal = pal;
	a.caster = caster;
	a.target = target ? target : caster;
	a.spellId = spellId;
	a.castSerial = serial;
	a.birth = birth;
	a.life = life;
	a.aeRange = aeRange;
	a.phase = PhaseFor(static_cast<uint32_t>(caster * 131 + spellId * 7 + static_cast<int>(s_active.size())));
	a.persistent = persistent;
	a.until = birth + life;
	a.shownSince = birth;
	a.lastA = BodyOf(caster);
	a.lastB = BodyOf(a.target);
	s_active.push_back(a);
}

bool CoveredByPersist(int spawnId, const Config& cfg);

void RefreshPersist(int spawnId, int spellId, float when)
{
	for (Active& a : s_active)
		if (a.persistent && a.target == spawnId && a.spellId == spellId)
			a.shownSince = when;
}

void ScheduleLook(const SpellLook& look, const Classified& c, int caster, int target, int spellId,
	uint32_t serial, float release, float travel, const Config& cfg, bool skipCaster = false, bool force = false)
{
	for (int i = 0; i < look.count; ++i)
	{
		const Layer& l = look.layers[i];
		float birth = release + l.delay;
		int anchorTarget = target;
		switch (l.stage)
		{
		case Stage::Charge:
			continue;
		case Stage::Release:
			if (skipCaster) continue;
			anchorTarget = caster;
			break;
		case Stage::Travel:
			if ((!cfg.travel && !force) || !target || target == caster) continue;
			break;
		case Stage::Impact:
			if ((!cfg.impacts && !force) || (!target && c.needsTarget)) continue;
			birth += travel;
			if (!target) anchorTarget = caster;
			break;
		case Stage::Aura:
			if ((!cfg.auras && !force) || (!target && c.needsTarget)) continue;
			birth += travel + 0.15f;
			if (!target) anchorTarget = caster;
			if (l.persist && !force && CoveredByPersist(anchorTarget, cfg))
			{
				RefreshPersist(anchorTarget, spellId, birth);
				continue;
			}
			break;
		}
		if (c.beam && l.stage == Stage::Impact)
			anchorTarget = caster;
		float life = StageLife(l, cfg);
		if (l.stage == Stage::Travel)
			life = std::max(travel / 0.7f, 0.25f);
		Push(l, c.pal, caster, anchorTarget, spellId, serial, birth, life, c.aeRange);
		if (s_active.empty() || s_active.back().layer != &l)
			continue;
		Active& a = s_active.back();
		if (l.stage == Stage::Aura && (l.flags & FxFlag_NoEnvelope))
			a.until = birth + cfg.auraTime;
		if (c.beam)
			a.beamRange = c.range > 1.0f ? std::min(c.range, 200.0f) : 60.0f;
	}
}

bool CoveredByPersist(int spawnId, const Config& cfg)
{
	PlayerClient* me = pLocalPlayer.get();
	if (!me || cfg.persist <= 0)
		return false;
	if (spawnId == static_cast<int>(me->SpawnID))
		return true;
	PlayerClient* sp = GetSpawnByID(spawnId);
	return cfg.persist >= 2 && sp && IsInGroup(sp);
}

float TravelTime(int caster, int target)
{
	const Body a = BodyOf(caster), b = BodyOf(target);
	if (!a.valid || !b.valid || caster == target)
		return 0.0f;
	const float dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
	return std::clamp(std::sqrt(dx * dx + dy * dy + dz * dz) / 140.0f, 0.2f, 0.8f);
}

void StartCast(int spellId, int caster, int target, uint32_t castMs, bool fromSelfPoll)
{
	const Config cfg = Cfg();
	if (!cfg.enabled || spellId <= 0 || caster <= 0 || !InScope(caster, cfg))
		return;

	const Classified& c = Classify(spellId);
	const float now = FxClockNow();
	const float castSec = castMs > 100 ? castMs / 1000.0f : 0.0f;
	const float release = now + castSec;
	const uint32_t serial = ++s_serial;
	s_statCasts.fetch_add(1, std::memory_order_relaxed);

	if (!c.needsTarget)
		target = caster;

	if (cfg.charge && castSec > 0.25f)
	{
		const SpellLook* ch = ChargeLook(c.charge);
		for (int i = 0; i < ch->count; ++i)
			Push(ch->layers[i], c.pal, caster, caster, spellId, serial, now, castSec, c.aeRange);
	}

	const float travel = target ? TravelTime(caster, target) : 0.0f;
	for (int i = 0; i < c.count; ++i)
		ScheduleLook(*c.looks[i], c, caster, target, spellId, serial, release, travel, cfg);

	if (castSec > 0.25f)
		s_pending.push_back({ serial, caster, spellId, target, now, release, fromSelfPoll });

	PruneRecent(now);
	if (target)
		s_recent.push_back({ spellId, target, release + travel });
	if ((c.casterArea || c.targetArea) && !c.beneficial)
		s_recent.push_back({ spellId, -caster, release + travel });
}

void Interrupt(const PendingCast& pc)
{
	const float now = FxClockNow();
	s_active.erase(std::remove_if(s_active.begin(), s_active.end(), [&](const Active& a) {
		return a.castSerial == pc.serial && (a.birth > now || a.layer->stage == Stage::Charge);
	}), s_active.end());

	const Classified& c = Classify(pc.spellId);
	for (int i = 0; i < kFizzle.count; ++i)
		Push(kFizzle.layers[i], c.pal, pc.caster, pc.caster, pc.spellId, 0, now, kFizzle.layers[i].life, 0);
}

void PulsePending()
{
	const float now = FxClockNow();
	for (auto it = s_pending.begin(); it != s_pending.end(); )
	{
		if (now >= it->release - 0.15f)
		{
			it = s_pending.erase(it);
			continue;
		}
		PlayerClient* sp = GetSpawnByID(it->caster);
		if (!sp)
		{
			it = s_pending.erase(it);
			continue;
		}
		const int cur = sp->CastingData.SpellID;
		if (cur == it->spellId)
		{
			it->seenActive = true;
			++it;
			continue;
		}
		if (it->seenActive)
		{
			Interrupt(*it);
			it = s_pending.erase(it);
			continue;
		}
		++it;
	}
}

void PollSelfCast()
{
	PlayerClient* me = pLocalPlayer.get();
	CDisplay* display = pDisplay.get();
	if (!me || !display)
		return;

	const int spell = me->CastingData.SpellID;
	const uint32_t eta = me->CastingData.SpellETA;
	if (spell <= 0)
	{
		s_lastSelfSpell = -1;
		return;
	}
	if (spell == s_lastSelfSpell && eta == s_lastSelfEta)
		return;
	s_lastSelfSpell = spell;
	s_lastSelfEta = eta;

	const uint32_t left = eta > display->TimeStamp ? eta - display->TimeStamp : 0;
	const int target = pTarget ? pTarget->SpawnID : me->SpawnID;
	StartCast(spell, me->SpawnID, target, left, true);
}

struct AbilityWatch
{
	int index;
	int spellId;
	int remaining;
};

std::vector<AbilityWatch> s_abilities;
float s_abilitiesListed = -1000.0f;
float s_abilitiesPolled = 0.0f;

void ListAbilities()
{
	s_abilities.clear();
	PcClient* pc = pLocalPC.get();
	PlayerClient* me = pLocalPlayer.get();
	if (!pc || !me || !pAltAdvManager)
		return;
	for (int i = 0; i < AA_CHAR_MAX_REAL; ++i)
	{
		const int index = pc->GetAlternateAbilityId(i);
		CAltAbilityData* ab = index > 0 ? GetAAById(index, me->Level) : nullptr;
		if (ab && ab->SpellID > 0 && ab->ReuseTimer > 0)
			s_abilities.push_back({ index, ab->SpellID, -1 });
	}
}

void PollAbilities(float now)
{
	if (now - s_abilitiesListed > 30.0f || now < s_abilitiesListed)
	{
		s_abilitiesListed = now;
		ListAbilities();
	}
	if (now - s_abilitiesPolled < 0.2f && now >= s_abilitiesPolled)
		return;
	s_abilitiesPolled = now;

	PcClient* pc = pLocalPC.get();
	PlayerClient* me = pLocalPlayer.get();
	if (!pc || !me || !pAltAdvManager)
		return;
	for (AbilityWatch& w : s_abilities)
	{
		CAltAbilityData* ab = GetAAById(w.index, me->Level);
		if (!ab)
			continue;
		int remaining = 0;
		pAltAdvManager->IsAbilityReady(pc, ab, &remaining);
		remaining = std::max(remaining, 0);
		const bool restarted = w.remaining >= 0 && remaining > w.remaining + 2;
		w.remaining = remaining;
		if (!restarted)
			continue;
		const bool started = std::any_of(s_recent.begin(), s_recent.end(), [&](const Recent& r) {
			return r.spellId == w.spellId && std::fabs(now - r.when) < 3.0f;
		});
		if (!started && me->CastingData.SpellID != w.spellId)
		{
			const int target = pTarget ? pTarget->SpawnID : me->SpawnID;
			StartCast(w.spellId, me->SpawnID, target, 0, true);
		}
	}
}

std::vector<int> s_selfBuffs;
float s_selfBuffsAt = -1000.0f;

void NoteSelfBuffs(const std::vector<int>& now, float t);

void MarkPersist(int spawnId, int spellId, float now)
{
	const Classified& c = Classify(spellId);
	if (!c.hasPersist)
		return;
	for (int li = 0; li < c.count; ++li)
	{
		const SpellLook& look = *c.looks[li];
		for (int i = 0; i < look.count; ++i)
		{
			const Layer& l = look.layers[i];
			if (!l.persist)
				continue;
			auto it = std::find_if(s_active.begin(), s_active.end(), [&](const Active& a) {
				return a.persistent && a.target == spawnId && a.layer == &l;
			});
			if (it != s_active.end())
			{
				it->seen = true;
				it->fadeStart = -1.0f;
				continue;
			}
			Push(l, c.pal, spawnId, spawnId, spellId, 0, now, l.life > 0 ? l.life : 0.0f, c.aeRange, true);
		}
	}
}

void ScanBuffs(const Config& cfg)
{
	const float now = FxClockNow();
	for (Active& a : s_active)
		if (a.persistent)
			a.seen = false;

	if (cfg.enabled)
	{
		PlayerClient* me = pLocalPlayer.get();
		PcClient* pc = pLocalPC.get();
		if (me && pc)
		{
			const int n = pc->GetMaxEffects();
			std::vector<int> ids;
			for (int i = 0; i < n; ++i)
			{
				const int id = pc->GetEffect(i).SpellID;
				if (id <= 0)
					continue;
				ids.push_back(id);
				if (cfg.persist > 0)
					MarkPersist(me->SpawnID, id, now);
			}
			NoteSelfBuffs(ids, now);
		}

		if (cfg.persist >= 2 && pc && pc->Group)
		{
			for (int g = 1; g < MAX_GROUP_SIZE; ++g)
			{
				CGroupMember* gm = pc->Group->GetGroupMember(g);
				PlayerClient* sp = gm ? gm->GetPlayer() : nullptr;
				if (!sp)
					continue;
				for (const CachedBuff& cb : FilterCachedBuffs(sp, [](const CachedBuff& b) {
					return b.spellId > 0 && b.Duration() > 0; }))
				{
					MarkPersist(sp->SpawnID, cb.spellId, now);
				}
			}
		}
	}

	for (Active& a : s_active)
		if (a.persistent && !a.seen && a.fadeStart < 0.0f)
			a.fadeStart = now;
}

uint32_t SegsFor(const Layer& l)
{
	auto items = [](float v) { return std::max(static_cast<uint32_t>(v), 1u); };
	uint32_t n = l.segs;
	switch (l.form)
	{
	case FxForm_Rune:    n = 52 + std::clamp(items(l.p0), 3u, 16u) + std::min(static_cast<uint32_t>(l.p2), 24u); break;
	case FxForm_Burst:   n = std::clamp(items(l.p0), 1u, 3u) * 32 + std::min(static_cast<uint32_t>(l.p2), 24u); break;
	case FxForm_Dome:    n = std::clamp(items(l.p0), 2u, 10u) * 8 + std::clamp(items(l.p1), 1u, 4u) * 16; break;
	case FxForm_Wings:   n = 2 * (8 + 3 * std::clamp(items(l.p0), 3u, 12u)); break;
	case FxForm_Crystal: n = items(l.p0) * 5; break;
	case FxForm_Bubbles: n = items(l.p0) * 8; break;
	case FxForm_Wind:    n = items(l.p0) * 6; break;
	case FxForm_Sparkle: n = items(l.p0) * 2; break;
	case FxForm_Rain:    n = items(l.p0); break;
	case FxForm_Cracks:  n = items(l.p0) * 8; break;
	case FxForm_Thorns:  n = items(l.p0) * items(l.p2); break;
	case FxForm_Drain:   n = items(l.p0) * std::max(items(l.p2), 4u); break;
	case FxForm_Flame:
	case FxForm_Vines:
	case FxForm_Vortex:
	case FxForm_Lightning:
	case FxForm_Bolt:
	{
		const uint32_t k = items(l.p0);
		n = k * std::max(std::min(l.segs, kMaxSegsPerInstance) / k, 1u);
		break;
	}
	default: break;
	}
	return std::min(n, kMaxSegsPerInstance);
}

const Pair& ColorsFor(const Active& a)
{
	switch (a.layer->col)
	{
	case Col::Primary: return a.pal.primary;
	case Col::Accent:  return a.pal.accent;
	default:           return a.layer->fixed;
	}
}

Layer Densified(const Layer& src, float intensity)
{
	Layer l = src;
	const float k = 0.5f + intensity;
	auto more = [k](float v) { return std::max(std::round(v * k), 1.0f); };
	switch (l.form)
	{
	case FxForm_Sparkle:
	case FxForm_Bubbles:
	case FxForm_Rain:
	case FxForm_Crystal:
	case FxForm_Wind:
	case FxForm_Cracks:
	case FxForm_Spire:
		l.p0 = more(l.p0);
		break;
	case FxForm_Flame:
	case FxForm_Orbit:
		l.p0 = more(l.p0);
		l.segs = static_cast<uint32_t>(more(static_cast<float>(l.segs)));
		break;
	case FxForm_Burst:
		l.p2 = std::round(l.p2 * k);
		break;
	default:
		break;
	}
	return l;
}

void Emit(const Active& a, const Body& A, const Body& B, float now, float alphaScale, float sizeScale,
	const AlwaysOnStyle& aura, std::vector<FxInstance>& out)
{
	const Layer l = Densified(*a.layer, a.intensity);
	const bool onCaster = l.stage == Stage::Charge || l.stage == Stage::Release;
	const Body& at = onCaster ? A : B;
	const float s = at.scale * sizeScale * (0.8f + 0.4f * a.intensity);
	alphaScale *= std::min(1.0f, 0.75f + 0.5f * a.intensity);

	FxInstance ix = {};
	ix.pos[0] = at.x; ix.pos[1] = at.y; ix.pos[2] = at.z;
	ix.radius = l.radius * s;
	if (l.size == Size::AERange)
		ix.radius = l.radius * (a.aeRange > 1.0f ? std::min(a.aeRange, 120.0f) : 3.0f * 6.0f * at.scale);
	ix.height = l.height * s;
	ix.phase = a.phase;
	ix.speed = l.speed;
	ix.width = l.width * std::sqrt(s);
	ix.form = l.form;
	ix.flags = l.flags;
	ix.p0 = l.p0; ix.p1 = l.p1; ix.p2 = l.p2; ix.p3 = l.p3; ix.p4 = l.p4 * s;
	ix.yaw = at.yaw;
	ix.birth = a.birth;
	ix.life = a.life;

	switch (l.form)
	{
	case FxForm_Wings:   ix.p2 = l.p2 * s; break;
	case FxForm_Sparkle:
	case FxForm_Bubbles: ix.p1 = l.p1 * s; break;
	case FxForm_Wind:    ix.p1 = l.p1 * s; break;
	case FxForm_Bolt:    ix.p1 = l.p1 * s; ix.p3 = l.p3 * s; break;
	case FxForm_Drain:   ix.p1 = l.p1 * s; break;
	case FxForm_Burst:   ix.p1 = l.p1 * s; break;
	default: break;
	}

	if (l.form == FxForm_Bolt || l.form == FxForm_Drain || l.form == FxForm_Lightning)
	{
		const float midA = A.z + 3.2f * A.scale, midB = B.z + 3.2f * B.scale;
		if (l.form == FxForm_Drain)
		{
			ix.pos[0] = B.x; ix.pos[1] = B.y; ix.pos[2] = midB;
			ix.pos2[0] = A.x; ix.pos2[1] = A.y; ix.pos2[2] = midA;
		}
		else if (l.form == FxForm_Lightning && l.p3 > 0.5f)
		{
			ix.pos2[0] = B.x; ix.pos2[1] = B.y; ix.pos2[2] = B.z;
			ix.height = l.height * B.scale;
		}
		else if (a.beamRange > 0.0f)
		{
			ix.pos[0] = A.x; ix.pos[1] = A.y; ix.pos[2] = midA;
			ix.pos2[0] = A.x + std::cos(A.yaw) * a.beamRange;
			ix.pos2[1] = A.y + std::sin(A.yaw) * a.beamRange;
			ix.pos2[2] = midA;
		}
		else
		{
			ix.pos[0] = A.x; ix.pos[1] = A.y; ix.pos[2] = midA;
			ix.pos2[0] = B.x; ix.pos2[1] = B.y; ix.pos2[2] = midB;
		}
	}

	const Pair& p = ColorsFor(a);
	float alpha = alphaScale;
	if (a.persistent)
	{
		alpha *= AlwaysOnFade(aura, now - a.shownSince, a.phase);
		if (a.fadeStart >= 0.0f)
			alpha *= std::max(0.0f, 1.0f - (now - a.fadeStart) / 0.6f);
		const float motion = aura.motion;
		if (motion > 0.0f && l.form != FxForm_Glyph)
		{
			ix.flags |= FxFlag_Drift;
			ix.p5 = l.form == FxForm_Wings ? 0.0f : 0.35f * motion * (a.phase < 0.5f ? 1.0f : -1.0f);
		}
	}
	else if (!a.persistent && a.until > a.birth + a.life + 0.01f)
		alpha *= std::clamp((a.until - now) / 0.6f, 0.0f, 1.0f);
	if (alpha * std::max(p.c1.a, p.c2.a) <= 1e-3f)
		return;
	ix.rgba[0] = p.c1.r; ix.rgba[1] = p.c1.g; ix.rgba[2] = p.c1.b; ix.rgba[3] = p.c1.a * alpha;
	ix.rgba2[0] = p.c2.r; ix.rgba2[1] = p.c2.g; ix.rgba2[2] = p.c2.b; ix.rgba2[3] = p.c2.a * alpha;

	if (l.form == FxForm_Glyph && a.glyphSlot >= 0)
	{
		ix.p5 = static_cast<float>(a.glyphSlot);
		ix.segments = GlyphSegments(static_cast<uint32_t>(l.p0));
		if (out.size() < kMaxInstances)
			out.push_back(ix);
		return;
	}

	if (l.form == FxForm_Glyph)
	{
		const uint32_t cnt = std::max(GlyphSegments(static_cast<uint32_t>(l.p0)), 1u);
		const uint32_t copies = std::max(static_cast<uint32_t>(l.p2), 1u);
		const uint32_t perInst = std::max(kMaxSegsPerInstance / cnt, 1u);
		for (uint32_t first = 0; first < copies && out.size() < kMaxInstances; first += perInst)
		{
			FxInstance g = ix;
			g.p5 = static_cast<float>(first);
			g.segments = std::min(copies - first, perInst) * cnt;
			out.push_back(g);
		}
		return;
	}

	ix.segments = SegsFor(l);
	if (out.size() < kMaxInstances)
		out.push_back(ix);
}

} // namespace

namespace {

void NoteSelfCastCall(int spellId, int castingBefore)
{
	PlayerClient* me = pLocalPlayer.get();
	if (!me || spellId <= 0)
		return;
	if (castingBefore > 0 && castingBefore != spellId)
		return;
	if (me->CastingData.SpellID == spellId)
		return;
	EQ_Spell* sp = GetSpellByID(spellId);
	if (!sp || sp->CastTime > 500)
		return;
	const int target = pTarget ? pTarget->SpawnID : me->SpawnID;
	StartCast(spellId, me->SpawnID, target, 0, true);
}

using u64 = uintptr_t;

class CastSpellHook
{
public:
	DETOUR_TRAMPOLINE_DEF_MEMBER(CastSpellHook, u64, CastSpell_Trampoline,
		(u64, u64, u64, u64, u64, u64, u64, u64, u64, u64, u64, u64, u64, u64, u64))
	u64 CastSpell_Detour(u64 gem, u64 spell, u64 a4, u64 a5, u64 a6, u64 a7, u64 a8, u64 a9,
		u64 a10, u64 a11, u64 a12, u64 a13, u64 a14, u64 a15, u64 a16)
	{
		const int spellId = static_cast<int>(spell & 0xFFFFFFFF);
		const int before = pLocalPlayer ? pLocalPlayer->CastingData.SpellID : -1;
		const u64 r = CastSpell_Trampoline(gem, spell, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
		if (r & 0xFF)
			NoteSelfCastCall(spellId, before);
		return r;
	}
};

uintptr_t s_castSpellAddr = 0;
bool      s_castSpellTried = false;

} // namespace

void InstallSelfCastHook()
{
	if (s_castSpellAddr || s_castSpellTried)
		return;
	s_castSpellTried = true;
	const uintptr_t addr = eqgfx::scan::Find(eqgfx::scan::Game(), eqgfx::sig::CharacterZoneClient__CastSpell);
	if (!addr)
		return;
	if (EzDetour(addr, &CastSpellHook::CastSpell_Detour, &CastSpellHook::CastSpell_Trampoline))
		s_castSpellAddr = addr;
}

void RemoveSelfCastHook()
{
	if (!s_castSpellAddr)
		return;
	RemoveDetour(s_castSpellAddr);
	s_castSpellAddr = 0;
}

float AlwaysOnFade(const AlwaysOnStyle& style, float shownFor, float phase)
{
	float a = style.opacity;
	if (style.fadeAfter > 0.0f && shownFor > style.fadeAfter)
	{
		const float k = std::clamp((shownFor - style.fadeAfter) / 2.0f, 0.0f, 1.0f);
		a *= 1.0f + (style.fadeTo - 1.0f) * k;
	}
	if (style.breathe)
	{
		const float w = 0.5f + 0.5f * std::sin(FxClockNow() * 1.2f + phase * 6.2831853f);
		a *= 0.35f + 0.65f * w;
	}
	return a;
}

bool AlwaysOnStyleOptions(AlwaysOnStyle& style)
{
	bool changed = false;
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("opacity", &style.opacity, 0.05f, 1.0f, "%.2f");
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("motion", &style.motion, 0.0f, 2.0f, "%.2f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("0 holds it still; higher turns, bobs and wanders it around the body");
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("fade after", &style.fadeAfter, 0.0f, 300.0f,
		style.fadeAfter > 0.0f ? "%.0f s" : "never");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("seconds it shows at full strength before fading");
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("faded opacity", &style.fadeTo, 0.0f, 1.0f, style.fadeTo > 0.0f ? "%.2f" : "hidden");
	changed |= ImGui::Checkbox("breathe", &style.breathe);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("opacity slowly swells and ebbs");
	return changed;
}

void SpellFxNoteCastStart(int spellId, int spawnId, uint32_t castTimeMs)
{
	if (!pLocalPlayer || spawnId <= 0)
		return;
	if (spawnId == static_cast<int>(pLocalPlayer->SpawnID))
		return;

	int target = 0;
	if (pTarget && static_cast<int>(pTarget->SpawnID) == spawnId && pLocalPlayer->TargetOfTarget)
		target = static_cast<int>(pLocalPlayer->TargetOfTarget);
	StartCast(spellId, spawnId, target, castTimeMs, false);
}

void SpellFxNoteLanded(int casterId, int targetId, int spellId, bool heal)
{
	(void)heal;
	const Config cfg = Cfg();
	if (!cfg.enabled || !cfg.landings || spellId <= 0 || targetId <= 0)
		return;
	if (!InScope(casterId > 0 ? casterId : targetId, cfg))
		return;

	const float now = FxClockNow();
	const Classified& cl = Classify(spellId);
	const bool areaOnce = (cl.casterArea || cl.targetArea) && !cl.beneficial;
	const int key = areaOnce ? -(casterId > 0 ? casterId : targetId) : targetId;

	PruneRecent(now);
	for (const Recent& r : s_recent)
		if (r.spellId == spellId && (r.target == targetId || r.target == key) && std::fabs(now - r.when) < 2.5f)
			return;
	s_recent.push_back({ spellId, key, now });

	const Classified& c = cl;
	const int caster = casterId > 0 ? casterId : targetId;
	const float travel = 0.0f;
	for (int i = 0; i < c.count; ++i)
	{
		const SpellLook& look = *c.looks[i];
		for (int j = 0; j < look.count; ++j)
		{
			const Layer& l = look.layers[j];
			if (l.stage != Stage::Impact && l.stage != Stage::Aura)
				continue;
			if ((l.stage == Stage::Impact && !cfg.impacts) || (l.stage == Stage::Aura && !cfg.auras))
				continue;
			if (l.persist && CoveredByPersist(targetId, cfg))
				continue;
			Push(l, c.pal, caster, targetId, spellId, 0, now + l.delay + travel, StageLife(l, cfg), c.aeRange);
		}
	}
}

namespace {

void NoteSelfBuffs(const std::vector<int>& ids, float t)
{
	PlayerClient* me = pLocalPlayer.get();
	if (me && t >= s_selfBuffsAt && t - s_selfBuffsAt < 2.0f)
	{
		std::vector<int> added;
		for (int id : ids)
			if (std::find(s_selfBuffs.begin(), s_selfBuffs.end(), id) == s_selfBuffs.end())
				added.push_back(id);
		if (added.size() <= 3)
			for (int id : added)
				SpellFxNoteLanded(0, me->SpawnID, id, false);
	}
	s_selfBuffs = ids;
	s_selfBuffsAt = t;
}

} // namespace

void PulseSpellFx(std::vector<FxInstance>& out)
{
	const Config cfg = Cfg();
	const float now = FxClockNow();

	FlushCfg(false);

	if (cfg.enabled)
	{
		PollSelfCast();
		PollAbilities(now);
		PulsePending();
	}

	if (now - s_lastBuffScan > 0.5f || now < s_lastBuffScan)
	{
		s_lastBuffScan = now;
		ScanBuffs(cfg);
	}

	s_active.erase(std::remove_if(s_active.begin(), s_active.end(), [&](const Active& a) {
		if (a.persistent)
			return a.fadeStart >= 0.0f && now - a.fadeStart > 0.6f;
		return now > a.until + 0.1f;
	}), s_active.end());

	s_statActive.store(static_cast<uint32_t>(s_active.size()), std::memory_order_relaxed);
	if (!cfg.enabled)
		return;

	for (Active& a : s_active)
	{
		Body A = BodyOf(a.caster), B = a.target == a.caster ? A : BodyOf(a.target);
		if (A.valid) a.lastA = A; else A = a.lastA;
		if (B.valid) a.lastB = B; else B = a.lastB;
		if (!A.valid && !B.valid)
			continue;
		if (!B.valid) B = A;
		if (!A.valid) A = B;
		Emit(a, A, B, now, a.persistent ? 1.0f : cfg.alpha, cfg.size, cfg.aura, out);
		if (out.size() >= kMaxInstances)
			break;
	}
}

void LoadSpellFxSettings()
{
	LoadCfg();
}

void FlushSpellFxSettings()
{
	FlushCfg(true);
}

void ForgetSpellFx()
{
	s_active.clear();
	s_pending.clear();
	s_recent.clear();
	s_lastSelfSpell = -1;
	s_selfBuffs.clear();
	s_selfBuffsAt = -1000.0f;
	s_abilities.clear();
	s_abilitiesListed = -1000.0f;
}

void OnSpellFxSpawnRemoved(int spawnId)
{
	s_active.erase(std::remove_if(s_active.begin(), s_active.end(), [&](const Active& a) {
		return a.persistent && a.target == spawnId;
	}), s_active.end());
	s_pending.erase(std::remove_if(s_pending.begin(), s_pending.end(), [&](const PendingCast& p) {
		return p.caster == spawnId;
	}), s_pending.end());
}

int SpellFxLookCount()
{
	return static_cast<int>(sizeof(kAllLooks) / sizeof(kAllLooks[0]));
}

bool SpellFxEnabled()
{
	return Cfg().enabled;
}

void SetSpellFxEnabled(bool on)
{
	Config c = Cfg();
	c.enabled = on;
	{
		std::scoped_lock lock(s_cfgMutex);
		s_cfg = c;
	}
	SaveCfg(c);
	if (!on)
		ForgetSpellFx();
}

void SpellFxPlay(int spellId, int casterId, int targetId, uint32_t castMs)
{
	if (casterId <= 0 && pLocalPlayer)
		casterId = pLocalPlayer->SpawnID;
	const Config cfg = Cfg();
	if (!cfg.enabled || spellId <= 0 || casterId <= 0)
		return;
	const Classified& c = Classify(spellId);
	const float now = FxClockNow();
	const float castSec = castMs / 1000.0f;
	const uint32_t serial = ++s_serial;
	const int target = c.needsTarget ? targetId : casterId;
	if (castSec > 0.25f)
	{
		const SpellLook* ch = ChargeLook(c.charge);
		for (int i = 0; i < ch->count; ++i)
			Push(ch->layers[i], c.pal, casterId, casterId, spellId, serial, now, castSec, c.aeRange);
	}
	const float travel = target ? TravelTime(casterId, target) : 0.0f;
	for (int i = 0; i < c.count; ++i)
		ScheduleLook(*c.looks[i], c, casterId, target, spellId, serial, now + castSec, travel, cfg);
}

std::string SpellFxDescribe(int spellId)
{
	const Classified& c = Classify(spellId);
	std::string s;
	for (int i = 0; i < c.count; ++i)
	{
		if (i) s += " + ";
		s += c.looks[i]->name;
	}
	return s;
}

bool SpellFxCommand(const char* szLine)
{
	char a1[MAX_STRING] = { 0 }, a2[MAX_STRING] = { 0 };
	GetArg(a1, szLine, 2);
	GetArg(a2, szLine, 3);

	Config c = Cfg();
	bool changed = false;

	auto resolve = [](const char* arg) -> int {
		if (!arg || !*arg)
			return 0;
		if (const int id = GetIntFromString(arg, 0))
			return id;
		EQ_Spell* sp = GetSpellByName(arg);
		return sp ? sp->ID : 0;
	};

	if (ci_equals(a1, "on"))              { c.enabled = true; changed = true; }
	else if (ci_equals(a1, "off"))        { c.enabled = false; changed = true; ForgetSpellFx(); }
	else if (ci_equals(a1, "self"))       { c.scope = 0; changed = true; }
	else if (ci_equals(a1, "group"))      { c.scope = 1; changed = true; }
	else if (ci_equals(a1, "all"))        { c.scope = 2; changed = true; }
	else if (ci_equals(a1, "radius"))     { c.range = std::max(10.0f, static_cast<float>(atof(a2))); changed = true; }
	else if (ci_equals(a1, "duration"))   { c.duration = ClampF(static_cast<float>(atof(a2)), 0.5f, 4.0f, c.duration); changed = true; }
	else if (ci_equals(a1, "motion"))     { c.aura.motion = ClampF(static_cast<float>(atof(a2)), 0.0f, 2.0f, c.aura.motion); changed = true; }
	else if (ci_equals(a1, "fade"))
	{
		char a3[MAX_STRING] = { 0 };
		GetArg(a3, szLine, 4);
		c.aura.fadeAfter = ci_equals(a2, "off") ? 0.0f : ClampF(static_cast<float>(atof(a2)), 0.0f, 600.0f, c.aura.fadeAfter);
		if (a3[0]) c.aura.fadeTo = ClampF(static_cast<float>(atof(a3)), 0.0f, 1.0f, c.aura.fadeTo);
		changed = true;
	}
	else if (ci_equals(a1, "breathe"))    { c.aura.breathe = !c.aura.breathe; changed = true; }
	else if (ci_equals(a1, "intensity"))
	{
		c.levelScale = !ci_equals(a2, "off");
		if (c.levelScale && a2[0])
			c.levelStrength = ClampF(static_cast<float>(atof(a2)), 0.0f, 1.0f, c.levelStrength);
		changed = true;
	}
	else if (ci_equals(a1, "persist"))
	{
		c.persist = ci_equals(a2, "off") ? 0 : ci_equals(a2, "self") ? 1 : 2;
		changed = true;
	}
	else if (ci_equals(a1, "list"))
	{
		std::string line;
		for (const SpellLook* l : kAllLooks)
		{
			if (!line.empty()) line += ", ";
			line += l->name;
			if (line.size() > 200)
			{
				WriteChatf("\ag[MQEQGFX]\ax %s", line.c_str());
				line.clear();
			}
		}
		if (!line.empty())
			WriteChatf("\ag[MQEQGFX]\ax %s", line.c_str());
		WriteChatf("\ag[MQEQGFX]\ax %d spell looks, %d class line looks, %d glyphs, %d GPU forms",
			SpellFxLookCount(), LineLookCount(), static_cast<int>(Glyph_Count), static_cast<int>(FxForm_Count));
		return true;
	}
	else if (ci_equals(a1, "why"))
	{
		const int id = resolve(GetNextArg(szLine, 2));
		if (!id)
		{
			WriteChatf("\ag[MQEQGFX]\ax usage: /eqgfx spells why <spell id|name>");
			return true;
		}
		const Classified& cl = Classify(id);
		EQ_Spell* sp = GetSpellByID(id);
		const int me = pLocalPlayer ? pLocalPlayer->SpawnID : 0;
		WriteChatf("\ag[MQEQGFX]\ax %s (%d): \ay%s\ax - %s, charge %s, %s%s%s, intensity %.2f",
			sp ? sp->Name : "?", id, SpellFxDescribe(id).c_str(), cl.why.c_str(),
			ChargeLook(cl.charge)->name, cl.beneficial ? "beneficial" : "detrimental",
			cl.needsTarget ? ", lands on the target" : ", on the caster",
			cl.hasPersist ? ", persistent aura" : "", IntensityFor(id, me, Cfg()));
		return true;
	}
	else if (ci_equals(a1, "test"))
	{
		const int id = resolve(GetNextArg(szLine, 2));
		if (!id || !pLocalPlayer)
		{
			WriteChatf("\ag[MQEQGFX]\ax usage: /eqgfx spells test <spell id|name> - plays it from you at your target");
			return true;
		}
		const int target = pTarget ? pTarget->SpawnID : pLocalPlayer->SpawnID;
		SpellFxPlay(id, pLocalPlayer->SpawnID, target, 2000);
		WriteChatf("\ag[MQEQGFX]\ax playing \ay%s\ax", SpellFxDescribe(id).c_str());
		return true;
	}
	else if (ci_equals(a1, "clear"))
	{
		ForgetSpellFx();
		WriteChatf("\ag[MQEQGFX]\ax spell animations cleared");
		return true;
	}

	if (changed)
	{
		{
			std::scoped_lock lock(s_cfgMutex);
			s_cfg = c;
		}
		SaveCfg(c);
	}

	static const char* kScope[] = { "you", "you, group and target", "everyone" };
	static const char* kPersist[] = { "off", "your buffs", "your and your group's buffs" };
	WriteChatf("\ag[MQEQGFX]\ax spell animations %s | %s within %.0f | persistent auras: %s | %u live, %u casts seen",
		c.enabled ? "\agon\ax" : "\ayoff\ax", kScope[c.scope], c.range, kPersist[c.persist],
		s_statActive.load(), s_statCasts.load());
	WriteChatf("\ag[MQEQGFX]\ax lasting x%.1f | level scaling %s | persistent auras: motion %.2f, opacity %.2f, fade %s to %.2f%s",
		c.duration, c.levelScale ? "on" : "off", c.aura.motion, c.aura.opacity, c.aura.fadeAfter > 0 ? "after " : "never", c.aura.fadeTo,
		c.aura.breathe ? ", breathing" : "");
	WriteChatf("\ag[MQEQGFX]\ax /eqgfx spells on|off|self|group|all|radius <n>|persist off|self|group|"
		"duration <x>|intensity <0-1|off>|motion <0-2>|fade <sec|off> [opacity]|breathe|list|why <spell>|"
		"test <spell>|clear");
	return true;
}

void DrawSpellFxOptions()
{
	Config c = Cfg();
	bool changed = false;

	changed |= ImGui::Checkbox("spell animations", &c.enabled);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("native cast, travel, impact and aura animations, picked from what each spell does");

	static const char* kScope[] = { "only mine", "mine, group and target", "everyone in range" };
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::Combo("whose casts", &c.scope, kScope, 3);
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("range", &c.range, 20.0f, 600.0f, "%.0f");

	changed |= ImGui::Checkbox("charge", &c.charge);
	ImGui::SameLine();
	changed |= ImGui::Checkbox("travel", &c.travel);
	ImGui::SameLine();
	changed |= ImGui::Checkbox("impacts", &c.impacts);
	ImGui::SameLine();
	changed |= ImGui::Checkbox("auras", &c.auras);

	changed |= ImGui::Checkbox("impacts from others' landings", &c.landings);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("use the hit/heal feed to place impacts for spells whose target we could not know");

	static const char* kPersist[] = { "off", "my buffs", "my and my group's buffs" };
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::Combo("persistent auras", &c.persist, kPersist, 3);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("wings while levitating, flames while a fire shield is up, and so on");

	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("opacity", &c.alpha, 0.05f, 1.0f, "%.2f");
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("size", &c.size, 0.25f, 3.0f, "%.2f");
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("effect duration", &c.duration, 0.5f, 4.0f, "x%.1f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("how long impacts, releases and auras linger");
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("aura seconds", &c.auraTime, 1.0f, 60.0f, "%.0f");
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderInt("max live", &c.maxActive, 50, 2000);

	changed |= ImGui::Checkbox("scale with spell level", &c.levelScale);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("higher level spells animate bigger, denser and brighter; abilities use the caster's level");
	if (c.levelScale)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(120.0f);
		changed |= ImGui::SliderFloat("##levelstrength", &c.levelStrength, 0.0f, 1.0f, "strength %.2f");
	}

	ImGui::SeparatorText("persistent buff auras");
	ImGui::TextDisabled("their own opacity and fading; the class animations have theirs under World effects");
	ImGui::PushID("aura");
	changed |= AlwaysOnStyleOptions(c.aura);
	ImGui::PopID();

	ImGui::TextDisabled("%u live, %u casts seen, %d looks, %d class line looks", s_statActive.load(),
		s_statCasts.load(), SpellFxLookCount(), LineLookCount());

	if (changed)
	{
		{
			std::scoped_lock lock(s_cfgMutex);
			s_cfg = c;
		}
		SaveCfg(c);
	}
}

namespace {

std::deque<Layer> s_demoLayers;

Classified DemoClass(const SpellLook& look, const Pair& primary = kArcane)
{
	Classified c;
	c.looks[0] = &look;
	c.count = 1;
	c.pal = Palette{ primary, kEarth };
	c.needsTarget = true;
	c.aeRange = 30.0f;
	c.range = 60.0f;
	c.beam = &look == &kBeam;
	return c;
}

} // namespace

const char* SpellFxLookName(int index)
{
	return index >= 0 && index < SpellFxLookCount() ? kAllLooks[index]->name : "";
}

namespace {

void PlayDemo(const SpellLook& look, const Pair& primary, Charge charge, int casterId, int targetId, float delay)
{
	Classified c = DemoClass(look, primary);
	c.charge = charge;
	const Config cfg = Cfg();
	const float start = FxClockNow() + delay;
	const float castSec = 1.2f;
	const uint32_t serial = ++s_serial;
	const int target = targetId > 0 ? targetId : casterId;

	const SpellLook* ch = ChargeLook(c.charge);
	for (int i = 0; i < ch->count; ++i)
		Push(ch->layers[i], c.pal, casterId, casterId, 0, serial, start, castSec, c.aeRange);
	ScheduleLook(look, c, casterId, target, 0, serial, start + castSec, TravelTime(casterId, target), cfg, false, true);
	if (c.beam)
		for (Active& a : s_active)
			if (a.castSerial == serial)
				a.beamRange = c.range;
}

} // namespace

void SpellFxPlayLook(int index, int casterId, int targetId, float delay)
{
	if (index < 0 || index >= SpellFxLookCount() || casterId <= 0)
		return;
	PlayDemo(*kAllLooks[index], kArcane, Charge::Arcane, casterId, targetId, delay);
}

int SpellFxLineCount()
{
	return LineLookCount();
}

int SpellFxDemoLevel()
{
	return s_demoLevel;
}

void SetSpellFxDemoLevel(int level)
{
	s_demoLevel = std::clamp(level, 1, 125);
}

const char* SpellFxLineClass(int index)
{
	const LineRef ref = LineLookAt(index);
	return ref.line ? ref.cls : "";
}

const char* SpellFxLineName(int index)
{
	const LineRef ref = LineLookAt(index);
	return ref.line ? ref.line->line : "";
}

void SpellFxPlayLine(int index, int casterId, int targetId, float delay)
{
	if (index < 0 || index >= LineLookCount() || casterId <= 0)
		return;
	const LineRef ref = LineLookAt(index);
	if (!ref.line)
		return;
	const Pair primary = ref.line->primary.c1.a > 0.0f ? ref.line->primary : kFire;
	PlayDemo(*ref.line->look, primary, ChargeForClass(ref.classId), casterId, targetId, delay);
}

void SpellFxDemoForm(uint32_t form, int casterId, int targetId)
{
	if (casterId <= 0)
		return;
	for (const SpellLook* look : kAllLooks)
	{
		for (int i = 0; i < look->count; ++i)
		{
			const Layer& l = look->layers[i];
			if (l.form != form)
				continue;
			const bool onCaster = l.stage == Stage::Charge || l.stage == Stage::Release;
			const int target = onCaster ? casterId : (targetId > 0 ? targetId : casterId);
			const float life = std::max(l.life > 0.0f ? l.life : 4.0f, 3.0f);
			Push(l, Palette{ kArcane, kEarth }, casterId, target, 0, 0, FxClockNow(), life, 30.0f);
			if (!s_active.empty() && (l.form == FxForm_Lightning || l.form == FxForm_Bolt || l.form == FxForm_Drain)
				&& target == casterId)
				s_active.back().beamRange = 40.0f;
			return;
		}
	}
}

void SpellFxDemoGlyphs(int spawnId)
{
	if (spawnId <= 0)
		return;
	if (s_demoLayers.empty())
		for (uint32_t id = 0; id < Glyph_Count; ++id)
			s_demoLayers.push_back(G(IM, id, 0.45f, 7.5f, 4, static_cast<float>(Glyph_Count), GA_PopBob, static_cast<float>(Glyph_Count) * 0.16f, FIX, kGold, 8.0f));
	const float now = FxClockNow();
	for (uint32_t id = 0; id < Glyph_Count; ++id)
	{
		const size_t before = s_active.size();
		Push(s_demoLayers[id], Palette{ kGold, kGold }, spawnId, spawnId, 0, 0, now, 8.0f, 0.0f);
		if (s_active.size() > before)
		{
			s_active.back().glyphSlot = static_cast<int>(id);
			s_active.back().phase = 0.0f;
		}
	}
}

uint32_t SpellFxActiveCount()
{
	return static_cast<uint32_t>(s_active.size());
}

bool SelfCastHookInstalled()
{
	return s_castSpellAddr != 0;
}
