#include "../Actions.h"
#include "../ClassProfile.h"
#include "../HotButtons.h"
#include "../Skins.h"
#include "../SpellBook.h"
#include "../Widgets.h"

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace eqgfx::ui {

namespace {

constexpr const char* kPayload = "EQGFX_HOTBUTTON";

struct Slot
{
	int bar, page, slot;
};

struct Editor
{
	bool open = false;
	int  serial = 0;
	std::string where;
	std::function<void(const HotButton&)> save;
	std::function<void()> remove;
	char label[64] = {};
	char cmd[2048] = {};
	char timerKey[64] = {};
	int  iconType = 0;
	int  icon = 0;
	int  timer = 0;
};

Editor s_editor;

bool s_dragging = false;
Slot s_dragFrom{};
bool s_delivered = false;

void FillEditor(const HotButton& b)
{
	strncpy_s(s_editor.label, sizeof(s_editor.label), b.label.c_str(), _TRUNCATE);
	strncpy_s(s_editor.cmd, sizeof(s_editor.cmd), b.cmd.c_str(), _TRUNCATE);
	strncpy_s(s_editor.timerKey, sizeof(s_editor.timerKey), b.timerKey.c_str(), _TRUNCATE);
	s_editor.iconType = static_cast<int>(b.iconType);
	s_editor.icon = b.icon;
	s_editor.timer = static_cast<int>(b.timer);
}

void OpenEditor(const Slot& at)
{
	char where[64];
	std::snprintf(where, sizeof(where), "bar %d, page %d, button %d", at.bar + 1, at.page + 1, at.slot + 1);
	EditButton(where, ButtonAt(at.bar, at.page, at.slot),
		[at](const HotButton& b) {
			ButtonAt(at.bar, at.page, at.slot) = b;
			ButtonChanged(at.bar, at.page);
		},
		[at]() {
			ButtonAt(at.bar, at.page, at.slot) = HotButton{};
			ButtonChanged(at.bar, at.page);
		});
}

void DrawIcon(ImDrawList* dl, const HotButton& b, ImVec2 mn, ImVec2 mx)
{
	if (b.iconType == ButtonIcon::Spell)
		Icon(dl, SpellIconSheet(), b.icon, mn, mx);
	else if (b.iconType == ButtonIcon::Item)
		Icon(dl, ItemIconSheet(), b.icon, mn, mx);
}

void Take(const Slot& at)
{
	if (AssignFromCursor(ButtonAt(at.bar, at.page, at.slot)))
		ButtonChanged(at.bar, at.page);
}

} // namespace

void DrawHotbar(SkinnedWindow& w)
{
	CHotButtonWnd* bar = static_cast<CHotButtonWnd*>(w.wnd);
	if (!bar->IsVisible() || ProfileClass().empty())
		return;
	const int index = w.bar;
	if (index < 0 || index >= kHotBars)
		return;
	const int page = HotBarPage(index);

	std::vector<NativeSlot> natives;
	std::vector<int> slots;
	for (int b = 0; b < kHotSlots; ++b)
		if (CHotButton* hb = bar->Buttons[b]; hb && hb->IsVisible())
		{
			natives.push_back(SlotOf(bar, hb));
			slots.push_back(b);
		}
	if (natives.empty())
		return;

	char title[32];
	std::snprintf(title, sizeof(title), "Hotbar %d##eqgfx", index + 1);
	const std::vector<Cell>& cells = BeginBar(title, bar, natives);
	const BarLayout& layout = CurrentBar();

	const Palette& p = Colors();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const bool carrying = CursorHasButton();

	for (size_t k = 0; k < cells.size(); ++k)
	{
		const int b = slots[k];
		const Slot at{ index, page, b };
		HotButton& button = ButtonAt(index, page, b);
		const ImVec2 mn = cells[k].min;
		const ImVec2 mx = cells[k].max;

		ImGui::PushID(b);
		ImGui::SetCursorScreenPos(mn);
		ImGui::InvisibleButton("##hot", ImVec2(mx.x - mn.x, mx.y - mn.y),
			ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
		const bool hovered = ImGui::IsItemHovered();
		const bool tap = LeftTap();
		const Press right = RightPress();

		if (!button.Empty() && ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(kPayload, &at, sizeof(at));
			ImGui::TextUnformatted(button.label.c_str());
			ImGui::TextDisabled("drop outside a bar to remove");
			ImGui::EndDragDropSource();
			s_dragging = true;
			s_dragFrom = at;
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayload))
			{
				const Slot from = *static_cast<const Slot*>(payload->Data);
				SwapButtons(from.bar, from.page, from.slot, at.bar, at.page, at.slot);
				s_delivered = true;
			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kSpellPayload))
			{
				MakeSpellButton(button, GetSpellByID(*static_cast<const int*>(payload->Data)));
				ButtonChanged(at.bar, at.page);
			}
			if (AcceptDraggedButton(button))
				ButtonChanged(at.bar, at.page);
			ImGui::EndDragDropTarget();
		}

		dl->AddRectFilled(mn, mx, hovered ? (carrying ? p.buttonOn : p.buttonHover) : p.button, p.rounding);

		if (!button.Empty())
		{
			const ImVec2 imn(mn.x + 2.0f, mn.y + 2.0f);
			const ImVec2 imx(mx.x - 2.0f, mx.y - 2.0f);
			DrawIcon(dl, button, imn, imx);

			uint32_t total = 0;
			const uint32_t left = ButtonCooldown(button, total);
			DrawCooldownOverlay(dl, imn, imx, left, total);

			if (layout.labels && !button.label.empty())
			{
				dl->PushClipRect(mn, mx, true);
				const ImVec2 size = ImGui::CalcTextSize(button.label.c_str());
				const float y = button.iconType == ButtonIcon::None ? (mn.y + mx.y) * 0.5f : mx.y - size.y * 0.5f - 1.0f;
				IconText(dl, ImVec2((mn.x + mx.x) * 0.5f, y), button.label.c_str(), p.text);
				dl->PopClipRect();
			}

			if (hovered && !carrying && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				const EQ_Spell* spell = button.iconType == ButtonIcon::Spell ? GetSpellByName(button.timerKey) : nullptr;
				if (spell)
					SpellTooltip(spell, static_cast<int>(left), false);
				else
					ImGui::SetTooltip("%s\n%s", button.label.c_str(), button.cmd.c_str());
			}
		}

		if (layout.keys)
		{
			const CXStr& key = bar->KeyMapStrings[b];
			if (!key.empty())
				dl->AddText(ImVec2(mn.x + 2.0f, mn.y + 1.0f), p.textDim, key.c_str());
		}

		dl->AddRect(mn, mx, p.buttonEdge, p.rounding);
		ImGui::PopID();

		if (tap)
		{
			if (carrying)
				Take(at);
			else if (!button.Empty())
				RunButton(button);
		}
		if (right == Press::Tap)
		{
			if (carrying)
				Take(at);
			else
				OpenEditor(at);
		}
	}

	if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheel != 0.0f)
		TurnHotbarPage(index, ImGui::GetIO().MouseWheel > 0.0f ? -1 : 1);

	char pageText[8];
	std::snprintf(pageText, sizeof(pageText), "%d", page + 1);
	const ImVec2 wp = ImGui::GetWindowPos();
	dl->AddText(ImVec2(wp.x + ImGui::GetWindowWidth() - ImGui::CalcTextSize(pageText).x - 3.0f, wp.y), p.textDim, pageText);

	EndBar();
}

void EditButton(std::string where, const HotButton& b, std::function<void(const HotButton&)> save,
	std::function<void()> remove)
{
	s_editor = Editor{};
	s_editor.open = true;
	s_editor.serial = ProfileSerial();
	s_editor.where = std::move(where);
	s_editor.save = std::move(save);
	s_editor.remove = std::move(remove);
	FillEditor(b);
}

void DrawHotButtonEditor()
{
	if (s_dragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		if (!s_delivered && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		{
			ButtonAt(s_dragFrom.bar, s_dragFrom.page, s_dragFrom.slot) = HotButton{};
			ButtonChanged(s_dragFrom.bar, s_dragFrom.page);
		}
		s_dragging = false;
		s_delivered = false;
	}

	if (s_editor.open && s_editor.serial != ProfileSerial())
		s_editor = Editor{};

	if (!s_editor.open)
		return;

	constexpr const char* kIconTypes[] = { "none", "spell", "item" };
	constexpr const char* kTimers[] = { "none", "spell gem", "item", "alternate ability", "discipline" };
	constexpr const char* kId = "Hot Button##eqgfx";

	PlaceWindow(kId);
	if (ImGui::Begin(kId, &s_editor.open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
	{
		NoteWindow(kId);
		ImGui::TextDisabled("%s", s_editor.where.c_str());

		ImGui::BeginDisabled(!CursorHasButton());
		if (ImGui::Button("from cursor"))
		{
			HotButton made;
			if (AssignFromCursor(made))
				FillEditor(made);
		}
		ImGui::EndDisabled();

		ImGui::SetNextItemWidth(260.0f);
		ImGui::InputText("label", s_editor.label, sizeof(s_editor.label));
		ImGui::InputTextMultiline("commands", s_editor.cmd, sizeof(s_editor.cmd), ImVec2(260.0f, 110.0f));
		ImGui::SetNextItemWidth(120.0f);
		ImGui::Combo("icon", &s_editor.iconType, kIconTypes, IM_ARRAYSIZE(kIconTypes));
		if (s_editor.iconType != 0)
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.0f);
			ImGui::InputInt("##icon", &s_editor.icon);
		}
		ImGui::SetNextItemWidth(160.0f);
		ImGui::Combo("timer", &s_editor.timer, kTimers, IM_ARRAYSIZE(kTimers));
		if (s_editor.timer != 0)
		{
			ImGui::SetNextItemWidth(260.0f);
			ImGui::InputText("timer name", s_editor.timerKey, sizeof(s_editor.timerKey));
		}

		if (ImGui::Button("Save"))
		{
			HotButton b;
			b.label = s_editor.label;
			b.cmd = s_editor.cmd;
			b.iconType = static_cast<ButtonIcon>(s_editor.iconType);
			b.icon = s_editor.icon;
			b.timer = static_cast<ButtonTimer>(s_editor.timer);
			b.timerKey = s_editor.timerKey;
			if (s_editor.save)
				s_editor.save(b);
			s_editor.open = false;
		}
		ImGui::SameLine();
		if (s_editor.remove && ImGui::Button("Remove"))
		{
			s_editor.remove();
			s_editor.open = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			s_editor.open = false;
	}
	ImGui::End();
}

} // namespace eqgfx::ui
