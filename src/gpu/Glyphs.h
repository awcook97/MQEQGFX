#pragma once

#include <cstdint>
#include <string>

namespace eqgfx::gpu {

enum GlyphId : uint32_t
{
	Glyph_Cross = 0,
	Glyph_Ankh,
	Glyph_Heart,
	Glyph_Skull,
	Glyph_Eye,
	Glyph_Star,
	Glyph_Z,
	Glyph_Moon,
	Glyph_Sun,
	Glyph_Hourglass,
	Glyph_Shield,
	Glyph_Lightning,
	Glyph_Note,
	Glyph_Paw,
	Glyph_Snowflake,
	Glyph_Leaf,
	Glyph_Skeleton,
	Glyph_Sword,
	Glyph_Feather,
	Glyph_Link,
	Glyph_Spiral,
	Glyph_Drop,
	Glyph_ArrowUp,
	Glyph_ArrowDown,
	Glyph_Flame,
	Glyph_Pentagram,
	Glyph_Rock,
	Glyph_Wave,
	Glyph_Tornado,
	Glyph_Hand,
	Glyph_Hammer,
	Glyph_Scales,
	Glyph_Crown,
	Glyph_Chalice,
	Glyph_Book,
	Glyph_Steed,
	Glyph_Dove,
	Glyph_Anvil,
	Glyph_Banner,
	Glyph_Shackle,
	Glyph_Tower,
	Glyph_Fist,
	Glyph_Lantern,
	Glyph_Helm,
	Glyph_Horn,
	Glyph_Tree,
	Glyph_FleurDeLis,
	Glyph_Cog,
	Glyph_Coin,
	Glyph_Door,
	Glyph_Gem,
	Glyph_Mask,
	Glyph_Fang,
	Glyph_Scythe,
	Glyph_Claw,
	Glyph_Lute,
	Glyph_Drum,
	Glyph_Totem,
	Glyph_Spirit,
	Glyph_Snake,
	Glyph_Insect,
	Glyph_Mouth,
	Glyph_Bat,
	Glyph_Clef,
	Glyph_Wolf,
	Glyph_Doll,
	Glyph_Count
};

const char* GlyphName(uint32_t id);

uint32_t GlyphSegments(uint32_t id);

const std::string& GlyphHlsl();

} // namespace eqgfx::gpu
