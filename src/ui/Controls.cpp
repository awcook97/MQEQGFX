#include "Controls.h"
#include "Suppress.h"

#include <mq/imgui/Widgets.h>

#include <algorithm>
#include <unordered_map>

namespace eqgfx::ui {

namespace {

using GaugeValueFn = float (*)(int eqType, CXStr* text, bool* smooth, CGaugeWnd* gauge);
using LabelFn = bool (*)(int eqType, CXStr* text, bool* enabled, COLORREF* color);

constexpr int kGaugeMemorize = 9;
constexpr int kGaugeScribe = 10;
float s_sideEffectValue[2] = { -1.0f, -1.0f };

struct CachedType
{
	const void* vtable;
	UIType      type;
};
std::unordered_map<const CXWnd*, CachedType> s_types;

UIType TypeOf(const CXWnd* wnd)
{
	const void* vtable = const_cast<CXWnd*>(wnd)->GetVFTable();
	auto it = s_types.find(wnd);
	if (it != s_types.end() && it->second.vtable == vtable)
		return it->second.type;
	const UIType type = wnd->GetType();
	s_types[wnd] = CachedType{ vtable, type };
	return type;
}

int FontOf(const CXWnd* wnd)
{
	return wnd->pFont ? wnd->pFont->FontStyle : FontStyle_12;
}

ImU32 TextColor(COLORREF argb, ImU32 fallback)
{
	if ((argb & 0xffffff) == 0)
		return fallback;
	return FromARGB(argb | 0xff000000);
}

void SyncClip(ImDrawList* dl)
{
	dl->PushClipRect(dl->GetClipRectMin(), dl->GetClipRectMax());
	dl->PopClipRect();
}

void ControlText(DrawContext& ctx, int fontStyle, const char* text, const CXRect& rect, ImU32 color, uint16_t flags)
{
	if (!text || !*text)
		return;
	ImFont* font = mq::imgui::GetEQImFont(fontStyle);
	if (!font || !font->OwnerAtlas)
		return;

	ctx.layers->SetCurrentChannel(ctx.dl, 1);
	SyncClip(ctx.dl);
	ctx.dl->PushTexture(font->OwnerAtlas->TexRef);
	Text(ctx.dl, ctx.map, fontStyle, text, rect, color, flags);
	ctx.dl->PopTexture();
	ctx.layers->SetCurrentChannel(ctx.dl, 0);
	SyncClip(ctx.dl);
}

void Outline(DrawContext& ctx, const CXRect& r)
{
	ctx.dl->AddRect(ctx.map.Min(r), ctx.map.Max(r), Colors().outline);
}

void DrawScreen(DrawContext& ctx, CXWnd* wnd, uint8_t a, const CXRect& rect)
{
	const Palette& p = Colors();
	const uint32_t style = wnd->WindowStyle;
	const ImVec2 mn = ctx.map.Min(rect);
	const ImVec2 mx = ctx.map.Max(rect);

	if (wnd->GetParentWindow())
	{
		if (wnd->BGType == 2 && !(style & CWS_TRANSPARENT))
		{
			const CXRect client = wnd->GetClientClipRect();
			ctx.dl->AddRectFilled(ctx.map.Min(client), ctx.map.Max(client),
				WithAlpha(FromARGB(wnd->BGColor), a));
		}
		return;
	}

	if (!(style & CWS_TRANSPARENT))
		ctx.dl->AddRectFilled(mn, mx, WithAlpha(p.panel, a), p.rounding);

	if (style & CWS_TITLE)
	{
		const CXRect client = wnd->GetClientRect();
		if (client.top > rect.top)
		{
			const CXRect title(rect.left, rect.top, rect.right, client.top);
			ctx.dl->AddRectFilled(mn, ctx.map.Max(title), WithAlpha(p.title, a), p.rounding,
				ImDrawFlags_RoundCornersTop);
			const CXRect textRect(title.left + 6, title.top, title.right - 6, title.bottom);
			ControlText(ctx, FontOf(wnd), wnd->WindowText.c_str(), textRect, WithAlpha(p.titleText, a),
				DrawText_NoWrap | DrawText_VCenter | DrawText_Elide);
		}
	}

	if (style & CWS_BORDER)
		ctx.dl->AddRect(mn, mx, WithAlpha(p.panelBorder, a), p.rounding);
}

void DrawCooldown(DrawContext& ctx, const CButtonWnd* b, ImVec2 mn, ImVec2 mx, uint8_t a)
{
	const uint32_t total = b->GetCoolDownTotalDuration();
	if (!total)
		return;
	const uint32_t left = b->GetCoolDownTimeRemaining();
	if (left)
		CooldownSweep(ctx.dl, mn, mx, static_cast<float>(left) / total, WithAlpha(Colors().cooldown, a));
}

void DrawCheckbox(DrawContext& ctx, CButtonWnd* b, uint8_t a, const CXRect& rect)
{
	const Palette& p = Colors();
	const int box = std::min(rect.GetHeight(), 14);
	const CXRect boxRect(rect.left, rect.top + (rect.GetHeight() - box) / 2, rect.left + box,
		rect.top + (rect.GetHeight() - box) / 2 + box);
	const ImVec2 bmn = ctx.map.Min(boxRect);
	const ImVec2 bmx = ctx.map.Max(boxRect);

	ctx.dl->AddRectFilled(bmn, bmx, WithAlpha(b->MouseOver ? p.buttonHover : p.button, a), p.rounding);
	ctx.dl->AddRect(bmn, bmx, WithAlpha(p.buttonEdge, a), p.rounding);
	if (b->Checked)
	{
		const float s = bmx.x - bmn.x;
		const ImVec2 pts[3] = {
			ImVec2(bmn.x + s * 0.22f, bmn.y + s * 0.52f),
			ImVec2(bmn.x + s * 0.42f, bmn.y + s * 0.72f),
			ImVec2(bmn.x + s * 0.78f, bmn.y + s * 0.28f),
		};
		ctx.dl->AddPolyline(pts, 3, WithAlpha(p.text, a), ImDrawFlags_None, 2.0f);
	}

	const CXRect textRect(boxRect.right + 4, rect.top, rect.right, rect.bottom);
	ControlText(ctx, FontOf(b), b->WindowText.c_str(), textRect,
		WithAlpha(b->Enabled ? TextColor(b->CRNormal, p.text) : p.textDim, a),
		DrawText_NoWrap | DrawText_VCenter);
}

void DrawButton(DrawContext& ctx, CButtonWnd* b, uint8_t a, const CXRect& rect)
{
	if (b->bIsCheckbox)
	{
		DrawCheckbox(ctx, b, a, rect);
		return;
	}

	const Palette& p = Colors();
	const bool enabled = b->Enabled;
	const bool hover = b->MouseOver;
	const bool down = hover && b->MouseButtonState != 0;
	const bool on = b->Checked;
	const ImVec2 mn = ctx.map.Min(rect);
	const ImVec2 mx = ctx.map.Max(rect);
	const CXStr& label = b->WindowText;

	if (label.empty())
	{
		const mq::MQColor tint(255, 255, 255, a);
		const CXRect scaled(static_cast<int>(mn.x), static_cast<int>(mn.y), static_cast<int>(mx.x), static_cast<int>(mx.y));
		if (CTextureAnimation* ta = b->DrawTemplate.GetAnimation(down || on, hover, !enabled, false))
			mq::imgui::DrawTextureAnimation(ctx.dl, ta, scaled, tint);
		if (CTextureAnimation* decal = b->DrawTemplate.GetAnimation(down || on, hover, !enabled, true))
		{
			const CXSize size = (b->DecalSize.cx > 0 && b->DecalSize.cy > 0) ? b->DecalSize : rect.GetSize();
			const ImVec2 at = ctx.map.Point(rect.left + b->DecalOffset.x, rect.top + b->DecalOffset.y);
			const uint32_t dt = b->DecalTint ? b->DecalTint : 0xffffffff;
			mq::imgui::DrawTextureAnimation(ctx.dl, decal,
				CXPoint(static_cast<int>(at.x), static_cast<int>(at.y)),
				CXSize(static_cast<int>(size.cx * ctx.map.scale), static_cast<int>(size.cy * ctx.map.scale)),
				mq::MQColor((dt >> 16) & 0xff, (dt >> 8) & 0xff, dt & 0xff, a));
		}
	}
	else
	{
		const ImU32 fill = !enabled ? p.disabled : down ? p.buttonDown : on ? p.buttonOn : hover ? p.buttonHover : p.button;
		ctx.dl->AddRectFilled(mn, mx, WithAlpha(fill, a), p.rounding);
		if (!(b->ButtonStyle & ButtonStyle_NoBorder))
			ctx.dl->AddRect(mn, mx, WithAlpha(p.buttonEdge, a), p.rounding);
		ControlText(ctx, FontOf(b), label.c_str(), rect,
			WithAlpha(enabled ? TextColor(b->CRNormal, p.text) : p.textDim, a),
			DrawText_NoWrap | DrawText_HCenter | DrawText_VCenter | DrawText_Elide);
	}

	DrawCooldown(ctx, b, mn, mx, a);
}

void DrawSpellGem(DrawContext& ctx, CSpellGemWnd* gem, uint8_t a, const CXRect& rect)
{
	const ImVec2 mn = ctx.map.Min(rect);
	mq::imgui::DrawSpellGem(ctx.dl, gem, CXPoint(static_cast<int>(mn.x), static_cast<int>(mn.y)), gem->MouseOver);
	DrawCooldown(ctx, gem, mn, ctx.map.Max(rect), a);
}

void DrawGauge(DrawContext& ctx, CGaugeWnd* g, uint8_t a)
{
	const Palette& p = Colors();

	CXStr text;
	float value;
	if (g->EQType == kGaugeMemorize || g->EQType == kGaugeScribe)
		value = s_sideEffectValue[g->EQType - kGaugeMemorize];
	else if (g->bUseTargetVal)
		value = g->TargetVal;
	else
	{
		bool smooth = g->bSmooth;
		value = reinterpret_cast<GaugeValueFn>(__GetGaugeValueFromEQ)(g->EQType, &text, &smooth, g);
	}
	if (value == -1.0f)
		return;

	ImU32 fill = FromARGB(g->FillTint | 0xff000000);
	if (value == -2.0f)
	{
		value = 1000.0f;
		fill = FromARGB(0xff800000);
	}
	else if (value <= -10.0f && value >= -110.0f)
	{
		value = (value + 110.0f) * 10.0f;
		fill = FromARGB(0xff800080);
	}
	else if (value <= -1000.0f && value >= -2000.0f)
	{
		value += 2000.0f;
		fill = FromARGB(0xffc08080);
	}

	const CXRect client = g->GetClientRect();
	const int barHeight = g->DrawTemplate.ptaFill ? g->DrawTemplate.ptaFill->GetHeight() : 8;
	const CXRect bar(client.left + g->GaugeOffsetX, client.top + g->GaugeOffsetY, client.right,
		client.top + g->GaugeOffsetY + std::max(barHeight, 2));
	Bar(ctx.dl, ctx.map.Min(bar), ctx.map.Max(bar), value / 1000.0f, WithAlpha(fill, a),
		WithAlpha(p.barBack, a), p.rounding);

	const CXStr& shown = text.empty() ? g->WindowText : text;
	if (!shown.empty())
	{
		const CXRect textRect(client.left + g->TextOffsetX, client.top + g->TextOffsetY, client.right, client.bottom);
		ControlText(ctx, FontOf(g), shown.c_str(), textRect, WithAlpha(TextColor(g->CRNormal, p.text), a),
			DrawText_NoWrap);
	}
}

void DrawLabel(DrawContext& ctx, CLabel* l, uint8_t a, const CXRect& rect)
{
	COLORREF color = l->CRNormal;
	CXStr text;
	if (l->EQType > 0)
	{
		bool enabled = true;
		reinterpret_cast<LabelFn>(__GetLabelFromEQ)(l->EQType, &text, &enabled, &color);
	}
	const CXStr& shown = !text.empty() ? text : !l->Text.empty() ? l->Text : l->WindowText;
	if (shown.empty())
		return;

	uint16_t flags = 0;
	if (l->bNoWrap)
		flags |= DrawText_NoWrap;
	if (l->bAlignRight)
		flags |= DrawText_RightJustify;
	else if (l->bAlignCenter)
		flags |= DrawText_HCenter;
	const CXRect textRect(rect.left + l->xOffset, rect.top, rect.right, rect.bottom);
	ControlText(ctx, FontOf(l), shown.c_str(), textRect, WithAlpha(TextColor(color, Colors().text), a), flags);
}

} // namespace

std::string LabelText(int eqType)
{
	CXStr text;
	bool enabled = true;
	COLORREF color = 0;
	reinterpret_cast<LabelFn>(__GetLabelFromEQ)(eqType, &text, &enabled, &color);
	return text.c_str();
}

void ForgetControlCache()
{
	s_types.clear();
}

void PulseSideEffectGauges()
{
	s_sideEffectValue[0] = s_sideEffectValue[1] = -1.0f;
	const CXWnd* bookWnd = pSpellBookWnd.get();
	if (!bookWnd || !bookWnd->IsVisible() || Debug().keepNative)
		return;
	const SkinnedWindow* book = FindSkinned("SpellBookWnd");
	if (!book || !book->replace || book->wnd != bookWnd)
		return;
	bool smooth = false;
	s_sideEffectValue[0] = reinterpret_cast<GaugeValueFn>(__GetGaugeValueFromEQ)(kGaugeMemorize, nullptr, &smooth, nullptr);
	s_sideEffectValue[1] = reinterpret_cast<GaugeValueFn>(__GetGaugeValueFromEQ)(kGaugeScribe, nullptr, &smooth, nullptr);
}

void DrawWindowTree(DrawContext& ctx, CXWnd* wnd, uint8_t alpha)
{
	if (!wnd->IsVisible() || wnd->bFullyScreenClipped)
		return;

	uint8_t a = alpha;
	if (wnd->WindowStyle & CWS_USEMYALPHA)
		a = wnd->GetAlpha();
	if (wnd->Fades)
		a = static_cast<uint8_t>(wnd->FadeAlpha * a / 255);

	const CXRect rect = wnd->GetScreenRect();
	const CXRect clip = wnd->GetScreenClipRect();
	ctx.dl->PushClipRect(ctx.map.Min(clip), ctx.map.Max(clip), true);
	++ctx.controls;

	switch (TypeOf(wnd))
	{
	case UI_Screen:
	case UI_Page:
		DrawScreen(ctx, wnd, a, rect);
		break;
	case UI_Button:
		DrawButton(ctx, static_cast<CButtonWnd*>(wnd), a, rect);
		break;
	case UI_SpellGem:
		DrawSpellGem(ctx, static_cast<CSpellGemWnd*>(wnd), a, rect);
		break;
	case UI_Gauge:
		DrawGauge(ctx, static_cast<CGaugeWnd*>(wnd), a);
		break;
	case UI_Label:
		DrawLabel(ctx, static_cast<CLabel*>(wnd), a, rect);
		break;
	default:
		if (wnd->BGType == 2 && !(wnd->WindowStyle & CWS_TRANSPARENT))
			ctx.dl->AddRectFilled(ctx.map.Min(rect), ctx.map.Max(rect), WithAlpha(FromARGB(wnd->BGColor), a));
		break;
	}

	if (ctx.outlines)
		Outline(ctx, rect);

	if (!wnd->Minimized)
		for (CXWnd* child = wnd->GetFirstChildWnd(); child; child = child->GetNextSiblingWnd())
			DrawWindowTree(ctx, child, a);

	ctx.dl->PopClipRect();
}

} // namespace eqgfx::ui
