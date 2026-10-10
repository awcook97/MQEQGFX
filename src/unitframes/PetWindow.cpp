#include "UnitWindow.h"

#include "../MQEQGFX.h"

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace eqgfx::unitframes {

namespace {

constexpr int kMaxPetButtons = 16;

struct PetButton
{
	char label[24] = { 0 };
	char command[96] = { 0 };
	char toggle[96] = { 0 };
	bool on = false;
};

struct PetPreset
{
	const char* label;
	const char* command;
	const char* toggle;
};

constexpr PetPreset kPresets[] = {
	{ "Attack",  "/pet attack",      ""                   },
	{ "Back",    "/pet back off",    ""                   },
	{ "GHold",   "/pet ghold",       "${Pet.GHold}"       },
	{ "Hold",    "/pet hold",        "${Pet.Hold}"        },
	{ "Follow",  "/pet follow",      ""                   },
	{ "Guard",   "/pet guard",       ""                   },
	{ "Sit",     "/pet sit",         "${Pet.Sitting}"     },
	{ "Taunt",   "/pet taunt",       "${Pet.Taunt}"       },
	{ "Focus",   "/pet focus",       "${Pet.Focus}"       },
	{ "SpellHold","/pet no cast",    "${Pet.SpellHold}"   },
	{ "Stop",    "/pet stop",        ""                   },
	{ "Regroup", "/pet regroup",     "${Pet.ReGroup}"     },
	{ "Swarm",   "/pet swarmburn",   ""                   },
	{ "Leave",   "/pet get lost",    ""                   },
};

void Assign(char* dst, size_t size, const char* src)
{
	std::strncpy(dst, src ? src : "", size - 1);
	dst[size - 1] = 0;
}

bool EvaluateToggle(const char* expr)
{
	if (!expr || !*expr)
		return false;

	char buf[MAX_STRING] = { 0 };
	if (std::strstr(expr, "${"))
		std::snprintf(buf, sizeof(buf), "%s", expr);
	else
		std::snprintf(buf, sizeof(buf), "${%s}", expr);

	ParseMacroData(buf, sizeof(buf));
	return ci_equals(buf, "TRUE") || ci_equals(buf, "1");
}

class PetWindow : public UnitWindow
{
public:
	PetWindow()
		: UnitWindow(Window::Pet, "EQGFX Pet", ImVec2(260, 86))
	{
		ResetButtons();
		FinishPanel();
	}

protected:
	bool HasContent() const override { return Pet() != nullptr; }

	void OnDraw(float width) override
	{
		PlayerClient* pet = Pet();
		if (!pet)
			return;

		RowOptions opts = DefaultRowOptions();
		Row(pet, opts, width);

		if (m_showTarget)
		{
			PlayerClient* petTarget = ValidateSpawn(pet->WhoFollowing);
			if (petTarget && petTarget != pet)
			{
				RowOptions sub = DefaultRowOptions();
				sub.buffs = false;
				sub.castBar = false;
				sub.nameOnly = true;
				sub.height = m_targetHeight;
				Row(petTarget, sub, width * 0.7f);
			}
		}

		if (m_showButtons)
			DrawButtons(width);
	}

	void OnBuildPanel(SettingsPanel& p) override
	{
		p.Header("pet");
		p.Bool("PetTarget", "what it is attacking, as a bar", &m_showTarget);
		p.Float("PetTargetHeight", "that bar's height", &m_targetHeight, 8.0f, 32.0f,
			nullptr, "%.0f px");
		p.Bool("Buttons", "command buttons", &m_showButtons);
		p.Int("ButtonsPerRow", "buttons per row", &m_perRow, 1, 8);
		p.Float("ButtonHeight", "button height", &m_buttonHeight, 12.0f, 40.0f,
			nullptr, "%.0f px");
		p.Bool("ButtonStretch", "stretch buttons to the window", &m_stretch);
		p.Color("ButtonOnColor", "toggled-on colour", &m_onColor);
		p.Color("ButtonOffColor", "toggled-off colour", &m_offColor, nullptr, true);
	}

	void OnDrawOptions() override
	{
		ImGui::SeparatorText("pet buttons");
		ImGui::TextDisabled("display, command, and an optional expression that makes it a toggle");

		int remove = -1;
		for (int i = 0; i < m_count; ++i)
		{
			PetButton& b = m_buttons[i];
			ImGui::PushID(i);

			ImGui::SetNextItemWidth(90.0f);
			bool dirty = ImGui::InputText("##label", b.label, sizeof(b.label));
			ImGui::SameLine();
			ImGui::SetNextItemWidth(150.0f);
			dirty |= ImGui::InputTextWithHint("##cmd", "/pet attack", b.command, sizeof(b.command));
			ImGui::SameLine();
			ImGui::SetNextItemWidth(140.0f);
			dirty |= ImGui::InputTextWithHint("##toggle", "${Pet.GHold}", b.toggle,
				sizeof(b.toggle));

			ImGui::SameLine();
			if (ImGui::SmallButton("^") && i > 0)
			{
				std::swap(m_buttons[i], m_buttons[i - 1]);
				dirty = true;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("v") && i + 1 < m_count)
			{
				std::swap(m_buttons[i], m_buttons[i + 1]);
				dirty = true;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("x"))
				remove = i;

			if (dirty)
				MarkSettingsDirty();

			ImGui::PopID();
		}

		if (remove >= 0)
		{
			for (int i = remove; i + 1 < m_count; ++i)
				m_buttons[i] = m_buttons[i + 1];
			--m_count;
			MarkSettingsDirty();
		}

		if (m_count < kMaxPetButtons)
		{
			if (ImGui::Button("add blank"))
			{
				m_buttons[m_count++] = PetButton();
				MarkSettingsDirty();
			}
			ImGui::SameLine();
			if (ImGui::BeginCombo("##preset", "add a known one"))
			{
				for (const PetPreset& preset : kPresets)
				{
					if (ImGui::Selectable(preset.label) && m_count < kMaxPetButtons)
					{
						PetButton& b = m_buttons[m_count++];
						b = PetButton();
						Assign(b.label, sizeof(b.label), preset.label);
						Assign(b.command, sizeof(b.command), preset.command);
						Assign(b.toggle, sizeof(b.toggle), preset.toggle);
						MarkSettingsDirty();
					}
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
		}

		if (ImGui::Button("default buttons"))
		{
			ResetButtons();
			MarkSettingsDirty();
		}
	}

	void OnPulse() override
	{
		if (m_pending[0])
		{
			char cmd[MAX_STRING];
			std::strncpy(cmd, m_pending, sizeof(cmd) - 1);
			cmd[sizeof(cmd) - 1] = 0;
			m_pending[0] = 0;
			EzCommand(cmd);
		}

		if (!Shown() || !m_showButtons || !ShouldDraw() || !Pet())
			return;

		const uint32_t now = static_cast<uint32_t>(EQGetTime());
		if (now - m_lastEval < 250)
			return;
		m_lastEval = now;

		for (int i = 0; i < m_count; ++i)
			m_buttons[i].on = EvaluateToggle(m_buttons[i].toggle);
	}

	void OnPersist(bool saving) override
	{
		PersistInt("ButtonCount", m_count, saving);
		m_count = std::clamp(m_count, 0, kMaxPetButtons);

		char key[32];
		for (int i = 0; i < m_count; ++i)
		{
			std::snprintf(key, sizeof(key), "Button%dLabel", i);
			PersistString(key, m_buttons[i].label, sizeof(m_buttons[i].label), saving);
			std::snprintf(key, sizeof(key), "Button%dCmd", i);
			PersistString(key, m_buttons[i].command, sizeof(m_buttons[i].command), saving);
			std::snprintf(key, sizeof(key), "Button%dToggle", i);
			PersistString(key, m_buttons[i].toggle, sizeof(m_buttons[i].toggle), saving);
		}
	}

	void OnReset() override
	{
		m_showTarget = true;
		m_showButtons = true;
		m_perRow = 3;
		m_buttonHeight = 20.0f;
		m_stretch = true;
		m_targetHeight = 12.0f;
		ResetButtons();
	}

private:
	static PlayerClient* Pet()
	{
		PlayerClient* me = pLocalPlayer.get();
		return (me && me->PetID > 0) ? GetSpawnByID(me->PetID) : nullptr;
	}

	void ResetButtons()
	{
		m_count = 0;
		for (const char* want : { "Attack", "Back", "GHold" })
		{
			for (const PetPreset& preset : kPresets)
			{
				if (!ci_equals(preset.label, want))
					continue;
				PetButton& b = m_buttons[m_count++];
				b = PetButton();
				Assign(b.label, sizeof(b.label), preset.label);
				Assign(b.command, sizeof(b.command), preset.command);
				Assign(b.toggle, sizeof(b.toggle), preset.toggle);
				break;
			}
		}
	}

	void DrawButtons(float width)
	{
		if (m_count == 0)
			return;

		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float w = m_stretch
			? std::max(20.0f, (width - spacing * (m_perRow - 1)) / m_perRow)
			: 0.0f;

		for (int i = 0; i < m_count; ++i)
		{
			PetButton& b = m_buttons[i];
			if (!b.label[0] && !b.command[0])
				continue;

			if (i % m_perRow != 0)
				ImGui::SameLine();

			ImGui::PushID(i);

			const bool toggled = b.toggle[0] != 0;
			if (toggled)
			{
				const ImVec4& c = b.on ? m_onColor : m_offColor;
				ImGui::PushStyleColor(ImGuiCol_Button, c);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
					ImVec4(c.x * 1.2f, c.y * 1.2f, c.z * 1.2f, c.w));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive,
					ImVec4(c.x * 1.4f, c.y * 1.4f, c.z * 1.4f, c.w));
			}

			if (ImGui::Button(b.label[0] ? b.label : "?", ImVec2(w, m_buttonHeight))
				&& b.command[0])
			{
				Assign(m_pending, sizeof(m_pending), b.command);
			}

			NoteHotItem(Window::Pet);

			if (toggled)
				ImGui::PopStyleColor(3);

			if (ImGui::IsItemHovered())
			{
				if (toggled)
					ImGui::SetTooltip("%s\n%s  -  %s", b.command, b.toggle,
						b.on ? "on" : "off");
				else
					ImGui::SetTooltip("%s", b.command);
			}

			ImGui::PopID();
		}
	}

	bool      m_showTarget = true;
	bool      m_showButtons = true;
	int       m_perRow = 3;
	float     m_buttonHeight = 20.0f;
	float     m_targetHeight = 12.0f;
	bool      m_stretch = true;
	ImVec4    m_onColor { 0.20f, 0.55f, 0.25f, 1.00f };
	ImVec4    m_offColor{ 0.22f, 0.22f, 0.26f, 1.00f };

	PetButton m_buttons[kMaxPetButtons];
	int       m_count = 0;

	char      m_pending[MAX_STRING] = { 0 };
	uint32_t  m_lastEval = 0;
};

} // namespace

UnitWindow* CreatePetWindow()
{
	return new PetWindow();
}

} // namespace eqgfx::unitframes
