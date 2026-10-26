#pragma once

#include <mq/Plugin.h>

#include <imgui.h>

namespace eqgfx::ui {

struct Palette
{
	ImU32 panel       = IM_COL32(18, 20, 26, 225);
	ImU32 panelBorder = IM_COL32(70, 76, 92, 255);
	ImU32 title       = IM_COL32(32, 36, 46, 240);
	ImU32 titleText   = IM_COL32(220, 224, 232, 255);
	ImU32 text        = IM_COL32(230, 230, 230, 255);
	ImU32 textDim     = IM_COL32(150, 150, 160, 255);
	ImU32 button      = IM_COL32(44, 50, 64, 235);
	ImU32 buttonHover = IM_COL32(62, 70, 90, 245);
	ImU32 buttonDown  = IM_COL32(88, 98, 124, 255);
	ImU32 buttonOn    = IM_COL32(60, 96, 150, 250);
	ImU32 buttonEdge  = IM_COL32(90, 98, 118, 255);
	ImU32 disabled    = IM_COL32(30, 32, 38, 200);
	ImU32 barBack     = IM_COL32(10, 10, 14, 220);
	ImU32 cooldown    = IM_COL32(0, 0, 0, 150);
	ImU32 recovery    = IM_COL32(40, 90, 190, 150);
	ImU32 outline     = IM_COL32(255, 0, 255, 200);
	float rounding    = 3.0f;
};

const Palette& Colors();

struct Mapping
{
	float scale = 1.0f;

	ImVec2 Point(int x, int y) const { return {x * scale, y * scale}; }
	ImVec2 Min(const eqlib::CXRect& r) const { return Point(r.left, r.top); }
	ImVec2 Max(const eqlib::CXRect& r) const { return Point(r.right, r.bottom); }
};

inline ImU32 WithAlpha(ImU32 c, uint8_t a)
{
	const uint32_t ca = (c >> IM_COL32_A_SHIFT) & 0xff;
	return (c & ~IM_COL32_A_MASK) | (((ca * a) / 255) << IM_COL32_A_SHIFT);
}

inline ImU32 FromARGB(uint32_t argb)
{
	return IM_COL32((argb >> 16) & 0xff, (argb >> 8) & 0xff, argb & 0xff, argb >> 24);
}

void Bar(ImDrawList* dl, ImVec2 min, ImVec2 max, float frac, ImU32 fill, ImU32 back, float rounding);

void CooldownSweep(ImDrawList* dl, ImVec2 min, ImVec2 max, float remaining, ImU32 color);

eqlib::CTextureAnimation* SpellIconSheet();
eqlib::CTextureAnimation* ItemIconSheet();
void ForgetIconSheets();

void Icon(ImDrawList* dl, eqlib::CTextureAnimation* sheet, int cell, ImVec2 min, ImVec2 max, uint8_t alpha = 255);

void Text(ImDrawList* dl, const Mapping& m, int fontStyle, const char* text, const eqlib::CXRect& rect,
	ImU32 color, uint16_t flags);

} // namespace eqgfx::ui
