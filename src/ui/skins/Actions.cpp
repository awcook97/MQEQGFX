#include "../HotButtons.h"
#include "../ClassProfile.h"
#include "../Skins.h"
#include "../Widgets.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>

namespace eqgfx::ui {

namespace {

HotButton Make(const char* label, std::string cmd)
{
	HotButton b;
	b.label = label;
	b.cmd = std::move(cmd);
	return b;
}

HotButton DiscButton(const EQ_Spell* spell)
{
	HotButton b = Make(spell->Name, std::string("/disc ") + spell->Name);
	b.iconType = ButtonIcon::Spell;
	b.icon = spell->SpellIcon;
	b.timer = ButtonTimer::Disc;
	b.timerKey = spell->Name;
	return b;
}

bool BeginOurWindow(SkinnedWindow& w, const char* id, const ImVec2& size, bool& open)
{
	const CXRect r = w.wnd->GetScreenRect();
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(r.left), static_cast<float>(r.top)), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
	PlaceWindow(id, true);
	const bool shown = ImGui::Begin(id, &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
	if (shown)
		NoteWindow(id);
	return shown;
}

void EndOurWindow(SkinnedWindow& w, bool open)
{
	ImGui::End();
	if (!open)
		w.wnd->Show(false);
}

bool BackgroundMenu(const char* id)
{
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && !ImGui::IsAnyItemHovered()
		&& ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup(id);
	return ImGui::BeginPopup(id);
}

void DrawButtonFace(const HotButton& b, bool labels)
{
	const Palette& p = Colors();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const ImVec2 mn = ImGui::GetItemRectMin();
	const ImVec2 mx = ImGui::GetItemRectMax();
	dl->AddRectFilled(mn, mx, ImGui::IsItemHovered() ? p.buttonHover : p.button, p.rounding);
	if (!b.Empty())
	{
		const ImVec2 imn(mn.x + 2.0f, mn.y + 2.0f);
		const ImVec2 imx(mx.x - 2.0f, mx.y - 2.0f);
		if (b.iconType == ButtonIcon::Spell)
			Icon(dl, SpellIconSheet(), b.icon, imn, imx);
		else if (b.iconType == ButtonIcon::Item)
			Icon(dl, ItemIconSheet(), b.icon, imn, imx);
		uint32_t total = 0;
		const uint32_t left = ButtonCooldown(b, total);
		DrawCooldownOverlay(dl, imn, imx, left, total);
		if (labels || b.iconType == ButtonIcon::None)
		{
			dl->PushClipRect(mn, mx, true);
			const ImVec2 ts = ImGui::CalcTextSize(b.label.c_str());
			const float y = b.iconType == ButtonIcon::None ? (mn.y + mx.y) * 0.5f : mx.y - ts.y * 0.5f - 1.0f;
			IconText(dl, ImVec2((mn.x + mx.x) * 0.5f, y), b.label.c_str(), p.text);
			dl->PopClipRect();
		}
	}
	dl->AddRect(mn, mx, p.buttonEdge, p.rounding);
}

constexpr const char* kActions = "Actions";

struct ActionTab
{
	const char* name;
	const char* set;
};

constexpr ActionTab kActionTabs[] = {
	{ "Main", "Actions.Main" }, { "Abilities", "Actions.Abilities" },
	{ "Combat", "Actions.Combat" }, { "Socials", "Actions.Socials" },
};

void SeedSkills(std::vector<HotButton>& out, const int* skills, int count)
{
	for (int i = 0; i < count; ++i)
	{
		const int skill = skills[i];
		const char* name = skill >= 0 && skill < NUM_SKILLS ? GetSkillName(skill) : nullptr;
		if (name && *name)
			out.push_back(Make(name, std::string("/doability \"") + name + "\""));
	}
}

void Seed(size_t tab, std::vector<HotButton>& out)
{
	if (!pEverQuestInfo)
		return;
	switch (tab)
	{
	case 0:
	{
		constexpr const char* kMain[][2] = {
			{ "Attack", "/attack" }, { "Sit", "/sit" }, { "Stand", "/stand" }, { "Who", "/who" },
			{ "Invite", "/invite" }, { "Disband", "/disband" }, { "Follow", "/follow" }, { "Assist", "/assist" },
			{ "Consider", "/consider" }, { "Hail", "/say Hail, %T" }, { "Loot", "/loot" }, { "Inspect", "/inspect" },
			{ "Camp", "/camp" },
		};
		for (const auto& c : kMain)
			out.push_back(Make(c[0], c[1]));
		break;
	}
	case 1:
		SeedSkills(out, pEverQuestInfo->abilities, static_cast<int>(std::size(pEverQuestInfo->abilities)));
		break;
	case 2:
		SeedSkills(out, pEverQuestInfo->combatSkill, static_cast<int>(std::size(pEverQuestInfo->combatSkill)));
		break;
	case 3:
		for (const auto& page : pEverQuestInfo->socials)
			for (const EQSocial& social : page)
			{
				if (!social.Name[0])
					continue;
				std::string cmd;
				for (const char* line : social.Line)
					if (line[0])
					{
						if (!cmd.empty())
							cmd += '\n';
						cmd += line;
					}
				out.push_back(Make(social.Name, std::move(cmd)));
			}
		break;
	}
}

void ActionsOptions()
{
	int columns = ClassInt(kActions, "Columns", 3, 1, 12);
	ImGui::SetNextItemWidth(140.0f);
	if (ImGui::SliderInt("columns", &columns, 1, 12))
		SetClassValue(kActions, "Columns", static_cast<float>(columns));
	float height = ClassValue(kActions, "Height", 0.0f, 0.0f, 80.0f);
	ImGui::SetNextItemWidth(140.0f);
	if (ImGui::SliderFloat("button height", &height, 0.0f, 80.0f, height > 0.0f ? "%.0f px" : "text"))
		SetClassValue(kActions, "Height", height);
	bool icons = ClassBool(kActions, "Icons", true);
	if (ImGui::Checkbox("icons", &icons))
		SetClassValue(kActions, "Icons", icons ? 1.0f : 0.0f);
}

void ActionsTab(size_t tab)
{
	const std::string set = kActionTabs[tab].set;
	bool stored = false;
	std::vector<HotButton>& buttons = ButtonSet(set, &stored);
	if (!stored && buttons.empty())
	{
		Seed(tab, buttons);
		SaveButtonSet(set);
	}

	const int columns = ClassInt(kActions, "Columns", 3, 1, 12);
	const float height = ClassValue(kActions, "Height", 0.0f, 0.0f, 80.0f);
	const bool icons = ClassBool(kActions, "Icons", true);
	const float spacing = ImGui::GetStyle().ItemSpacing.x;
	const float width = (ImGui::GetContentRegionAvail().x - spacing * (columns - 1)) / columns;
	const ImVec2 size(std::max(width, 8.0f), height > 0.0f ? height : ImGui::GetFrameHeight());

	int remove = -1;
	for (size_t i = 0; i < buttons.size(); ++i)
	{
		HotButton& b = buttons[i];
		ImGui::PushID(static_cast<int>(i));
		if (icons && height >= 24.0f && b.iconType != ButtonIcon::None)
		{
			ImGui::InvisibleButton("##b", size);
			DrawButtonFace(b, true);
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				RunButton(b);
		}
		else if (ImGui::Button(b.label.c_str(), size))
			RunButton(b);
		DragButtonSource(b);
		if (ImGui::BeginPopupContextItem("##edit"))
		{
			if (ImGui::MenuItem("Edit"))
			{
				const size_t at = i;
				EditButton(std::string(kActionTabs[tab].name) + " tab", b,
					[set, at](const HotButton& edited) {
						std::vector<HotButton>& list = ButtonSet(set);
						if (at < list.size())
							list[at] = edited;
						SaveButtonSet(set);
					});
			}
			if (ImGui::MenuItem("Move left", nullptr, false, i > 0))
			{
				std::swap(buttons[i], buttons[i - 1]);
				SaveButtonSet(set);
			}
			if (ImGui::MenuItem("Move right", nullptr, false, i + 1 < buttons.size()))
			{
				std::swap(buttons[i], buttons[i + 1]);
				SaveButtonSet(set);
			}
			if (ImGui::MenuItem("Remove"))
				remove = static_cast<int>(i);
			ImGui::EndPopup();
		}
		ImGui::PopID();
		if ((i + 1) % columns != 0)
			ImGui::SameLine();
	}

	if (ImGui::Button("+", size))
		EditButton(std::string("new, ") + kActionTabs[tab].name + " tab", HotButton{},
			[set](const HotButton& made) {
				ButtonSet(set).push_back(made);
				SaveButtonSet(set);
			});
	if (ImGui::BeginDragDropTarget())
	{
		HotButton dropped;
		if (AcceptDraggedButton(dropped))
		{
			buttons.push_back(dropped);
			SaveButtonSet(set);
		}
		ImGui::EndDragDropTarget();
	}

	if (remove >= 0)
	{
		buttons.erase(buttons.begin() + remove);
		SaveButtonSet(set);
	}
}

constexpr const char* kCombat = "CombatAbilities";

enum Sort { Sort_Name, Sort_Level, Sort_Reuse, Sort_Category, Sort_Count };
constexpr const char* kSorts[] = { "name", "level", "time to ready", "category" };

struct Disc
{
	EQ_Spell*   spell;
	int         level;
	uint32_t    left;
	const char* category;
};

const char* CategoryOf(const EQ_Spell* spell)
{
	const char* name = spell->Subcategory > 0 && pDBStr ? pDBStr->GetString(spell->Subcategory, eSpellCategory) : nullptr;
	return name && *name ? name : "Other";
}

EQ_Spell* ActiveDisc()
{
	if (!pCombatAbilityWnd)
		return nullptr;
	CXWnd* label = pCombatAbilityWnd->GetChildItem("CAW_CombatEffectLabel");
	if (!label)
		return nullptr;
	const CXStr name = label->GetWindowText();
	return name.empty() ? nullptr : GetSpellByName(name.c_str());
}

std::vector<Disc> Discs(const char* filter, bool readyOnly)
{
	std::vector<Disc> out;
	if (!pLocalPC)
		return out;
	const int cls = pLocalPlayer ? pLocalPlayer->GetClass() : 0;
	for (int i = 0; i < NUM_COMBAT_ABILITIES; ++i)
	{
		if (pCombatSkillsSelectWnd && !pCombatSkillsSelectWnd->ShouldDisplayThisSkill(i))
			continue;
		EQ_Spell* spell = GetSpellByID(pLocalPC->GetCombatAbility(i));
		if (!spell || (filter[0] && ci_find_substr(spell->Name, filter) == -1))
			continue;
		const uint32_t left = DiscCooldown(spell);
		if (readyOnly && left)
			continue;
		const int level = cls > 0 && cls <= MAX_CLASSES ? spell->ClassLevel[cls] : 0;
		out.push_back({ spell, level, left, CategoryOf(spell) });
	}

	const int sort = ClassInt(kCombat, "Sort", Sort_Level, 0, Sort_Count - 1);
	const bool descending = ClassBool(kCombat, "Descending", sort == Sort_Level);
	std::stable_sort(out.begin(), out.end(), [sort](const Disc& a, const Disc& b) {
		switch (sort)
		{
		case Sort_Level:    return a.level < b.level;
		case Sort_Reuse:    return a.left < b.left;
		case Sort_Category: return std::strcmp(a.category, b.category) < 0;
		default:            return std::strcmp(a.spell->Name, b.spell->Name) < 0;
		}
	});
	if (descending)
		std::reverse(out.begin(), out.end());
	return out;
}

void CombatOptions()
{
	int rows = ClassInt(kCombat, "Rows", 2, 1, 12);
	int cols = ClassInt(kCombat, "Cols", 4, 1, 12);
	int pages = ClassInt(kCombat, "Pages", 1, 1, 10);
	float size = ClassValue(kCombat, "Size", 40.0f, 20.0f, 96.0f);
	ImGui::SeparatorText("panels");
	ImGui::SetNextItemWidth(140.0f);
	if (ImGui::SliderInt("rows", &rows, 1, 12))
		SetClassValue(kCombat, "Rows", static_cast<float>(rows));
	ImGui::SetNextItemWidth(140.0f);
	if (ImGui::SliderInt("columns", &cols, 1, 12))
		SetClassValue(kCombat, "Cols", static_cast<float>(cols));
	ImGui::SetNextItemWidth(140.0f);
	if (ImGui::SliderInt("pages", &pages, 1, 10))
		SetClassValue(kCombat, "Pages", static_cast<float>(pages));
	ImGui::SetNextItemWidth(140.0f);
	if (ImGui::SliderFloat("button size", &size, 20.0f, 96.0f, "%.0f px"))
		SetClassValue(kCombat, "Size", size);
	bool labels = ClassBool(kCombat, "Labels", true);
	if (ImGui::Checkbox("labels", &labels))
		SetClassValue(kCombat, "Labels", labels ? 1.0f : 0.0f);

	ImGui::SeparatorText("list");
	bool showList = ClassBool(kCombat, "List", true);
	if (ImGui::Checkbox("show the list", &showList))
		SetClassValue(kCombat, "List", showList ? 1.0f : 0.0f);
	int sort = ClassInt(kCombat, "Sort", Sort_Level, 0, Sort_Count - 1);
	ImGui::SetNextItemWidth(140.0f);
	if (ImGui::Combo("sort by", &sort, kSorts, Sort_Count))
		SetClassValue(kCombat, "Sort", static_cast<float>(sort));
	bool descending = ClassBool(kCombat, "Descending", sort == Sort_Level);
	if (ImGui::Checkbox("descending", &descending))
		SetClassValue(kCombat, "Descending", descending ? 1.0f : 0.0f);
	bool readyOnly = ClassBool(kCombat, "ReadyOnly", false);
	if (ImGui::Checkbox("ready only", &readyOnly))
		SetClassValue(kCombat, "ReadyOnly", readyOnly ? 1.0f : 0.0f);
}

void Panels()
{
	const int rows = ClassInt(kCombat, "Rows", 2, 1, 12);
	const int cols = ClassInt(kCombat, "Cols", 4, 1, 12);
	const int pages = ClassInt(kCombat, "Pages", 1, 1, 10);
	const float size = ClassValue(kCombat, "Size", 40.0f, 20.0f, 96.0f);
	const bool labels = ClassBool(kCombat, "Labels", true);
	const size_t perPage = static_cast<size_t>(rows * cols);

	static int s_page = 0;
	s_page = std::clamp(s_page, 0, pages - 1);

	const std::string set = "Discs";
	std::vector<HotButton>& slots = ButtonSet(set);
	if (slots.size() < perPage * pages)
		slots.resize(perPage * pages);

	if (pages > 1)
	{
		if (ImGui::ArrowButton("##prev", ImGuiDir_Left))
			s_page = (s_page + pages - 1) % pages;
		ImGui::SameLine();
		ImGui::Text("%d / %d", s_page + 1, pages);
		ImGui::SameLine();
		if (ImGui::ArrowButton("##next", ImGuiDir_Right))
			s_page = (s_page + 1) % pages;
	}

	for (size_t k = 0; k < perPage; ++k)
	{
		const size_t at = static_cast<size_t>(s_page) * perPage + k;
		HotButton& b = slots[at];
		ImGui::PushID(static_cast<int>(at));
		ImGui::InvisibleButton("##slot", ImVec2(size, size));
		DrawButtonFace(b, labels);
		if (LeftTap() && !b.Empty())
			RunButton(b);
		if (!b.Empty())
		{
			DragButtonSource(b);
			if (ImGui::IsItemHovered() && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
				if (const EQ_Spell* spell = GetSpellByName(b.timerKey))
					SpellTooltip(spell, static_cast<int>(DiscCooldown(spell)), false);
		}
		if (ImGui::BeginDragDropTarget())
		{
			HotButton dropped;
			if (AcceptDraggedButton(dropped))
			{
				b = dropped;
				SaveButtonSet(set);
			}
			ImGui::EndDragDropTarget();
		}
		if (ImGui::BeginPopupContextItem("##slotmenu"))
		{
			if (ImGui::MenuItem("Edit"))
				EditButton("combat ability panel", b,
					[set, at](const HotButton& edited) {
						ButtonSet(set)[at] = edited;
						SaveButtonSet(set);
					},
					[set, at]() {
						ButtonSet(set)[at] = HotButton{};
						SaveButtonSet(set);
					});
			if (ImGui::MenuItem("Clear", nullptr, false, !b.Empty()))
			{
				b = HotButton{};
				SaveButtonSet(set);
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
		if ((k + 1) % static_cast<size_t>(cols) != 0)
			ImGui::SameLine();
	}
}

void DiscList()
{
	static char s_filter[64] = {};
	ImGui::SetNextItemWidth(-1.0f);
	ImGui::InputTextWithHint("##filter", "search", s_filter, sizeof(s_filter));

	const float iconSize = ImGui::GetTextLineHeight() + 6.0f;
	if (!ImGui::BeginChild("##discs"))
	{
		ImGui::EndChild();
		return;
	}
	const bool byCategory = ClassInt(kCombat, "Sort", Sort_Level, 0, Sort_Count - 1) == Sort_Category;
	const char* lastCategory = nullptr;
	int row = 0;
	for (const Disc& d : Discs(s_filter, ClassBool(kCombat, "ReadyOnly", false)))
	{
		if (byCategory && (!lastCategory || std::strcmp(lastCategory, d.category) != 0))
		{
			ImGui::SeparatorText(d.category);
			lastCategory = d.category;
		}
		const HotButton b = DiscButton(d.spell);
		ImGui::PushID(row++);
		const ImVec2 at = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos(ImVec2(at.x + iconSize + 4.0f, at.y));
		char text[160];
		if (d.left)
		{
			char when[16];
			FormatCooldown(d.left, when, sizeof(when));
			std::snprintf(text, sizeof(text), "%3d  %s  (%s)", d.level, d.spell->Name, when);
		}
		else
			std::snprintf(text, sizeof(text), "%3d  %s", d.level, d.spell->Name);
		if (ImGui::Selectable(text, false, 0, ImVec2(0.0f, iconSize)))
			RunButton(b);
		DragButtonSource(b);
		if (ImGui::IsItemHovered())
			SpellTooltip(d.spell, static_cast<int>(d.left), false);
		Icon(ImGui::GetWindowDrawList(), SpellIconSheet(), d.spell->SpellIcon, at,
			ImVec2(at.x + iconSize, at.y + iconSize), d.left ? 150 : 255);
		ImGui::PopID();
	}
	ImGui::EndChild();
}

} // namespace

void DrawActions(SkinnedWindow& w)
{
	if (!w.wnd->IsVisible() || !pLocalPlayer || ProfileClass().empty())
		return;

	bool open = true;
	if (BeginOurWindow(w, "Actions##eqgfx", ImVec2(320.0f, 240.0f), open))
	{
		if (ImGui::BeginTabBar("##actions"))
		{
			for (size_t t = 0; t < std::size(kActionTabs); ++t)
				if (ImGui::BeginTabItem(kActionTabs[t].name))
				{
					ActionsTab(t);
					ImGui::EndTabItem();
				}
			ImGui::EndTabBar();
		}
		if (BackgroundMenu("##actionsoptions"))
		{
			ActionsOptions();
			ImGui::EndPopup();
		}
	}
	EndOurWindow(w, open);
}

void DrawCombatAbilities(SkinnedWindow& w)
{
	if (!w.wnd->IsVisible() || !pLocalPC || ProfileClass().empty())
		return;

	bool open = true;
	if (BeginOurWindow(w, "Combat Abilities##eqgfx", ImVec2(320.0f, 420.0f), open))
	{
		if (EQ_Spell* active = ActiveDisc())
		{
			const float s = ImGui::GetTextLineHeight() + 6.0f;
			const ImVec2 at = ImGui::GetCursorScreenPos();
			Icon(ImGui::GetWindowDrawList(), SpellIconSheet(), active->SpellIcon, at, ImVec2(at.x + s, at.y + s));
			ImGui::Dummy(ImVec2(s, s));
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.35f, 1.0f), "active: %s", active->Name);
			if (ImGui::IsItemHovered())
				SpellTooltip(active, -1, true);
		}
		else
			ImGui::TextDisabled("no discipline active");
		ImGui::SameLine();
		if (ImGui::SmallButton("stop discipline"))
			DoCommand("/stopdisc");
		ImGui::Separator();

		Panels();
		if (ClassBool(kCombat, "List", true))
		{
			ImGui::Separator();
			DiscList();
		}

		if (BackgroundMenu("##combatoptions"))
		{
			CombatOptions();
			ImGui::EndPopup();
		}
	}
	EndOurWindow(w, open);
}

} // namespace eqgfx::ui
