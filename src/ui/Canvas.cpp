#include "ClassProfile.h"
#include "Controls.h"
#include "HotButtons.h"
#include "NativeUi.h"
#include "Skins.h"
#include "Suppress.h"

#include <chrono>

namespace eqgfx::ui {

namespace {

SkinnedWindow* GenericFor(const CXWnd* wnd, std::vector<SkinnedWindow>& windows)
{
	for (SkinnedWindow& w : windows)
		if (w.wnd == wnd && w.skin == Skin::Generic)
			return &w;
	return nullptr;
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
	bool any = false;
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

	for (CXWnd* top : pWndMgr->pWindows)
	{
		if (!top)
			continue;
		SkinnedWindow* w = GenericFor(top, windows);
		if (!w)
			continue;

		const auto start = std::chrono::steady_clock::now();
		ctx.controls = 0;
		layers.Split(ctx.dl, 2);
		DrawWindowTree(ctx, top);
		layers.Merge(ctx.dl);
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
