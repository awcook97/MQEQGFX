#include "UnitWindow.h"
#include "../ui/ClassProfile.h"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>

namespace eqgfx::unitframes {

namespace {

std::array<std::unique_ptr<UnitWindow>, kWindowCount> s_windows;

void EnsureWindows()
{
	if (s_windows[0])
		return;

	s_windows[static_cast<int>(Window::Player)].reset(CreatePlayerWindow());
	s_windows[static_cast<int>(Window::Target)].reset(CreateTargetWindow());
	s_windows[static_cast<int>(Window::Focus)].reset(CreateFocusWindow());
	s_windows[static_cast<int>(Window::Group)].reset(CreateGroupWindow());
	s_windows[static_cast<int>(Window::Raid)].reset(CreateRaidWindow());
	s_windows[static_cast<int>(Window::Pet)].reset(CreatePetWindow());
	s_windows[static_cast<int>(Window::XTarget)].reset(CreateXTargetWindow());
}

bool Inside(const ImVec4& r, const ImVec2& p, float pad)
{
	return p.x >= r.x - pad && p.x <= r.z + pad && p.y >= r.y - pad && p.y <= r.w + pad;
}

} // namespace

UnitWindow& WindowFor(Window id)
{
	EnsureWindows();

	const int i = static_cast<int>(id);
	return *s_windows[(i >= 0 && i < kWindowCount) ? i : 0];
}

WindowSettings& SettingsFor(Window id)
{
	return WindowFor(id).Config();
}

void ShutdownWindows()
{
	for (std::unique_ptr<UnitWindow>& w : s_windows)
		w.reset();
}

UnitWindow::UnitWindow(Window id, const char* title, const ImVec2& defaultSize,
	ImGuiWindowFlags flags)
	: m_id(id)
	, m_title(title)
	, m_flags(flags)
{
	m_config.width = defaultSize.x;
	m_config.height = defaultSize.y;
}

void UnitWindow::FinishPanel()
{
	BuildSharedPanel(m_panel, m_config);
	OnBuildPanel(m_panel);
	m_defaults = m_config;
}

void UnitWindow::ResetSettings()
{
	Rotation keep[kActionCount];
	for (int i = 0; i < kActionCount; ++i)
		keep[i] = m_config.binds[i];

	m_config = m_defaults;
	for (int i = 0; i < kActionCount; ++i)
		m_config.binds[i] = keep[i];

	m_appliedW = m_appliedH = -1.0f;
	OnReset();
	MarkSettingsDirty();
}

bool UnitWindow::MouseIsHot() const
{
	if (m_optionsOpen || MenuOpenWindow() == static_cast<int>(m_id))
		return true;

	const ImGuiIO& io = ImGui::GetIO();
	const ImVec2 mouse = MousePos();

	if (m_lastSize.x > 0.0f)
	{
		const float slack = m_config.hitPad + 2.0f;
		if (mouse.x < m_lastPos.x - slack || mouse.y < m_lastPos.y - slack
			|| mouse.x > m_lastPos.x + m_lastSize.x + slack
			|| mouse.y > m_lastPos.y + m_lastSize.y + slack)
		{
			return false;
		}
	}

	for (const ImVec4& r : RowRects(m_id))
	{
		if (Inside(r, mouse, m_config.hitPad))
			return true;
	}

	if (!m_config.locked && m_lastSize.x > 0.0f)
	{
		if (!m_config.noTitleBar)
		{
			const ImVec4 bar(m_lastPos.x, m_lastPos.y, m_lastPos.x + m_lastSize.x,
				m_lastPos.y + ImGui::GetFrameHeight());
			if (Inside(bar, mouse, 2.0f))
				return true;
		}

		if (io.KeyAlt)
		{
			const ImVec4 all(m_lastPos.x, m_lastPos.y, m_lastPos.x + m_lastSize.x,
				m_lastPos.y + m_lastSize.y);
			if (Inside(all, mouse, 0.0f))
				return true;
		}
	}

	return false;
}

void UnitWindow::Draw()
{
	if (m_config.hideWhenEmpty && !HasContent())
	{
		BeginRowCapture(m_id);
		m_lastSize = ImVec2(0.0f, 0.0f);
		return;
	}

	m_frameOpen = true;

	ImGuiWindowFlags flags = m_flags;
	if (!m_config.scrollbar)
		flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (m_config.locked)
		flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
	else if (m_config.noTitleBar)
		flags |= ImGuiWindowFlags_NoTitleBar;
	if (m_config.noBackground)
		flags |= ImGuiWindowFlags_NoBackground;

	const bool hot = !m_config.clickThrough || MouseIsHot();
	if (!hot)
		flags |= ImGuiWindowFlags_NoInputs;

	if (m_config.autoResize)
	{
		flags |= ImGuiWindowFlags_AlwaysAutoResize;
		m_appliedW = m_appliedH = -1.0f;
	}
	else
	{
		const ImVec2 want(std::max(m_config.width, 40.0f), std::max(m_config.height, 24.0f));
		if (std::fabs(want.x - m_appliedW) > 0.5f || std::fabs(want.y - m_appliedH) > 0.5f)
		{
			ImGui::SetNextWindowSize(want, ImGuiCond_Always);
			m_appliedW = want.x;
			m_appliedH = want.y;
		}
		else
		{
			ImGui::SetNextWindowSize(want, ImGuiCond_FirstUseEver);
		}
	}

	ImGui::SetNextWindowBgAlpha(m_config.opacity);

	BeginRowCapture(m_id);

	ui::PlaceWindow(m_title);
	if (ImGui::Begin(m_title, m_config.locked ? nullptr : &m_frameOpen, flags))
	{
		OnDraw(m_config.autoResize ? std::max(m_config.width, 40.0f)
			: ImGui::GetContentRegionAvail().x);
	}

	ui::NoteWindow(m_title);
	m_lastPos = ImGui::GetWindowPos();

	if (!ImGui::IsWindowCollapsed())
	{
		m_lastSize = ImGui::GetWindowSize();

		if (!m_config.autoResize
			&& (std::fabs(m_lastSize.x - m_appliedW) > 0.5f
				|| std::fabs(m_lastSize.y - m_appliedH) > 0.5f))
		{
			m_config.width = m_lastSize.x;
			m_config.height = m_lastSize.y;
			m_appliedW = m_lastSize.x;
			m_appliedH = m_lastSize.y;
			MarkSettingsDirty();
		}
	}
	else
	{
		m_lastSize = ImVec2(ImGui::GetWindowSize().x, ImGui::GetFrameHeight());
	}

	ImGui::End();

	if (m_config.showHitRects)
	{
		DebugDrawHotRects(m_id, hot ? ImVec4(0.2f, 1.0f, 0.3f, 0.9f)
			: ImVec4(1.0f, 0.5f, 0.1f, 0.9f));
	}

	if (!m_frameOpen)
	{
		m_config.show = false;
		MarkSettingsDirty();
	}
}

void UnitWindow::DrawOptions()
{
	char title[96];
	std::snprintf(title, sizeof(title), "%s Options###%sOptions", m_title, Name());

	bool open = true;
	ImGui::SetNextWindowSize(ImVec2(430, 620), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(title, &open))
	{
		ImGui::SetNextItemWidth(-90.0f);
		ImGui::InputTextWithHint("##filter", "filter settings", m_filter, sizeof(m_filter));
		ImGui::SameLine();
		if (ImGui::Button("clear"))
			m_filter[0] = 0;

		ImGui::Separator();

		if (ImGui::BeginChild("##settings", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2.2f)))
		{
			if (m_panel.Draw(m_filter))
				MarkSettingsDirty();

			OnDrawOptions();
			DrawSharedOptions(m_id, m_filter);
		}
		ImGui::EndChild();

		ImGui::Separator();
		if (ImGui::Button("Plugin options..."))
			ShowPluginOptions(true);
		ImGui::SameLine();
		if (ImGui::Button("Reset this window"))
			ResetSettings();
		ImGui::SameLine();
		ImGui::TextDisabled("(%s)", Name());
	}
	ImGui::End();

	if (!open)
		m_optionsOpen = false;
}

RowResult UnitWindow::Row(PlayerClient* sp, const RowOptions& opts, float width)
{
	return DrawUnitRow(sp, m_id, opts, width, [this](PlayerClient* unit) {
		OnRowMenu(unit);
		ImGui::Separator();
		if (ImGui::MenuItem("Options..."))
			m_optionsOpen = true;
	});
}

void UnitWindow::Persist(bool saving)
{
	if (saving)
		m_panel.Save(Name());
	else
		m_panel.Load(Name());

	OnPersist(saving);
}

void UnitWindow::PersistBool(const char* key, bool& value, bool saving)
{
	char full[160];
	std::snprintf(full, sizeof(full), "%s%s", Name(), key);

	if (saving)
		SettingPutBool(full, value);
	else
		value = SettingGetBool(full, value);
}

void UnitWindow::PersistInt(const char* key, int& value, bool saving)
{
	char full[160];
	std::snprintf(full, sizeof(full), "%s%s", Name(), key);

	if (saving)
		SettingPutInt(full, value);
	else
		value = SettingGetInt(full, value);
}

void UnitWindow::PersistString(const char* key, char* value, size_t size, bool saving)
{
	char full[160];
	std::snprintf(full, sizeof(full), "%s%s", Name(), key);

	if (saving)
	{
		SettingPutString(full, value);
	}
	else
	{
		const std::string text = SettingGetString(full, value);
		std::strncpy(value, text.c_str(), size - 1);
		value[size - 1] = 0;
	}
}

RowOptions UnitWindow::DefaultRowOptions() const
{
	RowOptions opts;
	opts.buffs = m_config.buffs.show;
	opts.castBar = m_config.cast.show;
	return opts;
}

} // namespace eqgfx::unitframes
