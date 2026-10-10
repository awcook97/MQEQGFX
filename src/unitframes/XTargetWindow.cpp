#include "UnitWindow.h"

#include "../MQEQGFX.h"

#include <imgui.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <vector>

namespace eqgfx::unitframes {

namespace {

bool SlotUsed(const ExtendedTargetSlot* xts)
{
	return xts && xts->xTargetType != 0 && xts->XTargetSlotStatus != eXTSlotEmpty
		&& xts->SpawnID != 0;
}

struct SlotRole
{
	const char* label;
	const char* token;
	int         type;
};

constexpr SlotRole kSlotRoles[] = {
	{ "Empty Target", "EmptyTarget", XTARGET_EMPTY_TARGET },
	{ "Auto", "AutoHater", XTARGET_AUTO_HATER },
	{ "Current Target", "CurrentTarget", -1 },
	{ "Target's Target", "TargetsTarget", XTARGET_TARGETS_TARGET },
	{ "Group Tank", "GroupTank", XTARGET_GROUP_TANK },
	{ "Group Tank's Target", "GroupTankTarget", XTARGET_GROUP_TANKS_TARGET },
	{ "Group Assist", "GroupAssist", XTARGET_GROUP_ASSIST },
	{ "Group Assist's Target", "GroupAssistTarget", XTARGET_GROUP_ASSIST_TARGET },
	{ "Puller", "GroupPuller", XTARGET_GROUP_PULLER },
	{ "Puller's Target", "GroupPullerTarget", XTARGET_GROUP_PULLER_TARGET },
	{ "Group Mark Target 1", "GroupMark1", XTARGET_GROUP_MARK1 },
	{ "Group Mark Target 2", "GroupMark2", XTARGET_GROUP_MARK2 },
	{ "Group Mark Target 3", "GroupMark3", XTARGET_GROUP_MARK3 },
	{ "Raid Assist 1", "RaidAssist1", XTARGET_RAID_ASSIST1 },
	{ "Raid Assist 2", "RaidAssist2", XTARGET_RAID_ASSIST2 },
	{ "Raid Assist 3", "RaidAssist3", XTARGET_RAID_ASSIST3 },
	{ "Raid Assist 1 Target", "RaidAssist1Target", XTARGET_RAID_ASSIST1_TARGET },
	{ "Raid Assist 2 Target", "RaidAssist2Target", XTARGET_RAID_ASSIST2_TARGET },
	{ "Raid Assist 3 Target", "RaidAssist3Target", XTARGET_RAID_ASSIST3_TARGET },
	{ "Raid Mark Target 1", "RaidMark1", XTARGET_RAID_MARK1 },
	{ "Raid Mark Target 2", "RaidMark2", XTARGET_RAID_MARK2 },
	{ "Raid Mark Target 3", "RaidMark3", XTARGET_RAID_MARK3 },
	{ "My Pet", "Pet", XTARGET_MY_PET },
	{ "My Pet's Target", "PetTarget", XTARGET_MY_PET_TARGET },
	{ "My Mercenary", "Mercenary", XTARGET_MY_MERCENARY },
	{ "My Mercenary's Target", "MercenaryTarget", XTARGET_MY_MERCENARY_TARGET },
};

void XCommand(const char* fmt, ...)
{
	char line[MAX_STRING];
	va_list args;
	va_start(args, fmt);
	std::vsnprintf(line, sizeof(line), fmt, args);
	va_end(args);
	DoCommand(line);
}

class XTargetWindow : public UnitWindow
{
public:
	XTargetWindow()
		: UnitWindow(Window::XTarget, "EQGFX Extended Targets", ImVec2(320, 360))
	{
		Config().show = true;
		FinishPanel();
	}

protected:
	bool HasContent() const override
	{
		ExtendedTargetList* xtm = pLocalPC ? pLocalPC->pExtendedTargetList : nullptr;
		if (!xtm)
			return false;
		if (m_showEmpty)
			return true;

		const int slots = xtm->GetNumSlots();
		for (int i = 0; i < slots; ++i)
		{
			ExtendedTargetSlot* xts = xtm->GetSlot(i);
			if (SlotUsed(xts) && GetSpawnByID(static_cast<int>(xts->SpawnID)))
				return true;
		}
		return false;
	}

	void OnDraw(float width) override
	{
		ExtendedTargetList* xtm = pLocalPC ? pLocalPC->pExtendedTargetList : nullptr;
		if (!xtm)
			return;

		struct Entry
		{
			PlayerClient* sp;
			int           slot;
			const char*   role;
		};

		std::vector<Entry> entries;
		const int slots = std::min(xtm->GetNumSlots(), m_maxSlots);

		for (int i = 0; i < slots; ++i)
		{
			ExtendedTargetSlot* xts = xtm->GetSlot(i);
			PlayerClient* sp = SlotUsed(xts) ? GetSpawnByID(static_cast<int>(xts->SpawnID)) : nullptr;
			if (!sp)
			{
				if (m_showEmpty && xts)
					EmptySlot(i, xtm->ExtendedTargetRoleName(xts->xTargetType));
				continue;
			}
			if (m_hideDead && sp->HPCurrent <= 0 && sp->HPMax > 0)
				continue;
			if (m_range > 0.0f && GetDistance3D(pLocalPlayer.get(), sp) > m_range)
				continue;

			entries.push_back({ sp, i, xtm->ExtendedTargetRoleName(xts->xTargetType) });
		}

		SortEntries(entries);

		for (const Entry& e : entries)
		{
			if (m_showRoleNames && e.role && *e.role)
				ImGui::TextDisabled("%s", e.role);

			char tag[8] = { 0 };
			if (m_slotNumbers)
				std::snprintf(tag, sizeof(tag), "%d", e.slot + 1);

			RowOptions opts = DefaultRowOptions();
			opts.pet = m_pet;
			opts.mana = m_mana;
			opts.endurance = m_endurance;
			opts.aggro = m_aggro;
			opts.aggroPct = XTargetAggroPct(e.slot);
			opts.tag = tag[0] ? tag : nullptr;
			opts.role = e.role;

			m_menuSlot = e.slot;
			Row(e.sp, opts, width);
			m_menuSlot = -1;
		}
	}

	void OnRowMenu(PlayerClient* sp) override
	{
		(void)sp;
		SlotMenu(m_menuSlot);
	}

	void OnBuildPanel(SettingsPanel& p) override
	{
		p.Header("extended targets");
		p.Bool("RoleNames", "role name above each bar", &m_showRoleNames,
			"the tooltip already carries it, and this costs a row per target");
		p.Bool("SlotNumbers", "slot number tag", &m_slotNumbers, nullptr, true);
		p.Bool("Empty", "unused slots", &m_showEmpty, "so a slot can be given a role");
		p.Bool("Pet", "its pet's health", &m_pet);
		p.Bool("Mana", "mana bars", &m_mana, nullptr, true);
		p.Bool("Endurance", "endurance bars", &m_endurance, nullptr, true);
		p.Bool("Aggro", "my aggro on it", &m_aggro, nullptr, true);
		p.Bool("HideDead", "hide dead targets", &m_hideDead, nullptr, true);
		p.Int("MaxSlots", "slots to show", &m_maxSlots, 1, 20);
		p.Float("Range", "hide beyond", &m_range, 0.0f, 1000.0f,
			"0 shows everything, whatever the distance", "%.0f");
		p.Enum("Sort", "order", &m_sort, kSortLabels, Sort::Count);
	}

	void OnReset() override
	{
		m_showRoleNames = false;
		m_slotNumbers = false;
		m_pet = true;
		m_mana = true;
		m_endurance = true;
		m_showEmpty = true;
		m_aggro = true;
		m_hideDead = false;
		m_maxSlots = 20;
		m_range = 0.0f;
		m_sort = Sort::None;
	}

private:
	void EmptySlot(int slot, const char* role)
	{
		ImGui::PushID(slot);
		char text[96];
		std::snprintf(text, sizeof(text), "%d  %s", slot + 1, role && *role ? role : "Empty");
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		ImGui::Selectable(text);
		ImGui::PopStyleColor();
		NoteHotItem(Id());
		if (ImGui::BeginPopupContextItem("##slot"))
		{
			SlotMenu(slot);
			ImGui::Separator();
			if (ImGui::MenuItem("Options..."))
				ShowOptions(true);
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}

	void SlotMenu(int slot)
	{
		ExtendedTargetList* xtm = pLocalPC ? pLocalPC->pExtendedTargetList : nullptr;
		if (!xtm)
			return;

		if (slot >= 0)
		{
			const ExtendedTargetSlot* xts = xtm->GetSlot(slot);
			char title[32];
			std::snprintf(title, sizeof(title), "Slot %d", slot + 1);
			if (ImGui::BeginMenu(title))
			{
				for (const SlotRole& r : kSlotRoles)
					if (ImGui::MenuItem(r.label, nullptr, xts && r.type >= 0 && static_cast<int>(xts->xTargetType) == r.type))
						XCommand("/xtarget set %d %s", slot + 1, r.token);
				ImGui::EndMenu();
			}
			ImGui::Separator();
		}

		if (ImGui::MenuItem("Auto Add Hater Targets", nullptr, xtm->GetAutoAddHaters()))
			XCommand("/xtarget auto %s", xtm->GetAutoAddHaters() ? "off" : "on");
		ImGui::Separator();

		bool changed = false;
		changed |= ImGui::MenuItem("Mana Display", nullptr, &m_mana);
		changed |= ImGui::MenuItem("Endurance Display", nullptr, &m_endurance);
		changed |= ImGui::MenuItem("Show Player Aggro Percent", nullptr, &m_aggro);
		if (changed)
			MarkSettingsDirty();
		ImGui::Separator();

		if (ImGui::BeginMenu("Save/Load target sets"))
		{
			if (ImGui::BeginMenu("Load target set"))
			{
				bool any = false;
				const TargetSetLoadout* sets = pEverQuestInfo ? pEverQuestInfo->targetSetLoadouts : nullptr;
				for (int i = 0; sets && i < NUM_XTARGET_SETS; ++i)
				{
					const TargetSetLoadout& set = sets[i];
					if (!set.inuse || !set.name[0])
						continue;
					any = true;
					if (ImGui::MenuItem(set.name))
						XCommand("/xtarget load %s", set.name);
				}
				if (!any)
					ImGui::TextDisabled("none saved");
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Save target set"))
			{
				ImGui::SetNextItemWidth(160.0f);
				ImGui::InputTextWithHint("##name", "name", m_setName, sizeof(m_setName));
				ImGui::SameLine();
				ImGui::BeginDisabled(!m_setName[0]);
				if (ImGui::Button("Save"))
				{
					XCommand("/xtarget save %s", m_setName);
					m_setName[0] = 0;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndDisabled();
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
	}

	template <typename T>
	void SortEntries(std::vector<T>& v) const
	{
		switch (m_sort)
		{
		case Sort::Distance:
			std::stable_sort(v.begin(), v.end(), [](const T& a, const T& b) {
				return GetDistanceSquared3D(pLocalPlayer.get(), a.sp) < GetDistanceSquared3D(pLocalPlayer.get(), b.sp); });
			break;
		case Sort::HealthLow:
			std::stable_sort(v.begin(), v.end(), [](const T& a, const T& b) {
				return PctHP(a.sp) < PctHP(b.sp); });
			break;
		case Sort::HealthHigh:
			std::stable_sort(v.begin(), v.end(), [](const T& a, const T& b) {
				return PctHP(a.sp) > PctHP(b.sp); });
			break;
		case Sort::Name:
			std::stable_sort(v.begin(), v.end(), [](const T& a, const T& b) {
				return _stricmp(a.sp->DisplayedName, b.sp->DisplayedName) < 0; });
			break;
		case Sort::Class:
			std::stable_sort(v.begin(), v.end(), [](const T& a, const T& b) {
				return a.sp->GetClass() < b.sp->GetClass(); });
			break;
		default:
			break;
		}
	}

	int   m_menuSlot = -1;
	char  m_setName[32] = {};
	bool  m_showEmpty = true;
	bool  m_mana = true;
	bool  m_endurance = true;
	bool  m_showRoleNames = false;
	bool  m_slotNumbers = false;
	bool  m_pet = true;
	bool  m_aggro = true;
	bool  m_hideDead = false;
	int   m_maxSlots = 20;
	float m_range = 0.0f;
	int   m_sort = Sort::None;
};

} // namespace

UnitWindow* CreateXTargetWindow()
{
	return new XTargetWindow();
}

} // namespace eqgfx::unitframes
