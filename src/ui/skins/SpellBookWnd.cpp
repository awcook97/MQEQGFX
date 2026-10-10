#include "../ClassProfile.h"
#include "../Skins.h"
#include "../SpellBook.h"
#include "../Widgets.h"

#include <cstdio>

namespace eqgfx::ui {

namespace {

char s_filter[64] = {};

int FirstEmptyGem()
{
	for (int gem = 0; gem < NUM_SPELL_GEMS; ++gem)
	{
		const CSpellGemWnd* slot = pCastSpellWnd ? pCastSpellWnd->SpellSlots[gem] : nullptr;
		if (slot && slot->IsVisible() && GetMemorizedSpell(gem) <= 0)
			return gem;
	}
	return -1;
}

void SpellRow(const BookSpell& s, float iconSize)
{
	ImGui::PushID(&s);
	const ImVec2 at = ImGui::GetCursorScreenPos();
	char row[160];
	std::snprintf(row, sizeof(row), "%3d  %s", s.level, s.spell->Name);

	ImGui::SetCursorScreenPos(ImVec2(at.x + iconSize + 4.0f, at.y));
	ImGui::Selectable(row, false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0.0f, iconSize));
	Icon(ImGui::GetWindowDrawList(), SpellIconSheet(), s.spell->SpellIcon, at, ImVec2(at.x + iconSize, at.y + iconSize));

	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		MemorizeSpell(FirstEmptyGem(), s.spell);

	if (ImGui::BeginDragDropSource())
	{
		const int id = s.spell->ID;
		ImGui::SetDragDropPayload(kSpellPayload, &id, sizeof(id));
		ImGui::TextUnformatted(s.spell->Name);
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginPopupContextItem("##gems"))
	{
		ImGui::TextDisabled("memorize into");
		for (int gem = 0; gem < NUM_SPELL_GEMS; ++gem)
		{
			const CSpellGemWnd* slot = pCastSpellWnd ? pCastSpellWnd->SpellSlots[gem] : nullptr;
			if (!slot || !slot->IsVisible())
				continue;
			const EQ_Spell* held = GetSpellByID(GetMemorizedSpell(gem));
			char item[160];
			std::snprintf(item, sizeof(item), "%d  %s", gem + 1, held ? held->Name : "(empty)");
			if (ImGui::MenuItem(item))
				MemorizeSpell(gem, s.spell);
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();
}

} // namespace

void DrawSpellBook(SkinnedWindow& w)
{
	if (!w.wnd->IsVisible() || ProfileClass().empty())
		return;

	constexpr const char* kId = "Spell Book##eqgfx";
	const CXRect r = w.wnd->GetScreenRect();
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(r.left), static_cast<float>(r.top)), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(360.0f, 480.0f), ImGuiCond_FirstUseEver);
	PlaceWindow(kId, true);

	bool open = true;
	if (ImGui::Begin(kId, &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
	{
		NoteWindow(kId);
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##filter", "search", s_filter, sizeof(s_filter));

		const float iconSize = ImGui::GetTextLineHeight() + 4.0f;
		if (ImGui::BeginChild("##spells"))
		{
			for (const BookCategory& cat : BookTree())
			{
				if (s_filter[0])
				{
					for (const BookGroup& group : cat.groups)
						for (const BookSpell& s : group.spells)
							if (ci_find_substr(s.spell->Name, s_filter) != -1)
								SpellRow(s, iconSize);
					continue;
				}
				if (!ImGui::TreeNode(cat.name.c_str()))
					continue;
				for (const BookGroup& group : cat.groups)
				{
					if (!ImGui::TreeNode(group.name.c_str()))
						continue;
					for (const BookSpell& s : group.spells)
						SpellRow(s, iconSize);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();

	if (!open)
		w.wnd->Show(false);
}

} // namespace eqgfx::ui
