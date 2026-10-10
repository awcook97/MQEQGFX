#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

namespace eqgfx::spellfx {

namespace {

LOOK(kAaActOfValor, "act of valor",
	G(RE, Glyph_Heart, 1.0f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kBlood, 2.4f),
	F(RE, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.0f, 2.0f, 3.0f, FIX, kBlood, 1.8f, 0.05f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 3, 2.0f, 20, 0, FIX, kGold, 1.6f, 0.10f, ADD, 1, 0.4f, AE),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kHealGold, 2.0f));

LOOK(kAaEnmity, "ageless enmity",
	F(RE, FxForm_Bloom, 2.0f, 4.0f, 48, 3, 0, 0, 0, FIX, kBlood, 1.4f, 0.06f),
	G(IM, Glyph_Eye, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kBlood, 2.5f),
	F(IM, FxForm_Burst, 2.4f, 0.5f, 72, 2, 0.8f, 16, 0, FIX, kBlood, 1.0f, 0.10f));

LOOK(kAaArmorExperience, "armor of experience",
	G(IM, Glyph_Helm, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kIron, 3.0f),
	F(IM, FxForm_Crystal, 1.0f, 5.5f, 60, 12, 0, 0.7f, 0, FIX, kIron, 3.0f, 0.06f, OVER),
	F(IM, FxForm_Ring, 1.3f, 3.0f, 48, 0, 0.8f, 1.0f, 0.4f, FIX, kSilver, 3.0f, 0.05f, COMET));

LOOK(kAaInquisitorArmor, "armor of the inquisitor",
	G(IM, Glyph_Scales, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kRoyal, 2.6f, 0.04f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kGold, 2.4f));

LOOK(kAaAureateBane, "aureate's bane",
	G(IM, Glyph_Crown, 0.9f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kGold, 2.4f),
	F(IM, FxForm_Bloom, 2.0f, 2.5f, 48, 3, 0, 0, 0, FIX, kShadow, 1.6f, 0.05f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Burst, 2.2f, 1.5f, 64, 2, 1.5f, 14, 0, FIX, kGold, 1.0f, 0.09f, ADD, 1, 0.3f));

LOOK(kAaBalefire, "balefire burst",
	F(RE, FxForm_Vortex, 1.6f, 7.0f, 96, 5, 1.2f, 2.5f, 5.0f, FIX, kFlame2, 1.6f, 0.07f),
	F(RE, FxForm_Flame, 1.2f, 7.0f, 96, 24, 0.5f, 1.5f, 0, FIX, kFire, 1.6f, 0.09f),
	F(RE, FxForm_Burst, 3.0f, 0.5f, 96, 3, 1.0f, 16, 0, FIX, kFire, 1.0f, 0.12f));

LOOK(kAaBreath, "dragon breath",
	F(RE, FxForm_Flame, 0.6f, 7.0f, 48, 12, 0.2f, 0.8f, 0, PRI, kFire, 1.2f, 0.06f),
	F(TR, FxForm_Bolt, 1, 0, 128, 6, 0.8f, 0.8f, 0.5f, PRI, kFire, 0.9f, 0.30f),
	F(IM, FxForm_Burst, 2.6f, 2.0f, 96, 3, 1.5f, 16, 0, PRI, kFire, 1.0f, 0.12f),
	F(IM, FxForm_Sparkle, 1.4f, 6.0f, 40, 20, 0.12f, 0, 0, PRI, kFire, 1.6f));

LOOK(kAaJester, "chaotic jester",
	G(RE, Glyph_Mask, 1.0f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kPrism, 3.0f),
	F(RE, FxForm_Sparkle, 2.0f, 6.0f, 60, 30, 0.14f, 2, 0, FIX, kPrism, 3.0f),
	F(RE, FxForm_Burst, 2.5f, 1.0f, 72, 2, 1.0f, 12, 0, FIX, kPrism, 1.2f, 0.08f));

LOOK(kAaRecovery, "expedient recovery",
	G(RE, Glyph_Ankh, 1.1f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(RE, FxForm_Vortex, 1.6f, 6.0f, 72, 4, 1.2f, 2.0f, -3.0f, FIX, kGold, 3.0f, 0.05f));

LOOK(kAaRejuvenation, "forceful rejuvenation",
	G(RE, Glyph_Hourglass, 1.0f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kGold, 2.4f),
	F(RE, FxForm_Ring, 1.3f, 4.0f, 48, 0, -2.0f, 1.0f, 1.0f, FIX, kGold, 2.2f, 0.06f, COMET),
	F(RE, FxForm_Sparkle, 1.6f, 6.0f, 40, 20, 0.12f, 2, 0, FIX, kGold, 2.2f));

LOOK(kAaFireworks, "fireworks",
	F(RE, FxForm_Spire, 0.2f, 12.0f, 96, 24, 0.2f, 14.0f, 0, FIX, kPrism, 1.4f, 0.07f),
	F(RE, FxForm_Spire, 0.2f, 15.0f, 96, 24, -0.2f, 12.0f, 0, FIX, kStar, 1.4f, 0.07f, ADD, 1, 0.5f),
	F(RE, FxForm_Spire, 0.2f, 10.0f, 96, 20, 0.3f, 10.0f, 0, FIX, kPink, 1.4f, 0.07f, ADD, 1, 1.0f),
	F(RE, FxForm_Sparkle, 2.5f, 15.0f, 80, 40, 0.14f, 1, 0, FIX, kPrism, 3.0f, 0.06f, ADD, 1, 0.3f));

LOOK(kAaPortal, "portal",
	G(RE, Glyph_Door, 1.3f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 4.0f),
	F(RE, FxForm_Vortex, 2.0f, 10.0f, 120, 6, 0.6f, 3.0f, 5.0f, FIX, kGold, 4.0f, 0.07f),
	F(RE, FxForm_Sparkle, 1.8f, 10.0f, 50, 25, 0.12f, 0, 0, FIX, kGold, 4.0f));

LOOK(kAaThrone, "throne of heroes",
	G(RE, Glyph_Crown, 1.2f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 4.0f),
	G(RE, Glyph_Door, 1.3f, 5.5f, 4, 1, GA_PopBob, 0, FIX, kRoyal, 4.0f),
	F(RE, FxForm_Vortex, 2.0f, 10.0f, 120, 6, 0.6f, 3.0f, 5.0f, FIX, kRoyal, 4.0f, 0.07f));

LOOK(kAaInfusion, "infusion of the faithful",
	G(IM, Glyph_Crown, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 7.0f, 96, 3, 2.0f, 0, 0.6f, FIX, kGold, 2.6f, 0.06f, COMET),
	F(IM, FxForm_Helix, 1.1f, 7.0f, 96, 3, -2.0f, 0, 0.6f, FIX, kHoly, 2.6f, 0.05f, COMET),
	F(IM, FxForm_Sparkle, 1.3f, 8.0f, 40, 20, 0.12f, 0, 0, FIX, kGold, 2.6f));

LOOK(kAaIntensity, "intensity of the resolute",
	G(IM, Glyph_Fist, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kZeal, 3.0f),
	F(IM, FxForm_Flame, 1.05f, 6.0f, 96, 24, 0.8f, 1.0f, 0, FIX, kZeal, 3.0f, 0.09f),
	F(IM, FxForm_Burst, 2.4f, 0.4f, 64, 2, 0.4f, 12, 0, FIX, kZeal, 0.9f, 0.10f));

LOOK(kAaLessonDevoted, "lesson of the devoted",
	G(IM, Glyph_Book, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Sparkle, 1.5f, 7.0f, 40, 20, 0.12f, 2, 0, FIX, kGold, 3.0f),
	F(IM, FxForm_Ring, 1.2f, 3.0f, 40, 0, 1.0f, 1.0f, 0.4f, FIX, kGold, 3.0f, 0.05f, COMET));

LOOK(kAaMassGroupBuff, "mass group buff",
	G(RE, Glyph_Crown, 1.2f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.6f),
	F(RE, FxForm_Rune, 1.0f, 0, 96, 8, 3, 24, 0, FIX, kGold, 2.6f, 0.06f, ADD, 1, 0, AE),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 1.5f, 24, 0, FIX, kGold, 1.6f, 0.10f, ADD, 1, 0.2f, AE));

LOOK(kAaMythicGlyph, "mythic glyph",
	G(IM, Glyph_Gem, 1.0f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kPrism, 3.0f),
	F(IM, FxForm_Rune, 2.0f, 0, 70, 6, 1, 12, 0, FIX, kPrism, 3.0f),
	F(IM, FxForm_Sparkle, 1.4f, 8.0f, 40, 20, 0.12f, 0, 0, FIX, kPrism, 3.0f));

LOOK(kAaStaunchRecovery, "staunch recovery",
	G(IM, Glyph_Cross, 0.9f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kHealGold, 2.6f),
	F(IM, FxForm_Flame, 0.9f, 9.0f, 80, 20, 0.1f, 3.0f, 0, FIX, kHeal, 2.4f, 0.08f),
	F(IM, FxForm_Sparkle, 1.4f, 8.0f, 48, 24, 0.12f, 0, 0, FIX, kMana, 2.4f));

LOOK(kAaServant, "steadfast servant",
	F(RE, FxForm_Rune, 3.0f, 0, 70, 6, 1, 12, 0, FIX, kGold, 3.5f),
	G(RE, Glyph_Helm, 1.0f, 3.0f, 4, 1, GlyphAnim_Rise | GlyphAnim_Pop, 0, FIX, kSilver, 3.5f),
	F(RE, FxForm_Flame, 0.9f, 7.0f, 72, 18, 0.2f, 1.2f, 0, FIX, kLight, 3.0f, 0.06f));

LOOK(kAaMerchant, "summon merchant",
	G(RE, Glyph_Coin, 0.4f, 4.0f, 4, 5, GA_Orbit | GlyphAnim_Roll, 1.4f, FIX, kGold, 3.0f),
	F(RE, FxForm_Rune, 2.2f, 0, 60, 6, 1, 8, 0, FIX, kGold, 3.0f),
	F(RE, FxForm_Sparkle, 1.6f, 6.0f, 40, 20, 0.12f, 0, 0, FIX, kGold, 3.0f));

LOOK(kAaBanker, "clockwork banker",
	G(RE, Glyph_Cog, 0.9f, 7.0f, 4, 1, GA_Roll, 0, FIX, kIron, 3.0f),
	G(RE, Glyph_Coin, 0.35f, 4.0f, 4, 4, GA_Orbit | GlyphAnim_Roll, 1.3f, FIX, kGold, 3.0f),
	F(RE, FxForm_Rune, 2.2f, 0, 60, 8, 3, 12, 0, FIX, kIron, 3.0f));

LOOK(kAaWildMagic, "wild magic",
	G(RE, Glyph_Tornado, 1.0f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kPrism, 3.0f),
	F(RE, FxForm_Vortex, 1.8f, 8.0f, 96, 5, 1.5f, 3.0f, 4.0f, FIX, kPrism, 3.0f, 0.06f),
	F(RE, FxForm_Sparkle, 2.0f, 8.0f, 60, 30, 0.14f, 0, 0, FIX, kPrism, 3.0f));

LOOK(kAaRadiantCure, "radiant cure",
	G(RE, Glyph_Sun, 1.1f, 8.5f, 4, 1, GA_PopRoll, 0, FIX, kWhite, 2.2f),
	F(RE, FxForm_Burst, 1.0f, 1.0f, 96, 2, 1.5f, 16, 0, FIX, kWhite, 1.4f, 0.09f, ADD, 1, 0.2f, AE),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kWhite, 2.0f));

LOOK(kAaTranquil, "tranquil blessings",
	G(RE, Glyph_Dove, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kWhite, 3.0f),
	F(RE, FxForm_Bloom, 1.0f, 2.0f, 96, 3, 0, 0, 0, FIX, kGold, 3.0f, 0.05f, ADD, 1, 0, AE));

LOOK(kAaShieldFlash, "shield flash",
	G(IM, Glyph_Shield, 1.4f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kWhite, 1.6f),
	F(IM, FxForm_Spire, 0.6f, 4.5f, 96, 16, 3.0f, 3.5f, 0, FIX, kWhite, 0.7f, 0.07f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0, 12, 0, FIX, kSilver, 0.6f, 0.08f));

LOOK(kAaDivineAura, "divine aura",
	F(IM, FxForm_Dome, 2.2f, 7.5f, 128, 10, 4, 0, 0, FIX, kDivine, 4.0f, 0.07f),
	F(IM, FxForm_Lattice, 2.0f, 7.0f, 96, 12, 0.6f, 0, 0, FIX, kGold, 4.0f, 0.05f),
	G(IM, Glyph_Sun, 1.0f, 9.0f, 4, 1, GA_PopRoll, 0, FIX, kGold, 4.0f),
	F(IM, FxForm_Flame, 1.0f, 11.0f, 72, 18, 0.1f, 3.0f, 0, FIX, kDivine, 3.0f, 0.06f));

LOOK(kAaCompanionAegis, "companion's aegis",
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kArcane, 3.0f, 0.05f),
	G(IM, Glyph_Paw, 0.5f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kArcane, 3.0f));

LOOK(kAaSummonCompanion, "summon companion",
	G(IM, Glyph_Paw, 0.8f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kArcane, 2.5f),
	F(IM, FxForm_Vortex, 1.4f, 4.0f, 64, 4, 1.2f, 2.0f, -3.0f, FIX, kArcane, 2.5f, 0.05f));

LOOK(kAaCompanionAura, "companion's divine aura",
	F(IM, FxForm_Dome, 1.9f, 7.0f, 112, 8, 3, 0, 0, FIX, kDivine, 3.0f, 0.06f),
	G(IM, Glyph_Paw, 0.5f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f));

LOOK(kAaCompanionBlessing, "companion's blessing",
	G(IM, Glyph_Cross, 0.5f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 2.4f),
	F(IM, FxForm_Helix, 1.0f, 6.0f, 64, 2, 1.2f, 0, 0.5f, FIX, kHeal, 2.4f, 0.05f, COMET));

LOOK(kAaCompanionShielding, "companion's shielding",
	G(IM, Glyph_Shield, 0.8f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kIron, 3.0f),
	F(IM, FxForm_Ring, 1.4f, 3.0f, 48, 0, 1.0f, 1.0f, 0.5f, FIX, kIron, 3.0f, 0.05f, COMET));

LOOK(kAaGate, "gate",
	G(RE, Glyph_Door, 1.2f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kArcane, 3.0f),
	F(RE, FxForm_Vortex, 1.6f, 9.0f, 96, 5, 0.8f, 2.5f, -5.0f, FIX, kArcane, 3.0f, 0.06f),
	F(RE, FxForm_Rune, 1.6f, 0, 64, 5, 2, 10, 0, FIX, kMana, 3.0f));

LOOK(kAaBindAffinity, "bind affinity",
	G(IM, Glyph_Link, 0.35f, 4.0f, 3, 6, GA_Orbit, 1.2f, FIX, kSilver, 3.0f),
	F(IM, FxForm_Rune, 1.4f, 0, 64, 4, 1, 8, 0, FIX, kSilver, 3.0f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.12f, 2, 0, FIX, kWhite, 2.6f));

LOOK(kAaMarkTravel, "mark of travel",
	G(IM, Glyph_Feather, 0.6f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kAir, 2.4f),
	F(IM, FxForm_Wind, 1.2f, 6.0f, 64, 10, 1.4f, 0, 0, FIX, kAir, 2.4f, 0.04f),
	F(IM, FxForm_Ring, 1.2f, 0.4f, 48, 0, 2.0f, 1.0f, 1.0f, FIX, kGold, 2.4f, 0.05f, COMET));

LOOK(kAaSecondaryRecall, "secondary recall",
	G(RE, Glyph_Door, 1.0f, 7.0f, 4, 2, GA_Orbit, 1.4f, FIX, kSea, 3.0f),
	F(RE, FxForm_Vortex, 1.8f, 9.0f, 96, 4, 1.0f, 2.0f, 4.0f, FIX, kSea, 3.0f, 0.06f),
	F(RE, FxForm_Sparkle, 1.6f, 9.0f, 48, 24, 0.12f, 0, 0, FIX, kWater, 3.0f));

LOOK(kAaTertiaryRecall, "tertiary recall",
	G(RE, Glyph_Door, 0.9f, 7.0f, 4, 3, GA_Orbit, 1.5f, FIX, kAurora, 3.0f),
	F(RE, FxForm_Helix, 1.4f, 9.0f, 96, 4, 3.0f, 0, 0.6f, FIX, kAurora, 3.0f, 0.05f, COMET),
	F(RE, FxForm_Bloom, 1.8f, 0.5f, 64, 3, 0, 0, 0, FIX, kAurora, 2.4f, 0.05f));

LOOK(kAaTeleportBind, "teleport bind",
	G(RE, Glyph_Link, 0.6f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kGold, 3.0f),
	F(RE, FxForm_Spire, 0.4f, 12.0f, 96, 20, 0.2f, 10.0f, 0, FIX, kGold, 2.6f, 0.06f),
	F(RE, FxForm_Rune, 1.5f, 0, 64, 6, 1, 12, 0, FIX, kGold, 3.0f));

LOOK(kAaGuildAnchor, "guild hall anchor",
	G(RE, Glyph_Banner, 1.0f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kRoyal, 3.0f),
	F(RE, FxForm_Lattice, 1.3f, 8.0f, 64, 6, 0.4f, 0, 0, FIX, kRoyal, 3.0f, 0.05f),
	F(RE, FxForm_Vortex, 1.5f, 8.0f, 80, 4, 1.0f, 2.0f, 3.0f, FIX, kGold, 3.0f, 0.05f));

LOOK(kAaEradicate, "eradicate magic",
	G(IM, Glyph_ArrowDown, 0.7f, 8.0f, 4, 3, GA_Rain, 1.0f, FIX, kMagic, 2.2f),
	F(IM, FxForm_Bloom, 2.0f, 4.0f, 64, 3, 0, 0, 0, FIX, kMagic, 1.6f, 0.06f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Sparkle, 1.4f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kWhite, 2.0f));

LOOK(kAaCompanionRelocation, "companion relocation",
	G(IM, Glyph_Paw, 0.7f, 6.0f, 4, 1, GA_PopRoll, 0, FIX, kArcane, 2.2f),
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.0f, 10, 0, FIX, kArcane, 1.2f, 0.05f),
	F(IM, FxForm_Bloom, 1.2f, 0.4f, 48, 2, 0, 0, 0, FIX, kArcane, 1.4f, 0.05f));

LOOK(kAaFortifyCompanion, "fortify companion",
	G(IM, Glyph_Tower, 0.7f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kStone, 3.0f),
	F(IM, FxForm_Lattice, 1.3f, 6.0f, 64, 8, 0.3f, 0, 0, FIX, kStone, 3.0f, 0.05f),
	F(IM, FxForm_Ring, 1.3f, 3.0f, 48, 0, -1.0f, 1.0f, 0.5f, FIX, kIron, 3.0f, 0.05f, COMET));

LOOK(kAaShrink, "shrink",
	G(IM, Glyph_ArrowDown, 0.6f, 9.0f, 4, 1, GA_Drop, 0, FIX, kPink, 2.0f),
	F(IM, FxForm_Bloom, 1.8f, 3.0f, 48, 3, 0, 0, 0, FIX, kPink, 1.6f, 0.05f, ADD | FxFlag_Reverse),
	F(IM, FxForm_Sparkle, 1.0f, 5.0f, 24, 12, 0.10f, 2, 0, FIX, kWhite, 1.8f));

enum Line : int
{
	DivineAura, ActOfValor, Enmity, ArmorExperience, InquisitorArmor, AureateBane, Balefire, Breath,
	Jester, Recovery, Rejuvenation, Fireworks, Portal, Throne, Infusion, Intensity, LessonDevoted,
	MassGroupBuff, MythicGlyph, StaunchRecovery, Servant, Merchant, Banker, WildMagic, RadiantCure,
	Tranquil, ShieldFlash, CompanionAegis, SummonCompanion, CompanionAura, CompanionBlessing,
	CompanionShielding,
	Gate, BindAffinity, MarkTravel, SecondaryRecall, TertiaryRecall, TeleportBind, GuildAnchor, Eradicate, CompanionRelocation, FortifyCompanion, Shrink,
};

const LineLook kLines[] = {
	{ "Divine Aura",                     &kAaDivineAura },
	{ "Act of Valor",                    &kAaActOfValor },
	{ "Ageless Enmity",                  &kAaEnmity },
	{ "Armor of Experience",             &kAaArmorExperience },
	{ "Armor of the Inquisitor",         &kAaInquisitorArmor },
	{ "Aureate's Bane",                  &kAaAureateBane },
	{ "Balefire Burst",                  &kAaBalefire },
	{ "Dragon Breath",                   &kAaBreath, kFromSpell },
	{ "Chaotic Jester",                  &kAaJester },
	{ "Expedient Recovery",              &kAaRecovery },
	{ "Forceful Rejuvenation",           &kAaRejuvenation },
	{ "Glyph of Fireworks",              &kAaFireworks },
	{ "Origin / Harmonic Dissonance",    &kAaPortal },
	{ "Throne of Heroes",                &kAaThrone },
	{ "Infusion of the Faithful",        &kAaInfusion },
	{ "Intensity of the Resolute",       &kAaIntensity },
	{ "Lesson of the Devoted",           &kAaLessonDevoted },
	{ "Mass Group Buff",                 &kAaMassGroupBuff },
	{ "Mythic Glyphs",                   &kAaMythicGlyph },
	{ "Staunch Recovery",                &kAaStaunchRecovery },
	{ "Steadfast Servant",               &kAaServant },
	{ "Summon merchants",                &kAaMerchant },
	{ "Summon Clockwork Banker",         &kAaBanker },
	{ "Wild Magic",                      &kAaWildMagic },
	{ "Radiant Cure",                    &kAaRadiantCure },
	{ "Tranquil Blessings",              &kAaTranquil },
	{ "Shield Flash",                    &kAaShieldFlash },
	{ "Companion's Aegis",               &kAaCompanionAegis },
	{ "Summon Companion",                &kAaSummonCompanion },
	{ "Companion's Divine Aura",         &kAaCompanionAura },
	{ "Companion's Blessing",            &kAaCompanionBlessing },
	{ "Companion's Shielding",           &kAaCompanionShielding },
	{ "Gate",                           &kAaGate },
	{ "Bind Affinity",                  &kAaBindAffinity },
	{ "Mark of Travel",                 &kAaMarkTravel },
	{ "Secondary Recall",               &kAaSecondaryRecall },
	{ "Tertiary Recall",                &kAaTertiaryRecall },
	{ "Teleport Bind",                  &kAaTeleportBind },
	{ "Teleport to Guild Hall Anchor",  &kAaGuildAnchor },
	{ "Eradicate Magic",                &kAaEradicate },
	{ "Companion's Discipline",         &kAaCompanionRelocation },
	{ "Companion's Fortification",      &kAaFortifyCompanion },
	{ "Shrink",                         &kAaShrink },
};

const NamedLine kAbilities[] = {
	{ "divine aura",                     DivineAura },
	{ "bestow divine aura",              DivineAura },
	{ "bestowed divine aura",            DivineAura },
	{ "act of valor",                    ActOfValor },
	{ "ageless enmity",                  Enmity },
	{ "armor of experience",             ArmorExperience },
	{ "armor of the inquisitor",         InquisitorArmor },
	{ "group armor of the inquisitor",   InquisitorArmor },
	{ "aureate's bane",                  AureateBane },
	{ "balefire burst",                  Balefire },
	{ "breath of atathus",               Breath },
	{ "breath of draton'ra",             Breath },
	{ "breath of keikolin",              Breath },
	{ "breath of mysaphar",              Breath },
	{ "breath of osh'vir",               Breath },
	{ "breath of venesh",                Breath },
	{ "chaotic jester",                  Jester },
	{ "expedient recovery",              Recovery },
	{ "forceful rejuvenation",           Rejuvenation },
	{ "glyph of fireworks",              Fireworks },
	{ "origin",                          Portal },
	{ "harmonic dissonance",             Portal },
	{ "throne of heroes",                Throne },
	{ "infusion of the faithful",        Infusion },
	{ "intensity of the resolute",       Intensity },
	{ "lesson of the devoted",           LessonDevoted },
	{ "mass group buff",                 MassGroupBuff },
	{ "staunch recovery",                StaunchRecovery },
	{ "steadfast servant",               Servant },
	{ "summon permutation peddler",      Merchant },
	{ "summon personal tribute master",  Merchant },
	{ "summon resupply agent",           Merchant },
	{ "summon clockwork banker",         Banker },
	{ "wild magic: unleash",             WildMagic },
	{ "wild magic: destroy",             WildMagic },
	{ "radiant cure",                    RadiantCure },
	{ "tranquil blessings",              Tranquil },
	{ "shield flash",                    ShieldFlash },
	{ "companion's aegis",               CompanionAegis },
	{ "summon companion",                SummonCompanion },
	{ "companion's intervening divine aura", CompanionAura },
	{ "companion's blessing",            CompanionBlessing },
	{ "companion's shielding",           CompanionShielding },
	{ "gate",                           Gate },
	{ "bind affinity",                  BindAffinity },
	{ "mark of travel",                 MarkTravel },
	{ "secondary recall",               SecondaryRecall },
	{ "tertiary recall",                TertiaryRecall },
	{ "teleport bind",                  TeleportBind },
	{ "teleport guild hall anchor",     GuildAnchor },
	{ "eradicate magic",                Eradicate },
	{ "companion relocation",           CompanionRelocation },
	{ "fortify companion",              FortifyCompanion },
	{ "shrink",                         Shrink },
	{ "alter plane: theater of blood",  Portal },
	{ "summon distiller vendor",        Merchant },
	{ "summon tribute master",          Merchant },
	{ "glyph of arcane secrets",        MythicGlyph },
	{ "glyph of dragon scales",         MythicGlyph },
	{ "glyph of ultimate power",        MythicGlyph },
};

int BySpell(const eqlib::EQ_Spell*, std::string_view base)
{
	return StartsWith(base, "mythic glyph") ? MythicGlyph : -1;
}

} // namespace

const ClassCatalogue& GeneralCatalogue()
{
	static const ClassCatalogue c{ "General", 0, kLines, 0, {}, {}, {}, kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
