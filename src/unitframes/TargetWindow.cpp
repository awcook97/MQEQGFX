#include "UnitWindow.h"

#include "../MQEQGFX.h"

#include <imgui.h>

namespace eqgfx::unitframes {

namespace {

class TargetWindow : public UnitWindow
{
public:
	TargetWindow()
		: UnitWindow(Window::Target, "EQGFX Target", ImVec2(280, 78))
	{
		Config().buffs.show = true;
		FinishPanel();
	}

protected:
	bool HasContent() const override { return pTarget != nullptr; }

	void OnDraw(float width) override
	{
		PlayerClient* sp = pTarget;
		if (!sp)
			return;

		RowOptions opts = DefaultRowOptions();
		opts.pet = m_pet;
		opts.aggro = m_aggro;
		opts.aggroPct = MyAggroOn(sp);

		Row(sp, opts, width);

		if (m_targetOfTarget && pLocalPlayer)
		{
			PlayerClient* tot = GetSpawnByID(pLocalPlayer->TargetOfTarget);
			if (tot && tot != sp)
			{
				RowOptions sub = DefaultRowOptions();
				sub.buffs = false;
				sub.castBar = m_totCastBar;
				sub.nameOnly = !m_totCastBar;
				sub.height = m_totHeight;
				Row(tot, sub, width * m_totWidth);
			}
		}

		if (m_focusButton)
		{
			if (ImGui::SmallButton("Set focus"))
				SetSecondaryTarget(sp->SpawnID);
			NoteHotItem(Window::Target);
		}
	}

	void OnBuildPanel(SettingsPanel& p) override
	{
		p.Header("target");
		p.Bool("Pet", "its pet's health", &m_pet);
		p.Bool("Aggro", "my aggro on it", &m_aggro, nullptr, true);
		p.Bool("FocusButton", "'set focus' button", &m_focusButton, nullptr, true);
		p.Bool("TargetOfTarget", "its target, as a bar", &m_targetOfTarget,
			"name and health only, and only when there is one");
		p.Float("TotHeight", "its target's bar height", &m_totHeight, 8.0f, 32.0f,
			nullptr, "%.0f px");
		p.Float("TotWidth", "its target's bar width", &m_totWidth, 0.2f, 1.0f,
			"as a fraction of the window", "%.2f");
		p.Bool("TotCastBar", "cast bar on its target", &m_totCastBar);
	}

	void OnReset() override
	{
		m_targetOfTarget = true;
		m_pet = true;
		m_aggro = true;
		m_focusButton = false;
		m_totCastBar = false;
		m_totHeight = 12.0f;
		m_totWidth = 0.7f;
	}

private:
	static int MyAggroOn(const PlayerClient* sp)
	{
		if (!sp || !pAggroInfo || pAggroInfo->AggroTargetID != sp->SpawnID)
			return -1;
		return GroupAggroPct(0);
	}

	bool  m_targetOfTarget = true;
	bool  m_pet = true;
	bool  m_aggro = true;
	bool  m_focusButton = false;
	bool  m_totCastBar = false;
	float m_totHeight = 12.0f;
	float m_totWidth = 0.7f;
};

} // namespace

UnitWindow* CreateTargetWindow()
{
	return new TargetWindow();
}

} // namespace eqgfx::unitframes
