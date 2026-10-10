#include "UnitWindow.h"

#include "../MQEQGFX.h"

#include <imgui.h>

namespace eqgfx::unitframes {

namespace {

class PlayerWindow : public UnitWindow
{
public:
	PlayerWindow()
		: UnitWindow(Window::Player, "EQGFX Player", ImVec2(280, 64))
	{
		FinishPanel();
	}

protected:
	bool HasContent() const override { return pLocalPlayer.get() != nullptr; }

	void OnDraw(float width) override
	{
		PlayerClient* me = pLocalPlayer.get();
		if (!me)
			return;

		RowOptions opts = DefaultRowOptions();
		opts.mana = m_mana;
		opts.endurance = m_endurance;
		opts.pet = m_pet;
		opts.aggro = m_aggro;
		opts.aggroPct = GroupAggroPct(0);

		Row(me, opts, width);
	}

	void OnBuildPanel(SettingsPanel& p) override
	{
		p.Header("player");
		p.Bool("Mana", "mana bar", &m_mana);
		p.Bool("Endurance", "endurance bar", &m_endurance, nullptr, true);
		p.Bool("PetBar", "pet health bar", &m_pet, nullptr, true);
		p.Bool("Aggro", "aggro percent", &m_aggro, nullptr, true);
	}

	void OnRowMenu(PlayerClient* sp) override
	{
		bool changed = false;
		changed |= ImGui::MenuItem("Pet Display", nullptr, &m_pet);
		changed |= ImGui::MenuItem("Mana Display", nullptr, &m_mana);
		changed |= ImGui::MenuItem("Endurance Display", nullptr, &m_endurance);
		changed |= ImGui::MenuItem("Show Player Aggro Percent", nullptr, &m_aggro);
		if (changed)
			MarkSettingsDirty();

		if (CGroupMember* me = GroupMemberFor(sp))
		{
			ImGui::Separator();
			if (ImGui::BeginMenu("Roles"))
			{
				GroupRoleItems(me);
				ImGui::EndMenu();
			}
		}
	}

	void OnReset() override
	{
		m_mana = m_endurance = m_pet = m_aggro = true;
	}

private:
	bool m_mana = true;
	bool m_endurance = true;
	bool m_pet = true;
	bool m_aggro = true;
};

} // namespace

UnitWindow* CreatePlayerWindow()
{
	return new PlayerWindow();
}

} // namespace eqgfx::unitframes
