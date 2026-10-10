#pragma once

#include "../Glyphs.h"
#include "../LineFX.h"

#include <cstdint>
#include <span>
#include <string_view>

namespace eqlib { class EQ_Spell; }

namespace eqgfx::spellfx {

using namespace eqgfx::gpu;

struct RGBA { float r, g, b, a; };

struct Pair { RGBA c1, c2; };

constexpr Pair kFire      { { 1.00f, 0.85f, 0.35f, 0.95f }, { 1.00f, 0.22f, 0.04f, 0.80f } };
constexpr Pair kCold      { { 0.92f, 0.98f, 1.00f, 0.95f }, { 0.35f, 0.75f, 1.00f, 0.80f } };
constexpr Pair kMagic     { { 0.88f, 0.72f, 1.00f, 0.95f }, { 0.45f, 0.30f, 1.00f, 0.80f } };
constexpr Pair kShock     { { 1.00f, 1.00f, 1.00f, 1.00f }, { 0.45f, 0.70f, 1.00f, 0.85f } };
constexpr Pair kPoison    { { 0.65f, 1.00f, 0.30f, 0.90f }, { 0.15f, 0.60f, 0.10f, 0.75f } };
constexpr Pair kDisease   { { 0.78f, 0.82f, 0.30f, 0.90f }, { 0.35f, 0.40f, 0.10f, 0.75f } };
constexpr Pair kCorrupt   { { 0.70f, 0.25f, 0.85f, 0.90f }, { 0.20f, 0.03f, 0.28f, 0.80f } };
constexpr Pair kDivine    { { 1.00f, 0.98f, 0.85f, 0.95f }, { 1.00f, 0.80f, 0.30f, 0.80f } };
constexpr Pair kHeal      { { 0.85f, 1.00f, 0.85f, 0.95f }, { 0.30f, 0.90f, 0.40f, 0.80f } };
constexpr Pair kNature    { { 0.55f, 0.95f, 0.35f, 0.90f }, { 0.25f, 0.55f, 0.10f, 0.80f } };
constexpr Pair kWood      { { 0.40f, 0.28f, 0.14f, 0.95f }, { 0.45f, 0.78f, 0.25f, 0.95f } };
constexpr Pair kThorn     { { 0.55f, 0.42f, 0.22f, 0.95f }, { 0.92f, 0.90f, 0.65f, 0.95f } };
constexpr Pair kNecro     { { 0.60f, 1.00f, 0.60f, 0.90f }, { 0.10f, 0.30f, 0.10f, 0.80f } };
constexpr Pair kBone      { { 0.96f, 0.94f, 0.86f, 0.95f }, { 0.72f, 0.70f, 0.62f, 0.90f } };
constexpr Pair kShadow    { { 0.45f, 0.15f, 0.55f, 0.85f }, { 0.06f, 0.00f, 0.10f, 0.80f } };
constexpr Pair kArcane    { { 0.72f, 0.92f, 1.00f, 0.95f }, { 0.20f, 0.50f, 1.00f, 0.80f } };
constexpr Pair kIllusion  { { 1.00f, 0.62f, 1.00f, 0.90f }, { 0.50f, 0.30f, 1.00f, 0.80f } };
constexpr Pair kMana      { { 0.65f, 0.82f, 1.00f, 0.90f }, { 0.20f, 0.40f, 1.00f, 0.80f } };
constexpr Pair kEarth     { { 0.80f, 0.62f, 0.40f, 0.95f }, { 0.40f, 0.30f, 0.15f, 0.90f } };
constexpr Pair kWater     { { 0.72f, 0.92f, 1.00f, 0.90f }, { 0.15f, 0.40f, 0.90f, 0.80f } };
constexpr Pair kAir       { { 0.96f, 1.00f, 1.00f, 0.85f }, { 0.60f, 0.80f, 0.92f, 0.70f } };
constexpr Pair kGold      { { 1.00f, 0.92f, 0.55f, 0.95f }, { 0.92f, 0.62f, 0.12f, 0.85f } };
constexpr Pair kSong      { { 1.00f, 0.92f, 0.62f, 0.95f }, { 0.92f, 0.70f, 0.30f, 0.85f } };
constexpr Pair kBlood     { { 1.00f, 0.35f, 0.35f, 0.95f }, { 0.55f, 0.00f, 0.05f, 0.85f } };
constexpr Pair kWhite     { { 1.00f, 1.00f, 1.00f, 0.95f }, { 0.80f, 0.86f, 1.00f, 0.80f } };
constexpr Pair kGrey      { { 0.75f, 0.75f, 0.80f, 0.80f }, { 0.45f, 0.45f, 0.50f, 0.60f } };
constexpr Pair kPink      { { 1.00f, 0.55f, 0.75f, 0.95f }, { 1.00f, 0.25f, 0.50f, 0.85f } };
constexpr Pair kStar      { { 1.00f, 1.00f, 0.60f, 1.00f }, { 1.00f, 0.80f, 0.20f, 0.90f } };
constexpr Pair kIron      { { 0.80f, 0.82f, 0.88f, 0.95f }, { 0.45f, 0.48f, 0.55f, 0.90f } };
constexpr Pair kFeather   { { 1.00f, 1.00f, 1.00f, 0.85f }, { 0.82f, 0.90f, 1.00f, 0.55f } };
constexpr Pair kPrism     { { 1.00f, 0.50f, 0.90f, 0.95f }, { 0.30f, 0.90f, 1.00f, 0.85f } };
constexpr Pair kHoly      { { 1.00f, 1.00f, 0.92f, 0.95f }, { 1.00f, 0.84f, 0.40f, 0.85f } };
constexpr Pair kHolyFire  { { 1.00f, 1.00f, 0.85f, 0.95f }, { 1.00f, 0.70f, 0.20f, 0.80f } };
constexpr Pair kZeal      { { 1.00f, 0.82f, 0.40f, 0.95f }, { 0.90f, 0.15f, 0.05f, 0.85f } };
constexpr Pair kSea       { { 0.70f, 1.00f, 0.95f, 0.95f }, { 0.10f, 0.45f, 0.75f, 0.85f } };
constexpr Pair kRoyal     { { 1.00f, 0.90f, 0.55f, 0.95f }, { 0.55f, 0.25f, 0.85f, 0.85f } };
constexpr Pair kAmber     { { 1.00f, 0.85f, 0.35f, 0.95f }, { 0.80f, 0.45f, 0.05f, 0.85f } };
constexpr Pair kAurora    { { 0.55f, 1.00f, 0.75f, 0.90f }, { 0.85f, 0.40f, 1.00f, 0.70f } };
constexpr Pair kTear      { { 0.85f, 0.95f, 1.00f, 0.95f }, { 0.40f, 0.60f, 1.00f, 0.80f } };
constexpr Pair kForge     { { 1.00f, 0.95f, 0.70f, 1.00f }, { 1.00f, 0.42f, 0.08f, 0.90f } };
constexpr Pair kSilver    { { 1.00f, 1.00f, 1.00f, 0.95f }, { 0.70f, 0.75f, 0.85f, 0.90f } };
constexpr Pair kStone     { { 0.78f, 0.74f, 0.66f, 0.95f }, { 0.48f, 0.44f, 0.38f, 0.90f } };
constexpr Pair kGhost     { { 0.92f, 0.96f, 1.00f, 0.60f }, { 0.60f, 0.70f, 1.00f, 0.25f } };
constexpr Pair kGoldVine  { { 0.60f, 0.45f, 0.18f, 0.95f }, { 1.00f, 0.85f, 0.35f, 0.95f } };
constexpr Pair kHealGold  { { 0.85f, 1.00f, 0.80f, 0.95f }, { 1.00f, 0.80f, 0.30f, 0.85f } };
constexpr Pair kLight     { { 1.00f, 1.00f, 1.00f, 0.45f }, { 1.00f, 0.90f, 0.60f, 0.15f } };
constexpr Pair kFlame2    { { 1.00f, 0.80f, 0.30f, 0.95f }, { 0.85f, 0.10f, 0.02f, 0.85f } };

struct Palette { Pair primary, accent; };

enum class Stage : uint8_t { Charge, Release, Travel, Impact, Aura };

enum class Col : uint8_t { Primary, Accent, Fixed };

enum class Size : uint8_t
{
	Body,
	AERange,
};

struct Layer
{
	Stage    stage  = Stage::Release;
	uint32_t form   = FxForm_Burst;
	float    radius = 1.0f;
	float    height = 0.0f;
	float    speed  = 1.0f;
	float    width  = 0.06f;
	uint32_t segs   = 32;
	uint32_t flags  = FxFlag_Additive;
	float    p0 = 0, p1 = 0, p2 = 0, p3 = 0, p4 = 0;
	Col      col    = Col::Primary;
	Pair     fixed  = kWhite;
	float    life   = 1.5f;
	float    delay  = 0.0f;
	Size     size   = Size::Body;
	bool     persist = false;
};

struct SpellLook
{
	const char*  name;
	const Layer* layers;
	int          count;
};

#define LOOK(var, label, ...) \
	const Layer var##_L[] = { __VA_ARGS__ }; \
	const SpellLook var = { label, var##_L, static_cast<int>(sizeof(var##_L) / sizeof(var##_L[0])) }

constexpr uint32_t ADD = FxFlag_Additive;
constexpr uint32_t OVER = 0;

constexpr Layer G(Stage st, uint32_t glyph, float size, float height, uint32_t orient,
	float copies, uint32_t anim, float ringR, Col c, Pair fixed, float life, float delay = 0.0f,
	uint32_t flags = ADD)
{
	Layer l;
	l.stage = st; l.form = FxForm_Glyph; l.radius = ringR; l.height = height; l.width = 0.05f;
	l.segs = 0; l.flags = flags;
	l.p0 = static_cast<float>(glyph); l.p1 = static_cast<float>(orient); l.p2 = copies;
	l.p3 = static_cast<float>(anim); l.p4 = size;
	l.col = c; l.fixed = fixed; l.life = life; l.delay = delay;
	return l;
}

constexpr Layer F(Stage st, uint32_t form, float radius, float height, uint32_t segs,
	float p0, float p1, float p2, float p3, Col c, Pair fixed, float life,
	float width = 0.06f, uint32_t flags = ADD, float speed = 1.0f, float delay = 0.0f,
	Size size = Size::Body)
{
	Layer l;
	l.stage = st; l.form = form; l.radius = radius; l.height = height; l.segs = segs;
	l.p0 = p0; l.p1 = p1; l.p2 = p2; l.p3 = p3; l.col = c; l.fixed = fixed; l.life = life;
	l.width = width; l.flags = flags; l.speed = speed; l.delay = delay; l.size = size;
	return l;
}

constexpr Layer Persist(Layer l) { l.persist = true; return l; }

constexpr uint32_t GA_RiseWiggle = GlyphAnim_Rise | GlyphAnim_Wiggle;
constexpr uint32_t GA_PopBob     = GlyphAnim_Pop | GlyphAnim_Bob;
constexpr uint32_t GA_PopAscend  = GlyphAnim_Pop | GlyphAnim_Ascend;
constexpr uint32_t GA_Orbit      = GlyphAnim_Pop | GlyphAnim_Spin | GlyphAnim_Bob;
constexpr uint32_t GA_Drop       = GlyphAnim_Pop | GlyphAnim_Descend;
constexpr uint32_t GA_Roll       = GlyphAnim_Pop | GlyphAnim_Roll | GlyphAnim_Bob;

constexpr Stage CH = Stage::Charge, RE = Stage::Release, TR = Stage::Travel,
                IM = Stage::Impact, AU = Stage::Aura;
constexpr Col PRI = Col::Primary, ACC = Col::Accent, FIX = Col::Fixed;

constexpr uint32_t GA_PopSpin  = GlyphAnim_Pop | GlyphAnim_Spin;
constexpr uint32_t GA_PopRoll  = GlyphAnim_Pop | GlyphAnim_Roll;
constexpr uint32_t GA_RingRise = GlyphAnim_Pop | GlyphAnim_Spin | GlyphAnim_Rise;
constexpr uint32_t GA_Rain     = GlyphAnim_Pop | GlyphAnim_Spin | GlyphAnim_Descend;
constexpr uint32_t COMET = ADD | FxFlag_Comet;
constexpr uint32_t HOLD  = ADD | FxFlag_NoEnvelope;
constexpr Size AE = Size::AERange;

constexpr Layer Stars(float delay)
{
	return G(IM, Glyph_Star, 0.3f, 7.0f, 0, 3, GA_PopSpin, 0.9f, FIX, kStar, 2.2f, delay);
}

constexpr Layer Notes(Pair colour, float life = 2.0f)
{
	return G(RE, Glyph_Note, 0.3f, 6.0f, 0, 3, GA_Orbit | GlyphAnim_Ascend, 1.3f, FIX, colour, life);
}

struct LineLook
{
	const char*      line;
	const SpellLook* look;
	Pair             primary = kGold;
};

constexpr Pair kFromSpell {};

struct GroupLine { int key; int line; };
struct NamedLine { const char* name; int line; };

struct ClassCatalogue
{
	const char*               cls;
	int                       classId;
	std::span<const LineLook> lines;
	int                       numbered = 0;
	std::span<const GroupLine> numberedLines;
	std::span<const GroupLine> modernLines;
	std::span<const GroupLine> otherLines;
	std::span<const NamedLine> abilities;
	int (*bySpell)(const eqlib::EQ_Spell* sp, std::string_view base) = nullptr;
};

const ClassCatalogue& PaladinCatalogue();
const ClassCatalogue& ShadowKnightCatalogue();
const ClassCatalogue& BardCatalogue();
const ClassCatalogue& ShamanCatalogue();
const ClassCatalogue& MageCatalogue();
const ClassCatalogue& WarriorCatalogue();
const ClassCatalogue& ClericCatalogue();
const ClassCatalogue& RangerCatalogue();
const ClassCatalogue& DruidCatalogue();
const ClassCatalogue& MonkCatalogue();
const ClassCatalogue& RogueCatalogue();
const ClassCatalogue& NecromancerCatalogue();
const ClassCatalogue& WizardCatalogue();
const ClassCatalogue& EnchanterCatalogue();
const ClassCatalogue& BeastlordCatalogue();
const ClassCatalogue& BerserkerCatalogue();
const ClassCatalogue& GeneralCatalogue();

bool EndsWith(std::string_view s, std::string_view suffix);
bool StartsWith(std::string_view s, std::string_view prefix);

struct LineRef { const char* cls; const LineLook* line; int classId = 0; };

LineRef FindLineLook(const eqlib::EQ_Spell* sp, int ownClass);

int     LineLookCount();
LineRef LineLookAt(int index);

} // namespace eqgfx::spellfx
