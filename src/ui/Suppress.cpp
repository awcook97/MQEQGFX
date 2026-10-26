#include "Suppress.h"
#include "Actions.h"
#include "ChatLog.h"
#include "ClassProfile.h"
#include "Controls.h"
#include "HotButtons.h"
#include "Layouts.h"
#include "NativeUi.h"

#include <algorithm>
#include <array>
#include <cstring>

namespace eqgfx::ui {

namespace {

struct KnownWindow
{
	const char* name;
	Skin        skin;
};

constexpr KnownWindow kKnownWindows[] = {
	{ "PlayerWindow", Skin::Hide },
	{ "TargetWindow", Skin::Hide },
	{ "TargetOfTargetWindow", Skin::Hide },
	{ "GroupWindow", Skin::Hide },
	{ "PetInfoWindow", Skin::Hide },
	{ "ExtendedTargetWnd", Skin::Hide },
	{ "RaidWindow", Skin::Hide },
	{ "CastingWindow", Skin::Hide },
	{ "BuffWindow", Skin::Hide },
	{ "ShortDurationBuffWindow", Skin::Hide },
	{ "CastSpellWnd", Skin::Gems },
	{ "SelectorWnd", Skin::Generic },
	{ "ActionsWindow", Skin::Actions },
	{ "CombatAbilityWnd", Skin::CombatAbilities },
	{ "CompassWindow", Skin::Generic },
	{ "SpellBookWnd", Skin::SpellBook },
	{ "InventoryWindow", Skin::Inventory },
	{ kChatName, Skin::Chat },
	{ "AAWindow", Skin::AltAbilities },
	{ "BandolierWnd", Skin::Bandolier },
	{ "KeyRingWnd", Skin::KeyRing },
	{ "MMGW_ManageWnd", Skin::Mercenary },
	{ kBagsName, Skin::Bags },
};

constexpr size_t kMaxSkinned = 128;
constexpr size_t kMaxWindowName = 200;

std::vector<SkinnedWindow> s_windows;
UiDebug s_debug;

constexpr size_t kMaxHidden = 64;
std::array<const CXWnd*, kMaxHidden> s_hidden = {};
size_t s_hiddenCount = 0;

bool s_hookInstalled = false;
bool s_hookTried = false;
std::vector<CXWnd*> s_chatTops;
std::vector<CXWnd*> s_bagWnds;
bool s_hotButtonHookInstalled = false;
bool s_hotButtonHookTried = false;

bool IsHidden(const CXWnd* wnd)
{
	for (size_t i = 0; i < s_hiddenCount; ++i)
		if (s_hidden[i] == wnd)
			return true;
	return false;
}

class CXWndDrawHook
{
public:
	DETOUR_TRAMPOLINE_DEF_MEMBER(CXWndDrawHook, int, DoAllDrawing_Trampoline, ())
	int DoAllDrawing_Detour()
	{
		const CXWnd* self = reinterpret_cast<const CXWnd*>(this);
		if (s_hiddenCount && !self->GetParentWindow() && IsHidden(self))
			return 0;
		return DoAllDrawing_Trampoline();
	}
};

bool RunOurHotButton(const CHotButtonWnd* wnd, int slot)
{
	if (s_debug.keepNative || ProfileClass().empty() || slot < 0 || slot >= kHotSlots)
		return false;
	for (const SkinnedWindow& w : s_windows)
	{
		if (w.skin != Skin::Hotbar || !w.wnd || w.wnd != wnd)
			continue;
		const HotButton& b = ButtonAt(w.bar, HotBarPage(w.bar), slot);
		if (!b.Empty())
			RunButton(b);
		return true;
	}
	return false;
}

class HotButtonHook
{
public:
	DETOUR_TRAMPOLINE_DEF_MEMBER(HotButtonHook, void, DoHotButton_Trampoline, (int, BOOL, const KeyCombo*))
	void DoHotButton_Detour(int slot, BOOL mouseClick, const KeyCombo* combo)
	{
		if (!RunOurHotButton(reinterpret_cast<const CHotButtonWnd*>(this), slot))
			DoHotButton_Trampoline(slot, mouseClick, combo);
	}
};

void InstallHotButtonHook()
{
	if (s_hotButtonHookInstalled || s_hotButtonHookTried || !CHotButtonWnd__DoHotButton)
		return;
	s_hotButtonHookTried = true;
	s_hotButtonHookInstalled = EzDetour(CHotButtonWnd__DoHotButton, &HotButtonHook::DoHotButton_Detour,
		&HotButtonHook::DoHotButton_Trampoline);
	if (!s_hotButtonHookInstalled)
		WriteChatf("\ag[MQEQGFX]\ax native ui: could not hook the hot button keys");
}

void InstallDrawHook()
{
	if (s_hookInstalled || s_hookTried || !CXWnd__DoAllDrawing)
		return;
	s_hookTried = true;
	s_hookInstalled = EzDetour(CXWnd__DoAllDrawing, &CXWndDrawHook::DoAllDrawing_Detour,
		&CXWndDrawHook::DoAllDrawing_Trampoline);
	if (!s_hookInstalled)
		WriteChatf("\ag[MQEQGFX]\ax native ui: could not hook window drawing, replaced windows stay as they are");
}

void RemoveDrawHook()
{
	s_hiddenCount = 0;
	s_hookTried = false;
	s_hotButtonHookTried = false;
	if (s_hotButtonHookInstalled)
	{
		RemoveDetour(CHotButtonWnd__DoHotButton);
		s_hotButtonHookInstalled = false;
	}
	if (!s_hookInstalled)
		return;
	RemoveDetour(CXWnd__DoAllDrawing);
	s_hookInstalled = false;
}

void SetClickThrough(SkinnedWindow& w, CXWnd* want)
{
	if (w.clickThrough == want)
		return;
	if (w.clickThrough)
		w.clickThrough->WindowStyle &= ~CWS_NOHITTEST;
	w.clickThrough = nullptr;
	if (want && !(want->WindowStyle & CWS_NOHITTEST))
	{
		want->WindowStyle |= CWS_NOHITTEST;
		w.clickThrough = want;
	}
}

void ChatTops(std::vector<CXWnd*>& out)
{
	out.clear();
	CChatWindowManager* cm = pChatManager.get();
	if (!cm)
		return;
	for (CChatWindow* chat : cm->ChatWindows)
	{
		if (!chat)
			continue;
		CXWnd* top = chat;
		while (top->GetParentWindow())
			top = top->GetParentWindow();
		if (std::find(out.begin(), out.end(), top) == out.end())
			out.push_back(top);
	}
}

void PulseChat(SkinnedWindow& w)
{
	std::vector<CXWnd*> tops;
	ChatTops(tops);
	const bool ours = w.replace && !tops.empty() && !s_debug.keepNative && s_hookInstalled;

	std::vector<CXWnd*> kept;
	for (CXWnd* t : s_chatTops)
	{
		if (std::find(tops.begin(), tops.end(), t) == tops.end())
			continue;
		if (ours)
			kept.push_back(t);
		else
			t->WindowStyle &= ~CWS_NOHITTEST;
	}
	s_chatTops.swap(kept);

	w.wnd = w.replace && !tops.empty() ? tops.front() : nullptr;
	if (!ours)
		return;
	for (CXWnd* t : tops)
	{
		if (!(t->WindowStyle & CWS_NOHITTEST))
		{
			t->WindowStyle |= CWS_NOHITTEST;
			s_chatTops.push_back(t);
		}
		if (s_hiddenCount < kMaxHidden)
			s_hidden[s_hiddenCount++] = t;
	}
}

void OpenBags(std::vector<CXWnd*>& out)
{
	out.clear();
	CContainerMgr* mgr = pContainerMgr.get();
	if (!mgr)
		return;
	for (CContainerWnd* wnd : mgr->pContainerWnds)
		if (wnd && wnd->IsVisible() && wnd->Container
			&& wnd->Location.GetLocation() == eItemContainerPossessions)
			out.push_back(wnd);
}

void PulseBags(SkinnedWindow& w)
{
	std::vector<CXWnd*> bags;
	OpenBags(bags);
	const bool ours = w.replace && !s_debug.keepNative && s_hookInstalled;

	std::vector<CXWnd*> kept;
	for (CXWnd* b : s_bagWnds)
	{
		if (std::find(bags.begin(), bags.end(), b) == bags.end())
			continue;
		if (ours)
			kept.push_back(b);
		else
			b->WindowStyle &= ~CWS_NOHITTEST;
	}
	s_bagWnds.swap(kept);

	w.wnd = w.replace && !bags.empty() ? bags.front() : nullptr;
	if (!ours)
		return;
	for (CXWnd* b : bags)
	{
		if (!(b->WindowStyle & CWS_NOHITTEST))
		{
			b->WindowStyle |= CWS_NOHITTEST;
			s_bagWnds.push_back(b);
		}
		if (s_hiddenCount < kMaxHidden)
			s_hidden[s_hiddenCount++] = b;
	}
}

void ForgetWindows()
{
	s_bagWnds.clear();
	s_chatTops.clear();
	s_hiddenCount = 0;
	for (SkinnedWindow& w : s_windows)
	{
		w.wnd = nullptr;
		w.clickThrough = nullptr;
	}
	ForgetControlCache();
	ForgetIconSheets();
}

bool SetReplace(SkinnedWindow& w, bool on)
{
	if (w.replace == on)
		return false;
	w.replace = on;
	MarkUiSettingsDirty();
	return true;
}

void ListWindows()
{
	WriteChatf("\ag[MQEQGFX]\ax native ui: window replacement %s, %d hidden", s_hookInstalled ? "\agactive\ax" : "\arinactive\ax", HiddenCount());
	for (const SkinnedWindow& w : s_windows)
		WriteChatf("\ag[MQEQGFX]\ax   %s %s%s", w.replace ? "\ag[ours]\ax  " : "\a-w[stock]\ax ", w.name.c_str(),
			w.wnd ? "" : " \a-w(not loaded)\ax");
}

} // namespace

std::vector<SkinnedWindow>& SkinnedWindows() { return s_windows; }
UiDebug& Debug()                              { return s_debug; }
bool DrawHookInstalled()                      { return s_hookInstalled; }
int  HiddenCount()                            { return static_cast<int>(s_hiddenCount); }

SkinnedWindow* FindSkinned(std::string_view name)
{
	for (SkinnedWindow& w : s_windows)
		if (ci_equals(w.name, name))
			return &w;
	return nullptr;
}

SkinnedWindow& AddSkinned(std::string_view name, Skin skin)
{
	if (SkinnedWindow* w = FindSkinned(name))
		return *w;
	SkinnedWindow w;
	w.name = std::string(name);
	w.skin = skin;
	s_windows.push_back(std::move(w));
	return s_windows.back();
}

SkinnedWindow* TryAddSkinned(std::string_view name)
{
	if (SkinnedWindow* w = FindSkinned(name))
		return w;
	if (s_windows.size() >= kMaxSkinned || name.empty() || name.size() > kMaxWindowName)
		return nullptr;
	return &AddSkinned(name);
}

void InitNativeUi()
{
	s_windows.clear();
	s_windows.reserve(kMaxSkinned);
	for (const KnownWindow& known : kKnownWindows)
		AddSkinned(known.name, known.skin);
	for (int bar = 0; bar < kHotBars; ++bar)
	{
		char name[32];
		if (bar == 0)
			strcpy_s(name, "HotButtonWnd");
		else
			sprintf_s(name, "HotButtonWnd%d", bar + 1);
		AddSkinned(name, Skin::Hotbar).bar = bar;
	}
	LoadUiSettings();
}

void ShutdownNativeUi()
{
	for (SkinnedWindow& w : s_windows)
	{
		SetClickThrough(w, nullptr);
		if (w.skin == Skin::Chat || w.skin == Skin::Bags)
		{
			SkinnedWindow restore = w;
			restore.replace = false;
			if (w.skin == Skin::Chat)
				PulseChat(restore);
			else
				PulseBags(restore);
		}
	}
	RemoveDrawHook();
	FlushUiSettings();
	FlushProfile();
	ForgetWindows();
}

void NativeUiCleanUI()  { ForgetWindows(); }
void NativeUiReloadUI() { ForgetWindows(); }

void PulseNativeUi()
{
	PulseUiSettings();

	s_hiddenCount = 0;
	if (GetGameState() != GAMESTATE_INGAME)
	{
		for (SkinnedWindow& w : s_windows)
		{
			SetClickThrough(w, nullptr);
			w.wnd = nullptr;
		}
		return;
	}

	RunQueuedActions();
	RunChatLinks();

	bool any = false;
	bool hotbars = false;
	for (SkinnedWindow& w : s_windows)
	{
		if (w.skin == Skin::Chat)
		{
			PulseChat(w);
			any |= w.wnd != nullptr;
			continue;
		}
		if (w.skin == Skin::Bags)
		{
			PulseBags(w);
			any |= w.wnd != nullptr;
			continue;
		}
		CXWnd* found = FindMQ2Window(w.name.c_str());
		if (w.clickThrough != found)
			w.clickThrough = nullptr;
		CXWnd* live = w.replace ? found : nullptr;
		w.wnd = live;

		const bool ours = live && !s_debug.keepNative;
		SetClickThrough(w, ours && s_hookInstalled && w.skin != Skin::Generic ? live : nullptr);
		if (!live)
			continue;
		any = true;
		hotbars |= w.skin == Skin::Hotbar;
		if (ours && s_hiddenCount < kMaxHidden)
			s_hidden[s_hiddenCount++] = live;
	}

	if (any)
		InstallDrawHook();
	if (hotbars)
		InstallHotButtonHook();

	PulseSideEffectGauges();
}

void NativeUiIncomingChat(const char* line, unsigned long color) { AddEqChat(line, static_cast<uint32_t>(color)); }
void NativeUiWriteChat(const char* line, int color)               { AddMqChat(line, color); }

std::string ReplaceList()
{
	std::string list;
	for (const SkinnedWindow& w : s_windows)
		if (w.replace)
			list += (list.empty() ? "" : ",") + w.name;
	return list;
}

void ApplyReplaceList(std::string_view list)
{
	std::vector<std::string> names;
	for (const std::string& name : split(std::string(list), ','))
		if (!name.empty())
			names.push_back(name);
	for (const std::string& name : names)
		TryAddSkinned(name);
	for (SkinnedWindow& w : s_windows)
		SetReplace(w, std::find_if(names.begin(), names.end(),
			[&](const std::string& n) { return ci_equals(n, w.name); }) != names.end());
}

bool NativeUiCommand(const char* szLine)
{
	char szArg[MAX_STRING] = { 0 };
	GetArg(szArg, szLine, 2);
	char szState[MAX_STRING] = { 0 };
	GetArg(szState, szLine, 3);

	if (!szArg[0] || ci_equals(szArg, "list"))
	{
		ListWindows();
		return true;
	}

	if (ci_equals(szArg, "layout"))
		return LayoutCommand(szLine);

	if (ci_equals(szArg, "native"))
	{
		s_debug.keepNative = ci_equals(szState, "on") ? true : ci_equals(szState, "off") ? false : !s_debug.keepNative;
		WriteChatf("\ag[MQEQGFX]\ax native ui: EQ %s drawing replaced windows", s_debug.keepNative ? "keeps" : "stops");
		return true;
	}

	const bool on = !ci_equals(szState, "off");
	if (ci_equals(szArg, "all"))
	{
		for (SkinnedWindow& w : s_windows)
			SetReplace(w, on);
		WriteChatf("\ag[MQEQGFX]\ax native ui: every listed window %s", on ? "\agours\ax" : "\aystock\ax");
		return true;
	}

	SkinnedWindow* w = FindSkinned(szArg);
	if (!w)
	{
		if (!on)
		{
			WriteChatf("\ag[MQEQGFX]\ax native ui: %s is not replaced", szArg);
			return true;
		}
		if (std::strlen(szArg) > kMaxWindowName || !FindMQ2Window(szArg))
		{
			WriteChatf("\ag[MQEQGFX]\ax native ui: no window named %s", szArg);
			return true;
		}
		w = TryAddSkinned(szArg);
		if (!w)
		{
			WriteChatf("\ag[MQEQGFX]\ax native ui: too many windows listed");
			return true;
		}
	}
	const bool state = szState[0] ? on : !w->replace;
	SetReplace(*w, state);
	WriteChatf("\ag[MQEQGFX]\ax native ui: %s %s", w->name.c_str(), state ? "\agours\ax" : "\aystock\ax");
	return true;
}

} // namespace eqgfx::ui
