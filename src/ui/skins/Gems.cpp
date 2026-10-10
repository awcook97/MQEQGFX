#include "../Actions.h"
#include "../Skins.h"
#include "../SpellBook.h"
#include "../Widgets.h"

#include <algorithm>
#include <cstdio>
#include <vector>

namespace eqgfx::ui {

namespace {

char s_search[64] = {};

} // namespace

void DrawGemBar(SkinnedWindow& w)
{
	CCastSpellWnd* wnd = static_cast<CCastSpellWnd*>(w.wnd);
	if (!wnd->IsVisible())
		return;

	std::vector<NativeSlot> natives;
	std::vector<int> gems;
	for (int i = 0; i < NUM_SPELL_GEMS; ++i)
		if (CSpellGemWnd* gem = wnd->SpellSlots[i]; gem && gem->IsVisible())
		{
			natives.push_back(SlotOf(wnd, gem));
			gems.push_back(i);
		}
	if (natives.empty())
		return;

	const std::vector<Cell>& cells = BeginBar("Spell Gems##eqgfx", wnd, natives);
	const BarLayout& layout = CurrentBar();
	const Palette& p = Colors();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const int casting = pLocalPlayer ? pLocalPlayer->CastingData.SpellID : -1;
	uint32_t recoveryTotal = 0;
	const uint32_t recovery = RecoveryLeft(recoveryTotal);

	for (size_t k = 0; k < cells.size(); ++k)
	{
		const int i = gems[k];
		const ImVec2 mn = cells[k].min;
		const ImVec2 mx = cells[k].max;

		const int spellId = GetMemorizedSpell(i);
		EQ_Spell* spell = spellId > 0 ? GetSpellByID(spellId) : nullptr;

		ImGui::PushID(i);
		ImGui::SetCursorScreenPos(mn);
		ImGui::InvisibleButton("##gem", ImVec2(mx.x - mn.x, mx.y - mn.y),
			ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
		const bool hovered = ImGui::IsItemHovered();
		const bool tap = LeftTap();
		const Press right = RightPress();

		if (spell && ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(kSpellPayload, &spellId, sizeof(spellId));
			ImGui::TextUnformatted(spell->Name);
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kSpellPayload))
				MemorizeSpell(i, GetSpellByID(*static_cast<const int*>(payload->Data)));
			ImGui::EndDragDropTarget();
		}

		dl->AddRectFilled(mn, mx, hovered ? p.buttonHover : p.button, p.rounding);

		if (spell)
		{
			uint32_t total = 0;
			const uint32_t left = GemCooldown(i, total);
			const bool busy = casting > 0 && spellId != casting;
			const ImVec2 imn(mn.x + 2.0f, mn.y + 2.0f);
			const ImVec2 imx(mx.x - 2.0f, mx.y - 2.0f);
			Icon(dl, SpellIconSheet(), spell->SpellIcon, imn, imx, left || recovery || busy ? 150 : 255);
			if (left >= recovery)
				DrawCooldownOverlay(dl, imn, imx, left, total);
			else if (recoveryTotal)
				CooldownSweep(dl, imn, imx, static_cast<float>(recovery) / recoveryTotal, p.recovery);
			if (spellId == casting)
				dl->AddRect(mn, mx, IM_COL32(255, 210, 90, 255), p.rounding, 0, 2.0f);
			if (layout.keys)
			{
				char number[4];
				std::snprintf(number, sizeof(number), "%d", i + 1);
				dl->AddText(ImVec2(mn.x + 2.0f, mn.y + 1.0f), p.textDim, number);
			}

			if (hovered && !ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
				SpellTooltip(spell, static_cast<int>(std::max(left, recovery)), false);

			if (tap)
				CastGem(i);
			if (right == Press::Tap)
				ForgetGem(i);
			else if (right == Press::Long)
				InspectSpell(spellId);
		}
		else if (right == Press::Tap)
		{
			s_search[0] = 0;
			ImGui::OpenPopup("##memorize");
		}

		if (ImGui::BeginPopup("##memorize"))
		{
			ImGui::TextDisabled("memorize into gem %d", i + 1);
			MemorizeMenu(i, s_search, sizeof(s_search));
			ImGui::EndPopup();
		}

		dl->AddRect(mn, mx, p.buttonEdge, p.rounding);
		ImGui::PopID();
	}

	EndBar();
}

} // namespace eqgfx::ui
