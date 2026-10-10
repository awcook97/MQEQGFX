#pragma once

#include "Suppress.h"

#include <imgui.h>

#include <functional>
#include <string>
#include <vector>

namespace eqgfx::ui {

void DrawGemBar(SkinnedWindow& w);
void DrawHotbar(SkinnedWindow& w);
void DrawSpellBook(SkinnedWindow& w);
void DrawActions(SkinnedWindow& w);
void DrawCombatAbilities(SkinnedWindow& w);
void DrawInventory(SkinnedWindow& w);
void DrawChat(SkinnedWindow& w);
void DrawAltAbilities(SkinnedWindow& w);
void DrawBandolier(SkinnedWindow& w);
void DrawKeyRing(SkinnedWindow& w);
void DrawMercenary(SkinnedWindow& w);
void DrawBags(SkinnedWindow& w);
void DrawHotButtonEditor();

struct HotButton;
void EditButton(std::string where, const HotButton& b, std::function<void(const HotButton&)> save,
	std::function<void()> remove = {});

struct NativeSlot
{
	ImVec2 offset;
	ImVec2 size;
};

inline NativeSlot SlotOf(const eqlib::CXWnd* parent, const eqlib::CXWnd* child)
{
	const eqlib::CXRect p = parent->GetScreenRect();
	const eqlib::CXRect c = child->GetScreenRect();
	return { ImVec2(static_cast<float>(c.left - p.left), static_cast<float>(c.top - p.top)),
		ImVec2(static_cast<float>(c.GetWidth()), static_cast<float>(c.GetHeight())) };
}

struct BarLayout
{
	float size = 0.0f;
	float spacing = 2.0f;
	int   columns = -1;
	float opacity = 0.88f;
	bool  labels = true;
	bool  keys = true;
	bool  locked = false;
};

struct Cell
{
	ImVec2 min;
	ImVec2 max;
};

const std::vector<Cell>& BeginBar(const char* id, const eqlib::CXWnd* native, const std::vector<NativeSlot>& natives);
const BarLayout& CurrentBar();
void EndBar();

bool LeftTap();

enum class Press { None, Tap, Long };
Press RightPress();

void SpellTooltip(const EQ_Spell* spell, int msLeft, bool buff, const char* caster = nullptr);

void FormatCooldown(uint32_t ms, char* out, size_t size);

void DrawCooldownOverlay(ImDrawList* dl, ImVec2 min, ImVec2 max, uint32_t leftMs, uint32_t totalMs);

uint32_t Countdown(const std::string& key, uint32_t readingMs, uint32_t& totalMs, uint32_t toleranceMs);

uint32_t GemReading(int gem);

uint32_t GemCooldown(int gem, uint32_t& totalMs);

uint32_t RecoveryLeft(uint32_t& totalMs);

void IconText(ImDrawList* dl, ImVec2 center, const char* text, ImU32 color);

} // namespace eqgfx::ui
