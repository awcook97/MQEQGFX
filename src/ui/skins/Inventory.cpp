#include "../Actions.h"
#include "../ClassProfile.h"
#include "../Controls.h"
#include "../Skins.h"
#include "../Widgets.h"

#include <cstdio>
#include <iterator>

namespace eqgfx::ui {

namespace {

constexpr const char* kInventory = "Inventory";

void Command(const char* fmt, const char* a, const char* b = "")
{
	char line[MAX_STRING];
	std::snprintf(line, sizeof(line), fmt, a, b);
	DoCommand(line);
}

void ItemCell(const ItemPtr& item, float size, const char* empty, ImVec2& mn, ImVec2& mx, bool& hovered,
	bool& tap, Press& right)
{
	const Palette& p = Colors();
	mn = ImGui::GetCursorScreenPos();
	mx = ImVec2(mn.x + size, mn.y + size);
	ImGui::InvisibleButton("##slot", ImVec2(size, size), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	hovered = ImGui::IsItemHovered();
	tap = LeftTap();
	right = RightPress();

	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->AddRectFilled(mn, mx, hovered ? p.buttonHover : p.button, p.rounding);
	if (item)
	{
		Icon(dl, ItemIconSheet(), item->GetIconID() - 500, ImVec2(mn.x + 2.0f, mn.y + 2.0f), ImVec2(mx.x - 2.0f, mx.y - 2.0f));
		if (item->GetItemCount() > 1)
		{
			char count[16];
			std::snprintf(count, sizeof(count), "%d", item->GetItemCount());
			const ImVec2 ts = ImGui::CalcTextSize(count);
			IconText(dl, ImVec2(mx.x - ts.x * 0.5f - 2.0f, mx.y - ts.y * 0.5f - 1.0f), count, p.text);
		}
		if (hovered && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
			ImGui::SetTooltip("%s\nright-click and hold to inspect", item->GetName());
	}
	else if (empty)
	{
		dl->PushClipRect(mn, mx, true);
		IconText(dl, ImVec2((mn.x + mx.x) * 0.5f, (mn.y + mx.y) * 0.5f), empty, p.textDim);
		dl->PopClipRect();
	}
	dl->AddRect(mn, mx, p.buttonEdge, p.rounding);
}

void SlotCell(int slot, float size)
{
	ImGui::PushID(slot);
	ItemPtr item = pLocalPC->GetInventorySlot(slot);
	ImVec2 mn, mx;
	bool hovered, tap;
	Press right;
	ItemCell(item, size, szItemSlot[slot], mn, mx, hovered, tap, right);
	if (tap)
		Command("/itemnotify %s leftmouseup", szItemSlot[slot]);
	if (right == Press::Tap)
		Command("/itemnotify %s rightmouseup", szItemSlot[slot]);
	else if (right == Press::Long && item)
		InspectItem(slot);
	ImGui::PopID();
}

void SlotRange(int first, int last, float size, int perRow)
{
	int column = 0;
	for (int slot = first; slot <= last; ++slot)
	{
		SlotCell(slot, size);
		if (++column < perRow && slot < last)
			ImGui::SameLine();
		else
			column = 0;
	}
}

void AutoInventory(float size)
{
	ImGui::PushID("auto");
	ItemPtr held = pLocalPC->GetInventorySlot(InvSlot_Cursor);
	ImVec2 mn, mx;
	bool hovered, tap;
	Press right;
	ItemCell(held, size, "auto", mn, mx, hovered, tap, right);
	if (tap && held)
		DoCommand("/autoinventory");
	if (hovered && !held)
		ImGui::SetTooltip("drop an item here to put it away");
	ImGui::PopID();
}

void Coins()
{
	struct Coin
	{
		const char* name;
		int64_t     amount;
		const char* button;
	};
	const Coin coins[] = {
		{ "pp", pLocalPC->GetPlatinum(), "IW_Money0" },
		{ "gp", pLocalPC->GetGold(), "IW_Money1" },
		{ "sp", pLocalPC->GetSilver(), "IW_Money2" },
		{ "cp", pLocalPC->GetCopper(), "IW_Money3" },
	};
	for (const Coin& c : coins)
	{
		char label[40];
		std::snprintf(label, sizeof(label), "%lld %s", static_cast<long long>(c.amount), c.name);
		if (ImGui::SmallButton(label) && c.amount > 0)
			Command("/notify InventoryWindow %s leftmouseup", c.button);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("take %s out", c.name);
		ImGui::SameLine();
	}
	ImGui::NewLine();
}

void ItemsTab()
{
	float size = ClassValue(kInventory, "Size", 40.0f, 24.0f, 72.0f);
	int columns = ClassInt(kInventory, "Columns", 6, 2, 12);

	ImGui::SeparatorText("worn");
	SlotRange(InvSlot_FirstWornItem, InvSlot_LastWornItem, size, columns);
	ImGui::SeparatorText("bags");
	SlotRange(InvSlot_FirstBagSlot, InvSlot_LastBonusBagSlot, size, columns);
	ImGui::SameLine();
	AutoInventory(size);
	ImGui::SeparatorText("coin");
	Coins();

	if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("##invoptions");
	if (ImGui::BeginPopup("##invoptions"))
	{
		ImGui::SetNextItemWidth(140.0f);
		if (ImGui::SliderFloat("slot size", &size, 24.0f, 72.0f, "%.0f px"))
			SetClassValue(kInventory, "Size", size);
		ImGui::SetNextItemWidth(140.0f);
		if (ImGui::SliderInt("columns", &columns, 2, 12))
			SetClassValue(kInventory, "Columns", static_cast<float>(columns));
		ImGui::EndPopup();
	}
}

struct Stat
{
	const char* name;
	int         current;
	int         max;
	int         heroic;
};

constexpr Stat kVitals[] = {
	{ "Hit points", 17, 18, 0 }, { "Mana", 124, 125, 0 }, { "Endurance", 126, 127, 0 },
	{ "Mitigation", 22, 0, 0 }, { "Evasion", 403, 0, 0 }, { "Offense", 23, 0, 0 },
	{ "Accuracy", 402, 0, 0 }, { "Haste", 211, 0, 0 }, { "Movement", 401, 0, 0 },
	{ "HP regen", 212, 0, 0 }, { "Mana regen", 213, 0, 0 }, { "End regen", 214, 0, 0 },
	{ "Weight", 24, 25, 0 },
};

constexpr Stat kAttributes[] = {
	{ "Strength", 238, 264, 251 }, { "Stamina", 239, 265, 252 }, { "Agility", 241, 267, 254 },
	{ "Dexterity", 240, 266, 253 }, { "Wisdom", 242, 268, 255 }, { "Intelligence", 243, 269, 256 },
	{ "Charisma", 244, 270, 257 },
};

constexpr Stat kResists[] = {
	{ "Magic", 249, 275, 0 }, { "Fire", 247, 273, 0 }, { "Cold", 248, 274, 0 },
	{ "Disease", 246, 272, 0 }, { "Poison", 245, 271, 0 }, { "Corruption", 250, 276, 0 },
};

constexpr Stat kBonuses[] = {
	{ "Combat effects", 216, 278, 0 }, { "Spell shielding", 215, 277, 0 }, { "Shielding", 217, 279, 0 },
	{ "Damage shielding", 218, 280, 0 }, { "DoT shielding", 219, 281, 0 },
	{ "DS mitigation", 220, 282, 0 }, { "Avoidance", 221, 283, 0 }, { "Accuracy", 222, 284, 0 },
	{ "Stun resist", 223, 285, 0 }, { "Strikethrough", 224, 286, 0 }, { "Heal amount", 225, 0, 0 },
	{ "Spell damage", 226, 0, 0 }, { "Clairvoyance", 227, 0, 0 }, { "Luck", 416, 0, 0 },
};

void StatTable(const char* id, const Stat* stats, size_t count)
{
	if (!ImGui::BeginTable(id, 4, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg))
		return;
	for (size_t i = 0; i < count; ++i)
	{
		const Stat& s = stats[i];
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(s.name);
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(LabelText(s.current).c_str());
		ImGui::TableNextColumn();
		if (s.max)
			ImGui::TextDisabled("/ %s", LabelText(s.max).c_str());
		ImGui::TableNextColumn();
		if (s.heroic)
			ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.35f, 1.0f), "+%s", LabelText(s.heroic).c_str());
	}
	ImGui::EndTable();
}

void StatsTab()
{
	ImGui::Text("%s  %s  %s", LabelText(1).c_str(), LabelText(2).c_str(), LabelText(3).c_str());
	ImGui::TextDisabled("%s", LabelText(4).c_str());
	ImGui::SeparatorText("vitals");
	StatTable("##vitals", kVitals, std::size(kVitals));
	ImGui::SeparatorText("attributes");
	StatTable("##attributes", kAttributes, std::size(kAttributes));
	ImGui::SeparatorText("resists");
	StatTable("##resists", kResists, std::size(kResists));
	ImGui::SeparatorText("item bonuses");
	StatTable("##bonuses", kBonuses, std::size(kBonuses));
}

void CurrencyRow(int id, const char* filter)
{
	const char* name = pDBStr ? pDBStr->GetString(id, eAltCurrencyName) : nullptr;
	if (!name || !*name)
		return;
	const int amount = pPlayerPointManager ? pPlayerPointManager->GetAltCurrency(id) : 0;
	if (amount <= 0 || (filter[0] && ci_find_substr(name, filter) == -1))
		return;
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::TextUnformatted(name);
	ImGui::TableNextColumn();
	ImGui::Text("%d", amount);
}

void CurrencyTab()
{
	static char s_filter[64] = {};
	ImGui::SetNextItemWidth(-1.0f);
	ImGui::InputTextWithHint("##filter", "search", s_filter, sizeof(s_filter));
	if (!ImGui::BeginTable("##currency", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg))
		return;
	for (int id = ALTCURRENCY_FIRST; id <= ALTCURRENCY_LAST; ++id)
		CurrencyRow(id, s_filter);
	CurrencyRow(ALTCURRENCY_CROWNS, s_filter);
	ImGui::EndTable();
}

} // namespace

void DrawInventory(SkinnedWindow& w)
{
	if (!w.wnd->IsVisible() || !pLocalPC || ProfileClass().empty())
		return;

	constexpr const char* kId = "Inventory##eqgfx";
	const CXRect r = w.wnd->GetScreenRect();
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(r.left), static_cast<float>(r.top)), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(320.0f, 460.0f), ImGuiCond_FirstUseEver);
	PlaceWindow(kId, true);

	bool open = true;
	if (ImGui::Begin(kId, &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
	{
		NoteWindow(kId);
		if (ImGui::BeginTabBar("##inventory"))
		{
			if (ImGui::BeginTabItem("Items"))
			{
				ItemsTab();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Stats"))
			{
				StatsTab();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Currency"))
			{
				CurrencyTab();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	if (!open)
		w.wnd->Show(false);
}

} // namespace eqgfx::ui
