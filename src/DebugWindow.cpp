#include "MQEQGFX.h"

#include "gpu/Glyphs.h"
#include "gpu/LineFX.h"
#include "ui/NativeUi.h"

#include <imgui.h>

#include <cstdio>
#include <cstring>

namespace {

bool s_shown = false;
int  s_spellInput = 0;
char s_spellName[64] = {};
bool s_onTarget = true;

constexpr const char* kFormNames[] = {
	"ring", "helix", "orbit", "polygon", "bloom", "spire", "lattice", "segment", "rect",
	"flame", "vines", "thorns", "glyph", "wings", "lightning", "bolt", "rain", "sparkle",
	"rune", "vortex", "dome", "crystal", "drain", "bubbles", "wind", "burst", "cracks",
};
static_assert(sizeof(kFormNames) / sizeof(kFormNames[0]) == eqgfx::gpu::FxForm_Count);

int Me()     { return pLocalPlayer ? pLocalPlayer->SpawnID : 0; }
int Target() { return s_onTarget && pTarget ? pTarget->SpawnID : Me(); }

void Status(const char* label, bool ok, const char* detail = nullptr)
{
	ImGui::TextColored(ok ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f) : ImVec4(1.0f, 0.4f, 0.4f, 1.0f), ok ? "on " : "off");
	ImGui::SameLine();
	ImGui::TextUnformatted(label);
	if (detail && *detail)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("(%s)", detail);
	}
}

void DrawPerformance()
{
	const FxStats fx = GetFxStats();
	const TextBatchStats t = GetTextBatchStats();
	ImGui::Text("client draws last pulse: %u", EqDrawsLastPulse());
	ImGui::Text("gpu effects: %u instances, %u segments", fx.instances, fx.segments);
	ImGui::Text("nametags: %u tags in %u draws, %u rebuilds", t.tags, t.tagDraws, t.tagRebuilds);
	ImGui::Text("ui text: %u merges planned, %u strings merged, %u draws saved, %u cached",
		t.uiMergesPlanned, t.uiMerged, t.uiDrawsSaved, t.uiCached);
	ImGui::Text("spell animations live: %u", SpellFxActiveCount());
	const ActorAnimationStats lod = GetActorAnimationStats();
	ImGui::Text("actors animated: %u (%u far, %u skipped)", lod.animated, lod.distant, lod.skipped);
	const ParticleGpuStats pg = GetParticleGpuStats();
	ImGui::Text("gpu particles: %u in %u emitters, %u draws, %u frames%s%s", pg.particles, pg.emitters, pg.draws,
		pg.gatedFrames, *pg.ungatedReason ? "; drawn by EQ: " : "", pg.ungatedReason);
}

void DrawTextBatching()
{
	const TextBatchStats t = GetTextBatchStats();

	bool tags = t.nametagsOn;
	if (ImGui::Checkbox("nametag batching", &tags))
		SetNametagBatchEnabled(tags);
	ImGui::SameLine();
	bool ui = t.uiOn;
	if (ImGui::Checkbox("ui text batching", &ui))
		SetUiTextBatchEnabled(ui);

	if (ImGui::Button("drop glyph caches"))
		ResetTextBatchCaches();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("every batched string and nametag rebuilds from the client's layout");
}

void DrawWorldEffects()
{
	GfxOptions o = GetGfxOptions();
	bool changed = false;
	changed |= ImGui::Checkbox("gpu effects", &o.gpuEffects);
	ImGui::SameLine();
	changed |= ImGui::Checkbox("class animations", &o.classFx);
	ImGui::SameLine();
	changed |= ImGui::Checkbox("test ring", &o.testRing);
	if (changed)
		SetGfxOptions(o);
}

void DrawSpellLooks()
{
	bool enabled = SpellFxEnabled();
	if (ImGui::Checkbox("spell animations", &enabled))
		SetSpellFxEnabled(enabled);
	ImGui::SameLine();
	ImGui::Checkbox("play at target", &s_onTarget);

	if (ImGui::Button("play every look"))
		for (int i = 0; i < SpellFxLookCount(); ++i)
			SpellFxPlayLook(i, Me(), Target(), i * 2.5f);
	ImGui::SameLine();
	if (ImGui::Button("glyph gallery"))
		SpellFxDemoGlyphs(Me());

	ImGui::SetNextItemWidth(120.0f);
	ImGui::InputInt("spell id", &s_spellInput);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(160.0f);
	ImGui::InputText("or name", s_spellName, sizeof(s_spellName));
	int spellId = s_spellInput;
	if (s_spellName[0])
		if (EQ_Spell* sp = GetSpellByName(s_spellName))
			spellId = sp->ID;
	if (spellId > 0)
	{
		ImGui::SameLine();
		if (ImGui::Button("cast"))
			SpellFxPlay(spellId, Me(), Target(), 1500);
		ImGui::TextDisabled("%d: %s", spellId, SpellFxDescribe(spellId).c_str());
	}

	if (ImGui::BeginTable("##looks", 4, ImGuiTableFlags_SizingStretchSame))
	{
		for (int i = 0; i < SpellFxLookCount(); ++i)
		{
			ImGui::TableNextColumn();
			ImGui::PushID(i);
			if (ImGui::SmallButton(SpellFxLookName(i)))
				SpellFxPlayLook(i, Me(), Target(), 0.0f);
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
}

void DrawClassLines()
{
	ImGui::Checkbox("play at target##lines", &s_onTarget);
	ImGui::SameLine();
	int level = SpellFxDemoLevel();
	ImGui::SetNextItemWidth(160.0f);
	if (ImGui::SliderInt("preview level", &level, 1, 125))
		SetSpellFxDemoLevel(level);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("previews animate as a spell of this level would");
	const char* cls = nullptr;
	bool open = false;
	for (int i = 0; i < SpellFxLineCount(); ++i)
	{
		if (!cls || std::strcmp(cls, SpellFxLineClass(i)) != 0)
		{
			if (open)
				ImGui::EndTable();
			cls = SpellFxLineClass(i);
			ImGui::SeparatorText(cls);
			ImGui::PushID(cls);
			if (ImGui::SmallButton("play all"))
				for (int j = i, k = 0; j < SpellFxLineCount() && std::strcmp(cls, SpellFxLineClass(j)) == 0; ++j, ++k)
					SpellFxPlayLine(j, Me(), Target(), k * 2.5f);
			ImGui::PopID();
			open = ImGui::BeginTable(cls, 3, ImGuiTableFlags_SizingStretchSame);
		}
		if (!open)
			continue;
		ImGui::TableNextColumn();
		ImGui::PushID(i + 10000);
		if (ImGui::SmallButton(SpellFxLineName(i)))
			SpellFxPlayLine(i, Me(), Target(), 0.0f);
		ImGui::PopID();
	}
	if (open)
		ImGui::EndTable();
}

void DrawForms()
{
	if (ImGui::BeginTable("##forms", 5, ImGuiTableFlags_SizingStretchSame))
	{
		for (uint32_t f = eqgfx::gpu::FxForm_Flame; f < eqgfx::gpu::FxForm_Count; ++f)
		{
			ImGui::TableNextColumn();
			ImGui::PushID(static_cast<int>(f));
			if (ImGui::SmallButton(kFormNames[f]))
				SpellFxDemoForm(f, Me(), Target());
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
}

void DrawHooks()
{
	const TextBatchStats t = GetTextBatchStats();
	Status("nametag batching", t.nametagsInstalled, t.nametagsInstalled ? "" : t.failReason);
	Status("ui text batching", t.uiInstalled);
	Status("nametag sort", t.sortInstalled);
#ifdef CEverQuest__StartCasting_x
	Status("cast-start tap", s_castTapInstalled);
#endif
	Status("hit/heal tap", s_dmgTapInstalled);
	Status("own cast hook", SelfCastHookInstalled());
	Status("actor update hook", AnimationLodInstalled());
	Status("particle frame hook", ParticleGpuInstalled(), ParticleGpuFailReason());
	const FxStats fx = GetFxStats();
	Status("gpu effect pipeline", fx.ready, fx.error);
}

} // namespace

void ShowDebugWindow(bool on) { s_shown = on; }
bool DebugWindowShown()       { return s_shown; }

void DrawDebugWindow()
{
	if (!s_shown)
		return;

	ImGui::SetNextWindowSize(ImVec2(560, 620), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("EQGFX Debug", &s_shown))
	{
		if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen))
			DrawPerformance();
		if (ImGui::CollapsingHeader("Native UI"))
			eqgfx::ui::DrawNativeUiDebug();
		if (ImGui::CollapsingHeader("Text batching", ImGuiTreeNodeFlags_DefaultOpen))
			DrawTextBatching();
		if (ImGui::CollapsingHeader("World effects"))
			DrawWorldEffects();
		if (ImGui::CollapsingHeader("Spell looks"))
			DrawSpellLooks();
		if (ImGui::CollapsingHeader("Class lines"))
			DrawClassLines();
		if (ImGui::CollapsingHeader("GPU forms"))
			DrawForms();
		if (ImGui::CollapsingHeader("Hooks"))
			DrawHooks();
	}
	ImGui::End();
}
