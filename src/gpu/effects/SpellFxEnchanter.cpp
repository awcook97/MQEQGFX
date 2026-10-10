#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

#include <cstring>

namespace eqgfx::spellfx {

namespace {

constexpr Pair kMesmer    { { 0.85f, 0.78f, 1.00f, 0.95f }, { 0.40f, 0.22f, 0.85f, 0.80f } };
constexpr Pair kDream     { { 0.78f, 0.90f, 1.00f, 0.90f }, { 1.00f, 0.62f, 0.92f, 0.75f } };
constexpr Pair kCharm     { { 1.00f, 0.60f, 0.85f, 0.95f }, { 0.70f, 0.10f, 0.55f, 0.85f } };
constexpr Pair kDominate  { { 1.00f, 0.85f, 0.45f, 0.95f }, { 0.60f, 0.10f, 0.60f, 0.85f } };
constexpr Pair kSpectrum  { { 1.00f, 0.92f, 0.35f, 0.95f }, { 0.55f, 0.25f, 1.00f, 0.85f } };
constexpr Pair kChromatic { { 0.35f, 1.00f, 0.85f, 0.95f }, { 1.00f, 0.30f, 0.55f, 0.85f } };
constexpr Pair kEncRune   { { 0.75f, 0.95f, 1.00f, 0.95f }, { 0.55f, 0.35f, 1.00f, 0.85f } };
constexpr Pair kQuick     { { 1.00f, 0.98f, 0.60f, 0.95f }, { 0.30f, 0.95f, 0.80f, 0.85f } };
constexpr Pair kTashan    { { 1.00f, 0.72f, 0.40f, 0.95f }, { 0.75f, 0.20f, 0.55f, 0.85f } };
constexpr Pair kBrimstone { { 1.00f, 0.78f, 0.40f, 0.95f }, { 0.45f, 0.36f, 0.30f, 0.90f } };
constexpr Pair kChoke     { { 0.80f, 0.95f, 0.92f, 0.85f }, { 0.30f, 0.35f, 0.60f, 0.70f } };
constexpr Pair kPhantasm  { { 0.85f, 0.92f, 1.00f, 0.60f }, { 0.60f, 0.40f, 0.95f, 0.30f } };
constexpr Pair kVoid      { { 0.55f, 0.35f, 0.95f, 0.90f }, { 0.06f, 0.02f, 0.18f, 0.85f } };
constexpr Pair kTwilight  { { 0.60f, 0.55f, 0.90f, 0.90f }, { 0.14f, 0.10f, 0.32f, 0.85f } };
constexpr Pair kHorror    { { 0.75f, 1.00f, 0.55f, 0.90f }, { 0.35f, 0.05f, 0.40f, 0.85f } };
constexpr Pair kMindLight { { 1.00f, 1.00f, 1.00f, 1.00f }, { 0.80f, 0.60f, 1.00f, 0.90f } };
constexpr Pair kLull      { { 0.85f, 0.95f, 1.00f, 0.85f }, { 0.55f, 0.70f, 0.95f, 0.60f } };
constexpr Pair kGravity   { { 0.80f, 0.65f, 1.00f, 0.95f }, { 0.20f, 0.08f, 0.45f, 0.85f } };
constexpr Pair kAnimBlade { { 1.00f, 1.00f, 1.00f, 1.00f }, { 0.75f, 0.55f, 1.00f, 0.90f } };

constexpr uint32_t GA_Emerge = GlyphAnim_Rise | GlyphAnim_Pop;
constexpr uint32_t GA_Blades = GlyphAnim_Rise | GlyphAnim_Spin | GlyphAnim_Roll | GlyphAnim_Pop;

LOOK(kEncMez, "mesmerize",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.8f, 0.4f, 1.0f, FIX, kMesmer, 0.9f, 0.10f),
	G(IM, Glyph_Spiral, 0.9f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kMesmer, 3.0f),
	G(IM, Glyph_Z, 0.35f, 7.0f, 0, 3, GA_PopAscend, 0.8f, FIX, kDream, 3.0f, 0.4f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kDream, 3.0f));

LOOK(kEncDocility, "docility",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.9f, 0.4f, 1.2f, FIX, kMesmer, 0.9f, 0.12f),
	F(IM, FxForm_Vortex, 1.4f, 6.0f, 96, 5, 1.2f, 2.5f, 3.0f, FIX, kMesmer, 3.0f, 0.05f),
	G(IM, Glyph_Z, 0.4f, 6.5f, 0, 5, GA_Orbit, 1.6f, FIX, kDream, 3.0f),
	F(IM, FxForm_Bloom, 1.0f, 2.0f, 96, 3, 0, 0, 0, FIX, kDream, 2.4f, 0.05f, ADD, 1, 0.2f, AE));

LOOK(kEncWave, "mesmerizing wave",
	G(IM, Glyph_Wave, 0.8f, 1.2f, 3, 6, GA_PopSpin, 2.4f, FIX, kDream, 2.8f),
	F(IM, FxForm_Bloom, 1.0f, 2.0f, 120, 4, 0, 0, 0, FIX, kDream, 2.8f, 0.07f, ADD, 1.2f, 0, AE),
	F(IM, FxForm_Ring, 1.0f, 1.0f, 96, 0, 0.8f, 1.0f, 0.4f, FIX, kMesmer, 2.4f, 0.05f, COMET, 1, 0.2f, AE),
	G(IM, Glyph_Z, 0.35f, 6.5f, 0, 4, GA_PopAscend, 1.0f, FIX, kMesmer, 2.6f, 0.4f));

LOOK(kEncFlash, "flash",
	F(IM, FxForm_Burst, 2.2f, 3.0f, 72, 1, 0.5f, 24, 0, FIX, kMindLight, 0.5f, 0.10f),
	F(IM, FxForm_Spire, 0.4f, 6.0f, 64, 16, 0.5f, 4.0f, 0, FIX, kMindLight, 0.6f, 0.05f),
	G(IM, Glyph_Eye, 0.8f, 7.4f, 4, 1, GlyphAnim_Pop, 0, FIX, kWhite, 0.8f),
	G(IM, Glyph_Z, 0.35f, 7.0f, 0, 3, GA_PopAscend, 0.8f, FIX, kMesmer, 2.6f, 0.4f));

LOOK(kEncStare, "hypnotic stare",
	G(IM, Glyph_Eye, 1.6f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kMesmer, 3.0f),
	F(IM, FxForm_Rune, 1.0f, 0, 96, 7, 3, 21, 0, FIX, kMesmer, 3.0f, 0.06f, ADD, 1, 0, AE),
	G(IM, Glyph_Z, 0.4f, 5.0f, 0, 6, GA_Orbit, 2.4f, FIX, kDream, 3.0f, 0.5f));

LOOK(kEncChaotic, "chaotic",
	G(IM, Glyph_Spiral, 0.8f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kPrism, 3.0f),
	F(IM, FxForm_Vortex, 1.1f, 5.0f, 64, 4, 1.2f, 2.0f, -4.0f, FIX, kPrism, 2.6f, 0.05f),
	F(RE, FxForm_Rune, 1.4f, 3.2f, 60, 6, 1, 12, 2, FIX, kMesmer, 2.2f, 0.05f),
	F(RE, FxForm_Rune, 1.4f, 3.2f, 60, 6, 1, 12, 2, FIX, kPrism, 2.2f, 0.05f, ADD, -1.0f));

LOOK(kEncPerilous, "perilous",
	F(RE, FxForm_Vortex, 1.0f, 4.0f, 128, 8, 0.6f, 2.0f, 2.0f, FIX, kMesmer, 2.6f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Spiral, 1.2f, 0.0f, 2, 1, GA_PopRoll, 0, FIX, kDream, 2.8f),
	G(RE, Glyph_Z, 0.5f, 5.0f, 0, 8, GA_RingRise, 2.8f, FIX, kMesmer, 3.0f, 0.4f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kDream, 2.4f, 0.06f, ADD, 1, 0.3f, AE));

LOOK(kEncGlance, "glance",
	G(IM, Glyph_Eye, 0.5f, 6.0f, 3, 5, GA_RingRise, 2.2f, FIX, kLull, 2.4f),
	F(IM, FxForm_Burst, 1.0f, 2.0f, 96, 1, 0.3f, 16, 0, FIX, kLull, 0.8f, 0.07f, ADD, 1, 0, AE),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 40, 20, 0.12f, 1, 0, FIX, kWhite, 2.0f));

LOOK(kEncWard, "ward",
	Persist(G(AU, Glyph_Z, 0.25f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kMesmer, 0)),
	Persist(F(AU, FxForm_Ring, 1.1f, 0.3f, 48, 0, 0.6f, 1.0f, 0.3f, FIX, kMesmer, 0, 0.04f, COMET)),
	F(IM, FxForm_Rune, 1.6f, 0, 70, 6, 1, 12, 0, FIX, kMesmer, 2.4f, 0.05f));

LOOK(kEncWardProc, "ward trigger",
	F(IM, FxForm_Burst, 1.4f, 4.0f, 48, 1, 0.3f, 12, 0, FIX, kMindLight, 0.5f, 0.06f),
	G(IM, Glyph_Z, 0.3f, 7.0f, 0, 2, GA_PopAscend, 0.6f, FIX, kMesmer, 1.8f));

LOOK(kEncMemory, "forget",
	G(IM, Glyph_Spiral, 0.7f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kGhost, 2.4f),
	F(IM, FxForm_Bloom, 1.6f, 5.0f, 48, 2, 0, 0, 0, FIX, kGhost, 2.0f, 0.05f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Sparkle, 1.0f, 8.0f, 32, 16, 0.10f, 0, 0, FIX, kLull, 2.4f));

LOOK(kEncCharm, "charm",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.6f, 0.4f, 1.5f, FIX, kCharm, 0.9f, 0.12f),
	G(IM, Glyph_Heart, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kCharm, 3.0f),
	G(IM, Glyph_Heart, 0.25f, 5.0f, 0, 4, GA_Orbit, 1.2f, FIX, kPink, 3.0f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 64, 2, 1.5f, 0, 0.5f, FIX, kCharm, 3.0f, 0.05f, COMET));

LOOK(kEncCommand, "command",
	G(IM, Glyph_Crown, 0.9f, 8.0f, 4, 1, GA_Drop, 0, FIX, kDominate, 3.0f),
	F(IM, FxForm_Lightning, 1, 14.0f, 48, 4, 0.05f, 0.5f, 1.0f, FIX, kDominate, 2.4f, 0.03f),
	F(IM, FxForm_Ring, 1.1f, 7.4f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kDominate, 3.0f, 0.05f, COMET),
	G(IM, Glyph_Heart, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kCharm, 3.0f));

LOOK(kEncDemand, "demand",
	G(IM, Glyph_Doll, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kCharm, 2.6f),
	G(IM, Glyph_Link, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.2f, FIX, kDominate, 2.6f),
	F(IM, FxForm_Wind, 1.2f, 5.0f, 48, 10, 2.5f, 0, 0, FIX, kQuick, 1.6f));

LOOK(kEncCharmEcho, "echo of subjugation",
	G(IM, Glyph_Heart, 0.8f, 0.0f, 2, 1, GA_PopRoll, 0, FIX, kCharm, 3.0f),
	F(IM, FxForm_Bloom, 1.0f, 1.0f, 96, 4, 0, 0, 0, FIX, kCharm, 3.0f, 0.05f, ADD, 0.8f, 0, AE),
	G(IM, Glyph_Clef, 0.35f, 4.0f, 0, 4, GA_Orbit, 1.8f, FIX, kCharm, 3.0f));

LOOK(kEncColor, "color",
	F(RE, FxForm_Burst, 1.0f, 1.0f, 120, 3, 1.5f, 24, 0, FIX, kPrism, 1.2f, 0.12f, ADD, 1, 0, AE),
	F(RE, FxForm_Burst, 0.8f, 1.5f, 96, 2, 1.0f, 16, 0, FIX, kSpectrum, 1.0f, 0.10f, ADD, 1, 0.15f, AE),
	F(RE, FxForm_Spire, 0.5f, 5.0f, 96, 24, 0.4f, 5.0f, 0, FIX, kChromatic, 0.8f, 0.07f),
	Stars(0.2f));

LOOK(kEncRemoteColor, "remote color",
	F(IM, FxForm_Lightning, 1, 36.0f, 64, 3, 0.6f, 10.0f, 1.0f, FIX, kPrism, 0.7f, 0.12f),
	F(IM, FxForm_Burst, 1.0f, 1.0f, 120, 3, 1.5f, 24, 0, FIX, kPrism, 1.2f, 0.12f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kSpectrum, 1.6f, 0.06f, ADD, 1.5f, 0.2f, AE),
	Stars(0.3f));

LOOK(kEncSpin, "dizzying",
	F(IM, FxForm_Vortex, 1.2f, 6.0f, 96, 6, 1.0f, 3.0f, 8.0f, FIX, kVoid, 3.0f, 0.06f, OVER),
	G(IM, Glyph_Spiral, 0.9f, 0.0f, 2, 1, GA_PopRoll, 0, FIX, kMesmer, 3.0f),
	F(IM, FxForm_Wind, 1.4f, 5.0f, 60, 12, 2.0f, 0, 0, FIX, kPhantasm, 2.4f),
	Stars(0.4f));

LOOK(kEncPolyRune, "polychromatic rune",
	Persist(F(AU, FxForm_Orbit, 1.3f, 6.0f, 48, 12, 0, 0, 0, FIX, kPrism, 0, 0.04f, ADD, 2.0f)),
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kPrism, 2.4f, 0.04f),
	G(IM, Glyph_Gem, 0.35f, 4.0f, 0, 6, GA_Orbit, 1.4f, FIX, kSpectrum, 2.6f));

LOOK(kEncRuneStrike, "rune strike",
	F(IM, FxForm_Burst, 1.0f, 2.0f, 120, 2, 1.0f, 32, 0, FIX, kPrism, 0.9f, 0.10f, ADD, 1, 0, AE),
	F(IM, FxForm_Crystal, 1.4f, 5.0f, 60, 12, 0, 1.2f, 0, FIX, kPrism, 1.2f, 0.05f),
	Stars(0.2f));

LOOK(kEncGravity, "gravity",
	F(IM, FxForm_Sparkle, 1.0f, 10.0f, 80, 40, 0.14f, 0, 0, FIX, kGravity, 2.0f, 0.06f, ADD, 1, 0, AE),
	F(IM, FxForm_Burst, 1.0f, 4.0f, 96, 2, 3.0f, 16, 0, FIX, kGravity, 1.0f, 0.10f, ADD, 1, 0, AE),
	G(IM, Glyph_ArrowUp, 0.5f, 3.0f, 4, 4, GA_PopAscend, 1.6f, FIX, kGravity, 1.6f),
	F(IM, FxForm_Cracks, 2.2f, 0, 48, 6, 1.2f, 0, 0, FIX, kVoid, 2.0f, 0.06f, OVER));

LOOK(kEncLurch, "lurch",
	F(IM, FxForm_Drain, 1, 0, 48, 3, 0.6f, 12, 0, FIX, kVoid, 1.8f, 0.08f, OVER),
	G(IM, Glyph_Hand, 0.8f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kGravity, 2.0f),
	G(IM, Glyph_Link, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.0f, FIX, kGravity, 2.0f));

LOOK(kEncAssault, "assault",
	F(TR, FxForm_Bolt, 1, 0, 128, 5, 0.9f, 0.45f, 1.2f, FIX, kPrism, 0.8f, 0.20f),
	F(IM, FxForm_Burst, 2.4f, 2.5f, 96, 3, 1.5f, 20, 0, FIX, kPrism, 0.9f, 0.12f),
	F(IM, FxForm_Burst, 2.0f, 3.0f, 64, 2, 1.0f, 12, 0, FIX, kSpectrum, 0.8f, 0.10f, ADD, 1, 0.2f),
	F(IM, FxForm_Sparkle, 1.4f, 7.0f, 48, 24, 0.12f, 0, 0, FIX, kChromatic, 1.6f));

LOOK(kEncChroma, "chroma",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.35f, 0.8f, FIX, kChromatic, 0.7f, 0.14f),
	F(IM, FxForm_Flame, 0.7f, 7.5f, 80, 20, 0.85f, 0.6f, 0, FIX, kChromatic, 1.4f, 0.07f),
	G(IM, Glyph_Flame, 0.6f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kPrism, 1.4f),
	F(IM, FxForm_Burst, 1.6f, 7.0f, 48, 1, 0.5f, 12, 0, FIX, kPrism, 0.7f, 0.08f));

LOOK(kEncJab, "chromatic jab",
	F(TR, FxForm_Bolt, 1, 0, 64, 1, 0.05f, 1.0f, 0.2f, FIX, kChromatic, 0.5f, 0.10f),
	F(IM, FxForm_Spire, 0.3f, 5.0f, 48, 12, 0.8f, 5.0f, 0, FIX, kChromatic, 0.6f, 0.05f),
	G(IM, Glyph_Gem, 0.4f, 6.5f, 4, 1, GA_PopAscend, 0, FIX, kPrism, 1.0f));

LOOK(kEncMindblade, "mindblade",
	G(IM, Glyph_Sword, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kMindLight, 1.2f),
	F(IM, FxForm_Lightning, 1, 30.0f, 48, 1, 0.02f, 1.0f, 1.0f, FIX, kMindLight, 0.6f, 0.20f, ADD, 1, 0.3f),
	F(IM, FxForm_Burst, 2.0f, 6.0f, 64, 2, 0.5f, 16, 0, FIX, kPrism, 0.8f, 0.10f, ADD, 1, 0.4f));

LOOK(kEncAppropriation, "appropriation",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.5f, 0.4f, 1.0f, FIX, kChromatic, 0.7f, 0.14f),
	F(IM, FxForm_Burst, 2.2f, 5.0f, 64, 2, 1.0f, 16, 0, FIX, kChromatic, 0.8f, 0.10f),
	F(IM, FxForm_Drain, 1, 0, 48, 3, 1.2f, 12, 0, FIX, kMana, 1.8f, 0.07f, ADD, 1, 0.3f),
	G(RE, Glyph_Drop, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.0f, FIX, kMana, 2.0f, 0.8f));

LOOK(kEncTears, "tears",
	F(IM, FxForm_Drain, 1, 0, 48, 3, 1.4f, 14, 0, FIX, kMana, 1.8f, 0.08f),
	G(IM, Glyph_Drop, 0.3f, 7.5f, 4, 5, GA_Rain, 0.8f, FIX, kTear, 2.0f),
	F(IM, FxForm_Burst, 1.8f, 5.0f, 64, 2, 0.5f, 12, 0, FIX, kMana, 0.8f, 0.08f));

LOOK(kEncTheft, "theft of thought",
	F(IM, FxForm_Drain, 1, 0, 48, 4, 0.8f, 16, 0, FIX, kMana, 2.0f, 0.07f),
	G(IM, Glyph_Eye, 0.6f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kMana, 1.8f),
	F(RE, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.10f, 2, 0, FIX, kMana, 2.0f, 0.06f, ADD, 1, 0.5f));

LOOK(kEncStrangle, "strangle",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.3f, 0.4f, 0.8f, FIX, kChoke, 0.8f, 0.10f),
	F(IM, FxForm_Ring, 0.6f, 5.6f, 40, 0, 2.0f, 1.0f, 0.8f, FIX, kChoke, 3.0f, 0.06f, COMET),
	F(IM, FxForm_Ring, 0.45f, 5.4f, 40, 0, -2.0f, 1.0f, 0.8f, FIX, kChoke, 3.0f, 0.05f, COMET, 1, 0.2f),
	F(IM, FxForm_Vortex, 0.9f, 6.0f, 64, 4, 0.8f, 2.0f, -3.0f, FIX, kAir, 2.4f, 0.04f),
	G(IM, Glyph_Mouth, 0.4f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kChoke, 2.4f));

LOOK(kEncConstriction, "constriction",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.5f, 0.4f, 1.0f, FIX, kVoid, 0.8f, 0.12f, OVER),
	G(IM, Glyph_Link, 0.3f, 5.5f, 0, 5, GA_Orbit, 0.9f, FIX, kVoid, 3.0f),
	F(IM, FxForm_Helix, 0.8f, 6.0f, 64, 3, 2.0f, 0, 0.6f, FIX, kChoke, 3.0f, 0.05f, COMET),
	G(IM, Glyph_ArrowDown, 0.35f, 7.5f, 4, 1, GA_Drop, 0, FIX, kChoke, 2.0f));

LOOK(kEncMindDot, "mind vortex",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.8f, 0.4f, 1.2f, FIX, kVoid, 0.8f, 0.14f, OVER),
	F(IM, FxForm_Vortex, 1.0f, 8.0f, 96, 5, 2.0f, 3.0f, 6.0f, FIX, kVoid, 3.0f, 0.05f, OVER),
	G(IM, Glyph_Spiral, 0.6f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kMesmer, 3.0f),
	F(IM, FxForm_Drain, 1, 0, 32, 2, 1.0f, 10, 0, FIX, kMana, 2.0f, 0.06f, ADD, 1, 0.5f));

LOOK(kEncCripple, "consciousness",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.6f, 0.4f, 1.0f, FIX, kTwilight, 0.8f, 0.12f),
	G(IM, Glyph_Shield, 0.8f, 7.0f, 4, 1, GA_Drop, 0, FIX, kTwilight, 2.4f),
	G(IM, Glyph_ArrowDown, 0.3f, 6.0f, 0, 4, GA_Orbit, 1.2f, FIX, kTwilight, 2.6f),
	F(IM, FxForm_Crystal, 1.0f, 5.0f, 48, 10, 0, 0.8f, 0, FIX, kTwilight, 1.6f, 0.05f, OVER));

LOOK(kEncTashan, "tashan",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.5f, 0.4f, 1.0f, FIX, kTashan, 0.8f, 0.12f),
	G(IM, Glyph_Wolf, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kTashan, 2.6f),
	F(IM, FxForm_Bloom, 1.2f, 6.0f, 48, 3, 0, 0, 0, FIX, kTashan, 2.0f, 0.05f, ADD, 1.5f),
	G(IM, Glyph_ArrowDown, 0.3f, 5.5f, 0, 3, GA_Orbit, 1.2f, FIX, kTashan, 2.6f));

LOOK(kEncHelix, "helix",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.8f, 0.4f, 1.0f, FIX, kTwilight, 0.8f, 0.12f),
	G(IM, Glyph_Hourglass, 0.8f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kMesmer, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 96, 4, 0.6f, 0, 0.3f, FIX, kMesmer, 3.0f, 0.05f, COMET | FxFlag_Reverse),
	G(IM, Glyph_Link, 0.3f, 3.0f, 0, 3, GA_Orbit, 1.0f, FIX, kTwilight, 3.0f));

LOOK(kEncDeeds, "deeds",
	G(IM, Glyph_Hourglass, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kLull, 3.0f),
	F(IM, FxForm_Sparkle, 1.1f, 7.0f, 40, 20, 0.10f, 1, 0, FIX, kLull, 3.0f),
	F(IM, FxForm_Ring, 1.0f, 4.0f, 40, 0, 0.3f, 1.0f, 0.2f, FIX, kLull, 3.0f, 0.05f, COMET));

LOOK(kEncCovenant, "chromatic covenant",
	G(IM, Glyph_Gem, 0.35f, 5.0f, 0, 5, GA_Orbit, 1.3f, FIX, kPrism, 3.5f),
	F(IM, FxForm_Rune, 1.4f, 7.8f, 70, 5, 2, 10, 1, FIX, kChromatic, 3.5f, 0.05f),
	F(IM, FxForm_Lattice, 1.2f, 6.0f, 96, 6, 0.5f, 0, 0, FIX, kPrism, 3.0f, 0.04f));

LOOK(kEncFetter, "fetter",
	G(IM, Glyph_Shackle, 0.8f, 1.5f, 4, 1, GA_Emerge, 0, FIX, kStone, 3.0f, 0, OVER),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 3.0f, 0.06f, OVER),
	F(IM, FxForm_Ring, 0.9f, 0.4f, 40, 0, 0.5f, 1.0f, 0.3f, FIX, kMesmer, 3.0f, 0.05f, COMET));

LOOK(kEncFear, "terror",
	G(IM, Glyph_Skull, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kHorror, 2.4f),
	G(IM, Glyph_Mouth, 0.4f, 6.0f, 0, 3, GA_Orbit, 1.3f, FIX, kHorror, 2.4f),
	F(IM, FxForm_Bloom, 1.4f, 5.0f, 48, 3, 0, 0, 0, FIX, kHorror, 2.0f, 0.05f, OVER, 1.5f));

LOOK(kEncDispel, "strip enchantment",
	F(IM, FxForm_Burst, 1.8f, 3.0f, 64, 2, 1.0f, 16, 0, FIX, kMagic, 0.8f, 0.08f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kMagic, 1.8f),
	G(IM, Glyph_Gem, 0.5f, 6.5f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 1.6f));

LOOK(kEncNightTerror, "night's terror",
	G(IM, Glyph_Skull, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kHorror, 3.0f),
	G(IM, Glyph_Bat, 0.3f, 5.5f, 0, 4, GA_Orbit, 1.4f, FIX, kTwilight, 3.0f),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.9f, 0.7f, 0, FIX, kHorror, 2.4f, 0.05f, OVER));

LOOK(kEncSelfRune, "rune",
	F(IM, FxForm_Rune, 1.5f, 3.4f, 70, 6, 1, 16, 2, FIX, kEncRune, 2.6f, 0.05f),
	F(IM, FxForm_Rune, 1.8f, 0, 70, 8, 3, 24, 0, FIX, kEncRune, 2.6f, 0.05f),
	Persist(F(AU, FxForm_Ring, 1.15f, 3.0f, 48, 0, 0.8f, 1.0f, 0.4f, FIX, kEncRune, 0, 0.04f, COMET)),
	G(IM, Glyph_Gem, 0.5f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kEncRune, 2.0f));

LOOK(kEncRuneOf, "rune of",
	F(TR, FxForm_Bolt, 1, 0, 96, 2, 0.3f, 0.5f, 0.8f, FIX, kEncRune, 0.8f, 0.10f),
	F(IM, FxForm_Rune, 1.4f, 0, 60, 5, 2, 10, 0, FIX, kEncRune, 2.6f, 0.05f),
	F(IM, FxForm_Dome, 1.5f, 6.4f, 80, 6, 2, 0, 0, FIX, kEncRune, 2.2f, 0.04f),
	G(IM, Glyph_Gem, 0.3f, 4.0f, 3, 4, GA_RingRise, 1.4f, FIX, kEncRune, 2.4f));

LOOK(kEncGroupRune, "group rune",
	F(RE, FxForm_Rune, 1.0f, 0, 96, 6, 1, 18, 0, FIX, kTwilight, 3.0f, 0.06f, ADD, 1, 0, AE),
	F(IM, FxForm_Dome, 1.6f, 6.4f, 88, 6, 2, 0, 0, FIX, kTwilight, 2.4f, 0.04f),
	G(IM, Glyph_Moon, 0.5f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kTwilight, 2.4f));

LOOK(kEncShadows, "obscuring shadows",
	F(IM, FxForm_Flame, 1.0f, 5.5f, 64, 16, 0.9f, 0.6f, 0, FIX, kTwilight, 1.6f, 0.05f, OVER),
	F(IM, FxForm_Bloom, 1.4f, 1.0f, 40, 2, 0, 0, 0, FIX, kTwilight, 1.4f, 0.04f, OVER | FxFlag_Reverse));

LOOK(kEncShield, "shielding",
	F(IM, FxForm_Lattice, 1.5f, 6.0f, 96, 6, 0.4f, 0, 0, FIX, kMagic, 2.4f, 0.04f),
	G(IM, Glyph_Shield, 0.8f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kMagic, 2.4f),
	F(IM, FxForm_Ring, 1.6f, 0.3f, 48, 0, 1.2f, 1.0f, 0.5f, FIX, kMagic, 2.4f, 0.05f, COMET));

LOOK(kEncBrimstone, "brimstone",
	F(IM, FxForm_Crystal, 1.0f, 5.5f, 60, 12, 0, 0.6f, 0, FIX, kBrimstone, 2.6f, 0.06f, OVER),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 64, 16, 0.2f, 0.6f, 0, FIX, kAmber, 2.0f, 0.05f),
	G(IM, Glyph_Rock, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kBrimstone, 2.4f, 0, OVER));

LOOK(kEncAegis, "aegis",
	F(TR, FxForm_Bolt, 1, 0, 96, 2, 0.3f, 0.5f, 0.8f, FIX, kRoyal, 0.8f, 0.10f),
	G(IM, Glyph_Shield, 0.9f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kRoyal, 2.6f),
	F(IM, FxForm_Lattice, 1.4f, 6.5f, 96, 8, 0.5f, 0, 0, FIX, kRoyal, 2.6f, 0.04f));

LOOK(kEncLegion, "legion",
	F(RE, FxForm_Lattice, 0.6f, 7.0f, 120, 12, 0.3f, 0, 0, FIX, kRoyal, 3.0f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Shield, 0.5f, 5.0f, 3, 6, GA_RingRise, 2.6f, FIX, kRoyal, 3.0f),
	F(IM, FxForm_Ring, 1.3f, 6.0f, 48, 0, 1.0f, 1.0f, 0.5f, FIX, kRoyal, 2.4f, 0.05f, COMET));

LOOK(kEncDotAegis, "aegis of the keeper",
	G(IM, Glyph_Shield, 0.8f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kAurora, 2.6f),
	F(IM, FxForm_Bubbles, 1.1f, 6.0f, 48, 10, 0.2f, 0, 0, FIX, kAurora, 2.6f, 0.04f),
	F(IM, FxForm_Dome, 1.6f, 6.4f, 80, 6, 2, 0, 0, FIX, kAurora, 2.2f, 0.04f));

LOOK(kEncDotLegion, "legion of the keeper",
	F(RE, FxForm_Bloom, 1.0f, 2.0f, 96, 3, 0, 0, 0, FIX, kAurora, 2.8f, 0.06f, ADD, 0.8f, 0, AE),
	G(RE, Glyph_Shield, 0.45f, 3.5f, 3, 5, GA_RingRise, 2.2f, FIX, kAurora, 3.0f),
	F(IM, FxForm_Bubbles, 1.1f, 6.0f, 40, 8, 0.2f, 0, 0, FIX, kAurora, 2.4f, 0.04f));

LOOK(kEncAuspice, "auspice",
	G(IM, Glyph_Shield, 0.8f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kSilver, 2.6f),
	G(IM, Glyph_Sword, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kSilver, 2.6f),
	F(IM, FxForm_Ring, 1.2f, 3.0f, 48, 0, 1.0f, 1.0f, 0.5f, FIX, kTwilight, 2.6f, 0.05f, COMET));

LOOK(kEncGroupAuspice, "group auspice",
	F(RE, FxForm_Rune, 1.0f, 0, 96, 5, 2, 15, 0, FIX, kRoyal, 3.0f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Hand, 0.5f, 3.5f, 3, 5, GA_RingRise, 2.2f, FIX, kRoyal, 3.0f),
	F(IM, FxForm_Dome, 1.6f, 6.4f, 80, 6, 2, 0, 0, FIX, kAurora, 2.2f, 0.04f));

LOOK(kEncHateRune, "horrifying rune",
	G(IM, Glyph_Mask, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kHorror, 2.6f),
	F(IM, FxForm_Rune, 1.4f, 3.4f, 60, 5, 2, 10, 1, FIX, kHorror, 2.6f, 0.05f),
	F(IM, FxForm_Dome, 1.6f, 6.4f, 80, 6, 2, 0, 0, FIX, kHorror, 2.2f, 0.04f, OVER));

LOOK(kEncHateRuneProc, "horrifying rune effect",
	G(IM, Glyph_Mask, 0.5f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kHorror, 1.4f),
	F(IM, FxForm_Burst, 1.4f, 4.0f, 48, 1, 0.5f, 10, 0, FIX, kHorror, 0.6f, 0.06f));

LOOK(kEncShieldFate, "shield of fate",
	G(IM, Glyph_Scales, 0.8f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kSpectrum, 2.6f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kSpectrum, 2.6f, 0.04f),
	F(IM, FxForm_Ring, 1.4f, 3.5f, 48, 0, 1.0f, 1.0f, 0.5f, FIX, kMesmer, 2.6f, 0.05f, COMET));

LOOK(kEncMirror, "reflect",
	G(IM, Glyph_Mask, 0.9f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kSilver, 2.2f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0, 12, 0, FIX, kPrism, 1.0f, 0.06f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Spire, 1.0f, 3.5f, 48, 12, 0.5f, 1.6f, 0, FIX, kSilver, 1.6f, 0.04f));

LOOK(kEncHaste, "speed",
	F(TR, FxForm_Bolt, 1, 0, 64, 2, 0.2f, 0.6f, 0.4f, FIX, kQuick, 0.5f, 0.08f),
	F(IM, FxForm_Helix, 1.0f, 6.5f, 72, 3, 6.0f, 0, 1.5f, FIX, kQuick, 2.0f, 0.05f, COMET),
	G(IM, Glyph_Lightning, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kQuick, 1.8f),
	F(IM, FxForm_Wind, 1.2f, 5.5f, 48, 12, 2.5f, 0, 0, FIX, kQuick, 1.8f));

LOOK(kEncGroupHaste, "hastening",
	F(RE, FxForm_Burst, 1.0f, 1.0f, 96, 2, 2.0f, 20, 0, FIX, kQuick, 1.2f, 0.09f, ADD, 1, 0, AE),
	G(RE, Glyph_Lightning, 0.5f, 2.5f, 3, 6, GA_RingRise, 2.4f, FIX, kQuick, 2.4f),
	F(IM, FxForm_Helix, 1.0f, 6.5f, 72, 3, 6.0f, 0, 1.5f, FIX, kQuick, 2.0f, 0.05f, COMET),
	F(IM, FxForm_Wind, 1.2f, 5.5f, 40, 10, 2.5f, 0, 0, FIX, kSpectrum, 1.6f));

LOOK(kEncClarity, "clarity",
	G(IM, Glyph_Book, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kMana, 3.0f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 48, 24, 0.12f, 0, 0, FIX, kMana, 3.0f),
	G(IM, Glyph_Drop, 0.25f, 4.0f, 0, 4, GA_Orbit | GlyphAnim_Ascend, 1.2f, FIX, kMana, 3.0f));

LOOK(kEncVoice, "voice",
	F(RE, FxForm_Rune, 1.0f, 0, 96, 6, 1, 18, 0, FIX, kMana, 3.0f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Book, 0.5f, 3.0f, 3, 5, GA_RingRise, 2.2f, FIX, kMana, 3.0f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 0, 0, FIX, kMana, 2.6f));

LOOK(kEncLull, "calm",
	F(TR, FxForm_Bolt, 1, 0, 64, 2, 0.4f, 0.3f, 0.6f, FIX, kLull, 0.9f, 0.08f),
	G(IM, Glyph_Dove, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kLull, 3.0f),
	F(IM, FxForm_Bloom, 1.4f, 4.0f, 48, 2, 0, 0, 0, FIX, kLull, 2.4f, 0.04f, ADD, 0.6f),
	F(IM, FxForm_Sparkle, 1.0f, 7.0f, 24, 12, 0.10f, 1, 0, FIX, kWhite, 2.4f));

LOOK(kEncIssuance, "issuance",
	G(IM, Glyph_Dove, 1.0f, 0.0f, 2, 1, GlyphAnim_Pop, 0, FIX, kLull, 3.5f),
	F(IM, FxForm_Bloom, 1.0f, 1.0f, 96, 4, 0, 0, 0, FIX, kLull, 3.5f, 0.05f, ADD, 0.6f, 0, AE),
	G(IM, Glyph_Note, 0.3f, 4.0f, 0, 5, GA_Orbit | GlyphAnim_Ascend, 1.8f, FIX, kLull, 3.0f));

LOOK(kEncManaFlare, "mana flare",
	G(IM, Glyph_Sun, 0.7f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kMana, 2.6f),
	F(IM, FxForm_Burst, 1.6f, 5.0f, 64, 2, 1.0f, 16, 0, FIX, kSpectrum, 0.8f, 0.08f),
	F(IM, FxForm_Orbit, 1.2f, 6.0f, 40, 8, 0, 0, 0, FIX, kMana, 2.6f, 0.04f, ADD, 2.0f));

LOOK(kEncManastorm, "manastorm",
	F(RE, FxForm_Vortex, 1.0f, 6.0f, 128, 8, 0.8f, 2.5f, 4.0f, FIX, kMana, 3.0f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Lightning, 0.4f, 5.0f, 0, 6, GA_Orbit, 2.2f, FIX, kMana, 3.0f),
	F(RE, FxForm_Rain, 0.8f, 12.0f, 60, 40, 0, 0.2f, 0, FIX, kSpectrum, 2.4f, 0.05f, ADD, 1, 0.2f, AE));

LOOK(kEncManaIssuance, "issuance of mana",
	F(IM, FxForm_Bloom, 1.0f, 1.0f, 96, 4, 0, 0, 0, FIX, kSpectrum, 3.5f, 0.05f, ADD, 0.8f, 0, AE),
	G(IM, Glyph_Gem, 0.35f, 3.0f, 3, 6, GA_RingRise, 2.0f, FIX, kMana, 3.0f),
	G(IM, Glyph_Sun, 0.8f, 0.0f, 2, 1, GA_PopRoll, 0, FIX, kMana, 3.5f));

LOOK(kEncEcho, "echo",
	F(IM, FxForm_Rune, 1.0f, 0, 96, 6, 1, 18, 0, FIX, kMana, 3.5f, 0.05f, ADD, 1, 0, AE),
	G(IM, Glyph_Clef, 0.4f, 4.0f, 0, 4, GA_Orbit | GlyphAnim_Ascend, 1.8f, FIX, kMana, 3.0f),
	F(IM, FxForm_Bloom, 1.0f, 1.0f, 64, 3, 0, 0, 0, FIX, kQuick, 3.0f, 0.04f, ADD, 0.6f, 0, AE));

LOOK(kEncUnity, "unity",
	F(IM, FxForm_Rune, 1.5f, 3.4f, 70, 6, 1, 16, 2, FIX, kEncRune, 2.6f, 0.05f),
	F(IM, FxForm_Rune, 1.5f, 3.4f, 70, 6, 1, 16, 2, FIX, kMagic, 2.6f, 0.05f, ADD, -1.0f),
	G(IM, Glyph_Shield, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kMagic, 2.6f),
	G(IM, Glyph_Gem, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kEncRune, 2.6f, 0.2f));

LOOK(kEncReinforcement, "reinforcement",
	F(RE, FxForm_Burst, 1.0f, 1.0f, 96, 2, 1.5f, 16, 0, FIX, kSpectrum, 1.4f, 0.08f, ADD, 1, 0, AE),
	G(RE, Glyph_Gem, 0.4f, 3.0f, 3, 4, GA_RingRise, 2.0f, FIX, kSpectrum, 2.6f),
	G(RE, Glyph_Drop, 0.4f, 3.0f, 3, 4, GA_RingRise, 2.0f, FIX, kMana, 2.6f, 0.3f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kSpectrum, 2.0f));

LOOK(kEncVisage, "visage",
	G(IM, Glyph_Mask, 0.9f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kIllusion, 2.6f),
	F(IM, FxForm_Bloom, 1.4f, 5.0f, 48, 2, 0, 0, 0, FIX, kIllusion, 2.0f, 0.04f, ADD, 0.8f),
	F(IM, FxForm_Sparkle, 1.1f, 7.0f, 32, 16, 0.10f, 0, 0, FIX, kPink, 2.4f));

LOOK(kEncIllusion, "illusion",
	G(IM, Glyph_Moon, 0.9f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kIllusion, 3.0f),
	F(IM, FxForm_Vortex, 1.3f, 7.0f, 96, 5, 1.0f, 2.5f, 4.0f, FIX, kIllusion, 2.8f, 0.05f),
	F(IM, FxForm_Sparkle, 1.4f, 8.0f, 48, 24, 0.12f, 1, 0, FIX, kPrism, 2.8f),
	F(IM, FxForm_Bloom, 1.6f, 2.0f, 48, 2, 0, 0, 0, FIX, kIllusion, 2.2f, 0.04f));

LOOK(kEncInvis, "invisibility",
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 48, 24, 0.12f, 2, 0, FIX, kPhantasm, 2.2f),
	F(IM, FxForm_Bloom, 1.6f, 4.0f, 48, 2, 0, 0, 0, FIX, kGhost, 2.0f, 0.04f, ADD | FxFlag_Reverse));

LOOK(kEncEnchant, "enchant",
	F(IM, FxForm_Rune, 1.4f, 0, 60, 5, 2, 8, 0, FIX, kEncRune, 2.0f),
	F(IM, FxForm_Sparkle, 1.2f, 5.0f, 40, 20, 0.12f, 2, 0, FIX, kGold, 2.0f),
	G(IM, Glyph_Coin, 0.5f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kGold, 1.8f, 0.4f));

LOOK(kEncMindAura, "mind aura",
	F(RE, FxForm_Rune, 1.0f, 0, 120, 8, 3, 24, 0, FIX, kMana, 4.0f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Drop, 0.4f, 1.5f, 3, 6, GA_RingRise, 2.4f, FIX, kMana, 4.0f),
	F(RE, FxForm_Sparkle, 1.0f, 5.0f, 60, 30, 0.12f, 0, 0, FIX, kMana, 4.0f, 0.06f, ADD, 1, 0, AE));

LOOK(kEncManaAura, "mana aura",
	F(RE, FxForm_Rune, 1.0f, 0, 120, 5, 2, 20, 0, FIX, kSpectrum, 4.0f, 0.05f, ADD, 1, 0, AE),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kMana, 4.0f, 0.05f, ADD, 1.4f, 0, AE),
	G(RE, Glyph_Sun, 0.4f, 1.5f, 3, 5, GA_RingRise, 2.2f, FIX, kMana, 4.0f));

LOOK(kEncLearnerAura, "learner's aura",
	F(RE, FxForm_Rune, 1.0f, 0, 120, 6, 1, 18, 0, FIX, kArcane, 4.0f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Book, 0.45f, 1.5f, 3, 5, GA_RingRise, 2.4f, FIX, kArcane, 4.0f));

LOOK(kEncGlamourAura, "aura of glamour",
	F(RE, FxForm_Rune, 1.0f, 0, 120, 7, 2, 21, 0, FIX, kIllusion, 4.0f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Mask, 0.45f, 1.5f, 3, 5, GA_RingRise, 2.4f, FIX, kIllusion, 4.0f),
	F(RE, FxForm_Sparkle, 1.0f, 5.0f, 48, 24, 0.12f, 1, 0, FIX, kPink, 4.0f, 0.06f, ADD, 1, 0, AE));

LOOK(kEncTwincastAura, "twincast aura",
	F(RE, FxForm_Rune, 1.0f, 0, 120, 6, 1, 18, 0, FIX, kPrism, 4.0f, 0.05f, ADD, 1, 0, AE),
	F(RE, FxForm_Rune, 0.8f, 0, 96, 6, 1, 18, 0, FIX, kMagic, 4.0f, 0.05f, ADD, -1.0f, 0, AE),
	G(RE, Glyph_Gem, 0.4f, 1.5f, 3, 6, GA_RingRise, 2.4f, FIX, kPrism, 4.0f));

LOOK(kEncAtrophyAura, "atrophy aura",
	F(RE, FxForm_Rune, 1.0f, 0, 120, 5, 2, 15, 0, FIX, kHorror, 4.0f, 0.05f, OVER, 1, 0, AE),
	G(RE, Glyph_ArrowDown, 0.45f, 2.0f, 3, 6, GA_RingRise, 2.4f, FIX, kHorror, 4.0f),
	F(RE, FxForm_Cracks, 1.0f, 0, 64, 8, 1.2f, 0, 0, FIX, kVoid, 4.0f, 0.06f, OVER, 1, 0, AE));

LOOK(kEncRunicAura, "runic aura",
	F(RE, FxForm_Rune, 1.0f, 0, 120, 8, 3, 24, 0, FIX, kEncRune, 4.0f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Gem, 0.35f, 8.0f, 4, 6, GA_Rain, 2.2f, FIX, kEncRune, 3.0f),
	F(RE, FxForm_Crystal, 1.8f, 3.5f, 48, 8, 1, 0.8f, 0, FIX, kEncRune, 3.5f, 0.05f));

LOOK(kEncTemptAura, "tempting aura",
	F(RE, FxForm_Rune, 1.0f, 0, 96, 8, 3, 16, 0, FIX, kCharm, 3.5f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Heart, 0.4f, 2.0f, 3, 6, GA_RingRise, 2.4f, FIX, kCharm, 3.5f));

LOOK(kEncDisjunctionAura, "arcane disjunction",
	F(RE, FxForm_Vortex, 1.0f, 5.0f, 128, 8, 0.7f, 2.0f, -3.0f, FIX, kVoid, 4.0f, 0.06f, OVER, 1, 0, AE),
	F(RE, FxForm_Sparkle, 1.0f, 6.0f, 60, 30, 0.14f, 1, 0, FIX, kMagic, 4.0f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Lightning, 0.4f, 4.0f, 0, 5, GA_Orbit, 2.0f, FIX, kMagic, 3.5f));

LOOK(kEncAmplifyAura, "amplifying aura",
	F(RE, FxForm_Rune, 1.0f, 0, 120, 5, 2, 20, 0, FIX, kSpectrum, 4.0f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_ArrowUp, 0.45f, 1.5f, 3, 6, GA_RingRise, 2.4f, FIX, kSpectrum, 4.0f),
	F(RE, FxForm_Spire, 0.4f, 0.5f, 96, 16, 0.3f, 3.0f, 0, FIX, kSpectrum, 3.0f, 0.05f));

LOOK(kEncAnimation, "animation",
	F(RE, FxForm_Rune, 2.4f, 0, 80, 6, 1, 12, 0, FIX, kEncRune, 4.0f, 0.05f),
	G(RE, Glyph_Sword, 0.8f, 3.0f, 3, 3, GA_Blades, 1.4f, FIX, kAnimBlade, 4.0f),
	F(RE, FxForm_Vortex, 1.4f, 5.0f, 64, 4, 1.2f, 2.0f, 5.0f, FIX, kPrism, 3.5f, 0.05f),
	F(RE, FxForm_Sparkle, 1.6f, 6.0f, 40, 20, 0.12f, 2, 0, FIX, kPrism, 3.5f));

LOOK(kEncMinion, "minion",
	G(IM, Glyph_Sword, 0.7f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kAnimBlade, 2.6f),
	F(IM, FxForm_Rune, 1.4f, 3.2f, 60, 5, 2, 10, 2, FIX, kEncRune, 2.6f, 0.05f),
	F(IM, FxForm_Flame, 1.0f, 5.0f, 64, 16, 0.6f, 0.7f, 0, FIX, kChromatic, 2.0f, 0.05f));

LOOK(kAaDoppelganger, "doppelganger",
	G(RE, Glyph_Mask, 0.7f, 4.0f, 4, 3, GA_RingRise, 2.0f, FIX, kPhantasm, 3.5f),
	F(RE, FxForm_Bloom, 2.0f, 2.0f, 48, 3, 0, 0, 0, FIX, kIllusion, 2.4f, 0.05f),
	F(RE, FxForm_Sparkle, 2.0f, 7.0f, 48, 24, 0.12f, 0, 0, FIX, kPhantasm, 3.0f));

LOOK(kAaEnchantedDoppelganger, "enchanted doppelganger",
	G(RE, Glyph_Mask, 0.9f, 4.0f, 4, 2, GA_RingRise, 1.6f, FIX, kPrism, 3.5f),
	F(RE, FxForm_Vortex, 1.6f, 6.0f, 96, 5, 1.2f, 2.5f, 5.0f, FIX, kPrism, 3.0f, 0.05f),
	F(RE, FxForm_Rune, 2.0f, 0, 70, 6, 1, 12, 0, FIX, kIllusion, 3.0f, 0.05f));

LOOK(kAaPhantasmalOpponent, "phantasmal opponent",
	G(IM, Glyph_Spirit, 1.1f, 4.0f, 4, 1, GA_Emerge, 0, FIX, kPhantasm, 3.5f),
	G(IM, Glyph_Sword, 0.5f, 5.0f, 0, 2, GA_Orbit, 1.2f, FIX, kAnimBlade, 3.0f, 0.4f),
	F(IM, FxForm_Bloom, 2.0f, 2.0f, 48, 3, 0, 0, 0, FIX, kPhantasm, 2.4f, 0.05f));

LOOK(kAaArcaneWhisper, "arcane whisper",
	G(IM, Glyph_Mouth, 0.6f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kMesmer, 2.0f),
	F(IM, FxForm_Bloom, 1.6f, 2.0f, 40, 2, 0, 0, 0, FIX, kMesmer, 2.0f, 0.04f, ADD | FxFlag_Reverse));

LOOK(kAaAzureCrystal, "azure mind crystal",
	F(RE, FxForm_Crystal, 0.8f, 5.0f, 48, 8, 0, 0.8f, 0, FIX, kMana, 2.6f, 0.05f),
	F(RE, FxForm_Sparkle, 1.4f, 6.0f, 40, 20, 0.12f, 2, 0, FIX, kMana, 2.4f),
	G(RE, Glyph_Gem, 0.6f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kMana, 2.6f, 0.4f));

LOOK(kAaSanguineCrystal, "sanguine mind crystal",
	F(RE, FxForm_Crystal, 0.8f, 5.0f, 48, 8, 0, 0.8f, 0, FIX, kBlood, 2.6f, 0.05f),
	F(RE, FxForm_Sparkle, 1.4f, 6.0f, 40, 20, 0.12f, 2, 0, FIX, kBlood, 2.4f),
	G(RE, Glyph_Gem, 0.6f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kBlood, 2.6f, 0.4f));

LOOK(kAaBeamSlumber, "beam of slumber",
	F(IM, FxForm_Lightning, 1, 0, 64, 3, 0.3f, 16.0f, 0, FIX, kDream, 1.0f, 0.20f),
	G(RE, Glyph_Z, 0.35f, 5.0f, 0, 4, GA_Orbit, 1.2f, FIX, kMesmer, 1.4f),
	F(RE, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.10f, 1, 0, FIX, kDream, 1.4f));

LOOK(kAaBanishment, "beguiler's banishment",
	F(RE, FxForm_Bloom, 1.0f, 2.0f, 96, 3, 0, 0, 0, FIX, kPhantasm, 2.0f, 0.06f, ADD, 1.5f, 0, AE),
	G(RE, Glyph_Door, 0.9f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kPhantasm, 2.2f),
	F(RE, FxForm_Wind, 1.6f, 5.0f, 60, 14, 3.0f, 0, 0, FIX, kPhantasm, 1.6f));

LOOK(kAaDirectedBanishment, "directed banishment",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.6f, 0.4f, 1.0f, FIX, kPhantasm, 0.7f, 0.12f),
	G(IM, Glyph_Door, 0.9f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kPhantasm, 2.0f),
	F(IM, FxForm_Vortex, 1.2f, 6.0f, 64, 4, 1.2f, 2.0f, 6.0f, FIX, kPhantasm, 1.6f, 0.05f));

LOOK(kAaBiteTashani, "bite of tashani",
	F(RE, FxForm_Bloom, 1.0f, 2.0f, 96, 3, 0, 0, 0, FIX, kTashan, 1.8f, 0.06f, ADD, 1.5f, 0, AE),
	G(RE, Glyph_Fang, 0.9f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kTashan, 2.0f),
	G(IM, Glyph_Wolf, 0.5f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kTashan, 1.6f));

LOOK(kAaBlanket, "blanket of forgetfulness",
	F(RE, FxForm_Bloom, 1.0f, 3.0f, 120, 4, 0, 0, 0, FIX, kGhost, 2.6f, 0.07f, ADD | FxFlag_Reverse, 1, 0, AE),
	G(RE, Glyph_Spiral, 0.5f, 4.0f, 3, 6, GA_RingRise, 2.4f, FIX, kLull, 2.6f),
	F(RE, FxForm_Sparkle, 1.0f, 6.0f, 60, 30, 0.12f, 0, 0, FIX, kGhost, 2.6f, 0.06f, ADD, 1, 0, AE));

LOOK(kAaCalculatedInsanity, "calculated insanity",
	G(IM, Glyph_Spiral, 0.9f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kPrism, 3.0f),
	F(IM, FxForm_Spire, 0.5f, 6.0f, 96, 20, 1.5f, 3.0f, 0, FIX, kChromatic, 2.4f, 0.05f),
	G(IM, Glyph_Gem, 0.3f, 8.0f, 4, 6, GA_Rain, 1.4f, FIX, kSpectrum, 2.4f));

LOOK(kAaChromaticHaze, "chromatic haze",
	F(IM, FxForm_Sparkle, 1.6f, 8.0f, 80, 40, 0.16f, 0, 0, FIX, kPrism, 3.0f),
	F(IM, FxForm_Vortex, 1.3f, 6.0f, 96, 5, 1.0f, 2.5f, 3.0f, FIX, kChromatic, 3.0f, 0.05f),
	G(IM, Glyph_Gem, 0.7f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kSpectrum, 3.0f));

LOOK(kAaDimensionalShield, "dimensional shield",
	F(IM, FxForm_Dome, 2.0f, 7.4f, 128, 10, 4, 0, 0, FIX, kVoid, 3.5f, 0.06f, OVER),
	G(IM, Glyph_Shield, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGravity, 3.5f),
	F(IM, FxForm_Sparkle, 1.8f, 7.0f, 40, 20, 0.12f, 2, 0, FIX, kGravity, 3.0f));

LOOK(kAaDireCharm, "dire charm",
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 0.8f, 0.5f, 1.8f, FIX, kCharm, 0.9f, 0.16f),
	G(IM, Glyph_Heart, 1.3f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kBlood, 3.5f),
	G(IM, Glyph_Crown, 0.6f, 9.6f, 4, 1, GA_Drop, 0, FIX, kDominate, 3.5f, 0.3f),
	F(IM, FxForm_Lightning, 1, 16.0f, 64, 5, 0.05f, 0.5f, 1.0f, FIX, kCharm, 3.0f, 0.03f),
	F(IM, FxForm_Burst, 2.4f, 1.0f, 96, 3, 1.5f, 20, 0, FIX, kCharm, 1.2f, 0.10f));

LOOK(kAaEldritchRune, "eldritch rune",
	F(IM, FxForm_Rune, 2.4f, 0, 96, 9, 4, 24, 0, FIX, kEncRune, 3.5f, 0.06f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 112, 8, 3, 0, 0, FIX, kGold, 3.0f, 0.04f),
	G(IM, Glyph_Gem, 0.4f, 4.5f, 3, 6, GA_RingRise, 1.8f, FIX, kEncRune, 3.5f));

LOOK(kAaEnvelopingHelix, "enveloping helix",
	F(IM, FxForm_Helix, 1.0f, 6.5f, 128, 5, 0.6f, 0, 0.3f, FIX, kMesmer, 3.0f, 0.06f, COMET | FxFlag_Reverse, 1, 0, AE),
	G(IM, Glyph_Hourglass, 0.45f, 5.0f, 0, 5, GA_Orbit, 2.2f, FIX, kTwilight, 3.0f));

LOOK(kAaSlowingHelix, "slowing helix",
	G(IM, Glyph_Hourglass, 1.0f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kLull, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 96, 4, 0.4f, 0, 0.2f, FIX, kLull, 3.0f, 0.05f, COMET | FxFlag_Reverse));

LOOK(kAaFocusArcanum, "focus of arcanum",
	G(IM, Glyph_Book, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kArcane, 3.0f),
	F(IM, FxForm_Spire, 0.6f, 4.0f, 96, 16, 1.0f, 3.0f, 0, FIX, kArcane, 2.6f, 0.05f));

LOOK(kAaFogMemories, "fog of memories",
	F(RE, FxForm_Bloom, 1.0f, 3.0f, 120, 3, 0, 0, 0, FIX, kGhost, 3.0f, 0.08f, ADD | FxFlag_Reverse, 0.6f, 0, AE),
	F(RE, FxForm_Sparkle, 1.0f, 4.0f, 80, 40, 0.16f, 0, 0, FIX, kGhost, 3.0f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Book, 0.6f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 2.4f));

LOOK(kAaFriendlyStasis, "friendly stasis",
	F(IM, FxForm_Crystal, 1.1f, 5.5f, 60, 12, 0, 0.6f, 0, FIX, kAmber, 3.0f, 0.05f),
	G(IM, Glyph_Hourglass, 0.6f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kAmber, 3.0f));

LOOK(kAaSelfStasis, "self stasis",
	F(IM, FxForm_Dome, 1.7f, 6.8f, 96, 8, 3, 0, 0, FIX, kAmber, 3.0f, 0.05f),
	G(IM, Glyph_Hourglass, 0.8f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kAmber, 3.0f),
	F(IM, FxForm_Sparkle, 1.4f, 7.0f, 32, 16, 0.10f, 2, 0, FIX, kAmber, 2.6f));

LOOK(kAaGatherMana, "gather mana",
	F(IM, FxForm_Vortex, 1.3f, 6.0f, 96, 6, 1.4f, 2.5f, -4.0f, FIX, kMana, 2.6f, 0.05f),
	F(IM, FxForm_Sparkle, 1.8f, 7.0f, 60, 30, 0.12f, 2, 0, FIX, kSpectrum, 2.6f),
	G(IM, Glyph_Drop, 0.7f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kMana, 2.4f));

LOOK(kAaGlyphSpray, "glyph spray",
	G(IM, Glyph_Gem, 0.4f, 12.0f, 4, 8, GA_Rain, 2.6f, FIX, kSpectrum, 2.4f),
	F(IM, FxForm_Burst, 1.0f, 0.5f, 96, 2, 1.0f, 16, 0, FIX, kPrism, 1.2f, 0.09f, ADD, 1, 0.8f, AE),
	F(IM, FxForm_Rain, 1.0f, 20.0f, 60, 50, 0, 0.2f, 0, FIX, kPrism, 2.4f, 0.05f, ADD, 1, 0, AE));

LOOK(kAaInvis, "perfected invisibility",
	F(IM, FxForm_Sparkle, 1.3f, 7.5f, 60, 30, 0.12f, 2, 0, FIX, kPhantasm, 2.4f),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 64, 16, 0.9f, 0.6f, 0, FIX, kPhantasm, 1.8f, 0.04f),
	G(IM, Glyph_Eye, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 1.8f));

LOOK(kAaGroupInvis, "group perfected invisibility",
	F(RE, FxForm_Bloom, 1.0f, 3.0f, 96, 3, 0, 0, 0, FIX, kPhantasm, 2.4f, 0.06f, ADD | FxFlag_Reverse, 1, 0, AE),
	G(RE, Glyph_Eye, 0.4f, 4.0f, 3, 5, GA_RingRise, 2.2f, FIX, kGhost, 2.4f),
	F(IM, FxForm_Sparkle, 1.3f, 7.5f, 48, 24, 0.12f, 2, 0, FIX, kPhantasm, 2.2f));

LOOK(kAaInvisUndead, "invisibility to undead",
	F(IM, FxForm_Sparkle, 1.3f, 7.5f, 60, 30, 0.12f, 2, 0, FIX, kGhost, 2.4f),
	G(IM, Glyph_Skull, 0.6f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 1.8f));

LOOK(kAaGroupInvisUndead, "group invisibility to undead",
	F(RE, FxForm_Bloom, 1.0f, 3.0f, 96, 3, 0, 0, 0, FIX, kGhost, 2.4f, 0.06f, ADD | FxFlag_Reverse, 1, 0, AE),
	G(RE, Glyph_Skull, 0.4f, 4.0f, 3, 5, GA_RingRise, 2.2f, FIX, kGhost, 2.4f));

LOOK(kAaLevitation, "perfected levitation",
	G(RE, Glyph_Feather, 0.4f, 1.0f, 0, 5, GA_PopAscend | GlyphAnim_Spin, 1.6f, FIX, kFeather, 2.4f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kLull, 2.0f, 0.05f, ADD, 1, 0, AE),
	F(IM, FxForm_Wind, 1.0f, 1.0f, 40, 10, 1.5f, 0, 0, FIX, kAir, 2.0f));

LOOK(kAaIllusionsGrandeur, "illusions of grandeur",
	G(IM, Glyph_Crown, 0.8f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kRoyal, 3.5f),
	G(IM, Glyph_Mask, 0.4f, 5.0f, 0, 4, GA_Orbit, 1.4f, FIX, kIllusion, 3.5f),
	F(IM, FxForm_Ring, 1.2f, 3.0f, 48, 0, 1.2f, 1.0f, 0.5f, FIX, kPrism, 3.5f, 0.05f, COMET));

LOOK(kAaImprovedTwincast, "improved twincast",
	F(IM, FxForm_Rune, 1.6f, 3.4f, 70, 6, 1, 12, 2, FIX, kMesmer, 3.0f, 0.05f),
	F(IM, FxForm_Rune, 1.6f, 3.4f, 70, 6, 1, 12, 2, FIX, kSpectrum, 3.0f, 0.05f, ADD, -1.0f),
	G(IM, Glyph_Gem, 0.45f, 7.6f, 4, 2, GA_PopSpin, 0.5f, FIX, kPrism, 3.0f));

LOOK(kAaMentalContortion, "mental contortion",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 1.0f, 0.4f, 1.2f, FIX, kVoid, 0.8f, 0.14f, OVER),
	G(IM, Glyph_Spiral, 1.0f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kVoid, 3.0f),
	F(IM, FxForm_Vortex, 1.1f, 7.0f, 96, 5, 1.8f, 3.0f, -7.0f, FIX, kGravity, 3.0f, 0.05f),
	Stars(0.3f));

LOOK(kAaMindOverMatter, "mind over matter",
	F(IM, FxForm_Dome, 1.7f, 6.8f, 96, 8, 3, 0, 0, FIX, kMana, 3.0f, 0.04f),
	G(IM, Glyph_Eye, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kMana, 3.0f),
	G(IM, Glyph_Drop, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kMana, 3.0f));

LOOK(kAaNoctambulate, "noctambulate",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.6f, 0.4f, 1.0f, FIX, kTwilight, 0.9f, 0.12f),
	G(IM, Glyph_Moon, 0.8f, 7.8f, 4, 1, GA_PopRoll, 0, FIX, kTwilight, 3.0f),
	G(IM, Glyph_Z, 0.35f, 6.5f, 0, 3, GA_PopAscend, 0.9f, FIX, kDream, 3.0f, 0.4f),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 64, 16, 0.9f, 0.6f, 0, FIX, kTwilight, 2.4f, 0.05f, OVER));

LOOK(kAaOratorsUnity, "orator's unity",
	G(IM, Glyph_Mouth, 0.7f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kRoyal, 3.0f),
	F(IM, FxForm_Rune, 1.6f, 0, 80, 7, 3, 21, 0, FIX, kRoyal, 3.0f, 0.05f),
	G(IM, Glyph_Book, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kMana, 3.0f));

LOOK(kAaPhantasmalStep, "phantasmal step",
	F(RE, FxForm_Vortex, 1.2f, 6.0f, 96, 5, 0.8f, 2.5f, 6.0f, FIX, kPhantasm, 1.2f, 0.06f),
	G(RE, Glyph_Door, 0.9f, 4.5f, 4, 1, GA_PopRoll, 0, FIX, kPhantasm, 1.2f),
	F(RE, FxForm_Sparkle, 1.4f, 6.0f, 40, 20, 0.12f, 2, 0, FIX, kPhantasm, 1.4f));

LOOK(kAaReactiveRune, "reactive rune",
	F(RE, FxForm_Rune, 1.0f, 0, 120, 6, 1, 18, 0, FIX, kEncRune, 3.0f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Lightning, 0.4f, 3.0f, 3, 6, GA_RingRise, 2.4f, FIX, kEncRune, 3.0f),
	F(IM, FxForm_Dome, 1.6f, 6.4f, 80, 6, 2, 0, 0, FIX, kEncRune, 2.2f, 0.04f));

LOOK(kAaRuneBanishment, "rune of banishment",
	F(IM, FxForm_Rune, 1.8f, 0, 80, 7, 2, 14, 0, FIX, kVoid, 3.0f, 0.06f),
	G(IM, Glyph_Door, 0.9f, 3.0f, 4, 1, GA_Emerge, 0, FIX, kGravity, 3.0f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, 5.0f, FIX, kVoid, 2.6f, 0.05f, OVER));

LOOK(kAaSilentCasting, "silent casting",
	G(IM, Glyph_Mouth, 0.7f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 2.4f),
	F(IM, FxForm_Bloom, 1.2f, 5.0f, 40, 2, 0, 0, 0, FIX, kGhost, 2.0f, 0.04f, ADD | FxFlag_Reverse));

LOOK(kAaSpire, "spire of enchantment",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kPrism, 2.6f, 0.10f),
	G(RE, Glyph_Mask, 1.1f, 9.0f, 4, 1, GA_Drop, 0, FIX, kIllusion, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kChromatic, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaVeilMindshadow, "veil of mindshadow",
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kShadow, 2.5f, 0.05f, OVER),
	G(IM, Glyph_Mask, 0.7f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kTwilight, 2.2f),
	F(IM, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kTwilight, 2.0f, 0.05f, OVER | FxFlag_Reverse));

enum Line : int
{
	Mez, Docility, Wave, Flash, Stare, Chaotic, Perilous, Glance, Ward, WardProc, Memory,
	Charm, Command, Demand, CharmEcho,
	Color, RemoteColor, Spin, PolyRune, RuneStrike, Gravity, Lurch,
	Assault, Chroma, Jab, Mindblade, Appropriation, Tears, Theft, Strangle, Constriction, MindDot,
	Cripple, Tashan, Helix, Deeds, Covenant, Fetter, Fear, Dispel, NightTerror,
	SelfRune, Rune, GroupRune, Shadows, Shield, Brimstone, Aegis, Legion, DotAegis, DotLegion,
	Auspice, GroupAuspice, HateRune, HateRuneProc, ShieldFate, Mirror,
	Haste, GroupHaste, Clarity, Voice, Lull, Issuance, ManaFlare, Manastorm, ManaIssuance, Echo,
	Unity, Reinforcement, Visage, Illusion, Invis, Enchant,
	MindAura, ManaAura, LearnerAura, GlamourAura, TwincastAura, AtrophyAura, RunicAura, TemptAura,
	DisjunctionAura, AmplifyAura,
	Animation, Minion,
	Doppelganger, EnchantedDoppelganger, PhantasmalOpponent, ArcaneWhisper, AzureCrystal,
	SanguineCrystal, BeamSlumber, Banishment, DirectedBanishment, BiteTashani, Blanket,
	CalculatedInsanity, ChromaticHaze, DimensionalShield, DireCharm, EldritchRune, EnvelopingHelix,
	SlowingHelix, FocusArcanum, FogMemories, FriendlyStasis, SelfStasis, GatherMana, GlyphSpray,
	PerfectedInvis, GroupInvis, InvisUndead, GroupInvisUndead, Levitation, IllusionsGrandeur,
	ImprovedTwincast, MentalContortion, MindOverMatter, Noctambulate, OratorsUnity, PhantasmalStep,
	ReactiveRune, RuneBanishment, SilentCasting, Spire, VeilMindshadow,
};

const LineLook kLines[] = {
	{ "Mesmerize (single)",               &kEncMez },
	{ "Docility (targeted mez)",          &kEncDocility },
	{ "Wave (area mez)",                  &kEncWave },
	{ "Flash (quick mez)",                &kEncFlash },
	{ "Stare (location mez)",             &kEncStare },
	{ "Chaotic (mez, twincast)",          &kEncChaotic },
	{ "Perilous (point blank mez)",       &kEncPerilous },
	{ "Glance (daze, forget)",            &kEncGlance },
	{ "Ward (mez ward)",                  &kEncWard },
	{ "ward trigger",                     &kEncWardProc },
	{ "Memory blur",                      &kEncMemory },
	{ "Charm",                            &kEncCharm },
	{ "Command (charm)",                  &kEncCommand },
	{ "Demand (short charm)",             &kEncDemand },
	{ "Echo of Subjugation",              &kEncCharmEcho },
	{ "Color (area stun)",                &kEncColor },
	{ "Remote Color (location stun)",     &kEncRemoteColor },
	{ "Dizzying (spin stun)",             &kEncSpin },
	{ "Polychromatic Rune",               &kEncPolyRune },
	{ "Rune Strike (rune stun)",          &kEncRuneStrike },
	{ "Gravity (knockback)",              &kEncGravity },
	{ "Compliant Lurch (pull)",           &kEncLurch },
	{ "Assault (nuke)",                   &kEncAssault },
	{ "Chroma (nuke)",                    &kEncChroma },
	{ "Chromatic Jab (nuke)",             &kEncJab },
	{ "Mindblade (nuke)",                 &kEncMindblade },
	{ "Appropriation (nuke, mana)",       &kEncAppropriation },
	{ "Tears (mana burn)",                &kEncTears },
	{ "Theft of Thought (mana drain)",    &kEncTheft },
	{ "Strangle (dot)",                   &kEncStrangle },
	{ "Constriction (dot, debuff)",       &kEncConstriction },
	{ "Mind Vortex (dot, mana)",          &kEncMindDot },
	{ "Consciousness (debuff)",           &kEncCripple },
	{ "Tashan (resist debuff)",           &kEncTashan },
	{ "Helix (slow)",                     &kEncHelix },
	{ "Deeds (slow)",                     &kEncDeeds },
	{ "Chromatic Covenant (debuff)",      &kEncCovenant },
	{ "Fetter (root)",                    &kEncFetter },
	{ "Terror (fear)",                    &kEncFear },
	{ "Strip Enchantment (dispel)",       &kEncDispel },
	{ "Night's Terror (illusion proc)",   &kEncNightTerror },
	{ "Rune (self)",                      &kEncSelfRune },
	{ "Rune of (target rune)",            &kEncRuneOf },
	{ "Group rune",                       &kEncGroupRune },
	{ "Obscuring Shadows (rune proc)",    &kEncShadows },
	{ "Shield (hit points)",              &kEncShield },
	{ "Brimstone (self rune)",            &kEncBrimstone },
	{ "Aegis (spell absorb)",             &kEncAegis },
	{ "Legion (group spell absorb)",      &kEncLegion },
	{ "Aegis of the Keeper (dot absorb)", &kEncDotAegis },
	{ "Legion of the Keeper (group dot absorb)", &kEncDotLegion },
	{ "Auspice (melee rune)",             &kEncAuspice },
	{ "Auspice (group aegis)",            &kEncGroupAuspice },
	{ "Horrifying Rune (hate rune)",      &kEncHateRune },
	{ "hate rune effect",                 &kEncHateRuneProc },
	{ "Shield of Fate",                   &kEncShieldFate },
	{ "Reflect",                          &kEncMirror },
	{ "Speed (haste)",                    &kEncHaste },
	{ "Hastening (group haste)",          &kEncGroupHaste },
	{ "Clarity (mana regen)",             &kEncClarity },
	{ "Voice (group mana regen)",         &kEncVoice },
	{ "Mind (calm)",                      &kEncLull },
	{ "Issuance (location calm)",         &kEncIssuance },
	{ "Mana Flare (spell proc)",          &kEncManaFlare },
	{ "Manastorm",                        &kEncManastorm },
	{ "Issuance of Mana",                 &kEncManaIssuance },
	{ "Echo (location regen)",            &kEncEcho },
	{ "Unity",                            &kEncUnity },
	{ "Reinforcement",                    &kEncReinforcement },
	{ "Visage (hate)",                    &kEncVisage },
	{ "Illusion",                         &kEncIllusion },
	{ "Invisibility",                     &kEncInvis },
	{ "Enchant (summon item)",            &kEncEnchant },
	{ "Mind aura (mana regen)",           &kEncMindAura },
	{ "Mana aura (spell proc)",           &kEncManaAura },
	{ "Learner's Aura",                   &kEncLearnerAura },
	{ "Aura of Endless Glamour",          &kEncGlamourAura },
	{ "Twincast Aura",                    &kEncTwincastAura },
	{ "Wake of Atrophy Aura",             &kEncAtrophyAura },
	{ "Runic Aura",                       &kEncRunicAura },
	{ "Tempting Aura",                    &kEncTemptAura },
	{ "Arcane Disjunction Aura",          &kEncDisjunctionAura },
	{ "Amplifying Aura",                  &kEncAmplifyAura },
	{ "Animation (pet)",                  &kEncAnimation },
	{ "Minion (pet buff)",                &kEncMinion },

	{ "AA: Doppelganger (swarm)",         &kAaDoppelganger },
	{ "AA: Enchanted Doppelganger (swarm)", &kAaEnchantedDoppelganger },
	{ "AA: Phantasmal Opponent (swarm)",  &kAaPhantasmalOpponent },
	{ "AA: Arcane Whisper",               &kAaArcaneWhisper },
	{ "AA: Azure Mind Crystal",           &kAaAzureCrystal },
	{ "AA: Sanguine Mind Crystal",        &kAaSanguineCrystal },
	{ "AA: Beam of Slumber",              &kAaBeamSlumber },
	{ "AA: Beguiler's Banishment",        &kAaBanishment },
	{ "AA: Beguiler's Directed Banishment", &kAaDirectedBanishment },
	{ "AA: Bite of Tashani",              &kAaBiteTashani },
	{ "AA: Blanket of Forgetfulness",     &kAaBlanket },
	{ "AA: Calculated Insanity",          &kAaCalculatedInsanity },
	{ "AA: Chromatic Haze",               &kAaChromaticHaze },
	{ "AA: Dimensional Shield",           &kAaDimensionalShield },
	{ "AA: Dire Charm",                   &kAaDireCharm },
	{ "AA: Eldritch Rune",                &kAaEldritchRune },
	{ "AA: Enveloping Helix",             &kAaEnvelopingHelix },
	{ "AA: Slowing Helix",                &kAaSlowingHelix },
	{ "AA: Focus of Arcanum",             &kAaFocusArcanum },
	{ "AA: Fog of Memories",              &kAaFogMemories },
	{ "AA: Friendly Stasis",              &kAaFriendlyStasis },
	{ "AA: Self Stasis",                  &kAaSelfStasis },
	{ "AA: Gather Mana",                  &kAaGatherMana },
	{ "AA: Glyph Spray",                  &kAaGlyphSpray },
	{ "AA: Perfected Invisibility",       &kAaInvis },
	{ "AA: Group Perfected Invisibility", &kAaGroupInvis },
	{ "AA: Perfected Invisibility to Undead", &kAaInvisUndead },
	{ "AA: Group Perfected Invisibility to Undead", &kAaGroupInvisUndead },
	{ "AA: Perfected Levitation",         &kAaLevitation },
	{ "AA: Illusions of Grandeur",        &kAaIllusionsGrandeur },
	{ "AA: Improved Twincast",            &kAaImprovedTwincast },
	{ "AA: Mental Contortion",            &kAaMentalContortion },
	{ "AA: Mind Over Matter",             &kAaMindOverMatter },
	{ "AA: Noctambulate",                 &kAaNoctambulate },
	{ "AA: Orator's Unity",               &kAaOratorsUnity },
	{ "AA: Phantasmal Step",              &kAaPhantasmalStep },
	{ "AA: Reactive Rune",                &kAaReactiveRune },
	{ "AA: Rune of Banishment",           &kAaRuneBanishment },
	{ "AA: Silent Casting",               &kAaSilentCasting },
	{ "AA: Spire of Enchantment",         &kAaSpire },
	{ "AA: Veil of Mindshadow",           &kAaVeilMindshadow },
};

const GroupLine kNumbered[] = {
	{ 0, Cripple },     { 1, SelfRune },    { 2, Shield },      { 3, Animation },   { 4, Rune },
	{ 5, Lull },        { 6, Haste },       { 7, Tashan },      { 8, Charm },       { 9, Clarity },
	{ 10, Aegis },      { 11, Assault },    { 12, Docility },   { 14, Strangle },   { 15, Wave },
	{ 16, Brimstone },  { 17, Color },      { 18, Mez },        { 19, Spin },       { 20, Legion },
	{ 21, Command },    { 22, GroupHaste }, { 23, Voice },      { 24, Ward },       { 25, PolyRune },
	{ 26, RuneStrike }, { 27, Tears },      { 28, GroupRune },  { 29, MindAura },   { 30, MindDot },
	{ 31, ManaFlare },  { 32, Flash },      { 33, ManaAura },   { 35, LearnerAura }, { 36, GlamourAura },
	{ 37, TwincastAura }, { 38, AtrophyAura }, { 39, RunicAura }, { 41, TemptAura }, { 43, Stare },
	{ 44, DisjunctionAura }, { 45, Chaotic }, { 47, Lurch },    { 48, Jab },        { 49, Chroma },
	{ 50, Issuance },   { 51, Helix },      { 52, Constriction }, { 53, CharmEcho }, { 54, Manastorm },
	{ 55, Mindblade },  { 56, Unity },      { 57, RemoteColor }, { 58, HateRune },  { 59, ManaIssuance },
	{ 60, Echo },       { 61, AmplifyAura }, { 62, Auspice },   { 63, Perilous },   { 64, Appropriation },
	{ 65, DotAegis },   { 66, DotLegion },  { 67, Minion },     { 68, Glance },     { 71, Reinforcement },
	{ 74, NightTerror }, { 75, Gravity },   { 76, Covenant },   { 77, GroupAuspice }, { 78, Demand },
};

const GroupLine kModern[] = {
	{ 10, Cripple },    { 20, SelfRune },   { 30, Shield },     { 40, Animation },  { 50, Rune },
	{ 60, Lull },       { 70, Haste },      { 80, Tashan },     { 90, Charm },      { 100, Clarity },
	{ 110, Aegis },     { 120, Assault },   { 130, Docility },  { 140, Strangle },  { 150, Wave },
	{ 160, Brimstone }, { 170, Color },     { 180, Mez },       { 190, Spin },      { 200, Legion },
	{ 210, Command },   { 220, GroupHaste }, { 230, Voice },    { 240, Ward },      { 250, PolyRune },
	{ 251, RuneStrike }, { 260, Tears },    { 270, GroupRune }, { 280, MindAura },  { 290, MindDot },
	{ 300, ManaFlare }, { 310, Flash },     { 320, ManaAura },  { 380, RunicAura }, { 400, Stare },
	{ 420, Chaotic },   { 440, Jab },       { 450, Chroma },    { 460, Issuance },  { 470, Helix },
	{ 480, Constriction }, { 490, CharmEcho }, { 510, Mindblade }, { 520, Unity },  { 530, RemoteColor },
	{ 540, HateRune },  { 550, ManaIssuance }, { 560, Echo },   { 570, AmplifyAura }, { 580, Auspice },
	{ 590, Perilous },  { 600, Appropriation }, { 610, DotAegis }, { 620, DotLegion }, { 630, Minion },
	{ 640, Glance },    { 650, Reinforcement }, { 660, NightTerror }, { 680, Covenant },
	{ 690, GroupAuspice }, { 700, Demand },
};

const GroupLine kOther[] = {
	{ 10000, Cripple },  { 10001, SelfRune }, { 10002, Shield },   { 10006, Rune },     { 10007, Lull },
	{ 10008, Haste },    { 10009, Illusion }, { 10010, Tashan },   { 10012, Charm },    { 10013, Clarity },
	{ 10014, Aegis },    { 10015, Assault },  { 10017, Docility }, { 10019, Strangle }, { 10020, Wave },
	{ 10021, Brimstone }, { 10022, Color },   { 10023, Mez },      { 10024, Spin },     { 10025, Legion },
	{ 10026, Command },  { 10027, GroupHaste }, { 10028, Voice },  { 10029, Ward },     { 10032, PolyRune },
	{ 10035, GroupRune }, { 10036, MindAura }, { 10037, MindDot }, { 10038, ManaFlare }, { 10039, Flash },
	{ 10040, Ward },
	{ 4573, Charm },     { 4576, Clarity },   { 5507, Haste },     { 5518, Assault },   { 5520, Mez },
	{ 7003, Wave },      { 28032, ManaFlare }, { 1314051, Deeds }, { 6004877, Deeds },  { 1599, Mirror },
	{ 9554, ShieldFate }, { 9654, ShieldFate }, { 9754, ShieldFate }, { 9854, ShieldFate }, { 9954, ShieldFate },
	{ 125110480, ShieldFate }, { 126110480, ShieldFate },
};

const NamedLine kAbilities[] = {
	{ "doppelganger",                          Doppelganger },
	{ "enchanted doppelganger",                EnchantedDoppelganger },
	{ "phantasmal opponent",                   PhantasmalOpponent },
	{ "arcane whisper",                        ArcaneWhisper },
	{ "azure mind crystal",                    AzureCrystal },
	{ "sanguine mind crystal",                 SanguineCrystal },
	{ "beam of slumber",                       BeamSlumber },
	{ "beguiler's banishment",                 Banishment },
	{ "beguiler's directed banishment",        DirectedBanishment },
	{ "bite of tashani",                       BiteTashani },
	{ "blanket of forgetfulness",              Blanket },
	{ "calculated insanity",                   CalculatedInsanity },
	{ "chromatic haze",                        ChromaticHaze },
	{ "gift of chromatic haze",                ChromaticHaze },
	{ "dimensional shield",                    DimensionalShield },
	{ "dire charm",                            DireCharm },
	{ "eldritch rune",                         EldritchRune },
	{ "enveloping helix",                      EnvelopingHelix },
	{ "slowing helix",                         SlowingHelix },
	{ "focus of arcanum",                      FocusArcanum },
	{ "fog of memories",                       FogMemories },
	{ "friendly stasis",                       FriendlyStasis },
	{ "self stasis",                           SelfStasis },
	{ "gather mana",                           GatherMana },
	{ "glyph spray",                           GlyphSpray },
	{ "perfected invisibility",                PerfectedInvis },
	{ "group perfected invisibility",          GroupInvis },
	{ "perfected invisibility to undead",      InvisUndead },
	{ "group perfected invisibility to undead", GroupInvisUndead },
	{ "perfected levitation",                  Levitation },
	{ "group perfected levitation",            Levitation },
	{ "illusions of grandeur",                 IllusionsGrandeur },
	{ "improved twincast",                     ImprovedTwincast },
	{ "mental contortion",                     MentalContortion },
	{ "mind over matter",                      MindOverMatter },
	{ "noctambulate",                          Noctambulate },
	{ "orator's unity",                        OratorsUnity },
	{ "phantasmal step",                       PhantasmalStep },
	{ "reactive rune",                         ReactiveRune },
	{ "rune of banishment",                    RuneBanishment },
	{ "silent casting",                        SilentCasting },
	{ "spire of enchantment",                  Spire },
	{ "veil of mindshadow",                    VeilMindshadow },
};

const NamedLine kClassic[] = {
	{ "mesmerize", Mez },              { "enthrall", Mez },              { "entrance", Mez },
	{ "dazzle", Mez },                 { "glamour of kintaz", Mez },     { "rapture", Mez },
	{ "ancient: eternal rapture", Mez }, { "apathy", Mez },              { "sleep", Mez },
	{ "bliss", Mez },                  { "felicity", Mez },              { "euphoria", Mez },
	{ "echoing madness", Mez },
	{ "mesmerization", Wave },         { "fascination", Wave },          { "word of morell", Wave },
	{ "bliss of the nihil", Wave },    { "wake of felicity", Wave },     { "circle of dreams", Wave },
	{ "memory blur", Memory },         { "mind wipe", Memory },          { "reoccurring amnesia", Memory },
	{ "memory flux", Memory },         { "boggle", Memory },
	{ "charm", Charm },                { "beguile", Charm },             { "cajoling whispers", Charm },
	{ "allure", Charm },               { "boltran's agacerie", Charm },  { "dictate", Charm },
	{ "beckon", Charm },               { "command of druzzil", Charm },  { "compel", Charm },
	{ "true name", Charm },            { "ancient: voice of muram", Charm }, { "ordinance", Charm },
	{ "color flux", Color },           { "color shift", Color },         { "color skew", Color },
	{ "color slant", Color },          { "color cloud", Color },         { "color snap", Color },
	{ "whirl till you hurl", Spin },   { "dyn's dizzying draught", Spin },
	{ "chaos flux", Assault },         { "sanity warp", Assault },       { "dementia", Assault },
	{ "dementing visions", Assault },  { "insanity", Assault },          { "madness of ikkibi", Assault },
	{ "ancient: chaotic visions", Assault }, { "ancient: chaos madness", Assault },
	{ "ancient: neurosis", Assault },  { "psychosis", Assault },       { "feedback", Assault },
	{ "chaotic feedback", Assault },   { "anarchy", Assault },           { "discordant mind", Assault },
	{ "entrancing lights", Mez },      { "gravity flux", Gravity },      { "benevolence", Clarity },
	{ "boon of the garou", Illusion },
	{ "colored chaos", Chroma },
	{ "largarn's lamentation", Tears },
	{ "theft of thought", Theft },     { "scryer's trespass", Theft },   { "mana sieve", Theft },
	{ "strangle", Strangle },          { "asphyxiate", Strangle },       { "arcane noose", Strangle },
	{ "suffocate", Strangle },         { "gasping embrace", Strangle },  { "choke", Strangle },
	{ "suffocating sphere", Strangle },
	{ "torment of argli", MindDot },   { "torment of scio", MindDot },   { "mind shatter", MindDot },
	{ "weaken", Cripple },             { "enfeeblement", Cripple },      { "ebbing strength", Cripple },
	{ "feckless might", Cripple },     { "insipid weakness", Cripple },  { "weakness", Cripple },
	{ "listless power", Cripple },     { "disempower", Cripple },        { "incapacitate", Cripple },
	{ "cripple", Cripple },            { "synapsis spasm", Cripple },
	{ "tashina", Tashan },             { "tashani", Tashan },            { "tashania", Tashan },
	{ "tashanian", Tashan },           { "howl of tashan", Tashan },     { "wind of tashani", Tashan },
	{ "wind of tashanian", Tashan },
	{ "languid pace", Deeds },         { "tepid deeds", Deeds },         { "shiftless deeds", Deeds },
	{ "forlorn deeds", Deeds },        { "desolate deeds", Deeds },      { "dreary deeds", Deeds },
	{ "root", Fetter },                { "fetter", Fetter },             { "greater fetter", Fetter },
	{ "instill", Fetter },             { "immobilize", Fetter },         { "paralyzing earth", Fetter },
	{ "fear", Fear },                  { "invoke fear", Fear },          { "trepidation", Fear },
	{ "phobia", Fear },                { "jitterskin", Fear },           { "anxiety attack", Fear },
	{ "cancel magic", Dispel },        { "nullify magic", Dispel },      { "strip enchantment", Dispel },
	{ "pillage enchantment", Dispel }, { "recant magic", Dispel },       { "taper enchantment", Dispel },
	{ "night's dark terror", NightTerror },
	{ "rune", Rune },                  { "rune of zebuxoruk", Rune },    { "rune of salik", Rune },
	{ "arcane rune", SelfRune },       { "ethereal rune", SelfRune },
	{ "rune of the scale", GroupRune }, { "rune of rikkukin", GroupRune },
	{ "minor shielding", Shield },     { "lesser shielding", Shield },   { "shielding", Shield },
	{ "major shielding", Shield },     { "greater shielding", Shield },  { "arch shielding", Shield },
	{ "shield of the magi", Shield },  { "shield of the arcane", Shield }, { "shield of maelin", Shield },
	{ "mystic shield", Shield },
	{ "berserker strength", Brimstone }, { "bedlam", Brimstone },        { "uproar", Brimstone },
	{ "mayhem", Brimstone },
	{ "ward of alendar", Aegis },      { "guard of alendar", Aegis },    { "protection of alendar", Aegis },
	{ "bulwark of alendar", Aegis },   { "wall of alendar", Aegis },
	{ "circle of alendar", Legion },   { "guard of druzzil", Legion },   { "group resist magic", Legion },
	{ "reflect", Mirror },             { "bounce", Mirror },
	{ "quickness", Haste },            { "alacrity", Haste },            { "celerity", Haste },
	{ "swift like the wind", Haste },  { "aanya's quickening", Haste },  { "wondrous rapidity", Haste },
	{ "augmentation", Haste },         { "augment", Haste },             { "visions of grandeur", Haste },
	{ "speed of vallon", Haste },      { "speed of salik", Haste },
	{ "speed of the brood", GroupHaste }, { "vallon's quickening", GroupHaste },
	{ "hastening of salik", GroupHaste },
	{ "breeze", Clarity },             { "clarity", Clarity },           { "insight", Clarity },
	{ "boon of the clear mind", Clarity }, { "gift of magic", Clarity }, { "gift of insight", Clarity },
	{ "gift of brilliance", Clarity }, { "clairvoyance", Clarity },      { "brilliance", Clarity },
	{ "enlightenment", Clarity },      { "intellectual advancement", Clarity },
	{ "intellectual superiority", Clarity },
	{ "gift of pure thought", Voice }, { "koadic's endless intellect", Voice }, { "tranquility", Voice },
	{ "voice of quellious", Voice },   { "voice of clairvoyance", Voice },
	{ "lull", Lull },                  { "soothe", Lull },               { "calm", Lull },
	{ "pacify", Lull },                { "wake of tranquility", Lull },  { "pacification", Lull },
	{ "placate", Lull },
	{ "mana flare", ManaFlare },
	{ "haunting visage", Visage },     { "calming visage", Visage },     { "beguiling visage", Visage },
	{ "horrifying visage", Visage },   { "glamorous visage", Visage },   { "radiant visage", Visage },
	{ "adorning grace", Visage },      { "overwhelming splendor", Visage },
	{ "trickster's augmentation", Illusion },
	{ "invisibility", Invis },         { "improved invisibility", Invis }, { "suppression field", Invis },
	{ "invisibility versus undead", Invis },
	{ "sympathetic aura", MindAura },  { "beguiler's aura", MindAura },  { "illusionist's aura", MindAura },
	{ "hypnotic stare mindwipe", Stare },
};

bool Contains(std::string_view s, std::string_view part)
{
	return s.find(part) != std::string_view::npos;
}

int BySpell(const eqlib::EQ_Spell* sp, std::string_view base)
{
	const std::string_view extra(sp->Extra, strnlen(sp->Extra, sizeof(sp->Extra)));
	if (StartsWith(extra, "PCPetEnc"))
		return Animation;
	if (StartsWith(extra, "SwarmPetEnc"))
		return Doppelganger;
	if (StartsWith(extra, "SpireDoppelganger"))
		return EnchantedDoppelganger;
	if (StartsWith(extra, "PhantasmalOpponent"))
		return PhantasmalOpponent;

	for (const NamedLine& n : kClassic)
		if (base == n.name)
			return n.line;

	if (StartsWith(base, "illusion: ") || StartsWith(base, "spell: illusion: ") || StartsWith(base, "visage of "))
		return Illusion;
	if (EndsWith(base, "'s animation"))
		return Animation;
	if (EndsWith(base, " rune effect"))
		return HateRuneProc;
	if (StartsWith(base, "obscuring ") || EndsWith(base, " shadows"))
		return Shadows;
	if (EndsWith(base, " effect"))
		return WardProc;
	if (StartsWith(base, "reactive rune"))
		return ReactiveRune;
	if (StartsWith(base, "beguiler's banishment"))
		return Banishment;
	if (StartsWith(base, "enchant ") || StartsWith(base, "mass enchant ") || StartsWith(base, "greater mass enchant ")
		|| StartsWith(base, "imbue ") || EndsWith(base, "empowering essence")
		|| (EndsWith(base, " mana") && (Contains(base, "clarify") || Contains(base, "crystallize")
			|| Contains(base, "distill") || Contains(base, "purify") || Contains(base, "thicken"))))
		return Enchant;
	return -1;
}

} // namespace

const ClassCatalogue& EnchanterCatalogue()
{
	static const ClassCatalogue c{ "Enchanter", eqlib::Enchanter, kLines, 6, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
