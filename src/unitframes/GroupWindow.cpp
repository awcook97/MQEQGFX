#include "UnitWindow.h"

#include "../MQEQGFX.h"

#include <imgui.h>

#include <cstdio>

namespace eqgfx::unitframes {

namespace {

struct RoleEntry
{
	eGroupRoles role;
	const char* label;
	char        letter;
};

constexpr RoleEntry kRoles[] = {
	{ GroupRoleTank,         "Main Tank",     'T' },
	{ GroupRoleAssist,       "Main Assist",   'A' },
	{ GroupRolePuller,       "Puller",        'P' },
	{ GroupRoleMarkNPC,      "Mark NPC",      'M' },
	{ GroupRoleMasterLooter, "Master Looter", 'L' },
};

class GroupWindow : public UnitWindow
{
public:
	GroupWindow()
		: UnitWindow(Window::Group, "EQGFX Group", ImVec2(320, 220))
	{
		Config().show = true;
		Config().buffs.show = false;
		FinishPanel();
	}

protected:
	bool HasContent() const override
	{
		if (pLocalPC && pLocalPC->Group)
			return true;
		return m_showSelf && pLocalPlayer.get() != nullptr;
	}

	void OnDraw(float width) override
	{
		PlayerClient* me = pLocalPlayer.get();

		if (!pLocalPC || !pLocalPC->Group)
		{
			if (m_showSelf && me)
				Row(me, RowOptionsFor(0, nullptr), width);
			return;
		}

		auto* group = pLocalPC->Group;
		const uint32_t maxSize = group->GetMaxGroupSize();

		for (uint32_t i = 0; i < maxSize; ++i)
		{
			CGroupMember* member = group->GetGroupMember(static_cast<int>(i));
			if (!member)
				continue;

			PlayerClient* sp = member->GetPlayer();
			if (sp && !m_showSelf && me && sp->SpawnID == me->SpawnID)
				continue;

			if (!sp)
			{
				if (m_showOffline)
					ImGui::TextDisabled("%s  (out of zone)", member->GetName());
				continue;
			}

			char letters[8];
			RoleLetters(member, letters, sizeof(letters));

			char role[64] = { 0 };
			RoleNames(member, role, sizeof(role));

			RowOptions opts = RowOptionsFor(static_cast<int>(i), role[0] ? role : nullptr);
			opts.tag = letters[0] ? letters : nullptr;

			m_menuMember = member;
			Row(sp, opts, width);
			m_menuMember = nullptr;
		}
	}

	void OnBuildPanel(SettingsPanel& p) override
	{
		p.Header("group");
		p.Bool("Mana", "mana bars", &m_mana);
		p.Bool("Endurance", "endurance bars", &m_endurance, nullptr, true);
		p.Bool("PetBar", "pet health bars", &m_pet, nullptr, true);
		p.Bool("Aggro", "aggro percent", &m_aggro, nullptr, true);
		p.Bool("ShowSelf", "include me", &m_showSelf);
		p.Bool("Offline", "members out of zone", &m_showOffline, nullptr, true);
	}

	void OnRowMenu(PlayerClient* sp) override
	{
		(void)sp;
		if (!m_menuMember)
			return;

		GroupRoleItems(m_menuMember);
	}

	void OnReset() override
	{
		m_mana = m_endurance = m_pet = m_aggro = true;
		m_showSelf = true;
		m_showOffline = true;
	}

private:
	RowOptions RowOptionsFor(int groupIndex, const char* role)
	{
		RowOptions opts = DefaultRowOptions();
		opts.mana = m_mana;
		opts.endurance = m_endurance;
		opts.pet = m_pet;
		opts.aggro = m_aggro;
		opts.aggroPct = GroupAggroPct(groupIndex);
		opts.role = role;
		return opts;
	}

	static void RoleLetters(const CGroupMember* member, char* out, size_t size)
	{
		out[0] = 0;
		if (!member || size < 8)
			return;

		size_t n = 0;
		for (const RoleEntry& r : kRoles)
		{
			if (member->GetRole(r.role) && n + 1 < size)
				out[n++] = r.letter;
		}
		out[n] = 0;
	}

	static void RoleNames(const CGroupMember* member, char* out, size_t size)
	{
		out[0] = 0;
		if (!member)
			return;

		size_t n = 0;
		for (const RoleEntry& r : kRoles)
		{
			if (!member->GetRole(r.role))
				continue;
			n += std::snprintf(out + n, size - n, "%s%s", n ? ", " : "", r.label);
			if (n >= size)
				break;
		}
	}

	CGroupMember* m_menuMember = nullptr;
	bool m_mana = true;
	bool m_endurance = true;
	bool m_pet = true;
	bool m_aggro = true;
	bool m_showSelf = true;
	bool m_showOffline = true;
};

} // namespace

CGroupMember* GroupMemberFor(const PlayerClient* sp)
{
	if (!sp || !pLocalPC || !pLocalPC->Group)
		return nullptr;
	auto* group = pLocalPC->Group;
	for (int i = 0; i < static_cast<int>(group->GetMaxGroupSize()); ++i)
		if (CGroupMember* member = group->GetGroupMember(i))
			if (member->GetPlayer() == sp)
				return member;
	return nullptr;
}

void GroupRoleItems(CGroupMember* member)
{
	if (!member)
		return;
	for (const RoleEntry& r : kRoles)
	{
		const bool on = member->GetRole(r.role);
		if (ImGui::MenuItem(r.label, nullptr, on))
		{
			char line[160];
			std::snprintf(line, sizeof(line), "/grouproles %s %s %d", on ? "unset" : "set",
				member->GetName(), static_cast<int>(r.role));
			DoCommand(line);
		}
	}
}

UnitWindow* CreateGroupWindow()
{
	return new GroupWindow();
}

} // namespace eqgfx::unitframes
