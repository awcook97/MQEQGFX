#include "Controls.h"
#include "Skins.h"
#include "Suppress.h"

#include <mq/imgui/Widgets.h>

#include <algorithm>
#include <cmath>
#include <imgui_internal.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>

namespace eqgfx::ui {

namespace {

using GaugeValueFn = float (*)(int eqType, CXStr* text, bool* smooth, CGaugeWnd* gauge);
using LabelFn = bool (*)(int eqType, CXStr* text, bool* enabled, COLORREF* color);

constexpr int kGaugeMemorize = 9;
constexpr int kGaugeScribe = 10;
float s_sideEffectValue[2] = { -1.0f, -1.0f };

enum class Widget
{
	None,
	Screen,
	Button,
	SpellGem,
	Gauge,
	Label,
	List,
	Combo,
	Edit,
	InvSlot,
	Slider,
	Tabs,
	Stml,
	HotButton,
	Map,
	Container,
};

struct CachedType
{
	const void* vtable;
	UIType      type;
	Widget      widget;
};
std::unordered_map<const CXWnd*, CachedType> s_types;

Widget Classify(UIType type, const CXWnd* wnd)
{
	if (pMapViewWnd && wnd == &pMapViewWnd->MapView)
		return Widget::Map;
	switch (type)
	{
	case UI_Screen:
	case UI_Page:           return Widget::Screen;
	case UI_Button:         return Widget::Button;
	case UI_SpellGem:       return Widget::SpellGem;
	case UI_Gauge:          return Widget::Gauge;
	case UI_Label:          return Widget::Label;
	case UI_Listbox:        return Widget::List;
	case UI_Combobox:       return Widget::Combo;
	case UI_EditBox:        return Widget::Edit;
	case UI_InvSlot:        return Widget::InvSlot;
	case UI_Slider:         return Widget::Slider;
	case UI_TabBox:         return Widget::Tabs;
	case UI_STMLBox:        return Widget::Stml;
	case UI_HotButton:      return Widget::HotButton;
	case UI_LayoutBox:
	case UI_HorizontalLayoutBox:
	case UI_VerticalLayoutBox:
	case UI_TileLayoutBox:  return Widget::Container;
	case UI_Unknown:
		if (std::strcmp(wnd->GetWndClassName(), "CListWnd") == 0)
			return Widget::List;

		return Widget::None;
	default:                return Widget::None;
	}
}

const CachedType& Lookup(const CXWnd* wnd)
{
	const void* vtable = const_cast<CXWnd*>(wnd)->GetVFTable();
	auto it = s_types.find(wnd);
	if (it != s_types.end() && it->second.vtable == vtable)
		return it->second;
	const UIType type = wnd->GetType();
	return s_types[wnd] = CachedType{ vtable, type, Classify(type, wnd) };
}

UIType TypeOf(const CXWnd* wnd)    { return Lookup(wnd).type; }
Widget WidgetOf(const CXWnd* wnd)  { return Lookup(wnd).widget; }

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
	const ImVec2 clipMin = ctx.dl->GetClipRectMin();
	const ImVec2 clipMax = ctx.dl->GetClipRectMax();
	const int before = ctx.dl->CmdBuffer.Size;
	const bool reused = before > 0 && ctx.dl->CmdBuffer[before - 1].ElemCount == 0;
	ctx.dl->PushTexture(font->OwnerAtlas->TexRef);
	Text(ctx.dl, ctx.map, fontStyle, text, rect, color, flags);
	ctx.dl->PopTexture();
	for (int i = reused ? before - 1 : before; i < ctx.dl->CmdBuffer.Size; ++i)
	{
		ImVec4& clip = ctx.dl->CmdBuffer[i].ClipRect;
		clip.x = std::max(clip.x, clipMin.x);
		clip.y = std::max(clip.y, clipMin.y);
		clip.z = std::min(clip.z, clipMax.x);
		clip.w = std::min(clip.w, clipMax.y);
	}
	ctx.layers->SetCurrentChannel(ctx.dl, 0);
	SyncClip(ctx.dl);
}

void Outline(DrawContext& ctx, const CXRect& r)
{
	ctx.dl->AddRect(ctx.map.Min(r), ctx.map.Max(r), Colors().outline);
}

void DrawTitleButtons(DrawContext& ctx, CXWnd* wnd, uint8_t a, const CXRect& title);

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
			const int buttons = ((style & CWS_CLOSE) ? 1 : 0) + ((style & CWS_MINIMIZE) ? 1 : 0);
			const int reserved = static_cast<int>(buttons * (ImGui::GetFontSize() + 4.0f) / ctx.map.scale);
			const CXRect textRect(title.left + 6, title.top, title.right - 6 - reserved, title.bottom);
			ControlText(ctx, FontOf(wnd), wnd->WindowText.c_str(), textRect, WithAlpha(p.titleText, a),
				DrawText_NoWrap | DrawText_VCenter | DrawText_Elide);
			DrawTitleButtons(ctx, wnd, a, title);
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
	const bool textured = b->DrawTemplate.ptaNormal || b->DrawTemplate.ptaNormalDecal;
	if (b->bIsCheckbox && !textured)
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

	if (textured)
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
		if (!label.empty())
		{
			const bool beside = b->DrawTemplate.ptaNormalDecal && !b->DrawTemplate.ptaNormal;
			const int left = beside ? rect.left + b->DecalOffset.x + b->DecalSize.cx + 4 : rect.left;
			ControlText(ctx, FontOf(b), label.c_str(), CXRect(left, rect.top, rect.right, rect.bottom),
				WithAlpha(enabled ? TextColor(b->CRNormal, p.text) : p.textDim, a),
				(beside ? DrawText_NoWrap | DrawText_VCenter : DrawText_NoWrap | DrawText_HCenter | DrawText_VCenter) | DrawText_Elide);
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
	if (gem->SpellIconIndex < 0)
		return;
	uint32_t total = 0;
	const uint32_t left = GemCooldown(gem->SpellSlot, total);
	DrawCooldownOverlay(ctx.dl, mn, ctx.map.Max(rect), left, total);
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

CXRect Pixels(DrawContext& ctx, const CXRect& r)
{
	const ImVec2 mn = ctx.map.Min(r);
	const ImVec2 mx = ctx.map.Max(r);
	return CXRect(static_cast<int>(mn.x), static_cast<int>(mn.y), static_cast<int>(mx.x), static_cast<int>(mx.y));
}

void DrawAnimation(DrawContext& ctx, const CTextureAnimation* ta, const CXRect& rect, uint8_t a)
{
	if (ta)
		mq::imgui::DrawTextureAnimation(ctx.dl, const_cast<CTextureAnimation*>(ta), Pixels(ctx, rect),
			mq::MQColor(255, 255, 255, a));
}

void DrawTinted(DrawContext& ctx, const CTextureAnimation* ta, const CXRect& rect, uint32_t tint, uint8_t a)
{
	mq::imgui::DrawTextureAnimation(ctx.dl, const_cast<CTextureAnimation*>(ta), Pixels(ctx, rect),
		mq::MQColor((tint >> 16) & 0xff, (tint >> 8) & 0xff, tint & 0xff, ((tint >> 24) & 0xff) * a / 255));
}

CXRect PieceRect(const CScreenPieceTemplate* piece, const CXRect& base)
{
	CXRect r = piece->rect;
	if (piece->bAutoStretchVertical)
	{
		r.top = piece->bTopAnchorToTop ? piece->nTopOffset : base.GetHeight() - piece->nTopOffset;
		r.bottom = piece->bBottomAnchorToTop ? piece->nBottomOffset : base.GetHeight() - piece->nBottomOffset;
	}
	if (piece->bAutoStretchHorizontal)
	{
		r.left = piece->bLeftAnchorToLeft ? piece->nLeftOffset : base.GetWidth() - piece->nLeftOffset;
		r.right = piece->bRightAnchorToLeft ? piece->nRightOffset : base.GetWidth() - piece->nRightOffset;
	}
	if (piece->bRelativePosition)
	{
		r.left += base.left;
		r.right += base.left;
		r.top += base.top;
		r.bottom += base.top;
	}
	return r;
}

uint16_t StaticTextFlags(const CScreenPieceTemplate* piece)
{
	return *reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(piece) + 0xa8);
}

void DrawStaticPieces(DrawContext& ctx, CXWnd* wnd, uint8_t a)
{
	CScreenTemplate* tmpl = static_cast<CSidlScreenWnd*>(wnd)->SidlPiece;
	if (!tmpl)
		return;

	CXRect base = wnd->GetClientRect();
	base.left -= wnd->HScrollPos;
	base.right -= wnd->HScrollPos;
	base.top -= wnd->VScrollPos;
	base.bottom -= wnd->VScrollPos;

	const Palette& p = Colors();
	for (int i = 0; i < tmpl->Pieces.GetLength(); ++i)
	{
		const CScreenPieceTemplate* piece = tmpl->Pieces[i];
		if (!piece)
			continue;
		const EStaticScreenPieceClasses kind = pSidlMgr->GetScreenPieceEnum(piece);
		if (kind != StaticScreenPieceText && kind != StaticScreenPieceAnimation
			&& kind != StaticScreenPieceTintedBlendAnimation)
			continue;
		if (!static_cast<const CStaticScreenPieceTemplate*>(piece)->bAutoDraw)
			continue;

		const CXRect rect = PieceRect(piece, base);
		if (kind == StaticScreenPieceAnimation)
			DrawAnimation(ctx, static_cast<const CStaticAnimationTemplate*>(piece)->ptaTextureAnimation, rect, a);
		else if (kind == StaticScreenPieceTintedBlendAnimation)
		{
			const auto* blend = static_cast<const CStaticTintedBlendAnimationTemplate*>(piece);
			if (blend->ptaLayerOneTexture)
				DrawTinted(ctx, blend->ptaLayerOneTexture, rect, blend->ptaLayerTwoTexture ? blend->colorLayerOneTint : 0xffffffff, a);
			if (blend->ptaLayerTwoTexture)
				DrawTinted(ctx, blend->ptaLayerTwoTexture, rect, blend->ptaLayerOneTexture ? blend->colorLayerTwoTint : 0xffffffff, a);
		}
		else
			ControlText(ctx, static_cast<int>(piece->nFont), piece->strText.c_str(), rect,
				WithAlpha(TextColor(piece->colorText, p.text), a), StaticTextFlags(piece));
	}
}

uint16_t Justify(int justification)
{
	switch (justification)
	{
	case 1:  return DrawText_HCenter;
	case 2:  return DrawText_RightJustify;
	default: return 0;
	}
}

void DrawList(DrawContext& ctx, CListWnd* list, uint8_t a)
{
	const Palette& p = Colors();
	const int count = list->GetItemCount();
	const int columns = list->GetColumnCount();
	if (columns <= 0)
		return;
	const CXRect client = list->GetClientRect();

	if (list->ListWndStyle & 0x200000)
	{
		int x = client.left;
		for (int col = 0; col < columns; ++col)
		{
			const SListWndColumn& column = list->Columns[col];
			const CXRect head(x, client.top, x + column.Width, client.top + list->HeaderHeight);
			x += column.Width;
			ctx.dl->AddRectFilled(ctx.map.Min(head), ctx.map.Max(head), WithAlpha(p.title, a));
			ControlText(ctx, FontOf(list), column.StrLabel.c_str(), head, WithAlpha(p.titleText, a),
				static_cast<uint16_t>(DrawText_NoWrap | DrawText_VCenter | DrawText_Elide | Justify(static_cast<int>(list->Columns[col].Flags & 3))));
		}
	}

	if (columns > 1 && list->pColumnSep)
	{
		int x = client.left;
		for (int col = 0; col < columns - 1; ++col)
		{
			x += list->Columns[col].Width;
			const ImVec2 top = ctx.map.Point(x, client.top);
			const ImVec2 bottom = ctx.map.Point(x, client.bottom);
			ctx.dl->AddLine(top, bottom, WithAlpha(p.panelBorder, static_cast<uint8_t>(a / 2)));
		}
	}

	for (int row = list->FirstVisibleLine; row >= 0 && row < count; ++row)
	{
		const SListWndLine& line = list->ItemsArray[row];
		if (!line.bVisible)
			continue;
		const CXRect first = list->GetItemRect(row, 0);
		if (first.top >= client.bottom)
			break;
		const CXRect last = list->GetItemRect(row, columns - 1);

		if (line.bIsSeparator == 1)
		{
			const float y = ctx.map.Point(0, (first.top + first.bottom) / 2).y;
			ctx.dl->AddLine(ImVec2(ctx.map.Min(first).x, y), ImVec2(ctx.map.Max(last).x, y), WithAlpha(p.panelBorder, a));
			continue;
		}

		if (row == list->CurSel || line.bSelected)
		{
			const CXRect whole(first.left, first.top, last.right, first.bottom);
			ctx.dl->AddRectFilled(ctx.map.Min(whole), ctx.map.Max(whole), WithAlpha(p.buttonOn, a), p.rounding);
		}

		const int cells = line.Cells.GetLength();
		for (int col = 0; col < columns && col < cells; ++col)
		{
			const CXRect cell = list->GetItemRect(row, col);
			const SListWndCell& c = line.Cells[col];
			CXRect textRect = cell;
			if (c.pTA)
			{
				CTextureAnimation* ta = const_cast<CTextureAnimation*>(c.pTA);
				const int w = std::min(ta->GetWidth(), cell.GetWidth());
				const int h = ta->GetHeight();
				const int top = cell.top + (cell.GetHeight() - h) / 2;
				DrawAnimation(ctx, ta, CXRect(cell.left, top, cell.left + w, top + h), a);
				textRect.left += w + 2;
			}
			if (c.bOnlyDrawTexture)
				continue;
			const CXStr text = list->GetItemText(row, col);
			if (text.empty())
				continue;
			const ImU32 color = line.bEnabled ? TextColor(c.Color, p.text) : p.textDim;
			ControlText(ctx, FontOf(list), text.c_str(), textRect, WithAlpha(color, a),
				static_cast<uint16_t>(DrawText_NoWrap | DrawText_VCenter | DrawText_Elide | Justify(static_cast<int>(list->Columns[col].Flags & 3))));
		}
	}
}

void DrawCombo(DrawContext& ctx, CComboWnd* combo, uint8_t a, const CXRect& rect)
{
	const Palette& p = Colors();
	const ImVec2 mn = ctx.map.Min(rect);
	const ImVec2 mx = ctx.map.Max(rect);
	ctx.dl->AddRectFilled(mn, mx, WithAlpha(combo->MouseOver ? p.buttonHover : p.button, a), p.rounding);
	ctx.dl->AddRect(mn, mx, WithAlpha(p.buttonEdge, a), p.rounding);

	const CXRect client = combo->GetClientRect();
	const int arrow = std::min(client.GetHeight(), 18);
	const CXRect textRect(client.left + 4, client.top, client.right - arrow, client.bottom);
	const CXStr text = combo->GetCurChoiceText();
	ControlText(ctx, FontOf(combo), text.c_str(), textRect, WithAlpha(combo->Enabled ? p.text : p.textDim, a),
		DrawText_NoWrap | DrawText_VCenter | DrawText_Elide);

	const ImVec2 c = ctx.map.Point(client.right - arrow / 2, (client.top + client.bottom) / 2);
	const float s = 3.5f * ctx.map.scale;
	ctx.dl->AddTriangleFilled(ImVec2(c.x - s, c.y - s * 0.5f), ImVec2(c.x + s, c.y - s * 0.5f), ImVec2(c.x, c.y + s * 0.6f),
		WithAlpha(combo->Enabled ? p.text : p.textDim, a));
}

void DrawEdit(DrawContext& ctx, CEditWnd* edit, uint8_t a, const CXRect& rect)
{
	const Palette& p = Colors();
	ctx.dl->AddRectFilled(ctx.map.Min(rect), ctx.map.Max(rect), WithAlpha(p.barBack, a), p.rounding);
	ctx.dl->AddRect(ctx.map.Min(rect), ctx.map.Max(rect), WithAlpha(p.buttonEdge, a), p.rounding);

	const CXRect client = edit->GetClientRect();
	const bool focused = pWndMgr && pWndMgr->FocusWindow == edit;

	if (focused && edit->StartPos != edit->EndPos)
	{
		const CXPoint s = edit->GetCharIndexPt(std::min(edit->StartPos, edit->EndPos));
		const CXPoint e = edit->GetCharIndexPt(std::max(edit->StartPos, edit->EndPos));
		const CXRect sel(s.x, client.top, e.x, client.bottom);
		ctx.dl->AddRectFilled(ctx.map.Min(sel), ctx.map.Max(sel), WithAlpha(p.buttonOn, a));
	}

	const CXStr text = edit->GetDisplayString();
	uint16_t flags = DrawText_NoWrap | DrawText_VCenter;
	if (edit->EditStyle & ewsMultiline)
		flags = 0;
	if (edit->eAlign == eta_Center)
		flags |= DrawText_HCenter;
	else if (edit->eAlign == eta_Right)
		flags |= DrawText_RightJustify;
	ControlText(ctx, FontOf(edit), text.c_str(), client, WithAlpha(p.text, a), flags);

	if (focused && (GetTickCount() / 500) % 2 == 0)
	{
		const CXPoint caret = edit->GetCaretPt();
		const CXRect line(caret.x, client.top + 1, caret.x + 1, client.bottom - 1);
		ctx.dl->AddRectFilled(ctx.map.Min(line), ctx.map.Max(line), WithAlpha(p.text, a));
	}
}

void DrawInvSlot(DrawContext& ctx, CInvSlotWnd* slot, uint8_t a, const CXRect& rect)
{
	const Palette& p = Colors();
	const ImVec2 mn = ctx.map.Min(rect);
	const ImVec2 mx = ctx.map.Max(rect);
	ctx.dl->AddRectFilled(mn, mx, WithAlpha(slot->MouseOver ? p.buttonHover : p.button, a), p.rounding);

	if (slot->ptItem)
	{
		const CXRect itemRect(rect.left + slot->ItemOffsetX, rect.top + slot->ItemOffsetY,
			rect.right - slot->ItemOffsetX, rect.bottom - slot->ItemOffsetY);
		DrawAnimation(ctx, slot->ptItem, itemRect, a);
	}

	if (slot->Quantity > 0)
	{
		char count[16];
		std::snprintf(count, sizeof(count), "%d", slot->Quantity);
		const CXRect countRect(rect.left, rect.bottom - 12, rect.right - 1, rect.bottom);
		ControlText(ctx, slot->TextFontStyle, count, countRect, WithAlpha(p.text, a),
			DrawText_NoWrap | DrawText_RightJustify);
	}

	ImU32 edge = p.buttonEdge;
	if (slot->bSelected)
		edge = IM_COL32(255, 255, 0, 255);
	else if (slot->bFindSelected)
		edge = IM_COL32(64, 192, 192, 255);
	ctx.dl->AddRect(mn, mx, WithAlpha(edge, a), p.rounding);
}

void DrawSlider(DrawContext& ctx, CSliderWnd* slider, uint8_t a)
{
	const Palette& p = Colors();
	const CXRect client = slider->GetClientRect();
	const int cy = (client.top + client.bottom) / 2;
	const int thumb = std::min(client.GetHeight(), 12);
	const CXRect track(client.left + thumb / 2, cy - 2, client.right - thumb / 2, cy + 2);
	ctx.dl->AddRectFilled(ctx.map.Min(track), ctx.map.Max(track), WithAlpha(p.barBack, a), p.rounding);

	const int span = std::max(slider->NumTicks - 1, 1);
	const float ratio = std::clamp(static_cast<float>(slider->Value) / span, 0.0f, 1.0f);
	const int x = track.left + static_cast<int>(ratio * track.GetWidth());
	const CXRect filled(track.left, track.top, x, track.bottom);
	ctx.dl->AddRectFilled(ctx.map.Min(filled), ctx.map.Max(filled), WithAlpha(p.buttonOn, a), p.rounding);

	const CXRect knob(x - thumb / 2, cy - thumb / 2, x + thumb / 2, cy + thumb / 2);
	ctx.dl->AddRectFilled(ctx.map.Min(knob), ctx.map.Max(knob), WithAlpha(slider->MouseOver ? p.buttonDown : p.buttonHover, a), p.rounding);
	ctx.dl->AddRect(ctx.map.Min(knob), ctx.map.Max(knob), WithAlpha(p.buttonEdge, a), p.rounding);
}

struct Binned
{
	int       count;
	int       pad;
	uint32_t  mask;
	uint32_t  shift;
	uint8_t** chunks;
};

const uint8_t* BinnedAt(const Binned& b, int i, size_t stride)
{
	return b.chunks[i >> (b.shift & 0x3f)] + static_cast<size_t>(i & b.mask) * stride;
}

using StmlLines = CircularArrayClass2<STextLine>;

const STextLine* StmlLine(const StmlLines& lines, int i)
{
	if (!lines.Chunks)
		return nullptr;
	const int index = i + (i < lines.WrapIndex ? lines.HeadIndex : -lines.WrapIndex);
	const STextLine* chunk = lines.Chunks[index >> lines.ChunkShift];
	return chunk ? chunk + (index & lines.ChunkMask) : nullptr;
}

ImU32 UserColor(int index)
{
	return GetColorForChatColor(index).ToImU32();
}

ImU32 StmlColor(const CStmlWnd* stml, const SFormattedText& piece)
{
	const int id = piece.LinkID;
	if (id < 0 || id >= stml->Links.GetLength())
		return FromARGB(piece.Font.color);

	const int type = stml->Links[id].Type;
	const bool over = id == stml->MousedOverLinkID;
	if (over)
	{
		if (type == 9)
			return FromARGB(0xff8aa3ff);
		if (type == 10)
			return FromARGB(0xff00ff00);
		return FromARGB(id == stml->CapturedLinkID ? stml->ALinkColor : stml->MLinkColor);
	}

	switch (type)
	{
	case 6:
	case 12:
	case 15:  return UserColor(USERCOLOR_LINK);
	case 9:   return FromARGB(piece.Font.color);
	case 10:  return FromARGB(0xff808000);
	case 11:  return UserColor(USERCOLOR_ACHIEVEMENT);
	case 13:
	case 14:  return UserColor(USERCOLOR_DIALOG_LINK);
	case 17:  return UserColor(USERCOLOR_FACTION_LINK);
	default:  return FromARGB(stml->LinkColor);
	}
}

void DrawStmlLines(DrawContext& ctx, CStmlWnd* stml, const StmlLines& lines, const CXRect& origin, uint8_t a);

void DrawStmlTable(DrawContext& ctx, CStmlWnd* stml, const STextLine* line, int tableId, const CXRect& origin, uint8_t a)
{
	if (tableId < 0 || tableId >= stml->Tables.GetLength())
		return;
	const uint8_t* tables = *reinterpret_cast<const uint8_t* const*>(reinterpret_cast<const uint8_t*>(&stml->Tables) + 0x08);
	if (!tables)
		return;

	const Binned& rows = *reinterpret_cast<const Binned*>(tables + static_cast<size_t>(tableId) * 0x48);
	int y = line->yTop + origin.top;
	for (int r = 0; r < rows.count; ++r)
	{
		const Binned& cells = *reinterpret_cast<const Binned*>(BinnedAt(rows, r, 0x20));
		int x = origin.left;
		int bottom = y;
		for (int c = 0; c < cells.count; ++c)
		{
			const uint8_t* cell = BinnedAt(cells, c, 0x40);
			const CXRect& rel = *reinterpret_cast<const CXRect*>(cell + 0x30);
			const CXRect rect(rel.left + x, rel.top + y, rel.right + x, rel.bottom + y);
			DrawStmlLines(ctx, stml, *reinterpret_cast<const StmlLines*>(cell), rect, a);
			x = rect.right;
			bottom = rect.bottom;
		}
		y = bottom;
	}
}

void DrawStmlLines(DrawContext& ctx, CStmlWnd* stml, const StmlLines& lines, const CXRect& origin, uint8_t a)
{
	const int count = lines.GetLength();
	for (int i = 0; i < count; ++i)
	{
		const STextLine* line = StmlLine(lines, i);
		if (!line)
			continue;
		if (line->yBottom - stml->VScrollPos < 0)
			continue;
		if (line->yTop - stml->VScrollPos + origin.top > origin.bottom)
			break;

		for (int j = 0; j < line->TextPieces.GetLength(); ++j)
		{
			const SFormattedText& piece = line->TextPieces[j];
			const int x = piece.Left - stml->HScrollPos + origin.left;
			const ImU32 color = StmlColor(stml, piece);

			if (piece.Texture)
			{
				const int w = piece.Texture->GetWidth();
				const int h = piece.Texture->GetHeight();
				const int top = line->yBottom - stml->VScrollPos - h + origin.top;
				if (piece.LinkID >= 0)
				{
					const CXRect back(x - 1, top - 1, x + w + 1, top + h + 1);
					ctx.dl->AddRectFilled(ctx.map.Min(back), ctx.map.Max(back), WithAlpha(color, a));
				}
				DrawAnimation(ctx, piece.Texture, CXRect(x, top, x + w, top + h), a);
			}
			else if (piece.TableID != -1)
				DrawStmlTable(ctx, stml, line, piece.TableID, origin, a);
			else if (piece.Font.face && !piece.Text.empty())
			{
				const int h = piece.Font.face->GetHeight();
				const int top = line->yBottom - stml->VScrollPos - h + origin.top;
				if (top <= origin.top - h || top >= origin.bottom)
					continue;
				ControlText(ctx, piece.Font.face->FontStyle, piece.Text.c_str(), CXRect(x, top, origin.right + 5, top + h),
					WithAlpha(color, a), stml->bAlignCenter ? DrawText_NoWrap | DrawText_HCenter : DrawText_NoWrap);
			}
		}
	}
}

void DrawStml(DrawContext& ctx, CStmlWnd* stml, uint8_t a)
{
	if (stml->bReparseNow)
		return;
	DrawStmlLines(ctx, stml, stml->TextLines, stml->GetClientClipRect(), a);
}

bool BeginHost(const void* key, int part, const ImVec2& pos, const ImVec2& size, uint8_t a)
{
	char name[48];
	std::snprintf(name, sizeof(name), "##mirror%p.%d", key, part);
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, a / 255.0f);
	const bool open = ImGui::Begin(name, nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav
		| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus);
	ImGui::PopStyleVar(3);
	return open;
}

bool Occluded(const DrawContext& ctx, const ImVec2& pos, const ImVec2& size)
{
	if (!ctx.above)
		return false;
	for (const CXRect& r : *ctx.above)
	{
		const ImVec2 mn = ctx.map.Min(r);
		const ImVec2 mx = ctx.map.Max(r);
		if (pos.x < mx.x && pos.x + size.x > mn.x && pos.y < mx.y && pos.y + size.y > mn.y)
			return true;
	}
	return false;
}

void DrawScrollbar(DrawContext& ctx, CXWnd* wnd, uint8_t a, const CXRect& rect, bool vertical)
{
	if (!ctx.hosts)
		return;
	const CXRect client = wnd->GetClientRect();
	const int max = vertical ? wnd->VScrollMax : wnd->HScrollMax;
	const int pos = vertical ? wnd->VScrollPos : wnd->HScrollPos;
	const int view = vertical ? client.GetHeight() : client.GetWidth();
	if (max <= 0 || view <= 0)
		return;

	const float thickness = ImGui::GetStyle().ScrollbarSize;
	const ImVec2 mn = ctx.map.Min(client);
	const ImVec2 mx = ctx.map.Max(client);
	const ImVec2 edge = ctx.map.Max(rect);
	const ImVec2 at = vertical ? ImVec2(edge.x - thickness, mn.y) : ImVec2(mn.x, edge.y - thickness);
	const ImVec2 size = vertical ? ImVec2(thickness, mx.y - mn.y) : ImVec2(mx.x - mn.x, thickness);
	if (size.x <= 0.0f || size.y <= 0.0f || Occluded(ctx, at, size))
		return;

	if (BeginHost(wnd, vertical ? 1 : 2, at, size, a))
	{
		ImGuiWindow* host = ImGui::GetCurrentWindow();
		const ImGuiAxis axis = vertical ? ImGuiAxis_Y : ImGuiAxis_X;
		ImS64 scroll = pos;
		const ImRect bb(host->Pos, ImVec2(host->Pos.x + size.x, host->Pos.y + size.y));
		ImGui::ScrollbarEx(bb, host->GetID(vertical ? "##v" : "##h"), axis, &scroll, view, static_cast<ImS64>(view) + max,
			ImDrawFlags_RoundCornersAll);
		if (scroll != pos)
		{
			if (vertical)
				wnd->SetVScrollPos(static_cast<int>(scroll));
			else
				wnd->SetHScrollPos(static_cast<int>(scroll));
		}
	}
	ImGui::End();
}

void DrawTitleButtons(DrawContext& ctx, CXWnd* wnd, uint8_t a, const CXRect& title)
{
	if (!ctx.hosts)
		return;
	const uint32_t style = wnd->WindowStyle;
	const bool close = (style & CWS_CLOSE) != 0;
	const bool minimize = (style & CWS_MINIMIZE) != 0;
	if (!close && !minimize)
		return;

	const float button = ImGui::GetFontSize();
	const float gap = 4.0f;
	const ImVec2 mn = ctx.map.Min(title);
	const ImVec2 mx = ctx.map.Max(title);
	const ImVec2 size((close && minimize ? 2.0f : 1.0f) * button + (close && minimize ? gap : 0.0f), button);
	const ImVec2 at(mx.x - size.x - 4.0f, mn.y + (mx.y - mn.y - button) * 0.5f);
	if (Occluded(ctx, at, size))
		return;

	if (BeginHost(wnd, 3, at, size, a))
	{
		ImGuiWindow* host = ImGui::GetCurrentWindow();
		float x = host->Pos.x;
		if (minimize)
		{
			host->Collapsed = wnd->Minimized;
			if (ImGui::CollapseButton(host->GetID("##minimize"), ImVec2(x, host->Pos.y), nullptr))
				wnd->Minimize(!wnd->Minimized);
			host->Collapsed = false;
			x += button + gap;
		}
		if (close && ImGui::CloseButton(host->GetID("##close"), ImVec2(x, host->Pos.y)))
			wnd->Show(false, false);
	}
	ImGui::End();
}

void DrawHotButton(DrawContext& ctx, CHotButton* hot, uint8_t a, const CXRect& rect)
{
	const Palette& p = Colors();
	const ImVec2 mn = ctx.map.Min(rect);
	const ImVec2 mx = ctx.map.Max(rect);
	ctx.dl->AddRectFilled(mn, mx, WithAlpha(hot->MouseOver ? p.buttonHover : p.button, a), p.rounding);
	DrawAnimation(ctx, hot->DecalIcon, CXRect(rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2), a);
	if (!hot->LastLabel.empty())
		ControlText(ctx, FontOf(hot), hot->LastLabel.c_str(), rect, WithAlpha(p.text, a),
			DrawText_NoWrap | DrawText_HCenter | DrawText_VCenter | DrawText_Elide);
	ctx.dl->AddRect(mn, mx, WithAlpha(p.buttonEdge, a), p.rounding);
}

struct MapSnapshot
{
	struct Line
	{
		float    x1, y1, x2, y2, z1, z2;
		uint32_t color;
		int      layer;
		bool     marked;
		const void* live;
	};
	struct Label
	{
		float       x, y, z;
		uint32_t    color;
		int         layer;
		int         size;
		int         width, height, offsetX, offsetY;
		bool        selected;
		std::string text;
		const void* live;
	};
	std::vector<Line>  lines;
	std::vector<Label> labels;
};

MapSnapshot s_mapSnapshot;
bool        s_mapCapture = false;
bool        s_mapHookTried = false;
bool        s_mapHooked = false;
uintptr_t   s_mapDrawAddress = 0;
int         s_mapDraws = 0;
int         s_mapPostDraws = 0;
int         s_mapCaptures = 0;
CXRect      s_mapInner;
bool        s_mapActive = false;

uintptr_t Resolve(uintptr_t preferred);

template <typename T>
T Field(const void* base, size_t offset);

void TakeMapSnapshot(const MapViewMap* map)
{
	MapSnapshot& snap = s_mapSnapshot;
	snap.lines.clear();
	snap.labels.clear();

	std::vector<const void*> marked;
	for (const uint8_t* node = Field<const uint8_t*>(map, 0x348); node; node = Field<const uint8_t*>(node, 0))
		marked.push_back(Field<const void*>(node, 8));
	const void* selected = Field<const void*>(map, 0x350);

	for (const MapViewLine* line = map->pLines; line; line = line->pNext)
		snap.lines.push_back({ line->Start.X, line->Start.Y, line->End.X, line->End.Y, line->Start.Z, line->End.Z,
			Field<uint32_t>(&line->Color, 0), line->Layer, std::find(marked.begin(), marked.end(), line) != marked.end(), line });

	for (const MapViewLabel* label = map->pLabels; label; label = label->pNext)
		snap.labels.push_back({ label->Location.X, label->Location.Y, label->Location.Z, Field<uint32_t>(&label->Color, 0),
			label->Layer, label->Size, label->Width, label->Height, label->OffsetX, label->OffsetY, label == selected,
			label->Label ? label->Label : "", label });
}

class MapDrawHook
{
public:
	DETOUR_TRAMPOLINE_DEF_MEMBER(MapDrawHook, void, Draw_Trampoline, (const int*))
	void Draw_Detour(const int* rect)
	{
		if (s_mapCapture)
		{
			++s_mapCaptures;
			TakeMapSnapshot(reinterpret_cast<const MapViewMap*>(this));
			return;
		}
		Draw_Trampoline(rect);
	}
};

void EnsureMapHook()
{
	if (s_mapHookTried)
		return;
	s_mapHookTried = true;
	s_mapDrawAddress = Resolve(0x140476840);
	s_mapHooked = EzDetour(s_mapDrawAddress, &MapDrawHook::Draw_Detour, &MapDrawHook::Draw_Trampoline);
}

template <typename T>
T Field(const void* base, size_t offset)
{
	T value;
	std::memcpy(&value, static_cast<const uint8_t*>(base) + offset, sizeof(T));
	return value;
}

uintptr_t Resolve(uintptr_t preferred)
{
	return preferred - EQGamePreferredAddress + EQGameBaseAddress;
}

ImU32 Opaque(uint32_t argb)
{
	return FromARGB(argb | 0xff000000);
}

void DrawPlayerIndicators(DrawContext& ctx, CXWnd* wnd, uint8_t a, const CXRect&)
{
	const bool attacking = Field<uint8_t>(wnd, 0x390) != 0;
	if (!attacking && Field<uint8_t>(wnd, 0x391) == 0)
		return;
	const uint32_t pulse = Field<uint32_t>(wnd, 0x3d0);
	uint32_t tint = pulse << 8 | (pulse & 0xff);
	tint = attacking ? (tint | 0xffff0000) : ((tint << 8) | 0xff0000ff);

	const CXRect base = wnd->GetClientRect();
	for (size_t offset = 0x3a0; offset <= 0x3c0; offset += 8)
	{
		const auto* piece = Field<const CStaticAnimationTemplate*>(wnd, offset);
		if (piece && piece->ptaTextureAnimation)
			DrawTinted(ctx, piece->ptaTextureAnimation, PieceRect(piece, base), tint, a);
	}
}

void DrawTargetIndicators(DrawContext& ctx, CXWnd* wnd, uint8_t a, const CXRect&)
{
	const uint32_t tint = Field<uint32_t>(wnd, 0x3c0);
	if (!pTarget || tint == 0)
		return;
	const CXRect base = wnd->GetClientRect();
	for (size_t offset = 0x390; offset <= 0x3b0; offset += 8)
	{
		const auto* piece = Field<const CStaticAnimationTemplate*>(wnd, offset);
		if (piece && piece->ptaTextureAnimation)
			DrawTinted(ctx, piece->ptaTextureAnimation, PieceRect(piece, base), tint, a);
	}
}

void DrawCompassMarkers(DrawContext& ctx, CXWnd* wnd, uint8_t a, const CXRect&)
{
	const int count = Field<int>(wnd, 0x348);
	const auto* markers = Field<const uint8_t* const*>(wnd, 0x350);
	if (!markers)
		return;
	const CXRect inner = wnd->GetInnerRect();
	for (int i = 0; i < count; ++i)
	{
		const uint8_t* marker = markers[i];
		if (!marker)
			continue;
		using Probe = uint8_t (*)(void*);
		if (const auto probe = Field<Probe>(marker, 0x28))
			*const_cast<uint8_t*>(marker + 0x18) = probe(Field<void*>(marker, 0x30));
		if (!Field<uint8_t>(marker, 0x18) || !Field<uint8_t>(marker, 0x20))
			continue;

		const int x = Field<int>(marker, 0x1c) + inner.left;
		const ImU32 color = WithAlpha(IM_COL32(Field<int>(marker, 0) & 0xff, Field<int>(marker, 4) & 0xff,
			Field<int>(marker, 8) & 0xff, 255), a);
		for (int dx = 0; dx < 2; ++dx)
			ctx.dl->AddLine(ctx.map.Point(x + dx, inner.top + 4), ctx.map.Point(x + dx, inner.bottom), color);
	}
}

void DrawExtendedTargetHighlight(DrawContext& ctx, CXWnd* wnd, uint8_t a, const CXRect&)
{
	const int index = Field<int>(wnd, 0x8f8);
	if (index < 0)
		return;
	CXWnd* upper = Field<CXWnd*>(wnd, 0x2e0 + static_cast<size_t>(index) * 8);
	CXWnd* lower = Field<CXWnd*>(wnd, 0x790 + static_cast<size_t>(index) * 8);
	if (!upper || !lower)
		return;
	const CXRect top = upper->GetClientRect();
	const CXRect bottom = lower->GetClientRect();
	const CXRect box(bottom.left - 1, top.top, top.right + 1, bottom.bottom + 1);
	ctx.dl->AddRect(ctx.map.Min(box), ctx.map.Max(box), WithAlpha(Opaque(upper->CRNormal), a));
}

void DrawMap(DrawContext& ctx, CXWnd* wnd, uint8_t a)
{
	MapViewMap* map = static_cast<MapViewMap*>(wnd);
	const CXRect inner = wnd->GetInnerRect();
	s_mapInner = inner;
	s_mapActive = wnd->IsActive();
	++s_mapDraws;
	if (inner.GetWidth() <= 0 || inner.GetHeight() <= 0)
		return;

	if (ctx.hosts)
	{
		EnsureMapHook();
		if (s_mapHooked)
		{
			s_mapCapture = true;
			++s_mapPostDraws;
			wnd->PostDraw();
			s_mapCapture = false;
		}
	}
	reinterpret_cast<void (*)(void*)>(Resolve(0x14047B980))(map);
	const auto transform = reinterpret_cast<void (*)(const void*, float*)>(Resolve(0x14047CC50));
	const auto eq = [&](float x, float y)
	{
		float p[3] = { x, y, 0.0f };
		transform(map, p);
		return ImVec2(inner.left + p[0], inner.top + p[1]);
	};
	const auto project = [&](float x, float y)
	{
		const ImVec2 at = eq(x, y);
		return ImVec2(at.x * ctx.map.scale, at.y * ctx.map.scale);
	};

	const PlayerClient* me = pLocalPlayer.get();
	const bool zoneMatches = me && Field<int>(me, 0x2ec) == static_cast<int>(map->zoneId);
	float zLow = 0.0f, zHigh = 0.0f;
	if (me)
	{
		const float feet = Field<float>(me, 0x7c) - Field<float>(me, 0x154) * 0.625f;
		zLow = feet - static_cast<float>(map->minZ);
		zHigh = feet + static_cast<float>(map->maxZ);
	}
	const bool zFilter = map->zFilterActive && me;

	const auto visible = [&](int layer) { return layer >= 0 && layer < 4 && ((map->layerFilter >> layer) & 1); };

	const MapSnapshot::Label* hoverLabel = nullptr;
	const MapSnapshot::Line*  hoverLine = nullptr;
	const CXPoint mouse = pWndMgr ? pWndMgr->MousePoint : CXPoint();
	if (wnd->IsActive() && mouse.x >= inner.left && mouse.x < inner.right && mouse.y >= inner.top && mouse.y < inner.bottom)
	{
		float labelDistance = 20.0f;
		float lineDistance = 3.0f;
		for (const MapSnapshot::Label& label : s_mapSnapshot.labels)
		{
			if (!visible(label.layer) || (zFilter && (label.z < zLow || label.z > zHigh)))
				continue;
			const ImVec2 at = eq(label.x, label.y);
			const float distance = std::hypot(mouse.x - at.x, mouse.y - at.y);
			if (distance < labelDistance)
			{
				hoverLabel = &label;
				labelDistance = distance;
			}
		}
		for (const MapSnapshot::Line& line : s_mapSnapshot.lines)
		{
			if (!visible(line.layer) || (zFilter && ((line.z1 < zLow && line.z2 < zLow) || (line.z1 > zHigh && line.z2 > zHigh))))
				continue;
			const ImVec2 a0 = eq(line.x1, line.y1);
			const ImVec2 b0 = eq(line.x2, line.y2);
			const float vx = b0.x - a0.x, vy = b0.y - a0.y;
			float dx = mouse.x - a0.x, dy = mouse.y - a0.y;
			const float along = dx * vx + dy * vy;
			if (along > 0.0f)
			{
				const float length = vx * vx + vy * vy;
				if (along < length)
				{
					dx = mouse.x - (a0.x + along / length * vx);
					dy = mouse.y - (a0.y + along / length * vy);
				}
				else
				{
					dx = mouse.x - b0.x;
					dy = mouse.y - b0.y;
				}
			}
			const float distance = std::hypot(dx, dy);
			if (distance < lineDistance)
			{
				hoverLine = &line;
				lineDistance = distance;
			}
		}
		if (hoverLabel && hoverLine)
		{
			if (labelDistance <= lineDistance)
				hoverLine = nullptr;
			else
				hoverLabel = nullptr;
		}
	}
	if (ctx.hosts)
	{
		if (__CurrentMapLabel)
			*reinterpret_cast<const void**>(__CurrentMapLabel) = hoverLabel ? hoverLabel->live : nullptr;
		*reinterpret_cast<const void**>(Resolve(0x140F5AA38)) = hoverLine ? hoverLine->live : nullptr;
	}

	ctx.dl->PushClipRect(ctx.map.Min(inner), ctx.map.Max(inner), true);

	for (const MapSnapshot::Line& line : s_mapSnapshot.lines)
	{
		if (!visible(line.layer))
			continue;
		if (zFilter && ((line.z1 < zLow && line.z2 < zLow) || (line.z1 > zHigh && line.z2 > zHigh)))
			continue;
		const ImU32 color = (line.marked || &line == hoverLine) ? IM_COL32(255, 255, 0, 255) : WithAlpha(Opaque(line.color), a);
		ctx.dl->AddLine(project(line.x1, line.y1), project(line.x2, line.y2), color);
	}

	if (map->lineActive && me)
		ctx.dl->AddLine(project(map->currentPoint.X, map->currentPoint.Y), project(-Field<float>(me, 0x78), -Field<float>(me, 0x74)),
			WithAlpha(Opaque(Field<uint32_t>(map, 0x324)), a));

	for (const MapSnapshot::Label& label : s_mapSnapshot.labels)
	{
		if (!visible(label.layer))
			continue;
		if (zFilter && (label.z < zLow || label.z > zHigh))
			continue;
		const bool highlighted = label.selected || &label == hoverLabel;
		const ImU32 color = highlighted ? IM_COL32(255, 255, 0, 255) : WithAlpha(Opaque(label.color), a);
		const ImVec2 at = project(label.x, label.y);
		const float s = ctx.map.scale;
		ctx.dl->AddLine(ImVec2(at.x - s, at.y), ImVec2(at.x + 2.0f * s, at.y), color);
		ctx.dl->AddLine(ImVec2(at.x, at.y - s), ImVec2(at.x, at.y + 2.0f * s), color);

		if ((!map->showLabels && !highlighted) || label.text.empty())
			continue;
		const int style = std::clamp(label.size, 1, 3) - 1;
		int x = static_cast<int>(at.x / s);
		int y = static_cast<int>(at.y / s);
		if (label.offsetX == 0 && label.offsetY == 0)
		{
			x -= label.width / 2;
			y -= label.height + 2;
		}
		else
		{
			x += label.offsetX;
			y += label.offsetY;
		}
		ControlText(ctx, style, label.text.c_str(), CXRect(x, y, x + 300, y + label.height), color, DrawText_NoWrap);
	}

	if (map->showGroup && zoneMatches)
	{
		const auto* pc = Field<const uint8_t*>(pLocalPC.get(), 0x2eb0);
		for (int i = 0; pc && i < 6; ++i)
		{
			const auto* member = Field<const uint8_t*>(pc, 8 + static_cast<size_t>(i) * 8);
			const auto* spawn = member ? Field<const uint8_t*>(member, 0x58) : nullptr;
			if (!spawn || spawn == reinterpret_cast<const uint8_t*>(me))
				continue;
			const ImVec2 at = project(-Field<float>(spawn, 0x78), -Field<float>(spawn, 0x74));
			const float s = 4.0f * ctx.map.scale;
			const ImU32 color = WithAlpha(Opaque(Field<uint32_t>(map, 0x32c)), a);
			ctx.dl->AddQuad(ImVec2(at.x, at.y - s), ImVec2(at.x + s, at.y), ImVec2(at.x, at.y + s), ImVec2(at.x - s, at.y), color);
			if (map->showNames)
			{
				const char* name = reinterpret_cast<const char*>(spawn + 0xb4);
				ControlText(ctx, 1, name, CXRect(static_cast<int>(at.x / ctx.map.scale) - 50, static_cast<int>(at.y / ctx.map.scale) - 16,
					static_cast<int>(at.x / ctx.map.scale) + 50, static_cast<int>(at.y / ctx.map.scale)), color, DrawText_NoWrap | DrawText_HCenter);
			}
		}
	}

	if (map->canAddToMap && me)
	{
		const ImVec2 at = project(-Field<float>(me, 0x78), -Field<float>(me, 0x74));
		const float angle = Field<float>(me, 0x90) * (6.2831853f / 512.0f);
		const ImVec2 forward(std::sin(angle), -std::cos(angle));
		const ImVec2 side(-forward.y, forward.x);
		const float s = ctx.map.scale;
		const ImVec2 tip(at.x + forward.x * 7.0f * s, at.y + forward.y * 7.0f * s);
		const ImVec2 left(at.x - forward.x * 4.0f * s + side.x * 4.0f * s, at.y - forward.y * 4.0f * s + side.y * 4.0f * s);
		const ImVec2 right(at.x - forward.x * 4.0f * s - side.x * 4.0f * s, at.y - forward.y * 4.0f * s - side.y * 4.0f * s);
		ctx.dl->AddTriangleFilled(tip, left, right, WithAlpha(Opaque(Field<uint32_t>(map, 0x328)), a));
	}

	ctx.dl->PopClipRect();

	char range[16];
	std::snprintf(range, sizeof(range), "%d", map->range);
	const char* format = pStringTable ? pStringTable->getString(0x1a4a) : nullptr;
	std::string text = format ? format : "%s";
	if (const size_t at = text.find("%s"); at != std::string::npos)
		text.replace(at, 2, range);
	else
		text = range;
	ControlText(ctx, 1, text.c_str(), CXRect(inner.left, inner.bottom - 14, inner.right - 2, inner.bottom),
		WithAlpha(IM_COL32(255, 255, 255, 255), a), DrawText_NoWrap | DrawText_RightJustify);
}

void DrawTabs(DrawContext& ctx, CTabWnd* tabs, uint8_t a)
{
	const Palette& p = Colors();
	const CXRect page = tabs->GetPageClientRect();
	ctx.dl->AddRect(ctx.map.Min(page), ctx.map.Max(page), WithAlpha(p.panelBorder, a), p.rounding);
	if (!tabs->bShowTabs)
		return;

	for (int i = 0; i < tabs->GetNumTabs(); ++i)
	{
		const CPageWnd* tabPage = tabs->GetPageFromTabIndex(i);
		if (!tabPage)
			continue;
		const CXRect tab = tabs->GetTabRect(i);
		const bool current = i == tabs->GetCurrentTabIndex();
		ctx.dl->AddRectFilled(ctx.map.Min(tab), ctx.map.Max(tab), WithAlpha(current ? p.buttonOn : p.button, a), p.rounding,
			ImDrawFlags_RoundCornersTop);
		ctx.dl->AddRect(ctx.map.Min(tab), ctx.map.Max(tab), WithAlpha(p.buttonEdge, a), p.rounding, ImDrawFlags_RoundCornersTop);
		const CXStr label = tabPage->GetTabText();
		ControlText(ctx, FontOf(tabs), label.c_str(), tab, WithAlpha(p.text, a),
			DrawText_NoWrap | DrawText_VCenter | DrawText_HCenter | DrawText_Elide);
	}
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

std::string MapStatus()
{
	char buffer[200];
	std::snprintf(buffer, sizeof(buffer), "map: hook %s, drawn %d, PostDraw calls %d, captures %d, lines %zu, labels %zu, inner %dx%d, active %d",
		s_mapHooked ? "on" : "off", s_mapDraws, s_mapPostDraws, s_mapCaptures, s_mapSnapshot.lines.size(),
		s_mapSnapshot.labels.size(), s_mapInner.GetWidth(), s_mapInner.GetHeight(), s_mapActive ? 1 : 0);
	return buffer;
}

void ShutdownControls()
{
	if (s_mapHooked)
		RemoveDetour(s_mapDrawAddress);
	s_mapHooked = false;
	s_mapHookTried = false;
}

void PulseSideEffectGauges()
{
	s_sideEffectValue[0] = s_sideEffectValue[1] = -1.0f;
	const CXWnd* bookWnd = pSpellBookWnd.get();
	if (!bookWnd || !bookWnd->IsVisible() || Debug().keepNative)
		return;
	const SkinnedWindow* book = FindSkinned("SpellBookWnd");
	const bool native = book && book->replace && book->wnd == bookWnd;
	if (!native && !IsMirrored(bookWnd))
		return;
	bool smooth = false;
	s_sideEffectValue[0] = reinterpret_cast<GaugeValueFn>(__GetGaugeValueFromEQ)(kGaugeMemorize, nullptr, &smooth, nullptr);
	s_sideEffectValue[1] = reinterpret_cast<GaugeValueFn>(__GetGaugeValueFromEQ)(kGaugeScribe, nullptr, &smooth, nullptr);
}

using CustomDrawFn = void (*)(DrawContext&, CXWnd*, uint8_t, const CXRect&);

struct CustomDraw
{
	const char*  name;
	uintptr_t    address;
	CustomDrawFn draw;
};

constexpr CustomDraw kCustomDraws[] = {
	{ "CBuffWindow::PostDraw",     0x1403aa120, nullptr },
	{ "CCastSpellWnd::PostDraw",   0x1403b1180, nullptr },
	{ "CContainerWnd::PostDraw",   0x1403d7ae0, nullptr },
	{ "CPlayerWnd::Draw",          0x1404d7180, DrawPlayerIndicators },
	{ "CTargetWnd::Draw",          0x14053c6d0, DrawTargetIndicators },
	{ "CCompassWnd::PostDraw",     0x1403d4660, DrawCompassMarkers },
	{ "CExtendedTargetWnd::Draw",  0x1402bcc00, DrawExtendedTargetHighlight },
};

constexpr size_t kDrawSlots[] = { 0x18, 0x20, 0x28, 0x48 };

void* const* BaseVTable(Widget widget)
{
	if (widget == Widget::Screen)
		return reinterpret_cast<void* const*>(CSidlScreenWnd__vftable);
	if (widget == Widget::Button)
		return reinterpret_cast<void* const*>(CButtonWnd__vftable);
	return nullptr;
}

const CustomDraw* FindCustomDraw(const void* function)
{
	const uintptr_t preferred = reinterpret_cast<uintptr_t>(function) - EQGameBaseAddress + EQGamePreferredAddress;
	for (const CustomDraw& custom : kCustomDraws)
		if (custom.address == preferred)
			return &custom;
	return nullptr;
}

bool InEqImage(const void* address)
{
	static uintptr_t size = 0;
	if (!size)
	{
		const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(EQGameBaseAddress);
		const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(EQGameBaseAddress + dos->e_lfanew);
		size = nt->OptionalHeader.SizeOfImage;
	}
	const uintptr_t at = reinterpret_cast<uintptr_t>(address);
	return at >= EQGameBaseAddress && at < EQGameBaseAddress + size;
}

const char* UnhandledDraw(const CXWnd* wnd, Widget widget)
{
	void* const* base = BaseVTable(widget);
	if (!base)
		return nullptr;
	void* const* vtable = *reinterpret_cast<void* const* const*>(wnd);
	if (!InEqImage(vtable))
		return nullptr;
	static char reason[96];
	for (const size_t slot : kDrawSlots)
	{
		if (vtable[slot / 8] == base[slot / 8] || FindCustomDraw(vtable[slot / 8]))
			continue;
		std::snprintf(reason, sizeof(reason), "%s slot 0x%zx @ %llx", wnd->GetWndClassName(), slot,
			static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(vtable[slot / 8]) - EQGameBaseAddress + EQGamePreferredAddress));
		return reason;
	}
	return nullptr;
}

void RunCustomDraws(DrawContext& ctx, CXWnd* wnd, Widget widget, uint8_t a, const CXRect& rect, bool post)
{
	void* const* base = BaseVTable(widget);
	if (!base)
		return;
	void* const* vtable = *reinterpret_cast<void* const* const*>(wnd);
	if (!InEqImage(vtable))
		return;
	for (const size_t slot : kDrawSlots)
		if ((slot == 0x28) == post && vtable[slot / 8] != base[slot / 8])
			if (const CustomDraw* custom = FindCustomDraw(vtable[slot / 8]); custom && custom->draw)
				custom->draw(ctx, wnd, a, rect);
}

const char* FirstUncovered(const CXWnd* wnd)
{
	if (!wnd->IsVisible())
		return nullptr;
	const Widget widget = WidgetOf(wnd);
	if (widget == Widget::None)
		return UITypeToString(TypeOf(wnd));
	if (widget == Widget::List && static_cast<const CListWnd*>(wnd)->bOwnerDraw)
		return "Listbox (owner drawn)";
	if (const char* reason = UnhandledDraw(wnd, widget))
		return reason;
	if (!wnd->Minimized && widget != Widget::HotButton)
		for (const CXWnd* child = wnd->GetFirstChildWnd(); child; child = child->GetNextSiblingWnd())
			if (const char* uncovered = FirstUncovered(child))
				return uncovered;
	return nullptr;
}

void DrawWindowTree(DrawContext& ctx, CXWnd* wnd, uint8_t alpha)
{
	if (!wnd->IsVisible())
		return;
	const CXRect clip = wnd->GetScreenClipRect();
	if (wnd->bFullyScreenClipped)
		return;

	uint8_t a = alpha;
	if (wnd->WindowStyle & CWS_USEMYALPHA)
		a = wnd->GetAlpha();
	if (wnd->Fades)
		a = static_cast<uint8_t>(wnd->FadeAlpha * a / 255);

	const CXRect rect = wnd->GetScreenRect();
	ctx.dl->PushClipRect(ctx.map.Min(clip), ctx.map.Max(clip), true);
	++ctx.controls;

	switch (WidgetOf(wnd))
	{
	case Widget::Screen:
		DrawScreen(ctx, wnd, a, rect);
		DrawStaticPieces(ctx, wnd, a);
		RunCustomDraws(ctx, wnd, Widget::Screen, a, rect, false);
		break;
	case Widget::Button:
		DrawButton(ctx, static_cast<CButtonWnd*>(wnd), a, rect);
		RunCustomDraws(ctx, wnd, Widget::Button, a, rect, false);
		break;
	case Widget::Map:
		if (wnd->BGType == 2 && !(wnd->WindowStyle & CWS_TRANSPARENT))
			ctx.dl->AddRectFilled(ctx.map.Min(rect), ctx.map.Max(rect), WithAlpha(FromARGB(wnd->BGColor), a));
		DrawMap(ctx, wnd, a);
		break;
	case Widget::SpellGem:
		DrawSpellGem(ctx, static_cast<CSpellGemWnd*>(wnd), a, rect);
		break;
	case Widget::Gauge:
		DrawGauge(ctx, static_cast<CGaugeWnd*>(wnd), a);
		break;
	case Widget::Label:
		DrawLabel(ctx, static_cast<CLabel*>(wnd), a, rect);
		break;
	case Widget::List:
		DrawList(ctx, static_cast<CListWnd*>(wnd), a);
		break;
	case Widget::Combo:
		DrawCombo(ctx, static_cast<CComboWnd*>(wnd), a, rect);
		break;
	case Widget::Edit:
		DrawEdit(ctx, static_cast<CEditWnd*>(wnd), a, rect);
		break;
	case Widget::InvSlot:
		DrawInvSlot(ctx, static_cast<CInvSlotWnd*>(wnd), a, rect);
		break;
	case Widget::Slider:
		DrawSlider(ctx, static_cast<CSliderWnd*>(wnd), a);
		break;
	case Widget::Tabs:
		DrawTabs(ctx, static_cast<CTabWnd*>(wnd), a);
		break;
	case Widget::Stml:
		DrawStml(ctx, static_cast<CStmlWnd*>(wnd), a);
		break;
	case Widget::HotButton:
		DrawHotButton(ctx, static_cast<CHotButton*>(wnd), a, rect);
		break;
	default:
		if (wnd->BGType == 2 && !(wnd->WindowStyle & CWS_TRANSPARENT))
			ctx.dl->AddRectFilled(ctx.map.Min(rect), ctx.map.Max(rect), WithAlpha(FromARGB(wnd->BGColor), a));
		break;
	}

	if (wnd->WindowStyle & CWS_VSCROLL)
		DrawScrollbar(ctx, wnd, a, rect, true);
	if (wnd->WindowStyle & CWS_HSCROLL)
		DrawScrollbar(ctx, wnd, a, rect, false);

	if (ctx.outlines)
		Outline(ctx, rect);

	if (!wnd->Minimized && WidgetOf(wnd) != Widget::HotButton)
		for (CXWnd* child = wnd->GetFirstChildWnd(); child; child = child->GetNextSiblingWnd())
			DrawWindowTree(ctx, child, a);

	if (const Widget widget = WidgetOf(wnd); widget == Widget::Screen || widget == Widget::Button)
		RunCustomDraws(ctx, wnd, widget, a, rect, true);

	ctx.dl->PopClipRect();
}

} // namespace eqgfx::ui
