#pragma once

#include <mq/Plugin.h>

#include <string>
#include <string_view>
#include <vector>

namespace eqgfx::ui {

enum class Skin
{
	Hide,
	Generic,
	Gems,
	Hotbar,
	SpellBook,
	Actions,
	CombatAbilities,
	Inventory,
	Chat,
	AltAbilities,
	Bandolier,
	KeyRing,
	Mercenary,
	Bags,
};

constexpr const char* kChatName = "ChatWindows";
constexpr const char* kBagsName = "Bags";

struct SkinnedWindow
{
	std::string    name;
	Skin           skin = Skin::Generic;
	bool           replace = false;
	eqlib::CXWnd*  wnd = nullptr;
	eqlib::CXWnd*  clickThrough = nullptr;
	int            bar = -1;

	float          drawUs = 0.0f;
	int            controls = 0;
};

std::vector<SkinnedWindow>& SkinnedWindows();
SkinnedWindow* FindSkinned(std::string_view name);
SkinnedWindow& AddSkinned(std::string_view name, Skin skin = Skin::Generic);
SkinnedWindow* TryAddSkinned(std::string_view name);

struct UiDebug
{
	bool keepNative = false;
	bool outlines = false;
};
UiDebug& Debug();

bool DrawHookInstalled();
int  HiddenCount();

std::string ReplaceList();
void ApplyReplaceList(std::string_view list);

void LoadUiSettings();
void MarkUiSettingsDirty();
void PulseUiSettings();
void FlushUiSettings();

} // namespace eqgfx::ui
