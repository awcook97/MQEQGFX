#include "UnitWindow.h"
#include "../SettingsFile.h"

#include "../MQEQGFX.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace eqgfx::unitframes {

namespace {

constexpr int kBindBufferSize = 40;

constexpr const char* kWindowNames[kWindowCount] = {
	"player", "target", "focus", "group", "raid", "pet", "xtarget",
};

constexpr const char* kActionNames[kActionCount] = {
	"hotkey", "left", "right",
};

char s_bindBuffer[kWindowCount][kActionCount][kBindBufferSize];

std::atomic<bool> s_dirty{ false };
uint32_t          s_lastSaveMs = 0;

constexpr uint32_t kSaveIntervalMs = 1500;

void SyncBindBuffer(Window w, Action a)
{
	const int wi = static_cast<int>(w);
	const int ai = static_cast<int>(a);
	const std::string text = FormatRotation(SettingsFor(w).binds[ai]);

	std::strncpy(s_bindBuffer[wi][ai], text.c_str(), kBindBufferSize - 1);
	s_bindBuffer[wi][ai][kBindBufferSize - 1] = 0;
}

void MigrateLegacy(Window w, WindowSettings& cfg)
{
	const char* name = kWindowNames[static_cast<int>(w)];
	char key[160];

	auto legacyKey = [&](const char* prefix, const char* suffix) -> const char* {
		std::snprintf(key, sizeof(key), "%s%s", prefix, suffix);
		return key;
	};

	cfg.show = SettingGetBool(legacyKey("Show", name), cfg.show);
	cfg.locked = SettingGetBool(legacyKey("Locked", name), cfg.locked);
	cfg.buffs.show = SettingGetBool(legacyKey("Buffs", name), cfg.buffs.show);
	cfg.opacity = std::clamp(SettingGetFloat(legacyKey("Opacity", name), cfg.opacity), 0.0f, 1.0f);

	char prefix[64];
	std::snprintf(prefix, sizeof(prefix), "Bind%s", name);
	for (int a = 0; a < kActionCount; ++a)
	{
		const std::string value = SettingGetString(legacyKey(prefix, kActionNames[a]), "");
		if (!value.empty())
			ParseRotation(value.c_str(), cfg.binds[a]);
	}
}

void BindKey(char* buf, size_t size, Window w, Action a)
{
	std::snprintf(buf, size, "%sBind%s", kWindowNames[static_cast<int>(w)],
		kActionNames[static_cast<int>(a)]);
}

} // namespace

const char* WindowName(Window w)
{
	const int i = static_cast<int>(w);
	return i >= 0 && i < kWindowCount ? kWindowNames[i] : "?";
}

const char* ActionName(Action a)
{
	const int i = static_cast<int>(a);
	return i >= 0 && i < kActionCount ? kActionNames[i] : "?";
}

bool ParseWindow(const char* name, Window& out)
{
	if (!name || !*name)
		return false;

	for (int i = 0; i < kWindowCount; ++i)
	{
		if (ci_equals(name, kWindowNames[i]))
		{
			out = static_cast<Window>(i);
			return true;
		}
	}

	if (ci_equals(name, "xt"))     { out = Window::XTarget; return true; }
	if (ci_equals(name, "me"))     { out = Window::Player;  return true; }
	if (ci_equals(name, "frames")) { out = Window::Group;   return true; }
	return false;
}

bool ParseAction(const char* name, Action& out)
{
	if (!name || !*name)
		return false;

	for (int i = 0; i < kActionCount; ++i)
	{
		if (ci_equals(name, kActionNames[i]))
		{
			out = static_cast<Action>(i);
			return true;
		}
	}

	if (ci_equals(name, "key") || ci_equals(name, "cast"))
	{
		out = Action::Hotkey;
		return true;
	}
	return false;
}

std::string FormatRotation(const Rotation& r)
{
	std::string out;
	for (int i = 0; i < r.count; ++i)
	{
		if (i)
			out += '|';
		out += std::to_string(r.gems[i]);
	}
	return out;
}

bool ParseRotation(const char* text, Rotation& out)
{
	out.clear();
	if (!text || !*text)
		return true;

	if (ci_equals(text, "off") || ci_equals(text, "none") || ci_equals(text, "clear")
		|| ci_equals(text, "target"))
		return true;

	const char* p = text;
	while (*p && out.count < kMaxRotationGems)
	{
		while (*p == '|' || *p == ',' || *p == ' ')
			++p;
		if (!*p)
			break;

		char* endp = nullptr;
		const long gem = std::strtol(p, &endp, 10);
		if (endp == p)
			return false;
		if (gem < 1 || gem > NUM_SPELL_GEMS)
			return false;

		out.gems[out.count++] = static_cast<int>(gem);
		p = endp;
	}

	return true;
}

void DrawSharedOptions(Window w, const char* filter)
{
	if (filter && *filter
		&& ci_find_substr("gems rotation bind left right hotkey cast", filter) == -1)
		return;

	bool dirty = false;

	ImGui::SeparatorText("click and hotkey gems");
	ImGui::TextDisabled("gems, first one off cooldown fires:  1|2|3");

	for (int a = 0; a < kActionCount; ++a)
	{
		const Action action = static_cast<Action>(a);
		const int wi = static_cast<int>(w);

		ImGui::PushID(a);
		ImGui::SetNextItemWidth(110.0f);

		const bool entered = ImGui::InputText(ActionName(action), s_bindBuffer[wi][a],
			kBindBufferSize, ImGuiInputTextFlags_EnterReturnsTrue);
		const bool editing = ImGui::IsItemActive();

		if (entered || ImGui::IsItemDeactivatedAfterEdit())
		{
			Rotation parsed;
			if (ParseRotation(s_bindBuffer[wi][a], parsed))
			{
				SettingsFor(w).binds[a] = parsed;
				dirty = true;
			}
		}

		if (!editing)
			SyncBindBuffer(w, action);

		ImGui::PopID();
	}

	ImGui::TextDisabled("left casts, middle targets, quick right casts, hold right for the menu");
	ImGui::TextDisabled("hotkey is cast by /eqgfx cast - hovering a row by itself does nothing");

	if (dirty)
		MarkSettingsDirty();
}

bool s_onlyForeground = true;

bool OnlyWhenForeground()
{
	return s_onlyForeground;
}

void SetOnlyWhenForeground(bool on)
{
	if (s_onlyForeground == on)
		return;
	s_onlyForeground = on;
	MarkSettingsDirty();
}

bool ShouldDraw()
{
	if (GetGameState() != GAMESTATE_INGAME)
		return false;
	if (s_onlyForeground && !gbInForeground)
		return false;
	return true;
}

void Draw()
{
	ClearHoverWindow();
	ApplyForgets();

	const bool frames = ShouldDraw();
	if (!frames)
	{
		for (int i = 0; i < kWindowCount; ++i)
			BeginRowCapture(static_cast<Window>(i));
	}

	if (GetGameState() != GAMESTATE_INGAME)
		return;

	BeginAnimContext();

	for (int i = 0; i < kWindowCount; ++i)
	{
		UnitWindow& window = WindowFor(static_cast<Window>(i));
		if (frames && window.Shown())
			window.Draw();
		if (window.OptionsShown())
			window.DrawOptions();
	}

	if (PluginOptionsShown())
		DrawPluginOptions();

	EndAnimContext();
}

void Pulse()
{
	for (int i = 0; i < kWindowCount; ++i)
		WindowFor(static_cast<Window>(i)).Pulse();
}

void LoadSettings()
{
	char key[160];

	for (int i = 0; i < kWindowCount; ++i)
	{
		const Window w = static_cast<Window>(i);
		WindowSettings& cfg = SettingsFor(w);

		MigrateLegacy(w, cfg);

		WindowFor(w).Persist(false);

		for (int a = 0; a < kActionCount; ++a)
		{
			BindKey(key, sizeof(key), w, static_cast<Action>(a));
			const std::string value = SettingGetString(key, "");
			if (!value.empty())
				ParseRotation(value.c_str(), cfg.binds[a]);

			SyncBindBuffer(w, static_cast<Action>(a));
		}
	}

	s_onlyForeground = SettingGetBool("OnlyWhenForeground", s_onlyForeground);
}

namespace {
settings::Section* s_sink = nullptr;
uint32_t           s_lastChangeMs = 0;
}

bool SettingGetBool(const char* key, bool fallback)
{
	return settings::Global().GetBool(kSettingsSection, key, fallback);
}

int SettingGetInt(const char* key, int fallback)
{
	return settings::Global().GetInt(kSettingsSection, key, fallback);
}

float SettingGetFloat(const char* key, float fallback)
{
	return settings::Global().GetFloat(kSettingsSection, key, fallback);
}

std::string SettingGetString(const char* key, const char* fallback)
{
	return settings::Global().GetString(kSettingsSection, key, fallback ? fallback : "");
}

void SettingPutBool(const char* key, bool value)
{
	if (s_sink) s_sink->PutBool(key, value);
	else settings::Global().Set(kSettingsSection, key, value);
}

void SettingPutInt(const char* key, int value)
{
	if (s_sink) s_sink->PutInt(key, value);
	else settings::Global().Set(kSettingsSection, key, value);
}

void SettingPutFloat(const char* key, float value)
{
	if (s_sink) s_sink->PutFloat(key, value);
	else settings::Global().Set(kSettingsSection, key, value);
}

void SettingPutString(const char* key, const char* value)
{
	if (s_sink) s_sink->Put(key, value);
	else settings::Global().Set(kSettingsSection, key, value ? value : "");
}

void SaveSettings()
{
	char key[160];

	settings::Section section;
	s_sink = &section;

	SettingPutBool("OnlyWhenForeground", s_onlyForeground);

	for (int i = 0; i < kWindowCount; ++i)
	{
		const Window w = static_cast<Window>(i);
		const WindowSettings& cfg = SettingsFor(w);

		WindowFor(w).Persist(true);

		for (int a = 0; a < kActionCount; ++a)
		{
			BindKey(key, sizeof(key), w, static_cast<Action>(a));
			SettingPutString(key, FormatRotation(cfg.binds[a]).c_str());
		}
	}

	s_sink = nullptr;
	settings::Global().SetSection(kSettingsSection, std::move(section));

	s_dirty.store(false, std::memory_order_relaxed);
	s_lastSaveMs = static_cast<uint32_t>(EQGetTime());
}

void MarkSettingsDirty()
{
	s_dirty.store(true, std::memory_order_relaxed);
	s_lastChangeMs = static_cast<uint32_t>(EQGetTime());
}

void FlushSettings()
{
	if (!s_dirty.load(std::memory_order_relaxed))
		return;

	const uint32_t now = static_cast<uint32_t>(EQGetTime());
	if (now - s_lastChangeMs < kSaveIntervalMs || now - s_lastSaveMs < kSaveIntervalMs)
		return;

	SaveSettings();
}

void PrintHelp()
{
	WriteChatf("\ag[MQEQGFX]\ax \ayoptions\ax - hold right-click on any row and pick "
		"\ayOptions...\ax for that window, or \ay/eqgfx uf <window> options\ax. "
		"\ay/eqgfx options\ax opens the plugin-wide panel");
	WriteChatf("\ag[MQEQGFX]\ax \aywindows\ax - \ay/eqgfx uf <window> [on|off]\ax, "
		"windows: player target focus group raid pet xtarget (or \ayall\ax)");
	WriteChatf("\ag[MQEQGFX]\ax \aysize\ax - \ay/eqgfx uf <window> size <w> <h>|auto\ax, "
		"\aylock\ax - \ay/eqgfx uf <window> lock\ax, "
		"\ayreset\ax - \ay/eqgfx uf <window> reset\ax");
	WriteChatf("\ag[MQEQGFX]\ax \aygems\ax - \ay/eqgfx bind <window> <hotkey|left|right> <rotation>\ax");
	WriteChatf("\ag[MQEQGFX]\ax   a rotation is gems in fallback order: \ay1|2|3\ax casts 1, "
		"or 2 if 1 is on cooldown, or 3 if both are. \ayoff\ax clears it");
	WriteChatf("\ag[MQEQGFX]\ax \ayclicks\ax - left casts its rotation (targets when unbound), "
		"middle targets, a quick right-click casts, holding right-click opens the menu");
	WriteChatf("\ag[MQEQGFX]\ax   the frames are click-through everywhere except on a bar, "
		"so an idle window never eats a click meant for the game");
	WriteChatf("\ag[MQEQGFX]\ax \ayhotkey\ax - bind a key to \ay/eqgfx cast\ax. It casts the hotkey "
		"rotation of whichever window your mouse is over. Hovering alone never casts anything");
}

namespace {

void PrintStatus()
{
	WriteChatf("\ag[MQEQGFX]\ax unit frames:");
	for (int i = 0; i < kWindowCount; ++i)
	{
		const WindowSettings& cfg = SettingsFor(static_cast<Window>(i));
		char size[32];
		if (cfg.autoResize)
			std::snprintf(size, sizeof(size), "auto");
		else
			std::snprintf(size, sizeof(size), "%.0fx%.0f", cfg.width, cfg.height);

		WriteChatf("\ag[MQEQGFX]\ax   %-8s %s | %s%s | hotkey %s | left %s | right %s",
			kWindowNames[i],
			cfg.show ? "\agopen\ax " : "\ayclosed\ax",
			size,
			cfg.locked ? " locked" : "",
			cfg.binds[0].empty() ? "-" : FormatRotation(cfg.binds[0]).c_str(),
			cfg.binds[1].empty() ? "-" : FormatRotation(cfg.binds[1]).c_str(),
			cfg.binds[2].empty() ? "-" : FormatRotation(cfg.binds[2]).c_str());
	}
}

void CastCommand(const char* szLine)
{
	char arg[MAX_STRING] = { 0 };
	GetArg(arg, szLine, 2);

	Window w = Window::Target;
	const int hovered = HoverWindowIndex();
	if (arg[0])
	{
		if (!ParseWindow(arg, w))
		{
			WriteChatf("\ag[MQEQGFX]\ax \arno window called '%s'\ax", arg);
			return;
		}
	}
	else if (hovered >= 0)
	{
		w = static_cast<Window>(hovered);
	}

	const int unit = ResolveCastUnit();
	if (unit == 0)
	{
		WriteChatf("\ag[MQEQGFX]\ax \arnothing to cast at - point at a row, set a focus, or target something\ax");
		return;
	}

	const Rotation& r = SettingsFor(w).binds[static_cast<int>(Action::Hotkey)];
	if (r.empty())
	{
		WriteChatf("\ag[MQEQGFX]\ax \arno hotkey rotation bound for %s\ax - /eqgfx bind %s hotkey <gems>",
			WindowName(w), WindowName(w));
		return;
	}

	if (!FireRotation(r, unit))
		WriteChatf("\ag[MQEQGFX]\ax \aynothing in %s is ready\ax", FormatRotation(r).c_str());
}

void BindCommand(const char* szLine)
{
	char szWindow[MAX_STRING] = { 0 }, szAction[MAX_STRING] = { 0 }, szGems[MAX_STRING] = { 0 };
	GetArg(szWindow, szLine, 2);
	GetArg(szAction, szLine, 3);
	GetArg(szGems, szLine, 4);

	Window w;
	Action a;
	if (!szWindow[0] || !ParseWindow(szWindow, w))
	{
		WriteChatf("\ag[MQEQGFX]\ax usage: /eqgfx bind <player|target|focus|group|raid|pet|xtarget> "
			"<hotkey|left|right> <gems, e.g. 1|2|3>");
		return;
	}
	if (!szAction[0] || !ParseAction(szAction, a))
	{
		WriteChatf("\ag[MQEQGFX]\ax \arsecond argument must be hotkey, left or right\ax");
		return;
	}

	if (!szGems[0])
	{
		const Rotation& cur = SettingsFor(w).binds[static_cast<int>(a)];
		WriteChatf("\ag[MQEQGFX]\ax %s %s is %s", WindowName(w), ActionName(a),
			cur.empty() ? "unbound" : FormatRotation(cur).c_str());
		return;
	}

	Rotation parsed;
	if (!ParseRotation(szGems, parsed))
	{
		WriteChatf("\ag[MQEQGFX]\ax \ar'%s' is not a rotation\ax - gems 1-%d separated by |",
			szGems, NUM_SPELL_GEMS);
		return;
	}

	SettingsFor(w).binds[static_cast<int>(a)] = parsed;
	SyncBindBuffer(w, a);
	SaveSettings();

	if (parsed.empty())
		WriteChatf("\ag[MQEQGFX]\ax %s %s cleared", WindowName(w), ActionName(a));
	else
		WriteChatf("\ag[MQEQGFX]\ax %s %s casts \ay%s\ax", WindowName(w), ActionName(a),
			FormatRotation(parsed).c_str());
}

void WindowCommand(const char* szLine)
{
	char szWindow[MAX_STRING] = { 0 }, szState[MAX_STRING] = { 0 };
	char szA[MAX_STRING] = { 0 }, szB[MAX_STRING] = { 0 };
	GetArg(szWindow, szLine, 2);
	GetArg(szState, szLine, 3);
	GetArg(szA, szLine, 4);
	GetArg(szB, szLine, 5);

	if (!szWindow[0])
	{
		PrintStatus();
		return;
	}

	if (ci_equals(szWindow, "all"))
	{
		const bool on = !ci_equals(szState, "off");
		for (int i = 0; i < kWindowCount; ++i)
			WindowFor(static_cast<Window>(i)).Show(on);
		SaveSettings();
		WriteChatf("\ag[MQEQGFX]\ax all unit frame windows %s", on ? "\agON\ax" : "\ayOFF\ax");
		return;
	}

	Window w;
	if (!ParseWindow(szWindow, w))
	{
		WriteChatf("\ag[MQEQGFX]\ax \arno window called '%s'\ax - player target focus group raid pet xtarget",
			szWindow);
		return;
	}

	UnitWindow& window = WindowFor(w);
	WindowSettings& cfg = window.Config();

	if (ci_equals(szState, "options") || ci_equals(szState, "opts"))
	{
		window.ShowOptions(true);
		WriteChatf("\ag[MQEQGFX]\ax %s options open", window.Name());
		return;
	}

	if (ci_equals(szState, "size"))
	{
		if (ci_equals(szA, "auto"))
		{
			cfg.autoResize = true;
			SaveSettings();
			WriteChatf("\ag[MQEQGFX]\ax %s shrinks to fit its content", window.Name());
			return;
		}

		if (!szA[0] || !szB[0])
		{
			WriteChatf("\ag[MQEQGFX]\ax %s is %s - /eqgfx uf %s size <w> <h>, "
				"or \ayauto\ax to shrink to fit",
				window.Name(),
				cfg.autoResize ? "shrink-to-fit" : "fixed", window.Name());
			return;
		}

		const float newW = GetFloatFromString(szA, -1.0f);
		const float newH = GetFloatFromString(szB, -1.0f);
		if (!std::isfinite(newW) || !std::isfinite(newH) || newW < 0.0f || newH < 0.0f)
		{
			WriteChatf("\ag[MQEQGFX]\ax \arwidth and height must be numbers\ax - /eqgfx uf %s size <w> <h>",
				window.Name());
			return;
		}

		cfg.width = std::clamp(newW, 80.0f, 1200.0f);
		cfg.height = std::clamp(newH, 24.0f, 1400.0f);

		const bool wasAuto = cfg.autoResize;
		cfg.autoResize = false;

		SaveSettings();
		WriteChatf("\ag[MQEQGFX]\ax %s is now %.0fx%.0f%s", window.Name(),
			cfg.width, cfg.height, wasAuto ? " (shrink-to-fit off)" : "");
		return;
	}

	if (ci_equals(szState, "lock"))
	{
		cfg.locked = !cfg.locked;
		SaveSettings();
		WriteChatf("\ag[MQEQGFX]\ax %s %s", window.Name(),
			cfg.locked ? "\aglocked\ax" : "\ayunlocked\ax");
		return;
	}

	if (ci_equals(szState, "clickthrough") || ci_equals(szState, "ct"))
	{
		cfg.clickThrough = !cfg.clickThrough;
		SaveSettings();
		WriteChatf("\ag[MQEQGFX]\ax %s click-through %s", window.Name(),
			cfg.clickThrough ? "\agON\ax" : "\ayOFF\ax");
		return;
	}

	if (ci_equals(szState, "reset"))
	{
		window.ResetSettings();
		SaveSettings();
		WriteChatf("\ag[MQEQGFX]\ax %s settings reset to defaults", window.Name());
		return;
	}

	bool on = !window.Shown();
	if (ci_equals(szState, "on"))       on = true;
	else if (ci_equals(szState, "off")) on = false;

	window.Show(on);
	SaveSettings();
	WriteChatf("\ag[MQEQGFX]\ax %s window %s", window.Name(), on ? "\agON\ax" : "\ayOFF\ax");
}

} // namespace

bool Command(const char* szCmd, const char* szLine)
{
	if (ci_equals(szCmd, "uf") || ci_equals(szCmd, "unitframes"))
	{
		WindowCommand(szLine);
		return true;
	}

	if (ci_equals(szCmd, "options") || ci_equals(szCmd, "opts"))
	{
		char szState[MAX_STRING] = { 0 };
		GetArg(szState, szLine, 2);
		const bool on = !ci_equals(szState, "off");
		ShowPluginOptions(on);
		WriteChatf("\ag[MQEQGFX]\ax plugin options %s", on ? "\agON\ax" : "\ayOFF\ax");
		return true;
	}

	if (ci_equals(szCmd, "bind"))
	{
		BindCommand(szLine);
		return true;
	}

	if (ci_equals(szCmd, "cast"))
	{
		CastCommand(szLine);
		return true;
	}

	return false;
}

} // namespace eqgfx::unitframes
