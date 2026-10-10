#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

namespace eqgfx::spellfx {

namespace {

constexpr Pair kSun        { { 1.00f, 0.97f, 0.70f, 1.00f }, { 1.00f, 0.55f, 0.10f, 0.85f } };
constexpr Pair kRo         { { 1.00f, 0.90f, 0.45f, 1.00f }, { 0.95f, 0.20f, 0.05f, 0.90f } };
constexpr Pair kDawn       { { 1.00f, 0.88f, 0.75f, 0.95f }, { 1.00f, 0.45f, 0.55f, 0.85f } };
constexpr Pair kMoon       { { 0.90f, 0.94f, 1.00f, 0.95f }, { 0.55f, 0.55f, 0.95f, 0.80f } };
constexpr Pair kMoonfire   { { 0.85f, 0.90f, 1.00f, 0.95f }, { 0.60f, 0.25f, 0.95f, 0.85f } };
constexpr Pair kStorm      { { 0.85f, 0.92f, 1.00f, 0.95f }, { 0.35f, 0.42f, 0.60f, 0.85f } };
constexpr Pair kHail       { { 1.00f, 1.00f, 1.00f, 1.00f }, { 0.55f, 0.70f, 0.85f, 0.90f } };
constexpr Pair kWinterFire { { 0.75f, 0.95f, 1.00f, 0.95f }, { 1.00f, 0.45f, 0.10f, 0.85f } };
constexpr Pair kBriar      { { 0.30f, 0.45f, 0.15f, 0.95f }, { 0.75f, 0.12f, 0.10f, 0.90f } };
constexpr Pair kFern       { { 0.70f, 1.00f, 0.55f, 0.95f }, { 0.10f, 0.50f, 0.20f, 0.85f } };
constexpr Pair kGrove      { { 0.80f, 1.00f, 0.60f, 0.95f }, { 1.00f, 0.85f, 0.35f, 0.85f } };
constexpr Pair kAutumn     { { 1.00f, 0.75f, 0.25f, 0.95f }, { 0.75f, 0.25f, 0.05f, 0.85f } };
constexpr Pair kBark       { { 0.62f, 0.45f, 0.28f, 0.95f }, { 0.35f, 0.22f, 0.10f, 0.90f } };
constexpr Pair kScale      { { 0.60f, 0.85f, 0.45f, 0.95f }, { 0.25f, 0.40f, 0.20f, 0.90f } };
constexpr Pair kSpore      { { 0.90f, 0.80f, 0.60f, 0.95f }, { 0.55f, 0.30f, 0.65f, 0.85f } };
constexpr Pair kFirefly    { { 1.00f, 1.00f, 0.55f, 1.00f }, { 0.55f, 0.95f, 0.25f, 0.85f } };
constexpr Pair kWildfire   { { 1.00f, 0.95f, 0.40f, 1.00f }, { 0.30f, 0.75f, 0.10f, 0.85f } };
constexpr Pair kPelt       { { 0.85f, 0.85f, 0.88f, 0.95f }, { 0.45f, 0.42f, 0.40f, 0.85f } };

constexpr uint32_t GA_Emerge = GlyphAnim_Rise | GlyphAnim_Pop;

LOOK(kDruSunray, "sunray",
	F(IM, FxForm_Lightning, 1, 38.0f, 64, 3, 0.1f, 4.0f, 1.0f, FIX, kSun, 1.2f, 0.20f),
	G(IM, Glyph_Sun, 1.0f, 9.5f, 4, 1, GA_PopRoll, 0, FIX, kSun, 2.0f),
	F(IM, FxForm_Flame, 0.9f, 5.0f, 80, 20, 0.6f, 1.0f, 0, FIX, kFire, 2.2f, 0.07f, ADD, 1, 0.3f),
	F(IM, FxForm_Ring, 1.3f, 0.3f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kSun, 2.0f, 0.05f, COMET, 1, 0.3f));

LOOK(kDruSunsear, "sunsear",
	F(TR, FxForm_Bolt, 1, 0, 128, 5, 0.5f, 0.45f, 1.4f, FIX, kSun, 0.8f, 0.22f),
	G(IM, Glyph_Sun, 0.9f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kSun, 1.4f),
	F(IM, FxForm_Burst, 2.6f, 1.2f, 96, 3, 1.5f, 20, 0, FIX, kSun, 0.9f, 0.12f),
	F(IM, FxForm_Sparkle, 1.4f, 6.0f, 40, 20, 0.12f, 1, 0, FIX, kFire, 1.6f));

LOOK(kDruSolstice, "solstice",
	F(IM, FxForm_Spire, 0.2f, 10.0f, 96, 16, 0.4f, 4.0f, 0, FIX, kSun, 1.2f, 0.08f),
	G(IM, Glyph_Sun, 1.3f, 11.0f, 4, 1, GA_Drop, 0, FIX, kGold, 1.8f),
	F(IM, FxForm_Burst, 3.0f, 0.4f, 96, 3, 0.6f, 24, 0, FIX, kFire, 1.0f, 0.12f, ADD, 1, 0.35f),
	F(IM, FxForm_Flame, 1.0f, 6.5f, 96, 24, 0.6f, 1.3f, 0, FIX, kSun, 1.6f, 0.08f, ADD, 1, 0.35f));

LOOK(kDruReaping, "reaping inferno",
	F(IM, FxForm_Flame, 1.2f, 8.0f, 112, 28, 0.3f, 2.0f, 0, FIX, kFlame2, 1.8f, 0.10f),
	G(IM, Glyph_Scythe, 1.0f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kAutumn, 1.6f),
	F(IM, FxForm_Burst, 2.6f, 0.4f, 72, 2, 0.4f, 16, 0, FIX, kAutumn, 1.0f, 0.10f, ADD, 1, 0.3f));

LOOK(kDruDaybreak, "daybreak",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.3f, 0.4f, 2.4f, FIX, kDawn, 0.8f, 0.18f),
	G(IM, Glyph_Sun, 0.8f, 1.5f, 4, 1, GA_Emerge, 0, FIX, kDawn, 1.8f),
	F(IM, FxForm_Spire, 0.6f, 6.0f, 72, 12, 1.2f, 2.4f, 0, FIX, kDawn, 1.2f, 0.06f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kSun, 1.8f));

LOOK(kDruHeliacal, "heliacal",
	F(TR, FxForm_Bolt, 1, 0, 128, 6, 0.8f, 0.6f, 3.0f, FIX, kRo, 1.0f, 0.28f),
	G(IM, Glyph_Sun, 1.6f, 9.0f, 4, 1, GA_PopRoll, 0, FIX, kRo, 2.0f),
	F(IM, FxForm_Burst, 3.6f, 1.5f, 120, 3, 2.5f, 24, 0, FIX, kFire, 1.2f, 0.14f, ADD, 1, 0.35f),
	F(IM, FxForm_Flame, 1.3f, 9.0f, 120, 30, 0.4f, 2.2f, 0, FIX, kRo, 1.8f, 0.10f, ADD, 1, 0.35f),
	F(IM, FxForm_Cracks, 2.4f, 0, 48, 6, 1.2f, 0, 0, FIX, kFire, 2.0f, 0.06f, ADD, 1, 0.35f));

LOOK(kDruWrath, "nature's wrath",
	Persist(F(AU, FxForm_Flame, 1.0f, 4.5f, 72, 18, 0.9f, 0.7f, 0, FIX, kWildfire, 0, 0.06f)),
	F(IM, FxForm_Vines, 1.0f, 4.0f, 96, 4, 0.9f, 0, 0, FIX, kWood, 2.4f, 0.06f, OVER),
	G(IM, Glyph_Leaf, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kAutumn, 2.4f));

LOOK(kDruWildfireAura, "wildfire aura",
	Persist(F(AU, FxForm_Rune, 1.0f, 0, 80, 8, 3, 16, 0, FIX, kWildfire, 0, 0.05f, ADD, 1, 0, AE)),
	Persist(F(AU, FxForm_Flame, 1.0f, 1.2f, 96, 24, 0.1f, 0.5f, 0, FIX, kWildfire, 0, 0.05f, ADD, 1, 0, AE)),
	G(IM, Glyph_Flame, 0.8f, 7.5f, 4, 1, GA_PopBob, 0, FIX, kWildfire, 2.4f));

LOOK(kDruGrasp, "grasp of ro",
	G(IM, Glyph_Hand, 1.2f, 9.0f, 4, 1, GA_Drop, 0, FIX, kRo, 2.4f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 96, 24, 0.8f, 0.9f, 0, FIX, kRo, 3.0f, 0.08f, ADD, 1, 0.3f),
	F(IM, FxForm_Ring, 1.1f, 2.5f, 40, 0, 1.5f, 1.0f, 0.5f, FIX, kSun, 3.0f, 0.06f, COMET, 1, 0.3f),
	G(IM, Glyph_ArrowDown, 0.3f, 6.0f, 0, 3, GA_Orbit, 1.3f, FIX, kRo, 2.6f));

LOOK(kDruPillar, "pillar of ro",
	F(IM, FxForm_Flame, 0.6f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kRo, 2.6f, 0.10f),
	G(IM, Glyph_Eye, 1.0f, 12.0f, 4, 1, GA_PopBob, 0, FIX, kSun, 2.6f),
	F(IM, FxForm_Rune, 1.6f, 0, 70, 6, 1, 12, 0, FIX, kRo, 2.8f, 0.05f),
	G(IM, Glyph_ArrowDown, 0.3f, 6.0f, 0, 4, GA_Orbit, 1.4f, FIX, kSun, 2.6f));

LOOK(kDruStormborn, "stormborn",
	F(IM, FxForm_Vortex, 1.4f, 7.0f, 96, 5, 1.4f, 2.5f, 5.0f, FIX, kStorm, 1.6f, 0.07f),
	F(IM, FxForm_Lightning, 1, 38.0f, 64, 2, 0.8f, 14.0f, 1.0f, FIX, kShock, 0.8f, 0.12f, ADD, 1, 0.3f),
	G(IM, Glyph_Tornado, 0.9f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kStorm, 1.8f),
	Stars(0.35f));

LOOK(kDruRoar, "roar",
	F(RE, FxForm_Wind, 1.6f, 6.0f, 72, 14, 3.5f, 0, 0, FIX, kStorm, 1.4f, 0.06f),
	G(IM, Glyph_Mouth, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kStorm, 1.6f),
	F(IM, FxForm_Vortex, 1.8f, 9.0f, 112, 6, 1.6f, 3.0f, -6.0f, FIX, kStorm, 1.8f, 0.08f),
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 3, 1.0f, 16.0f, 1.0f, FIX, kShock, 0.9f, 0.12f, ADD, 1, 0.3f),
	Stars(0.4f));

LOOK(kDruHail, "hail",
	F(IM, FxForm_Rain, 1.4f, 20.0f, 80, 60, 0, 0.2f, 0, FIX, kHail, 2.4f, 0.07f),
	G(IM, Glyph_Rock, 0.3f, 12.0f, 4, 6, GA_Rain, 1.4f, FIX, kHail, 2.2f),
	F(IM, FxForm_Burst, 2.2f, 0.3f, 72, 2, 0.3f, 16, 0, FIX, kCold, 1.0f, 0.09f, ADD, 1, 0.6f),
	F(IM, FxForm_Sparkle, 1.4f, 3.0f, 40, 20, 0.12f, 1, 0, FIX, kHail, 1.8f, 0.06f, ADD, 1, 0.6f));

LOOK(kDruTectonic, "tectonic",
	F(RE, FxForm_Cracks, 1.0f, 0, 96, 10, 1.6f, 0, 0, FIX, kEarth, 2.4f, 0.09f, OVER, 1, 0, AE),
	F(RE, FxForm_Crystal, 1.0f, 3.0f, 60, 12, 1, 0.9f, 0, FIX, kStone, 2.2f, 0.08f, OVER, 1, 0.2f, AE),
	F(RE, FxForm_Burst, 1.0f, 0.2f, 96, 3, 0.3f, 0, 0, FIX, kEarth, 1.4f, 0.12f, OVER, 1, 0, AE),
	G(RE, Glyph_Rock, 0.4f, 1.0f, 3, 6, GlyphAnim_Rise | GlyphAnim_Spin | GlyphAnim_Bob, 2.4f, FIX, kStone, 2.2f, 0, OVER));

LOOK(kDruFrost, "frost",
	F(IM, FxForm_Crystal, 1.1f, 5.5f, 60, 12, 0, 0.8f, 0, FIX, kCold, 2.6f, 0.06f),
	G(IM, Glyph_Snowflake, 0.9f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kCold, 2.2f),
	G(IM, Glyph_ArrowDown, 0.3f, 5.5f, 0, 3, GA_Orbit, 1.2f, FIX, kMoon, 2.4f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 32, 16, 0.10f, 1, 0, FIX, kWhite, 2.0f));

LOOK(kDruBreath, "breath",
	F(TR, FxForm_Drain, 1, 0, 64, 4, 0.6f, 16, 0, FIX, kCold, 1.2f, 0.10f),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 12, 3.0f, 0, 0, FIX, kCold, 1.6f, 0.05f),
	F(IM, FxForm_Crystal, 1.0f, 4.0f, 40, 8, 0, 0.6f, 0, FIX, kHail, 2.0f, 0.05f, ADD, 1, 0.3f),
	G(IM, Glyph_Snowflake, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kCold, 2.2f));

LOOK(kDruCrystals, "crystals",
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 0.4f, 0.5f, 1.2f, FIX, kCold, 0.8f, 0.20f),
	F(IM, FxForm_Crystal, 1.8f, 3.5f, 72, 14, 1, 1.3f, 0, FIX, kCold, 2.0f, 0.08f),
	F(IM, FxForm_Burst, 2.6f, 1.0f, 96, 3, 1.0f, 20, 0, FIX, kHail, 0.9f, 0.12f),
	G(IM, Glyph_Snowflake, 0.4f, 6.0f, 4, 5, GA_Rain, 1.4f, FIX, kHail, 1.8f));

LOOK(kDruChill, "chill",
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 0.8f, 0.5f, 2.0f, FIX, kMoon, 0.8f, 0.20f),
	G(IM, Glyph_Moon, 1.1f, 9.0f, 4, 1, GA_PopRoll, 0, FIX, kMoon, 1.8f),
	F(IM, FxForm_Crystal, 1.3f, 5.0f, 60, 12, 0, 0.9f, 0, FIX, kCold, 1.8f, 0.06f, ADD, 1, 0.3f),
	F(IM, FxForm_Burst, 2.6f, 2.0f, 96, 3, 1.5f, 16, 0, FIX, kMoon, 1.0f, 0.12f, ADD, 1, 0.3f));

LOOK(kDruNatureward, "natureward",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.4f, 1.0f, FIX, kCold, 0.8f, 0.18f),
	F(IM, FxForm_Crystal, 1.1f, 4.5f, 48, 10, 0, 0.8f, 0, FIX, kCold, 1.8f, 0.06f),
	G(IM, Glyph_Leaf, 0.35f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kHeal, 2.2f, 0.3f),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 32, 16, 0.10f, 0, 0, FIX, kHeal, 1.8f, 0.06f, ADD, 1, 0.3f));

LOOK(kDruMoonbeam, "moonbeam",
	F(IM, FxForm_Lightning, 1, 38.0f, 48, 1, 0.02f, 0.4f, 1.0f, FIX, kMoonfire, 1.8f, 0.30f),
	G(IM, Glyph_Moon, 1.0f, 10.0f, 4, 1, GA_PopBob, 0, FIX, kMoon, 2.4f),
	Persist(F(AU, FxForm_Sparkle, 1.1f, 7.0f, 24, 12, 0.10f, 1, 0, FIX, kMoonfire, 0)),
	F(IM, FxForm_Ring, 1.2f, 0.3f, 40, 0, 0.8f, 1.0f, 0.4f, FIX, kMoon, 2.4f, 0.05f, COMET));

LOOK(kDruFrostAura, "frost aura",
	Persist(F(AU, FxForm_Rune, 1.0f, 0, 80, 6, 1, 24, 0, FIX, kCold, 0, 0.05f, ADD, 1, 0, AE)),
	Persist(F(AU, FxForm_Sparkle, 1.0f, 4.0f, 48, 24, 0.10f, 1, 0, FIX, kHail, 0, 0.05f, ADD, 1, 0, AE)),
	G(IM, Glyph_Snowflake, 0.8f, 7.5f, 4, 1, GA_PopRoll, 0, FIX, kCold, 2.4f));

LOOK(kDruWinters, "winter's flame",
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 0.7f, 0.5f, 1.4f, FIX, kWinterFire, 0.8f, 0.22f),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 96, 24, 0.7f, 1.2f, 0, FIX, kWinterFire, 1.8f, 0.08f),
	G(IM, Glyph_Snowflake, 0.4f, 5.5f, 0, 2, GA_Orbit, 1.3f, FIX, kCold, 2.0f),
	G(IM, Glyph_Flame, 0.4f, 5.5f, 0, 2, GA_Orbit, 1.3f, FIX, kFire, 2.0f));

LOOK(kDruFrostbite, "frostbite",
	F(IM, FxForm_Crystal, 1.0f, 4.0f, 40, 8, 0, 0.6f, 0, FIX, kCold, 1.4f, 0.05f),
	F(IM, FxForm_Burst, 1.6f, 3.0f, 48, 1, 0.3f, 10, 0, FIX, kCold, 0.6f, 0.07f));

LOOK(kDruBurn, "burn",
	F(IM, FxForm_Flame, 0.9f, 4.5f, 64, 16, 0.7f, 0.8f, 0, FIX, kFire, 1.4f, 0.06f),
	F(IM, FxForm_Burst, 1.6f, 3.0f, 48, 1, 0.3f, 10, 0, FIX, kFire, 0.6f, 0.07f));

LOOK(kDruWinds, "winds",
	F(IM, FxForm_Wind, 1.4f, 5.0f, 72, 12, 3.0f, 0, 0, FIX, kAir, 1.8f, 0.05f),
	G(IM, Glyph_Sun, 0.6f, 7.5f, 3, 1, GA_PopRoll, 0.9f, FIX, kSun, 2.0f),
	G(IM, Glyph_Moon, 0.6f, 7.5f, 3, 1, GA_PopRoll, 0.9f, FIX, kMoon, 2.0f, 0.2f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, 4.0f, FIX, kStorm, 1.6f, 0.05f));

LOOK(kDruRemoteSun, "remote sunflare",
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 3, 0.2f, 6.0f, 1.0f, FIX, kSun, 1.0f, 0.18f),
	F(IM, FxForm_Burst, 2.6f, 0.5f, 96, 3, 1.2f, 20, 0, FIX, kSun, 1.0f, 0.12f, ADD, 1, 0.2f),
	G(IM, Glyph_Sun, 0.9f, 8.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kFire, 1.4f));

LOOK(kDruRemoteMoon, "remote moonfire",
	F(IM, FxForm_Lightning, 1, 40.0f, 64, 3, 0.2f, 6.0f, 1.0f, FIX, kMoonfire, 1.0f, 0.18f),
	F(IM, FxForm_Burst, 2.6f, 0.5f, 96, 3, 1.2f, 20, 0, FIX, kMoonfire, 1.0f, 0.12f, ADD, 1, 0.2f),
	G(IM, Glyph_Moon, 0.9f, 8.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kMoon, 1.4f));

LOOK(kDruSunEnervation, "sun enervation",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.0f, 12, 0, FIX, kSun, 1.4f, 0.07f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 24, 12, 0.10f, 2, 0, FIX, kMana, 1.4f));

LOOK(kDruMoonEnervation, "moon enervation",
	F(TR, FxForm_Drain, 1, 0, 48, 3, 1.0f, 12, 0, FIX, kMoonfire, 1.4f, 0.07f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 24, 12, 0.10f, 2, 0, FIX, kMoon, 1.4f));

LOOK(kDruVida, "vida",
	G(IM, Glyph_Leaf, 1.0f, 9.0f, 4, 1, GA_Drop, 0, FIX, kHeal, 2.2f),
	F(IM, FxForm_Flame, 0.8f, 8.0f, 72, 18, 0.1f, 2.4f, 0, FIX, kHeal, 1.8f, 0.07f, ADD, 1, 0.35f),
	F(IM, FxForm_Burst, 2.0f, 0.5f, 64, 2, 1.5f, 12, 0, FIX, kFern, 1.0f, 0.08f, ADD, 1, 0.35f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 40, 20, 0.12f, 1, 0, FIX, kHeal, 2.0f));

LOOK(kDruLuna, "luna",
	G(IM, Glyph_Moon, 1.2f, 9.5f, 4, 1, GA_Drop, 0, FIX, kMoon, 2.6f),
	F(IM, FxForm_Lightning, 1, 36.0f, 48, 1, 0.03f, 0.5f, 1.0f, FIX, kMoon, 1.2f, 0.30f, ADD, 1, 0.3f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 72, 3, 1.2f, 0, 0.5f, FIX, kHeal, 2.4f, 0.05f, COMET, 1, 0.3f),
	F(IM, FxForm_Sparkle, 1.2f, 8.0f, 48, 24, 0.12f, 1, 0, FIX, kMoon, 2.4f));

LOOK(kDruAdrenaline, "adrenaline",
	G(IM, Glyph_Heart, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kHeal, 1.8f),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 64, 2, 2.5f, 12, 0, FIX, kHeal, 0.9f, 0.08f),
	F(IM, FxForm_Wind, 1.2f, 5.0f, 60, 10, 3.0f, 0, 0, FIX, kFern, 1.4f, 0.05f),
	F(IM, FxForm_Spire, 0.6f, 6.0f, 64, 12, 1.0f, 2.4f, 0, FIX, kHeal, 1.2f, 0.05f));

LOOK(kDruRejuv, "rejuvenation",
	G(RE, Glyph_Leaf, 0.7f, 3.0f, 3, 6, GA_RingRise, 2.6f, FIX, kFern, 2.6f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 4, 0, 0, 0, FIX, kHeal, 2.2f, 0.07f, ADD, 1, 0, AE),
	F(RE, FxForm_Vines, 1.0f, 1.5f, 120, 6, 0.4f, 0, 1.0f, FIX, kFern, 2.4f, 0.06f, ADD, 1, 0.1f, AE),
	F(IM, FxForm_Sparkle, 1.1f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kHeal, 2.0f));

LOOK(kDruSurvival, "survival",
	G(IM, Glyph_Tree, 1.0f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kGrove, 3.0f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 96, 2, 1.0f, 0, 0.4f, FIX, kHeal, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Ring, 1.4f, 0.3f, 48, 0, 0.8f, 1.0f, 0.4f, FIX, kGrove, 3.0f, 0.05f, COMET));

LOOK(kDruPromised, "promised",
	G(IM, Glyph_Hourglass, 0.8f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kGrove, 3.0f),
	F(IM, FxForm_Vines, 1.0f, 3.0f, 96, 4, 0.6f, 0, 1.0f, FIX, kFern, 3.0f, 0.05f),
	G(IM, Glyph_Leaf, 0.25f, 4.0f, 0, 4, GA_Orbit, 1.2f, FIX, kHeal, 3.0f));

LOOK(kDruSunBlessing, "sun blessing",
	F(TR, FxForm_Bolt, 1, 0, 96, 4, 0.5f, 0.4f, 1.4f, FIX, kSun, 0.8f, 0.18f),
	F(IM, FxForm_Burst, 2.4f, 1.0f, 72, 2, 1.5f, 16, 0, FIX, kSun, 0.9f, 0.10f),
	G(RE, Glyph_Sun, 0.5f, 7.0f, 0, 3, GA_Orbit, 1.3f, FIX, kHealGold, 2.0f),
	F(RE, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kHealGold, 2.0f, 0.06f, ADD, 1, 0.4f));

LOOK(kDruGrowth, "growth",
	F(IM, FxForm_Vines, 1.1f, 6.5f, 120, 6, 1.2f, 0, 0, FIX, kFern, 3.0f, 0.07f, OVER),
	G(IM, Glyph_Tree, 0.9f, 8.5f, 4, 1, GA_Emerge, 0, FIX, kFern, 3.0f),
	G(IM, Glyph_ArrowUp, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kHeal, 2.6f));

LOOK(kDruNurturing, "nurturing growth",
	G(RE, Glyph_Tree, 1.2f, 3.0f, 4, 1, GA_Emerge, 0, FIX, kGrove, 3.5f),
	F(RE, FxForm_Vines, 1.3f, 3.0f, 120, 6, 0.5f, 0, 1.0f, FIX, kWood, 3.5f, 0.07f, OVER),
	F(RE, FxForm_Bloom, 2.4f, 1.0f, 48, 3, 0, 0, 0, FIX, kHeal, 3.0f, 0.05f),
	F(RE, FxForm_Sparkle, 1.6f, 6.0f, 40, 20, 0.12f, 0, 0, FIX, kGrove, 3.0f));

LOOK(kDruResponse, "nature's response",
	Persist(F(AU, FxForm_Ring, 1.2f, 3.5f, 40, 0, 0.8f, 1.0f, 0.4f, FIX, kGrove, 0, 0.05f, COMET)),
	G(IM, Glyph_Leaf, 0.4f, 4.0f, 3, 4, GA_RingRise, 1.4f, FIX, kGrove, 2.6f),
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kFern, 2.2f, 0.03f));

LOOK(kDruPreinc, "preincarnation",
	G(IM, Glyph_Ankh, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGrove, 3.0f),
	F(IM, FxForm_Vines, 1.0f, 5.0f, 96, 4, 1.0f, 0, 0, FIX, kFern, 3.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.2f, 8.0f, 32, 16, 0.12f, 1, 0, FIX, kGrove, 3.0f));

LOOK(kDruRegen, "regeneration",
	Persist(F(AU, FxForm_Orbit, 1.2f, 6.0f, 40, 10, 0, 0, 0, FIX, kFern, 0, 0.04f, ADD, 1.5f)),
	G(IM, Glyph_Leaf, 0.3f, 5.0f, 0, 3, GA_PopAscend | GlyphAnim_Spin, 1.1f, FIX, kHeal, 2.4f),
	F(IM, FxForm_Ring, 1.1f, 0.3f, 40, 0, 1.0f, 1.0f, 0.4f, FIX, kFern, 2.4f, 0.05f, COMET));

LOOK(kDruBreeze, "breeze",
	F(RE, FxForm_Wind, 1.4f, 5.0f, 72, 12, 2.5f, 0, 0, FIX, kAir, 2.0f, 0.05f),
	F(RE, FxForm_Bloom, 1.0f, 2.0f, 64, 3, 0, 0, 0, FIX, kFeather, 2.0f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Leaf, 0.35f, 6.0f, 0, 5, GA_Orbit | GlyphAnim_Ascend, 1.6f, FIX, kFern, 2.2f));

LOOK(kDruBlood, "blood",
	G(IM, Glyph_Drop, 0.8f, 8.5f, 4, 1, GA_Drop, 0, FIX, kBlood, 2.0f),
	F(IM, FxForm_Bloom, 1.4f, 1.0f, 40, 3, 0, 0, 0, FIX, kHeal, 2.0f, 0.05f, ADD, 1, 0.35f),
	F(IM, FxForm_Bubbles, 0.9f, 6.0f, 48, 6, 0.18f, 0, 0, FIX, kHeal, 2.0f, 0.035f, ADD, 1, 0.35f));

LOOK(kDruCorruption, "corruption",
	G(IM, Glyph_Leaf, 0.8f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kFern, 2.0f),
	F(IM, FxForm_Drain, 1, 0, 32, 2, 0.8f, 10, 0, FIX, kCorrupt, 1.2f, 0.06f, OVER | FxFlag_Reverse),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kFern, 2.0f));

LOOK(kDruSkin, "skin",
	F(IM, FxForm_Crystal, 1.0f, 5.5f, 60, 12, 0, 0.6f, 0, FIX, kBark, 2.6f, 0.07f, OVER),
	G(IM, Glyph_Tree, 0.8f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kWood, 2.6f),
	F(IM, FxForm_Ring, 1.2f, 0.3f, 40, 0, 0.8f, 1.0f, 0.4f, FIX, kWood, 2.4f, 0.05f, COMET));

LOOK(kDruBlessing, "blessing",
	G(RE, Glyph_Tree, 1.2f, 9.0f, 4, 1, GA_PopBob, 0, FIX, kWood, 2.8f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kBark, 1.4f, 0.10f, OVER, 1, 0.2f, AE),
	F(IM, FxForm_Crystal, 1.0f, 5.0f, 48, 10, 0, 0.5f, 0, FIX, kBark, 2.4f, 0.06f, OVER),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kFern, 2.2f));

LOOK(kDruSkinTo, "skin to vines",
	Persist(F(AU, FxForm_Vines, 1.05f, 5.5f, 96, 4, 1.4f, 0, 0, FIX, kFern, 0, 0.05f, OVER | FxFlag_NoEnvelope)),
	G(IM, Glyph_Leaf, 0.7f, 7.8f, 4, 1, GA_PopAscend, 0, FIX, kFern, 2.4f));

LOOK(kDruReptile, "reptile",
	G(IM, Glyph_Snake, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kScale, 3.0f),
	F(IM, FxForm_Lattice, 1.2f, 6.0f, 72, 12, 0.5f, 0, 0, FIX, kScale, 2.8f, 0.05f, OVER),
	F(IM, FxForm_Sparkle, 1.2f, 6.0f, 24, 12, 0.10f, 0, 0, FIX, kScale, 2.4f));

LOOK(kDruGuard, "beastwood guard",
	G(IM, Glyph_Tree, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kBark, 3.0f, 0, OVER),
	G(IM, Glyph_Claw, 0.35f, 4.0f, 3, 3, GA_Orbit, 1.4f, FIX, kWood, 3.0f),
	F(IM, FxForm_Dome, 1.6f, 6.4f, 88, 6, 2, 0, 0, FIX, kWood, 2.4f, 0.04f));

LOOK(kDruMammoth, "mammoth",
	G(IM, Glyph_Fist, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kEarth, 2.6f),
	F(IM, FxForm_Cracks, 1.8f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 2.4f, 0.06f, OVER),
	G(IM, Glyph_ArrowUp, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kGold, 2.4f));

LOOK(kDruMask, "mask",
	G(IM, Glyph_Mask, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kFern, 2.8f),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 2, 0, FIX, kMana, 2.4f),
	G(IM, Glyph_Paw, 0.25f, 4.5f, 0, 4, GA_Orbit, 1.2f, FIX, kFern, 2.6f));

LOOK(kDruEyes, "sharp eyes",
	G(IM, Glyph_Eye, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGold, 2.6f),
	G(IM, Glyph_Feather, 0.3f, 5.5f, 0, 3, GA_Orbit, 1.3f, FIX, kFeather, 2.6f));

LOOK(kDruUnity, "unity",
	G(IM, Glyph_Tree, 0.8f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGrove, 3.0f),
	F(IM, FxForm_Vines, 1.0f, 3.5f, 96, 4, 0.8f, 0, 0, FIX, kWood, 3.0f, 0.06f, OVER),
	F(IM, FxForm_Crystal, 1.0f, 5.0f, 48, 10, 0, 0.5f, 0, FIX, kBark, 2.6f, 0.05f, OVER),
	G(IM, Glyph_Leaf, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kGrove, 3.0f));

LOOK(kDruAbsolution, "arboreal absolution",
	G(IM, Glyph_Tree, 0.8f, 8.0f, 4, 1, GA_PopRoll, 0, FIX, kGrove, 2.8f),
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 2, FIX, kGrove, 2.8f, 0.05f),
	F(IM, FxForm_Sparkle, 1.3f, 7.0f, 32, 16, 0.12f, 0, 0, FIX, kFirefly, 2.4f));

LOOK(kDruWays, "ways",
	G(IM, Glyph_Spiral, 0.8f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kAurora, 3.0f),
	F(IM, FxForm_Orbit, 1.3f, 6.0f, 48, 10, 0, 0, 0, FIX, kFirefly, 3.0f, 0.04f, ADD, 2.0f));

LOOK(kDruAlliance, "alliance",
	G(IM, Glyph_Tree, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kGrove, 3.0f),
	G(IM, Glyph_Link, 0.3f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kGold, 3.0f),
	F(IM, FxForm_Burst, 2.0f, 1.5f, 48, 1, 1.5f, 12, 0, FIX, kGrove, 0.9f, 0.08f));

LOOK(kDruFortitude, "fortitude",
	F(IM, FxForm_Dome, 1.8f, 7.0f, 96, 8, 3, 0, 0, FIX, kGrove, 2.6f, 0.04f),
	G(IM, Glyph_Shield, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kWood, 2.6f),
	F(IM, FxForm_Vines, 1.0f, 2.0f, 96, 4, 0.5f, 0, 1.0f, FIX, kWood, 2.6f, 0.06f, OVER));

LOOK(kDruTwincast, "twincast",
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 2, FIX, kSun, 2.6f, 0.05f),
	F(IM, FxForm_Rune, 1.5f, 3.4f, 60, 6, 1, 12, 2, FIX, kMoon, 2.6f, 0.05f, ADD, -1.0f),
	G(IM, Glyph_Leaf, 0.5f, 7.6f, 4, 1, GA_PopRoll, 0, FIX, kGrove, 2.6f));

LOOK(kDruCoat, "coat",
	Persist(F(AU, FxForm_Vines,  1.10f, 5.5f, 120, 5, 1.1f, 0, 0, FIX, kWood, 0, 0.06f, OVER | FxFlag_NoEnvelope)),
	Persist(F(AU, FxForm_Thorns, 1.10f, 5.5f,  30, 5, 1.1f, 6, 0, FIX, kThorn, 0, 0.05f, OVER | FxFlag_NoEnvelope)),
	F(IM, FxForm_Burst, 2.2f, 0, 48, 1, 1.0f, 12, 0, FIX, kWood, 0.8f, 0.08f));

LOOK(kDruLegacy, "legacy",
	F(RE, FxForm_Vines, 1.0f, 1.5f, 120, 6, 0.4f, 0, 1.0f, FIX, kWood, 2.2f, 0.07f, OVER, 1, 0, AE),
	F(RE, FxForm_Thorns, 1.0f, 1.5f, 24, 6, 0.4f, 4, 1.0f, FIX, kThorn, 2.2f, 0.05f, OVER, 1, 0, AE),
	Persist(F(AU, FxForm_Thorns, 1.10f, 5.5f, 30, 5, 1.1f, 6, 0, FIX, kThorn, 0, 0.05f, OVER | FxFlag_NoEnvelope)));

LOOK(kDruBulwark, "bulwark",
	Persist(F(AU, FxForm_Thorns, 1.15f, 6.0f, 36, 6, 1.3f, 6, 0, FIX, kBriar, 0, 0.05f, OVER | FxFlag_NoEnvelope)),
	F(IM, FxForm_Dome, 1.7f, 6.6f, 96, 8, 3, 0, 0, FIX, kFern, 2.2f, 0.04f),
	G(IM, Glyph_Shield, 0.8f, 7.6f, 4, 1, GA_PopAscend, 0, FIX, kWood, 2.4f));

LOOK(kDruSpur, "spur",
	F(IM, FxForm_Thorns, 1.1f, 5.0f, 32, 4, 1.0f, 8, 0, FIX, kThorn, 2.6f, 0.06f, OVER),
	G(IM, Glyph_Fang, 0.6f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kThorn, 2.0f),
	F(IM, FxForm_Burst, 1.8f, 1.5f, 48, 1, 0.8f, 12, 0, FIX, kFern, 0.8f, 0.07f));

LOOK(kDruSurgeVines, "surge of vines",
	F(IM, FxForm_Vines, 1.2f, 7.0f, 120, 6, 1.6f, 0, 1.0f, FIX, kBriar, 2.4f, 0.08f, OVER),
	F(IM, FxForm_Thorns, 1.2f, 7.0f, 36, 6, 1.6f, 6, 1.0f, FIX, kThorn, 2.4f, 0.06f, OVER),
	F(IM, FxForm_Burst, 2.4f, 0.3f, 64, 2, 0.4f, 12, 0, FIX, kBriar, 1.0f, 0.09f));

LOOK(kDruRush, "rush",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.2f, 0.6f, 0.8f, FIX, kBriar, 0.7f, 0.14f, OVER),
	F(IM, FxForm_Thorns, 1.0f, 4.0f, 24, 4, 0.8f, 6, 1.0f, FIX, kThorn, 1.6f, 0.06f, OVER),
	G(IM, Glyph_Fang, 0.4f, 5.0f, 3, 3, GA_RingRise, 1.2f, FIX, kBriar, 1.6f));

LOOK(kDruVinelash, "vinelash",
	F(AU, FxForm_Vines, 1.0f, 2.2f, 120, 6, 0.5f, 0, 1.0f, FIX, kWood, 5.0f, 0.08f, OVER),
	F(AU, FxForm_Thorns, 1.0f, 2.2f, 24, 6, 0.5f, 4, 1.0f, FIX, kThorn, 5.0f, 0.06f, OVER),
	F(IM, FxForm_Cracks, 1.6f, 0, 48, 6, 1.0f, 0, 0, FIX, kEarth, 3.0f, 0.06f, OVER));

LOOK(kDruThornmawVines, "thornmaw vines",
	F(AU, FxForm_Vines, 1.0f, 1.2f, 96, 4, 0.3f, 0, 1.0f, FIX, kBriar, 4.0f, 0.07f, OVER),
	G(IM, Glyph_Link, 0.3f, 0.6f, 3, 4, GA_PopSpin, 1.0f, FIX, kWood, 3.0f));

LOOK(kDruThornmaw, "thornmaw",
	F(IM, FxForm_Thorns, 1.0f, 3.0f, 30, 5, 0.6f, 6, 1.0f, FIX, kBriar, 2.4f, 0.06f, OVER),
	G(IM, Glyph_Fang, 1.0f, 6.5f, 4, 1, GA_PopBob, 0, FIX, kBriar, 1.8f),
	F(IM, FxForm_Vines, 1.0f, 3.0f, 100, 5, 0.6f, 0, 1.0f, FIX, kWood, 2.4f, 0.07f, OVER));

LOOK(kDruHorde, "horde",
	F(TR, FxForm_Drain, 1, 0, 64, 4, 1.4f, 16, 0, FIX, kFirefly, 1.4f, 0.06f),
	G(IM, Glyph_Insect, 0.35f, 5.5f, 0, 6, GA_Orbit, 1.4f, FIX, kAmber, 2.0f),
	F(IM, FxForm_Orbit, 1.3f, 6.0f, 60, 12, 0, 0, 0, FIX, kFire, 2.0f, 0.04f, ADD, 3.0f));

LOOK(kDruAssault, "spore assault",
	F(IM, FxForm_Bubbles, 1.4f, 3.0f, 64, 10, 0.3f, 0, 0, FIX, kSpore, 2.6f, 0.04f),
	F(IM, FxForm_Cracks, 2.0f, 0, 48, 6, 1.0f, 0, 0, FIX, kSpore, 2.6f, 0.06f, OVER),
	F(IM, FxForm_Sparkle, 1.6f, 4.0f, 48, 24, 0.12f, 0, 0, FIX, kSpore, 2.4f),
	G(IM, Glyph_Leaf, 0.4f, 1.5f, 3, 4, GA_Emerge, 1.6f, FIX, kSpore, 2.6f));

LOOK(kDruCharm, "beast's charm",
	G(IM, Glyph_Paw, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kFern, 3.0f),
	G(IM, Glyph_Heart, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.2f, FIX, kPink, 3.0f),
	F(IM, FxForm_Ring, 1.2f, 7.0f, 40, 0, 1.2f, 1.0f, 0.5f, FIX, kFern, 3.0f, 0.05f, COMET));

LOOK(kDruCalm, "nature's calm",
	G(IM, Glyph_Leaf, 0.35f, 8.0f, 4, 5, GA_Rain, 1.4f, FIX, kFern, 2.6f),
	F(IM, FxForm_Bloom, 1.8f, 2.0f, 40, 2, 0, 0, 0, FIX, kFern, 2.4f, 0.04f),
	G(IM, Glyph_Dove, 0.6f, 7.0f, 4, 1, GA_PopAscend, 0, FIX, kWhite, 2.4f));

LOOK(kDruChant, "chant",
	Persist(F(AU, FxForm_Vines, 1.0f, 4.5f, 96, 4, 1.0f, 0, 0, FIX, kCorrupt, 0, 0.05f, OVER)),
	G(IM, Glyph_Snake, 0.7f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kPoison, 2.4f),
	F(IM, FxForm_Bubbles, 1.0f, 5.0f, 32, 6, 0.18f, 0, 0, FIX, kPoison, 2.2f, 0.035f));

LOOK(kDruSummonItem, "carve",
	G(IM, Glyph_Leaf, 0.5f, 5.0f, 4, 1, GA_PopAscend, 0, FIX, kWood, 1.8f, 0.3f),
	F(IM, FxForm_Sparkle, 1.2f, 5.0f, 32, 16, 0.12f, 2, 0, FIX, kWood, 2.0f));

LOOK(kDruUnnatural, "annihilate the unnatural",
	G(IM, Glyph_Tree, 1.2f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kFern, 1.2f),
	F(IM, FxForm_Vortex, 1.4f, 6.0f, 72, 5, 1.2f, 2.5f, 6.0f, FIX, kAir, 1.2f, 0.06f),
	F(IM, FxForm_Burst, 2.4f, 2.0f, 72, 2, 0, 16, 0, FIX, kFern, 0.9f, 0.10f, ADD | FxFlag_Reverse, 1, 0.3f));

LOOK(kDruWolf, "spirit of wolf",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 12, 3.5f, 0, 0, FIX, kPelt, 2.6f, 0.05f),
	G(IM, Glyph_Wolf, 0.6f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kPelt, 1.8f));

LOOK(kDruEagle, "eagle",
	F(IM, FxForm_Wings, 1.0f, 4.3f, 58, 7, 0.6f, 3.2f, 0, FIX, kFeather, 2.8f, 0.06f),
	G(IM, Glyph_Feather, 0.35f, 5.0f, 0, 4, GA_Orbit | GlyphAnim_Ascend, 1.3f, FIX, kFeather, 2.6f),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 60, 10, 2.5f, 0, 0, FIX, kAir, 2.4f, 0.04f));

LOOK(kDruRing, "ring",
	F(RE, FxForm_Rune, 2.2f, 0, 80, 8, 3, 16, 0, FIX, kFern, 3.0f, 0.05f),
	F(RE, FxForm_Vortex, 1.6f, 7.0f, 96, 5, 0.8f, 2.5f, 5.0f, FIX, kGrove, 3.0f, 0.06f),
	G(RE, Glyph_Door, 1.1f, 7.0f, 4, 1, GA_Emerge, 0, FIX, kGrove, 3.0f));

LOOK(kDruCircle, "circle",
	F(RE, FxForm_Rune, 1.0f, 0, 96, 12, 5, 24, 0, FIX, kFern, 3.0f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Tree, 0.7f, 2.0f, 3, 6, GA_RingRise, 2.6f, FIX, kGrove, 3.0f),
	F(RE, FxForm_Vortex, 1.8f, 8.0f, 112, 6, 0.8f, 3.0f, 5.0f, FIX, kGrove, 3.0f, 0.07f),
	F(RE, FxForm_Sparkle, 2.0f, 8.0f, 60, 30, 0.12f, 0, 0, FIX, kFirefly, 3.0f));

LOOK(kDruZephyr, "zephyr",
	F(IM, FxForm_Vortex, 1.4f, 7.0f, 96, 5, 1.0f, 2.5f, 6.0f, FIX, kAir, 2.8f, 0.06f),
	G(IM, Glyph_Tornado, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kAir, 2.6f),
	G(IM, Glyph_Leaf, 0.3f, 5.0f, 0, 4, GA_Orbit | GlyphAnim_Ascend, 1.4f, FIX, kFern, 2.6f));

LOOK(kDruSuccor, "succor",
	F(RE, FxForm_Wind, 1.4f, 6.0f, 72, 14, 3.5f, 0, 0, FIX, kFern, 2.0f, 0.05f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 64, 3, 0, 0, 0, FIX, kGrove, 2.0f, 0.05f, ADD | FxFlag_Reverse, 1, 0, AE),
	G(RE, Glyph_Door, 0.9f, 7.0f, 4, 1, GA_PopBob, 0, FIX, kFern, 2.2f));

LOOK(kDruProcFrostHeal, "frost heal",
	G(IM, Glyph_Snowflake, 0.35f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kHeal, 1.4f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 20, 10, 0.10f, 0, 0, FIX, kHeal, 1.2f));

LOOK(kDruProcSunHeal, "sun blessing trigger",
	G(IM, Glyph_Sun, 0.35f, 6.8f, 4, 1, GA_PopAscend, 0, FIX, kHealGold, 1.4f),
	F(IM, FxForm_Sparkle, 1.0f, 6.0f, 20, 10, 0.10f, 0, 0, FIX, kHealGold, 1.2f));

LOOK(kDruProcRo, "ro's reflection",
	F(IM, FxForm_Ring, 1.0f, 4.0f, 32, 0, 2.0f, 1.0f, 0.8f, FIX, kRo, 1.0f, 0.04f, COMET),
	F(IM, FxForm_Sparkle, 1.0f, 6.5f, 24, 12, 0.10f, 1, 0, FIX, kHealGold, 1.2f));

LOOK(kDruProcGlint, "triggered effect",
	F(IM, FxForm_Sparkle, 1.0f, 6.5f, 24, 12, 0.10f, 0, 0, FIX, kFern, 1.0f),
	F(IM, FxForm_Ring, 1.0f, 3.0f, 32, 0, 2.0f, 1.0f, 0.8f, FIX, kFern, 1.0f, 0.04f, COMET));

LOOK(kAaApex, "apex of nature",
	G(IM, Glyph_Tree, 1.4f, 9.5f, 4, 1, GA_Emerge, 0, FIX, kGrove, 3.5f),
	F(IM, FxForm_Vines, 1.2f, 8.0f, 120, 6, 1.4f, 0, 0, FIX, kWood, 3.5f, 0.08f, OVER),
	F(IM, FxForm_Thorns, 1.2f, 8.0f, 36, 6, 1.4f, 6, 0, FIX, kThorn, 3.5f, 0.06f, OVER),
	F(IM, FxForm_Burst, 3.0f, 0.4f, 96, 3, 1.0f, 20, 0, FIX, kGrove, 1.4f, 0.12f),
	F(IM, FxForm_Sparkle, 1.6f, 10.0f, 60, 30, 0.14f, 1, 0, FIX, kFirefly, 3.5f));

LOOK(kAaBlessingRo, "blessing of ro",
	G(RE, Glyph_Sun, 1.3f, 10.0f, 4, 1, GA_PopRoll, 0, FIX, kRo, 2.6f),
	F(RE, FxForm_Spire, 0.8f, 9.0f, 96, 16, 0.8f, 3.0f, 0, FIX, kSun, 2.0f, 0.07f),
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 0.6f, 0.5f, 2.0f, FIX, kRo, 0.9f, 0.22f),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 96, 24, 0.7f, 1.2f, 0, FIX, kRo, 2.4f, 0.08f));

LOOK(kAaVortexRo, "vortex of ro",
	F(IM, FxForm_Vortex, 1.6f, 8.0f, 112, 6, 1.4f, 3.0f, 6.0f, FIX, kRo, 2.4f, 0.08f),
	G(IM, Glyph_Sun, 0.8f, 9.0f, 4, 1, GA_PopRoll, 0, FIX, kSun, 2.2f),
	F(IM, FxForm_Sparkle, 1.6f, 8.0f, 48, 24, 0.12f, 2, 0, FIX, kFire, 2.2f));

LOOK(kAaBlessingTunare, "blessing of tunare",
	G(RE, Glyph_Tree, 1.3f, 9.5f, 4, 1, GA_PopBob, 0, FIX, kGrove, 3.0f),
	F(RE, FxForm_Burst, 1.0f, 0.5f, 120, 3, 2.5f, 24, 0, FIX, kHeal, 1.8f, 0.12f, ADD, 1, 0.2f, AE),
	F(RE, FxForm_Rain, 1.0f, 14.0f, 80, 60, 0, 0.1f, 0, FIX, kGrove, 2.4f, 0.05f, ADD, 1, 0.2f, AE),
	G(IM, Glyph_Leaf, 0.3f, 5.0f, 0, 3, GA_Orbit, 1.3f, FIX, kHeal, 2.4f));

LOOK(kAaCallWild, "call of the wild",
	G(IM, Glyph_Horn, 0.9f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kFern, 3.0f),
	G(IM, Glyph_Paw, 0.35f, 1.0f, 3, 5, GA_RingRise, 1.8f, FIX, kPelt, 3.0f),
	F(IM, FxForm_Bloom, 2.0f, 3.0f, 48, 3, 0, 0, 0, FIX, kFern, 2.4f, 0.05f));

LOOK(kAaConvergence, "convergence of spirits",
	G(IM, Glyph_Spirit, 0.5f, 5.0f, 3, 4, GA_Orbit, 1.6f, FIX, kGhost, 3.0f),
	F(IM, FxForm_Sparkle, 1.6f, 7.0f, 48, 24, 0.12f, 2, 0, FIX, kHeal, 2.8f),
	F(IM, FxForm_Helix, 1.1f, 6.5f, 96, 3, 1.4f, 0, 0.5f, FIX, kGrove, 3.0f, 0.05f, COMET),
	F(IM, FxForm_Burst, 2.2f, 3.0f, 64, 2, 2.0f, 12, 0, FIX, kHeal, 1.0f, 0.08f, ADD, 1, 0.5f));

LOOK(kAaDestructiveVortex, "destructive vortex",
	F(RE, FxForm_Vortex, 2.0f, 12.0f, 128, 6, 1.6f, 3.0f, 6.0f, FIX, kStorm, 3.0f, 0.09f),
	F(RE, FxForm_Lightning, 1, 40.0f, 64, 2, 1.0f, 14.0f, 1.0f, FIX, kShock, 1.2f, 0.12f, ADD, 1, 0.4f),
	G(RE, Glyph_Tornado, 1.2f, 10.0f, 4, 1, GA_PopBob, 0, FIX, kStorm, 3.0f));

LOOK(kAaDireCharm, "dire charm",
	G(IM, Glyph_Wolf, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kPelt, 3.0f),
	G(IM, Glyph_Heart, 0.35f, 5.0f, 0, 4, GA_Orbit, 1.3f, FIX, kBlood, 3.0f),
	F(IM, FxForm_Vortex, 1.2f, 5.0f, 64, 4, 1.2f, 2.0f, -3.0f, FIX, kFern, 2.6f, 0.05f));

LOOK(kAaDistantConflagration, "distant conflagration",
	F(IM, FxForm_Lightning, 1, 44.0f, 64, 4, 0.3f, 8.0f, 1.0f, FIX, kRo, 1.2f, 0.22f),
	F(IM, FxForm_Burst, 3.4f, 1.0f, 120, 3, 1.5f, 24, 0, FIX, kFire, 1.2f, 0.14f, ADD, 1, 0.2f),
	F(IM, FxForm_Flame, 1.2f, 8.0f, 112, 28, 0.4f, 2.0f, 0, FIX, kFlame2, 1.8f, 0.10f, ADD, 1, 0.2f),
	G(IM, Glyph_Moon, 0.7f, 10.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kMoon, 1.6f));

LOOK(kAaExodus, "exodus",
	F(RE, FxForm_Vortex, 1.8f, 9.0f, 112, 6, 0.8f, 3.0f, 7.0f, FIX, kGrove, 1.8f, 0.07f),
	G(RE, Glyph_Door, 1.2f, 7.5f, 4, 1, GA_PopRoll, 0, FIX, kGrove, 1.8f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 64, 3, 0, 0, 0, FIX, kFern, 1.6f, 0.05f, ADD | FxFlag_Reverse, 1, 0, AE));

LOOK(kAaCamouflage, "camouflage",
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kFern, 2.5f, 0.05f, OVER),
	G(IM, Glyph_Leaf, 0.35f, 5.0f, 0, 5, GA_Orbit, 1.3f, FIX, kFern, 2.4f, 0, OVER),
	F(IM, FxForm_Bloom, 1.8f, 1.0f, 40, 2, 0, 0, 0, FIX, kWood, 2.0f, 0.05f, OVER | FxFlag_Reverse));

LOOK(kAaGreatWolf, "spirit of the great wolf",
	G(IM, Glyph_Wolf, 1.3f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kPelt, 3.0f),
	F(IM, FxForm_Wind, 1.4f, 5.0f, 72, 14, 3.5f, 0, 0, FIX, kPelt, 3.0f, 0.05f),
	F(IM, FxForm_Flame, 1.0f, 5.5f, 80, 20, 0.9f, 0.8f, 0, FIX, kGhost, 3.0f, 0.05f),
	G(IM, Glyph_Moon, 0.5f, 10.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kMoon, 3.0f, 0.3f));

LOOK(kAaBear, "spirit of the bear",
	G(IM, Glyph_Paw, 1.2f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kBark, 3.0f, 0, OVER),
	G(IM, Glyph_Claw, 0.6f, 4.0f, 3, 3, GA_Orbit, 1.5f, FIX, kBark, 3.0f),
	F(IM, FxForm_Cracks, 2.2f, 0, 48, 6, 1.2f, 0, 0, FIX, kEarth, 3.0f, 0.07f, OVER),
	F(IM, FxForm_Burst, 2.6f, 0.3f, 64, 2, 0.4f, 12, 0, FIX, kBark, 1.0f, 0.10f, OVER));

LOOK(kAaSpiritWood, "spirit of the wood",
	F(RE, FxForm_Vines, 1.0f, 2.0f, 120, 6, 0.4f, 0, 1.0f, FIX, kWood, 3.0f, 0.07f, OVER, 1, 0, AE),
	G(RE, Glyph_Tree, 0.6f, 2.0f, 3, 6, GA_RingRise, 2.4f, FIX, kWood, 3.0f),
	F(IM, FxForm_Orbit, 1.2f, 6.0f, 40, 10, 0, 0, 0, FIX, kFern, 3.0f, 0.04f, ADD, 1.5f));

LOOK(kAaCheetah, "communion of the cheetah",
	F(IM, FxForm_Wind, 1.3f, 5.0f, 72, 14, 4.0f, 0, 0, FIX, kAmber, 2.6f, 0.05f),
	G(IM, Glyph_Paw, 0.3f, 1.0f, 3, 4, GA_PopSpin, 1.2f, FIX, kAmber, 2.2f));

LOOK(kAaNaturesBoon, "nature's boon",
	G(RE, Glyph_Tree, 1.2f, 3.0f, 4, 1, GA_Emerge, 0, FIX, kHealGold, 3.5f),
	F(RE, FxForm_Rune, 2.4f, 0, 80, 8, 3, 16, 0, FIX, kHeal, 3.5f, 0.05f),
	F(RE, FxForm_Sparkle, 1.8f, 7.0f, 48, 24, 0.12f, 1, 0, FIX, kHealGold, 3.5f));

LOOK(kAaNaturesGuardian, "nature's guardian",
	G(RE, Glyph_Tree, 1.8f, 4.0f, 4, 1, GA_Emerge, 0, FIX, kWood, 4.0f, 0, OVER),
	F(RE, FxForm_Vines, 1.6f, 6.0f, 120, 6, 0.8f, 0, 1.0f, FIX, kWood, 4.0f, 0.08f, OVER),
	F(RE, FxForm_Cracks, 3.0f, 0, 48, 6, 1.2f, 0, 0, FIX, kEarth, 4.0f, 0.07f, OVER));

LOOK(kAaNaturesFury, "nature's fury",
	F(IM, FxForm_Flame, 1.1f, 7.0f, 112, 28, 0.8f, 1.4f, 0, FIX, kWildfire, 3.5f, 0.10f),
	G(IM, Glyph_Leaf, 0.9f, 8.5f, 4, 1, GA_PopRoll, 0, FIX, kWildfire, 3.0f),
	G(IM, Glyph_Lightning, 0.4f, 5.0f, 3, 3, GA_Orbit, 1.4f, FIX, kShock, 3.0f));

LOOK(kAaNaturesFire, "nature's fire",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.4f, 1.0f, FIX, kWildfire, 0.8f, 0.18f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 72, 2, 1.0f, 14, 0, FIX, kWildfire, 0.9f, 0.10f),
	G(IM, Glyph_Leaf, 0.5f, 6.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kFire, 1.2f));

LOOK(kAaNaturesFrost, "nature's frost",
	F(TR, FxForm_Bolt, 1, 0, 96, 3, 0.4f, 0.4f, 1.0f, FIX, kCold, 0.8f, 0.18f),
	F(IM, FxForm_Crystal, 1.0f, 4.0f, 40, 8, 0, 0.7f, 0, FIX, kCold, 1.4f, 0.05f),
	G(IM, Glyph_Leaf, 0.5f, 6.5f, 4, 1, GlyphAnim_Pop, 0, FIX, kHail, 1.2f));

LOOK(kAaSeasonsWrath, "season's wrath",
	G(IM, Glyph_Leaf, 0.35f, 9.0f, 4, 6, GA_Rain, 1.6f, FIX, kAutumn, 3.0f),
	G(IM, Glyph_Snowflake, 0.35f, 9.0f, 4, 4, GA_Rain, 1.2f, FIX, kCold, 3.0f, 0.3f),
	F(IM, FxForm_Rune, 1.6f, 3.2f, 70, 4, 1, 16, 1, FIX, kAutumn, 3.0f, 0.05f),
	G(IM, Glyph_ArrowDown, 0.3f, 6.0f, 0, 4, GA_Orbit, 1.3f, FIX, kWinterFire, 3.0f));

LOOK(kAaSpireNature, "spire of nature",
	F(RE, FxForm_Flame, 0.5f, 16.0f, 96, 12, 0.0f, 5.0f, 0, FIX, kGrove, 2.6f, 0.10f),
	G(RE, Glyph_Tree, 1.3f, 9.0f, 4, 1, GA_Drop, 0, FIX, kGrove, 2.6f),
	F(RE, FxForm_Burst, 1.0f, 0.4f, 96, 2, 1.0f, 16, 0, FIX, kFern, 1.4f, 0.10f, ADD, 1, 0.35f, AE));

LOOK(kAaWallWind, "wall of wind",
	F(RE, FxForm_Wind, 1.8f, 7.0f, 96, 16, 4.0f, 0, 0, FIX, kAir, 1.8f, 0.07f),
	F(RE, FxForm_Bloom, 1.0f, 4.0f, 96, 4, 0, 0, 0, FIX, kAir, 1.6f, 0.06f, ADD, 1, 0, AE),
	G(IM, Glyph_Tornado, 0.8f, 7.0f, 4, 1, GlyphAnim_Pop, 0, FIX, kStorm, 1.6f));

LOOK(kAaWrathWild, "wrath of the wild",
	Persist(F(AU, FxForm_Thorns, 1.2f, 6.5f, 48, 6, 1.4f, 8, 0, FIX, kBriar, 0, 0.06f, OVER | FxFlag_NoEnvelope)),
	G(IM, Glyph_Claw, 1.0f, 8.0f, 4, 1, GA_PopBob, 0, FIX, kBlood, 3.0f),
	F(IM, FxForm_Burst, 2.8f, 0.3f, 72, 2, 0.4f, 16, 0, FIX, kBriar, 1.0f, 0.10f));

LOOK(kAaWildtenderSurvival, "wildtender's survival",
	G(IM, Glyph_Tree, 1.0f, 8.5f, 4, 1, GA_PopBob, 0, FIX, kGrove, 3.5f),
	F(IM, FxForm_Dome, 1.9f, 7.2f, 112, 8, 3, 0, 0, FIX, kHeal, 3.5f, 0.05f),
	F(IM, FxForm_Vines, 1.0f, 2.5f, 96, 6, 0.4f, 0, 1.0f, FIX, kFern, 3.5f, 0.06f));

LOOK(kAaDirewood, "protection of direwood",
	F(IM, FxForm_Crystal, 1.2f, 5.5f, 60, 12, 0, 0.8f, 0, FIX, kBark, 3.0f, 0.07f, OVER),
	F(IM, FxForm_Thorns, 1.1f, 5.0f, 30, 5, 1.0f, 6, 0, FIX, kThorn, 3.0f, 0.05f, OVER),
	G(IM, Glyph_Shield, 0.8f, 7.8f, 4, 1, GA_PopBob, 0, FIX, kWood, 3.0f));

LOOK(kAaEntrap, "entrap",
	F(IM, FxForm_Vines, 1.0f, 1.0f, 96, 6, 0.3f, 0, 1.0f, FIX, kFern, 3.0f, 0.07f, OVER),
	G(IM, Glyph_Link, 0.35f, 0.6f, 3, 5, GA_PopSpin, 1.1f, FIX, kFern, 3.0f));

LOOK(kAaSilentCasting, "silent casting",
	G(IM, Glyph_Hand, 0.7f, 7.4f, 4, 1, GA_PopAscend, 0, FIX, kGhost, 2.0f),
	F(IM, FxForm_Bloom, 1.6f, 2.0f, 40, 2, 0, 0, 0, FIX, kFern, 2.0f, 0.04f, ADD | FxFlag_Reverse));

LOOK(kAaUnderbrush, "veil of the underbrush",
	F(IM, FxForm_Vines, 1.1f, 6.0f, 120, 6, 1.0f, 0, 1.0f, FIX, kFern, 2.4f, 0.07f, OVER),
	F(IM, FxForm_Flame, 1.0f, 6.0f, 80, 20, 0.95f, 0.7f, 0, FIX, kWood, 2.4f, 0.05f, OVER),
	G(IM, Glyph_Leaf, 0.4f, 6.0f, 0, 4, GA_Drop, 1.3f, FIX, kFern, 2.2f, 0, OVER));

LOOK(kAaCleansingRains, "cleansing rains",
	F(RE, FxForm_Rain, 1.0f, 16.0f, 80, 70, 0, 0.15f, 0, FIX, kWater, 2.8f, 0.05f, ADD, 1, 0, AE),
	G(RE, Glyph_Drop, 0.35f, 10.0f, 4, 6, GA_Rain, 1.8f, FIX, kTear, 2.4f),
	F(RE, FxForm_Bloom, 1.0f, 1.0f, 96, 3, 0, 0, 0, FIX, kHeal, 2.4f, 0.05f, ADD, 1, 0.6f, AE),
	F(IM, FxForm_Sparkle, 1.2f, 7.0f, 32, 16, 0.12f, 1, 0, FIX, kHeal, 2.0f));

LOOK(kAaMistwalk, "mistwalk",
	F(IM, FxForm_Bubbles, 1.3f, 6.0f, 48, 20, 0.5f, 0, 0, FIX, kGhost, 3.0f, 0.05f, OVER),
	F(IM, FxForm_Wind, 1.3f, 5.0f, 64, 12, 1.2f, 0, 0, FIX, kGhost, 3.0f, 0.05f),
	G(IM, Glyph_Wave, 0.6f, 7.2f, 4, 1, GA_PopBob, 0, FIX, kWater, 2.4f));

LOOK(kAaNaturesBlessing, "nature's blessing",
	G(IM, Glyph_Tree, 1.0f, 8.0f, 4, 1, GA_PopAscend, 0, FIX, kNature, 3.0f),
	F(IM, FxForm_Vines, 1.0f, 7.0f, 96, 4, 2.5f, 0, 0, FIX, kWood, 3.0f, 0.05f),
	F(IM, FxForm_Sparkle, 1.4f, 8.0f, 40, 20, 0.12f, 0, 0, FIX, kHeal, 3.0f));

LOOK(kAaNaturesBolt, "nature's bolt",
	F(TR, FxForm_Bolt, 1, 0, 128, 4, 0.5f, 0.8f, 0.5f, FIX, kNature, 0.9f, 0.10f),
	G(IM, Glyph_Leaf, 0.4f, 6.0f, 0, 5, GA_PopSpin, 1.0f, FIX, kNature, 1.6f),
	F(IM, FxForm_Burst, 2.2f, 1.0f, 72, 2, 1.0f, 14, 0, FIX, kWood, 1.0f, 0.09f));

LOOK(kAaParalyticSpores, "paralytic spores",
	F(IM, FxForm_Sparkle, 2.0f, 3.0f, 60, 40, 0.14f, 1, 0, FIX, kDisease, 3.0f, 0.06f, ADD, 1, 0, AE),
	F(IM, FxForm_Bubbles, 1.6f, 4.0f, 48, 24, 0.4f, 0, 0, FIX, kDisease, 3.0f, 0.05f, ADD, 1, 0, AE),
	G(IM, Glyph_Shackle, 0.5f, 7.0f, 4, 1, GA_Drop, 0, FIX, kPoison, 2.6f));

LOOK(kAaParalyticSpray, "paralytic spray",
	F(TR, FxForm_Drain, 1, 0, 64, 5, 1.2f, 18, 0, FIX, kPoison, 1.2f, 0.06f),
	F(IM, FxForm_Rain, 1.4f, 7.0f, 48, 40, 0.6f, 0.2f, 0, FIX, kPoison, 1.8f, 0.04f),
	G(IM, Glyph_Link, 0.35f, 1.5f, 3, 6, GA_PopSpin, 1.0f, FIX, kDisease, 2.6f));

LOOK(kAaSpiritNature, "spirit of nature",
	G(IM, Glyph_Wolf, 0.9f, 7.6f, 4, 1, GA_PopBob, 0, FIX, kNature, 3.0f),
	F(IM, FxForm_Orbit, 1.3f, 5.0f, 64, 10, 0, 0, 0, FIX, kNature, 3.0f, 0.05f, ADD, 1.6f),
	F(IM, FxForm_Vines, 1.1f, 6.0f, 80, 3, 2.0f, 0, 1, FIX, kGoldVine, 3.0f, 0.05f));

LOOK(kAaFireflySwarm, "swarm of fireflies",
	F(RE, FxForm_Orbit, 1.6f, 6.0f, 96, 18, 0, 0, 0, FIX, kFirefly, 3.5f, 0.04f, ADD, 2.4f),
	F(TR, FxForm_Drain, 1, 0, 64, 6, 1.6f, 20, 0, FIX, kFirefly, 1.4f, 0.05f),
	F(IM, FxForm_Sparkle, 1.6f, 7.0f, 60, 30, 0.12f, 0, 0, FIX, kFirefly, 3.0f));

enum Line : int
{
	Sunray, Sunsear, Solstice, Reaping, Daybreak, Heliacal, Wrath, WildfireAura, Grasp, Pillar,
	Stormborn, Roar, Hail, Tectonic, Frost, Breath, Crystals, Chill, Natureward, Moonbeam, FrostAura,
	Winters, Frostbite, Burn, Winds, RemoteSun, RemoteMoon, SunEnervation, MoonEnervation, Vida, Luna,
	Adrenaline, Rejuv, Survival, Promised, SunBlessing, Growth, Nurturing, Response, Preinc, Regen,
	Breeze, Blood, Corruption, Skin, Blessing, SkinTo, Reptile, Guard, Mammoth, Mask, Eyes, Unity,
	Absolution, Ways, Alliance, Fortitude, Twincast, Coat, Legacy, Bulwark, Spur, SurgeVines, Rush,
	Vinelash, ThornmawVines, Thornmaw, Horde, Assault, Charm, Calm, Chant, SummonItem, Unnatural, Wolf,
	Eagle, Ring, Circle, Zephyr, Succor, ProcFrostHeal, ProcSunHeal, ProcRo, ProcGlint,
	Apex, BlessingRo, VortexRo, BlessingTunare, CallWild, Convergence, DestructiveVortex, DireCharm,
	DistantConflagration, Exodus, Camouflage, GreatWolf, Bear, SpiritWood, Cheetah, NaturesBoon,
	NaturesGuardian, NaturesFury, NaturesFire, NaturesFrost, SeasonsWrath, SpireNature, WallWind,
	WrathWild, WildtenderSurvival, Direwood, Entrap, SilentCasting, Underbrush, CleansingRains,
	Mistwalk, NaturesBlessing, NaturesBolt, ParalyticSpores, ParalyticSpray, SpiritNature, FireflySwarm,
};

const LineLook kLines[] = {
	{ "Sunray (fire debuff)",          &kDruSunray },
	{ "Sunsear (fire nuke)",           &kDruSunsear },
	{ "Solstice (fire nuke)",          &kDruSolstice },
	{ "Reaping Inferno (fire nuke)",   &kDruReaping },
	{ "Daybreak (fire nuke)",          &kDruDaybreak },
	{ "Heliacal (fire nuke)",          &kDruHeliacal },
	{ "Nature's Wrath (fire dot)",     &kDruWrath },
	{ "Wildfire Aura",                 &kDruWildfireAura },
	{ "Grasp of Ro (debuff)",          &kDruGrasp },
	{ "Pillar of Ro (debuff)",         &kDruPillar },
	{ "Stormborn (nuke, stun)",        &kDruStormborn },
	{ "Roar (nuke, stun)",             &kDruRoar },
	{ "Hail (cold nuke)",              &kDruHail },
	{ "Tectonic (earthquake)",         &kDruTectonic },
	{ "Frost (debuff)",                &kDruFrost },
	{ "Breath (cold nuke)",            &kDruBreath },
	{ "Crystals (cold nuke)",          &kDruCrystals },
	{ "Chill (cold nuke)",             &kDruChill },
	{ "Natureward (cold nuke, heal)",  &kDruNatureward },
	{ "Moonbeam (dot)",                &kDruMoonbeam },
	{ "Frost Aura",                    &kDruFrostAura },
	{ "Winter's Flame (dual nuke)",    &kDruWinters },
	{ "Winter's Frostbite",            &kDruFrostbite },
	{ "Winter's Burn",                 &kDruBurn },
	{ "Winds (sun or moon)",           &kDruWinds },
	{ "Remote Sunflare (nuke)",        &kDruRemoteSun },
	{ "Remote Moonfire (nuke)",        &kDruRemoteMoon },
	{ "Sun Enervation",                &kDruSunEnervation },
	{ "Moon Enervation",               &kDruMoonEnervation },
	{ "Vida (single heal)",            &kDruVida },
	{ "Luna (single heal)",            &kDruLuna },
	{ "Adrenaline (quick heal)",       &kDruAdrenaline },
	{ "Rejuvilation (group heal)",     &kDruRejuv },
	{ "Survival (heal ward)",          &kDruSurvival },
	{ "Promised (delayed heal)",       &kDruPromised },
	{ "Sun Blessing (nuke, heal)",     &kDruSunBlessing },
	{ "Growth (hit points)",           &kDruGrowth },
	{ "Nurturing Growth",              &kDruNurturing },
	{ "Nature's Response (ward)",      &kDruResponse },
	{ "Preincarnation",                &kDruPreinc },
	{ "Regeneration",                  &kDruRegen },
	{ "Breeze (group cure)",           &kDruBreeze },
	{ "Blood (cure)",                  &kDruBlood },
	{ "Corruption (cure)",             &kDruCorruption },
	{ "Skin (armor)",                  &kDruSkin },
	{ "Blessing (group armor)",        &kDruBlessing },
	{ "Skin to (self)",                &kDruSkinTo },
	{ "Reptile (defensive proc)",      &kDruReptile },
	{ "Beastwood Guard",               &kDruGuard },
	{ "Mammoth (strength)",            &kDruMammoth },
	{ "Mask (mana)",                   &kDruMask },
	{ "Sharp Eyes (vision)",           &kDruEyes },
	{ "Unity (self)",                  &kDruUnity },
	{ "Arboreal Absolution",           &kDruAbsolution },
	{ "Ways (spell proc)",             &kDruWays },
	{ "Alliance / Covariance",         &kDruAlliance },
	{ "Fortitude",                     &kDruFortitude },
	{ "Twincast",                      &kDruTwincast },
	{ "Coat (damage shield)",          &kDruCoat },
	{ "Legacy (group damage shield)",  &kDruLegacy },
	{ "Bulwark (self shield)",         &kDruBulwark },
	{ "Spur (damage shield)",          &kDruSpur },
	{ "Surge of Vines",                &kDruSurgeVines },
	{ "Rush (thorns)",                 &kDruRush },
	{ "Vinelash (root)",               &kDruVinelash },
	{ "Thornmaw Vines (snare)",        &kDruThornmawVines },
	{ "Thornmaw (dot, snare)",         &kDruThornmaw },
	{ "Horde (swarm nuke)",            &kDruHorde },
	{ "Assault (spore swarm)",         &kDruAssault },
	{ "Beast's (charm animal)",        &kDruCharm },
	{ "Nature's Calm (pacify)",        &kDruCalm },
	{ "Chant (dot)",                   &kDruChant },
	{ "Carve (summon item)",           &kDruSummonItem },
	{ "Unnatural (anti-summoned)",     &kDruUnnatural },
	{ "Spirit of Wolf (speed)",        &kDruWolf },
	{ "Eagle (levitate, speed)",       &kDruEagle },
	{ "Ring (self port)",              &kDruRing },
	{ "Circle (group port)",           &kDruCircle },
	{ "Zephyr (translocate)",          &kDruZephyr },
	{ "Succor / Egress (evacuate)",    &kDruSuccor },
	{ "frost heal proc",               &kDruProcFrostHeal },
	{ "sun blessing proc",             &kDruProcSunHeal },
	{ "Ro's Reflection proc",          &kDruProcRo },
	{ "triggered effect",              &kDruProcGlint },

	{ "AA: Apex of Nature",            &kAaApex },
	{ "AA: Blessing of Ro",            &kAaBlessingRo },
	{ "AA: Vortex of Ro",              &kAaVortexRo },
	{ "AA: Blessing of Tunare",        &kAaBlessingTunare },
	{ "AA: Call of the Wild",          &kAaCallWild },
	{ "AA: Convergence of Spirits",    &kAaConvergence },
	{ "AA: Destructive Vortex",        &kAaDestructiveVortex },
	{ "AA: Dire Charm",                &kAaDireCharm },
	{ "AA: Distant Conflagration",     &kAaDistantConflagration },
	{ "AA: Exodus",                    &kAaExodus },
	{ "AA: Camouflage",                &kAaCamouflage },
	{ "AA: Spirit of the Great Wolf",  &kAaGreatWolf },
	{ "AA: Spirit of the Bear",        &kAaBear },
	{ "AA: Spirit of the Wood",        &kAaSpiritWood },
	{ "AA: Communion of the Cheetah",  &kAaCheetah },
	{ "AA: Nature's Boon",             &kAaNaturesBoon },
	{ "AA: Nature's Guardian",         &kAaNaturesGuardian },
	{ "AA: Nature's Fury",             &kAaNaturesFury },
	{ "AA: Nature's Fire",             &kAaNaturesFire },
	{ "AA: Nature's Frost",            &kAaNaturesFrost },
	{ "AA: Season's Wrath",            &kAaSeasonsWrath },
	{ "AA: Spire of Nature",           &kAaSpireNature },
	{ "AA: Wall of Wind",              &kAaWallWind },
	{ "AA: Wrath of the Wild",         &kAaWrathWild },
	{ "AA: Wildtender's Survival",     &kAaWildtenderSurvival },
	{ "AA: Protection of Direwood",    &kAaDirewood },
	{ "AA: Entrap",                    &kAaEntrap },
	{ "AA: Silent Casting",            &kAaSilentCasting },
	{ "AA: Veil of the Underbrush",    &kAaUnderbrush },
	{ "AA: Cleansing Rains",           &kAaCleansingRains },
	{ "AA: Mistwalk",            &kAaMistwalk },
	{ "AA: Nature's Blessing",   &kAaNaturesBlessing },
	{ "AA: Nature's Bolt",       &kAaNaturesBolt },
	{ "AA: Paralytic Spores",    &kAaParalyticSpores },
	{ "AA: Paralytic Spray",     &kAaParalyticSpray },
	{ "AA: Spirit of Nature",    &kAaSpiritNature },
	{ "AA: Swarm of Fireflies",  &kAaFireflySwarm },
};

const GroupLine kNumbered[] = {
	{ 0, Sunray },      { 1, Stormborn },   { 2, Wrath },       { 3, Hail },        { 4, Frost },
	{ 5, Calm },        { 9, Breath },      { 10, Bulwark },    { 11, SkinTo },     { 12, Skin },
	{ 13, Vida },       { 14, Horde },      { 15, Coat },       { 16, Winters },    { 17, Sunsear },
	{ 19, Solstice },   { 22, Chant },      { 23, Charm },      { 24, Legacy },     { 25, Blessing },
	{ 26, Crystals },   { 27, Mask },       { 28, Adrenaline }, { 29, Preinc },     { 30, Unnatural },
	{ 31, Spur },       { 32, Luna },       { 33, Reptile },    { 34, Eyes },       { 35, Reaping },
	{ 36, Daybreak },   { 37, Survival },   { 38, Promised },   { 39, SummonItem }, { 40, Growth },
	{ 41, Frostbite },  { 42, Burn },       { 43, SurgeVines }, { 44, SunBlessing }, { 45, RemoteSun },
	{ 46, Assault },    { 47, Grasp },      { 48, WildfireAura }, { 49, FrostAura }, { 50, Moonbeam },
	{ 51, Absolution }, { 52, Rejuv },      { 53, Heliacal },   { 54, Breeze },     { 55, Chill },
	{ 56, Guard },      { 57, Roar },       { 58, Natureward }, { 59, ThornmawVines }, { 60, Thornmaw },
	{ 61, Rush },       { 62, Vinelash },   { 63, Pillar },     { 64, RemoteMoon }, { 65, Unity },
	{ 66, Nurturing },  { 67, Ways },       { 68, Winds },      { 71, Response },   { 72, Alliance },
	{ 73, Fortitude },
};

const GroupLine kModern[] = {
	{ 10, Sunray },     { 20, Stormborn },  { 30, Wrath },      { 40, Hail },       { 50, Frost },
	{ 60, Calm },       { 70, Breath },     { 80, Bulwark },    { 100, Skin },      { 110, Vida },
	{ 120, Horde },     { 130, Coat },      { 140, Winters },   { 141, Frostbite }, { 142, Burn },
	{ 150, Sunsear },   { 160, Solstice },  { 170, Chant },     { 180, Charm },     { 190, Legacy },
	{ 200, Blessing },  { 210, Crystals },  { 220, Mask },      { 230, Adrenaline }, { 250, Unnatural },
	{ 260, Spur },      { 270, Luna },      { 280, Reptile },   { 320, Survival },  { 330, Promised },
	{ 350, Growth },    { 370, SunBlessing }, { 380, RemoteSun }, { 381, SunEnervation }, { 390, Assault },
	{ 400, Grasp },     { 420, FrostAura }, { 430, Moonbeam },  { 450, Rejuv },     { 460, Heliacal },
	{ 470, Breeze },    { 480, Chill },     { 500, Roar },      { 510, Natureward }, { 530, Thornmaw },
	{ 540, Rush },      { 560, Pillar },    { 570, RemoteMoon }, { 571, MoonEnervation }, { 590, Nurturing },
	{ 600, Ways },      { 610, Winds },     { 611, ProcGlint }, { 612, ProcGlint }, { 620, Response },
	{ 630, Alliance },
};

const GroupLine kOther[] = {
	{ 2002, Stormborn },  { 2003, Hail },       { 2006, Frost },      { 2007, Calm },       { 2008, Sunray },
	{ 2009, Mammoth },    { 2109, Mammoth },    { 2011, Breath },     { 2012, Bulwark },    { 2013, SkinTo },
	{ 2014, Skin },       { 2015, Vida },       { 2016, Horde },      { 2017, Coat },       { 2018, Winters },
	{ 2019, Wrath },      { 2022, Guard },      { 2023, Solstice },   { 2024, Sunsear },    { 2025, Vinelash },
	{ 2026, Chant },      { 2027, Charm },      { 2028, Legacy },     { 2029, Blessing },   { 2030, Crystals },
	{ 2033, Adrenaline }, { 2034, Preinc },     { 2035, Spur },       { 5032, Luna },
	{ 1000, Corruption }, { 1007, Corruption }, { 4105, Corruption }, { 4205, Corruption }, { 4305, Corruption },
	{ 4405, Corruption }, { 4505, Corruption }, { 4705, Corruption }, { 4905, Corruption },
	{ 125020060, Corruption }, { 126020060, Corruption },
	{ 1022, Tectonic },   { 4118, Tectonic },   { 4218, Tectonic },   { 4318, Tectonic },   { 4418, Tectonic },
	{ 4518, Tectonic },   { 4618, Tectonic },   { 4718, Tectonic },   { 4818, Tectonic },   { 4918, Tectonic },
	{ 125020180, Tectonic }, { 126020180, Tectonic },
	{ 4246, Blood },      { 4346, Blood },      { 4446, Blood },      { 4546, Blood },      { 4646, Blood },
	{ 4746, Blood },      { 4946, Blood },      { 125020450, Blood }, { 126020450, Blood },
	{ 14102, Regen },     { 14115, Regen },     { 14202, Regen },     { 14215, Regen },     { 14302, Regen },
	{ 14315, Regen },     { 14402, Regen },     { 14415, Regen },     { 14502, Regen },     { 14515, Regen },
	{ 14602, Regen },     { 14615, Regen },     { 14702, Regen },     { 14715, Regen },     { 14802, Regen },
	{ 14815, Regen },     { 14902, Regen },     { 14915, Regen },     { 125100030, Regen }, { 125100140, Regen },
	{ 126100140, Regen },
	{ 7228, Twincast },
	{ 11012, Unnatural }, { 20115, Unnatural }, { 20116, Unnatural }, { 20248, Unnatural }, { 20663, Unnatural },
	{ 20664, Unnatural }, { 21529, Unnatural }, { 21531, Unnatural }, { 23238, Unnatural },
	{ 22602, SunEnervation }, { 25804, SunEnervation }, { 28900, SunEnervation }, { 34929, SunEnervation },
	{ 43838, SunEnervation }, { 43938, SunEnervation }, { 59336, SunEnervation }, { 62828, SunEnervation },
	{ 40190, MoonEnervation }, { 43886, MoonEnervation }, { 43986, MoonEnervation }, { 59382, MoonEnervation },
	{ 62883, MoonEnervation },
	{ 906069, Pillar },   { 1306069, Pillar },  { 2406046, Assault },
	{ 210997, Wolf },     { 1106986, Eagle },   { 1506986, Eagle },   { 1306042, Eagle },   { 1906042, Eagle },
	{ 1566, Succor },
};

const NamedLine kAbilities[] = {
	{ "apex of nature",                  Apex },
	{ "apex of nature thorncoat",        Apex },
	{ "blessing of ro",                  BlessingRo },
	{ "vortex of ro",                    VortexRo },
	{ "blessing of tunare",              BlessingTunare },
	{ "call of the wild",                CallWild },
	{ "convergence of spirits",          Convergence },
	{ "destructive vortex",              DestructiveVortex },
	{ "dire charm",                      DireCharm },
	{ "distant conflagration",           DistantConflagration },
	{ "exodus",                          Exodus },
	{ "egress",                          Exodus },
	{ "camouflage",                      Camouflage },
	{ "perfected camouflage",            Camouflage },
	{ "shared camouflage",               Camouflage },
	{ "shared perfected camouflage",     Camouflage },
	{ "spirit of the great wolf",        GreatWolf },
	{ "group spirit of the great wolf",  GreatWolf },
	{ "form of the great wolf",          GreatWolf },
	{ "great wolf form",                 GreatWolf },
	{ "spirit of the bear",              Bear },
	{ "spirit of the wood",              SpiritWood },
	{ "communion of the cheetah",        Cheetah },
	{ "nature's boon",                   NaturesBoon },
	{ "nature's guardian",               NaturesGuardian },
	{ "nature's fury",                   NaturesFury },
	{ "nature's fire",                   NaturesFire },
	{ "nature's frost",                  NaturesFrost },
	{ "season's wrath",                  SeasonsWrath },
	{ "spire of nature",                 SpireNature },
	{ "wall of wind",                    WallWind },
	{ "wrath of the wild",               WrathWild },
	{ "wildtender's survival",           WildtenderSurvival },
	{ "protection of direwood",          Direwood },
	{ "protection of direwood jolt",     Direwood },
	{ "entrap",                          Entrap },
	{ "silent casting",                  SilentCasting },
	{ "veil of the underbrush",          Underbrush },
	{ "cleansing rains",                 CleansingRains },
	{ "improved twincast",               Twincast },
	{ "flight of eagles",                Eagle },
	{ "spirit of eagle",                 Eagle },
	{ "preincarnation",                  Preinc },
	{ "wildtender's unity",              Unity },
	{ "mistwalk",                 Mistwalk },
	{ "nature's blessing",        NaturesBlessing },
	{ "nature's bolt",            NaturesBolt },
	{ "paralytic spores",         ParalyticSpores },
	{ "paralytic spray",          ParalyticSpray },
	{ "spirit of nature",         SpiritNature },
	{ "firefly guardian spirit",  FireflySwarm },
};

const NamedLine kOldSpells[] = {
	{ "ancient: starfire of ro",   Sunsear },    { "starfire",                  Sunsear },
	{ "dawnstrike",                Daybreak },   { "sylvan fire",               Solstice },
	{ "summer's flame",            Solstice },   { "solstice strike",           Solstice },
	{ "immolation of ro",          Wrath },      { "immolation of the sun",     Wrath },
	{ "sylvan embers",             Wrath },      { "vengeance of tunare",       Wrath },
	{ "vengeance of the sun",      Wrath },      { "ro's illumination",         Pillar },
	{ "winter's frost",            Crystals },   { "glitterfrost",              Crystals },
	{ "ancient: glacier frost",    Crystals },   { "ancient: chaos frost",      Crystals },
	{ "e`ci's frosty breath",      Breath },     { "glacier breath",            Breath },
	{ "moonfire",                  Moonbeam },   { "karana's rage",             Stormborn },
	{ "storm's fury",              Stormborn },  { "tempest wind",              Stormborn },
	{ "winter's storm",            Stormborn },  { "earth shiver",              Tectonic },
	{ "nature's touch",            Vida },       { "nature's infusion",         Vida },
	{ "sylvan infusion",           Vida },       { "chlorotrope",               Vida },
	{ "ancient: chlorobon",        Vida },       { "karana's renewal",          Vida },
	{ "protection of the glades",  Skin },       { "protection of the nine",    Skin },
	{ "protection of seasons",     Skin },       { "steeloak skin",             Skin },
	{ "blessing of the nine",      Blessing },   { "blessing of steeloak",      Blessing },
	{ "blessing of oak",           Blessing },   { "shield of bracken",         Coat },
	{ "nettle shield",             Coat },       { "brackencoat",               Coat },
	{ "nettlecoat",                Coat },       { "legacy of bracken",         Legacy },
	{ "legacy of nettles",         Legacy },     { "ancient: legacy of blades", Legacy },
	{ "entrapping roots",          Vinelash },   { "earthen roots",             Vinelash },
	{ "entangle",                  Vinelash },   { "savage roots",              Vinelash },
	{ "serpent vines",             Vinelash },   { "mire thorns",               ThornmawVines },
	{ "hungry vines",              ThornmawVines }, { "command of tunare",      Charm },
	{ "nature's beckon",           Charm },      { "swarming death",            Horde },
	{ "wasp swarm",                Horde },      { "mask of the hunter",        Mask },
	{ "mask of the stalker",       Mask },       { "mask of the forest",        Mask },
	{ "mask of the wild",          Mask },       { "replenishment",             Regen },
	{ "blessing of replenishment", Regen },      { "nature's might",            Mammoth },
	{ "lion's strength",           Mammoth },    { "skin of the reptile",       Reptile },
	{ "barkspur",                  Spur },       { "desolate summoned",         Unnatural },
	{ "nature's serenity",         Calm },
};

int BySpell(const eqlib::EQ_Spell*, std::string_view base)
{
	if (StartsWith(base, "ring of "))
		return Ring;
	if (StartsWith(base, "circle of ") && base != "circle of winter" && base != "circle of summer"
		&& base != "circle of seasons")
		return Circle;
	if (StartsWith(base, "zephyr: "))
		return Zephyr;
	if (StartsWith(base, "succor"))
		return Succor;
	if (EndsWith(base, " blessing trigger"))
		return ProcSunHeal;
	if (StartsWith(base, "ro's reflection"))
		return ProcRo;
	if (EndsWith(base, " heal"))
		return ProcFrostHeal;
	if (StartsWith(base, "winter's flame frostbite"))
		return Frostbite;
	if (StartsWith(base, "winter's flame burn"))
		return Burn;
	for (const NamedLine& n : kOldSpells)
		if (base == n.name)
			return n.line;
	return -1;
}

} // namespace

const ClassCatalogue& DruidCatalogue()
{
	static const ClassCatalogue c{ "Druid", eqlib::Druid, kLines, 5, kNumbered, kModern, kOther,
		kAbilities, &BySpell };
	return c;
}

} // namespace eqgfx::spellfx
