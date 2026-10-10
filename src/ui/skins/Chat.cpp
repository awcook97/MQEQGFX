#include "../ChatLog.h"
#include "../ClassProfile.h"
#include "../Skins.h"
#include "../Widgets.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace eqgfx::ui {

namespace {

constexpr size_t kKeepLines = 5000;
constexpr int    kShowLines = 400;
constexpr const char* kId = "Chat##eqgfx";

struct Tab
{
	std::string   name;
	ChatFilterSet shows;
};

std::vector<ChatLine>    s_lines;
std::vector<Tab>         s_tabs;
int                      s_tabsSerial = -1;
char                     s_input[512] = {};
bool                     s_focusInput = false;
bool                     s_cursorToEnd = false;
std::vector<std::string> s_history;
int                      s_historyPos = -1;
char                     s_rename[32] = {};

ChatFilterSet Groups(std::initializer_list<const char*> groups)
{
	ChatFilterSet set;
	for (const char* g : groups)
		set |= ChatFilterGroup(g);
	return set;
}

void DefaultTabs()
{
	s_tabs = {
		{ "All", AllChatFilters() },
		{ "Chat", Groups({ "Chat", "Chat channels" }) },
		{ "Combat", Groups({ "Melee", "Spells", "Pets", "Damage shields" }) },
		{ "MQ", Groups({ "MacroQuest" }) },
	};
}

void SaveTabs()
{
	const std::span<const ChatFilter> filters = ChatFilters();
	const ChatFilterSet all = AllChatFilters();
	settings::json tabs = settings::json::array();
	for (const Tab& t : s_tabs)
	{
		settings::json tab = { { "name", t.name } };
		if (t.shows == all)
			tab["all"] = true;
		else
		{
			settings::json keys = settings::json::array();
			for (size_t i = 0; i < filters.size(); ++i)
				if (t.shows.test(i))
					keys.push_back(filters[i].key);
			tab["filters"] = std::move(keys);
		}
		tabs.push_back(std::move(tab));
	}
	settings::json& chat = ProfileRoot()["Chat"];
	if (!chat.is_object())
		chat = settings::json::object();
	chat["Tabs"] = std::move(tabs);
	MarkProfileDirty();
}

void LoadTabs()
{
	s_tabs.clear();
	const settings::json& chat = ProfileRoot()["Chat"];
	const auto tabs = chat.is_object() ? chat.find("Tabs") : chat.end();
	if (chat.is_object() && tabs != chat.end() && tabs->is_array())
		for (const settings::json& t : *tabs)
		{
			if (!t.is_object())
				continue;
			Tab tab;
			if (const auto name = t.find("name"); name != t.end())
				tab.name = settings::AsString(*name, "");
			if (const auto all = t.find("all"); all != t.end() && settings::AsBool(*all, false))
				tab.shows = AllChatFilters();
			else if (const auto keys = t.find("filters"); keys != t.end() && keys->is_array())
			{
				for (const settings::json& key : *keys)
					if (const int i = ChatFilterIndex(settings::AsString(key, "")); i >= 0)
						tab.shows.set(static_cast<size_t>(i));
			}
			else if (const auto mask = t.find("mask"); mask != t.end() && mask->is_number())
				tab.shows = ChatFiltersForLegacyMask(mask->get<uint32_t>());
			s_tabs.push_back(std::move(tab));
		}
	if (s_tabs.empty())
		DefaultTabs();
}

void TakeEqInput()
{
	CChatWindowManager* cm = pChatManager.get();
	if (!cm || !pWndMgr)
		return;
	for (CChatWindow* chat : cm->ChatWindows)
	{
		CEditWnd* edit = chat ? chat->GetInputWnd() : nullptr;
		if (!edit || pWndMgr->FocusWindow != edit)
			continue;
		strncpy_s(s_input, sizeof(s_input), edit->InputText.c_str(), _TRUNCATE);
		edit->InputText.clear();
		edit->StartPos = edit->EndPos = 0;
		edit->ClrFocus();
		s_focusInput = true;
		s_cursorToEnd = true;
		return;
	}
}

struct Piece
{
	std::string_view text;
	uint32_t         rgba;
	size_t           open = std::string::npos;
	size_t           length = 0;
};

void DrawLine(const ChatLine& line)
{
	const std::string& t = line.text;
	if (t.find(ITEM_TAG_CHAR) == std::string::npos)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, line.rgba);
		ImGui::TextWrapped("%s", t.c_str());
		ImGui::PopStyleColor();
		return;
	}

	constexpr uint32_t kLink = IM_COL32(120, 180, 255, 255);
	std::vector<Piece> pieces;
	for (size_t pos = 0; pos < t.size(); )
	{
		const size_t open = t.find(ITEM_TAG_CHAR, pos);
		const size_t close = open == std::string::npos ? std::string::npos : t.find(ITEM_TAG_CHAR, open + 1);
		if (close == std::string::npos)
		{
			pieces.push_back({ std::string_view(t).substr(pos), line.rgba });
			break;
		}
		if (open > pos)
			pieces.push_back({ std::string_view(t).substr(pos, open - pos), line.rgba });
		const TextTagInfo tag = ExtractLink(std::string_view(t.data() + open, close - open + 1));
		pieces.push_back({ tag.text.empty() ? std::string_view("[link]") : tag.text, kLink, open, close - open + 1 });
		pos = close + 1;
	}

	const float left = ImGui::GetCursorPosX();
	const float right = left + ImGui::GetContentRegionAvail().x;
	float x = left;
	bool lineStart = true;
	for (const Piece& piece : pieces)
	{
		for (size_t i = 0; i < piece.text.size(); )
		{
			size_t end = piece.text.find(' ', i);
			end = end == std::string_view::npos ? piece.text.size() : end;
			while (end < piece.text.size() && piece.text[end] == ' ')
				++end;
			const std::string_view word = piece.text.substr(i, end - i);
			i = end;

			const float width = ImGui::CalcTextSize(word.data(), word.data() + word.size()).x;
			if (!lineStart && x + width > right)
			{
				x = left;
				lineStart = true;
			}
			if (!lineStart)
				ImGui::SameLine(0.0f, 0.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, piece.rgba);
			ImGui::TextUnformatted(word.data(), word.data() + word.size());
			ImGui::PopStyleColor();
			x += width;
			lineStart = false;

			if (piece.open == std::string::npos)
				continue;
			if (ImGui::IsItemHovered())
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
				const ImVec2 lo = ImGui::GetItemRectMin();
				const ImVec2 hi = ImGui::GetItemRectMax();
				ImGui::GetWindowDrawList()->AddLine(ImVec2(lo.x, hi.y), hi, kLink);
			}
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				QueueChatLink(t, piece.open, piece.length);
		}
	}
}

void TabMenu(size_t index)
{
	Tab& tab = s_tabs[index];
	if (ImGui::IsWindowAppearing())
		strncpy_s(s_rename, sizeof(s_rename), tab.name.c_str(), _TRUNCATE);
	ImGui::SetNextItemWidth(140.0f);
	if (ImGui::InputText("name", s_rename, sizeof(s_rename), ImGuiInputTextFlags_EnterReturnsTrue) && s_rename[0])
	{
		tab.name = s_rename;
		SaveTabs();
	}
	ImGui::Separator();
	bool changed = false;
	if (ImGui::SmallButton("show all"))
	{
		tab.shows = AllChatFilters();
		changed = true;
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("show none"))
	{
		tab.shows.reset();
		changed = true;
	}

	const std::span<const ChatFilter> filters = ChatFilters();
	for (size_t start = 0; start < filters.size(); )
	{
		const char* group = filters[start].group;
		size_t end = start;
		int shown = 0;
		while (end < filters.size() && std::string_view(filters[end].group) == group)
			shown += tab.shows.test(end++) ? 1 : 0;

		char label[64];
		std::snprintf(label, sizeof(label), "%s (%d/%d)###%s", group, shown, static_cast<int>(end - start), group);
		if (ImGui::BeginMenu(label))
		{
			if (ImGui::SmallButton("all"))
			{
				for (size_t i = start; i < end; ++i)
					tab.shows.set(i);
				changed = true;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("none"))
			{
				for (size_t i = start; i < end; ++i)
					tab.shows.reset(i);
				changed = true;
			}
			for (size_t i = start; i < end; ++i)
			{
				bool on = tab.shows.test(i);
				if (ImGui::Checkbox(filters[i].label, &on))
				{
					tab.shows.set(i, on);
					changed = true;
				}
			}
			ImGui::EndMenu();
		}
		start = end;
	}
	if (changed)
		SaveTabs();
	ImGui::Separator();
	ImGui::BeginDisabled(s_tabs.size() <= 1);
	if (ImGui::MenuItem("delete tab"))
	{
		s_tabs.erase(s_tabs.begin() + static_cast<std::ptrdiff_t>(index));
		SaveTabs();
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndDisabled();
}

int InputCallback(ImGuiInputTextCallbackData* data)
{
	switch (data->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackAlways:
		if (s_cursorToEnd)
		{
			data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen;
			s_cursorToEnd = false;
		}
		break;
	case ImGuiInputTextFlags_CallbackEdit:
		if (data->BufTextLen > 0 && data->Buf[0] != '/')
			data->InsertChars(0, "/");
		break;
	case ImGuiInputTextFlags_CallbackHistory:
		if (s_history.empty())
			break;
		if (data->EventKey == ImGuiKey_UpArrow)
			s_historyPos = s_historyPos < 0 ? static_cast<int>(s_history.size()) - 1 : std::max(0, s_historyPos - 1);
		else if (data->EventKey == ImGuiKey_DownArrow && s_historyPos >= 0)
			s_historyPos = s_historyPos + 1 < static_cast<int>(s_history.size()) ? s_historyPos + 1 : -1;
		data->DeleteChars(0, data->BufTextLen);
		if (s_historyPos >= 0)
			data->InsertChars(0, s_history[s_historyPos].c_str());
		break;
	default:
		break;
	}
	return 0;
}

void Send()
{
	std::string text = s_input;
	s_input[0] = 0;
	s_historyPos = -1;
	while (!text.empty() && text.back() == ' ')
		text.pop_back();
	if (text.empty() || text[0] != '/')
		return;
	if (s_history.empty() || s_history.back() != text)
		s_history.push_back(text);
	if (s_history.size() > 40)
		s_history.erase(s_history.begin());

	DoCommand(text.c_str());
}

} // namespace

void DrawChat(SkinnedWindow& w)
{
	if (s_tabsSerial != ProfileSerial())
	{
		s_tabsSerial = ProfileSerial();
		LoadTabs();
	}
	DrainChat(s_lines, kKeepLines);
	TakeEqInput();

	const CXRect r = w.wnd->GetScreenRect();
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(r.left), static_cast<float>(r.top)), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(520.0f, 260.0f), ImGuiCond_FirstUseEver);
	PlaceWindow(kId, true);
	if (!ImGui::Begin(kId, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		ImGui::End();
		return;
	}
	NoteWindow(kId);

	static size_t s_active = 0;
	if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_Reorderable))
	{
		for (size_t i = 0; i < s_tabs.size(); ++i)
		{
			ImGui::PushID(static_cast<int>(i));
			if (ImGui::BeginTabItem(s_tabs[i].name.c_str()))
			{
				s_active = i;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginPopupContextItem("##tab"))
			{
				TabMenu(i);
				ImGui::EndPopup();
			}
			ImGui::PopID();
		}
		if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip))
		{
			s_tabs.push_back({ "New", AllChatFilters() });
			SaveTabs();
		}
		ImGui::EndTabBar();
	}
	if (s_active >= s_tabs.size())
		s_active = 0;
	const ChatFilterSet shows = s_tabs.empty() ? AllChatFilters() : s_tabs[s_active].shows;

	const float inputHeight = ImGui::GetFrameHeightWithSpacing();
	if (ImGui::BeginChild("##log", ImVec2(0.0f, -inputHeight)))
	{
		const bool atBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 4.0f;

		int first = static_cast<int>(s_lines.size());
		for (int shown = 0; first > 0 && shown < kShowLines; )
			if (shows.test(s_lines[--first].filter))
				++shown;
		ImGui::PushTextWrapPos(0.0f);
		for (size_t i = static_cast<size_t>(first); i < s_lines.size(); ++i)
			if (shows.test(s_lines[i].filter))
				DrawLine(s_lines[i]);
		ImGui::PopTextWrapPos();

		if (atBottom)
			ImGui::SetScrollHereY(1.0f);
	}
	ImGui::EndChild();

	ImGui::SetNextItemWidth(-1.0f);
	if (s_focusInput)
	{
		ImGui::SetKeyboardFocusHere();
		s_focusInput = false;
	}
	if (ImGui::InputTextWithHint("##input", "/command", s_input, sizeof(s_input),
		ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory
		| ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackEdit, InputCallback))
		Send();

	ImGui::End();
}

} // namespace eqgfx::ui
