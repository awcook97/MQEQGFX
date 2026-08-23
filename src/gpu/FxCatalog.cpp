/*
 * gpu/FxCatalog.cpp - the effect tables.
 *
 * Read this as a design document as much as data. Each class's list starts with
 * its signature - the silhouette you should recognise across a zone - followed
 * by the abilities that class is actually known for. Plate classes get heavy,
 * grounded, slow figures; casters get orbiting geometry; priests get rising
 * light; rogues and monks get small fast motion. Elements only appear on the
 * classes that HAVE them, which is why the counts differ: a Magician gets four
 * pet elements, a Warrior gets none and gets stances instead.
 */

#include "FxCatalog.h"

#include <cstring>

namespace eqgfx::gpu {

namespace {

constexpr float PI = 3.14159265358979323846f;

// Signature colours: WoW class palette mapped to the closest EQ class, same as
// the Lua version used, so the two paths agree while both exist.
// (values are linear 0..1)
#define C_WAR 0.776f, 0.608f, 0.427f     // Warrior
#define C_CLR 1.000f, 1.000f, 1.000f     // Priest
#define C_PAL 0.957f, 0.549f, 0.729f     // Paladin
#define C_RNG 0.667f, 0.827f, 0.447f     // Hunter
#define C_SHD 0.769f, 0.118f, 0.227f     // Death Knight
#define C_DRU 1.000f, 0.486f, 0.039f     // Druid
#define C_MNK 0.000f, 1.000f, 0.596f     // Monk
#define C_BRD 0.902f, 0.800f, 0.502f     // heirloom gold
#define C_ROG 1.000f, 0.957f, 0.408f     // Rogue
#define C_SHM 0.000f, 0.439f, 0.867f     // Shaman
#define C_NEC 0.529f, 0.533f, 0.933f     // Warlock
#define C_WIZ 0.247f, 0.780f, 0.922f     // Mage
#define C_MAG 0.200f, 0.576f, 0.498f     // Evoker
#define C_ENC 0.639f, 0.188f, 0.788f     // Demon Hunter
#define C_BST 0.118f, 1.000f, 0.000f     // uncommon green
#define C_BER 0.639f, 0.208f, 0.933f     // epic purple

// Element colours, shared across every class that actually casts them.
#define E_FIRE  1.000f, 0.420f, 0.110f
#define E_COLD  0.400f, 0.800f, 1.000f
#define E_MAGIC 0.600f, 0.400f, 1.000f
#define E_POIS  0.400f, 0.950f, 0.200f
#define E_DIS   0.640f, 0.780f, 0.290f
#define E_CORR  0.470f, 0.180f, 0.600f
#define E_EARTH 0.600f, 0.450f, 0.250f
#define E_AIR   0.780f, 0.900f, 1.000f
#define E_WATER 0.250f, 0.550f, 0.900f
#define E_DIVINE 1.000f, 0.930f, 0.650f
#define E_NATURE 0.400f, 0.900f, 0.400f

// ---------------------------------------------------------------------------
// PLATE / MELEE - no elements. Grounded, heavy, disciplines and stances.

const FxDef kWAR[] = {
	{ "Shield Wall", FxForm_Ring, 4.5f, 2.4f, 0.55f, 0.0f, 0.06f, 40, FxFlag_Comet, 0, 0.55f, 1.00f, 0.20f, { C_WAR, 0.85f } },
	{ "Defensive", FxForm_Lattice, 4.2f, 5.5f, 0.35f, 0.0f, 0.05f, 64, 0, 6, 0.30f, 0, 0, { C_WAR, 0.55f } },
	{ "Rampage", FxForm_Spire, 3.4f, 1.2f, 1.60f, 0.0f, 0.07f, 32, 0, 4, 1.90f, 1.60f, 0, { 0.95f, 0.35f, 0.20f, 0.80f } },
	{ "Taunt", FxForm_Bloom, 3.0f, 2.0f, 1.30f, 0.0f, 0.05f, 48, 0, 2, 0, 0, 0, { 0.95f, 0.55f, 0.25f, 0.70f } },
};

const FxDef kMNK[] = {
	{ "Chi", FxForm_Orbit, 2.6f, 5.0f, 1.00f, 0.0f, 0.045f, 48, 0, 5, 0, 0, 0, { C_MNK, 0.80f } },
	{ "Flying Kick", FxForm_Spire, 2.4f, 3.0f, 2.40f, 0.0f, 0.06f, 24, 0, 3, 2.60f, 1.80f, 0, { C_MNK, 0.95f } },
	{ "Feign Death", FxForm_Ring, 3.2f, 0.2f, 0.45f, 0.0f, 0.04f, 40, 0, 0, 0.20f, 1.00f, 0, { 0.45f, 0.45f, 0.50f, 0.55f } },
	{ "Mend", FxForm_Bloom, 2.2f, 4.0f, 0.90f, 0.0f, 0.05f, 40, 0, 2, 0, 0, 0, { E_NATURE, 0.70f } },
};

const FxDef kROG[] = {
	{ "Shadows", FxForm_Ring, 3.0f, 0.35f, 0.60f, 0.0f, 0.04f, 40, FxFlag_Comet, 0, 0.50f, 1.00f, 0.35f, { C_ROG, 0.70f } },
	{ "Poison", FxForm_Orbit, 2.4f, 3.4f, 1.10f, 0.0f, 0.045f, 36, 0, 4, 0, 0, 0, { E_POIS, 0.75f } },
	{ "Backstab", FxForm_Spire, 1.8f, 3.2f, 2.80f, 0.0f, 0.06f, 18, 0, 2, 3.10f, 2.20f, 0, { C_ROG, 0.95f } },
	{ "Evade", FxForm_Bloom, 2.6f, 1.6f, 1.50f, 0.0f, 0.04f, 32, FxFlag_Reverse, 2, 0, 0, 0, { 0.55f, 0.55f, 0.60f, 0.55f } },
};

const FxDef kBER[] = {
	{ "Rage", FxForm_Ring, 3.6f, 2.1f, 2.20f, 0.0f, 0.07f, 56, FxFlag_Comet, 0, 1.40f, 1.00f, 0.60f, { C_BER, 0.90f } },
	{ "Frenzy", FxForm_Spire, 3.0f, 1.4f, 3.00f, 0.0f, 0.07f, 30, FxFlag_Additive, 5, 2.60f, 1.55f, 0, { 0.95f, 0.30f, 0.30f, 0.85f } },
	{ "Axe Throw", FxForm_Polygon, 3.2f, 2.6f, 2.60f, 0.0f, 0.05f, 24, 0, 3, 2.60f, 0, 0, { C_BER, 0.80f } },
	{ "Battle Cry", FxForm_Bloom, 3.2f, 2.2f, 1.40f, 0.0f, 0.06f, 48, 0, 3, 0, 0, 0, { 0.95f, 0.45f, 0.35f, 0.70f } },
};

// ---------------------------------------------------------------------------
// HYBRIDS - a little of everything, so they mix a melee silhouette with one or
// two real spell lines.

const FxDef kPAL[] = {
	{ "Halo", FxForm_Ring, 2.6f, 6.2f, 0.30f, 0.0f, 0.05f, 40, FxFlag_Comet, 0, 0.28f, 1.00f, 0.22f, { C_PAL, 0.95f } },
	{ "Divine", FxForm_Bloom, 3.0f, 5.5f, 0.70f, 0.0f, 0.05f, 44, FxFlag_Additive, 2, 0, 0, 0, { E_DIVINE, 0.80f } },
	{ "Lay Hands", FxForm_Bloom, 4.0f, 6.5f, 0.45f, 0.0f, 0.07f, 56, FxFlag_Additive, 1, 0, 0, 0, { 1.00f, 0.98f, 0.85f, 0.95f } },
	{ "Stun", FxForm_Polygon, 3.0f, 4.2f, 3.20f, 0.0f, 0.05f, 24, 0, 4, 3.20f, 0, 0, { E_DIVINE, 0.85f } },
	{ "Undead Smite", FxForm_Spire, 3.0f, 3.0f, 1.80f, 0.0f, 0.06f, 32, 0, 4, 1.80f, 1.70f, 0, { C_PAL, 0.85f } },
};

const FxDef kSHD[] = {
	{ "Unholy", FxForm_Bloom, 3.4f, 5.0f, 0.55f, 0.0f, 0.06f, 48, FxFlag_Reverse, 2, 0, 0, 0, { C_SHD, 0.80f } },
	{ "Lifetap", FxForm_Helix, 2.6f, 5.4f, 1.10f, 0.0f, 0.05f, 64, FxFlag_Comet | FxFlag_Reverse, 2, -1.6f, 0, 0.55f, { C_SHD, 0.85f } },
	{ "Disease", FxForm_Orbit, 3.0f, 4.2f, 0.85f, 0.0f, 0.05f, 40, 0, 5, 0, 0, 0, { E_DIS, 0.75f } },
	{ "Fear", FxForm_Bloom, 3.8f, 3.0f, 1.20f, 0.0f, 0.05f, 44, 0, 3, 0, 0, 0, { E_CORR, 0.70f } },
	{ "Harm Touch", FxForm_Spire, 2.8f, 2.4f, 2.20f, 0.0f, 0.08f, 28, FxFlag_Additive, 3, 2.20f, 2.00f, 0, { 0.85f, 0.10f, 0.15f, 0.95f } },
};

const FxDef kRNG[] = {
	{ "Bow", FxForm_Ring, 3.4f, 3.0f, 0.50f, PI / 2, 0.05f, 32, FxFlag_Comet, 0, 0.50f, 1.00f, 0.30f, { C_RNG, 0.85f } },
	{ "Flame Arrow", FxForm_Spire, 2.6f, 3.2f, 2.00f, 0.0f, 0.06f, 24, FxFlag_Additive, 3, 2.00f, 1.90f, 0, { E_FIRE, 0.90f } },
	{ "Ice Arrow", FxForm_Ring, 3.0f, 3.2f, 1.30f, PI / 3, 0.05f, 36, FxFlag_Comet, 0, 1.30f, 0.92f, 0.40f, { E_COLD, 0.85f } },
	{ "Snare", FxForm_Lattice, 3.2f, 1.4f, 0.60f, 0.0f, 0.045f, 56, 0, 6, 0.55f, 0, 0, { E_NATURE, 0.65f } },
	{ "Track", FxForm_Bloom, 3.6f, 0.8f, 0.80f, 0.0f, 0.04f, 40, 0, 2, 0, 0, 0, { C_RNG, 0.55f } },
};

const FxDef kBST[] = {
	{ "Warder", FxForm_Orbit, 3.2f, 4.6f, 0.85f, 0.0f, 0.05f, 40, 0, 2, 0, 0, 0, { C_BST, 0.80f } },
	{ "Frost", FxForm_Ring, 3.0f, 3.0f, 1.10f, PI / 4, 0.05f, 36, FxFlag_Comet, 0, 1.10f, 1.00f, 0.35f, { E_COLD, 0.80f } },
	{ "Venom", FxForm_Orbit, 2.6f, 3.6f, 1.20f, 0.0f, 0.045f, 36, 0, 4, 0, 0, 0, { E_POIS, 0.75f } },
	{ "Slow", FxForm_Ring, 3.6f, 0.6f, 0.35f, 0.0f, 0.05f, 40, 0, 0, 0.35f, 1.15f, 0, { C_BST, 0.60f } },
	{ "Spirit Bond", FxForm_Bloom, 3.0f, 4.4f, 0.70f, 0.0f, 0.05f, 40, 0, 2, 0, 0, 0, { E_NATURE, 0.70f } },
};

// ---------------------------------------------------------------------------
// PRIESTS - rising light, heals, and each one's own damage school.

const FxDef kCLR[] = {
	{ "Divine Light", FxForm_Bloom, 2.8f, 6.0f, 0.60f, 0.0f, 0.055f, 48, FxFlag_Additive, 2, 0, 0, 0, { C_CLR, 0.85f } },
	{ "Heal", FxForm_Bloom, 3.2f, 5.6f, 0.90f, 0.0f, 0.05f, 44, FxFlag_Additive, 2, 0, 0, 0, { E_NATURE, 0.80f } },
	{ "Divine Aura", FxForm_Lattice, 3.6f, 5.2f, 0.40f, 0.0f, 0.05f, 64, 0, 8, 0.40f, 0, 0, { E_DIVINE, 0.70f } },
	{ "Undead Nuke", FxForm_Spire, 2.8f, 3.4f, 1.70f, 0.0f, 0.06f, 32, FxFlag_Additive, 4, 1.70f, 1.80f, 0, { E_DIVINE, 0.90f } },
	{ "Stun", FxForm_Polygon, 2.8f, 4.0f, 3.00f, 0.0f, 0.05f, 24, 0, 4, 3.00f, 0, 0, { C_CLR, 0.80f } },
};

const FxDef kDRU[] = {
	{ "Nature", FxForm_Ring, 3.2f, 2.8f, 0.55f, PI / 3, 0.05f, 40, FxFlag_Comet, 0, 0.55f, 1.00f, 0.24f, { C_DRU, 0.85f } },
	{ "Wildfire", FxForm_Spire, 2.8f, 3.0f, 1.90f, 0.0f, 0.06f, 32, FxFlag_Additive, 5, 1.90f, 1.70f, 0, { E_FIRE, 0.90f } },
	{ "Winter", FxForm_Ring, 3.0f, 3.2f, 1.20f, PI / 4, 0.05f, 36, FxFlag_Comet, 0, 1.20f, 0.95f, 0.38f, { E_COLD, 0.85f } },
	{ "Poison", FxForm_Orbit, 2.6f, 3.6f, 1.00f, 0.0f, 0.045f, 36, 0, 4, 0, 0, 0, { E_POIS, 0.75f } },
	{ "Regrowth", FxForm_Bloom, 3.0f, 5.0f, 0.75f, 0.0f, 0.05f, 44, FxFlag_Additive, 2, 0, 0, 0, { E_NATURE, 0.80f } },
	{ "Snare", FxForm_Lattice, 3.2f, 1.2f, 0.55f, 0.0f, 0.045f, 56, 0, 6, 0.50f, 0, 0, { E_NATURE, 0.60f } },
};

const FxDef kSHM[] = {
	{ "Totem", FxForm_Helix, 2.8f, 5.4f, 0.80f, 0.0f, 0.05f, 72, FxFlag_Comet, 2, -0.9f, 0, 0.28f, { C_SHM, 0.85f } },
	{ "Frost", FxForm_Ring, 3.0f, 3.0f, 1.10f, PI / 4, 0.05f, 36, FxFlag_Comet, 0, 1.10f, 1.00f, 0.35f, { E_COLD, 0.85f } },
	{ "Blight", FxForm_Orbit, 2.8f, 3.8f, 0.95f, 0.0f, 0.05f, 40, 0, 5, 0, 0, 0, { E_DIS, 0.75f } },
	{ "Venom", FxForm_Orbit, 2.4f, 3.4f, 1.15f, 0.0f, 0.045f, 36, 0, 4, 0, 0, 0, { E_POIS, 0.75f } },
	{ "Slow", FxForm_Ring, 3.8f, 0.5f, 0.30f, 0.0f, 0.055f, 44, 0, 0, 0.30f, 1.20f, 0, { C_SHM, 0.65f } },
	{ "Canni", FxForm_Bloom, 2.6f, 4.6f, 1.10f, 0.0f, 0.05f, 40, FxFlag_Reverse, 2, 0, 0, 0, { C_SHM, 0.70f } },
};

// ---------------------------------------------------------------------------
// CASTERS - orbiting arcane geometry, one look per school they actually cast.

const FxDef kWIZ[] = {
	{ "Arcane", FxForm_Ring, 3.4f, 3.0f, 1.30f, 0.0f, 0.05f, 44, FxFlag_Comet, 0, 1.30f, 1.00f, 0.40f, { C_WIZ, 0.90f } },
	{ "Fire", FxForm_Spire, 3.0f, 3.2f, 2.10f, 0.0f, 0.06f, 32, FxFlag_Additive, 6, 2.10f, 1.80f, 0, { E_FIRE, 0.90f } },
	{ "Ice", FxForm_Polygon, 3.2f, 3.2f, 1.00f, 0.0f, 0.05f, 30, 0, 6, 1.00f, 0, 0, { E_COLD, 0.90f } },
	{ "Magic", FxForm_Ring, 3.2f, 3.2f, 1.60f, PI / 2, 0.05f, 40, FxFlag_Comet, 0, -1.6f, 1.00f, 0.45f, { E_MAGIC, 0.90f } },
	{ "Translocate", FxForm_Lattice, 3.0f, 5.0f, 0.90f, 0.0f, 0.05f, 64, FxFlag_Additive, 6, 0.90f, 0, 0, { C_WIZ, 0.75f } },
};

const FxDef kMAG[] = {
	{ "Elementalist", FxForm_Lattice, 3.2f, 4.6f, 0.65f, 0.0f, 0.05f, 64, 0, 4, 0.65f, 0, 0, { C_MAG, 0.85f } },
	{ "Fire Pet", FxForm_Spire, 2.8f, 3.0f, 2.00f, 0.0f, 0.06f, 30, FxFlag_Additive, 5, 2.00f, 1.70f, 0, { E_FIRE, 0.85f } },
	{ "Earth Pet", FxForm_Polygon, 3.4f, 1.2f, 0.45f, 0.0f, 0.065f, 24, 0, 4, 0.45f, 0, 0, { E_EARTH, 0.85f } },
	{ "Air Pet", FxForm_Orbit, 3.2f, 5.2f, 1.40f, 0.0f, 0.04f, 44, 0, 6, 0, 0, 0, { E_AIR, 0.80f } },
	{ "Water Pet", FxForm_Ring, 3.0f, 2.6f, 0.95f, PI / 5, 0.05f, 40, FxFlag_Comet, 0, 0.95f, 1.00f, 0.30f, { E_WATER, 0.85f } },
	{ "Malo", FxForm_Bloom, 3.4f, 2.4f, 1.10f, 0.0f, 0.05f, 40, 0, 2, 0, 0, 0, { E_MAGIC, 0.70f } },
};

const FxDef kNEC[] = {
	{ "Necromancy", FxForm_Bloom, 3.0f, 4.6f, 0.85f, 0.0f, 0.05f, 44, FxFlag_Reverse, 2, 0, 0, 0, { C_NEC, 0.85f } },
	{ "Lifetap", FxForm_Helix, 2.4f, 5.0f, 1.20f, 0.0f, 0.05f, 64, FxFlag_Comet | FxFlag_Reverse, 2, -1.8f, 0, 0.60f, { C_NEC, 0.85f } },
	{ "Poison", FxForm_Orbit, 2.6f, 3.8f, 1.15f, 0.0f, 0.045f, 40, 0, 5, 0, 0, 0, { E_POIS, 0.80f } },
	{ "Disease", FxForm_Orbit, 3.0f, 4.2f, 0.90f, 0.0f, 0.05f, 40, 0, 5, 0, 0, 0, { E_DIS, 0.80f } },
	{ "Fear", FxForm_Bloom, 3.6f, 3.0f, 1.25f, 0.0f, 0.05f, 44, 0, 3, 0, 0, 0, { E_CORR, 0.70f } },
	{ "Pet", FxForm_Lattice, 2.8f, 4.2f, 0.70f, 0.0f, 0.045f, 56, 0, 5, 0.70f, 0, 0, { C_NEC, 0.65f } },
};

const FxDef kENC[] = {
	{ "Illusion", FxForm_Polygon, 3.2f, 3.0f, 0.70f, 0.0f, 0.05f, 36, 0, 6, 0.70f, 0, 0, { C_ENC, 0.90f } },
	{ "Mesmerize", FxForm_Lattice, 3.0f, 4.4f, 0.55f, 0.0f, 0.05f, 64, 0, 6, 0.55f, 0, 0, { E_MAGIC, 0.80f } },
	{ "Rune", FxForm_Ring, 3.4f, 2.6f, 0.90f, 0.0f, 0.06f, 44, FxFlag_Comet, 0, 0.90f, 1.00f, 0.30f, { 0.80f, 0.80f, 1.00f, 0.85f } },
	{ "Charm", FxForm_Orbit, 3.0f, 4.0f, 1.05f, 0.0f, 0.045f, 40, 0, 4, 0, 0, 0, { C_ENC, 0.75f } },
	{ "Haste", FxForm_Bloom, 2.8f, 3.4f, 1.60f, 0.0f, 0.045f, 40, FxFlag_Additive, 3, 0, 0, 0, { 0.95f, 0.85f, 1.00f, 0.70f } },
	{ "Tash", FxForm_Spire, 2.8f, 2.2f, 1.50f, 0.0f, 0.055f, 28, 0, 4, 1.50f, 1.60f, 0, { E_MAGIC, 0.80f } },
};

// ---------------------------------------------------------------------------
// BARD - song families rather than elements.

const FxDef kBRD[] = {
	{ "Song", FxForm_Bloom, 3.0f, 2.4f, 0.90f, 0.0f, 0.05f, 48, 0, 3, 0, 0, 0, { C_BRD, 0.85f } },
	{ "Selo's", FxForm_Helix, 2.4f, 4.8f, 1.50f, 0.0f, 0.045f, 64, FxFlag_Comet, 2, -2.0f, 0, 0.50f, { C_BRD, 0.80f } },
	{ "Mez", FxForm_Lattice, 3.0f, 4.0f, 0.50f, 0.0f, 0.05f, 56, 0, 6, 0.50f, 0, 0, { E_MAGIC, 0.75f } },
	{ "Charm", FxForm_Polygon, 3.0f, 3.2f, 0.85f, 0.0f, 0.05f, 30, 0, 5, 0.85f, 0, 0, { C_ENC, 0.75f } },
	{ "Fire Song", FxForm_Spire, 2.6f, 2.8f, 1.80f, 0.0f, 0.055f, 28, FxFlag_Additive, 4, 1.80f, 1.60f, 0, { E_FIRE, 0.80f } },
	{ "Resist Song", FxForm_Ring, 3.4f, 1.0f, 0.60f, 0.0f, 0.05f, 40, FxFlag_Comet, 0, 0.60f, 1.10f, 0.25f, { E_DIVINE, 0.70f } },
};

const FxDef kMER[] = {
	{ "Mercenary", FxForm_Ring, 2.8f, 0.4f, 0.30f, 0.0f, 0.04f, 32, 0, 0, 0.30f, 1.00f, 0, { 0.73f, 0.73f, 0.73f, 0.45f } },
};

const ClassFx kClasses[] = {
	{ "WAR", "Warrior",       kWAR, (int)(sizeof(kWAR) / sizeof(kWAR[0])) },
	{ "CLR", "Cleric",        kCLR, (int)(sizeof(kCLR) / sizeof(kCLR[0])) },
	{ "PAL", "Paladin",       kPAL, (int)(sizeof(kPAL) / sizeof(kPAL[0])) },
	{ "RNG", "Ranger",        kRNG, (int)(sizeof(kRNG) / sizeof(kRNG[0])) },
	{ "SHD", "Shadow Knight", kSHD, (int)(sizeof(kSHD) / sizeof(kSHD[0])) },
	{ "DRU", "Druid",         kDRU, (int)(sizeof(kDRU) / sizeof(kDRU[0])) },
	{ "MNK", "Monk",          kMNK, (int)(sizeof(kMNK) / sizeof(kMNK[0])) },
	{ "BRD", "Bard",          kBRD, (int)(sizeof(kBRD) / sizeof(kBRD[0])) },
	{ "ROG", "Rogue",         kROG, (int)(sizeof(kROG) / sizeof(kROG[0])) },
	{ "SHM", "Shaman",        kSHM, (int)(sizeof(kSHM) / sizeof(kSHM[0])) },
	{ "NEC", "Necromancer",   kNEC, (int)(sizeof(kNEC) / sizeof(kNEC[0])) },
	{ "WIZ", "Wizard",        kWIZ, (int)(sizeof(kWIZ) / sizeof(kWIZ[0])) },
	{ "MAG", "Magician",      kMAG, (int)(sizeof(kMAG) / sizeof(kMAG[0])) },
	{ "ENC", "Enchanter",     kENC, (int)(sizeof(kENC) / sizeof(kENC[0])) },
	{ "BST", "Beastlord",     kBST, (int)(sizeof(kBST) / sizeof(kBST[0])) },
	{ "BER", "Berserker",     kBER, (int)(sizeof(kBER) / sizeof(kBER[0])) },
	{ "MER", "Mercenary",     kMER, (int)(sizeof(kMER) / sizeof(kMER[0])) },
};

} // namespace

const ClassFx* FindClassFx(const char* shortName)
{
	if (!shortName || !*shortName)
		return nullptr;
	for (const ClassFx& c : kClasses)
	{
		if (std::strncmp(c.shortName, shortName, 4) == 0)
			return &c;
	}
	return nullptr;
}

const ClassFx* AllClassFx(int& countOut)
{
	countOut = (int)(sizeof(kClasses) / sizeof(kClasses[0]));
	return kClasses;
}

int TotalFxCount()
{
	int total = 0;
	for (const ClassFx& c : kClasses)
		total += c.count;
	return total;
}

void MakeInstance(const FxDef& def, const float pos[3], float phase,
	float alphaScale, FxInstance& out)
{
	out = {};
	out.pos[0] = pos[0];
	out.pos[1] = pos[1];
	out.pos[2] = pos[2];
	out.radius = def.radius;
	out.height = def.height;
	out.phase = phase;
	out.speed = def.speed;
	out.tilt = def.tilt;
	out.rgba[0] = def.rgba[0];
	out.rgba[1] = def.rgba[1];
	out.rgba[2] = def.rgba[2];
	out.rgba[3] = def.rgba[3] * alphaScale;
	out.form = def.form;
	out.segments = def.segments;
	out.flags = def.flags;
	out.width = def.width;
	out.p0 = def.p0;
	out.p1 = def.p1;
	out.p2 = def.p2;
	out.p3 = def.p3;
}

} // namespace eqgfx::gpu
