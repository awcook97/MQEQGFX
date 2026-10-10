#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

#include <initializer_list>

namespace eqgfx::spellfx {

namespace {

constexpr Pair kVie       { { 0.90f, 0.97f, 1.00f, 0.95f }, { 0.50f, 0.72f, 1.00f, 0.85f } };
constexpr Pair kSanctum   { { 1.00f, 0.97f, 0.88f, 0.95f }, { 0.70f, 0.80f, 1.00f, 0.80f } };
constexpr Pair kMartyr    { { 1.00f, 0.80f, 0.60f, 0.95f }, { 0.75f, 0.05f, 0.12f, 0.85f } };
constexpr Pair kPurge     { { 1.00f, 1.00f, 1.00f, 0.95f }, { 0.70f, 0.45f, 0.95f, 0.75f } };
constexpr Pair kTranquil  { { 0.88f, 0.90f, 1.00f, 0.85f }, { 0.60f, 0.62f, 0.95f, 0.60f } };
constexpr Pair kDawn      { { 1.00f, 0.96f, 0.80f, 0.95f }, { 1.00f, 0.55f, 0.50f, 0.80f } };
constexpr Pair kHallowed  { { 1.00f, 0.90f, 0.55f, 0.95f }, { 0.50f, 0.38f, 0.20f, 0.90f } };
constexpr Pair kAwe       { { 1.00f, 1.00f, 1.00f, 1.00f }, { 1.00f, 0.85f, 0.25f, 0.90f } };
constexpr Pair kSpiritual { { 0.95f, 0.95f, 1.00f, 0.95f }, { 0.62f, 0.50f, 1.00f, 0.85f } };
constexpr Pair kLifespring{ { 0.80f, 1.00f, 0.90f, 0.95f }, { 0.35f, 0.85f, 0.75f, 0.80f } };

LOOK(kCleRemedy, "remedy",
	G(IM, Glyph_Cross, 0.9f, 8.5f, 4, 1, GA_Drop, 0, FIX, kHoly, 1.8f),
	F(IM, FxForm_Flame, 0.75f, 7.5f, 64, 16, 0.1f, 2.4f, 0, FIX, kHealGold, 1.4f, 0.06f, ADD, 1.4f, 0.3f),
	F(IM, FxForm_Sparkle, 1.1f, 7.0f, 40, 20, 0.10f, 1, 0, FIX, kWhite, 1.6f, 0.06f, ADD, 1, 0.3f));

LOOK(kCleLight, "light",
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 2, 0.05f, 0.5f, 1.0f, FIX, kSanctum, 1.4f, 0.40f),
	G(IM, Glyph_Sun, 1.0f, 9.0f, 4, 1, GA_PopRoll, 0, FIX, kGold, 2.2f, 0.2f),
	F(IM, FxForm_Spire, 0.7f, 8.0f, 96, 16, 0.6f, 2.8f, 0, FIX, kHoly, 1.8f, 0.06f, ADD, 1, 0.2f),
	F(IM, FxForm_Sparkle, 1.2f, 10.0f, 48, 24, 0.12f, 1, 0, FIX, kWhite, 2.4f),
	F(IM, FxForm_Bloom, 1.6f, 0.8f, 40, 2, 0, 0, 0, FIX, kSanctum, 1.8f, 0.05f, ADD, 1, 0.3f));

LOOK(kCleElixir, "elixir",
	G(IM, Glyph_Chalice, 0.7f, 8.8f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.6f),
	F(IM, FxForm_Bubbles, 0.9f, 7.0f, 64, 10, 0.18f, 0, 0, FIX, kHealGold, 2.8f, 0.035f),
	Persist(F(AU, FxForm_Helix, 0.95f, 6.5f, 64, 2, 1.2f, 0, 0.4f, FIX, kHealGold, 0, 0.04f, COMET)));

LOOK(kCleGroupElixir, "group elixir",
	G(RE, Glyph_Chalice, 0.55f, 5.5f, 3, 5, GA_RingRise, 2.4f, FIX, kGold, 2.8f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kHealGold, 2.4f, 0.06f, ADD, 1.2f, 0, AE),
	F(IM, FxForm_Bubbles, 0.9f, 6.5f, 48, 8, 0.18f, 0, 0, FIX, kHeal, 2.4f, 0.035f),
	Persist(F(AU, FxForm_Helix, 0.95f, 6.5f, 64, 2, -1.2f, 0, 0.4f, FIX, kHeal, 0, 0.04f, COMET)));

LOOK(kClePromised, "promised",
	G(IM, Glyph_Hourglass, 0.7f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kSanctum, 3.0f),
	F(IM, FxForm_Ring, 1.0f, 7.8f, 40, 0, 1.0f, 1.0f, 0.5f, FIX, kGold, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Flame, 0.8f, 8.5f, 72, 18, 0.1f, 2.8f, 0, FIX, kHoly, 1.6f, 0.07f, ADD, 1, 1.6f),
	F(IM, FxForm_Burst, 2.0f, 0.5f, 72, 2, 1.8f, 16, 0, FIX, kGold, 1.2f, 0.08f, ADD, 1, 1.6f));

LOOK(kCleRenewal, "renewal",
	G(IM, Glyph_Sun, 1.3f, 10.0f, 4, 1, GA_Drop, 0, FIX, kDawn, 2.4f),
	F(IM, FxForm_Flame, 0.9f, 11.0f, 96, 24, 0.1f, 3.4f, 0, FIX, kDawn, 2.2f, 0.09f, ADD, 1, 0.4f),
	F(IM, FxForm_Rain, 1.0f, 12.0f, 40, 30, 0, 0.1f, 0, FIX, kHoly, 2.4f, 0.05f, ADD, 1, 0.4f),
	F(IM, FxForm_Burst, 2.6f, 0.4f, 96, 3, 2.2f, 20, 0, FIX, kHealGold, 1.4f, 0.10f, ADD, 1, 0.4f));

LOOK(kCleEleventh, "eleventh hour",
	F(IM, FxForm_Lightning, 1, 42.0f, 64, 3, 0.2f, 8.0f, 1.0f, FIX, kAwe, 0.7f, 0.14f),
	G(IM, Glyph_Heart, 1.1f, 7.5f, 4, 1, GA_PopRoll, 0, FIX, kDawn, 1.8f, 0.1f),
	F(IM, FxForm_Burst, 2.8f, 3.0f, 96, 3, 3.0f, 24, 0, FIX, kHoly, 1.0f, 0.10f, ADD, 1.6f, 0.1f),
	F(IM, FxForm_Sparkle, 1.4f, 9.0f, 60, 30, 0.14f, 0, 0, FIX, kWhite, 1.6f, 0.06f, ADD, 1, 0.2f));

LOOK(kCleWordCure, "word (heal, cure)",
	G(RE, Glyph_Book, 1.1f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.2f),
	F(RE, FxForm_Bloom, 1.0f, 1.5f, 96, 4, 0, 0, 0, FIX, kHoly, 2.2f, 0.07f, ADD, 1.3f, 0, AE),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.5f, 20, 0, FIX, kGold, 1.4f, 0.09f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Bubbles, 0.8f, 6.0f, 48, 6, 0.18f, 0, 0, FIX, kWater, 1.8f, 0.035f),
	F(IM, FxForm_Sparkle, 1.1f, 7.5f, 32, 16, 0.12f, 1, 0, FIX, kHoly, 1.8f));

LOOK(kCleWord, "word",
	G(RE, Glyph_Dove, 0.6f, 6.0f, 3, 4, GA_RingRise, 2.2f, FIX, kWhite, 2.2f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kHeal, 2.0f, 0.07f, ADD, 1.4f, 0, AE),
	F(IM, FxForm_Flame, 0.7f, 6.5f, 48, 12, 0.1f, 2.0f, 0, FIX, kHealGold, 1.4f, 0.05f));

LOOK(kCleSyllable, "syllable",
	G(RE, Glyph_Cross, 0.35f, 4.0f, 3, 8, GA_RingRise, 2.6f, FIX, kHoly, 2.0f),
	F(RE, FxForm_Burst, 1.0f, 0.3f, 96, 2, 0.8f, 16, 0, FIX, kHeal, 1.2f, 0.08f, ADD, 1.5f, 0, AE),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 24, 12, 0.10f, 1, 0, FIX, kHeal, 1.6f));

LOOK(kCleSplash, "splash",
	G(IM, Glyph_Wave, 1.2f, 8.5f, 4, 1, GA_Drop, 0, FIX, kLifespring, 1.4f),
	F(IM, FxForm_Rain, 1.0f, 10.0f, 60, 50, 0, 0.1f, 0, FIX, kLifespring, 1.6f, 0.05f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Bloom, 1.0f, 0.5f, 96, 3, 0, 0, 0, FIX, kWater, 1.8f, 0.06f, ADD, 1, 0.45f, AE),
	F(IM, FxForm_Sparkle, 0.7f, 7.0f, 60, 30, 0.12f, 1, 0, FIX, kHealGold, 2.0f, 0.06f, ADD, 1, 0.5f, AE));

LOOK(kCleIntervention, "intervention",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.3f, 0.3f, 1.0f, FIX, kHoly, 0.6f, 0.15f),
	F(IM, FxForm_Lightning, 1, 36.0f, 48, 2, 0.4f, 10.0f, 1.0f, FIX, kAwe, 0.7f, 0.11f),
	G(IM, Glyph_Cross, 0.9f, 7.5f, 4, 1, GA_PopRoll, 0, FIX, kGold, 1.6f),
	F(IM, FxForm_Burst, 2.0f, 2.5f, 64, 2, 0.6f, 12, 0, FIX, kHoly, 0.8f, 0.08f));

LOOK(kCleContravention, "contravention",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.4f, 0.4f, 1.4f, FIX, kAwe, 0.7f, 0.16f),
	G(IM, Glyph_Hammer, 0.9f, 7.5f, 4, 1, GA_PopRoll, 0, FIX, kSilver, 1.4f),
	F(IM, FxForm_Burst, 2.2f, 2.0f, 72, 2, 0.5f, 14, 0, FIX, kGold, 0.9f, 0.09f),
	F(IM, FxForm_Drain, 1, 0, 48, 3, 1.0f, 12, 0, FIX, kHeal, 1.8f, 0.08f, ADD, 1, 0.3f));

LOOK(kCleDenunciation, "denunciation",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.35f, 1.2f, FIX, kGold, 0.8f, 0.16f),
	G(IM, Glyph_Sun, 1.0f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kGold, 2.0f),
	F(IM, FxForm_Spire, 0.6f, 3.5f, 96, 16, 2.0f, 3.2f, 0, FIX, kHoly, 1.0f, 0.07f),
	G(IM, Glyph_Cross, 0.3f, 5.0f, 0, 2, GA_Orbit, 1.1f, FIX, kHeal, 1.8f, 0.3f));

LOOK(kCleAcquittal, "acquittal",
	F(RE, FxForm_Rain, 1.0f, 11.0f, 60, 50, 0, 0.12f, 0, FIX, kPurge, 1.8f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Chalice, 1.0f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kWhite, 2.2f),
	F(RE, FxForm_Bloom, 1.0f, 0.8f, 96, 3, 0, 0, 0, FIX, kHeal, 2.0f, 0.06f, ADD, 1, 0.3f, AE),
	F(IM, FxForm_Bubbles, 0.8f, 6.0f, 48, 6, 0.18f, 0, 0, FIX, kPurge, 2.0f, 0.035f));

LOOK(kCleIssuance, "issuance",
	F(IM, FxForm_Rune, 1.0f, 0, 96, 8, 3, 24, 0, FIX, kHealGold, 5.0f, 0.05f, ADD, 0.6f, 0, AE),
	G(IM, Glyph_Lantern, 1.1f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 5.0f),
	F(IM, FxForm_Sparkle, 0.8f, 6.0f, 80, 40, 0.12f, 0, 0, FIX, kHeal, 5.0f, 0.06f, ADD, 1, 0, AE),
	F(IM, FxForm_Bloom, 0.8f, 0.5f, 64, 2, 0, 0, 0, FIX, kHealGold, 5.0f, 0.05f, ADD, 0.5f, 0, AE));

LOOK(kCleGround, "ground",
	F(RE, FxForm_Rune, 0.8f, 0, 96, 6, 1, 18, 0, FIX, kHallowed, 4.0f, 0.06f, ADD, 1, 0, AE),
	G(RE, Glyph_Cross, 1.6f, 0.1f, 2, 1, GlyphAnim_Pop, 0, FIX, kGold, 4.0f),
	F(RE, FxForm_Flame, 0.8f, 1.2f, 128, 32, 0.1f, 0.5f, 0, FIX, kHolyFire, 3.5f, 0.05f, ADD, 1, 0.3f, AE),
	F(RE, FxForm_Sparkle, 0.8f, 3.0f, 60, 30, 0.10f, 0, 0, FIX, kGold, 3.5f, 0.05f, ADD, 1, 0.3f, AE));

LOOK(kCleAlliance, "alliance",
	G(IM, Glyph_Hand, 0.6f, 8.0f, 3, 2, GA_PopSpin, 0.6f, FIX, kGold, 3.0f),
	F(IM, FxForm_Rain, 1.6f, 11.0f, 40, 40, 0, 0.1f, 0, FIX, kHeal, 3.0f, 0.05f),
	F(IM, FxForm_Ring, 1.6f, 10.5f, 48, 0, 0.8f, 1.0f, 0.3f, FIX, kHealGold, 3.0f, 0.05f, COMET));

LOOK(kCleBlessing, "blessing (heal, hit points)",
	G(IM, Glyph_Heart, 1.0f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kDawn, 2.6f),
	F(IM, FxForm_Helix, 1.1f, 7.0f, 96, 2, 1.5f, 0, 0.6f, FIX, kHealGold, 2.6f, 0.05f, COMET),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kDawn, 2.0f, 0.06f, ADD, 1, 0.3f, AE));

LOOK(kCleBlessingProc, "blessing trigger",
	G(IM, Glyph_Heart, 0.35f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kDawn, 1.4f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 20, 10, 0.10f, 0, 0, FIX, kHealGold, 1.2f));

LOOK(kCleResponse, "divine response",
	G(IM, Glyph_Eye, 0.7f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kSanctum, 3.0f),
	F(IM, FxForm_Ring, 0.6f, 8.0f, 40, 0, 1.2f, 1.0f, 0.4f, FIX, kGold, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Lattice, 1.2f, 6.0f, 64, 8, 0.4f, 0, 0, FIX, kSanctum, 2.4f, 0.04f));

LOOK(kCleIntercession, "intercession",
	G(IM, Glyph_Ankh, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Wings, 1.0f, 4.3f, 58, 6, 0.4f, 2.8f, 0, FIX, kFeather, 2.6f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 7.5f, 32, 16, 0.12f, 1, 0, FIX, kGold, 2.4f));

LOOK(kCleAnticipated, "anticipated",
	G(IM, Glyph_Shield, 0.8f, 5.0f, 4, 1, GA_PopBob, 0, FIX, kVie, 2.8f),
	G(IM, Glyph_Cross, 0.35f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kHeal, 2.8f, 0.3f),
	F(IM, FxForm_Ring, 1.3f, 5.0f, 40, 0, 1.0f, 1.0f, 0.5f, FIX, kHeal, 2.8f, 0.05f, COMET));

LOOK(kCleBenediction, "benediction",
	G(IM, Glyph_Book, 0.8f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kMana, 2.6f),
	F(IM, FxForm_Orbit, 1.2f, 6.5f, 60, 12, 0, 0, 0, FIX, kMana, 2.6f, 0.04f, ADD, 3.0f),
	F(IM, FxForm_Sparkle, 1.0f, 7.0f, 32, 16, 0.10f, 1, 0, FIX, kSanctum, 2.2f));

LOOK(kCleHandHaste, "hand (spell haste)",
	G(RE, Glyph_Book, 0.55f, 5.5f, 3, 5, GA_RingRise, 2.4f, FIX, kMana, 2.6f),
	F(RE, FxForm_Orbit, 1.0f, 6.0f, 96, 16, 0, 0, 0, FIX, kMana, 2.6f, 0.04f, ADD, 3.0f, 0, AE),
	F(RE, FxForm_Burst, 1.0f, 0.3f, 72, 2, 0.8f, 12, 0, FIX, kSanctum, 1.2f, 0.07f, ADD, 1, 0.2f, AE));

LOOK(kCleSymbol, "symbol",
	F(IM, FxForm_Rune, 1.2f, 4.0f, 80, 8, 3, 24, 1, FIX, kGold, 3.0f, 0.05f),
	G(IM, Glyph_Sun, 1.4f, 0.0f, 2, 1, GlyphAnim_Pop, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Sparkle, 1.1f, 7.5f, 40, 20, 0.12f, 1, 0, FIX, kHoly, 2.6f));

LOOK(kCleUnifiedSymbol, "unified symbol",
	F(RE, FxForm_Rune, 1.0f, 0, 96, 8, 3, 24, 0, FIX, kGold, 3.0f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Sun, 0.6f, 5.5f, 3, 6, GA_RingRise, 2.6f, FIX, kGold, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 64, 2.5f, 3.0f, 0, 1.0f, FIX, kHoly, 2.0f, 0.05f, COMET));

LOOK(kCleAegolism, "aegolism",
	G(IM, Glyph_Shield, 1.0f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kGold, 2.6f),
	F(IM, FxForm_Lattice, 1.3f, 6.5f, 72, 8, 0.4f, 0, 0, FIX, kHoly, 2.6f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kGold, 2.4f));

LOOK(kCleGroupAegolism, "group aegolism",
	G(RE, Glyph_Shield, 0.6f, 5.0f, 3, 6, GA_RingRise, 2.6f, FIX, kGold, 3.0f),
	F(RE, FxForm_Lattice, 1.0f, 7.0f, 120, 24, 0.3f, 0, 0, FIX, kHoly, 2.6f, 0.07f, ADD, 1, 0, AE),
	F(IM, FxForm_Ring, 1.2f, 0.4f, 40, 0, 1.0f, 1.0f, 0.5f, FIX, kGold, 2.4f, 0.05f, COMET));

LOOK(kCleUnity, "unity",
	G(IM, Glyph_Crown, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 96, 2, 1.2f, 0, 0.4f, FIX, kGold, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Helix, 1.2f, 6.5f, 96, 2, -1.2f, 0, 0.4f, FIX, kVie, 3.0f, 0.05f, COMET));

LOOK(kCleUnified, "unified",
	G(IM, Glyph_Shield, 0.8f, 6.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.8f),
	G(IM, Glyph_Sun, 0.5f, 8.5f, 4, 1, GA_PopRoll, 0, FIX, kHoly, 2.8f, 0.2f),
	F(IM, FxForm_Ring, 1.3f, 0.3f, 48, 0, 1.0f, 1.0f, 0.5f, FIX, kGold, 2.8f, 0.05f, COMET));

LOOK(kCleWardAC, "ward (armor class)",
	F(IM, FxForm_Dome, 1.7f, 6.8f, 96, 8, 3, 0, 0, FIX, kSilver, 2.4f, 0.03f),
	G(IM, Glyph_Tower, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kSilver, 2.4f));

LOOK(kCleVie, "vie",
	F(IM, FxForm_Dome, 1.8f, 7.0f, 112, 8, 3, 0, 0, FIX, kVie, 2.6f, 0.04f),
	F(IM, FxForm_Rune, 1.8f, 0.2f, 70, 6, 1, 18, 0, FIX, kVie, 2.6f, 0.05f),
	F(IM, FxForm_Crystal, 1.0f, 5.5f, 48, 8, 0, 0.5f, 0, FIX, kVie, 2.4f, 0.05f),
	Persist(F(AU, FxForm_Ring, 1.2f, 0.4f, 40, 0, 0.6f, 1.0f, 0.3f, FIX, kVie, 0, 0.04f, COMET)));

LOOK(kCleOrder, "order (group vie)",
	G(RE, Glyph_Shield, 0.5f, 4.5f, 3, 6, GA_RingRise, 2.4f, FIX, kVie, 2.8f),
	F(RE, FxForm_Lattice, 1.0f, 6.0f, 120, 24, 0.4f, 0, 0, FIX, kVie, 2.6f, 0.07f, ADD, 1, 0, AE),
	F(IM, FxForm_Crystal, 1.0f, 5.0f, 48, 8, 0, 0.5f, 0, FIX, kVie, 2.2f, 0.05f));

LOOK(kCleRalliedVie, "rallied vie",
	F(RE, FxForm_Dome, 1.0f, 8.0f, 128, 10, 4, 0, 0, FIX, kVie, 3.0f, 0.06f, ADD, 1, 0, AE),
	F(RE, FxForm_Rune, 1.0f, 0, 96, 8, 3, 24, 0, FIX, kVie, 3.0f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Shield, 1.1f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kSanctum, 2.6f),
	Persist(F(AU, FxForm_Ring, 1.2f, 0.4f, 40, 0, -0.6f, 1.0f, 0.3f, FIX, kVie, 0, 0.04f, COMET)));

LOOK(kCleShining, "shining",
	F(IM, FxForm_Crystal, 1.1f, 5.8f, 72, 12, 0, 0.8f, 0, FIX, kSilver, 2.8f, 0.06f),
	F(IM, FxForm_Spire, 1.0f, 5.0f, 96, 16, 1.2f, 2.0f, 0, FIX, kHoly, 1.6f, 0.05f, ADD, 1, 0.3f),
	G(IM, Glyph_Shield, 0.9f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kSilver, 2.4f));

LOOK(kCleWardAbsorb, "ward (absorb)",
	G(IM, Glyph_Shield, 0.45f, 4.0f, 0, 3, GA_Orbit, 1.3f, FIX, kVie, 3.0f),
	F(IM, FxForm_Ring, 1.2f, 4.0f, 48, 0, 1.4f, 1.0f, 0.6f, FIX, kHeal, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 24, 12, 0.10f, 0, 0, FIX, kVie, 2.4f));

LOOK(kCleArmor, "armor",
	G(IM, Glyph_Helm, 0.9f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kSanctum, 2.6f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.9f, 0.6f, 0, FIX, kSanctum, 2.6f, 0.05f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kMana, 2.4f));

LOOK(kCleYaulp, "yaulp",
	G(RE, Glyph_Mouth, 0.9f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 1.6f),
	F(RE, FxForm_Bloom, 2.2f, 5.0f, 48, 3, 0, 0, 0, FIX, kZeal, 1.4f, 0.06f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.9f, 0.9f, 0, FIX, kZeal, 2.2f, 0.07f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kMana, 2.0f));

LOOK(kCleVow, "vow",
	G(IM, Glyph_Hammer, 0.5f, 4.0f, 3, 3, GA_Orbit, 1.4f, FIX, kGold, 3.0f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.9f, 0.8f, 0, FIX, kHolyFire, 3.0f, 0.07f),
	F(IM, FxForm_Burst, 2.0f, 1.0f, 48, 1, 1.0f, 12, 0, FIX, kGold, 0.9f, 0.08f));

LOOK(kCleVowWrath, "vow (wrath)",
	G(IM, Glyph_Fist, 0.9f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kZeal, 2.6f),
	G(IM, Glyph_Hammer, 0.45f, 3.5f, 3, 2, GA_Orbit, 1.3f, FIX, kSilver, 3.0f),
	F(IM, FxForm_Burst, 2.2f, 0.3f, 64, 2, 0.3f, 14, 0, FIX, kZeal, 0.9f, 0.09f));

LOOK(kCleWardRetort, "ward (retribution)",
	G(IM, Glyph_Shield, 0.9f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.6f),
	F(IM, FxForm_Spire, 1.2f, 3.0f, 96, 24, 2.0f, 1.5f, 0, FIX, kHolyFire, 1.6f, 0.05f),
	F(IM, FxForm_Orbit, 1.4f, 5.5f, 48, 10, 0, 0, 0, FIX, kGold, 2.6f, 0.04f, ADD, 2.5f));

LOOK(kCleResistCorruption, "resist corruption",
	F(IM, FxForm_Dome, 1.6f, 6.6f, 88, 6, 2, 0, 0, FIX, kPurge, 2.2f, 0.03f),
	G(IM, Glyph_Shield, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kPurge, 2.4f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 24, 12, 0.10f, 1, 0, FIX, kWhite, 2.0f));

LOOK(kCleDivinity, "divinity aura",
	Persist(F(AU, FxForm_Rune, 1.0f, 0, 96, 8, 3, 24, 0, FIX, kGold, 0, 0.04f, ADD, 0.4f, 0, AE)),
	G(RE, Glyph_Sun, 1.3f, 9.0f, 4, 1, GA_PopRoll, 0, FIX, kGold, 2.6f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kHoly, 2.2f, 0.06f, ADD, 1, 0.2f, AE));

LOOK(kCleReverentAura, "reverent aura",
	Persist(F(AU, FxForm_Rune, 1.0f, 0, 96, 6, 1, 18, 0, FIX, kVie, 0, 0.04f, ADD, 0.4f, 0, AE)),
	G(RE, Glyph_Shield, 0.5f, 3.5f, 3, 6, GA_RingRise, 2.4f, FIX, kVie, 2.6f),
	F(RE, FxForm_Dome, 1.0f, 6.0f, 112, 8, 3, 0, 0, FIX, kVie, 2.2f, 0.04f, ADD, 1, 0.2f, AE));

LOOK(kCleDivine, "divine",
	F(IM, FxForm_Dome, 1.9f, 7.5f, 128, 10, 4, 0, 0, FIX, kGold, 3.5f, 0.06f),
	G(IM, Glyph_Sun, 0.9f, 8.8f, 4, 1, GA_PopRoll, 0, FIX, kGold, 3.5f),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 96, 24, 0.95f, 0.7f, 0, FIX, kHoly, 3.0f, 0.06f));

LOOK(kCleInfusion, "infusion",
	G(IM, Glyph_Hand, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kHealGold, 2.6f),
	G(IM, Glyph_Hammer, 0.35f, 3.5f, 0, 2, GA_Orbit, 1.2f, FIX, kGold, 3.0f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.10f, 0, 0, FIX, kHeal, 2.6f));

LOOK(kCleHandInfusion, "hand of infusion",
	G(RE, Glyph_Hand, 0.6f, 5.0f, 3, 6, GA_RingRise, 2.4f, FIX, kHealGold, 2.8f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.2f, 16, 0, FIX, kGold, 1.2f, 0.08f, ADD, 1, 0, AE),
	F(IM, FxForm_Orbit, 1.2f, 4.5f, 40, 8, 0, 0, 0, FIX, kHeal, 2.4f, 0.04f, ADD, 2.0f));

LOOK(kCleMark, "mark",
	F(IM, FxForm_Rune, 1.0f, 8.5f, 60, 6, 1, 12, 1, FIX, kGold, 4.0f, 0.05f),
	G(IM, Glyph_Cross, 0.5f, 8.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kHeal, 4.0f),
	F(IM, FxForm_Burst, 1.6f, 4.0f, 48, 1, 0.3f, 8, 0, FIX, kGold, 0.8f, 0.07f));

LOOK(kCleMarkTohan, "mark of tohan",
	G(IM, Glyph_Crown, 0.8f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kGhost, 4.0f),
	F(IM, FxForm_Ring, 1.3f, 0.3f, 40, 0, 0.6f, 1.0f, 0.3f, FIX, kGold, 4.0f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 24, 12, 0.12f, 1, 0, FIX, kGhost, 3.0f));

LOOK(kCleBlood, "blood of the martyr",
	G(IM, Glyph_Drop, 0.35f, 7.5f, 4, 5, GA_Rain, 1.3f, FIX, kMartyr, 2.4f),
	F(IM, FxForm_Rune, 1.4f, 0, 60, 6, 1, 12, 0, FIX, kMartyr, 4.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 24, 12, 0.10f, 1, 0, FIX, kGold, 2.4f));

LOOK(kCleRetort, "retort",
	G(IM, Glyph_Scales, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Orbit, 1.3f, 6.0f, 48, 10, 0, 0, 0, FIX, kHealGold, 3.0f, 0.04f, ADD, 2.0f),
	F(IM, FxForm_Ring, 1.3f, 0.4f, 40, 0, 1.0f, 1.0f, 0.5f, FIX, kGold, 3.0f, 0.05f, COMET));

LOOK(kCleRetortProc, "retort trigger",
	G(IM, Glyph_Scales, 0.4f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kGold, 1.2f),
	F(IM, FxForm_Burst, 1.2f, 3.5f, 32, 1, 0, 8, 0, FIX, kHealGold, 0.5f, 0.06f));

LOOK(kCleCorruptionCure, "corruption cure",
	G(RE, Glyph_Drop, 0.8f, 8.5f, 4, 1, GA_PopRoll, 0, FIX, kPurge, 2.0f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 2, 2.0f, 16, 0, FIX, kPurge, 1.4f, 0.09f, ADD, 1, 0.1f, AE),
	F(IM, FxForm_Drain, 1, 0, 32, 3, 0.8f, 8, 0, FIX, kCorrupt, 1.4f, 0.06f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kWhite, 1.8f, 0.06f, ADD, 1, 0.3f));

LOOK(kCleBloodCure, "cleansed blood",
	G(IM, Glyph_Drop, 0.7f, 8.5f, 4, 1, GA_Drop, 0, FIX, kMartyr, 1.8f),
	F(IM, FxForm_Bloom, 1.4f, 1.0f, 40, 3, 0, 0, 0, FIX, kWhite, 1.8f, 0.05f, ADD, 1, 0.35f),
	F(IM, FxForm_Bubbles, 0.8f, 6.0f, 48, 6, 0.18f, 0, 0, FIX, kPurge, 1.8f, 0.035f, ADD, 1, 0.35f));

LOOK(kCleGlow, "glow",
	G(IM, Glyph_Dove, 0.8f, 7.5f, 4, 1, GA_PopAscend, 0, FIX, kTranquil, 2.6f),
	F(IM, FxForm_Bloom, 1.8f, 3.0f, 40, 2, 0, 0, 0, FIX, kTranquil, 2.4f, 0.04f),
	F(IM, FxForm_Flame, 0.9f, 5.0f, 64, 16, 0.9f, 0.6f, 0, FIX, kTranquil, 2.4f, 0.04f));

LOOK(kCleAbsolution, "absolution",
	G(IM, Glyph_Spiral, 0.8f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kFeather, 2.2f),
	F(IM, FxForm_Bloom, 2.4f, 4.0f, 48, 3, 0, 0, 0, FIX, kFeather, 2.0f, 0.05f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Sparkle, 1.2f, 8.0f, 24, 12, 0.12f, 1, 0, FIX, kTranquil, 2.0f));

LOOK(kCleFoolFallen, "invisibility to undead",
	G(RE, Glyph_Skull, 0.8f, 7.5f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 2.2f, 0, OVER),
	F(RE, FxForm_Dome, 1.8f, 6.5f, 96, 6, 2, 0, 0, FIX, kLight, 2.4f, 0.03f, ADD, 1, 0, AE),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kLight, 2.4f, 0.05f));

LOOK(kCleProvidence, "providence",
	F(IM, FxForm_Dome, 1.8f, 7.0f, 112, 8, 3, 0, 0, FIX, kPrism, 2.6f, 0.04f),
	G(IM, Glyph_Eye, 0.6f, 8.2f, 4, 1, GA_PopBob, 0, FIX, kSilver, 2.6f),
	F(IM, FxForm_Ring, 1.8f, 3.5f, 48, 0, 1.6f, 1.0f, 0.6f, FIX, kPrism, 2.6f, 0.05f, COMET));

LOOK(kCleAquifer, "blessed water",
	G(IM, Glyph_Drop, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kTear, 2.4f),
	F(IM, FxForm_Bubbles, 0.9f, 7.0f, 48, 8, 0.2f, 0, 0, FIX, kWater, 2.4f, 0.035f),
	F(IM, FxForm_Sparkle, 1.0f, 8.0f, 24, 12, 0.12f, 1, 0, FIX, kHoly, 2.2f));

LOOK(kCleShackle, "shackle",
	F(IM, FxForm_Cracks, 2.2f, 0, 48, 6, 1.0f, 0, 0, FIX, kHallowed, 4.0f, 0.06f, OVER),
	G(IM, Glyph_Shackle, 0.6f, 0.8f, 3, 2, GlyphAnim_Pop, 0.45f, FIX, kStone, 4.0f, 0, OVER),
	F(AU, FxForm_Vines, 1.0f, 2.0f, 120, 6, 0.4f, 0, 1.0f, FIX, kHallowed, 5.0f, 0.07f));

LOOK(kCleRez, "resurrection",
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 2, 0.05f, 0.5f, 1.0f, FIX, kHoly, 2.0f, 0.30f),
	G(IM, Glyph_Ankh, 1.4f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 4.0f, 0.4f),
	F(IM, FxForm_Wings, 1.0f, 4.4f, 70, 10, 0.3f, 4.2f, 0, FIX, kFeather, 4.0f, 0.06f, ADD, 1, 0.4f),
	F(IM, FxForm_Sparkle, 1.4f, 10.0f, 60, 30, 0.14f, 1, 0, FIX, kHoly, 4.0f));

LOOK(kCleReproach, "reproach",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.3f, 0.3f, 0.8f, FIX, kHoly, 0.6f, 0.16f),
	G(IM, Glyph_Sun, 0.8f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kGold, 1.4f),
	F(IM, FxForm_Burst, 2.0f, 3.0f, 64, 2, 0.5f, 14, 0, FIX, kHolyFire, 0.8f, 0.09f));

LOOK(kCleChroma, "chroma",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.4f, 1.0f, FIX, kPrism, 0.7f, 0.18f),
	G(IM, Glyph_Gem, 0.9f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kPrism, 1.6f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 72, 2, 0.4f, 16, 0, FIX, kPrism, 0.9f, 0.09f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 40, 20, 0.12f, 0, 0, FIX, kAurora, 1.4f));

LOOK(kCleAwe, "awe",
	F(IM, FxForm_Lightning, 1, 36.0f, 48, 2, 0.6f, 12.0f, 1.0f, FIX, kAwe, 0.6f, 0.12f),
	F(IM, FxForm_Burst, 1.8f, 3.5f, 48, 1, 0.3f, 12, 0, FIX, kAwe, 0.6f, 0.08f),
	G(IM, Glyph_Cross, 0.3f, 5.0f, 0, 1, GA_PopAscend, 0, FIX, kHeal, 1.4f, 0.3f),
	Stars(0.15f));

LOOK(kCleSound, "sound",
	G(RE, Glyph_Horn, 0.8f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 1.4f),
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.6f, 0.3f, 0.6f, FIX, kSanctum, 0.6f, 0.14f),
	F(IM, FxForm_Bloom, 1.6f, 4.0f, 48, 4, 0, 0, 0, FIX, kSanctum, 1.0f, 0.06f, ADD, 1.6f),
	Stars(0.2f));

LOOK(kCleSilent, "silent decree",
	G(RE, Glyph_Book, 1.1f, 9.0f, 4, 1, GA_PopRoll, 0, FIX, kGold, 1.8f),
	F(RE, FxForm_Rune, 1.0f, 0, 96, 8, 3, 24, 0, FIX, kGold, 1.6f, 0.05f, ADD, 1, 0, AE),
	F(RE, FxForm_Burst, 1.0f, 1.0f, 120, 3, 1.0f, 24, 0, FIX, kAwe, 1.2f, 0.10f, ADD, 1, 0.2f, AE),
	Stars(0.3f));

LOOK(kCleTectonic, "tectonic",
	F(RE, FxForm_Cracks, 1.0f, 0, 96, 10, 1.4f, 0, 0, FIX, kHallowed, 2.4f, 0.08f, OVER, 1, 0, AE),
	F(RE, FxForm_Crystal, 1.0f, 3.0f, 72, 12, 1, 0.9f, 0, FIX, kStone, 2.2f, 0.07f, OVER, 1, 0.1f, AE),
	F(RE, FxForm_Burst, 1.0f, 0.2f, 96, 2, 0.3f, 0, 0, FIX, kGold, 1.2f, 0.10f, ADD, 1, 0.1f, AE),
	G(IM, Glyph_Rock, 0.4f, 1.0f, 3, 4, GlyphAnim_Rise | GlyphAnim_Spin | GlyphAnim_Bob, 1.2f, FIX, kStone, 1.8f, 0, OVER));

LOOK(kCleUndead, "undead nuke",
	G(IM, Glyph_Skull, 0.9f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kBone, 1.6f, 0, OVER),
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 3, 0.4f, 12.0f, 1.0f, FIX, kSanctum, 0.9f, 0.14f, ADD, 1, 0.2f),
	F(IM, FxForm_Flame, 0.9f, 12.0f, 96, 24, 0.1f, 3.6f, 0, FIX, kHoly, 1.6f, 0.09f, ADD, 1, 0.2f),
	F(IM, FxForm_Burst, 2.6f, 0.5f, 96, 2, 1.0f, 20, 0, FIX, kGold, 1.1f, 0.10f, ADD, 1, 0.2f));

LOOK(kCleUndeadFear, "undead fear",
	G(IM, Glyph_Skull, 0.8f, 7.0f, 4, 1, GlyphAnim_Pop | GlyphAnim_Wiggle, 0, FIX, kBone, 2.0f, 0, OVER),
	F(IM, FxForm_Bloom, 2.0f, 4.0f, 48, 3, 0, 0, 0, FIX, kHoly, 1.4f, 0.06f, ADD, 1.5f),
	G(IM, Glyph_Mouth, 0.6f, 9.0f, 4, 1, GA_PopAscend, 0, FIX, kGold, 1.4f));

LOOK(kCleSermon, "sermon",
	G(IM, Glyph_Book, 0.9f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.4f),
	F(IM, FxForm_Flame, 0.9f, 7.0f, 80, 20, 0.2f, 1.6f, 0, FIX, kHolyFire, 2.4f, 0.07f, ADD, 1, 0.2f),
	F(IM, FxForm_Rain, 0.9f, 10.0f, 30, 30, 0, 0.1f, 0, FIX, kHoly, 2.0f, 0.05f));

LOOK(kCleHammerPet, "spiritual hammer",
	G(RE, Glyph_Hammer, 1.4f, 7.0f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop | GlyphAnim_Roll, 0, FIX, kSpiritual, 3.0f),
	F(RE, FxForm_Rune, 2.2f, 0, 70, 6, 1, 12, 0, FIX, kSpiritual, 3.0f, 0.05f),
	F(RE, FxForm_Vortex, 1.2f, 6.0f, 72, 4, 1.2f, 2.0f, 4.0f, FIX, kSpiritual, 2.4f, 0.06f),
	F(RE, FxForm_Sparkle, 1.6f, 8.0f, 48, 24, 0.12f, 1, 0, FIX, kWhite, 2.6f));

LOOK(kCleHammerSmite, "hammer smite",
	G(IM, Glyph_Hammer, 1.3f, 9.0f, 4, 1, GA_Drop, 0, FIX, kSpiritual, 1.6f),
	F(IM, FxForm_Lightning, 1, 38.0f, 48, 2, 0.4f, 10.0f, 1.0f, FIX, kSpiritual, 0.8f, 0.12f, ADD, 1, 0.35f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kGold, 1.6f, 0.06f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.6f, 0.4f, 96, 3, 0.6f, 20, 0, FIX, kHoly, 1.0f, 0.11f, ADD, 1, 0.35f));

LOOK(kAaBeaconLife, "beacon of life",
	G(RE, Glyph_Lantern, 1.3f, 9.5f, 4, 1, GA_PopBob, 0, FIX, kHealGold, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 2.0f, 24, 0, FIX, kHeal, 1.6f, 0.12f, ADD, 1, 0.2f, AE),
	F(RE, FxForm_Spire, 0.1f, 1.0f, 96, 16, 0.5f, 6.0f, 0, FIX, kHealGold, 1.6f, 0.07f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Sparkle, 1.2f, 8.0f, 40, 20, 0.12f, 1, 0, FIX, kHeal, 2.2f));

LOOK(kAaBlessedChains, "blessed chains",
	G(IM, Glyph_Link, 0.4f, 1.0f, 3, 8, GA_PopSpin, 1.2f, FIX, kGold, 4.0f),
	G(IM, Glyph_Shackle, 0.7f, 7.0f, 4, 1, GA_Drop, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Ring, 1.2f, 1.0f, 48, 0, 0.3f, 1.0f, 0, FIX, kGold, 4.0f, 0.05f));

LOOK(kAaBlessingRes, "blessing of resurrection",
	G(IM, Glyph_Ankh, 1.2f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.5f, 0.3f),
	F(IM, FxForm_Flame, 0.9f, 10.0f, 96, 24, 0.15f, 3.0f, 0, FIX, kDawn, 3.5f, 0.08f),
	F(IM, FxForm_Rune, 1.8f, 0, 70, 8, 3, 16, 0, FIX, kGold, 3.5f, 0.05f));

LOOK(kAaDivineRes, "divine resurrection",
	F(IM, FxForm_Lightning, 1, 44.0f, 80, 3, 0.05f, 0.5f, 1.0f, FIX, kAwe, 2.4f, 0.40f),
	G(IM, Glyph_Ankh, 1.6f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 4.5f, 0.3f),
	F(IM, FxForm_Wings, 1.0f, 4.4f, 84, 12, 0.3f, 5.0f, 0, FIX, kFeather, 4.5f, 0.07f, ADD, 1, 0.3f),
	F(IM, FxForm_Burst, 3.4f, 0.4f, 120, 3, 2.0f, 24, 0, FIX, kGold, 1.6f, 0.12f, ADD, 1, 0.3f),
	F(IM, FxForm_Sparkle, 1.6f, 12.0f, 80, 40, 0.14f, 1, 0, FIX, kHoly, 4.0f));

LOOK(kAaSanctuaryBlessing, "blessing of sanctuary",
	G(IM, Glyph_Door, 0.9f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kTranquil, 3.0f),
	F(IM, FxForm_Dome, 1.7f, 6.8f, 96, 8, 3, 0, 0, FIX, kTranquil, 3.0f, 0.04f),
	F(IM, FxForm_Bloom, 1.8f, 3.0f, 40, 2, 0, 0, 0, FIX, kFeather, 2.4f, 0.04f, ADD | FxFlag_Reverse));

LOOK(kAaSanctuary, "sanctuary",
	F(RE, FxForm_Dome, 1.9f, 7.2f, 112, 8, 3, 0, 0, FIX, kTranquil, 3.0f, 0.05f),
	G(RE, Glyph_Dove, 0.5f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kWhite, 3.0f),
	F(RE, FxForm_Sparkle, 1.4f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kTranquil, 2.4f));

LOOK(kAaBurstLife, "burst of life",
	G(IM, Glyph_Heart, 1.2f, 8.5f, 4, 1, GA_PopRoll, 0, FIX, kHeal, 1.8f),
	F(IM, FxForm_Burst, 2.8f, 2.0f, 96, 3, 3.0f, 24, 0, FIX, kHeal, 1.0f, 0.10f, ADD, 1.6f),
	F(IM, FxForm_Flame, 0.8f, 9.0f, 72, 18, 0.1f, 2.8f, 0, FIX, kHealGold, 1.6f, 0.07f, ADD, 1.4f, 0.1f));

LOOK(kAaCallHerald, "call of the herald",
	G(RE, Glyph_Horn, 1.0f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.0f),
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.2f, 16, 0, FIX, kGhost, 1.8f, 0.08f),
	G(IM, Glyph_Ankh, 0.8f, 6.0f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 2.0f));

LOOK(kAaCelestialHammer, "celestial hammer",
	G(IM, Glyph_Hammer, 1.8f, 11.0f, 4, 1, GA_Drop, 0, FIX, kSanctum, 1.8f),
	F(IM, FxForm_Lightning, 1, 44.0f, 64, 3, 0.5f, 12.0f, 1.0f, FIX, kAwe, 0.9f, 0.14f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 3.2f, 0.4f, 120, 3, 0.8f, 24, 0, FIX, kGold, 1.2f, 0.12f, ADD, 1, 0.35f),
	F(IM, FxForm_Cracks, 2.6f, 0, 64, 8, 1.2f, 0, 0, FIX, kHallowed, 2.0f, 0.07f, OVER, 1, 0.35f));

LOOK(kAaRapidity, "celestial rapidity",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 12, 3.0f, 0, 0, FIX, kMana, 2.6f, 0.05f),
	G(IM, Glyph_Book, 0.6f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kMana, 2.2f),
	F(IM, FxForm_Orbit, 1.2f, 6.0f, 60, 12, 0, 0, 0, FIX, kSanctum, 2.6f, 0.04f, ADD, 4.0f));

LOOK(kAaCelestialRegen, "celestial regeneration",
	G(RE, Glyph_Leaf, 0.45f, 5.0f, 3, 6, GA_RingRise, 2.4f, FIX, kLifespring, 3.0f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kLifespring, 2.6f, 0.06f, ADD, 1, 0, AE),
	Persist(F(AU, FxForm_Helix, 1.0f, 6.5f, 64, 2, 1.0f, 0, 0.3f, FIX, kLifespring, 0, 0.04f, COMET)));

LOOK(kAaFocusedRegen, "focused celestial regeneration",
	G(IM, Glyph_Leaf, 0.35f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kLifespring, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 7.0f, 96, 3, 1.4f, 0, 0.6f, FIX, kLifespring, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.2f, 7.5f, 40, 20, 0.12f, 1, 0, FIX, kHeal, 2.6f));

LOOK(kAaChanneling, "channeling the divine",
	G(RE, Glyph_Sun, 1.1f, 9.5f, 4, 1, GA_PopRoll, 0, FIX, kGold, 3.0f),
	F(RE, FxForm_Vortex, 1.2f, 9.0f, 96, 5, 1.2f, 2.5f, 4.0f, FIX, kSanctum, 3.0f, 0.06f),
	F(RE, FxForm_Sparkle, 1.6f, 9.0f, 60, 30, 0.12f, 2, 0, FIX, kHoly, 3.0f));

LOOK(kAaArbitration, "divine arbitration",
	G(RE, Glyph_Scales, 1.3f, 9.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.6f),
	F(RE, FxForm_Drain, 1, 0, 48, 4, 1.0f, 16, 0, FIX, kHealGold, 1.6f, 0.07f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 1.5f, 20, 0, FIX, kHoly, 1.4f, 0.10f, ADD, 1, 0.2f, AE),
	F(RE, FxForm_Ring, 1.0f, 8.5f, 64, 0, 1.2f, 1.0f, 0.4f, FIX, kGold, 2.6f, 0.05f, COMET, 1, 0, AE));

LOOK(kAaAvatar, "divine avatar",
	F(IM, FxForm_Wings, 1.0f, 4.4f, 84, 12, 0.3f, 5.0f, 0, FIX, kHoly, 3.5f, 0.07f),
	F(IM, FxForm_Flame, 1.1f, 7.0f, 112, 28, 0.8f, 1.2f, 0, FIX, kHolyFire, 3.5f, 0.09f),
	G(IM, Glyph_Crown, 0.8f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.5f),
	F(IM, FxForm_Burst, 3.0f, 0.3f, 96, 2, 0.6f, 20, 0, FIX, kGold, 1.0f, 0.12f));

LOOK(kAaGuardian, "divine guardian",
	F(IM, FxForm_Wings, 1.0f, 4.4f, 70, 10, 0.3f, 4.2f, 0, FIX, kGhost, 3.5f, 0.06f),
	G(IM, Glyph_Spirit, 1.2f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGhost, 3.5f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kHoly, 3.0f, 0.04f));

LOOK(kAaPeace, "divine peace",
	G(RE, Glyph_Dove, 1.1f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kWhite, 3.0f),
	F(RE, FxForm_Dome, 1.8f, 7.0f, 96, 6, 2, 0, 0, FIX, kTranquil, 3.0f, 0.04f),
	F(RE, FxForm_Bloom, 2.4f, 2.0f, 48, 3, 0, 0, 0, FIX, kFeather, 2.4f, 0.05f, ADD | FxFlag_Reverse));

LOOK(kAaRetribution, "divine retribution",
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 2, 0.5f, 12.0f, 1.0f, FIX, kHolyFire, 0.8f, 0.12f),
	G(IM, Glyph_Fist, 0.8f, 7.5f, 4, 1, GA_PopRoll, 0, FIX, kGold, 1.4f),
	F(IM, FxForm_Burst, 2.2f, 2.0f, 64, 2, 0.4f, 14, 0, FIX, kHolyFire, 0.8f, 0.09f),
	Stars(0.2f));

LOOK(kAaExquisite, "exquisite benediction",
	G(RE, Glyph_Chalice, 1.3f, 9.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(RE, FxForm_Rune, 1.0f, 0, 96, 8, 3, 24, 0, FIX, kHealGold, 3.0f, 0.05f, ADD, 1, 0, AE),
	F(RE, FxForm_Rain, 1.0f, 12.0f, 60, 50, 0, 0.1f, 0, FIX, kHealGold, 3.0f, 0.05f, ADD, 1, 0.3f, AE),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kHeal, 2.4f));

LOOK(kAaFlurryLife, "flurry of life",
	G(IM, Glyph_Cross, 0.35f, 5.0f, 0, 5, GA_Orbit | GlyphAnim_Ascend, 1.3f, FIX, kHeal, 2.4f),
	F(IM, FxForm_Orbit, 1.3f, 6.5f, 72, 14, 0, 0, 0, FIX, kHeal, 2.6f, 0.04f, ADD, 4.0f),
	F(IM, FxForm_Burst, 2.2f, 2.0f, 64, 2, 1.5f, 14, 0, FIX, kHealGold, 0.9f, 0.08f));

LOOK(kAaHealingFrenzy, "healing frenzy",
	F(IM, FxForm_Flame, 1.1f, 6.5f, 112, 28, 0.8f, 1.2f, 0, FIX, kHealGold, 3.0f, 0.08f),
	G(IM, Glyph_Cross, 0.9f, 8.2f, 4, 1, GA_PopRoll, 0, FIX, kHeal, 2.6f),
	F(IM, FxForm_Wind, 1.2f, 4.5f, 60, 10, 2.5f, 0, 0, FIX, kHeal, 2.4f));

LOOK(kAaPurifySoul, "purify soul",
	G(IM, Glyph_Spirit, 0.9f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kPurge, 2.4f),
	F(IM, FxForm_Vortex, 1.0f, 7.0f, 72, 4, 1.2f, 2.0f, -4.0f, FIX, kPurge, 2.0f, 0.06f),
	F(IM, FxForm_Sparkle, 1.2f, 8.0f, 40, 20, 0.12f, 0, 0, FIX, kWhite, 2.0f));

LOOK(kAaGroupPurify, "group purify soul",
	G(RE, Glyph_Spirit, 0.5f, 5.0f, 3, 6, GA_RingRise, 2.4f, FIX, kPurge, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 2.5f, 24, 0, FIX, kPurge, 1.6f, 0.10f, ADD, 1, 0.1f, AE),
	F(RE, FxForm_Sparkle, 0.8f, 8.0f, 80, 40, 0.12f, 0, 0, FIX, kWhite, 2.2f, 0.06f, ADD, 1, 0.3f, AE));

LOOK(kAaQuietPrayer, "quiet prayer",
	G(IM, Glyph_Hand, 0.7f, 8.0f, 4, 2, GA_PopBob, 0.25f, FIX, kMana, 2.8f),
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.0f, 12, 0, FIX, kMana, 2.0f, 0.07f),
	F(IM, FxForm_Bubbles, 0.9f, 6.5f, 48, 8, 0.18f, 0, 0, FIX, kMana, 2.6f, 0.035f));

LOOK(kAaRepelWicked, "repel the wicked",
	F(RE, FxForm_Burst, 1.0f, 1.0f, 120, 3, 0.5f, 24, 0, FIX, kAwe, 1.0f, 0.12f, ADD, 1.8f, 0, AE),
	F(RE, FxForm_Bloom, 1.0f, 3.0f, 96, 3, 0, 0, 0, FIX, kHoly, 1.2f, 0.07f, ADD, 1.8f, 0, AE),
	G(RE, Glyph_Hand, 1.2f, 7.5f, 4, 1, GA_PopRoll, 0, FIX, kGold, 1.6f));

LOOK(kAaSaintsUnity, "saint's unity",
	G(IM, Glyph_Crown, 1.0f, 8.6f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	G(IM, Glyph_Sun, 0.35f, 4.5f, 0, 4, GA_Orbit, 1.4f, FIX, kHoly, 3.0f),
	F(IM, FxForm_Rune, 1.8f, 0, 80, 8, 3, 16, 0, FIX, kGold, 3.0f, 0.05f));

LOOK(kAaSanctifiedBlessing, "sanctified blessing",
	G(IM, Glyph_Heart, 0.9f, 8.4f, 4, 1, GA_PopAscend, 0, FIX, kGold, 2.6f),
	F(IM, FxForm_Rain, 0.8f, 10.0f, 30, 24, 0, 0.1f, 0, FIX, kGold, 2.4f, 0.05f),
	F(IM, FxForm_Ring, 1.1f, 7.5f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kHealGold, 2.6f, 0.05f, COMET));

LOOK(kAaSilentCasting, "silent casting",
	G(IM, Glyph_Mouth, 0.7f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 2.4f),
	F(IM, FxForm_Ring, 1.0f, 6.5f, 40, 0, 0.8f, 1.0f, 0.3f, FIX, kTranquil, 2.4f, 0.04f, COMET),
	F(IM, FxForm_Sparkle, 1.0f, 7.0f, 24, 12, 0.10f, 1, 0, FIX, kTranquil, 2.0f));

LOOK(kAaSpireVicar, "spire of the vicar",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kSanctum, 2.6f, 0.10f),
	G(RE, Glyph_Ankh, 1.2f, 9.5f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kHoly, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaTurnUndead, "turn undead",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.3f, 1.0f, FIX, kHoly, 0.7f, 0.16f),
	G(IM, Glyph_Skull, 0.9f, 7.0f, 4, 1, GA_PopRoll, 0, FIX, kBone, 2.0f, 0, OVER),
	F(IM, FxForm_Flame, 0.9f, 7.0f, 80, 20, 0.1f, 2.0f, 0, FIX, kHolyFire, 2.4f, 0.07f, ADD, 1, 0.2f));

LOOK(kAaVeturika, "veturika's perseverance",
	F(IM, FxForm_Dome, 1.9f, 7.5f, 128, 10, 4, 0, 0, FIX, kVie, 3.0f, 0.06f),
	G(IM, Glyph_Tower, 1.0f, 8.4f, 4, 1, GA_PopBob, 0, FIX, kVie, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 72, 2, 1.0f, 0, 0.4f, FIX, kHeal, 3.0f, 0.05f, COMET));

LOOK(kAaWardPurity, "ward of purity",
	G(IM, Glyph_Shield, 0.8f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kPurge, 3.0f),
	F(IM, FxForm_Lattice, 1.3f, 6.0f, 64, 6, 0.5f, 0, 0, FIX, kPurge, 2.8f, 0.05f),
	F(IM, FxForm_Bubbles, 0.9f, 6.0f, 48, 6, 0.18f, 0, 0, FIX, kWhite, 2.4f, 0.035f));

LOOK(kAaHolyStep, "holy step",
	F(RE, FxForm_Wind, 1.4f, 4.0f, 72, 12, 3.0f, 0, 0, FIX, kHoly, 1.4f),
	G(RE, Glyph_Feather, 0.6f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kGold, 1.6f),
	F(RE, FxForm_Bloom, 1.8f, 0.5f, 40, 2, 0, 0, 0, FIX, kGold, 1.2f, 0.05f));

LOOK(kAaTouchDivine, "touch of the divine",
	G(IM, Glyph_Hand, 1.0f, 9.0f, 4, 1, GA_Drop, 0, FIX, kAwe, 2.0f),
	F(IM, FxForm_Flame, 0.8f, 10.0f, 80, 20, 0.1f, 3.0f, 0, FIX, kHoly, 2.0f, 0.08f, ADD, 1.4f, 0.3f),
	F(IM, FxForm_Wings, 1.0f, 4.3f, 58, 6, 0.5f, 2.6f, 0, FIX, kFeather, 2.2f, 0.05f, ADD, 1, 0.3f));

LOOK(kAaBattleFrenzy, "battle frenzy",
	G(IM, Glyph_Hammer, 0.8f, 7.6f, 4, 2, GA_Orbit, 1.1f, FIX, kZeal, 2.6f),
	F(IM, FxForm_Flame, 1.0f, 6.5f, 80, 18, 0.6f, 1.2f, 0, FIX, kHolyFire, 2.4f, 0.08f),
	F(IM, FxForm_Burst, 2.0f, 0.5f, 64, 2, 0.6f, 12, 0, FIX, kZeal, 1.0f, 0.09f));

enum Line : int
{
	Remedy, Light, Elixir, GroupElixir, Promised, Renewal, Eleventh, WordCure, Word, Syllable,
	Splash, Intervention, Contravention, Denunciation, Acquittal, Issuance, Ground, Alliance,
	Blessing, BlessingProc, Response, Intercession, Anticipated, Benediction, HandHaste, Symbol,
	UnifiedSymbol, Aegolism, GroupAegolism, Unity, Unified, WardAC, Vie, Order, RalliedVie, Shining,
	WardAbsorb, Armor, Yaulp, Vow, VowWrath, WardRetort, ResistCorruption, Divinity, ReverentAura,
	Divine, Infusion, HandInfusion, Mark, MarkTohan, Blood, Retort, RetortProc, CorruptionCure,
	BloodCure, Glow, Absolution, FoolFallen, Providence, Aquifer, Shackle, Rez, Reproach, Chroma,
	Awe, Sound, Silent, Tectonic, Undead, UndeadFear, Sermon, HammerPet, HammerSmite,
	BeaconLife, BlessedChains, BlessingRes, DivineRes, SanctuaryBlessing, Sanctuary, BurstLife,
	CallHerald, CelestialHammer, Rapidity, CelestialRegen, FocusedRegen, Channeling, Arbitration,
	Avatar, Guardian, Peace, Retribution, Exquisite, FlurryLife, HealingFrenzy, PurifySoul,
	GroupPurify, QuietPrayer, RepelWicked, SaintsUnity, SanctifiedBlessing, SilentCasting,
	SpireVicar, TurnUndead, Veturika, WardPurity, HolyStep, TouchDivine,
	BattleFrenzy,
};

const LineLook kLines[] = {
	{ "Remedy (fast heal)",              &kCleRemedy },
	{ "Light (single heal)",             &kCleLight },
	{ "Elixir (heal over time)",         &kCleElixir },
	{ "Elixir (group heal over time)",   &kCleGroupElixir },
	{ "Promised (delayed heal)",         &kClePromised },
	{ "Renewal (heal, cure)",            &kCleRenewal },
	{ "Eleventh-Hour (emergency heal)",  &kCleEleventh },
	{ "Word (group heal, cure)",         &kCleWordCure },
	{ "Word (group heal)",               &kCleWord },
	{ "Syllable (group heal)",           &kCleSyllable },
	{ "Splash (area heal)",              &kCleSplash },
	{ "Intervention (heal, smite)",      &kCleIntervention },
	{ "Contravention (smite, heal)",     &kCleContravention },
	{ "Denunciation (nuke, twinheal)",   &kCleDenunciation },
	{ "Acquittal (group cure, heal)",    &kCleAcquittal },
	{ "Issuance (healing ward)",         &kCleIssuance },
	{ "Ground (hallowed ground)",        &kCleGround },
	{ "Alliance (healing rain)",         &kCleAlliance },
	{ "Blessing (heal, hit points)",     &kCleBlessing },
	{ "Blessing / Lifespring trigger",   &kCleBlessingProc },
	{ "Divine Response (group heal)",    &kCleResponse },
	{ "Intercession (death save)",       &kCleIntercession },
	{ "Anticipated (heal ward)",         &kCleAnticipated },
	{ "Benediction (spell haste)",       &kCleBenediction },
	{ "Hand (group spell haste)",        &kCleHandHaste },
	{ "Symbol (hit points)",             &kCleSymbol },
	{ "Unified Symbol (group)",          &kCleUnifiedSymbol },
	{ "Aegolism (hit points, AC)",       &kCleAegolism },
	{ "Aegolism (group)",                &kCleGroupAegolism },
	{ "Unity (combined buffs)",          &kCleUnity },
	{ "Unified (combined buffs)",        &kCleUnified },
	{ "Ward (armor class)",              &kCleWardAC },
	{ "Vie (absorb)",                    &kCleVie },
	{ "Order (group absorb)",            &kCleOrder },
	{ "Rallied Vie (group absorb)",      &kCleRalliedVie },
	{ "Shining (absorb, lash)",          &kCleShining },
	{ "Ward (absorb, heal)",             &kCleWardAbsorb },
	{ "Armor (self)",                    &kCleArmor },
	{ "Yaulp (self)",                    &kCleYaulp },
	{ "Vow (melee proc)",                &kCleVow },
	{ "Vow (wrath proc)",                &kCleVowWrath },
	{ "Ward (retribution)",              &kCleWardRetort },
	{ "Resist Corruption",               &kCleResistCorruption },
	{ "Divinity (aura)",                 &kCleDivinity },
	{ "Reverent (aura)",                 &kCleReverentAura },
	{ "Divine (invulnerability)",        &kCleDivine },
	{ "Infusion (heal proc)",            &kCleInfusion },
	{ "Hand of Infusion (group)",        &kCleHandInfusion },
	{ "Mark (heal on strike)",           &kCleMark },
	{ "Mark of Tohan",                   &kCleMarkTohan },
	{ "Blood (martyr's mark)",           &kCleBlood },
	{ "Retort (heal proc)",              &kCleRetort },
	{ "Retort trigger",                  &kCleRetortProc },
	{ "Corruption (cure)",               &kCleCorruptionCure },
	{ "Blood (cure)",                    &kCleBloodCure },
	{ "Glow (pacify)",                   &kCleGlow },
	{ "Absolution (memory blur)",        &kCleAbsolution },
	{ "Invisibility to undead",          &kCleFoolFallen },
	{ "Providence (reflect)",            &kCleProvidence },
	{ "Blessed Aquifer",                 &kCleAquifer },
	{ "Shackle (root)",                  &kCleShackle },
	{ "Resurrection",                    &kCleRez },
	{ "Reproach (magic nuke)",           &kCleReproach },
	{ "Chroma (nuke)",                   &kCleChroma },
	{ "Awe (stun, heal)",                &kCleAwe },
	{ "Sound (stun, heal)",              &kCleSound },
	{ "Silent (area stun)",              &kCleSilent },
	{ "Tectonic (area nuke)",            &kCleTectonic },
	{ "Undead (undead nuke)",            &kCleUndead },
	{ "Undead fear",                     &kCleUndeadFear },
	{ "Sermon (undead DoT)",             &kCleSermon },
	{ "Hammer of Zeal (pet)",            &kCleHammerPet },
	{ "Hammer (smiting pet)",            &kCleHammerSmite },

	{ "AA: Beacon of Life",              &kAaBeaconLife },
	{ "AA: Blessed Chains",              &kAaBlessedChains },
	{ "AA: Blessing of Resurrection",    &kAaBlessingRes },
	{ "AA: Divine Resurrection",         &kAaDivineRes },
	{ "AA: Blessing of Sanctuary",       &kAaSanctuaryBlessing },
	{ "AA: Sanctuary",                   &kAaSanctuary },
	{ "AA: Burst of Life",               &kAaBurstLife },
	{ "AA: Call of the Herald",          &kAaCallHerald },
	{ "AA: Celestial Hammer",            &kAaCelestialHammer },
	{ "AA: Celestial Rapidity",          &kAaRapidity },
	{ "AA: Celestial Regeneration",      &kAaCelestialRegen },
	{ "AA: Focused Celestial Regen",     &kAaFocusedRegen },
	{ "AA: Channeling the Divine",       &kAaChanneling },
	{ "AA: Divine Arbitration",          &kAaArbitration },
	{ "AA: Divine Avatar",               &kAaAvatar },
	{ "AA: Divine Guardian",             &kAaGuardian },
	{ "AA: Divine Peace",                &kAaPeace },
	{ "AA: Divine Retribution",          &kAaRetribution },
	{ "AA: Exquisite Benediction",       &kAaExquisite },
	{ "AA: Flurry of Life",              &kAaFlurryLife },
	{ "AA: Healing Frenzy",              &kAaHealingFrenzy },
	{ "AA: Purify Soul",                 &kAaPurifySoul },
	{ "AA: Group Purify Soul",           &kAaGroupPurify },
	{ "AA: Quiet Prayer",                &kAaQuietPrayer },
	{ "AA: Repel the Wicked",            &kAaRepelWicked },
	{ "AA: Saint's Unity",               &kAaSaintsUnity },
	{ "AA: Sanctified Blessing",         &kAaSanctifiedBlessing },
	{ "AA: Silent Casting",              &kAaSilentCasting },
	{ "AA: Spire of the Vicar",          &kAaSpireVicar },
	{ "AA: Turn Undead",                 &kAaTurnUndead },
	{ "AA: Veturika's Perseverance",     &kAaVeturika },
	{ "AA: Ward of Purity",              &kAaWardPurity },
	{ "AA: Holy Step",                   &kAaHolyStep },
	{ "AA: Touch of the Divine",         &kAaTouchDivine },
	{ "AA: Battle Frenzy",  &kAaBattleFrenzy },
};

const GroupLine kNumbered[] = {
	{ 0, Benediction },    { 1, Remedy },        { 2, Symbol },         { 3, WardAC },       { 4, Awe },
	{ 6, Aegolism },       { 8, Elixir },        { 9, Reproach },       { 10, Vie },         { 11, Light },
	{ 12, Sound },         { 13, Order },        { 14, Promised },      { 15, Vow },         { 17, Mark },
	{ 18, Tectonic },      { 19, HandHaste },    { 20, Yaulp },         { 21, WardRetort },  { 23, CorruptionCure },
	{ 24, Glow },          { 25, Silent },       { 26, Armor },         { 27, UnifiedSymbol }, { 28, GroupAegolism },
	{ 29, GroupElixir },   { 30, RalliedVie },   { 31, HammerPet },     { 32, Divine },      { 33, Blood },
	{ 34, Ground },        { 35, Divinity },     { 36, Eleventh },      { 38, Chroma },      { 39, Sermon },
	{ 40, Renewal },       { 42, WordCure },     { 44, Aquifer },       { 46, BloodCure },   { 47, Retort },
	{ 48, Intervention },  { 49, Denunciation }, { 50, Contravention }, { 51, Undead },      { 52, Absolution },
	{ 53, Word },          { 54, BloodCure },    { 55, Shining },       { 56, Splash },      { 57, Intercession },
	{ 58, Infusion },      { 59, MarkTohan },    { 60, Issuance },      { 61, VowWrath },    { 63, HandInfusion },
	{ 64, Unity },         { 65, Unified },      { 66, Shackle },       { 67, WardAbsorb },  { 68, ReverentAura },
	{ 69, Anticipated },   { 70, Syllable },     { 71, Acquittal },     { 72, HammerSmite }, { 74, Blessing },
	{ 75, BlessingProc },  { 76, BlessingProc }, { 77, Response },      { 78, Alliance },
};

const GroupLine kModern[] = {
	{ 10, Benediction },   { 20, Remedy },       { 30, Symbol },        { 40, WardAC },      { 50, Awe },
	{ 60, ResistCorruption }, { 70, Aegolism },  { 90, Elixir },        { 100, Reproach },   { 110, Vie },
	{ 120, Light },        { 130, Sound },       { 140, Order },        { 150, Promised },   { 160, Vow },
	{ 170, Mark },         { 180, Tectonic },    { 190, HandHaste },    { 200, Yaulp },      { 210, WardRetort },
	{ 230, CorruptionCure }, { 240, Glow },      { 250, Silent },       { 260, Armor },      { 270, UnifiedSymbol },
	{ 280, GroupAegolism }, { 290, GroupElixir }, { 300, RalliedVie },  { 310, HammerPet },  { 320, Divine },
	{ 340, Ground },       { 360, Eleventh },    { 370, Chroma },       { 380, Sermon },     { 390, Renewal },
	{ 410, WordCure },     { 450, BloodCure },   { 460, Retort },       { 470, Intervention }, { 480, Denunciation },
	{ 490, Contravention }, { 500, Undead },     { 510, Absolution },   { 520, Word },       { 540, Shining },
	{ 550, Splash },       { 560, Intercession }, { 590, Issuance },    { 600, VowWrath },   { 610, HandInfusion },
	{ 620, Unity },        { 630, Unified },     { 640, WardAbsorb },   { 660, Anticipated }, { 670, Syllable },
	{ 680, Acquittal },    { 690, HammerSmite }, { 710, Blessing },     { 711, BlessingProc }, { 712, BlessingProc },
	{ 730, Response },     { 740, Alliance },    { 900, Remedy },       { 910, GroupAegolism },
};

const GroupLine kOther[] = {
	{ 1002, Remedy },      { 1003, Symbol },     { 1004, WardAC },      { 1005, Awe },       { 1007, ResistCorruption },
	{ 1009, Aegolism },    { 1010, Benediction }, { 1011, Elixir },     { 1012, Reproach },  { 1013, Vie },
	{ 1014, Light },       { 1015, Sound },      { 1017, Promised },    { 1019, Vow },       { 1021, Mark },
	{ 1022, Tectonic },    { 1023, HandHaste },  { 1024, Yaulp },       { 1025, WardRetort }, { 1026, CorruptionCure },
	{ 1027, CorruptionCure }, { 1028, Glow },    { 1029, Silent },      { 1030, Armor },     { 1031, UnifiedSymbol },
	{ 1032, GroupAegolism }, { 1033, GroupElixir }, { 1035, RalliedVie }, { 1037, HammerPet }, { 1038, Divine },
	{ 1039, Ground },      { 1040, Blood },      { 5257, Aegolism },    { 5259, Elixir },    { 5260, Reproach },
	{ 5265, Light },       { 5269, Mark },       { 18007, Renewal },    { 101546, Intercession }, { 2402068, ReverentAura },
	{ 4105, ResistCorruption }, { 4205, ResistCorruption }, { 4305, ResistCorruption }, { 4405, ResistCorruption },
	{ 4505, ResistCorruption }, { 4705, ResistCorruption }, { 4905, ResistCorruption },
	{ 4141, FoolFallen },  { 4241, FoolFallen }, { 4243, Providence },
};

const NamedLine kAbilities[] = {
	{ "beacon of life",                       BeaconLife },
	{ "blessed chains",                       BlessedChains },
	{ "blessing of resurrection",             BlessingRes },
	{ "divine resurrection",                  DivineRes },
	{ "blessing of sanctuary",                SanctuaryBlessing },
	{ "sanctuary",                            Sanctuary },
	{ "burst of life",                        BurstLife },
	{ "call of the herald",                   CallHerald },
	{ "celestial hammer",                     CelestialHammer },
	{ "celestial rapidity",                   Rapidity },
	{ "celestial regeneration",               CelestialRegen },
	{ "focused celestial regeneration",       FocusedRegen },
	{ "channeling the divine",                Channeling },
	{ "divine arbitration",                   Arbitration },
	{ "divine avatar",                        Avatar },
	{ "divine guardian spirit",               Guardian },
	{ "divine peace",                         Peace },
	{ "divine retribution",                   Retribution },
	{ "divine retribution strike",            Retribution },
	{ "exquisite benediction",                Exquisite },
	{ "flurry of life",                       FlurryLife },
	{ "healing frenzy",                       HealingFrenzy },
	{ "purify soul",                          PurifySoul },
	{ "purify soul cleansing",                PurifySoul },
	{ "group purify soul",                    GroupPurify },
	{ "group purify soul cleansing",          GroupPurify },
	{ "quiet prayer",                         QuietPrayer },
	{ "repel the wicked",                     RepelWicked },
	{ "saint's unity",                        SaintsUnity },
	{ "sanctified blessing",                  SanctifiedBlessing },
	{ "silent casting",                       SilentCasting },
	{ "spire of the vicar",                   SpireVicar },
	{ "apex of the vicar",                    SpireVicar },
	{ "turn undead",                          TurnUndead },
	{ "turn undead strike",                   TurnUndead },
	{ "veturika's perseverance",              Veturika },
	{ "ward of purity",                       WardPurity },
	{ "holy step",                            HolyStep },
	{ "touch of the divine",                  TouchDivine },
	{ "invisibility versus undead",           FoolFallen },
	{ "perfected invisibility versus undead", FoolFallen },
	{ "battle frenzy",  BattleFrenzy },
	{ "yaulping",       Yaulp },
};

bool Contains(std::string_view s, std::string_view part)
{
	return s.find(part) != std::string_view::npos;
}

bool AnyOf(std::string_view base, std::initializer_list<std::string_view> names)
{
	for (std::string_view n : names)
		if (base == n)
			return true;
	return false;
}

int BySpell(const eqlib::EQ_Spell*, std::string_view base)
{
	if (EndsWith(base, " retort effect") || EndsWith(base, " channeled mark"))
		return RetortProc;
	if (base == "yaulp")
		return Yaulp;
	if (base == "root")
		return BlessedChains;
	if (base == "reviviscence" || EndsWith(base, " reviviscence")
		|| AnyOf(base, { "resurrection", "revive", "reanimation", "reconstitution", "restoration", "resuscitate", "reparation" }))
		return Rez;
	if (AnyOf(base, { "sunskin", "sun cloak", "fool the fallen", "distract the departed" }))
		return FoolFallen;
	if (StartsWith(base, "symbol of "))
		return Symbol;
	if (EndsWith(base, "'s mark"))
		return UnifiedSymbol;
	if (StartsWith(base, "mark of "))
		return Mark;
	if (EndsWith(base, " of vie"))
		return Vie;
	if (base == "remedy" || EndsWith(base, " remedy"))
		return Remedy;
	if (EndsWith(base, " light"))
		return Light;
	if (StartsWith(base, "elixir of ") || base == "ethereal elixir")
		return GroupElixir;
	if (EndsWith(base, " elixir"))
		return Elixir;
	if (AnyOf(base, { "word divine", "the unspoken word", "stun command", "the silent command", "silent dictation" }))
		return Silent;
	if (StartsWith(base, "word of "))
		return WordCure;
	if (AnyOf(base, { "blessing of aegolism", "ancient: gift of aegolism", "heroic bond", "hand of virtue",
		"hand of conviction" }))
		return GroupAegolism;
	if (AnyOf(base, { "aegolism", "virtue", "faith", "confidence", "heroism", "fortitude", "courage", "center",
		"daring", "bravery", "valor", "resolution" }))
		return Aegolism;
	if (AnyOf(base, { "blessing of reverence", "blessing of devotion", "blessing of faith" }))
		return Benediction;
	if (AnyOf(base, { "aura of reverence", "aura of devotion" }))
		return HandHaste;
	if (AnyOf(base, { "aura of the pious", "aura of the zealot" }))
		return ReverentAura;
	if (AnyOf(base, { "ward of retribution", "ward of vengeance", "ward of rebuke", "ward of the divine" }))
		return WardRetort;
	if (AnyOf(base, { "ward of gallantry", "ward of valiance", "bulwark of faith", "aegis", "guard", "armor of faith",
		"shield of words", "holy armor", "spirit armor" }))
		return WardAC;
	if (StartsWith(base, "armor of the ") || base == "blessed armor of the risen"
		|| base == "ancient: high priest's bulwark")
		return Armor;
	if (StartsWith(base, "sermon of ") || base == "epitaph of life")
		return Sermon;
	if (StartsWith(base, "sound of "))
		return Sound;
	if (AnyOf(base, { "enforced reverence", "shock of wonder", "tarnation" }))
		return Awe;
	if (AnyOf(base, { "upheaval", "catastrophe", "calamity", "earthquake" }))
		return Tectonic;
	if (AnyOf(base, { "deistic voice", "deistic bellow", "deistic howl", "cower the dead", "death's despair",
		"revulsion of death", "trepidation", "dread of night", "spook the dead", "rebuke the dead" }))
		return UndeadFear;
	if (EndsWith(base, " undead") || base == "death's regret" || base == "banishment of shadows")
		return Undead;
	if (base == "chromastrike")
		return Chroma;
	if (AnyOf(base, { "reckoning", "judgment", "condemnation", "order", "ancient: chaos censure",
		"ancient: pious conscience", "strike", "smite", "wrath" }))
		return Reproach;
	if (StartsWith(base, "unswerving hammer"))
		return HammerPet;
	if (StartsWith(base, "hammer of "))
		return HammerSmite;
	if (AnyOf(base, { "pacification", "placate", "wake of tranquility", "calm", "soothe", "lull" }))
		return Glow;
	if (Contains(base, "immobilize") || EndsWith(base, "ing earth"))
		return Shackle;
	if (AnyOf(base, { "pure blood", "puratus", "antidote", "remove greater curse" }) || StartsWith(base, "eradicate "))
		return BloodCure;
	if (base == "death pact" || base == "divine intervention")
		return Intercession;
	if (base == "shining defense")
		return Shining;
	return -1;
}

} // namespace

const ClassCatalogue& ClericCatalogue()
{
	static const ClassCatalogue c{ "Cleric", eqlib::Cleric, kLines, 4, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
