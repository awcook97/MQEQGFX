#include "NativeUi.h"
#include "Suppress.h"

#include "../SettingsFile.h"
#include "../MQEQGFX.h"

#include <imgui.h>

namespace eqgfx::ui {

namespace {

constexpr const char* kSection = "UI";

bool     s_loaded = false;
bool     s_dirty = false;
uint32_t s_changedAt = 0;

void Write()
{
	std::string list;
	for (const SkinnedWindow& w : SkinnedWindows())
	{
		if (!w.replace)
			continue;
		if (!list.empty())
			list += ',';
		list += w.name;
	}
	settings::Global().Set(kSection, "Replace", list);
}

const char* SkinName(Skin skin)
{
	switch (skin)
	{
	case Skin::Hide:    return "unit frames";
	case Skin::Generic: return "redrawn";
	case Skin::Gems:    return "gem bar";
	case Skin::Hotbar:  return "hot bar";
	case Skin::SpellBook: return "spell book";
	case Skin::Actions: return "actions";
	case Skin::CombatAbilities: return "combat abilities";
	case Skin::Inventory: return "inventory";
	case Skin::Chat: return "chat";
	case Skin::AltAbilities: return "alternate abilities";
	case Skin::Bandolier: return "bandolier";
	case Skin::KeyRing: return "key rings";
	case Skin::Mercenary: return "mercenary";
	case Skin::Bags: return "bags";
	}
	return "";
}

void DrawWindowRows(bool debug)
{
	const int columns = debug ? 5 : 2;
	if (!ImGui::BeginTable("##nativeui", columns, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg))
		return;

	if (debug)
	{
		ImGui::TableSetupColumn("window");
		ImGui::TableSetupColumn("ours");
		ImGui::TableSetupColumn("state");
		ImGui::TableSetupColumn("controls");
		ImGui::TableSetupColumn("us");
		ImGui::TableHeadersRow();
	}

	int i = 0;
	for (SkinnedWindow& w : SkinnedWindows())
	{
		ImGui::PushID(i++);
		ImGui::TableNextRow();

		if (debug)
		{
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(w.name.c_str());
			ImGui::TableNextColumn();
			if (ImGui::Checkbox("##replace", &w.replace))
				MarkUiSettingsDirty();
			ImGui::TableNextColumn();
			if (!w.replace)
				ImGui::TextDisabled("stock");
			else if (!w.wnd)
				ImGui::TextDisabled("not loaded");
			else if (!w.wnd->IsVisible())
				ImGui::TextDisabled("closed");
			else
				ImGui::TextUnformatted("drawn");
			ImGui::TableNextColumn();
			ImGui::Text("%d", w.wnd ? w.controls : 0);
			ImGui::TableNextColumn();
			ImGui::Text("%.1f", w.wnd ? w.drawUs : 0.0f);
		}
		else
		{
			ImGui::TableNextColumn();
			if (ImGui::Checkbox(w.name.c_str(), &w.replace))
				MarkUiSettingsDirty();
			ImGui::TableNextColumn();
			if (w.replace && !w.wnd)
				ImGui::TextDisabled("not loaded");
			else
				ImGui::TextDisabled("%s", SkinName(w.skin));
		}

		ImGui::PopID();
	}
	ImGui::EndTable();
}

void SetAll(bool on)
{
	for (SkinnedWindow& w : SkinnedWindows())
		w.replace = on;
	MarkUiSettingsDirty();
}

} // namespace

void LoadUiSettings()
{
	const std::string list = settings::Global().GetString(kSection, "Replace", "");
	for (const std::string& name : split(list, ','))
		if (SkinnedWindow* w = TryAddSkinned(name))
			w->replace = true;
	s_loaded = true;
	s_dirty = false;
}

void MarkUiSettingsDirty()
{
	s_dirty = true;
	s_changedAt = static_cast<uint32_t>(EQGetTime());
}

void PulseUiSettings()
{
	if (s_loaded && s_dirty && static_cast<uint32_t>(EQGetTime()) - s_changedAt >= 1500)
	{
		s_dirty = false;
		Write();
	}
}

void FlushUiSettings()
{
	if (s_loaded && s_dirty)
		Write();
	s_dirty = false;
}

void DrawNativeUiOptions()
{
	ImGui::TextWrapped("Checked windows are hidden. The unit frames stand in for the player, target, group, "
		"pet, raid, buff and casting windows; spell gems, hot bars and the spell book get windows of our own "
		"that act directly, laid out and stored per class; anything else is redrawn in place and still "
		"clicks through to the game.");
	if (ImGui::Button("draw every window"))
		SetAll(true);
	ImGui::SameLine();
	if (ImGui::Button("restore every stock window"))
		SetAll(false);
	DrawWindowRows(false);
}

void DrawNativeUiDebug()
{
	UiDebug& d = Debug();
	ImGui::Checkbox("EQ keeps drawing", &d.keepNative);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("both draw: check that ours lines up with the stock window");
	ImGui::SameLine();
	ImGui::Checkbox("control outlines", &d.outlines);

	float total = 0.0f;
	for (const SkinnedWindow& w : SkinnedWindows())
		if (w.wnd)
			total += w.drawUs;
	ImGui::Text("window replacement %s, %d windows hidden, %.1f us to draw ours, client draws last pulse: %u",
		DrawHookInstalled() ? "active" : "inactive", HiddenCount(), total, EqDrawsLastPulse());

	DrawWindowRows(true);
}

} // namespace eqgfx::ui
