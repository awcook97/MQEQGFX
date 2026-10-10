#pragma once

#include "Widgets.h"

#include <string>

namespace eqgfx::ui {

struct DrawContext
{
	ImDrawList*         dl = nullptr;
	ImDrawListSplitter* layers = nullptr;
	Mapping             map;
	bool                outlines = false;
	int                 controls = 0;
};

void DrawWindowTree(DrawContext& ctx, eqlib::CXWnd* wnd, uint8_t alpha = 255);

std::string LabelText(int eqType);

void ForgetControlCache();

void PulseSideEffectGauges();

} // namespace eqgfx::ui
