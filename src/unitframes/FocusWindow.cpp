#include "UnitWindow.h"

#include "../MQEQGFX.h"

#include <imgui.h>

namespace eqgfx::unitframes {

namespace {

class FocusWindow : public UnitWindow
{
public:
	FocusWindow()
		: UnitWindow(Window::Focus, "EQGFX Focus", ImVec2(280, 78))
	{
		Config().show = true;
		Config().buffs.show = true;
		FinishPanel();
	}

protected:
	bool HasContent() const override
	{
		const int id = GetSecondaryTarget();
		return id != 0 && GetSpawnByID(id) != nullptr;
	}

	void OnDraw(float width) override
	{
		const int focusId = GetSecondaryTarget();
		PlayerClient* sp = focusId ? GetSpawnByID(focusId) : nullptr;
		if (!sp)
			return;

		RowOptions opts = DefaultRowOptions();
		opts.pet = m_pet;

		Row(sp, opts, width);

		if (m_buttons)
		{
			if (ImGui::SmallButton("Target"))
				pTarget = sp;
			NoteHotItem(Window::Focus);
			ImGui::SameLine();
			if (ImGui::SmallButton("Clear"))
				SetSecondaryTarget(0);
			NoteHotItem(Window::Focus);
		}
	}

	void OnBuildPanel(SettingsPanel& p) override
	{
		p.Header("focus");
		p.Bool("Pet", "its pet's health", &m_pet);
		p.Bool("Buttons", "target / clear buttons", &m_buttons, nullptr, true);
	}

	void OnDrawOptions() override
	{
		UnitOptions o = GetUnitOptions();
		if (ImGui::Checkbox("ring under it in the world", &o.focusMarker))
			SetUnitOptions(o);
	}

	void OnReset() override
	{
		m_pet = true;
		m_buttons = true;
	}

private:
	bool  m_pet = true;
	bool  m_buttons = true;
};

} // namespace

UnitWindow* CreateFocusWindow()
{
	return new FocusWindow();
}

} // namespace eqgfx::unitframes
