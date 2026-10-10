#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

namespace eqgfx::spellfx {

namespace {

constexpr Pair kSkBlood    { { 1.00f, 0.30f, 0.25f, 0.95f }, { 0.40f, 0.00f, 0.05f, 0.90f } };
constexpr Pair kSkShadow   { { 0.55f, 0.20f, 0.75f, 0.90f }, { 0.08f, 0.00f, 0.12f, 0.85f } };
constexpr Pair kSkBile     { { 0.70f, 1.00f, 0.40f, 0.90f }, { 0.20f, 0.35f, 0.05f, 0.85f } };
constexpr Pair kSkSoul     { { 0.80f, 0.95f, 1.00f, 0.80f }, { 0.30f, 0.20f, 0.60f, 0.60f } };
constexpr Pair kSkEmber    { { 1.00f, 0.55f, 0.20f, 1.00f }, { 0.45f, 0.02f, 0.02f, 0.90f } };
constexpr Pair kSkMend     { { 1.00f, 0.45f, 0.40f, 0.90f }, { 0.30f, 0.90f, 0.40f, 0.80f } };
constexpr Pair kSkChitin   { { 0.50f, 0.30f, 0.60f, 0.95f }, { 0.10f, 0.05f, 0.15f, 0.90f } };
constexpr Pair kSkVine     { { 0.35f, 0.05f, 0.08f, 0.95f }, { 0.80f, 0.10f, 0.15f, 0.95f } };
constexpr Pair kSkBatWing  { { 0.35f, 0.10f, 0.40f, 0.90f }, { 0.05f, 0.00f, 0.08f, 0.80f } };

constexpr uint32_t DARK = OVER;

LOOK(kSkTouch, "touch",
	G(IM, Glyph_Hand, 0.9f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kSkBlood, 1.6f, 0, DARK),
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.5f, 16, 0, FIX, kSkBlood, 2.0f, 0.10f),
	F(IM, FxForm_Vortex, 1.1f, 4.5f, 64, 4, 1.0f, 2.0f, -3.0f, FIX, kSkBlood, 1.8f, 0.05f),
	F(RE, FxForm_Sparkle, 1.0f, 6.0f, 32, 16, 0.12f, 2, 0, FIX, kSkBlood, 2.0f, 0.06f, ADD, 1, 0.5f));

LOOK(kSkSiphon, "siphon",
	F(TR, FxForm_Drain, 1, 0, 64, 4, 2.0f, 20, 0, FIX, kSkBile, 2.2f, 0.10f),
	G(IM, Glyph_Spirit, 0.6f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kSkSoul, 1.8f),
	F(IM, FxForm_Vortex, 1.0f, 5.0f, 64, 4, 1.0f, 2.0f, 3.0f, FIX, kSkBile, 1.8f, 0.05f));

LOOK(kSkDire, "dire tap",
	G(IM, Glyph_Hand, 1.3f, 9.0f, 4, 1, GA_Drop, 0, FIX, kSkShadow, 1.8f, 0, DARK),
	F(TR, FxForm_Drain, 1, 0, 96, 5, 2.2f, 24, 0, FIX, kSkBlood, 2.4f, 0.12f, ADD, 1, 0.35f),
	F(IM, FxForm_Vortex, 1.4f, 6.0f, 96, 5, 1.2f, 2.5f, -4.0f, FIX, kSkBlood, 2.2f, 0.06f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.4f, 1.0f, 72, 2, 1.0f, 16, 0, FIX, kSkBlood, 0.9f, 0.10f, ADD, 1, 0.35f));

LOOK(kSkBond, "bond",
	G(AU, Glyph_Link, 0.35f, 4.5f, 3, 6, GA_PopSpin, 1.1f, FIX, kSkBlood, 6.0f, 0, DARK),
	F(TR, FxForm_Drain, 1, 0, 40, 2, 0.8f, 10, 0, FIX, kSkBlood, 2.5f, 0.07f),
	F(AU, FxForm_Orbit, 1.2f, 5.5f, 40, 8, 0, 0, 0, FIX, kSkBlood, 6.0f, 0.04f, ADD, 2.0f));

LOOK(kSkBite, "bite",
	G(IM, Glyph_Fang, 1.0f, 6.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kSkBlood, 1.2f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0.2f, 10, 0, FIX, kSkBlood, 0.7f, 0.08f),
	F(TR, FxForm_Drain, 1, 0, 48, 2, 1.0f, 12, 0, FIX, kSkBlood, 1.4f, 0.08f, ADD, 1, 0.3f));

LOOK(kSkRecourse, "recourse",
	F(IM, FxForm_Sparkle, 1.1f, 6.0f, 24, 12, 0.10f, 2, 0, FIX, kSkMend, 1.4f),
	F(IM, FxForm_Ring, 1.0f, 3.0f, 32, 0, 2.0f, 1.0f, 0.8f, FIX, kSkMend, 1.2f, 0.04f, COMET));

LOOK(kSkInsidious, "insidious",
	F(RE, FxForm_Bloom, 1.0f, 2.0f, 96, 3, 0, 0, 0, FIX, kSkBile, 2.0f, 0.07f, ADD, 1, 0, AE),
	F(RE, FxForm_Vortex, 1.3f, 5.0f, 64, 4, 1.2f, 2.0f, -3.0f, FIX, kSkBile, 2.0f, 0.05f),
	F(RE, FxForm_Sparkle, 1.4f, 6.0f, 40, 20, 0.12f, 2, 0, FIX, kSkBile, 2.0f));

LOOK(kSkBloodNuke, "blood",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.3f, 1.2f, FIX, kSkBlood, 0.8f, 0.16f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 72, 2, 1.5f, 14, 0, FIX, kSkBlood, 0.9f, 0.10f),
	G(IM, Glyph_Drop, 0.25f, 7.0f, 0, 5, GA_Rain, 1.0f, FIX, kSkBlood, 1.6f));

LOOK(kSkSpear, "spear",
	F(TR, FxForm_Bolt, 1, 0, 128, 2, 0.1f, 0.9f, 0.3f, FIX, kSkBile, 0.7f, 0.14f),
	F(IM, FxForm_Burst, 2.6f, 0.5f, 96, 3, 0.8f, 18, 0, FIX, kSkBile, 1.0f, 0.12f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kSkBile, 1.6f, 0.06f),
	F(IM, FxForm_Flame, 0.9f, 4.5f, 64, 16, 0.6f, 0.8f, 0, FIX, kSkBile, 1.4f, 0.07f));

LOOK(kSkPlague, "plague",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.45f, 0.3f, 1.2f, FIX, kDisease, 0.8f, 0.16f),
	G(IM, Glyph_Insect, 0.3f, 5.0f, 0, 6, GA_Orbit, 1.3f, FIX, kDisease, 3.0f),
	F(IM, FxForm_Bubbles, 1.1f, 5.0f, 64, 8, 0.2f, 0, 0, FIX, kDisease, 3.0f, 0.04f, DARK));

LOOK(kSkBlight, "blight",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.6f, 0.4f, 1.2f, FIX, kCorrupt, 0.8f, 0.18f, DARK),
	F(IM, FxForm_Vortex, 1.1f, 4.5f, 64, 4, 1.0f, 2.0f, -2.0f, FIX, kCorrupt, 3.0f, 0.05f, DARK),
	G(IM, Glyph_Skull, 0.6f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kCorrupt, 3.0f, 0, DARK));

LOOK(kSkDarkness, "darkness",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.6f, 0.4f, 1.0f, FIX, kSkShadow, 0.8f, 0.18f, DARK),
	F(IM, FxForm_Dome, 1.4f, 5.5f, 72, 6, 2, 0, 0, FIX, kSkShadow, 3.0f, 0.06f, DARK),
	F(IM, FxForm_Vortex, 1.2f, 1.5f, 48, 3, 1.0f, 1.5f, -2.0f, FIX, kSkShadow, 3.0f, 0.05f, DARK));

LOOK(kSkConstrict, "constriction",
	F(AU, FxForm_Vines, 0.9f, 5.5f, 120, 5, 1.6f, 0, 0, FIX, kSkVine, 6.0f, 0.07f, DARK),
	F(AU, FxForm_Thorns, 0.9f, 5.5f, 30, 5, 1.6f, 5, 0, FIX, kSkBlood, 6.0f, 0.05f, DARK),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0, 8, 0, FIX, kSkBlood, 0.8f, 0.07f, ADD | FxFlag_Reverse));

LOOK(kSkCurse, "curse",
	G(IM, Glyph_ArrowDown, 0.35f, 7.0f, 0, 3, GA_Orbit, 1.2f, FIX, kSkBlood, 3.0f),
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 5, 2, 8, 1, FIX, kSkShadow, 2.4f, 0.05f, DARK));

LOOK(kSkTorrent, "torrent",
	F(IM, FxForm_Rain, 1.0f, 14.0f, 60, 50, 0, 0.1f, 0, FIX, kSkShadow, 2.4f, 0.07f, DARK),
	F(IM, FxForm_Cracks, 1.8f, 0, 48, 6, 1.0f, 0, 0, FIX, kSkShadow, 2.4f, 0.06f, DARK));

LOOK(kSkTerror, "terror",
	G(IM, Glyph_Skull, 1.1f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kSkEmber, 2.2f),
	F(IM, FxForm_Bloom, 2.0f, 2.5f, 48, 3, 0, 0, 0, FIX, kSkBlood, 1.8f, 0.06f));

LOOK(kSkPower, "challenge for power",
	G(RE, Glyph_Horn, 0.8f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kSkShadow, 1.6f, 0, DARK),
	F(RE, FxForm_Bloom, 2.0f, 5.0f, 48, 3, 0, 0, 0, FIX, kSkBlood, 1.6f, 0.05f),
	G(IM, Glyph_Banner, 0.9f, 8.0f, 4, 1, GA_Drop, 0, FIX, kSkShadow, 2.4f, 0, DARK),
	F(IM, FxForm_Burst, 2.2f, 0.4f, 64, 2, 0.6f, 10, 0, FIX, kSkBlood, 0.9f, 0.09f, ADD, 1, 0.35f));

LOOK(kSkRevile, "revile",
	G(RE, Glyph_Eye, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kSkEmber, 2.0f),
	F(RE, FxForm_Bloom, 1.0f, 2.0f, 96, 3, 0, 0, 0, FIX, kSkBlood, 2.0f, 0.07f, ADD, 1, 0, AE),
	F(RE, FxForm_Burst, 1.0f, 0.3f, 96, 2, 0.3f, 16, 0, FIX, kSkShadow, 1.0f, 0.10f, DARK, 1, 0, AE));

LOOK(kSkAudacity, "audacity",
	F(RE, FxForm_Bloom, 2.2f, 4.0f, 48, 3, 0, 0, 0, FIX, kSkBlood, 1.4f, 0.06f),
	G(IM, Glyph_Fist, 1.0f, 8.0f, 4, 1, GA_Drop, 0, FIX, kSkShadow, 1.8f, 0, DARK),
	F(IM, FxForm_Burst, 2.6f, 0.3f, 72, 2, 0.4f, 16, 0, FIX, kSkBlood, 1.0f, 0.10f, ADD, 1, 0.35f));

LOOK(kSkAcrimony, "acrimony",
	G(IM, Glyph_Eye, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSkBlood, 2.0f),
	F(IM, FxForm_Spire, 0.8f, 4.0f, 72, 12, 1.0f, 2.6f, 0, FIX, kSkBlood, 1.2f, 0.06f));

LOOK(kSkBargain, "bargain",
	G(RE, Glyph_Pentagram, 1.8f, 0.0f, 2, 1, GA_PopRoll, 0, FIX, kSkBlood, 2.0f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 1.0f, 24, 0, FIX, kSkBlood, 1.4f, 0.12f, ADD, 1, 0, AE),
	F(RE, FxForm_Cracks, 0.8f, 0, 64, 8, 1.2f, 0, 0, FIX, kSkBlood, 1.8f, 0.06f, ADD, 1, 0, AE));

LOOK(kSkShroud, "shroud",
	F(IM, FxForm_Flame, 1.05f, 5.8f, 96, 24, 0.9f, 0.9f, 0, FIX, kSkShadow, 3.0f, 0.08f, DARK),
	G(IM, Glyph_Bat, 0.4f, 5.5f, 4, 4, GA_Orbit, 1.4f, FIX, kSkShadow, 3.0f, 0, DARK),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 24, 12, 0.10f, 0, 0, FIX, kSkBlood, 3.0f));

LOOK(kSkHorror, "horror",
	G(IM, Glyph_Spirit, 0.45f, 5.0f, 0, 3, GA_Orbit, 1.4f, FIX, kSkSoul, 3.0f),
	F(IM, FxForm_Vortex, 1.1f, 4.0f, 64, 4, 1.0f, 1.5f, -2.0f, FIX, kSkShadow, 2.6f, 0.05f, DARK));

LOOK(kSkMental, "mental",
	G(IM, Glyph_Spiral, 0.8f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kSkShadow, 3.0f, 0, DARK),
	G(IM, Glyph_Eye, 0.5f, 7.4f, 4, 1, GlyphAnim_Pop, 0, FIX, kSkEmber, 3.0f));

LOOK(kSkCarapace, "carapace",
	F(IM, FxForm_Crystal, 1.1f, 5.0f, 60, 14, 0, 0.5f, 0, FIX, kSkChitin, 2.8f, 0.07f, DARK),
	G(IM, Glyph_Insect, 0.6f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kSkShadow, 2.4f, 0, DARK),
	F(IM, FxForm_Dome, 1.6f, 6.3f, 88, 6, 2, 0, 0, FIX, kSkShadow, 2.0f, 0.03f));

LOOK(kSkUmbral, "umbral skin",
	F(IM, FxForm_Orbit, 1.3f, 6.0f, 60, 12, 0, 0, 0, FIX, kSkShadow, 3.0f, 0.05f, DARK, 2.0f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 72, 18, 0.9f, 0.7f, 0, FIX, kSkShadow, 2.6f, 0.06f, DARK));

LOOK(kSkRotSkin, "rotting skin",
	Persist(F(AU, FxForm_Bubbles, 1.1f, 6.0f, 48, 6, 0.18f, 0, 0, FIX, kSkBile, 0, 0.035f, DARK)),
	F(IM, FxForm_Bubbles, 1.1f, 6.0f, 72, 10, 0.2f, 0, 0, FIX, kSkBile, 3.0f, 0.04f, DARK),
	G(IM, Glyph_Skull, 0.3f, 4.0f, 0, 3, GA_Orbit, 1.2f, FIX, kSkBile, 3.0f, 0, DARK));

LOOK(kSkDrape, "drape",
	F(IM, FxForm_Crystal, 1.0f, 5.5f, 60, 12, 0, 0.7f, 0, FIX, kSkShadow, 2.6f, 0.06f, DARK),
	F(IM, FxForm_Helix, 1.1f, 6.0f, 96, 2, 1.0f, 0, 0.3f, FIX, kSkBlood, 2.6f, 0.06f, COMET),
	G(IM, Glyph_Shield, 0.7f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kSkShadow, 2.4f, 0, DARK));

LOOK(kSkMantle, "mantle",
	Persist(F(AU, FxForm_Helix, 1.05f, 6.0f, 96, 2, 1.0f, 0, 0.3f, FIX, kSkBlood, 0, 0.06f, COMET)),
	G(IM, Glyph_Scythe, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kSkShadow, 3.0f, 0, DARK),
	F(IM, FxForm_Flame, 1.05f, 5.5f, 72, 18, 0.9f, 0.7f, 0, FIX, kSkShadow, 2.6f, 0.06f, DARK));

LOOK(kSkCall, "call",
	G(IM, Glyph_Bat, 0.35f, 6.0f, 0, 6, GA_Orbit, 1.8f, FIX, kSkShadow, 3.0f, 0, DARK),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 60, 12, 2.0f, 0, 0, FIX, kSkShadow, 2.4f, 0.05f, DARK));

LOOK(kSkCovenant, "covenant",
	G(IM, Glyph_Pentagram, 1.6f, 0.0f, 2, 1, GA_PopRoll, 0, FIX, kSkBlood, 3.0f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, 3.0f, FIX, kSkShadow, 2.4f, 0.05f, DARK),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.12f, 2, 0, FIX, kMana, 2.4f));

LOOK(kSkLastBreath, "last breath",
	G(IM, Glyph_Skull, 0.7f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kGrey, 3.0f, 0, DARK),
	F(IM, FxForm_Bloom, 1.6f, 1.0f, 40, 2, 0, 0, 0, FIX, kSkSoul, 3.0f, 0.04f));

LOOK(kSkSweetBreath, "sweet breath",
	F(IM, FxForm_Wind, 1.1f, 3.5f, 48, 8, 1.5f, 0, 0, FIX, kSkMend, 2.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.1f, 6.0f, 32, 16, 0.12f, 0, 0, FIX, kHeal, 2.0f));

LOOK(kSkDemeanor, "demeanor",
	G(IM, Glyph_Mask, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSkShadow, 3.0f, 0, DARK),
	F(IM, FxForm_Ring, 1.3f, 0.3f, 40, 0, 0.6f, 1.0f, 0.3f, FIX, kSkBlood, 3.0f, 0.05f, COMET));

LOOK(kSkGuardian, "guardian",
	Persist(F(AU, FxForm_Wings, 1.0f, 4.4f, 70, 10, 0.3f, 4.0f, 0, FIX, kSkBatWing, 1.2f, 0.07f,
		DARK | FxFlag_NoEnvelope)),
	F(IM, FxForm_Dome, 1.9f, 7.2f, 112, 8, 3, 0, 0, FIX, kSkBlood, 3.5f, 0.04f),
	G(IM, Glyph_Skull, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kSkEmber, 3.5f));

LOOK(kSkDisruption, "disruption",
	F(IM, FxForm_Rune, 1.6f, 3.0f, 60, 7, 3, 7, 2, FIX, kSkBlood, 2.4f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.12f, 0, 0, FIX, kSkShadow, 2.4f, 0.06f, DARK));

LOOK(kSkBloodletting, "bloodletting",
	G(IM, Glyph_Heart, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSkBlood, 2.6f),
	G(IM, Glyph_Drop, 0.25f, 7.0f, 0, 5, GA_Rain, 1.0f, FIX, kSkBlood, 2.0f));

LOOK(kSkMinion, "minion",
	F(RE, FxForm_Cracks, 3.5f, 0, 48, 6, 1.2f, 0, 0, FIX, kSkBlood, 4.0f, 0.08f, DARK),
	G(RE, Glyph_Skeleton, 3.0f, 3.0f, 3, 1, GA_RiseWiggle, 0, FIX, kBone, 4.8f, 0.2f, DARK),
	G(RE, Glyph_Scythe, 1.1f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kSkShadow, 3.5f, 0, DARK),
	F(RE, FxForm_Rune, 3.0f, 0, 60, 5, 2, 8, 0, FIX, kSkBlood, 4.0f));

LOOK(kSkPetGift, "dark gift",
	G(IM, Glyph_Skull, 0.5f, 6.5f, 4, 1, GA_PopAscend, 0, FIX, kSkEmber, 2.0f),
	F(IM, FxForm_Flame, 1.0f, 5.0f, 72, 18, 0.8f, 0.8f, 0, FIX, kSkEmber, 2.4f, 0.07f));

LOOK(kSkSacrifice, "sacrifice",
	G(IM, Glyph_Pentagram, 1.8f, 0.0f, 2, 1, GlyphAnim_Pop, 0, FIX, kSkBlood, 2.4f),
	F(IM, FxForm_Burst, 3.0f, 1.0f, 96, 3, 1.5f, 16, 0, FIX, kSkBlood, 1.2f, 0.12f),
	G(IM, Glyph_Skull, 0.8f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kBone, 1.8f, 0, DARK));

LOOK(kSkReaper, "reaper's call",
	G(IM, Glyph_Scythe, 1.2f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kSkShadow, 3.0f, 0, DARK),
	F(IM, FxForm_Vortex, 1.6f, 4.0f, 64, 4, 1.2f, 2.0f, -3.0f, FIX, kSkShadow, 3.0f, 0.05f, DARK),
	G(IM, Glyph_Skull, 0.6f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kBone, 2.5f, 0.4f, DARK));

LOOK(kSkBlade, "blade",
	G(IM, Glyph_Sword, 0.9f, 4.0f, 4, 1, GA_PopRoll, 0, FIX, kSkBlood, 0.9f),
	F(IM, FxForm_Burst, 1.6f, 3.5f, 32, 1, 0, 10, 0, FIX, kSkBlood, 0.6f, 0.08f));

LOOK(kSkCrimson, "crimson blade",
	G(IM, Glyph_Scythe, 1.1f, 4.5f, 4, 1, GA_PopRoll, 0, FIX, kSkBlood, 1.0f),
	F(IM, FxForm_Spire, 0.4f, 4.0f, 64, 10, 3.0f, 5.0f, 0, FIX, kSkBlood, 0.7f, 0.07f),
	G(IM, Glyph_Drop, 0.2f, 4.5f, 0, 6, GA_Rain, 0.9f, FIX, kSkBlood, 1.2f));

LOOK(kSkReflexive, "reflexive",
	G(IM, Glyph_Shield, 0.9f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kSkShadow, 2.0f, 0, DARK),
	F(IM, FxForm_Burst, 2.0f, 3.0f, 64, 2, 0, 12, 0, FIX, kSkBlood, 1.0f, 0.07f, ADD | FxFlag_Reverse));

LOOK(kSkFang, "fang",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.4f, 1.2f, FIX, kSkShadow, 0.8f, 0.16f, DARK),
	G(IM, Glyph_Fang, 1.0f, 7.5f, 4, 1, GA_Drop, 0, FIX, kSkBlood, 1.8f),
	G(IM, Glyph_Moon, 0.8f, 8.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kSkShadow, 2.4f, 0.2f, DARK));

LOOK(kSkProcHeal, "dark mending",
	G(IM, Glyph_Heart, 0.3f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kSkMend, 1.4f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 20, 10, 0.10f, 0, 0, FIX, kSkMend, 1.2f));

LOOK(kSkSpite, "spite",
	G(IM, Glyph_Fang, 0.6f, 4.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kSkBlood, 0.8f),
	F(IM, FxForm_Burst, 1.3f, 3.5f, 32, 1, 0, 8, 0, FIX, kSkBlood, 0.5f, 0.07f));

LOOK(kSkGiftProc, "hateful gift",
	G(IM, Glyph_Skull, 0.3f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kSkEmber, 1.2f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 20, 10, 0.10f, 0, 0, FIX, kSkBlood, 1.0f));

LOOK(kAaHarmTouch, "harm touch",
	G(IM, Glyph_Hand, 1.8f, 11.0f, 4, 1, GA_Drop, 0, FIX, kSkShadow, 2.6f, 0, DARK),
	F(IM, FxForm_Flame, 1.0f, 12.0f, 112, 28, 0.1f, 3.5f, 0, FIX, kSkBlood, 2.4f, 0.10f, ADD, 1, 0.45f),
	F(IM, FxForm_Burst, 3.5f, 0.5f, 96, 3, 2.0f, 24, 0, FIX, kSkBlood, 1.4f, 0.12f, ADD, 1, 0.45f),
	F(IM, FxForm_Cracks, 3.0f, 0, 64, 8, 1.4f, 0, 0, FIX, kSkBlood, 3.0f, 0.08f, ADD, 1, 0.45f),
	G(IM, Glyph_Skull, 1.0f, 6.5f, 4, 1, GA_PopAscend, 0, FIX, kSkEmber, 2.4f, 0.5f));

LOOK(kAaLeechTouch, "leech touch",
	G(IM, Glyph_Hand, 0.9f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kSkBile, 1.6f),
	F(TR, FxForm_Drain, 1, 0, 64, 4, 1.8f, 20, 0, FIX, kSkBile, 2.2f, 0.10f));

LOOK(kAaHatesAttraction, "hate's attraction",
	F(TR, FxForm_Drain, 1, 0, 64, 3, 0.6f, 16, 0, FIX, kSkBlood, 1.6f, 0.10f),
	G(IM, Glyph_Eye, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSkEmber, 1.8f));

LOOK(kAaVisageDeath, "visage of death",
	G(IM, Glyph_Skull, 1.4f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kSkEmber, 4.0f),
	F(IM, FxForm_Flame, 1.05f, 6.0f, 112, 28, 0.9f, 1.0f, 0, FIX, kSkShadow, 4.0f, 0.09f, DARK));

LOOK(kAaVisageDecay, "visage of decay",
	G(IM, Glyph_Skull, 1.4f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kSkBile, 4.0f),
	F(IM, FxForm_Bubbles, 1.2f, 6.5f, 96, 12, 0.22f, 0, 0, FIX, kSkBile, 4.0f, 0.04f, DARK));

LOOK(kAaVeil, "veil of darkness",
	F(IM, FxForm_Dome, 1.5f, 6.5f, 96, 8, 3, 0, 0, FIX, kSkShadow, 3.5f, 0.07f, DARK),
	G(IM, Glyph_Eye, 0.7f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kSkShadow, 3.5f, 0, DARK));

LOOK(kAaScourgeSkin, "scourge skin",
	F(IM, FxForm_Vines, 1.1f, 5.5f, 120, 5, 1.1f, 0, 0, FIX, kSkVine, 3.0f, 0.07f, DARK),
	F(IM, FxForm_Thorns, 1.1f, 5.5f, 30, 5, 1.1f, 6, 0, FIX, kSkShadow, 3.0f, 0.06f, DARK));

LOOK(kAaThoughtLeech, "thought leech",
	F(TR, FxForm_Drain, 1, 0, 64, 3, 1.5f, 16, 0, FIX, kMana, 2.0f, 0.10f),
	G(IM, Glyph_Spiral, 0.8f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kMana, 2.0f));

LOOK(kAaSpireReavers, "spire of the reavers",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kSkBlood, 2.6f, 0.10f),
	G(RE, Glyph_Scythe, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kSkShadow, 2.6f, 0, DARK),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kSkBlood, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaChatteringBones, "chattering bones",
	F(IM, FxForm_Cracks, 3.0f, 0, 48, 6, 1.2f, 0, 0, FIX, kSkBlood, 4.0f, 0.08f, DARK),
	G(IM, Glyph_Skeleton, 1.8f, 2.5f, 3, 3, GA_RiseWiggle, 1.8f, FIX, kBone, 4.0f, 0, DARK));

LOOK(kAaDeathPeace, "death peace",
	G(IM, Glyph_Skull, 0.7f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kGrey, 2.5f, 0, DARK),
	F(IM, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kSkShadow, 2.0f, 0.05f, DARK | FxFlag_Reverse));

LOOK(kAaTheftDefense, "theft of defense",
	G(IM, Glyph_Shield, 0.8f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kSkBlood, 3.0f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, 3.0f, FIX, kSkShadow, 2.4f, 0.05f, DARK));

LOOK(kAaTheftLife, "theft of life",
	G(RE, Glyph_Heart, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kSkMend, 2.4f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 1.5f, 16, 0, FIX, kSkMend, 1.6f, 0.10f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 2, 0, FIX, kSkMend, 2.0f));

LOOK(kAaViciousBite, "vicious bite of chaos",
	G(IM, Glyph_Fang, 1.4f, 7.0f, 4, 1, GA_Drop, 0, FIX, kPrism, 1.6f),
	F(IM, FxForm_Burst, 2.6f, 2.0f, 96, 3, 1.0f, 16, 0, FIX, kPrism, 1.0f, 0.10f, ADD, 1, 0.35f));

LOOK(kAaExplosionSpite, "explosion of spite",
	G(RE, Glyph_Skull, 1.1f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kSkEmber, 1.8f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 1.5f, 24, 0, FIX, kSkBlood, 1.4f, 0.12f, ADD, 1, 0, AE),
	F(RE, FxForm_Bloom, 1.0f, 2.0f, 96, 3, 0, 0, 0, FIX, kSkShadow, 1.8f, 0.06f, DARK, 1, 0.2f, AE));

LOOK(kAaExplosionHatred, "explosion of hatred",
	F(IM, FxForm_Burst, 1.0f, 0.5f, 120, 3, 1.5f, 24, 0, FIX, kSkBlood, 1.4f, 0.12f, ADD, 1, 0, AE),
	G(IM, Glyph_Eye, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kSkEmber, 1.8f));

LOOK(kAaProjectionDoom, "projection of doom",
	G(RE, Glyph_Helm, 1.2f, 6.5f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kSkShadow, 3.5f, 0, DARK),
	F(RE, FxForm_Flame, 1.0f, 7.0f, 80, 20, 0.2f, 1.2f, 0, FIX, kSkBlood, 3.5f, 0.06f),
	F(RE, FxForm_Rune, 2.4f, 0, 60, 6, 1, 12, 0, FIX, kSkBlood, 3.5f));

LOOK(kAaPurityDeath, "purity of death",
	G(IM, Glyph_Skull, 0.8f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kWhite, 2.4f),
	F(IM, FxForm_Bloom, 1.8f, 1.0f, 40, 3, 0, 0, 0, FIX, kWhite, 2.0f, 0.05f));

LOOK(kAaSoulFlay, "soul flay",
	G(RE, Glyph_Spirit, 0.5f, 5.5f, 3, 5, GA_RingRise, 2.0f, FIX, kSkSoul, 2.6f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kMana, 2.2f));

LOOK(kAaEncroaching, "encroaching darkness",
	F(IM, FxForm_Dome, 1.2f, 1.2f, 72, 6, 2, 0, 0, FIX, kSkShadow, 4.0f, 0.06f, DARK),
	G(IM, Glyph_Link, 0.35f, 0.6f, 3, 6, GA_PopSpin, 1.1f, FIX, kSkShadow, 4.0f, 0, DARK));

LOOK(kAaHateStep, "hate step",
	F(RE, FxForm_Wind, 1.4f, 4.0f, 60, 12, 3.0f, 0, 0, FIX, kSkShadow, 1.2f, 0.05f, DARK),
	F(RE, FxForm_Burst, 2.5f, 0.2f, 64, 2, 0.3f, 12, 0, FIX, kSkBlood, 1.0f, 0.10f));

LOOK(kAaStreamHatred, "stream of hatred",
	F(IM, FxForm_Lightning, 1, 0, 64, 3, 0.4f, 20.0f, 0, FIX, kSkBlood, 0.9f, 0.18f),
	F(IM, FxForm_Burst, 1.5f, 1.0f, 48, 1, 0.5f, 8, 0, FIX, kSkBlood, 0.6f));

LOOK(kAaTvylsResolve, "t`vyl's resolve",
	G(IM, Glyph_Fist, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kSkShadow, 3.0f, 0, DARK),
	G(IM, Glyph_Scythe, 0.5f, 3.5f, 3, 2, GA_Orbit, 1.3f, FIX, kSkBlood, 3.0f));

LOOK(kAaReaversBargain, "reaver's bargain",
	G(IM, Glyph_Pentagram, 1.8f, 0.0f, 2, 1, GA_PopRoll, 0, FIX, kSkBlood, 3.0f),
	G(IM, Glyph_Heart, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSkBlood, 3.0f));

LOOK(kAaBobbingCorpse, "bobbing corpse",
	G(IM, Glyph_Skeleton, 0.8f, 5.0f, 4, 1, GA_PopBob, 0, FIX, kBone, 3.0f, 0, DARK),
	F(IM, FxForm_Bubbles, 1.1f, 6.0f, 48, 8, 0.2f, 0, 0, FIX, kWater, 3.0f, 0.04f));

LOOK(kAaCloakShadows, "cloak of shadows",
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kSkShadow, 2.5f, 0.05f, DARK),
	F(IM, FxForm_Dome, 1.8f, 6.5f, 96, 6, 2, 0, 0, FIX, kSkShadow, 2.0f, 0.03f, DARK));

LOOK(kAaVortex, "destructive vortex",
	F(IM, FxForm_Vortex, 1.6f, 6.0f, 96, 6, 1.4f, 3.0f, 6.0f, FIX, kSkBlood, 3.0f, 0.07f),
	F(IM, FxForm_Vortex, 1.3f, 5.0f, 64, 4, 1.2f, 2.0f, -5.0f, FIX, kSkShadow, 3.0f, 0.05f, DARK));

LOOK(kAaQuickSpear, "gift of the quick spear",
	G(IM, Glyph_Sword, 0.9f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kSkBile, 2.2f),
	F(IM, FxForm_Wind, 1.2f, 4.0f, 60, 10, 2.5f, 0, 0, FIX, kSkBile, 2.2f));

LOOK(kAaUnholyAura, "unholy aura",
	F(RE, FxForm_Rune, 1.0f, 0, 96, 7, 3, 14, 0, FIX, kSkBlood, 3.0f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Skull, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kSkEmber, 3.0f));

LOOK(kAaVoiceThule, "voice of thule",
	G(IM, Glyph_Mouth, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSkShadow, 2.4f, 0, DARK),
	F(IM, FxForm_Bloom, 2.0f, 3.0f, 48, 3, 0, 0, 0, FIX, kSkBlood, 2.0f, 0.06f));

LOOK(kAaMortalCoil, "mortal coil",
	F(IM, FxForm_Helix, 1.1f, 6.0f, 96, 4, 2.0f, 0, 0.8f, FIX, kSkBlood, 3.0f, 0.07f, COMET),
	G(IM, Glyph_Heart, 0.6f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSkBlood, 3.0f));

LOOK(kAaHandDeath, "hand of death",
	G(IM, Glyph_Hand, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSkShadow, 3.0f, 0, DARK),
	G(IM, Glyph_Skull, 0.3f, 4.5f, 0, 5, GA_Orbit, 1.4f, FIX, kSkEmber, 3.0f));

LOOK(kAaDarkLordUnity, "dark lord's unity",
	G(IM, Glyph_Crown, 0.9f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kShadow, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 7.5f, 96, 3, 2.0f, 0, 0.6f, FIX, kShadow, 3.0f, 0.06f, COMET),
	F(IM, FxForm_Helix, 1.1f, 7.5f, 96, 3, -2.0f, 0, 0.6f, FIX, kBlood, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.3f, 8.0f, 40, 20, 0.12f, 0, 0, FIX, kCorrupt, 2.6f));

LOOK(kAaDeathsEffigy, "death's effigy",
	G(IM, Glyph_Skeleton, 1.0f, 3.5f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kBone, 3.5f),
	F(IM, FxForm_Bloom, 1.6f, 0.3f, 64, 3, 0, 0, 0, FIX, kShadow, 2.4f, 0.06f, OVER),
	F(IM, FxForm_Flame, 0.9f, 6.0f, 64, 16, 0.2f, 1.2f, 0, FIX, kNecro, 3.0f, 0.06f));

LOOK(kAaUnbridledFear, "unbridled strike of fear",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.6f, 0.6f, FIX, kShadow, 0.8f, 0.10f),
	G(IM, Glyph_Skull, 1.0f, 7.5f, 4, 1, GlyphAnim_Pop | GlyphAnim_Wiggle, 0, FIX, kCorrupt, 2.0f),
	F(IM, FxForm_Burst, 2.4f, 1.5f, 72, 2, 1.0f, 16, 0, FIX, kCorrupt, 1.0f, 0.10f));

enum Line : int
{
	Touch, Siphon, Dire, Bond, Bite, Recourse, Insidious, BloodNuke, Spear, Plague, Blight, Darkness,
	Constrict, Curse, Torrent, Terror, Power, Revile, Audacity, Acrimony, Bargain, Shroud, Horror,
	Mental, Carapace, Umbral, RotSkin, Drape, Mantle, Call, Covenant, LastBreath, SweetBreath,
	Demeanor, Guardian, Disruption, Bloodletting, Minion, PetGift, Sacrifice, Reaper, Blade, Crimson,
	Reflexive, Fang, ProcHeal, Spite, GiftProc,
	HarmTouch, LeechTouch, HatesAttraction, VisageDeath, VisageDecay, Veil, ScourgeSkin, ThoughtLeech,
	SpireReavers, ChatteringBones, DeathPeace, TheftDefense, TheftLife, ViciousBite, ExplosionSpite,
	ExplosionHatred, ProjectionDoom, PurityDeath, SoulFlay, Encroaching, HateStep, StreamHatred,
	TvylsResolve, ReaversBargain, BobbingCorpse, CloakShadows, Vortex, QuickSpear, UnholyAura,
	VoiceThule, MortalCoil, HandDeath,
	DarkLordUnity, DeathsEffigy, UnbridledFear,
};

const LineLook kLines[] = {
	{ "Touch (lifetap)",               &kSkTouch },
	{ "Siphon (lifetap)",              &kSkSiphon },
	{ "Dire (big lifetap)",            &kSkDire },
	{ "Bond (lifetap over time)",      &kSkBond },
	{ "Bite (lifetap)",                &kSkBite },
	{ "Bite recourse (group heal)",    &kSkRecourse },
	{ "Insidious (area hate, tap)",    &kSkInsidious },
	{ "Blood (nuke)",                  &kSkBloodNuke },
	{ "Spear (nuke)",                  &kSkSpear },
	{ "Plague (disease)",              &kSkPlague },
	{ "Blight (corruption)",           &kSkBlight },
	{ "Darkness (dot, snare)",         &kSkDarkness },
	{ "Constriction (dot, debuff)",    &kSkConstrict },
	{ "Curse (debuff)",                &kSkCurse },
	{ "Torrent (armor debuff)",        &kSkTorrent },
	{ "Terror (hate)",                 &kSkTerror },
	{ "Challenge for Power (hate)",    &kSkPower },
	{ "Revile (area hate)",            &kSkRevile },
	{ "Audacity (hate)",               &kSkAudacity },
	{ "Acrimony (hate triggers)",      &kSkAcrimony },
	{ "Bargain (area hate, damage)",   &kSkBargain },
	{ "Shroud (weapon proc)",          &kSkShroud },
	{ "Horror (weapon proc)",          &kSkHorror },
	{ "Mental (weapon proc)",          &kSkMental },
	{ "Carapace (mitigation)",         &kSkCarapace },
	{ "Umbral Skin (defensive proc)",  &kSkUmbral },
	{ "Rotting Skin (damage shield)",  &kSkRotSkin },
	{ "Drape (armor)",                 &kSkDrape },
	{ "Mantle (heal proc)",            &kSkMantle },
	{ "Call (attack)",                 &kSkCall },
	{ "Covenant (mana)",               &kSkCovenant },
	{ "Last Breath (death save)",      &kSkLastBreath },
	{ "Sweet Breath (self heal)",      &kSkSweetBreath },
	{ "Demeanor",                      &kSkDemeanor },
	{ "Guardian (discipline)",         &kSkGuardian },
	{ "Disruption (heal trigger)",     &kSkDisruption },
	{ "Bloodletting (triggers)",       &kSkBloodletting },
	{ "Minion (undead pet)",           &kSkMinion },
	{ "Gift (pet buff)",               &kSkPetGift },
	{ "Sacrifice (pet)",               &kSkSacrifice },
	{ "Reaper's Call (summon corpse)", &kSkReaper },
	{ "Blade (strike)",                &kSkBlade },
	{ "Crimson Blade (strike)",        &kSkCrimson },
	{ "Reflexive (heal proc)",         &kSkReflexive },
	{ "Reciprocal Fang",               &kSkFang },
	{ "dark mending proc",             &kSkProcHeal },
	{ "spite proc",                    &kSkSpite },
	{ "hateful gift proc",             &kSkGiftProc },

	{ "AA: Harm Touch",                &kAaHarmTouch },
	{ "AA: Leech Touch",               &kAaLeechTouch },
	{ "AA: Hate's Attraction",         &kAaHatesAttraction },
	{ "AA: Visage of Death",           &kAaVisageDeath },
	{ "AA: Visage of Decay",           &kAaVisageDecay },
	{ "AA: Veil of Darkness",          &kAaVeil },
	{ "AA: Scourge Skin",              &kAaScourgeSkin },
	{ "AA: Thought Leech",             &kAaThoughtLeech },
	{ "AA: Spire of the Reavers",      &kAaSpireReavers },
	{ "AA: Chattering Bones",          &kAaChatteringBones },
	{ "AA: Death Peace",               &kAaDeathPeace },
	{ "AA: Cascading Theft of Defense", &kAaTheftDefense },
	{ "AA: Cascading Theft of Life",   &kAaTheftLife },
	{ "AA: Vicious Bite of Chaos",     &kAaViciousBite },
	{ "AA: Explosion of Spite",        &kAaExplosionSpite },
	{ "AA: Explosion of Hatred",       &kAaExplosionHatred },
	{ "AA: Projection of Doom",        &kAaProjectionDoom },
	{ "AA: Purity of Death",           &kAaPurityDeath },
	{ "AA: Soul Flay",                 &kAaSoulFlay },
	{ "AA: Encroaching Darkness",      &kAaEncroaching },
	{ "AA: Hate Step",                 &kAaHateStep },
	{ "AA: Stream of Hatred",          &kAaStreamHatred },
	{ "AA: T`Vyl's Resolve",           &kAaTvylsResolve },
	{ "AA: Reaver's Bargain",          &kAaReaversBargain },
	{ "AA: Bobbing Corpse",            &kAaBobbingCorpse },
	{ "AA: Cloak of Shadows",          &kAaCloakShadows },
	{ "AA: Destructive Vortex",        &kAaVortex },
	{ "AA: Gift of the Quick Spear",   &kAaQuickSpear },
	{ "AA: Unholy Aura",               &kAaUnholyAura },
	{ "AA: Voice of Thule",            &kAaVoiceThule },
	{ "AA: Mortal Coil",               &kAaMortalCoil },
	{ "AA: Hand of Death",             &kAaHandDeath },
	{ "AA: Dark Lord's Unity",         &kAaDarkLordUnity },
	{ "AA: Death's Effigy",            &kAaDeathsEffigy },
	{ "AA: Unbridled Strike of Fear",  &kAaUnbridledFear },
};

const GroupLine kNumbered[] = {
	{ 0, Touch },       { 1, Bond },        { 3, Bite },        { 4, Recourse },    { 5, Shroud },
	{ 6, Terror },      { 7, Power },       { 9, BloodNuke },   { 10, Minion },     { 11, PetGift },
	{ 12, Covenant },   { 13, Spear },      { 14, Revile },     { 15, Drape },      { 16, LastBreath },
	{ 17, SweetBreath }, { 21, Siphon },    { 22, Horror },     { 23, Carapace },   { 24, Umbral },
	{ 25, Plague },     { 26, LastBreath }, { 27, Curse },      { 29, Mantle },     { 30, Dire },
	{ 31, Constrict },  { 32, Sacrifice },  { 33, RotSkin },    { 34, Darkness },   { 35, Call },
	{ 36, Blight },     { 37, Audacity },   { 38, Blade },      { 39, Acrimony },   { 40, Crimson },
	{ 41, Reflexive },  { 42, Bargain },    { 43, Guardian },   { 44, Mental },     { 45, Insidious },
	{ 46, Torrent },    { 47, ProcHeal },   { 48, Spite },      { 50, Fang },       { 52, Disruption },
	{ 53, Bloodletting },
};

const GroupLine kModern[] = {
	{ 10, Touch },      { 11, GiftProc },   { 20, Bond },       { 30, Bite },       { 31, Recourse },
	{ 40, Shroud },     { 50, Terror },     { 60, Power },      { 70, BloodNuke },  { 80, Minion },
	{ 90, PetGift },    { 100, Covenant },  { 110, Spear },     { 120, Revile },    { 130, Drape },
	{ 150, Siphon },    { 160, Horror },    { 170, Carapace },  { 180, Umbral },    { 190, Plague },
	{ 220, Mantle },    { 230, Dire },      { 240, Constrict }, { 260, RotSkin },   { 270, Darkness },
	{ 280, Call },      { 290, Blight },    { 300, Audacity },  { 310, Blade },     { 320, Acrimony },
	{ 330, Crimson },   { 340, Reflexive }, { 350, Bargain },   { 370, Mental },    { 380, Insidious },
	{ 390, Torrent },   { 400, ProcHeal },  { 420, Fang },      { 430, Disruption }, { 440, Bloodletting },
};

const GroupLine kOther[] = {
	{ 6000, Touch },    { 6002, Bond },     { 6006, Bite },     { 6008, Shroud },   { 6010, Terror },
	{ 6011, Power },    { 6015, BloodNuke }, { 6016, Siphon },  { 6018, PetGift },  { 6019, Covenant },
	{ 6020, Spear },    { 6021, Revile },   { 6024, Drape },    { 6026, LastBreath }, { 6029, Demeanor },
	{ 6031, Siphon },   { 6032, Horror },   { 6033, Carapace }, { 35340, Bite },    { 2405113, Demeanor },
	{ 40166, Spite },   { 55764, Spite },   { 59180, Spite },
	{ 43713, GiftProc }, { 55816, GiftProc }, { 59232, GiftProc }, { 62710, GiftProc },
	{ 1505994, Reaper }, { 1605994, Reaper }, { 1705994, Reaper }, { 1805994, Reaper }, { 1905994, Reaper },
	{ 2005994, Reaper }, { 2105994, Reaper }, { 2205994, Reaper }, { 2305994, Reaper }, { 2405994, Reaper },
	{ 125110600, Reaper },
};

const NamedLine kAbilities[] = {
	{ "harm touch",                  HarmTouch },
	{ "leech touch",                 LeechTouch },
	{ "hate's attraction",           HatesAttraction },
	{ "visage of death",             VisageDeath },
	{ "visage of decay",             VisageDecay },
	{ "veil of darkness",            Veil },
	{ "scourge skin",                ScourgeSkin },
	{ "thought leech",               ThoughtLeech },
	{ "spire of the reavers",        SpireReavers },
	{ "chattering bones",            ChatteringBones },
	{ "death peace",                 DeathPeace },
	{ "cascading theft of defense",  TheftDefense },
	{ "cascading theft of life",     TheftLife },
	{ "vicious bite of chaos",       ViciousBite },
	{ "explosion of spite",          ExplosionSpite },
	{ "explosion of hatred",         ExplosionHatred },
	{ "projection of doom",          ProjectionDoom },
	{ "purity of death",             PurityDeath },
	{ "soul flay",                   SoulFlay },
	{ "encroaching darkness",        Encroaching },
	{ "hate step",                   HateStep },
	{ "stream of hatred",            StreamHatred },
	{ "t`vyl's resolve",             TvylsResolve },
	{ "reaver's bargain",            ReaversBargain },
	{ "bobbing corpse",              BobbingCorpse },
	{ "cloak of shadows",            CloakShadows },
	{ "destructive vortex",          Vortex },
	{ "gift of the quick spear",     QuickSpear },
	{ "unholy aura",                 UnholyAura },
	{ "voice of thule",              VoiceThule },
	{ "mortal coil",                 MortalCoil },
	{ "hand of death",               HandDeath },
	{ "dark lord's unity",         DarkLordUnity },
	{ "death's shadowy effigy",    DeathsEffigy },
	{ "unbridled strike of fear",  UnbridledFear },
};

int BySpell(const eqlib::EQ_Spell*, std::string_view base)
{
	if (EndsWith(base, " recourse"))
		return Recourse;
	if (EndsWith(base, " heal"))
		return ProcHeal;
	return -1;
}

} // namespace

const ClassCatalogue& ShadowKnightCatalogue()
{
	static const ClassCatalogue c{ "Shadow Knight", eqlib::Shadowknight, kLines, 13, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
