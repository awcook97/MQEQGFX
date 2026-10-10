#pragma once

#include "UnitFrames.h"

namespace eqgfx::unitframes {

class UnitWindow
{
public:
	virtual ~UnitWindow() = default;

	UnitWindow(const UnitWindow&) = delete;
	UnitWindow& operator=(const UnitWindow&) = delete;

	Window      Id() const { return m_id; }
	const char* Name() const { return WindowName(m_id); }
	const char* Title() const { return m_title; }

	WindowSettings&       Config()       { return m_config; }
	const WindowSettings& Config() const { return m_config; }

	bool Shown() const  { return m_config.show; }
	void Show(bool on)  { m_config.show = on; }

	bool OptionsShown() const   { return m_optionsOpen; }
	void ShowOptions(bool on)   { m_optionsOpen = on; }

	void Draw();
	void DrawOptions();

	void Pulse() { OnPulse(); }

	void Persist(bool saving);

	void ResetSettings();

protected:
	UnitWindow(Window id, const char* title, const ImVec2& defaultSize,
		ImGuiWindowFlags flags = 0);

	virtual void OnDraw(float width) = 0;

	virtual void OnBuildPanel(SettingsPanel& panel) { (void)panel; }

	virtual void OnDrawOptions() {}

	virtual void OnRowMenu(PlayerClient* sp) { (void)sp; }

	virtual void OnPulse() {}

	virtual void OnPersist(bool saving) { (void)saving; }

	virtual void OnReset() {}

	void PersistBool(const char* key, bool& value, bool saving);
	void PersistInt(const char* key, int& value, bool saving);
	void PersistString(const char* key, char* value, size_t size, bool saving);

	RowResult Row(PlayerClient* sp, const RowOptions& opts, float width);

	RowOptions DefaultRowOptions() const;

	virtual bool HasContent() const { return true; }

	SettingsPanel& Panel() { return m_panel; }

	void FinishPanel();

	bool MouseIsHot() const;

private:
	Window           m_id;
	const char*      m_title;
	ImGuiWindowFlags m_flags;
	WindowSettings   m_config;
	WindowSettings   m_defaults;
	SettingsPanel    m_panel;
	bool             m_frameOpen = true;
	bool             m_optionsOpen = false;
	char             m_filter[64] = { 0 };

	ImVec2           m_lastPos{ 0.0f, 0.0f };
	ImVec2           m_lastSize{ 0.0f, 0.0f };
	float            m_appliedW = -1.0f;
	float            m_appliedH = -1.0f;
};

UnitWindow& WindowFor(Window id);
WindowSettings& SettingsFor(Window id);

UnitWindow* CreatePlayerWindow();
UnitWindow* CreateTargetWindow();
UnitWindow* CreateFocusWindow();
UnitWindow* CreateGroupWindow();
UnitWindow* CreateRaidWindow();
UnitWindow* CreatePetWindow();
UnitWindow* CreateXTargetWindow();

} // namespace eqgfx::unitframes
