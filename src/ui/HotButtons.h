#pragma once

#include "../SettingsFile.h"

#include <mq/Plugin.h>

#include <cstdint>
#include <string>
#include <vector>

namespace eqgfx::ui {

enum class ButtonIcon : uint8_t { None, Spell, Item };
enum class ButtonTimer : uint8_t { None, SpellGem, Item, AA, Disc };

struct HotButton
{
	std::string label;
	std::string cmd;
	ButtonIcon  iconType = ButtonIcon::None;
	int         icon = 0;
	ButtonTimer timer = ButtonTimer::None;
	std::string timerKey;

	bool Empty() const { return label.empty() && cmd.empty(); }
};

constexpr int kHotBars = MAX_HOTBUTTON_WNDS;
constexpr int kHotPages = NUM_HOTBUTTON_PAGES;
constexpr int kHotSlots = HOTBUTTONS_PER_PAGE;

void UpdateHotButtons();

int        HotBarPage(int bar);
HotButton& ButtonAt(int bar, int page, int slot);
void       ButtonChanged(int bar, int page);
void       SwapButtons(int bar, int page, int slot, int bar2, int page2, int slot2);

bool CursorHasButton();
bool AssignFromCursor(HotButton& b);

void MakeSpellButton(HotButton& b, const EQ_Spell* spell);

void     RunButton(const HotButton& b);
uint32_t ButtonCooldown(const HotButton& b, uint32_t& totalMs);

void DragButtonSource(const HotButton& b);
bool AcceptDraggedButton(HotButton& into);

std::vector<HotButton>& ButtonSet(const std::string& name, bool* stored = nullptr);
void SaveButtonSet(const std::string& name);

settings::json ButtonJson(const HotButton& b);
HotButton      ButtonFromJson(const settings::json& j);
HotButton      ButtonFromLegacy(const std::string& text);

uint32_t DiscCooldown(const EQ_Spell* spell);

} // namespace eqgfx::ui
