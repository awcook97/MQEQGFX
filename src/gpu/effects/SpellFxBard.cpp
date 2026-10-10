#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

namespace eqgfx::spellfx {

namespace {

constexpr Pair kLullaby { { 0.85f, 0.80f, 1.00f, 0.95f }, { 0.45f, 0.35f, 0.90f, 0.80f } };
constexpr Pair kMarch   { { 1.00f, 0.75f, 0.30f, 0.95f }, { 0.80f, 0.15f, 0.10f, 0.85f } };
constexpr Pair kHush    { { 0.80f, 0.82f, 0.90f, 0.70f }, { 0.40f, 0.42f, 0.55f, 0.40f } };
constexpr Pair kMelody  { { 0.80f, 1.00f, 0.85f, 0.95f }, { 1.00f, 0.80f, 0.35f, 0.85f } };

LOOK(kBrdSilence, "silence",
	Notes(kHush),
	G(IM, Glyph_Note, 0.4f, 8.0f, 4, 3, GA_Rain, 0.9f, FIX, kHush, 2.2f),
	F(IM, FxForm_Bloom, 1.6f, 2.0f, 40, 2, 0, 0, 0, FIX, kHush, 2.2f, 0.04f, ADD | FxFlag_Reverse));

LOOK(kBrdLullaby, "lullaby",
	Notes(kLullaby),
	G(AU, Glyph_Moon, 0.6f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kLullaby, 5.0f),
	G(AU, Glyph_Note, 0.3f, 7.0f, 0, 3, GA_Orbit, 0.9f, FIX, kLullaby, 5.0f),
	F(AU, FxForm_Ring, 0.9f, 7.0f, 32, 0, 0.8f, 1.0f, 0.3f, FIX, kLullaby, 5.0f, 0.04f, COMET));

LOOK(kBrdSlumber, "slumber",
	Notes(kLullaby),
	G(AU, Glyph_Clef, 0.6f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kLullaby, 5.0f),
	G(AU, Glyph_Z, 0.35f, 8.0f, 0, 1, GlyphAnim_Ascend, 0.5f, FIX, kWhite, 2.0f, 0.3f),
	G(AU, Glyph_Z, 0.45f, 8.3f, 0, 1, GlyphAnim_Ascend, 0.5f, FIX, kWhite, 2.0f, 1.2f));

LOOK(kBrdWaveSleep, "wave of sleep",
	G(RE, Glyph_Note, 0.35f, 5.0f, 3, 8, GA_RingRise, 2.6f, FIX, kLullaby, 2.4f),
	F(RE, FxForm_Bloom, 1.0f, 1.5f, 96, 3, 0, 0, 0, FIX, kLullaby, 2.4f, 0.06f, ADD, 0.8f, 0, AE),
	G(IM, Glyph_Z, 0.45f, 7.8f, 0, 1, GlyphAnim_Ascend, 0.5f, FIX, kWhite, 2.0f));

LOOK(kBrdVoice, "voice",
	Notes(kPink),
	G(AU, Glyph_Heart, 0.35f, 7.2f, 0, 3, GA_Orbit, 1.1f, FIX, kPink, 6.0f),
	G(AU, Glyph_Note, 0.3f, 6.2f, 0, 3, GA_Orbit, 1.4f, FIX, kPink, 6.0f));

LOOK(kBrdBinding, "melodic binding",
	Notes(kHush),
	G(AU, Glyph_Link, 0.35f, 0.6f, 3, 6, GA_PopSpin, 1.1f, FIX, kSong, 5.0f),
	G(AU, Glyph_Hourglass, 0.6f, 7.4f, 4, 1, GA_PopBob, 0, FIX, kHush, 5.0f));

LOOK(kBrdAssonant, "assonant binding",
	Notes(kSong),
	G(AU, Glyph_Shackle, 0.5f, 0.8f, 3, 2, GlyphAnim_Pop, 0.45f, FIX, kSong, 5.0f),
	G(AU, Glyph_Note, 0.3f, 1.2f, 3, 4, GA_PopSpin, 1.3f, FIX, kSong, 5.0f));

LOOK(kBrdCantata, "cantata",
	G(RE, Glyph_Note, 0.35f, 5.0f, 3, 6, GA_RingRise, 2.2f, FIX, kHeal, 2.4f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 2, 0, 0, 0, FIX, kHeal, 2.4f, 0.05f, ADD, 1, 0, AE));

LOOK(kBrdChorus, "chorus",
	G(RE, Glyph_Lute, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kSong, 2.4f),
	G(RE, Glyph_Note, 0.3f, 4.5f, 3, 8, GA_RingRise, 2.4f, FIX, kMelody, 2.4f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 2, 0, 0, 0, FIX, kMana, 2.4f, 0.05f, ADD, 1, 0, AE));

LOOK(kBrdPulse, "pulse",
	G(RE, Glyph_Heart, 0.6f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kHeal, 2.4f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 2, 1.0f, 0, 0, FIX, kHeal, 1.0f, 0.08f, ADD, 1, 0, AE),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 2, 1.0f, 0, 0, FIX, kHeal, 1.0f, 0.08f, ADD, 1, 0.6f, AE),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 96, 2, 1.0f, 0, 0, FIX, kHeal, 1.0f, 0.08f, ADD, 1, 1.2f, AE));

LOOK(kBrdFatesong, "fatesong",
	Notes(kCold),
	G(RE, Glyph_Snowflake, 0.35f, 5.5f, 0, 4, GA_Orbit, 1.4f, FIX, kCold, 2.4f),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.10f, 1, 0, FIX, kCold, 2.0f));

LOOK(kBrdAria, "aria",
	G(RE, Glyph_Mouth, 0.7f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSong, 2.0f),
	G(RE, Glyph_Note, 0.3f, 7.0f, 0, 5, GA_PopAscend | GlyphAnim_Spin, 1.0f, FIX, kSong, 2.2f),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.12f, 0, 0, FIX, kSong, 2.0f));

LOOK(kBrdPotency, "psalm of potency",
	Notes(kFire),
	F(RE, FxForm_Flame, 0.9f, 3.0f, 48, 12, 0.3f, 0.8f, 0, FIX, kFire, 2.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.12f, 0, 0, FIX, kFire, 2.0f));

LOOK(kBrdHasteAria, "aria of the artist",
	G(RE, Glyph_Clef, 0.8f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kStar, 2.2f),
	F(RE, FxForm_Wind, 1.3f, 5.0f, 60, 12, 2.5f, 0, 0, FIX, kStar, 2.4f, 0.05f),
	F(IM, FxForm_Wind, 1.2f, 4.0f, 48, 8, 2.5f, 0, 0, FIX, kStar, 1.6f, 0.05f));

LOOK(kBrdDirge, "dirge",
	G(RE, Glyph_Drum, 0.8f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kIron, 2.4f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kIron, 2.4f, 0.06f, ADD, 1, 0, AE));

LOOK(kBrdSonata, "sonata",
	Notes(kFeather),
	G(RE, Glyph_Feather, 0.35f, 5.5f, 0, 4, GA_Orbit, 1.4f, FIX, kFeather, 2.4f),
	F(IM, FxForm_Wind, 1.2f, 4.0f, 48, 8, 1.5f, 0, 0, FIX, kFeather, 1.8f, 0.04f));

LOOK(kBrdWarMarch, "war march",
	G(RE, Glyph_Drum, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kMarch, 2.4f),
	G(RE, Glyph_Banner, 0.7f, 7.0f, 3, 2, GA_PopSpin, 1.6f, FIX, kMarch, 2.4f),
	F(RE, FxForm_Burst, 1.0f, 0.2f, 96, 2, 0.2f, 12, 0, FIX, kMarch, 1.2f, 0.08f, ADD, 1, 0, AE),
	F(IM, FxForm_Wind, 1.2f, 4.0f, 48, 8, 2.0f, 0, 0, FIX, kMarch, 1.6f, 0.05f));

LOOK(kBrdArcane, "arcane",
	Notes(kArcane),
	F(RE, FxForm_Rune, 1.9f, 0, 70, 5, 2, 10, 0, FIX, kArcane, 2.4f),
	F(IM, FxForm_Sparkle, 1.2f, 6.5f, 32, 16, 0.12f, 0, 0, FIX, kArcane, 2.0f));

LOOK(kBrdAura, "aura",
	F(RE, FxForm_Rune, 2.6f, 0, 80, 8, 3, 24, 0, FIX, kSong, 3.0f, 0.05f),
	G(RE, Glyph_Note, 0.35f, 1.0f, 3, 8, GA_PopSpin, 2.4f, FIX, kSong, 3.0f));

LOOK(kBrdAuraHeal, "healing aura",
	F(RE, FxForm_Rune, 2.6f, 0, 80, 8, 3, 24, 0, FIX, kHeal, 3.0f, 0.05f),
	G(RE, Glyph_Cross, 0.3f, 1.0f, 3, 6, GA_PopSpin, 2.4f, FIX, kHeal, 3.0f),
	G(RE, Glyph_Note, 0.3f, 4.0f, 0, 4, GA_Orbit, 1.6f, FIX, kHeal, 3.0f));

LOOK(kBrdBarrier, "barrier",
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kSong, 2.4f, 0.04f),
	G(IM, Glyph_Note, 0.3f, 4.0f, 3, 6, GA_Orbit, 1.5f, FIX, kSong, 2.4f));

LOOK(kBrdCourage, "courage",
	Notes(kGold),
	G(IM, Glyph_Shield, 0.6f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kGold, 2.2f));

LOOK(kBrdMirror, "mirror melody",
	G(IM, Glyph_Shield, 0.8f, 5.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kSilver, 2.2f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 0, 12, 0, FIX, kSilver, 1.0f, 0.06f, ADD | FxFlag_Reverse),
	Notes(kSilver));

LOOK(kBrdPious, "psalm of the pious",
	Notes(kWhite),
	G(IM, Glyph_Shield, 0.55f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kWhite, 2.2f),
	F(IM, FxForm_Spire, 1.0f, 3.5f, 48, 12, 0.5f, 1.6f, 0, FIX, kWhite, 1.8f, 0.04f));

LOOK(kBrdSpiteful, "spiteful lyric",
	Notes(kBlood),
	F(IM, FxForm_Spire, 1.0f, 3.5f, 48, 12, 0.5f, 1.8f, 0, FIX, kBlood, 2.0f, 0.05f),
	G(IM, Glyph_Note, 0.25f, 5.0f, 0, 4, GA_Orbit, 1.2f, FIX, kBlood, 2.4f));

LOOK(kBrdSuffering, "song of suffering",
	Notes(kBlood),
	G(IM, Glyph_Sword, 0.45f, 4.0f, 3, 3, GA_Orbit, 1.3f, FIX, kBlood, 2.4f));

LOOK(kBrdAccelerando, "accelerando",
	G(IM, Glyph_Hourglass, 0.7f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kSong, 2.4f),
	Notes(kSong));

LOOK(kBrdCrescendo, "crescendo",
	G(IM, Glyph_Clef, 0.9f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kSong, 2.4f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kMana, 2.4f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 2.0f, 12, 0, FIX, kSong, 0.8f, 0.07f, ADD, 1, 1.2f));

LOOK(kBrdReckless, "reckless renewal",
	G(IM, Glyph_Heart, 0.6f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kSong, 2.4f),
	F(IM, FxForm_Burst, 1.8f, 3.0f, 48, 1, 1.5f, 12, 0, FIX, kSong, 0.8f, 0.07f));

LOOK(kBrdReflexive, "reflexive",
	G(IM, Glyph_Note, 0.5f, 4.5f, 4, 1, GA_PopRoll, 0, FIX, kSilver, 1.2f),
	F(IM, FxForm_Burst, 1.6f, 3.0f, 48, 1, 0, 10, 0, FIX, kSilver, 0.8f, 0.06f, ADD | FxFlag_Reverse));

LOOK(kBrdReciprocal, "reciprocal psalm",
	G(RE, Glyph_Sun, 0.7f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kSong, 2.4f),
	G(RE, Glyph_Moon, 0.7f, 8.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kLullaby, 2.4f, 0.3f),
	G(RE, Glyph_Note, 0.3f, 4.5f, 3, 6, GA_RingRise, 2.0f, FIX, kSong, 2.4f));

LOOK(kBrdSelo, "selo's",
	Notes(kNature),
	G(IM, Glyph_Feather, 0.45f, 6.8f, 0, 1, GA_PopAscend, 0, FIX, kNature, 1.6f),
	F(IM, FxForm_Wind, 1.2f, 2.5f, 72, 12, 3.0f, 0, 0, FIX, kNature, 3.0f, 0.05f));

LOOK(kBrdFermata, "fermata",
	F(IM, FxForm_Dome, 2.0f, 7.0f, 112, 8, 3, 0, 0, FIX, kGold, 3.0f, 0.06f),
	G(IM, Glyph_Clef, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f));

LOOK(kBrdChantDisease, "chant of disease",
	Notes(kDisease),
	G(IM, Glyph_Note, 0.3f, 6.5f, 0, 3, GA_Orbit, 1.0f, FIX, kDisease, 2.4f),
	F(IM, FxForm_Bubbles, 1.1f, 5.0f, 64, 8, 0.2f, 0, 0, FIX, kDisease, 2.4f, 0.04f, OVER));

LOOK(kBrdChantFrost, "chant of frost",
	Notes(kCold),
	G(IM, Glyph_Note, 0.3f, 6.5f, 0, 3, GA_Orbit, 1.0f, FIX, kCold, 2.4f),
	F(IM, FxForm_Crystal, 1.1f, 4.5f, 40, 8, 0, 0.9f, 0, FIX, kCold, 2.2f, 0.05f));

LOOK(kBrdChantPoison, "chant of poison",
	Notes(kPoison),
	G(IM, Glyph_Note, 0.3f, 6.5f, 0, 3, GA_Orbit, 1.0f, FIX, kPoison, 2.4f),
	F(IM, FxForm_Bubbles, 1.1f, 5.5f, 64, 10, 0.2f, 0, 0, FIX, kPoison, 2.4f, 0.04f));

LOOK(kBrdChantFlame, "chant of flame",
	Notes(kFire),
	G(IM, Glyph_Note, 0.3f, 6.5f, 0, 3, GA_Orbit, 1.0f, FIX, kFire, 2.4f),
	F(IM, FxForm_Flame, 1.0f, 5.0f, 64, 16, 0.8f, 0.8f, 0, FIX, kFire, 2.2f, 0.07f));

LOOK(kBrdChantResist, "chant of weakness",
	Notes(kSong),
	G(IM, Glyph_Note, 0.3f, 6.5f, 0, 3, GA_Orbit, 1.0f, PRI, kSong, 2.4f),
	F(IM, FxForm_Bloom, 1.8f, 2.0f, 40, 3, 0, 0, 0, PRI, kSong, 2.0f, 0.05f, ADD | FxFlag_Reverse));

LOOK(kBrdDenon, "denon's dirge",
	G(IM, Glyph_Drum, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kShadow, 2.4f, 0, OVER),
	F(IM, FxForm_Bloom, 1.0f, 1.5f, 96, 3, 0, 0, 0, FIX, kCorrupt, 2.4f, 0.06f, ADD, 1, 0, AE),
	G(IM, Glyph_Note, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.4f, FIX, kCorrupt, 2.4f));

LOOK(kBrdInsult, "insult",
	G(RE, Glyph_Mouth, 0.8f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kBlood, 1.4f),
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.6f, 0.4f, 0.8f, FIX, kSong, 0.7f, 0.14f),
	F(IM, FxForm_Burst, 2.2f, 2.0f, 64, 2, 1.0f, 12, 0, FIX, kBlood, 0.9f, 0.09f),
	G(IM, Glyph_Note, 0.35f, 6.5f, 0, 3, GA_PopAscend | GlyphAnim_Spin, 0.9f, FIX, kBlood, 1.6f));

LOOK(kBrdBurningCall, "burning call",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.35f, 1.5f, FIX, kFire, 0.8f, 0.18f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 72, 2, 2.0f, 8, 0, FIX, kFire, 0.8f, 0.12f),
	G(IM, Glyph_Note, 0.3f, 5.0f, 3, 6, GA_RingRise, 1.4f, FIX, kFire, 1.6f));

LOOK(kBrdSticks, "sticks and stones",
	G(RE, Glyph_Rock, 0.35f, 5.0f, 0, 5, GA_Orbit, 1.3f, FIX, kEarth, 2.4f, 0, OVER),
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.3f, 2.0f, FIX, kEarth, 0.8f, 0.16f, OVER),
	G(IM, Glyph_Mouth, 0.7f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kSong, 1.8f));

LOOK(kBrdPiper, "piper",
	Notes(kSong),
	G(RE, Glyph_Paw, 0.35f, 0.3f, 2, 8, GlyphAnim_Pop | GlyphAnim_Spin, 2.0f, FIX, kGrey, 3.0f, 0, OVER),
	F(RE, FxForm_Rune, 2.2f, 0, 60, 6, 1, 8, 0, FIX, kSong, 3.0f));

LOOK(kBrdAbsolution, "aria of absolution",
	G(IM, Glyph_Note, 0.4f, 8.0f, 4, 3, GA_Rain, 0.9f, FIX, kWhite, 2.0f),
	F(IM, FxForm_Bloom, 1.4f, 1.0f, 40, 3, 0, 0, 0, FIX, kWhite, 2.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kWhite, 2.0f));

LOOK(kBrdRequiem, "requiem",
	G(IM, Glyph_Ankh, 0.7f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kHush, 2.4f),
	Notes(kHush));

LOOK(kAaBoastful, "boastful bellow",
	G(RE, Glyph_Mouth, 1.0f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kBlood, 1.6f),
	F(TR, FxForm_Bolt, 1, 0, 128, 6, 0.8f, 0.8f, 0.5f, FIX, kSong, 0.8f, 0.30f),
	F(IM, FxForm_Burst, 2.4f, 2.5f, 72, 2, 0.5f, 14, 0, FIX, kSong, 0.9f, 0.10f),
	Stars(0.3f));

LOOK(kAaVainglorious, "vainglorious shout",
	G(RE, Glyph_Mouth, 1.0f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kMarch, 1.6f),
	F(IM, FxForm_Lightning, 1, 0, 64, 3, 0.6f, 16.0f, 0, FIX, kSong, 0.8f, 0.20f),
	Stars(0.3f));

LOOK(kAaBladedSong, "bladed song",
	G(IM, Glyph_Sword, 0.45f, 4.0f, 3, 6, GA_Orbit, 1.5f, FIX, kSilver, 3.0f),
	Notes(kSilver));

LOOK(kAaDanceBlades, "dance of blades",
	G(IM, Glyph_Sword, 0.6f, 3.5f, 3, 6, GA_Orbit, 1.6f, FIX, kSilver, 4.0f),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 60, 12, 2.5f, 0, 0, FIX, kSilver, 3.0f, 0.05f),
	Notes(kSong));

LOOK(kAaSleepwalker, "dirge of the sleepwalker",
	G(AU, Glyph_Spiral, 0.8f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kLullaby, 5.0f),
	G(AU, Glyph_Z, 0.4f, 8.0f, 0, 1, GlyphAnim_Ascend, 0.5f, FIX, kWhite, 2.0f, 0.5f),
	Notes(kLullaby));

LOOK(kAaFierceEye, "fierce eye",
	G(RE, Glyph_Eye, 1.1f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kMarch, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kMarch, 1.4f, 0.10f, ADD, 1, 0.2f, AE),
	G(IM, Glyph_Eye, 0.4f, 7.2f, 4, 1, GA_PopAscend, 0, FIX, kMarch, 1.8f));

LOOK(kAaFlurryNotes, "flurry of notes",
	G(IM, Glyph_Note, 0.3f, 5.0f, 0, 10, GA_Orbit | GlyphAnim_Ascend, 1.6f, FIX, kSong, 2.4f),
	F(IM, FxForm_Burst, 2.0f, 2.0f, 48, 1, 1.5f, 12, 0, FIX, kSong, 0.8f, 0.08f));

LOOK(kAaFuneralDirge, "funeral dirge",
	G(IM, Glyph_Drum, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGrey, 3.0f, 0, OVER),
	G(IM, Glyph_Skull, 0.5f, 6.0f, 4, 1, GA_PopAscend, 0, FIX, kBone, 3.0f, 0.4f, OVER),
	Notes(kGrey));

LOOK(kAaLastStand, "hymn of the last stand",
	G(IM, Glyph_Banner, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 3.0f),
	G(IM, Glyph_Shield, 0.45f, 4.0f, 3, 4, GA_Orbit, 1.4f, FIX, kGold, 3.0f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kSong, 2.6f, 0.04f));

LOOK(kAaSiren, "lure of the siren's song",
	G(RE, Glyph_Heart, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kPink, 2.4f),
	F(RE, FxForm_Bloom, 1.0f, 2.0f, 96, 3, 0, 0, 0, FIX, kPink, 2.4f, 0.06f, ADD | FxFlag_Reverse, 1, 0, AE),
	G(RE, Glyph_Note, 0.3f, 5.0f, 3, 8, GA_RingRise, 2.6f, FIX, kPink, 2.4f));

LOOK(kAaLyreLeap, "lyre leap",
	G(RE, Glyph_Lute, 0.8f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kSong, 1.8f),
	F(RE, FxForm_Wind, 1.4f, 4.0f, 72, 12, 3.0f, 0, 0, FIX, kSong, 1.4f));

LOOK(kAaPrankster, "lyrical prankster",
	G(RE, Glyph_Mask, 1.0f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kPrism, 3.0f),
	G(RE, Glyph_Note, 0.3f, 5.0f, 0, 6, GA_Orbit | GlyphAnim_Ascend, 1.4f, FIX, kPrism, 3.0f));

LOOK(kAaQuickTime, "quick time",
	G(RE, Glyph_Hourglass, 0.9f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kStar, 2.4f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kStar, 1.4f, 0.10f, ADD, 1, 0.2f, AE),
	Notes(kStar));

LOOK(kAaRallying, "rallying",
	G(RE, Glyph_Horn, 0.9f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kSong, 2.0f),
	F(RE, FxForm_Bloom, 2.0f, 4.0f, 48, 3, 0, 0, 0, FIX, kSong, 1.6f, 0.05f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kMana, 2.2f));

LOOK(kAaSelosKick, "selo's kick",
	G(IM, Glyph_Note, 0.5f, 3.0f, 4, 1, GA_PopRoll, 0, FIX, kSong, 0.8f),
	F(IM, FxForm_Burst, 1.3f, 1.5f, 32, 1, 0, 8, 0, FIX, kSong, 0.5f, 0.07f));

LOOK(kAaShauri, "shauri's sonorous clouding",
	F(RE, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kLight, 2.5f, 0.05f),
	G(RE, Glyph_Feather, 0.35f, 5.0f, 0, 4, GA_Orbit, 1.4f, FIX, kLight, 2.5f),
	Notes(kLight));

LOOK(kAaShieldNotes, "shield of notes",
	G(IM, Glyph_Note, 0.4f, 4.0f, 3, 6, GA_Orbit, 1.5f, FIX, kSong, 3.0f),
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kSong, 2.6f, 0.04f));

LOOK(kAaDisplacement, "sonic displacement",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.8f, 0.5f, 0.5f, FIX, kSong, 0.6f, 0.20f),
	F(IM, FxForm_Bloom, 1.6f, 3.0f, 48, 3, 0, 0, 0, FIX, kSong, 1.2f, 0.06f),
	F(IM, FxForm_Burst, 2.0f, 3.0f, 48, 1, 0, 12, 0, FIX, kWhite, 0.6f, 0.08f));

LOOK(kAaSongOfStone, "song of stone",
	G(RE, Glyph_Rock, 0.5f, 1.5f, 3, 3, GlyphAnim_Rise | GlyphAnim_Spin | GlyphAnim_Bob, 2.0f, FIX, kStone, 4.0f, 0, OVER),
	F(RE, FxForm_Cracks, 3.0f, 0, 48, 6, 1.2f, 0, 0, FIX, kEarth, 4.0f, 0.08f, OVER),
	Notes(kStone));

LOOK(kAaCacophony, "cacophony",
	G(IM, Glyph_Note, 0.35f, 6.0f, 0, 8, GA_PopAscend | GlyphAnim_Spin, 1.4f, FIX, kPrism, 1.8f),
	F(IM, FxForm_Burst, 2.4f, 2.5f, 72, 2, 1.0f, 16, 0, FIX, kPrism, 0.9f, 0.10f),
	F(IM, FxForm_Lightning, 1, 30.0f, 48, 2, 1.2f, 14.0f, 1.0f, FIX, kPrism, 0.6f, 0.10f));

LOOK(kAaDissonant, "dissonant chord",
	G(IM, Glyph_Note, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kBlood, 2.4f),
	G(IM, Glyph_Eye, 0.5f, 6.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kBlood, 2.4f),
	F(IM, FxForm_Bloom, 2.0f, 2.5f, 48, 3, 0, 0, 0, FIX, kBlood, 1.8f, 0.06f));

LOOK(kAaFadingMemories, "fading memories",
	G(RE, Glyph_Mask, 0.8f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kHush, 2.4f),
	F(RE, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kHush, 2.0f, 0.05f, ADD | FxFlag_Reverse));

LOOK(kAaSelosSonata, "selo's sonata",
	G(RE, Glyph_Feather, 0.35f, 5.5f, 0, 4, GA_Orbit, 1.4f, FIX, kNature, 2.4f),
	F(RE, FxForm_Wind, 1.3f, 3.0f, 72, 12, 3.0f, 0, 0, FIX, kNature, 2.4f, 0.05f),
	Notes(kNature));

LOOK(kAaSpireMinstrels, "spire of the minstrels",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kSong, 2.6f, 0.10f),
	G(RE, Glyph_Lute, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kSong, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kSong, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaThousandBlades, "thousand blades",
	G(IM, Glyph_Sword, 0.4f, 4.0f, 3, 12, GA_Orbit, 1.8f, FIX, kSilver, 4.0f),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 12, 3.0f, 0, 0, FIX, kSilver, 3.0f, 0.05f));

LOOK(kAaFrenziedKicks, "frenzied kicks",
	F(IM, FxForm_Burst, 1.3f, 1.0f, 32, 1, 0, 8, 0, FIX, kSong, 0.8f, 0.07f),
	Notes(kSong, 1.2f));

LOOK(kAaNoteworthyDisguise, "noteworthy disguise",
	G(IM, Glyph_Mask, 0.9f, 7.4f, 4, 1, GA_PopRoll, 0, FIX, kSong, 2.4f),
	Notes(kSea, 2.4f),
	F(IM, FxForm_Bloom, 1.6f, 3.0f, 64, 3, 0, 0, 0, FIX, kSea, 2.0f, 0.05f));

enum Line : int
{
	Silence, Lullaby, Slumber, WaveSleep, Voice, Binding, Assonant, Cantata, Chorus, Pulse, Fatesong,
	Aria, Potency, HasteAria, Dirge, Sonata, WarMarch, Arcane, Aura, AuraHeal, Barrier, Courage,
	Mirror, Pious, Spiteful, Suffering, Accelerando, Crescendo, Reckless, Reflexive, Reciprocal,
	Selo, Fermata, ChantDisease, ChantFrost, ChantPoison, ChantFlame, ChantResist, Denon, Insult,
	BurningCall, Sticks, Piper, Absolution, Requiem,
	Boastful, Vainglorious, BladedSong, DanceBlades, Sleepwalker, FierceEye, FlurryNotes, FuneralDirge,
	LastStand, Siren, LyreLeap, Prankster, QuickTime, Rallying, SelosKick, Shauri, ShieldNotes,
	Displacement, SongOfStone, Cacophony, Dissonant, FadingMemories, SelosSonata, SpireMinstrels,
	ThousandBlades, FrenziedKicks,
	NoteworthyDisguise,
};

const LineLook kLines[] = {
	{ "Silence (pacify)",                &kBrdSilence },
	{ "Lullaby (mesmerize)",             &kBrdLullaby },
	{ "Slumber (mesmerize)",             &kBrdSlumber },
	{ "Wave of Sleep (area mesmerize)",  &kBrdWaveSleep },
	{ "Voice / Demand (charm)",          &kBrdVoice },
	{ "Melodic Binding (slow)",          &kBrdBinding },
	{ "Assonant Binding (snare)",        &kBrdAssonant },
	{ "Cantata (group regen)",           &kBrdCantata },
	{ "Chorus (group regen)",            &kBrdChorus },
	{ "Pulse (group regen)",             &kBrdPulse },
	{ "Fatesong (cold damage)",          &kBrdFatesong },
	{ "Aria (spell damage)",             &kBrdAria },
	{ "Psalm of Potency",                &kBrdPotency },
	{ "Aria of the Artist (haste)",      &kBrdHasteAria },
	{ "Dirge (mitigation)",              &kBrdDirge },
	{ "Spry Sonata (avoidance)",         &kBrdSonata },
	{ "War March (haste, attack)",       &kBrdWarMarch },
	{ "Arcane (spell proc)",             &kBrdArcane },
	{ "Aura / Echo",                     &kBrdAura },
	{ "Healing aura",                    &kBrdAuraHeal },
	{ "Barrier (defensive proc)",        &kBrdBarrier },
	{ "Cantata of Courage",              &kBrdCourage },
	{ "Mirror Melody (reflect)",         &kBrdMirror },
	{ "Psalm (shield, resists)",         &kBrdPious },
	{ "Spiteful Lyric (damage shield)",  &kBrdSpiteful },
	{ "Song of Suffering (melee proc)",  &kBrdSuffering },
	{ "Accelerando",                     &kBrdAccelerando },
	{ "Lively Crescendo (self regen)",   &kBrdCrescendo },
	{ "Reckless Renewal",                &kBrdReckless },
	{ "Reflexive (proc)",                &kBrdReflexive },
	{ "Reciprocal Psalm",                &kBrdReciprocal },
	{ "Selo's (run speed)",              &kBrdSelo },
	{ "Fermata (invulnerable)",          &kBrdFermata },
	{ "Chant of Disease",                &kBrdChantDisease },
	{ "Chant of Frost",                  &kBrdChantFrost },
	{ "Chant of Poison",                 &kBrdChantPoison },
	{ "Chant of Flame",                  &kBrdChantFlame },
	{ "Resist chants",                   &kBrdChantResist, kFromSpell },
	{ "Denon's Dirge (area dot)",        &kBrdDenon },
	{ "Insult / Disdain (nuke)",         &kBrdInsult },
	{ "Burning Call (nuke)",             &kBrdBurningCall },
	{ "Sticks and Stones",               &kBrdSticks },
	{ "Piper (swarm)",                   &kBrdPiper },
	{ "Aria of Absolution (cure)",       &kBrdAbsolution },
	{ "Requiem",                         &kBrdRequiem },

	{ "AA: Boastful Bellow",             &kAaBoastful },
	{ "AA: Vainglorious Shout",          &kAaVainglorious },
	{ "AA: Bladed Song",                 &kAaBladedSong },
	{ "AA: Dance of Blades",             &kAaDanceBlades },
	{ "AA: Dirge of the Sleepwalker",    &kAaSleepwalker },
	{ "AA: Fierce Eye",                  &kAaFierceEye },
	{ "AA: Flurry of Notes",             &kAaFlurryNotes },
	{ "AA: Funeral Dirge",               &kAaFuneralDirge },
	{ "AA: Hymn of the Last Stand",      &kAaLastStand },
	{ "AA: Lure of the Siren's Song",    &kAaSiren },
	{ "AA: Lyre Leap",                   &kAaLyreLeap },
	{ "AA: Lyrical Prankster",           &kAaPrankster },
	{ "AA: Quick Time",                  &kAaQuickTime },
	{ "AA: Rallying Call / Solo",        &kAaRallying },
	{ "AA: Selo's Kick",                 &kAaSelosKick },
	{ "AA: Shauri's Sonorous Clouding",  &kAaShauri },
	{ "AA: Shield of Notes",             &kAaShieldNotes },
	{ "AA: Sonic Displacement",          &kAaDisplacement },
	{ "AA: Song of Stone",               &kAaSongOfStone },
	{ "AA: Cacophony",                   &kAaCacophony },
	{ "AA: Dissonant Chord",             &kAaDissonant },
	{ "AA: Fading Memories",             &kAaFadingMemories },
	{ "AA: Selo's Sonata",               &kAaSelosSonata },
	{ "AA: Spire of the Minstrels",      &kAaSpireMinstrels },
	{ "AA: Thousand Blades",             &kAaThousandBlades },
	{ "AA: Frenzied Kicks",              &kAaFrenziedKicks },
	{ "AA: Noteworthy Disguise",  &kAaNoteworthyDisguise },
};

const GroupLine kNumbered[] = {
	{ 0, Silence },     { 1, Lullaby },     { 2, Cantata },     { 4, Fatesong },    { 6, Dirge },
	{ 7, Aria },        { 8, Chorus },      { 9, Absolution },  { 10, Voice },      { 11, Slumber },
	{ 13, Crescendo },  { 15, Denon },      { 18, Sonata },     { 19, Potency },    { 20, HasteAria },
	{ 21, ChantDisease }, { 22, ChantFrost }, { 23, ChantPoison }, { 24, ChantFlame }, { 25, Pulse },
	{ 26, WarMarch },   { 27, Arcane },     { 28, Aura },       { 29, Courage },    { 31, Requiem },
	{ 32, Mirror },     { 33, Piper },      { 34, AuraHeal },   { 35, ChantResist }, { 36, ChantResist },
	{ 37, ChantResist }, { 38, ChantResist }, { 39, ChantResist }, { 40, ChantResist }, { 41, Barrier },
	{ 42, WaveSleep },  { 43, Insult },     { 46, Accelerando }, { 47, Suffering }, { 48, Spiteful },
	{ 49, Aura },       { 50, Aura },       { 51, Silence },    { 52, Reckless },   { 55, Reflexive },
	{ 56, ChantResist }, { 57, Binding },   { 58, Binding },    { 59, WarMarch },   { 60, Pious },
	{ 61, BurningCall }, { 62, Insult },    { 63, Assonant },   { 65, Reciprocal }, { 68, Voice },
	{ 69, Sticks },
};

const GroupLine kModern[] = {
	{ 10, Silence },    { 20, Lullaby },    { 40, Fatesong },   { 50, Dirge },      { 60, Aria },
	{ 70, Chorus },     { 90, Voice },      { 100, Slumber },   { 110, Crescendo }, { 130, Sonata },
	{ 140, Potency },   { 150, HasteAria }, { 160, ChantDisease }, { 170, ChantFrost }, { 180, ChantPoison },
	{ 190, ChantFlame }, { 200, Pulse },    { 210, WarMarch },  { 220, Arcane },    { 230, Aura },
	{ 270, Piper },     { 280, AuraHeal },  { 290, Barrier },   { 300, WaveSleep }, { 310, Insult },
	{ 340, Accelerando }, { 350, Suffering }, { 360, Spiteful }, { 370, Aura },     { 380, Aura },
	{ 400, Reckless },  { 430, Reflexive }, { 450, Binding },   { 480, Pious },     { 490, BurningCall },
	{ 500, Insult },    { 510, Assonant },  { 520, Reciprocal }, { 530, Voice },    { 540, Sticks },
};

const GroupLine kOther[] = {
	{ 8000, Silence },  { 8004, Lullaby },  { 8006, Cantata },  { 8008, Barrier },  { 8010, Fatesong },
	{ 8011, Fermata },  { 8012, Dirge },    { 8013, Potency },  { 8014, Chorus },   { 8015, Absolution },
	{ 8016, Voice },    { 8018, Lullaby },  { 8019, Accelerando }, { 8020, Crescendo }, { 8022, Denon },
	{ 8023, HasteAria }, { 8024, Accelerando }, { 8025, Sonata }, { 1508042, Selo },
};

const NamedLine kAbilities[] = {
	{ "boastful bellow",             Boastful },
	{ "vainglorious shout",          Vainglorious },
	{ "bladed song",                 BladedSong },
	{ "dance of blades",             DanceBlades },
	{ "dirge of the sleepwalker",    Sleepwalker },
	{ "fierce eye",                  FierceEye },
	{ "flurry of notes",             FlurryNotes },
	{ "funeral dirge",               FuneralDirge },
	{ "hymn of the last stand",      LastStand },
	{ "lure of the siren's song",    Siren },
	{ "lyre leap",                   LyreLeap },
	{ "lyrical prankster",           Prankster },
	{ "quick time",                  QuickTime },
	{ "rallying call",               Rallying },
	{ "rallying solo",               Rallying },
	{ "selo's kick",                 SelosKick },
	{ "shauri's sonorous clouding",  Shauri },
	{ "shield of notes",             ShieldNotes },
	{ "silent displacement",         Displacement },
	{ "sonic displacement",          Displacement },
	{ "sonic disturbance",           Displacement },
	{ "song of stone",               SongOfStone },
	{ "cacophony",                   Cacophony },
	{ "dissonant chord",             Dissonant },
	{ "fading memories",             FadingMemories },
	{ "selo's sonata",               SelosSonata },
	{ "spire of the minstrels",      SpireMinstrels },
	{ "thousand blades",             ThousandBlades },
	{ "frenzied kicks",              FrenziedKicks },
	{ "noteworthy disguise: blue drake",  NoteworthyDisguise },
};

} // namespace

const ClassCatalogue& BardCatalogue()
{
	static const ClassCatalogue c{ "Bard", eqlib::Bard, kLines, 1, kNumbered, kModern, kOther, kAbilities };
	return c;
}

} // namespace eqgfx::spellfx
