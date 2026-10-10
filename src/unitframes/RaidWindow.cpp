#include "UnitWindow.h"

#include "../MQEQGFX.h"

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <vector>

namespace eqgfx::unitframes {

namespace {

class RaidWindow : public UnitWindow
{
public:
	RaidWindow()
		: UnitWindow(Window::Raid, "EQGFX Raid", ImVec2(320, 420))
	{
		FinishPanel();
	}

protected:
	bool HasContent() const override
	{
		return pRaid && pRaid->Invited == RaidStateInRaid && pRaid->RaidMemberCount > 0;
	}

	void OnDraw(float width) override
	{
		if (!HasContent())
			return;

		struct Entry
		{
			PlayerClient* sp;
			int           group;
			char          tag[8];
			char          role[48];
		};

		const int count = std::min<int>(pRaid->RaidMemberCount, MAX_RAID_SIZE);
		const int myGroup = MyGroupNumber();

		std::vector<Entry> entries;
		entries.reserve(count);

		for (int i = 0; i < MAX_RAID_SIZE; ++i)
		{
			if (!pRaid->RaidMemberUsed[i])
				continue;

			const RaidMember& rm = pRaid->RaidMember[i];
			if (rm.Name[0] == 0)
				continue;
			if (m_onlyMyGroup && myGroup >= 0 && rm.GroupNumber != myGroup)
				continue;

			PlayerClient* sp = GetSpawnByName(rm.Name);
			if (!sp)
			{
				if (m_showOffline)
					ImGui::TextDisabled("  %s  (out of zone)", rm.Name);
				continue;
			}

			Entry e{};
			e.sp = sp;
			e.group = rm.GroupNumber;

			size_t n = 0;
			if (rm.RaidLeader)     e.tag[n++] = 'L';
			if (rm.RaidMainAssist) e.tag[n++] = 'A';
			if (rm.RaidMarker)     e.tag[n++] = 'M';
			e.tag[n] = 0;

			std::snprintf(e.role, sizeof(e.role), "group %d%s%s%s", rm.GroupNumber + 1,
				rm.RaidLeader ? ", raid leader" : "",
				rm.RaidMainAssist ? ", main assist" : "",
				rm.RaidMarker ? ", marker" : "");

			entries.push_back(e);
		}

		SortEntries(entries);

		int lastGroup = -1;
		for (const Entry& e : entries)
		{
			if (m_groupHeadings && m_sort == Sort::None && e.group != lastGroup)
			{
				lastGroup = e.group;
				ImGui::TextDisabled("group %d", lastGroup + 1);
			}

			RowOptions opts = DefaultRowOptions();
			opts.mana = m_mana;
			opts.endurance = m_endurance;
			opts.pet = m_pet;
			opts.tag = e.tag[0] ? e.tag : nullptr;
			opts.role = e.role;

			Row(e.sp, opts, width);
		}
	}

	void OnBuildPanel(SettingsPanel& p) override
	{
		p.Header("raid");
		p.Bool("Mana", "mana bars", &m_mana);
		p.Bool("Endurance", "endurance bars", &m_endurance, nullptr, true);
		p.Bool("PetBar", "pet health bars", &m_pet, nullptr, true);
		p.Bool("GroupHeadings", "group headings", &m_groupHeadings,
			"only meaningful while the list is in raid order");
		p.Bool("OnlyMyGroup", "only my group", &m_onlyMyGroup, nullptr, true);
		p.Bool("Offline", "members out of zone", &m_showOffline, nullptr, true);
		p.Enum("Sort", "order", &m_sort, kSortLabels, Sort::Count);
	}

	void OnReset() override
	{
		m_mana = m_endurance = true;
		m_pet = false;
		m_groupHeadings = true;
		m_onlyMyGroup = false;
		m_showOffline = false;
		m_sort = Sort::None;
	}

private:
	static int MyGroupNumber()
	{
		if (!pRaid || !pLocalPlayer)
			return -1;

		const int index = GetRaidMemberIndex(pLocalPlayer.get());
		return index >= 0 ? pRaid->RaidMember[index].GroupNumber : -1;
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

	bool m_mana = true;
	bool m_endurance = true;
	bool m_pet = false;
	bool m_groupHeadings = true;
	bool m_onlyMyGroup = false;
	bool m_showOffline = false;
	int  m_sort = Sort::None;
};

} // namespace

UnitWindow* CreateRaidWindow()
{
	return new RaidWindow();
}

} // namespace eqgfx::unitframes
