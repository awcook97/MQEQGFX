#include "../Actions.h"
#include "../ClassProfile.h"
#include "../HotButtons.h"
#include "../Skins.h"
#include "../Widgets.h"

#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <iterator>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace eqgfx::ui {

namespace {

void Command(const char* fmt, ...)
{
	char line[MAX_STRING];
	va_list args;
	va_start(args, fmt);
	std::vsnprintf(line, sizeof(line), fmt, args);
	va_end(args);
	DoCommand(line);
}

bool BeginOurWindow(const CXWnd* native, const char* id, const ImVec2& size, bool& open,
	ImGuiWindowFlags flags = 0)
{
	const CXRect r = native->GetScreenRect();
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(r.left), static_cast<float>(r.top)), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
	PlaceWindow(id, true);
	const bool shown = ImGui::Begin(id, &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | flags);
	if (shown)
		NoteWindow(id);
	return shown;
}

void ItemIcon(int iconId, float size)
{
	const ImVec2 at = ImGui::GetCursorScreenPos();
	Icon(ImGui::GetWindowDrawList(), ItemIconSheet(), iconId - 500, at, ImVec2(at.x + size, at.y + size));
	ImGui::Dummy(ImVec2(size, size));
}

struct AAEntry
{
	int         group;
	int         index;
	const char* name;
	int         type;
	int         rank;
	int         maxRank;
	int         nextIndex;
	int         nextCost;
	int         nextLevel;
	bool        canTrain;
	bool        activatable;
};

std::vector<AAEntry> s_aas;
uint64_t s_aasBuiltFor = 0;
int      s_aasFrame = -2;

constexpr const char* kAATypes[] = { "General", "Archetype", "Class", "Special", "Focus", "Other" };
constexpr int kOtherType = 6;

void BuildAAs()
{
	s_aas.clear();
	if (!pAltAdvManager || !pAltAdvManager->abilities || !pLocalPC || !pLocalPlayer || !pDBStr)
		return;
	const int cls = pLocalPlayer->GetClass();
	const int level = pLocalPlayer->Level;
	std::unordered_set<int> byGroup;

	auto add = [&](CAltAbilityData* shown, int rank) -> AAEntry* {
		const char* name = pDBStr->GetString(shown->nName, eAltAbilityName);
		if (!name || !*name)
			return nullptr;
		AAEntry e{};
		e.group = shown->GroupID;
		e.index = shown->Index;
		e.name = name;
		e.type = shown->Type >= 1 && shown->Type < kOtherType ? shown->Type : kOtherType;
		e.rank = rank;
		e.maxRank = shown->MaxRank;
		e.activatable = shown->SpellID > 0 || shown->ReuseTimer > 0;
		byGroup.insert(e.group);
		s_aas.push_back(e);
		return &s_aas.back();
	};
	auto setNext = [&](AAEntry& e, CAltAbilityData* next) {
		e.nextIndex = next ? next->Index : -1;
		e.nextCost = next ? next->Cost : 0;
		e.nextLevel = next ? next->MinLevel : 0;
		e.canTrain = next && pAltAdvManager->CanTrainAbility(pLocalPC, next, false, false, false);
	};

	for (int n = 0; n < AA_CHAR_MAX_REAL; ++n)
	{
		const int id = pLocalPC->GetAlternateAbilityId(n);
		if (id < 0)
			continue;
		CAltAbilityData* owned = GetAAById(id, level);
		if (!owned)
			owned = GetAAById(id);
		if (!owned || byGroup.count(owned->GroupID) || !pLocalPC->HasAlternateAbility(owned->Index))
			continue;
		if (AAEntry* e = add(owned, owned->CurrentRank))
			setNext(*e, owned->NextGroupAbilityId >= 0 ? GetAAById(owned->NextGroupAbilityId) : nullptr);
	}

	std::unordered_map<int, CAltAbilityData*> lowest;
	for (CAltAbilityData** it = pAltAdvManager->abilities->WalkFirst(); it; it = pAltAdvManager->abilities->WalkNext(it))
	{
		CAltAbilityData* ab = *it;
		if (!ab || !(ab->Classes & (1 << cls)) || byGroup.count(ab->GroupID))
			continue;
		CAltAbilityData*& low = lowest[ab->GroupID];
		if (!low || ab->CurrentRank < low->CurrentRank)
			low = ab;
	}
	for (const auto& [group, first] : lowest)
	{
		if (CAltAbilityData* owned = pAltAdvManager->GetOwnedAbilityFromGroupID(pLocalPC, group))
		{
			if (AAEntry* e = add(owned, owned->CurrentRank))
				setNext(*e, owned->NextGroupAbilityId >= 0 ? GetAAById(owned->NextGroupAbilityId) : nullptr);
			continue;
		}
		const int rank = first->CurrentRank - 1 + (pLocalPC->HasAlternateAbility(first->Index) ? 1 : 0);
		if (AAEntry* e = add(first, rank))
			setNext(*e, rank > 0 ? (first->NextGroupAbilityId >= 0 ? GetAAById(first->NextGroupAbilityId) : nullptr) : first);
	}

	std::sort(s_aas.begin(), s_aas.end(), [](const AAEntry& a, const AAEntry& b) { return std::strcmp(a.name, b.name) < 0; });
}

HotButton AAButton(const AAEntry& e)
{
	HotButton b;
	b.label = e.name;
	b.cmd = "/alt activate " + std::to_string(e.group);
	b.timer = ButtonTimer::AA;
	b.timerKey = e.name;
	if (CAltAbilityData* ab = GetAAById(e.index))
		if (const EQ_Spell* spell = ab->SpellID > 0 ? GetSpellByID(ab->SpellID) : nullptr)
		{
			b.iconType = ButtonIcon::Spell;
			b.icon = spell->SpellIcon;
		}
	return b;
}

void AddToBarMenu(const HotButton& b)
{
	ImGui::TextDisabled("add to hot bar");
	bool any = false;
	for (const SkinnedWindow& w : SkinnedWindows())
	{
		if (w.skin != Skin::Hotbar || !w.replace || w.bar < 0 || w.bar >= kHotBars || !w.wnd || !w.wnd->IsVisible())
			continue;
		any = true;
		const CHotButtonWnd* wnd = static_cast<const CHotButtonWnd*>(w.wnd);
		const int page = HotBarPage(w.bar);
		int free = -1;
		for (int slot = 0; slot < kHotSlots && free < 0; ++slot)
			if (const CHotButton* hb = wnd->Buttons[slot]; hb && hb->IsVisible() && ButtonAt(w.bar, page, slot).Empty())
				free = slot;
		char item[64];
		if (free >= 0)
			std::snprintf(item, sizeof(item), "bar %d, button %d", w.bar + 1, free + 1);
		else
			std::snprintf(item, sizeof(item), "bar %d (full)", w.bar + 1);
		if (ImGui::MenuItem(item, nullptr, false, free >= 0))
		{
			ButtonAt(w.bar, page, free) = b;
			ButtonChanged(w.bar, page);
		}
	}
	if (!any)
		ImGui::TextDisabled("no hot bars shown");
}

void Bag(CContainerWnd* wnd)
{
	const ItemPtr& bag = wnd->Container;
	const int top = wnd->Location.GetTopSlot();
	if (!bag || top < 0 || top > InvSlot_Max)
		return;
	const char* pack = szItemSlot[top];
	const int slots = bag->GetItemDefinition() ? bag->GetItemDefinition()->Slots : 0;

	char id[96];
	std::snprintf(id, sizeof(id), "%s##bag_%s", bag->GetName(), pack);
	bool open = true;
	if (BeginOurWindow(wnd, id, ImVec2(200.0f, 160.0f), open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		const float size = ClassValue("Bags", "Size", 40.0f, 24.0f, 72.0f);
		const int columns = ClassInt("Bags", "Columns", 5, 1, 12);
		const Palette& p = Colors();
		for (int k = 0; k < slots; ++k)
		{
			ImGui::PushID(k);
			ItemPtr item = bag->GetHeldItem(k);
			const ImVec2 mn = ImGui::GetCursorScreenPos();
			const ImVec2 mx(mn.x + size, mn.y + size);
			ImGui::InvisibleButton("##slot", ImVec2(size, size), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
			const bool hovered = ImGui::IsItemHovered();
			const bool tap = LeftTap();
			const Press right = RightPress();
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
			dl->AddRect(mn, mx, p.buttonEdge, p.rounding);
			if (tap)
				Command("/itemnotify in %s %d leftmouseup", pack, k + 1);
			if (right == Press::Tap)
				Command("/itemnotify in %s %d rightmouseup", pack, k + 1);
			else if (right == Press::Long && item)
				InspectItem(top, k);
			ImGui::PopID();
			if ((k + 1) % columns != 0 && k + 1 < slots)
				ImGui::SameLine();
		}

		if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			ImGui::OpenPopup("##bagoptions");
		if (ImGui::BeginPopup("##bagoptions"))
		{
			float s = size;
			int c = columns;
			ImGui::SetNextItemWidth(140.0f);
			if (ImGui::SliderFloat("slot size", &s, 24.0f, 72.0f, "%.0f px"))
				SetClassValue("Bags", "Size", s);
			ImGui::SetNextItemWidth(140.0f);
			if (ImGui::SliderInt("columns", &c, 1, 12))
				SetClassValue("Bags", "Columns", static_cast<float>(c));
			ImGui::EndPopup();
		}
	}
	ImGui::End();
	if (!open)
		Command("/itemnotify %s rightmouseup", pack);
}

} // namespace

void DrawAltAbilities(SkinnedWindow& w)
{
	if (!w.wnd->IsVisible() || ProfileClass().empty())
		return;
	const PcProfile* profile = GetPcProfile();
	const uint64_t state = profile && pLocalPlayer
		? (static_cast<uint64_t>(profile->AAPoints) << 40) ^ (static_cast<uint64_t>(profile->AAPointsSpent) << 16)
			^ static_cast<uint64_t>(pLocalPlayer->Level)
		: 0;
	const int frame = ImGui::GetFrameCount();
	if (state != s_aasBuiltFor || frame != s_aasFrame + 1)
	{
		BuildAAs();
		s_aasBuiltFor = state;
	}
	s_aasFrame = frame;

	bool open = true;
	if (BeginOurWindow(w.wnd, "Alternate Abilities##eqgfx", ImVec2(460.0f, 480.0f), open))
	{
		if (profile)
			ImGui::Text("points: %d   spent: %d", profile->AAPoints, profile->AAPointsSpent);

		static char s_filter[64] = {};
		static bool s_trainable = false;
		static bool s_owned = false;
		ImGui::SetNextItemWidth(180.0f);
		ImGui::InputTextWithHint("##filter", "search", s_filter, sizeof(s_filter));
		ImGui::SameLine();
		ImGui::Checkbox("can train", &s_trainable);
		ImGui::SameLine();
		ImGui::Checkbox("owned", &s_owned);

		if (ImGui::BeginTabBar("##aatypes"))
		{
			for (int type = 1; type <= static_cast<int>(std::size(kAATypes)); ++type)
			{
				if (!ImGui::BeginTabItem(kAATypes[type - 1]))
					continue;
				if (ImGui::BeginTable("##aas", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY
					| ImGuiTableFlags_SizingStretchProp, ImVec2(0.0f, 0.0f)))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("ability");
					ImGui::TableSetupColumn("rank");
					ImGui::TableSetupColumn("cost");
					ImGui::TableSetupColumn("level");
					ImGui::TableSetupColumn("");
					ImGui::TableHeadersRow();
					static std::vector<const AAEntry*> s_rows;
					s_rows.clear();
					for (const AAEntry& e : s_aas)
					{
						if (e.type != type || (s_filter[0] && ci_find_substr(e.name, s_filter) == -1))
							continue;
						if ((s_trainable && !e.canTrain) || (s_owned && e.rank == 0))
							continue;
						s_rows.push_back(&e);
					}
					ImGuiListClipper clipper;
					clipper.Begin(static_cast<int>(s_rows.size()));
					while (clipper.Step())
						for (int r = clipper.DisplayStart; r < clipper.DisplayEnd; ++r)
						{
							const AAEntry& e = *s_rows[r];
							const bool canTrain = e.canTrain;
							ImGui::PushID(static_cast<int>(&e - s_aas.data()));
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							ImGui::Selectable(e.name);
							const bool nameHovered = ImGui::IsItemHovered();
							if (e.activatable)
							{
								if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
									ImGui::OpenPopup("##aamenu");
								DragButtonSource(AAButton(e));
								if (ImGui::BeginPopup("##aamenu"))
								{
									AddToBarMenu(AAButton(e));
									ImGui::EndPopup();
								}
							}
							if (nameHovered && !ImGui::IsPopupOpen("##aamenu") && pDBStr)
								if (CAltAbilityData* ab = GetAAById(e.index))
									if (const char* desc = pDBStr->GetString(ab->nDesc, eAltAbilityDescription))
									{
										ImGui::BeginTooltip();
										ImGui::PushTextWrapPos(ImGui::GetFontSize() * 26.0f);
										ImGui::TextUnformatted(desc);
										ImGui::PopTextWrapPos();
										ImGui::EndTooltip();
									}
							ImGui::TableNextColumn();
							ImGui::Text("%d / %d", e.rank, e.maxRank);
							ImGui::TableNextColumn();
							if (e.nextIndex >= 0)
								ImGui::Text("%d", e.nextCost);
							ImGui::TableNextColumn();
							if (e.nextIndex >= 0)
								ImGui::Text("%d", e.nextLevel);
							ImGui::TableNextColumn();
							ImGui::BeginDisabled(!canTrain);
							if (ImGui::SmallButton("train"))
							{
								Command("/alt buy %d", e.group);
							}
							ImGui::EndDisabled();
							if (e.activatable && e.rank > 0)
							{
								ImGui::SameLine();
								if (ImGui::SmallButton("use"))
									Command("/alt activate %d", e.group);
								DragButtonSource(AAButton(e));
							}
							ImGui::PopID();
						}
					ImGui::EndTable();
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
	if (!open)
		w.wnd->Show(false);
}

void DrawBandolier(SkinnedWindow& w)
{
	if (!w.wnd->IsVisible() || ProfileClass().empty())
		return;
	const PcProfile* profile = GetPcProfile();
	if (!profile)
		return;

	bool open = true;
	if (BeginOurWindow(w.wnd, "Bandolier##eqgfx", ImVec2(360.0f, 300.0f), open))
	{
		const float size = ImGui::GetFrameHeight();
		for (int i = 0; i < MAX_BANDOLIER_ITEMS; ++i)
		{
			const BandolierSet& set = profile->Bandolier[i];
			if (!set.Name[0])
				continue;
			ImGui::PushID(i);
			for (const BandolierItemInfo& item : set.Items)
			{
				if (item.IconID > 0)
					ItemIcon(item.IconID, size);
				else
					ImGui::Dummy(ImVec2(size, size));
				if (ImGui::IsItemHovered() && item.Name[0])
					ImGui::SetTooltip("%s", item.Name);
				ImGui::SameLine();
			}
			ImGui::TextUnformatted(set.Name);
			ImGui::SameLine();
			if (ImGui::SmallButton("use"))
				Command("/bandolier activate %s", set.Name);
			ImGui::SameLine();
			if (ImGui::SmallButton("delete"))
				Command("/bandolier delete %s", set.Name);
			HotButton b;
			b.label = set.Name;
			b.cmd = std::string("/bandolier activate ") + set.Name;
			DragButtonSource(b);
			ImGui::PopID();
		}

		ImGui::Separator();
		static char s_name[32] = {};
		ImGui::SetNextItemWidth(160.0f);
		ImGui::InputTextWithHint("##name", "name", s_name, sizeof(s_name));
		ImGui::SameLine();
		ImGui::BeginDisabled(!s_name[0]);
		if (ImGui::Button("save what I hold"))
		{
			Command("/bandolier add %s", s_name);
			s_name[0] = 0;
		}
		ImGui::EndDisabled();
	}
	ImGui::End();
	if (!open)
		w.wnd->Show(false);
}

void DrawKeyRing(SkinnedWindow& w)
{
	if (!w.wnd->IsVisible() || !pLocalPC || ProfileClass().empty())
		return;

	constexpr const char* kTabs[] = { "Mounts", "Illusions", "Familiars", "Hero's Forge", "Teleport", "Activated", "Equipment" };
	bool open = true;
	if (BeginOurWindow(w.wnd, "Key Rings##eqgfx", ImVec2(320.0f, 380.0f), open))
	{
		static char s_filter[64] = {};
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##filter", "search", s_filter, sizeof(s_filter));
		const float size = ImGui::GetTextLineHeight() + 6.0f;
		if (ImGui::BeginTabBar("##rings", ImGuiTabBarFlags_FittingPolicyScroll))
		{
			for (int type = eKeyRingTypeFirst; type <= eKeyRingTypeLast; ++type)
			{
				if (!ImGui::BeginTabItem(kTabs[type]))
					continue;
				if (ImGui::BeginChild("##items"))
				{
					int n = 0;
					for (const ItemPtr& item : pLocalPC->GetKeyRingItems(static_cast<KeyRingType>(type)))
					{
						if (!item || (s_filter[0] && ci_find_substr(item->GetName(), s_filter) == -1))
							continue;
						ImGui::PushID(n++);
						const ImVec2 at = ImGui::GetCursorScreenPos();
						ImGui::SetCursorScreenPos(ImVec2(at.x + size + 4.0f, at.y));
						HotButton b;
						b.label = item->GetName();
						b.cmd = std::string("/useitem \"") + item->GetName() + "\"";
						b.iconType = ButtonIcon::Item;
						b.icon = item->GetIconID() - 500;
						if (ImGui::Selectable(item->GetName(), false, 0, ImVec2(0.0f, size)))
							RunButton(b);
						DragButtonSource(b);
						Icon(ImGui::GetWindowDrawList(), ItemIconSheet(), item->GetIconID() - 500, at,
							ImVec2(at.x + size, at.y + size));
						ImGui::PopID();
					}
					if (!n)
						ImGui::TextDisabled("none");
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
	if (!open)
		w.wnd->Show(false);
}

void DrawMercenary(SkinnedWindow& w)
{
	if (!w.wnd->IsVisible() || !pMercManager || ProfileClass().empty())
		return;

	bool open = true;
	if (BeginOurWindow(w.wnd, "Mercenary##eqgfx", ImVec2(300.0f, 260.0f), open))
	{
		PlayerClient* merc = pMercManager->mercenarySpawnId ? GetSpawnByID(pMercManager->mercenarySpawnId) : nullptr;
		ImGui::Text("%s", merc ? merc->DisplayedName : "no mercenary out");
		const eMercenaryState state = pMercManager->GetMercenaryState();
		ImGui::TextDisabled("%s", state == MercenaryState_Active ? "active"
			: state == MercenaryState_Suspended ? "suspended" : state == MercenaryState_Dead ? "dead" : "none");

		struct Btn { const char* label; const char* id; };
		constexpr Btn kButtons[] = {
			{ "Suspend / Unsuspend", "MMGW_SuspendButton" }, { "Dismiss", "MMGW_DismissButton" },
			{ "Call for assist", "MMGW_CallForAssistButton" }, { "Info", "MMGW_InfoButton" },
		};
		for (const Btn& b : kButtons)
		{
			if (ImGui::Button(b.label))
				Command("/notify MMGW_ManageWnd %s leftmouseup", b.id);
			ImGui::SameLine();
		}
		ImGui::NewLine();

		ImGui::SeparatorText("stance");
		const MercenaryStanceInfo* active = pMercManager->GetActiveMercenaryStance();
		for (const MercenaryStanceInfo* stance : pMercManager->mercenaryStanceList)
		{
			const char* name = pDBStr ? pDBStr->GetString(stance->stanceStringId, eMercenaryStanceName) : nullptr;
			if (!name || !*name)
				continue;
			if (ImGui::RadioButton(name, stance == active))
				Command("/stance %s", name);
			ImGui::SameLine();
		}
		ImGui::NewLine();

		if (pMercManager->numMercenaries > 1)
		{
			ImGui::SeparatorText("switch");
			for (int i = 1; i <= pMercManager->numMercenaries; ++i)
			{
				char label[16];
				std::snprintf(label, sizeof(label), "%d", i);
				if (ImGui::Button(label))
					Command("/mercswitch %d", i);
				ImGui::SameLine();
			}
			ImGui::NewLine();
		}
	}
	ImGui::End();
	if (!open)
		w.wnd->Show(false);
}

void DrawBags(SkinnedWindow& w)
{
	(void)w;
	CContainerMgr* mgr = pContainerMgr.get();
	if (!mgr || !pLocalPC || ProfileClass().empty())
		return;
	for (CContainerWnd* wnd : mgr->pContainerWnds)
		if (wnd && wnd->IsVisible() && wnd->Container && wnd->Location.GetLocation() == eItemContainerPossessions)
			Bag(wnd);
}

} // namespace eqgfx::ui
