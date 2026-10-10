#include "Widgets.h"

#include <mq/imgui/Widgets.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <cmath>

namespace eqgfx::ui {

const Palette& Colors()
{
	static const Palette palette;
	return palette;
}

namespace {

constexpr float kPi = 3.14159265f;

ImU32 Lighten(ImU32 c, float k)
{
	ImVec4 v = ImGui::ColorConvertU32ToFloat4(c);
	v.x += (1.0f - v.x) * k;
	v.y += (1.0f - v.y) * k;
	v.z += (1.0f - v.z) * k;
	return ImGui::ColorConvertFloat4ToU32(v);
}

} // namespace

void Bar(ImDrawList* dl, ImVec2 min, ImVec2 max, float frac, ImU32 fill, ImU32 back, float rounding)
{
	dl->AddRectFilled(min, max, back, rounding);
	frac = std::clamp(frac, 0.0f, 1.0f);
	if (frac <= 0.0f)
		return;
	const ImVec2 end(min.x + (max.x - min.x) * frac, max.y);
	const ImU32 top = Lighten(fill, 0.25f);
	dl->AddRectFilledMultiColor(min, end, top, top, fill, fill);
}

void CooldownSweep(ImDrawList* dl, ImVec2 min, ImVec2 max, float remaining, ImU32 color)
{
	remaining = std::clamp(remaining, 0.0f, 1.0f);
	if (remaining <= 0.0f)
		return;

	const ImVec2 c((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
	const float r = std::sqrt((max.x - c.x) * (max.x - c.x) + (max.y - c.y) * (max.y - c.y));
	constexpr float kTop = -kPi * 0.5f;
	const float from = kTop + (1.0f - remaining) * 2.0f * kPi;

	dl->PushClipRect(min, max, true);
	const float to = kTop + 2.0f * kPi;
	for (float a = from; a < to; a += kPi)
	{
		const float b = std::min(a + kPi, to);
		dl->PathLineTo(c);
		dl->PathArcTo(c, r, a, b, std::max(2, static_cast<int>(12.0f * (b - a) / kPi)));
		dl->PathFillConvex(color);
	}
	dl->PopClipRect();
}

namespace {

std::unique_ptr<CTextureAnimation> s_spellSheet;
std::unique_ptr<CTextureAnimation> s_itemSheet;
std::atomic<bool>                  s_sheetsStale{ false };

CTextureAnimation* Sheet(std::unique_ptr<CTextureAnimation>& sheet, const char* name)
{
	if (s_sheetsStale.exchange(false))
	{
		s_spellSheet.reset();
		s_itemSheet.reset();
	}
	if (!sheet && pSidlMgr)
		if (CTextureAnimation* src = pSidlMgr->FindAnimation(name))
			sheet = std::make_unique<CTextureAnimation>(*src);
	return sheet.get();
}

} // namespace

CTextureAnimation* SpellIconSheet() { return Sheet(s_spellSheet, "A_SpellIcons"); }
CTextureAnimation* ItemIconSheet()  { return Sheet(s_itemSheet, "A_DragItem"); }
void ForgetIconSheets()             { s_sheetsStale = true; }

void Icon(ImDrawList* dl, CTextureAnimation* sheet, int cell, ImVec2 min, ImVec2 max, uint8_t alpha)
{
	if (!sheet || cell < 0)
		return;
	sheet->SetCurCell(cell);
	mq::imgui::DrawTextureAnimation(dl, sheet,
		CXPoint(static_cast<int>(min.x), static_cast<int>(min.y)),
		CXSize(static_cast<int>(max.x - min.x), static_cast<int>(max.y - min.y)),
		mq::MQColor(255, 255, 255, alpha));
}

void Text(ImDrawList* dl, const Mapping& m, int fontStyle, const char* text, const eqlib::CXRect& rect,
	ImU32 color, uint16_t flags)
{
	if (!text || !*text)
		return;
	const eqlib::CXRect r(static_cast<int>(rect.left * m.scale), static_cast<int>(rect.top * m.scale),
		static_cast<int>(rect.right * m.scale), static_cast<int>(rect.bottom * m.scale));
	const ImVec4 v = ImGui::ColorConvertU32ToFloat4(color);
	mq::imgui::DrawEQText(dl, fontStyle, text, r,
		mq::MQColor(static_cast<uint8_t>(v.x * 255), static_cast<uint8_t>(v.y * 255),
			static_cast<uint8_t>(v.z * 255), static_cast<uint8_t>(v.w * 255)), flags);
}

} // namespace eqgfx::ui
