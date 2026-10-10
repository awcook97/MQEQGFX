#include "UnitWindow.h"

#include "../MQEQGFX.h"
#include "../ui/Layouts.h"
#include "../ui/NativeUi.h"

#include <imgui.h>

namespace eqgfx::unitframes {

namespace {

bool s_shown = false;

void DrawWindowList()
{
	bool dirty = false;

	bool onlyFg = OnlyWhenForeground();
	if (ImGui::Checkbox("draw only while EverQuest has focus", &onlyFg))
		SetOnlyWhenForeground(onlyFg);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("no frames, rows or buff lists while the game is in the background");

	ImGui::Separator();

	if (ImGui::BeginTable("##windows", 6, ImGuiTableFlags_SizingFixedFit))
	{
		for (int i = 0; i < kWindowCount; ++i)
		{
			UnitWindow& window = WindowFor(static_cast<Window>(i));
			WindowSettings& cfg = window.Config();

			ImGui::TableNextRow();
			ImGui::PushID(i);

			ImGui::TableNextColumn();
			bool shown = window.Shown();
			if (ImGui::Checkbox(window.Name(), &shown))
			{
				window.Show(shown);
				dirty = true;
			}

			ImGui::TableNextColumn();
			dirty |= ImGui::Checkbox("locked", &cfg.locked);

			ImGui::TableNextColumn();
			dirty |= ImGui::Checkbox("click through", &cfg.clickThrough);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("the frame only takes the mouse on or near a bar");

			ImGui::TableNextColumn();
			dirty |= ImGui::Checkbox("shrink to fit", &cfg.autoResize);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("the frame is only as big as what it drew");

			ImGui::TableNextColumn();
			if (cfg.autoResize)
				ImGui::TextDisabled("auto");
			else
				ImGui::TextDisabled("%.0fx%.0f", cfg.width, cfg.height);

			ImGui::TableNextColumn();
			if (ImGui::SmallButton("options"))
				window.ShowOptions(true);

			ImGui::PopID();
		}
		ImGui::EndTable();
	}

	if (ImGui::Button("shrink every window to fit"))
	{
		for (int i = 0; i < kWindowCount; ++i)
			WindowFor(static_cast<Window>(i)).Config().autoResize = true;
		dirty = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("toggle hit-rect overlay"))
	{
		for (int i = 0; i < kWindowCount; ++i)
		{
			WindowSettings& c = WindowFor(static_cast<Window>(i)).Config();
			c.showHitRects = !c.showHitRects;
		}
		dirty = true;
	}

	if (ImGui::Button("lock every window"))
	{
		for (int i = 0; i < kWindowCount; ++i)
			WindowFor(static_cast<Window>(i)).Config().locked = true;
		dirty = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("unlock every window"))
	{
		for (int i = 0; i < kWindowCount; ++i)
			WindowFor(static_cast<Window>(i)).Config().locked = false;
		dirty = true;
	}

	if (dirty)
		MarkSettingsDirty();
}

void DrawTargeting()
{
	UnitOptions o = GetUnitOptions();
	bool changed = false;

	changed |= ImGui::Checkbox("world hover", &o.worldHover);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("a spawn under the mouse in the 3D world is what /eqgfx cast aims at");

	ImGui::SameLine();
	changed |= ImGui::Checkbox("focus marker", &o.focusMarker);

	changed |= ImGui::Checkbox("fall back to focus, then target", &o.hoverFallback);

	if (changed)
		SetUnitOptions(o);
}

void DrawWorldEffects()
{
	GfxOptions o = GetGfxOptions();
	bool changed = false;

	changed |= ImGui::Checkbox("class animations", &o.classFx);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("the passive per-class effect under every nearby spawn");

	ImGui::SameLine();
	changed |= ImGui::Checkbox("on PCs", &o.classFxPCs);
	ImGui::SameLine();
	changed |= ImGui::Checkbox("on NPCs", &o.classFxNPCs);

	ImGui::SetNextItemWidth(160.0f);
	changed |= ImGui::SliderFloat("radius", &o.classFxRadius, 20.0f, 600.0f, "%.0f");
	ImGui::SetNextItemWidth(160.0f);
	changed |= ImGui::SliderInt("max spawns", &o.classFxMaxSpawns, 1, 200);
	ImGui::SeparatorText("class animations look");
	ImGui::PushID("classfx");
	changed |= AlwaysOnStyleOptions(o.classFxStyle);
	ImGui::PopID();
	ImGui::SetNextItemWidth(160.0f);
	changed |= ImGui::InputInt("slot mask", &o.classFxSlotMask, 1, 1);

	changed |= ImGui::Checkbox("gpu effects", &o.gpuEffects);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("the whole effect pass: class animations, focus ring, Lua shapes");
	ImGui::SameLine();
	changed |= ImGui::Checkbox("test ring", &o.testRing);
	ImGui::SameLine();
	changed |= ImGui::Checkbox("damage debug", &o.damageDebug);

	if (changed)
		SetGfxOptions(o);

	ImGui::Checkbox("appearance capture logging", &s_captureVerbose);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("chat-log every material swap the appearance capture records");
}

void DrawPerformance()
{
	const TextBatchStats t = GetTextBatchStats();

	bool tags = t.nametagsOn;
	if (ImGui::Checkbox("nametag batching", &tags))
		SetNametagBatchEnabled(tags);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("draw every nametag from one buffer instead of one draw per tag");
	ImGui::SameLine();
	bool ui = t.uiOn;
	if (ImGui::Checkbox("ui text batching", &ui))
		SetUiTextBatchEnabled(ui);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("draw runs of adjacent UI strings together");

	if (!t.nametagsInstalled)
		ImGui::TextDisabled("text batching unavailable: %s", *t.failReason ? t.failReason : "not installed yet");

	AnimationLodOptions lod = GetAnimationLodOptions();
	bool lodChanged = ImGui::Checkbox("animation lod", &lod.enabled);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("animate actors past the distance less often; you, your target, pet and group always animate");
	ImGui::SetNextItemWidth(160.0f);
	lodChanged |= ImGui::SliderFloat("lod distance", &lod.distance, 10.0f, 500.0f, "%.0f");
	ImGui::SetNextItemWidth(160.0f);
	lodChanged |= ImGui::SliderInt("lod max interval", &lod.maxInterval, 1, 30);
	if (lodChanged)
		SetAnimationLodOptions(lod);
	if (!AnimationLodInstalled())
		ImGui::TextDisabled("animation lod: unavailable on this client");

	bool particles = ParticleGpuEnabled();
	if (ImGui::Checkbox("gpu particles", &particles))
		SetParticleGpuEnabled(particles);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("EQ keeps simulating its particles; placing and drawing them moves to the gpu");
	int budget = GetParticleGpuBudget();
	ImGui::SetNextItemWidth(160.0f);
	if (ImGui::SliderInt("particle budget", &budget, kMinParticleBudget, 262144, "%d", ImGuiSliderFlags_Logarithmic))
		SetParticleGpuBudget(budget);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("above this many particles EQ draws them itself for a few seconds");
	ImGui::TextDisabled("particle buffers: %.1f MB", static_cast<double>(ParticleGpuBufferBytes()) / (1024.0 * 1024.0));
	if (*ParticleGpuFailReason())
		ImGui::TextDisabled("gpu particles: %s", ParticleGpuFailReason());

	if (ImGui::Button("drop glyph caches"))
		ResetTextBatchCaches();
	ImGui::SameLine();
	if (ImGui::Button("debug window"))
		ShowDebugWindow(!DebugWindowShown());
}

} // namespace

void ShowPluginOptions(bool on) { s_shown = on; }
bool PluginOptionsShown()       { return s_shown; }

void DrawPluginOptions()
{
	bool open = true;
	ImGui::SetNextWindowSize(ImVec2(400, 0), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("EQGFX Options", &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::CollapsingHeader("Windows", ImGuiTreeNodeFlags_DefaultOpen))
			DrawWindowList();

		if (ImGui::CollapsingHeader("Native UI"))
			eqgfx::ui::DrawNativeUiOptions();

		if (ImGui::CollapsingHeader("UI layouts"))
			eqgfx::ui::DrawLayoutOptions();

		if (ImGui::CollapsingHeader("Attack indicator"))
		{
			ImGui::PushID("attack");
			DrawAttackOptions();
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Target ring"))
		{
			ImGui::PushID("targetring");
			DrawTargetRingOptions();
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Targeting", ImGuiTreeNodeFlags_DefaultOpen))
			DrawTargeting();

		if (ImGui::CollapsingHeader("World effects"))
			DrawWorldEffects();

		if (ImGui::CollapsingHeader("Performance"))
			DrawPerformance();

		if (ImGui::CollapsingHeader("Spell animations"))
			DrawSpellFxOptions();
	}
	ImGui::End();

	if (!open)
		s_shown = false;
}

} // namespace eqgfx::unitframes
