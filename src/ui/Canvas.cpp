#include "ClassProfile.h"
#include "Controls.h"
#include "HotButtons.h"
#include "NativeUi.h"
#include "Skins.h"
#include "Suppress.h"

#include <algorithm>
#include <chrono>
#include <vector>

namespace eqgfx::ui {

namespace {

SkinnedWindow* GenericFor(const CXWnd* wnd, std::vector<SkinnedWindow>& windows)
{
	for (SkinnedWindow& w : windows)
		if (w.wnd == wnd && w.skin == Skin::Generic)
			return &w;
	return nullptr;
}

void CollectHoles(CXWnd* wnd, std::vector<CXRect>& out)
{
	const CXStr* name = wnd->GetWindowName();
	if (name && !name->empty())
	{
		out.push_back(wnd->GetScreenRect());
		return;
	}
	for (CXWnd* child = wnd->GetFirstChildWnd(); child; child = child->GetNextSiblingWnd())
		if (child->IsVisible())
			out.push_back(child->GetScreenRect());
}

std::vector<CXRect> Subtract(const CXRect& bounds, const std::vector<CXRect>& holes)
{
	std::vector<CXRect> pieces{ bounds };
	for (const CXRect& h : holes)
	{
		std::vector<CXRect> next;
		for (const CXRect& p : pieces)
		{
			if (p.left >= h.right || p.right <= h.left || p.top >= h.bottom || p.bottom <= h.top)
			{
				next.push_back(p);
				continue;
			}
			const int top = std::max(p.top, h.top);
			const int bottom = std::min(p.bottom, h.bottom);
			if (p.top < h.top)
				next.emplace_back(p.left, p.top, p.right, h.top);
			if (p.bottom > h.bottom)
				next.emplace_back(p.left, h.bottom, p.right, p.bottom);
			if (p.left < h.left)
				next.emplace_back(p.left, top, h.left, bottom);
			if (p.right > h.right)
				next.emplace_back(h.right, top, p.right, bottom);
		}
		pieces.swap(next);
	}
	return pieces;
}

float UiScale()
{
	const float extent = static_cast<float>(pWndMgr->ScreenExtentX);
	if (extent <= 0.0f)
		return 1.0f;
	const float scale = ImGui::GetIO().DisplaySize.x / extent;
	return (scale > 0.25f && scale < 8.0f) ? scale : 1.0f;
}

} // namespace

void DrawNativeUi()
{
	UpdateProfile();
	UpdateHotButtons();
	if (GetGameState() != GAMESTATE_INGAME || !pWndMgr)
		return;

	std::vector<SkinnedWindow>& windows = SkinnedWindows();
	bool any = MirroredCount() != 0;
	for (const SkinnedWindow& w : windows)
		any |= w.wnd != nullptr;
	if (!any)
		return;

	static ImDrawListSplitter layers;

	DrawContext ctx;
	ctx.dl = ImGui::GetBackgroundDrawList();
	ctx.layers = &layers;
	ctx.map.scale = UiScale();
	ctx.outlines = Debug().outlines;

	std::vector<CXWnd*> order;
	std::vector<CXRect> rects;
	for (CXWnd* top : pWndMgr->pWindows)
	{
		if (!top || top->GetParentWindow() || !top->IsVisible())
			continue;
		order.push_back(top);
		rects.push_back(top->GetScreenRect());
	}

	for (size_t k = 0; k < order.size(); ++k)
	{
		CXWnd* top = order[k];
		SkinnedWindow* w = GenericFor(top, windows);
		if (!w && !IsMirrored(top))
			continue;

		std::vector<CXRect> holes;
		std::vector<CXRect> above;
		for (size_t j = k + 1; j < order.size(); ++j)
		{
			if (HiddenByUs(order[j]))
			{
				if (GenericFor(order[j], windows))
					above.push_back(rects[j]);
				continue;
			}
			if (IsMirrored(order[j]))
				above.push_back(rects[j]);
			else
				CollectHoles(order[j], holes);
		}
		above.insert(above.end(), holes.begin(), holes.end());
		if (Debug().keepNative)
			holes.clear();
		ctx.above = &above;

		const CXRect bounds(rects[k].left - 4, rects[k].top - 4, rects[k].right + 4, rects[k].bottom + 4);

		const auto start = std::chrono::steady_clock::now();
		ctx.controls = 0;
		ctx.hosts = true;
		for (const CXRect& piece : Subtract(bounds, holes))
		{
			ctx.dl->PushClipRect(ctx.map.Min(piece), ctx.map.Max(piece), true);
			layers.Split(ctx.dl, 2);
			DrawWindowTree(ctx, top);
			layers.Merge(ctx.dl);
			ctx.dl->PopClipRect();
			ctx.hosts = false;
		}
		if (!w)
			continue;
		const float us = std::chrono::duration<float, std::micro>(std::chrono::steady_clock::now() - start).count();

		w->drawUs = w->drawUs * 0.9f + us * 0.1f;
		w->controls = ctx.controls;
	}

	for (SkinnedWindow& w : windows)
	{
		if (!w.wnd)
			continue;
		const auto start = std::chrono::steady_clock::now();
		switch (w.skin)
		{
		case Skin::Gems:   DrawGemBar(w); break;
		case Skin::Hotbar: DrawHotbar(w); break;
		case Skin::SpellBook: DrawSpellBook(w); break;
		case Skin::Actions: DrawActions(w); break;
		case Skin::CombatAbilities: DrawCombatAbilities(w); break;
		case Skin::Inventory: DrawInventory(w); break;
		case Skin::Chat: DrawChat(w); break;
		case Skin::AltAbilities: DrawAltAbilities(w); break;
		case Skin::Bandolier: DrawBandolier(w); break;
		case Skin::KeyRing: DrawKeyRing(w); break;
		case Skin::Mercenary: DrawMercenary(w); break;
		case Skin::Bags: DrawBags(w); break;
		default:           continue;
		}
		const float us = std::chrono::duration<float, std::micro>(std::chrono::steady_clock::now() - start).count();
		w.drawUs = w.drawUs * 0.9f + us * 0.1f;
	}

	DrawHotButtonEditor();
}

} // namespace eqgfx::ui
