#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

#include <cstring>

namespace eqgfx::spellfx {

namespace {

constexpr Pair kNcVenom   { { 0.75f, 1.00f, 0.35f, 0.95f }, { 0.20f, 0.55f, 0.05f, 0.85f } };
constexpr Pair kNcPlague  { { 0.88f, 0.86f, 0.38f, 0.90f }, { 0.38f, 0.30f, 0.08f, 0.85f } };
constexpr Pair kNcCurse   { { 0.82f, 0.42f, 1.00f, 0.90f }, { 0.25f, 0.05f, 0.40f, 0.85f } };
constexpr Pair kNcPyre    { { 0.72f, 1.00f, 0.50f, 0.95f }, { 0.10f, 0.45f, 0.10f, 0.85f } };
constexpr Pair kNcBloodfire { { 1.00f, 0.55f, 0.25f, 0.95f }, { 0.55f, 0.02f, 0.05f, 0.85f } };
constexpr Pair kNcLife    { { 1.00f, 0.35f, 0.38f, 0.95f }, { 0.45f, 0.00f, 0.10f, 0.85f } };
constexpr Pair kNcSoul    { { 0.75f, 1.00f, 0.92f, 0.80f }, { 0.20f, 0.60f, 0.55f, 0.60f } };
constexpr Pair kNcGrave   { { 0.58f, 0.62f, 0.52f, 0.95f }, { 0.20f, 0.22f, 0.18f, 0.90f } };
constexpr Pair kNcShade   { { 0.48f, 0.36f, 0.62f, 0.85f }, { 0.05f, 0.02f, 0.10f, 0.85f } };
constexpr Pair kNcEmerald { { 0.60f, 1.00f, 0.72f, 0.95f }, { 0.05f, 0.50f, 0.25f, 0.85f } };
constexpr Pair kNcRot     { { 0.85f, 1.00f, 0.55f, 0.90f }, { 0.40f, 0.30f, 0.05f, 0.85f } };
constexpr Pair kNcRime    { { 0.88f, 0.96f, 1.00f, 0.95f }, { 0.35f, 0.55f, 0.75f, 0.85f } };
constexpr Pair kNcLich    { { 0.55f, 1.00f, 0.95f, 0.95f }, { 0.15f, 0.25f, 0.60f, 0.85f } };
constexpr Pair kNcMind    { { 0.70f, 0.85f, 1.00f, 0.90f }, { 0.35f, 0.10f, 0.60f, 0.80f } };
constexpr Pair kNcDusk    { { 0.95f, 0.55f, 0.85f, 0.90f }, { 0.20f, 0.05f, 0.30f, 0.85f } };

constexpr uint32_t DARK = OVER;
constexpr uint32_t GA_Emerge = GlyphAnim_Rise | GlyphAnim_Pop;

LOOK(kNecPetBones, "cavorting bones",
	F(RE, FxForm_Cracks, 2.6f, 0, 48, 6, 1.2f, 0, 0, FIX, kNcGrave, 4.0f, 0.08f, DARK),
	G(RE, Glyph_Skeleton, 1.6f, 1.6f, 4, 1, GA_RiseWiggle, 0, FIX, kNcRime, 4.0f, 0.3f),
	F(RE, FxForm_Crystal, 1.8f, 2.5f, 40, 8, 1, 0.8f, 0, FIX, kNcRime, 3.5f, 0.06f));

LOOK(kNecPetSkeleton, "animate dead",
	F(RE, FxForm_Cracks, 3.2f, 0, 64, 8, 1.4f, 0, 0, FIX, kNcGrave, 4.5f, 0.09f, DARK),
	G(RE, Glyph_Skeleton, 1.9f, 1.9f, 4, 1, GA_RiseWiggle, 0, FIX, kBone, 4.0f, 0.3f, DARK),
	G(RE, Glyph_Skull, 0.4f, 1.0f, 2, 5, GA_PopSpin, 2.4f, FIX, kBone, 4.0f, 0, DARK),
	F(RE, FxForm_Sparkle, 2.0f, 6.0f, 40, 20, 0.12f, 0, 0, FIX, kNecro, 3.5f));

LOOK(kNecPetCharred, "charred skeleton",
	F(RE, FxForm_Cracks, 3.2f, 0, 64, 8, 1.4f, 0, 0, FIX, kNcBloodfire, 4.5f, 0.09f),
	G(RE, Glyph_Skeleton, 2.0f, 2.0f, 4, 1, GA_RiseWiggle, 0, FIX, kGrey, 4.0f, 0.3f, DARK),
	F(RE, FxForm_Flame, 1.2f, 6.0f, 96, 24, 0.2f, 1.4f, 0, FIX, kNcBloodfire, 3.5f, 0.07f, ADD, 1, 0.4f),
	F(RE, FxForm_Sparkle, 2.0f, 7.0f, 48, 24, 0.12f, 0, 0, FIX, kFire, 3.5f));

LOOK(kNecPetSpecter, "specter",
	F(RE, FxForm_Vortex, 1.8f, 8.0f, 112, 5, 1.2f, 2.5f, -4.0f, FIX, kNcShade, 4.0f, 0.08f, DARK),
	G(RE, Glyph_Spirit, 1.4f, 5.5f, 4, 1, GA_Emerge, 0, FIX, kGhost, 4.0f, 0.4f),
	G(RE, Glyph_Skull, 0.35f, 4.5f, 0, 4, GA_Orbit, 1.6f, FIX, kNcSoul, 3.5f),
	F(RE, FxForm_Rune, 2.8f, 0, 70, 5, 2, 10, 0, FIX, kNecro, 4.0f, 0.05f));

LOOK(kNecPetShade, "shade",
	F(RE, FxForm_Flame, 1.3f, 9.0f, 112, 28, 0.3f, 2.2f, 0, FIX, kNcShade, 4.0f, 0.10f, DARK),
	G(RE, Glyph_Bat, 0.5f, 6.0f, 3, 5, GA_Orbit, 2.0f, FIX, kNcShade, 4.0f, 0, DARK),
	G(RE, Glyph_Spirit, 1.4f, 5.5f, 4, 1, GA_Emerge, 0, FIX, kNcLich, 3.5f, 0.4f),
	F(RE, FxForm_Rune, 3.0f, 0, 80, 7, 3, 14, 0, FIX, kNcLich, 4.0f, 0.05f));

LOOK(kNecPetAssassin, "assassin",
	F(RE, FxForm_Bloom, 3.0f, 0.5f, 64, 3, 0, 0, 0, FIX, kNcShade, 3.0f, 0.07f, DARK),
	G(RE, Glyph_Sword, 0.9f, 4.0f, 3, 2, GA_RingRise, 1.2f, FIX, kNcEmerald, 3.5f, 0.3f),
	G(RE, Glyph_Skull, 1.1f, 6.0f, 4, 1, GA_Emerge, 0, FIX, kNcEmerald, 3.5f, 0.3f),
	F(RE, FxForm_Wind, 1.6f, 6.0f, 60, 12, 3.0f, 0, 0, FIX, kNcShade, 3.0f, 0.05f, DARK));

LOOK(kNecSkeletons, "call skeleton",
	F(IM, FxForm_Cracks, 2.6f, 0, 48, 6, 1.2f, 0, 0, FIX, kNcGrave, 3.0f, 0.08f, DARK),
	G(IM, Glyph_Skeleton, 1.5f, 1.5f, 4, 1, GA_RiseWiggle, 0, FIX, kBone, 3.0f, 0.2f, DARK),
	F(IM, FxForm_Burst, 2.2f, 0.3f, 64, 2, 0.3f, 12, 0, FIX, kNecro, 1.0f, 0.09f, ADD, 1, 0.2f));

LOOK(kNecScapegoat, "scapegoat",
	G(RE, Glyph_Skeleton, 1.5f, 1.5f, 4, 1, GA_RiseWiggle, 0, FIX, kGrey, 3.0f, 0, DARK),
	G(RE, Glyph_Skull, 0.6f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 2.4f, 0.5f),
	F(RE, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kNcShade, 2.0f, 0.05f, DARK | FxFlag_Reverse));

LOOK(kNecSmoldering, "smoldering bones",
	F(IM, FxForm_Cracks, 2.4f, 0, 48, 6, 1.2f, 0, 0, FIX, kNcBloodfire, 3.0f, 0.08f),
	G(IM, Glyph_Skeleton, 1.4f, 1.4f, 3, 2, GA_RiseWiggle, 1.0f, FIX, kGrey, 3.0f, 0.2f, DARK),
	F(IM, FxForm_Flame, 1.2f, 4.0f, 80, 20, 0.2f, 1.0f, 0, FIX, kNcBloodfire, 2.6f, 0.07f, ADD, 1, 0.3f));

LOOK(kNecPetForm, "pet form",
	G(IM, Glyph_Mask, 0.7f, 7.2f, 4, 1, GA_PopRoll, 0, FIX, kBone, 2.4f, 0, DARK),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, 3.0f, FIX, kNecro, 2.2f, 0.05f));

LOOK(kNecSigil, "sigil",
	G(IM, Glyph_Skull, 0.7f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kNecro, 2.6f),
	G(IM, Glyph_ArrowUp, 0.3f, 4.5f, 0, 4, GA_Orbit, 1.3f, FIX, kNecro, 2.6f),
	F(IM, FxForm_Rune, 1.6f, 0, 60, 5, 2, 10, 0, FIX, kNecro, 2.6f, 0.05f));

LOOK(kNecAlly, "ally",
	G(IM, Glyph_Fang, 0.7f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kNcLife, 2.6f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.8f, 0.9f, 0, FIX, kNcShade, 2.6f, 0.07f, DARK),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.10f, 2, 0, FIX, kNcLife, 2.4f));

LOOK(kNecMending, "renewal",
	G(IM, Glyph_Snowflake, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kNcRime, 2.4f),
	F(IM, FxForm_Crystal, 1.1f, 5.0f, 48, 10, 0, 0.8f, 0, FIX, kNcRime, 2.2f, 0.05f),
	G(IM, Glyph_Cross, 0.5f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kNecro, 2.0f));

LOOK(kNecWard, "spectral ward",
	F(IM, FxForm_Dome, 1.6f, 6.4f, 88, 6, 2, 0, 0, FIX, kGhost, 2.6f, 0.04f),
	G(IM, Glyph_Spirit, 0.35f, 4.5f, 0, 4, GA_Orbit, 1.5f, FIX, kGhost, 2.6f));

LOOK(kNecAegis, "aegis",
	G(IM, Glyph_Shield, 0.8f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kBone, 2.6f, 0, DARK),
	G(IM, Glyph_Skull, 0.3f, 4.5f, 0, 5, GA_Orbit, 1.4f, FIX, kBone, 2.6f, 0, DARK));

LOOK(kNecCascading, "cascading shield",
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kBone, 2.6f, 0.04f, DARK),
	F(IM, FxForm_Rain, 1.2f, 9.0f, 40, 30, 0, 0.2f, 0, FIX, kBone, 2.0f, 0.04f, ADD, 1, 0.3f),
	G(IM, Glyph_Shield, 0.4f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kNecro, 2.0f, 0.4f));

LOOK(kNecEssence, "essence",
	F(TR, FxForm_Drain, 1, 0, 64, 4, 1.6f, 18, 0, FIX, kNcLife, 2.0f, 0.10f),
	G(IM, Glyph_Heart, 0.8f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kNcLife, 1.6f),
	F(RE, FxForm_Sparkle, 1.0f, 6.0f, 32, 16, 0.12f, 2, 0, FIX, kNcLife, 2.0f, 0.06f, ADD, 1, 0.5f));

LOOK(kNecSoul, "soulrip",
	F(TR, FxForm_Drain, 1, 0, 64, 4, 2.0f, 20, 0, FIX, kNcSoul, 2.2f, 0.10f),
	G(IM, Glyph_Spirit, 0.8f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kNcSoul, 1.8f),
	G(RE, Glyph_Gem, 0.45f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNcSoul, 2.0f, 0.6f));

LOOK(kNecParoxysm, "paroxysm",
	G(IM, Glyph_Hand, 1.2f, 8.5f, 4, 1, GA_Drop, 0, FIX, kNcShade, 1.8f, 0, DARK),
	F(TR, FxForm_Drain, 1, 0, 96, 5, 2.2f, 24, 0, FIX, kNcLife, 2.4f, 0.12f, ADD, 1, 0.35f),
	F(IM, FxForm_Vortex, 1.4f, 6.0f, 96, 5, 1.2f, 2.5f, -4.0f, FIX, kNcLife, 2.2f, 0.06f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.4f, 1.0f, 72, 2, 1.0f, 16, 0, FIX, kNcShade, 0.9f, 0.10f, DARK, 1, 0.35f));

LOOK(kNecGrasp, "grasp",
	G(IM, Glyph_Claw, 0.8f, 6.8f, 4, 1, GlyphAnim_Pop, 0, FIX, kNcLife, 1.6f),
	F(TR, FxForm_Drain, 1, 0, 40, 2, 0.8f, 10, 0, FIX, kNcLife, 2.5f, 0.07f),
	F(AU, FxForm_Orbit, 1.2f, 5.5f, 40, 8, 0, 0, 0, FIX, kNcLife, 6.0f, 0.04f, ADD, 2.0f));

LOOK(kNecLeech, "leech",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.2f, 12, 0, FIX, kNcLife, 2.5f, 0.08f),
	G(IM, Glyph_Bat, 0.7f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kNcShade, 2.4f, 0, DARK),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 64, 2, 0, 0, 0, FIX, kNcLife, 2.0f, 0.05f, ADD, 1, 0.5f, AE));

LOOK(kNecSwiftLifedraw, "swift lifedraw",
	F(TR, FxForm_Drain, 1, 0, 64, 3, 2.4f, 16, 0, FIX, kCorrupt, 1.6f, 0.08f),
	F(IM, FxForm_Vortex, 1.0f, 4.5f, 64, 4, 1.0f, 2.0f, -5.0f, FIX, kCorrupt, 2.0f, 0.05f, DARK),
	G(IM, Glyph_Heart, 0.5f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kCorrupt, 1.6f, 0, DARK));

LOOK(kNecVenin, "venin",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.4f, 1.0f, FIX, kNcVenom, 0.8f, 0.18f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 72, 2, 1.5f, 14, 0, FIX, kNcVenom, 0.9f, 0.10f),
	G(IM, Glyph_Drop, 0.25f, 7.0f, 0, 5, GA_Rain, 1.0f, FIX, kNcVenom, 1.6f),
	F(IM, FxForm_Bubbles, 1.0f, 4.0f, 48, 8, 0.2f, 0, 0, FIX, kNcVenom, 1.6f, 0.04f));

LOOK(kNecBlood, "for blood",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.3f, 0.3f, 1.4f, FIX, kNcLife, 0.8f, 0.16f),
	G(IM, Glyph_Fang, 0.9f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kNcVenom, 1.4f),
	F(IM, FxForm_Burst, 2.0f, 2.0f, 64, 2, 1.5f, 12, 0, FIX, kNcLife, 0.8f, 0.09f));

LOOK(kNecHaze, "pallid haze",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.8f, 0.5f, 1.0f, FIX, kNcVenom, 0.8f, 0.20f, DARK),
	F(AU, FxForm_Vortex, 1.2f, 5.0f, 72, 4, 1.4f, 2.0f, 2.0f, FIX, kNcVenom, 6.0f, 0.06f, DARK),
	G(IM, Glyph_Snake, 0.7f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kNcVenom, 2.4f));

LOOK(kNecVenom, "venom",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.45f, 0.3f, 1.2f, FIX, kNcVenom, 0.8f, 0.16f),
	G(IM, Glyph_Insect, 0.3f, 5.0f, 0, 5, GA_Orbit, 1.3f, FIX, kNcVenom, 3.0f),
	F(IM, FxForm_Bubbles, 1.1f, 5.0f, 64, 8, 0.2f, 0, 0, FIX, kPoison, 3.0f, 0.04f));

LOOK(kNecSwiftVenom, "swift venom",
	F(TR, FxForm_Bolt, 1, 0, 128, 2, 0.1f, 0.9f, 0.3f, FIX, kNcVenom, 0.6f, 0.12f),
	G(IM, Glyph_Snake, 0.9f, 6.8f, 4, 1, GA_Drop, 0, FIX, kNcVenom, 1.6f),
	F(IM, FxForm_Spire, 0.8f, 3.5f, 48, 10, 1.0f, 2.2f, 0, FIX, kNcVenom, 1.0f, 0.05f));

LOOK(kNecFleshPoison, "flesh to poison",
	Persist(F(AU, FxForm_Bubbles, 1.1f, 6.0f, 48, 8, 0.18f, 0, 0, FIX, kNcVenom, 0, 0.04f)),
	G(IM, Glyph_Snake, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNcVenom, 2.6f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.9f, 0.8f, 0, FIX, kNcVenom, 2.4f, 0.06f));

LOOK(kNecDecay, "decay",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.45f, 0.3f, 1.2f, FIX, kNcPlague, 0.8f, 0.16f),
	G(IM, Glyph_Insect, 0.3f, 5.0f, 0, 6, GA_Orbit, 1.3f, FIX, kNcPlague, 3.0f),
	F(IM, FxForm_Bubbles, 1.1f, 5.0f, 64, 8, 0.2f, 0, 0, FIX, kDisease, 3.0f, 0.04f, DARK));

LOOK(kNecSwiftSickness, "swift sickness",
	F(TR, FxForm_Bolt, 1, 0, 96, 2, 0.2f, 0.8f, 0.5f, FIX, kNcPlague, 0.6f, 0.12f),
	F(IM, FxForm_Sparkle, 1.2f, 5.0f, 48, 24, 0.14f, 2, 0, FIX, kNcPlague, 1.8f),
	G(IM, Glyph_Insect, 0.8f, 7.0f, 4, 1, GA_Drop, 0, FIX, kNcPlague, 1.6f));

LOOK(kNecGrip, "grip",
	G(IM, Glyph_Hand, 0.9f, 5.5f, 4, 1, GA_PopRoll, 0, FIX, kNcPlague, 2.0f, 0, DARK),
	F(IM, FxForm_Bloom, 1.2f, 4.0f, 48, 3, 0, 0, 0, FIX, kNcPlague, 1.6f, 0.05f, ADD | FxFlag_Reverse),
	G(IM, Glyph_ArrowDown, 0.3f, 6.5f, 0, 3, GA_Orbit, 1.2f, FIX, kNcPlague, 2.4f));

LOOK(kNecGripDecay, "grip of decay",
	G(IM, Glyph_Hand, 1.1f, 6.0f, 4, 1, GA_PopRoll, 0, FIX, kNcPlague, 2.4f, 0, DARK),
	G(IM, Glyph_Insect, 0.3f, 5.0f, 0, 6, GA_Orbit, 1.4f, FIX, kNcPlague, 3.0f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 72, 4, 1.2f, 2.0f, -3.0f, FIX, kNcRot, 2.4f, 0.05f, DARK));

LOOK(kNecPustules, "pustules",
	Persist(F(AU, FxForm_Bubbles, 1.1f, 6.0f, 64, 10, 0.25f, 0, 0, FIX, kNcRot, 0, 0.05f, DARK)),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0.5f, 12, 0, FIX, kNcRot, 0.9f, 0.08f));

LOOK(kNecWounds, "wounds",
	G(IM, Glyph_Claw, 0.8f, 6.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kNcLife, 1.6f),
	F(IM, FxForm_Bubbles, 1.0f, 5.0f, 48, 8, 0.2f, 0, 0, FIX, kNcRot, 3.0f, 0.04f, DARK),
	F(IM, FxForm_Cracks, 1.4f, 0, 40, 5, 1.2f, 0, 0, FIX, kNcLife, 3.0f, 0.05f, DARK));

LOOK(kNecCurse, "curse",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.6f, 0.4f, 1.2f, FIX, kNcCurse, 0.8f, 0.18f),
	G(IM, Glyph_Skull, 0.8f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kNcCurse, 3.0f),
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 5, 2, 8, 1, FIX, kNcCurse, 2.4f, 0.05f));

LOOK(kNecSwiftDecon, "swift deconstruction",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.2f, 0.8f, 0.6f, FIX, kNcCurse, 0.6f, 0.12f),
	F(IM, FxForm_Lightning, 1, 16.0f, 48, 2, 0.6f, 14.0f, 0, FIX, kNcCurse, 0.7f, 0.10f),
	G(IM, Glyph_Skull, 0.3f, 5.5f, 0, 4, GA_Orbit, 1.2f, FIX, kNcCurse, 2.0f));

LOOK(kNecScourge, "scourge",
	G(IM, Glyph_Scythe, 1.1f, 7.5f, 4, 1, GA_Drop, 0, FIX, kNcCurse, 1.8f, 0, DARK),
	F(IM, FxForm_Burst, 2.2f, 2.0f, 64, 2, 1.0f, 12, 0, FIX, kNcCurse, 0.9f, 0.09f, ADD, 1, 0.35f),
	F(IM, FxForm_Vortex, 1.1f, 4.5f, 64, 4, 1.0f, 2.0f, -2.0f, FIX, kNcShade, 3.0f, 0.05f, DARK));

LOOK(kNecPutrefaction, "putrefaction",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.6f, 0.4f, 1.2f, FIX, kCorrupt, 0.8f, 0.18f, DARK),
	F(IM, FxForm_Vortex, 1.1f, 4.5f, 64, 4, 1.0f, 2.0f, -2.0f, FIX, kCorrupt, 3.0f, 0.05f, DARK),
	F(IM, FxForm_Bubbles, 1.0f, 4.0f, 48, 8, 0.2f, 0, 0, FIX, kNcRot, 3.0f, 0.04f));

LOOK(kNecSplort, "splort",
	F(IM, FxForm_Bubbles, 1.1f, 6.0f, 64, 12, 0.3f, 0, 0, FIX, kNcRot, 3.0f, 0.05f),
	F(IM, FxForm_Burst, 1.6f, 3.0f, 48, 1, 0.3f, 10, 0, FIX, kNcRot, 0.8f, 0.07f, ADD, 1, 0.8f),
	F(IM, FxForm_Burst, 2.0f, 3.0f, 64, 1, 0.3f, 14, 0, FIX, kNcRot, 0.8f, 0.09f, ADD, 1, 1.6f));

LOOK(kNecChaotic, "chaotic",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.5f, 0.35f, 1.2f, FIX, kNcCurse, 0.8f, 0.16f),
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.5f, 0.35f, 1.6f, FIX, kNcPlague, 0.8f, 0.16f, ADD, 1, 0.15f),
	G(IM, Glyph_Skull, 0.4f, 5.5f, 0, 3, GA_Orbit, 1.3f, FIX, kNcCurse, 2.6f),
	G(IM, Glyph_Insect, 0.3f, 4.5f, 0, 3, GA_Orbit, 1.1f, FIX, kNcPlague, 2.6f));

LOOK(kNecMalevolent, "malevolent",
	G(IM, Glyph_Pentagram, 1.3f, 0.0f, 2, 1, GA_PopRoll, 0, FIX, kNcCurse, 3.0f),
	G(IM, Glyph_Link, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kNcCurse, 3.0f),
	F(IM, FxForm_Bloom, 1.4f, 2.0f, 40, 2, 0, 0, 0, FIX, kNcShade, 2.2f, 0.05f, DARK));

LOOK(kNecIgnite, "ignite",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.2f, 14, 0, FIX, kNcMind, 2.0f, 0.08f),
	G(IM, Glyph_Flame, 0.5f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNcMind, 2.4f),
	F(IM, FxForm_Flame, 0.6f, 7.0f, 48, 12, 0.85f, 0.4f, 0, FIX, kNcMind, 2.4f, 0.05f));

LOOK(kNecMind, "mind",
	F(TR, FxForm_Drain, 1, 0, 64, 4, 1.4f, 16, 0, FIX, kMana, 2.0f, 0.08f),
	G(IM, Glyph_Spiral, 0.8f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kNcMind, 2.0f),
	F(RE, FxForm_Bloom, 1.0f, 1.5f, 64, 2, 0, 0, 0, FIX, kMana, 2.0f, 0.05f, ADD, 1, 0.5f, AE));

LOOK(kNecPyre, "pyre",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.6f, 0.4f, 1.4f, FIX, kNcPyre, 0.8f, 0.18f),
	F(IM, FxForm_Flame, 1.05f, 6.0f, 96, 24, 0.9f, 1.0f, 0, FIX, kNcPyre, 3.0f, 0.08f),
	G(IM, Glyph_Skull, 0.6f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kNcPyre, 2.4f, 0.3f));

LOOK(kNecBloodPyre, "blood pyre",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.3f, 1.2f, FIX, kNcBloodfire, 0.8f, 0.16f),
	F(IM, FxForm_Flame, 1.0f, 5.0f, 80, 20, 0.8f, 0.8f, 0, FIX, kNcBloodfire, 3.0f, 0.07f),
	G(IM, Glyph_Drop, 0.25f, 6.5f, 0, 4, GA_Orbit, 1.1f, FIX, kNcLife, 2.6f));

LOOK(kNecShadow, "searing shadow",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.6f, 0.4f, 1.0f, FIX, kNcShade, 0.8f, 0.18f, DARK),
	F(IM, FxForm_Flame, 1.05f, 6.0f, 96, 24, 0.9f, 1.0f, 0, FIX, kNcShade, 3.0f, 0.08f, DARK),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.10f, 0, 0, FIX, kNcBloodfire, 3.0f));

LOOK(kNecFlashblaze, "flashblaze",
	F(IM, FxForm_Lightning, 1, 30.0f, 48, 2, 0.5f, 12.0f, 1.0f, FIX, kNcPyre, 0.6f, 0.10f),
	F(IM, FxForm_Burst, 2.4f, 1.5f, 72, 2, 1.5f, 16, 0, FIX, kNcPyre, 0.8f, 0.10f, ADD, 1, 0.2f),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.6f, 1.2f, 0, FIX, kNcPyre, 1.6f, 0.07f, ADD, 1, 0.2f));

LOOK(kNecBones, "scorch bones",
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 0.5f, 0.45f, 1.6f, FIX, kNcBloodfire, 0.8f, 0.22f),
	G(IM, Glyph_Skeleton, 1.3f, 1.3f, 4, 1, GA_RiseWiggle, 0, FIX, kNcBloodfire, 1.8f, 0.3f),
	F(IM, FxForm_Burst, 2.6f, 1.0f, 96, 3, 1.5f, 16, 0, FIX, kNcBloodfire, 0.9f, 0.12f),
	Stars(0.4f));

LOOK(kNecDarkness, "darkness",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.6f, 0.4f, 1.0f, FIX, kNcShade, 0.8f, 0.18f, DARK),
	F(AU, FxForm_Vortex, 1.2f, 1.5f, 48, 3, 1.0f, 1.5f, -2.0f, FIX, kNcShade, 6.0f, 0.05f, DARK),
	G(IM, Glyph_Link, 0.3f, 1.0f, 0, 4, GA_PopSpin, 1.0f, FIX, kNecro, 2.6f));

LOOK(kNecScent, "scent",
	F(IM, FxForm_Wind, 1.2f, 4.5f, 48, 8, 1.5f, 0, 0, FIX, kNcPlague, 2.4f, 0.05f),
	G(IM, Glyph_ArrowDown, 0.35f, 7.0f, 0, 3, GA_Orbit, 1.2f, FIX, kNcVenom, 3.0f),
	F(IM, FxForm_Sparkle, 1.1f, 6.0f, 32, 16, 0.12f, 1, 0, FIX, kNcPlague, 2.4f));

LOOK(kNecShriek, "shriek",
	G(IM, Glyph_Skull, 1.2f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kGhost, 2.2f),
	G(IM, Glyph_Mouth, 0.5f, 6.0f, 0, 3, GA_Orbit, 1.4f, FIX, kNcSoul, 2.0f),
	F(IM, FxForm_Bloom, 2.2f, 4.0f, 48, 3, 0, 0, 0, FIX, kGhost, 1.6f, 0.05f));

LOOK(kNecManacles, "manacles",
	G(IM, Glyph_Shackle, 0.9f, 1.5f, 4, 1, GA_Drop, 0, FIX, kBone, 2.6f, 0, DARK),
	G(IM, Glyph_Link, 0.3f, 1.0f, 0, 5, GA_PopSpin, 1.0f, FIX, kNcShade, 3.0f, 0, DARK),
	F(IM, FxForm_Cracks, 1.6f, 0, 40, 5, 1.0f, 0, 0, FIX, kNcGrave, 3.0f, 0.06f, DARK));

LOOK(kNecBinding, "binding",
	G(IM, Glyph_Hourglass, 0.7f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kBone, 2.6f, 0, DARK),
	G(IM, Glyph_Link, 0.3f, 4.0f, 0, 4, GA_Orbit, 1.2f, FIX, kBone, 2.6f, 0, DARK));

LOOK(kNecCommand, "command",
	G(IM, Glyph_Crown, 0.7f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kNecro, 3.0f),
	G(IM, Glyph_Skull, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kBone, 3.0f, 0, DARK),
	F(IM, FxForm_Rune, 1.5f, 0, 60, 5, 2, 10, 0, FIX, kNecro, 3.0f, 0.05f));

LOOK(kNecUndead, "annihilate undead",
	G(IM, Glyph_Skull, 1.2f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kWhite, 1.0f),
	F(IM, FxForm_Lightning, 1, 36.0f, 48, 2, 0.5f, 12.0f, 1.0f, FIX, kNcLich, 0.7f, 0.12f, ADD, 1, 0.3f),
	F(IM, FxForm_Burst, 2.4f, 2.0f, 72, 2, 0, 16, 0, FIX, kBone, 0.9f, 0.10f, ADD | FxFlag_Reverse, 1, 0.3f));

LOOK(kNecCrippling, "crippling",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.0f, 12, 0, FIX, kNcGrave, 1.8f, 0.08f),
	G(IM, Glyph_Fist, 0.8f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kNcGrave, 2.0f, 0, DARK),
	G(IM, Glyph_ArrowDown, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kNcShade, 2.4f));

LOOK(kNecOrb, "orb",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.2f, 14, 0, FIX, kNcEmerald, 2.0f, 0.08f),
	G(RE, Glyph_Gem, 0.6f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kNcEmerald, 2.4f, 0.6f),
	F(RE, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 2, 0, FIX, kNcEmerald, 2.4f, 0.06f, ADD, 1, 0.6f));

LOOK(kNecFetish, "fetish",
	G(RE, Glyph_Doll, 1.0f, 2.0f, 4, 1, GA_Emerge, 0, FIX, kBone, 3.5f, 0, DARK),
	F(RE, FxForm_Rune, 2.2f, 0, 70, 5, 2, 10, 0, FIX, kNecro, 3.5f, 0.05f),
	G(RE, Glyph_Skull, 0.3f, 2.5f, 0, 4, GA_Orbit, 1.4f, FIX, kNecro, 3.0f));

LOOK(kNecSphere, "sphere",
	F(RE, FxForm_Bloom, 1.0f, 0.5f, 96, 3, 0, 0, 0, FIX, kNcRot, 3.0f, 0.07f, ADD, 1, 0, AE),
	F(RE, FxForm_Dome, 1.0f, 2.0f, 96, 8, 3, 0, 0, FIX, kCorrupt, 3.0f, 0.05f, DARK, 1, 0.2f, AE),
	G(RE, Glyph_Skull, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNcRot, 3.0f));

LOOK(kNecEcho, "echo",
	F(IM, FxForm_Bloom, 1.0f, 0.5f, 96, 4, 0, 0, 0, FIX, kCorrupt, 3.0f, 0.06f, DARK, 1, 0, AE),
	F(IM, FxForm_Bloom, 1.0f, 0.5f, 96, 3, 0, 0, 0, FIX, kNcRot, 2.4f, 0.05f, ADD, 1, 0.6f, AE),
	F(IM, FxForm_Sparkle, 1.4f, 5.0f, 48, 24, 0.12f, 0, 0, FIX, kNcRot, 3.0f));

LOOK(kNecSkin, "skin",
	F(IM, FxForm_Dome, 1.6f, 6.4f, 88, 6, 2, 0, 0, FIX, kNcGrave, 2.4f, 0.04f, DARK),
	G(IM, Glyph_Skull, 0.3f, 4.5f, 0, 4, GA_Orbit, 1.4f, FIX, kBone, 2.6f, 0, DARK),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 24, 12, 0.10f, 1, 0, FIX, kMana, 2.4f));

LOOK(kNecBulwark, "bulwark",
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kNcShade, 2.4f, 0.05f, DARK),
	G(IM, Glyph_Shield, 0.8f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kNcShade, 2.4f, 0, DARK));

LOOK(kNecShielding, "shielding",
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kNecro, 2.4f, 0.04f),
	G(IM, Glyph_Shield, 0.8f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kNecro, 2.4f),
	F(IM, FxForm_Rune, 1.8f, 0, 60, 6, 1, 12, 0, FIX, kNecro, 2.4f));

LOOK(kNecShieldFate, "shield of fate",
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kNcCurse, 2.6f, 0.04f),
	G(IM, Glyph_Scythe, 0.7f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kNcCurse, 2.6f));

LOOK(kNecVeil, "quivering veil",
	F(IM, FxForm_Dome, 1.8f, 7.0f, 112, 10, 4, 0, 0, FIX, kNcLich, 3.0f, 0.06f),
	F(IM, FxForm_Vortex, 1.4f, 6.5f, 96, 5, 1.0f, 2.5f, 6.0f, FIX, kNcShade, 3.0f, 0.06f, DARK),
	G(IM, Glyph_Hourglass, 0.6f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kNcLich, 3.0f));

LOOK(kNecMirror, "mirror",
	G(IM, Glyph_Shield, 0.9f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kNcLich, 2.2f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0, 12, 0, FIX, kNcShade, 1.0f, 0.06f, DARK | FxFlag_Reverse));

LOOK(kNecPact, "pact",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.4f, 14, 0, FIX, kNcLife, 2.2f, 0.08f),
	G(IM, Glyph_Heart, 0.7f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kNcLife, 3.0f),
	G(RE, Glyph_Link, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kNcLife, 2.6f));

LOOK(kNecSide, "otherside",
	Persist(F(AU, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kNcShade, 0, 0.05f, DARK)),
	Persist(F(AU, FxForm_Orbit, 1.2f, 6.0f, 40, 10, 0, 0, 0, FIX, kMana, 0, 0.04f, ADD, 1.5f)),
	G(IM, Glyph_Eye, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNcLich, 2.6f));

LOOK(kNecLich, "lich",
	Persist(F(AU, FxForm_Flame, 1.05f, 6.0f, 96, 24, 0.9f, 0.9f, 0, FIX, kNcLich, 0, 0.06f)),
	Persist(F(AU, FxForm_Ring, 1.2f, 6.8f, 40, 0, 1.0f, 1.0f, 0.5f, FIX, kNcLich, 0, 0.04f, COMET)),
	G(IM, Glyph_Skull, 1.2f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kNcLich, 3.0f),
	F(IM, FxForm_Crystal, 1.2f, 5.0f, 48, 10, 0, 0.8f, 0, FIX, kNcRime, 2.6f, 0.05f));

LOOK(kNecBestow, "bestow undeath",
	G(IM, Glyph_Skeleton, 1.8f, 2.0f, 4, 1, GA_RiseWiggle, 0, FIX, kNcLich, 3.0f, 0, DARK),
	F(IM, FxForm_Vortex, 1.3f, 6.0f, 96, 5, 1.0f, 2.5f, 4.0f, FIX, kNecro, 3.0f, 0.06f),
	Persist(F(AU, FxForm_Sparkle, 1.2f, 7.0f, 24, 12, 0.10f, 2, 0, FIX, kNcLife, 0)));

LOOK(kNecReaper, "summon corpse",
	G(RE, Glyph_Scythe, 1.2f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kNcGrave, 3.0f, 0, DARK),
	F(RE, FxForm_Vortex, 1.6f, 6.0f, 96, 5, 1.0f, 2.5f, -4.0f, FIX, kNcShade, 3.0f, 0.06f, DARK),
	F(RE, FxForm_Rune, 2.2f, 0, 60, 5, 2, 8, 0, FIX, kNecro, 3.0f, 0.05f));

LOOK(kNecFeign, "feign death",
	G(IM, Glyph_Skull, 0.7f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kGrey, 2.5f, 0, DARK),
	F(IM, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kNcGrave, 2.0f, 0.05f, DARK | FxFlag_Reverse));

LOOK(kAaSwarmDecay, "swarm of decay",
	G(IM, Glyph_Skeleton, 1.4f, 1.4f, 4, 1, GA_RiseWiggle, 0, FIX, kNcRot, 3.0f, 0, DARK),
	G(IM, Glyph_Insect, 0.3f, 4.0f, 0, 8, GA_Orbit, 1.8f, FIX, kNcPlague, 3.5f),
	F(IM, FxForm_Cracks, 2.2f, 0, 48, 6, 1.2f, 0, 0, FIX, kNcRot, 3.0f, 0.07f, DARK));

LOOK(kAaRiseBones, "rise of bones",
	F(IM, FxForm_Cracks, 3.0f, 0, 64, 8, 1.4f, 0, 0, FIX, kNcGrave, 4.0f, 0.08f, DARK),
	G(IM, Glyph_Skeleton, 1.6f, 2.0f, 3, 3, GA_RiseWiggle, 2.0f, FIX, kBone, 4.0f, 0, DARK),
	F(IM, FxForm_Burst, 3.0f, 0.3f, 96, 2, 0.3f, 16, 0, FIX, kNecro, 1.2f, 0.10f, ADD, 1, 0.3f));

LOOK(kAaWakeDead, "wake the dead",
	G(IM, Glyph_Spirit, 0.9f, 3.5f, 3, 3, GA_RingRise, 2.0f, FIX, kGhost, 4.0f),
	F(IM, FxForm_Vortex, 2.0f, 7.0f, 112, 6, 1.4f, 3.0f, 4.0f, FIX, kNcSoul, 4.0f, 0.06f),
	F(IM, FxForm_Rune, 2.8f, 0, 70, 5, 2, 10, 0, FIX, kNcSoul, 4.0f, 0.05f));

LOOK(kAaDarkMessenger, "dark messenger",
	G(RE, Glyph_Bat, 1.2f, 6.0f, 4, 1, GA_Emerge, 0, FIX, kNcShade, 3.0f, 0, DARK),
	F(RE, FxForm_Wind, 1.6f, 6.0f, 60, 12, 3.0f, 0, 0, FIX, kNcShade, 3.0f, 0.05f, DARK),
	G(RE, Glyph_Spirit, 0.3f, 5.5f, 0, 3, GA_Orbit, 1.4f, FIX, kNcSoul, 3.0f));

LOOK(kAaServantDeath, "servant of death",
	G(IM, Glyph_Crown, 0.8f, 8.2f, 4, 1, GA_Drop, 0, FIX, kNcShade, 3.0f, 0, DARK),
	F(IM, FxForm_Vortex, 1.3f, 5.5f, 80, 5, 1.2f, 2.5f, -4.0f, FIX, kNecro, 3.0f, 0.06f),
	G(IM, Glyph_Link, 0.3f, 5.0f, 0, 5, GA_Orbit, 1.4f, FIX, kNcShade, 3.0f, 0, DARK));

LOOK(kAaDireCharm, "dire charm",
	G(IM, Glyph_Crown, 1.0f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kNcLich, 4.0f),
	G(IM, Glyph_Skull, 0.35f, 5.0f, 0, 6, GA_Orbit, 1.6f, FIX, kNcLich, 4.0f),
	F(IM, FxForm_Rune, 2.0f, 0, 80, 7, 3, 14, 0, FIX, kNcLich, 4.0f, 0.05f));

LOOK(kAaDeadMez, "dead mesmerization",
	G(IM, Glyph_Spiral, 0.8f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kBone, 3.0f, 0, DARK),
	G(IM, Glyph_Z, 0.35f, 7.8f, 0, 1, GlyphAnim_Ascend, 0.5f, FIX, kNcSoul, 2.0f, 0.2f));

LOOK(kAaRestDead, "rest the dead",
	G(IM, Glyph_Dove, 0.7f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kBone, 2.6f, 0, DARK),
	F(IM, FxForm_Bloom, 1.6f, 1.0f, 40, 2, 0, 0, 0, FIX, kNcGrave, 2.2f, 0.05f, DARK | FxFlag_Reverse));

LOOK(kAaFearStorm, "fear storm",
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kGhost, 2.2f, 0.07f, ADD, 1, 0, AE),
	G(RE, Glyph_Skull, 0.6f, 4.0f, 3, 6, GA_RingRise, 2.4f, FIX, kGhost, 2.6f),
	F(RE, FxForm_Vortex, 1.6f, 7.0f, 96, 5, 1.4f, 2.5f, 6.0f, FIX, kNcShade, 2.4f, 0.06f, DARK));

LOOK(kAaParalysis, "pestilent paralysis",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.3f, 0.8f, FIX, kNcPlague, 0.6f, 0.16f),
	F(IM, FxForm_Burst, 2.4f, 1.0f, 72, 2, 0.5f, 16, 0, FIX, kNcPlague, 0.8f, 0.10f),
	G(IM, Glyph_Shackle, 0.8f, 1.5f, 4, 1, GA_Drop, 0, FIX, kNcPlague, 2.6f, 0.2f, DARK));

LOOK(kAaEncroaching, "encroaching darkness",
	F(IM, FxForm_Vines, 0.9f, 3.5f, 96, 5, 1.2f, 0, 1, FIX, kNcShade, 3.0f, 0.06f, DARK),
	G(IM, Glyph_Link, 0.3f, 1.0f, 0, 5, GA_PopSpin, 1.0f, FIX, kNcShade, 3.0f, 0, DARK));

LOOK(kAaDyingGrasp, "dying grasp",
	G(IM, Glyph_Hand, 1.4f, 9.0f, 4, 1, GA_Drop, 0, FIX, kNcShade, 2.0f, 0, DARK),
	F(TR, FxForm_Drain, 1, 0, 96, 5, 2.0f, 24, 0, FIX, kNcLife, 2.4f, 0.12f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.6f, 1.0f, 72, 2, 1.0f, 16, 0, FIX, kNcLife, 0.9f, 0.10f, ADD, 1, 0.35f));

LOOK(kAaLifeBurn, "life burn",
	F(RE, FxForm_Flame, 1.0f, 6.0f, 96, 24, 0.8f, 1.0f, 0, FIX, kNcLife, 1.6f, 0.08f),
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 0.6f, 0.5f, 1.6f, FIX, kNcLife, 0.8f, 0.24f),
	F(IM, FxForm_Burst, 3.0f, 1.5f, 96, 3, 2.0f, 20, 0, FIX, kNcBloodfire, 1.0f, 0.14f),
	G(IM, Glyph_Heart, 0.9f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kNcLife, 1.8f));

LOOK(kAaLifeBurnRecourse, "lifeburn recourse",
	F(IM, FxForm_Sparkle, 1.1f, 6.0f, 24, 12, 0.10f, 2, 0, FIX, kNcLife, 1.6f),
	F(IM, FxForm_Ring, 1.0f, 3.0f, 32, 0, 2.0f, 1.0f, 0.8f, FIX, kNcLife, 1.4f, 0.04f, COMET));

LOOK(kAaLifeshare, "reluctant lifeshare",
	G(IM, Glyph_Heart, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNcLife, 2.2f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 64, 2, 1.5f, 0, 0.5f, FIX, kNcLife, 2.2f, 0.05f, COMET));

LOOK(kAaDeathBloom, "death bloom",
	F(IM, FxForm_Vines, 1.0f, 5.5f, 96, 5, 1.0f, 0, 0, FIX, kNcShade, 3.0f, 0.06f, DARK),
	G(IM, Glyph_Skull, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kMana, 3.0f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kMana, 3.0f));

LOOK(kAaGatheringDusk, "gathering dusk",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.6f, 0.4f, 1.2f, FIX, kNcDusk, 0.8f, 0.18f),
	F(IM, FxForm_Dome, 1.4f, 5.5f, 72, 6, 2, 0, 0, FIX, kNcDusk, 3.0f, 0.06f, DARK),
	G(IM, Glyph_Moon, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNcDusk, 3.0f));

LOOK(kAaDuskfall, "duskfall empowerment",
	G(IM, Glyph_Moon, 0.9f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kNcDusk, 3.5f),
	F(IM, FxForm_Flame, 1.05f, 6.0f, 112, 28, 0.9f, 1.0f, 0, FIX, kNcDusk, 3.5f, 0.09f),
	G(IM, Glyph_ArrowUp, 0.3f, 4.5f, 0, 4, GA_Orbit, 1.4f, FIX, kNcDusk, 3.0f));

LOOK(kAaHandDeath, "hand of death",
	G(IM, Glyph_Hand, 1.2f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kNecro, 3.5f),
	F(IM, FxForm_Flame, 1.05f, 6.0f, 112, 28, 0.9f, 1.0f, 0, FIX, kNecro, 3.5f, 0.09f),
	G(IM, Glyph_Skull, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.4f, FIX, kNecro, 3.5f));

LOOK(kAaSpire, "spire of necromancy",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kNecro, 2.6f, 0.10f),
	G(RE, Glyph_Skull, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kBone, 2.6f, 0, DARK),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kNecro, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaFuneralPyre, "funeral pyre",
	F(IM, FxForm_Flame, 1.2f, 7.0f, 112, 28, 0.6f, 1.4f, 0, FIX, kNcPyre, 3.5f, 0.10f),
	G(IM, Glyph_Skull, 0.4f, 5.0f, 3, 3, GA_Orbit, 1.4f, FIX, kNcPyre, 3.5f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kNcPyre, 3.0f, 0.06f));

LOOK(kAaTwincast, "heretic's twincast",
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 2, FIX, kNecro, 2.6f, 0.05f),
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 2, FIX, kNcCurse, 2.6f, 0.05f, ADD, -1.0f),
	G(IM, Glyph_Skull, 0.5f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kNecro, 2.6f));

LOOK(kAaMercurial, "mercurial torment",
	G(IM, Glyph_Hourglass, 0.7f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kNcCurse, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 64, 2.5f, 3.0f, 0, 1.0f, FIX, kNcCurse, 2.6f, 0.05f, COMET));

LOOK(kAaSynergy, "defiler's synergy",
	G(IM, Glyph_Gem, 0.6f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kNcCurse, 2.6f),
	F(IM, FxForm_Burst, 1.8f, 2.0f, 48, 1, 1.5f, 12, 0, FIX, kNcCurse, 0.9f, 0.07f));

LOOK(kAaUnity, "mortifier's unity",
	G(IM, Glyph_Skull, 0.4f, 4.5f, 3, 3, GA_Orbit, 1.4f, FIX, kNecro, 3.0f),
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kNcShade, 2.6f, 0.04f, DARK),
	F(IM, FxForm_Burst, 1.8f, 2.0f, 48, 1, 1.5f, 12, 0, FIX, kNecro, 0.9f, 0.07f));

LOOK(kAaScentThule, "scent of thule",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 60, 10, 1.8f, 0, 0, FIX, kNcVenom, 2.6f, 0.05f),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 60, 10, 1.8f, 0, 0, FIX, kNcPlague, 2.6f, 0.05f, ADD, -1.0f),
	G(IM, Glyph_Skull, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNcPlague, 2.6f));

LOOK(kAaMalaise, "death's malaise",
	G(IM, Glyph_Skull, 0.7f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kGrey, 2.6f, 0, DARK),
	F(IM, FxForm_Bloom, 1.8f, 2.0f, 40, 3, 0, 0, 0, FIX, kNcGrave, 2.2f, 0.05f, DARK | FxFlag_Reverse),
	G(IM, Glyph_ArrowDown, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kGrey, 2.4f));

LOOK(kAaEffigy, "death's effigy",
	G(IM, Glyph_Doll, 1.1f, 2.5f, 4, 1, GA_Emerge, 0, FIX, kBone, 3.0f, 0, DARK),
	G(IM, Glyph_Spirit, 0.7f, 6.0f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 2.4f, 0.4f),
	F(IM, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kNcShade, 2.0f, 0.05f, DARK | FxFlag_Reverse));

LOOK(kAaDeathPeace, "death peace",
	G(IM, Glyph_Skull, 0.8f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kNcGrave, 2.5f, 0, DARK),
	F(IM, FxForm_Cracks, 1.6f, 0, 40, 5, 1.0f, 0, 0, FIX, kNcGrave, 2.4f, 0.05f, DARK));

LOOK(kAaCarapace, "embalmer's carapace",
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kNcGrave, 3.0f, 0.05f, DARK),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 64, 3, 2.5f, 0, 0.8f, FIX, kBone, 3.0f, 0.05f, COMET),
	G(IM, Glyph_Shield, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kBone, 3.0f, 0, DARK));

LOOK(kAaGiftGrave, "gift of the grave",
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kNcSoul, 2.6f, 0.04f),
	G(IM, Glyph_Ankh, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNcSoul, 2.6f));

LOOK(kAaHarmshield, "harmshield",
	F(IM, FxForm_Dome, 1.9f, 7.2f, 128, 10, 4, 0, 0, FIX, kNcShade, 3.0f, 0.07f, DARK),
	G(IM, Glyph_Shield, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kNcLich, 3.0f),
	F(IM, FxForm_Orbit, 1.4f, 6.0f, 48, 10, 0, 0, 0, FIX, kNcLich, 3.0f, 0.04f, ADD, 2.0f));

LOOK(kAaEmbraceDecay, "embrace the decay",
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kNcRot, 2.2f),
	G(IM, Glyph_Chalice, 0.7f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kNcRot, 2.2f),
	F(IM, FxForm_Burst, 1.8f, 2.0f, 48, 1, 1.0f, 10, 0, FIX, kNcRot, 0.8f, 0.07f, ADD | FxFlag_Reverse));

LOOK(kAaFrenzy, "frenzy of the dead",
	G(IM, Glyph_Fist, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNcLife, 3.0f),
	F(IM, FxForm_Flame, 1.05f, 6.0f, 112, 28, 0.9f, 1.0f, 0, FIX, kNcShade, 3.0f, 0.09f, DARK),
	G(IM, Glyph_Skull, 0.35f, 5.0f, 0, 4, GA_Orbit, 1.4f, FIX, kNcLife, 3.0f));

LOOK(kAaCompanionNecro, "companion's necromancy",
	G(IM, Glyph_Fang, 0.6f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kNecro, 2.4f),
	F(IM, FxForm_Orbit, 1.2f, 5.5f, 40, 8, 0, 0, 0, FIX, kNecro, 2.6f, 0.04f, ADD, 2.0f));

LOOK(kAaNightmares, "lingering nightmares",
	G(IM, Glyph_Eye, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNcDusk, 3.0f),
	G(IM, Glyph_Bat, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.4f, FIX, kNcShade, 3.0f, 0, DARK));

LOOK(kAaCascadeRot, "cascade of decay",
	F(IM, FxForm_Rain, 1.2f, 9.0f, 40, 30, 0, 0.2f, 0, FIX, kNcRot, 2.0f, 0.05f),
	F(IM, FxForm_Bubbles, 1.0f, 4.0f, 40, 8, 0.2f, 0, 0, FIX, kNcRot, 2.0f, 0.04f, DARK));

LOOK(kAaFormDeath, "form of death",
	G(IM, Glyph_Skeleton, 1.8f, 2.0f, 4, 1, GA_RiseWiggle, 0, FIX, kNcRot, 3.0f, 0, DARK),
	F(IM, FxForm_Bubbles, 1.1f, 5.5f, 64, 8, 0.2f, 0, 0, FIX, kNcRot, 3.0f, 0.04f, DARK));

LOOK(kAaFleshBone, "flesh to bone",
	G(IM, Glyph_Skull, 0.6f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kBone, 2.0f, 0, DARK),
	F(IM, FxForm_Sparkle, 1.2f, 5.0f, 32, 16, 0.12f, 2, 0, FIX, kBone, 2.0f));

LOOK(kAaSummonRemains, "summon remains",
	G(IM, Glyph_Skeleton, 1.2f, 1.2f, 4, 1, GA_Drop, 0, FIX, kBone, 2.4f, 0, DARK),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, 3.0f, FIX, kNcGrave, 2.2f, 0.05f, DARK));

LOOK(kAaWhisperwind, "whisperwind",
	F(RE, FxForm_Wind, 1.6f, 6.0f, 72, 14, 3.0f, 0, 0, FIX, kGhost, 2.4f, 0.05f),
	G(RE, Glyph_Spirit, 0.35f, 5.5f, 0, 4, GA_Orbit, 1.5f, FIX, kNcSoul, 2.4f));

LOOK(kAaLevant, "levant",
	G(RE, Glyph_Door, 1.2f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kNcShade, 3.0f, 0, DARK),
	F(RE, FxForm_Vortex, 1.6f, 6.0f, 96, 5, 0.8f, 2.5f, -5.0f, FIX, kNcShade, 3.0f, 0.06f, DARK),
	F(RE, FxForm_Sparkle, 1.6f, 6.0f, 40, 20, 0.12f, 2, 0, FIX, kNcLich, 3.0f));

LOOK(kAaConvergence, "convergence",
	G(IM, Glyph_Ankh, 1.2f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kNcEmerald, 3.5f, 0.3f),
	F(IM, FxForm_Vortex, 1.4f, 7.0f, 96, 5, 1.0f, 2.5f, 4.0f, FIX, kNcEmerald, 3.5f, 0.06f),
	F(IM, FxForm_Sparkle, 1.4f, 8.0f, 48, 24, 0.12f, 1, 0, FIX, kNcEmerald, 3.0f));

LOOK(kAaFloating, "dead man floating",
	G(IM, Glyph_Feather, 0.35f, 2.0f, 0, 3, GA_Orbit, 1.2f, FIX, kBone, 3.0f, 0, DARK),
	G(IM, Glyph_Skull, 0.5f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kNcSoul, 2.4f),
	F(IM, FxForm_Ring, 1.2f, 0.4f, 40, 0, 0.8f, 1.0f, 0.4f, FIX, kNcSoul, 2.6f, 0.05f, COMET));

LOOK(kAaInvisUndead, "invisibility versus undead",
	F(IM, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kGhost, 2.0f, 0.05f, ADD | FxFlag_Reverse),
	G(IM, Glyph_Skull, 0.6f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 2.0f));

LOOK(kAaBloodMagic, "blood magic",
	G(RE, Glyph_Drop, 0.7f, 7.6f, 4, 3, GA_Orbit, 1.0f, FIX, kBlood, 2.6f),
	F(RE, FxForm_Vortex, 1.3f, 6.0f, 72, 4, 1.2f, 2.0f, -3.0f, FIX, kBlood, 2.4f, 0.05f),
	F(RE, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 2, 0, FIX, kMana, 2.4f));

enum Line : int
{
	PetBones, PetSkeleton, PetCharred, PetSpecter, PetShade, PetAssassin, Skeletons, Scapegoat, Smoldering,
	PetForm, Sigil, Ally, Mending, Ward, Aegis, Cascading, Essence, Soul, Paroxysm, Grasp, Leech,
	SwiftLifedraw, Venin, Blood, Haze, Venom, SwiftVenom, FleshPoison, Decay, SwiftSickness, Grip, GripDecay,
	Pustules, Wounds, Curse, SwiftDecon, Scourge, Putrefaction, Splort, Chaotic, Malevolent, Ignite, Mind,
	Pyre, BloodPyre, Shadow, Flashblaze, Bones, Darkness, Scent, Shriek, Manacles, Binding, Command, Undead,
	Crippling, Orb, Fetish, Sphere, Echo, Skin, Bulwark, Shielding, ShieldFate, Veil, Mirror, Pact, Side,
	Lich, Bestow, Reaper, Feign,
	SwarmDecay, RiseBones, WakeDead, DarkMessenger, ServantDeath, DireCharm, DeadMez, RestDead, FearStorm,
	Paralysis, Encroaching, DyingGrasp, LifeBurn, LifeBurnRecourse, Lifeshare, DeathBloom, GatheringDusk,
	Duskfall, HandDeath, Spire, FuneralPyre, Twincast, Mercurial, Synergy, Unity, ScentThule, Malaise, Effigy,
	DeathPeace, Carapace, GiftGrave, Harmshield, EmbraceDecay, Frenzy, CompanionNecro, Nightmares,
	CascadeRot, FormDeath, FleshBone, SummonRemains, Whisperwind, Levant, Convergence, Floating, InvisUndead,
	BloodMagic,
};

const LineLook kLines[] = {
	{ "Skeleton (early pet)",            &kNecPetBones },
	{ "Skeleton (pet)",                  &kNecPetSkeleton },
	{ "Charred skeleton (pet)",          &kNecPetCharred },
	{ "Specter (pet)",                   &kNecPetSpecter },
	{ "Shade (pet)",                     &kNecPetShade },
	{ "Assassin (pet)",                  &kNecPetAssassin },
	{ "Call Skeleton (swarm)",           &kNecSkeletons },
	{ "Scapegoat (swarm decoy)",         &kNecScapegoat },
	{ "Smoldering Bones (swarm)",        &kNecSmoldering },
	{ "Form (pet illusion)",             &kNecPetForm },
	{ "Sigil (pet buff)",                &kNecSigil },
	{ "Ally (pet proc)",                 &kNecAlly },
	{ "Mending (pet heal)",              &kNecMending },
	{ "Spectral Ward (pet)",             &kNecWard },
	{ "Aegis / Calliav (pet rune)",      &kNecAegis },
	{ "Cascading shield (pet rune)",     &kNecCascading },
	{ "Essence (lifetap)",               &kNecEssence },
	{ "Soulrip (lifetap)",               &kNecSoul },
	{ "Paroxysm (big lifetap)",          &kNecParoxysm },
	{ "Grasp (lifetap over time)",       &kNecGrasp },
	{ "Leech (tap, group heal)",         &kNecLeech },
	{ "Swift Lifedraw (tap)",            &kNecSwiftLifedraw },
	{ "Venin (poison nuke)",             &kNecVenin },
	{ "For Blood (poison nuke)",         &kNecBlood },
	{ "Pallid Haze (poison)",            &kNecHaze },
	{ "Venom (poison)",                  &kNecVenom },
	{ "Swift Venom (poison)",            &kNecSwiftVenom },
	{ "Flesh to Poison (self)",          &kNecFleshPoison },
	{ "Decay (disease)",                 &kNecDecay },
	{ "Swift Sickness (disease)",        &kNecSwiftSickness },
	{ "Grip (disease, debuff)",          &kNecGrip },
	{ "Grip of Decay (disease)",         &kNecGripDecay },
	{ "Necrotic Pustules",               &kNecPustules },
	{ "Wounds (proliferation)",          &kNecWounds },
	{ "Curse (magic)",                   &kNecCurse },
	{ "Swift Deconstruction (magic)",    &kNecSwiftDecon },
	{ "Scourge (magic)",                 &kNecScourge },
	{ "Putrefaction (corruption)",       &kNecPutrefaction },
	{ "Splort (growing dot)",            &kNecSplort },
	{ "Chaotic (dual dot)",              &kNecChaotic },
	{ "Malevolent (dot amp)",            &kNecMalevolent },
	{ "Ignite (mana burn)",              &kNecIgnite },
	{ "Mind (mana drain)",               &kNecMind },
	{ "Pyre (fire)",                     &kNecPyre },
	{ "Blood pyre (fire)",               &kNecBloodPyre },
	{ "Searing Shadow (fire)",           &kNecShadow },
	{ "Flashblaze (fire)",               &kNecFlashblaze },
	{ "Scorch Bones (stun)",             &kNecBones },
	{ "Darkness (dot, snare)",           &kNecDarkness },
	{ "Scent (debuff)",                  &kNecScent },
	{ "Shriek (fear)",                   &kNecShriek },
	{ "Manacles (hold)",                 &kNecManacles },
	{ "Binding (undead slow)",           &kNecBinding },
	{ "Command (undead charm)",          &kNecCommand },
	{ "Annihilate Undead (nuke)",        &kNecUndead },
	{ "Crippling (attack theft)",        &kNecCrippling },
	{ "Orb (life orb)",                  &kNecOrb },
	{ "Fetish (trap)",                   &kNecFetish },
	{ "Sphere (aura)",                   &kNecSphere },
	{ "Echo (pulse)",                    &kNecEcho },
	{ "Skin (rune, mana)",               &kNecSkin },
	{ "Bulwark (hit points)",            &kNecBulwark },
	{ "Shielding (hit points)",          &kNecShielding },
	{ "Shield of Fate",                  &kNecShieldFate },
	{ "Quivering Veil",                  &kNecVeil },
	{ "Mirror (reflect)",                &kNecMirror },
	{ "Pact (heal by life)",             &kNecPact },
	{ "Otherside (mana)",                &kNecSide },
	{ "Lich (mana)",                     &kNecLich },
	{ "Bestow (undead form)",            &kNecBestow },
	{ "Reaper (summon corpse)",          &kNecReaper },
	{ "Feign Death",                     &kNecFeign },

	{ "AA: Swarm of Decay",              &kAaSwarmDecay },
	{ "AA: Rise of Bones",               &kAaRiseBones },
	{ "AA: Wake the Dead",               &kAaWakeDead },
	{ "AA: Dark Messenger",              &kAaDarkMessenger },
	{ "AA: Servant of Death",            &kAaServantDeath },
	{ "AA: Dire Charm",                  &kAaDireCharm },
	{ "AA: Dead Mesmerization",          &kAaDeadMez },
	{ "AA: Rest the Dead",               &kAaRestDead },
	{ "AA: Fear Storm",                  &kAaFearStorm },
	{ "AA: Pestilent Paralysis",         &kAaParalysis },
	{ "AA: Encroaching Darkness",        &kAaEncroaching },
	{ "AA: Dying Grasp",                 &kAaDyingGrasp },
	{ "AA: Life Burn",                   &kAaLifeBurn },
	{ "AA: Lifeburn Recourse",           &kAaLifeBurnRecourse },
	{ "AA: Reluctant Lifeshare",         &kAaLifeshare },
	{ "AA: Death Bloom",                 &kAaDeathBloom },
	{ "AA: Gathering Dusk",              &kAaGatheringDusk },
	{ "AA: Duskfall Empowerment",        &kAaDuskfall },
	{ "AA: Hand of Death",               &kAaHandDeath },
	{ "AA: Spire of Necromancy",         &kAaSpire },
	{ "AA: Funeral Pyre",                &kAaFuneralPyre },
	{ "AA: Heretic's Twincast",          &kAaTwincast },
	{ "AA: Mercurial Torment",           &kAaMercurial },
	{ "AA: Defiler's Synergy",           &kAaSynergy },
	{ "AA: Mortifier's Unity",           &kAaUnity },
	{ "AA: Scent of Thule",              &kAaScentThule },
	{ "AA: Death's Malaise",             &kAaMalaise },
	{ "AA: Death's Effigy",              &kAaEffigy },
	{ "AA: Death Peace",                 &kAaDeathPeace },
	{ "AA: Embalmer's Carapace",         &kAaCarapace },
	{ "AA: Gift of the Grave",           &kAaGiftGrave },
	{ "AA: Harmshield",                  &kAaHarmshield },
	{ "AA: Embrace the Decay",           &kAaEmbraceDecay },
	{ "AA: Frenzy of the Dead",          &kAaFrenzy },
	{ "AA: Companion's Necromancy",      &kAaCompanionNecro },
	{ "AA: Lingering Nightmares",        &kAaNightmares },
	{ "AA: Cascade of Decay",            &kAaCascadeRot },
	{ "AA: Form of Death",               &kAaFormDeath },
	{ "AA: Flesh to Bone",               &kAaFleshBone },
	{ "AA: Summon Remains",              &kAaSummonRemains },
	{ "AA: Whisperwind",                 &kAaWhisperwind },
	{ "AA: Levant",                      &kAaLevant },
	{ "AA: Convergence",                 &kAaConvergence },
	{ "AA: Dead Man Floating",           &kAaFloating },
	{ "AA: Invisibility versus Undead",  &kAaInvisUndead },
	{ "AA: Blood Magic",  &kAaBloodMagic },
};

const GroupLine kNumbered[] = {
	{ 0, Venin },       { 2, Bulwark },     { 3, Decay },       { 4, Blood },       { 6, Essence },
	{ 8, Sigil },       { 9, Curse },       { 10, Grasp },      { 12, Scent },      { 13, Darkness },
	{ 14, Shriek },     { 15, Ward },       { 16, Skin },       { 17, Manacles },   { 18, Mending },
	{ 19, Pyre },       { 20, Haze },       { 21, Side },       { 22, Command },    { 24, Aegis },
	{ 25, Venom },      { 26, Orb },        { 27, Fetish },     { 28, Mind },       { 29, BloodPyre },
	{ 30, Splort },     { 31, Putrefaction }, { 32, Shadow },   { 33, Ignite },     { 34, Sphere },
	{ 35, Ally },       { 36, Bestow },     { 37, Pustules },   { 38, Skeletons },  { 40, SwiftVenom },
	{ 41, Cascading },  { 42, SwiftDecon }, { 43, Flashblaze }, { 44, SwiftSickness }, { 45, Scapegoat },
	{ 46, SwiftLifedraw }, { 47, Bones },   { 48, Grip },       { 49, Veil },       { 50, Chaotic },
	{ 51, Wounds },     { 52, Crippling },  { 53, Binding },    { 54, ShieldFate }, { 56, Pact },
	{ 57, Soul },       { 58, Leech },      { 59, Scourge },    { 60, FleshPoison }, { 62, Paroxysm },
	{ 63, Paroxysm },   { 64, Echo },       { 65, GripDecay },  { 66, Malevolent },
};

const GroupLine kModern[] = {
	{ 10, Venin },      { 20, Decay },      { 30, Blood },      { 40, Essence },    { 50, Sigil },
	{ 60, Curse },      { 70, Grasp },      { 80, Scent },      { 90, Darkness },   { 100, Shriek },
	{ 120, Skin },      { 130, Manacles },  { 140, Mending },   { 150, Pyre },      { 160, Haze },
	{ 170, Side },      { 180, Command },   { 190, Aegis },     { 200, Venom },     { 230, Mind },
	{ 240, BloodPyre }, { 260, Putrefaction }, { 270, Shadow }, { 280, Ignite },    { 290, Sphere },
	{ 300, Ally },      { 310, Bestow },    { 330, Skeletons }, { 340, SwiftVenom }, { 350, Cascading },
	{ 360, SwiftDecon }, { 370, Flashblaze }, { 380, SwiftSickness }, { 400, SwiftLifedraw }, { 410, Bones },
	{ 420, Grip },      { 440, Chaotic },   { 450, Wounds },    { 480, ShieldFate }, { 500, Soul },
	{ 510, Leech },     { 520, Scourge },   { 540, Paroxysm },  { 541, Paroxysm },  { 560, GripDecay },
	{ 570, Malevolent }, { 600, Reaper },
};

const GroupLine kOther[] = {
	{ 9000, Venin },    { 9002, Bulwark },  { 9003, Decay },    { 9004, Blood },    { 9006, Essence },
	{ 9008, Essence },  { 9011, Sigil },    { 9012, Curse },    { 9013, Grasp },    { 9015, Scent },
	{ 9016, Darkness }, { 9017, Shriek },   { 9019, Ward },     { 9020, Skin },     { 9021, Manacles },
	{ 9022, Mending },  { 9024, BloodPyre }, { 9025, Putrefaction }, { 9027, Haze }, { 9028, Side },
	{ 9029, Command },  { 9030, Undead },   { 9031, Venom },    { 9032, Orb },      { 9033, Fetish },
	{ 9034, Mind },     { 9035, Pyre },
	{ 1599, Mirror },   { 1716, Scent },    { 4566, Manacles }, { 11014, Aegis },   { 29629, Smoldering },
	{ 16106, Encroaching },
	{ 6202, Shielding }, { 6302, Shielding }, { 6402, Shielding }, { 6502, Shielding }, { 6602, Shielding },
	{ 6702, Shielding }, { 6802, Shielding }, { 6902, Shielding }, { 125140030, Shielding },
	{ 126140030, Shielding },
	{ 305994, Reaper },  { 1205994, Reaper }, { 1505994, Reaper }, { 1605994, Reaper }, { 1705994, Reaper },
	{ 1805994, Reaper }, { 1905994, Reaper }, { 2005994, Reaper }, { 2105994, Reaper }, { 2205994, Reaper },
	{ 2305994, Reaper }, { 2405994, Reaper },
};

const NamedLine kAbilities[] = {
	{ "swarm of decay",                  SwarmDecay },
	{ "rise of bones",                   RiseBones },
	{ "wake the dead",                   WakeDead },
	{ "dark messenger",                  DarkMessenger },
	{ "servant of death",                ServantDeath },
	{ "dire charm",                      DireCharm },
	{ "dead mesmerization",              DeadMez },
	{ "rest the dead",                   RestDead },
	{ "fear storm",                      FearStorm },
	{ "pestilent paralysis",             Paralysis },
	{ "encroaching darkness",            Encroaching },
	{ "entombing darkness",              Encroaching },
	{ "dying grasp",                     DyingGrasp },
	{ "life burn",                       LifeBurn },
	{ "lifeburn recourse",               LifeBurnRecourse },
	{ "reluctant lifeshare",             Lifeshare },
	{ "death bloom",                     DeathBloom },
	{ "gathering dusk",                  GatheringDusk },
	{ "duskfall empowerment",            Duskfall },
	{ "hand of death",                   HandDeath },
	{ "spire of necromancy",             Spire },
	{ "funeral pyre",                    FuneralPyre },
	{ "heretic's twincast",              Twincast },
	{ "mercurial torment",               Mercurial },
	{ "defiler's synergy",               Synergy },
	{ "mortifier's unity",               Unity },
	{ "scent of thule",                  ScentThule },
	{ "death's malaise",                 Malaise },
	{ "death's necromantic effigy",      Effigy },
	{ "death peace",                     DeathPeace },
	{ "embalmer's carapace",             Carapace },
	{ "gift of the grave",               GiftGrave },
	{ "harmshield",                      Harmshield },
	{ "embrace the decay",               EmbraceDecay },
	{ "frenzy of the dead",              Frenzy },
	{ "companion's necromancy",          CompanionNecro },
	{ "lingering nightmares",            Nightmares },
	{ "cascade of decay rot",            CascadeRot },
	{ "form of death",                   FormDeath },
	{ "flesh to bone",                   FleshBone },
	{ "summon remains",                  SummonRemains },
	{ "whisperwind",                     Whisperwind },
	{ "levant",                          Levant },
	{ "convergence",                     Convergence },
	{ "dead man floating",               Floating },
	{ "dead men floating",               Floating },
	{ "perfected dead man floating",     Floating },
	{ "perfected dead men floating",     Floating },
	{ "invisibility versus undead",      InvisUndead },
	{ "perfected invisibility versus undead", InvisUndead },
	{ "group perfected invisibility to undead", InvisUndead },
	{ "improved invisibility to undead", InvisUndead },
	{ "blood magic",  BloodMagic },
};

const NamedLine kOlder[] = {
	{ "neurotoxin", Venin },             { "ancient: lifebane", Venin },      { "acikin", Venin },
	{ "call for blood", Blood },
	{ "lifetap", Essence },              { "lifespike", Essence },            { "lifedraw", Essence },
	{ "siphon life", Essence },          { "drain spirit", Essence },         { "drain soul", Essence },
	{ "spirit tap", Essence },           { "gangrenous touch of zum`uul", Essence },
	{ "touch of night", Essence },       { "touch of mujaki", Essence },      { "soulspike", Essence },
	{ "ancient: touch of orshilak", Essence }, { "trucidation", Essence },
	{ "leech", Grasp },                  { "bond of death", Grasp },          { "vexing mordinia", Grasp },
	{ "saryrn's kiss", Grasp },          { "night's beckon", Grasp },         { "fang of death", Grasp },
	{ "zevfeer's theft of vitae", Leech }, { "night stalker", Leech },
	{ "plague", Decay },                 { "dark plague", Decay },            { "chaos plague", Decay },
	{ "cessation of cor", Grip },        { "grip of mori", Grip },
	{ "horror", Curse },                 { "dark nightmare", Curse },         { "ancient: curse of mori", Curse },
	{ "venom of the snake", Haze },      { "blood of thule", Haze },          { "chaos venom", Haze },
	{ "corath venom", Venom },
	{ "heat blood", BloodPyre },         { "boil blood", BloodPyre },         { "ignite blood", BloodPyre },
	{ "pyrocruor", BloodPyre },          { "funeral pyre of kelador", BloodPyre }, { "night fire", BloodPyre },
	{ "pyre of mori", BloodPyre },       { "dread pyre", BloodPyre },
	{ "ignite bones", Bones },           { "incinerate bones", Bones },       { "chill bones", Bones },
	{ "conglaciation of bone", Bones },
	{ "clinging darkness", Darkness },   { "engulfing darkness", Darkness },  { "dooming darkness", Darkness },
	{ "cascading darkness", Darkness },  { "devouring darkness", Darkness },  { "embracing darkness", Darkness },
	{ "desecrating darkness", Darkness },
	{ "scent of dusk", Scent },          { "scent of shadow", Scent },        { "scent of darkness", Scent },
	{ "scent of midnight", Scent },
	{ "fear", Shriek },                  { "screaming terror", Shriek },      { "invoke fear", Shriek },
	{ "dread of night", Shriek },        { "trepidation", Shriek },           { "eidolon voice", Shriek },
	{ "eidolon bellow", Shriek },        { "eidolon howl", Shriek },          { "unholy voice", Shriek },
	{ "unholy bellow", Shriek },         { "unholy howl", Shriek },
	{ "shackle of bone", Manacles },     { "shackle of spirit", Manacles },   { "paralyzing earth", Manacles },
	{ "immobilize", Manacles },          { "greater immobilize", Manacles },  { "petrifying earth", Manacles },
	{ "death's silence", Manacles },     { "dark hold", Manacles },
	{ "dominate undead", Command },      { "beguile undead", Command },       { "cajole undead", Command },
	{ "enslave death", Command },        { "word of terris", Command },       { "word of chaos", Command },
	{ "expulse undead", Undead },        { "dismiss undead", Undead },        { "expel undead", Undead },
	{ "banish undead", Undead },         { "exile undead", Undead },          { "destroy undead", Undead },
	{ "desolate undead", Undead },      { "banishment of shadows", Undead },
	{ "crippling claudication", Crippling },
	{ "mind wrack", Mind },              { "mind flay", Mind },
	{ "soul orb", Orb },                 { "shadow orb", Orb },
	{ "shieldskin", Skin },              { "leatherskin", Skin },             { "steelskin", Skin },
	{ "diamondskin", Skin },             { "manaskin", Skin },                { "force shield", Skin },
	{ "dull pain", Skin },
	{ "shield of the magi", Bulwark },   { "shield of the arcane", Bulwark }, { "shield of maelin", Bulwark },
	{ "shadow guard", Bulwark },
	{ "quivering veil of xarn", Veil },
	{ "reflect", Mirror },
	{ "lich", Lich },                    { "demi lich", Lich },               { "arch lich", Lich },
	{ "ancient: master of death", Lich }, { "seduction of saryrn", Lich },    { "ancient: seduction of chaos", Lich },
	{ "dark possession", Lich },
	{ "grave pact", Bestow },
	{ "intensify death", Sigil },        { "augment death", Sigil },          { "augmentation of death", Sigil },
	{ "rune of death", Sigil },          { "glyph of darkness", Sigil },
	{ "mend bones", Mending },           { "renew bones", Mending },          { "touch of death", Mending },
	{ "dark salve", Mending },
	{ "ward of calliav", Aegis },        { "guard of calliav", Aegis },       { "protection of calliav", Aegis },
	{ "bulwark of calliav", Aegis },
	{ "splurt", Splort },
	{ "summon corpse", Reaper },
	{ "feign death", Feign },
	{ "shadow of death", InvisUndead },
	{ "smoldering bones", Smoldering },
};

bool Contains(std::string_view s, std::string_view part)
{
	return s.find(part) != std::string_view::npos;
}

int BySpell(const eqlib::EQ_Spell* sp, std::string_view base)
{
	const std::string_view extra(sp->Extra, strnlen(sp->Extra, sizeof(sp->Extra)));
	if (StartsWith(extra, "PCPetNec"))
	{
		if (Contains(extra, "Skel2Ice"))  return PetBones;
		if (Contains(extra, "Skel2Char")) return PetCharred;
		if (Contains(extra, "Skel2"))     return PetSkeleton;
		if (Contains(extra, "Spec2"))
		{
			if (Contains(base, "shade") || Contains(base, "shadow") || Contains(base, "conqueror"))
				return PetShade;
			if (Contains(base, "assassin") || Contains(base, "servant") || Contains(base, "murderer"))
				return PetAssassin;
			return PetSpecter;
		}
	}
	if (StartsWith(extra, "PCIObNec") && Contains(extra, "Trap"))
		return Fetish;
	for (const NamedLine& n : kOlder)
		if (base == n.name)
			return n.line;
	if (StartsWith(base, "form of "))
		return PetForm;
	return -1;
}

} // namespace

const ClassCatalogue& NecromancerCatalogue()
{
	static const ClassCatalogue c{ "Necromancer", eqlib::Necromancer, kLines, 9, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
