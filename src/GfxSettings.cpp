#include "MQEQGFX.h"
#include "SettingsFile.h"

#include <algorithm>

namespace {

constexpr const char* kSection = "Gfx";

struct Snapshot
{
	bool  classFx, classFxPCs, classFxNPCs;
	float classFxRadius;
	int   classFxMaxSpawns, classFxSlotMask;
	AlwaysOnStyle classFxStyle;
	bool  gpuEffects;
	bool  nametagBatching, uiTextBatching;
	bool  animLod;
	float animLodDistance;
	int   animLodInterval;
	bool  particleGpu;
	int   particleBudget;
	bool  worldHover, focusMarker, hoverFallback, buffProbe;

	bool operator==(const Snapshot&) const = default;
};

Snapshot s_saved = {};
bool     s_loaded = false;
bool     s_dirty = false;
uint32_t s_changedAt = 0;

Snapshot Capture()
{
	const GfxOptions g = GetGfxOptions();
	const UnitOptions u = GetUnitOptions();
	const TextBatchStats t = GetTextBatchStats();
	const AnimationLodOptions a = GetAnimationLodOptions();
	return { g.classFx, g.classFxPCs, g.classFxNPCs, g.classFxRadius, g.classFxMaxSpawns,
		g.classFxSlotMask, g.classFxStyle, g.gpuEffects, t.nametagsOn, t.uiOn,
		a.enabled, a.distance, a.maxInterval, ParticleGpuEnabled(), GetParticleGpuBudget(),
		u.worldHover, u.focusMarker, u.hoverFallback, u.buffProbe };
}

void Write(const Snapshot& s)
{
	eqgfx::settings::Section sec;
	sec.PutBool("ClassFx", s.classFx);
	sec.PutBool("ClassFxPCs", s.classFxPCs);
	sec.PutBool("ClassFxNPCs", s.classFxNPCs);
	sec.PutFloat("ClassFxRadius", s.classFxRadius);
	sec.PutInt("ClassFxMaxSpawns", s.classFxMaxSpawns);
	sec.PutInt("ClassFxSlotMask", s.classFxSlotMask);
	sec.PutFloat("ClassFxOpacity", s.classFxStyle.opacity);
	sec.PutFloat("ClassFxMotion", s.classFxStyle.motion);
	sec.PutFloat("ClassFxFadeAfter", s.classFxStyle.fadeAfter);
	sec.PutFloat("ClassFxFadeTo", s.classFxStyle.fadeTo);
	sec.PutBool("ClassFxBreathe", s.classFxStyle.breathe);
	sec.PutBool("GpuEffects", s.gpuEffects);
	sec.PutBool("NametagBatching", s.nametagBatching);
	sec.PutBool("UiTextBatching", s.uiTextBatching);
	sec.PutBool("AnimLod", s.animLod);
	sec.PutFloat("AnimLodDistance", s.animLodDistance);
	sec.PutInt("AnimLodInterval", s.animLodInterval);
	sec.PutBool("ParticleGpu", s.particleGpu);
	sec.PutInt("ParticleGpuBudget", s.particleBudget);
	sec.PutBool("WorldHover", s.worldHover);
	sec.PutBool("FocusMarker", s.focusMarker);
	sec.PutBool("HoverFallback", s.hoverFallback);
	sec.PutBool("BuffProbe", s.buffProbe);
	eqgfx::settings::Global().SetSection(kSection, std::move(sec));
}

} // namespace

void LoadGfxSettings()
{
	const Snapshot d = Capture();

	GfxOptions g = GetGfxOptions();
	g.classFx          = eqgfx::settings::Global().GetBool(kSection, "ClassFx", d.classFx);
	g.classFxPCs       = eqgfx::settings::Global().GetBool(kSection, "ClassFxPCs", d.classFxPCs);
	g.classFxNPCs      = eqgfx::settings::Global().GetBool(kSection, "ClassFxNPCs", d.classFxNPCs);
	g.classFxRadius    = eqgfx::settings::Global().GetFloat(kSection, "ClassFxRadius", d.classFxRadius);
	g.classFxMaxSpawns = eqgfx::settings::Global().GetInt(kSection, "ClassFxMaxSpawns", d.classFxMaxSpawns);
	g.classFxSlotMask  = eqgfx::settings::Global().GetInt(kSection, "ClassFxSlotMask", d.classFxSlotMask);
	AlwaysOnStyle& st = g.classFxStyle;
	st.opacity   = eqgfx::settings::Global().GetFloat(kSection, "ClassFxAlpha", 1.0f)
		* eqgfx::settings::Global().GetFloat("SpellFx", "AlwaysOnOpacity", st.opacity);
	st.motion    = eqgfx::settings::Global().GetFloat("SpellFx", "AlwaysOnMotion", st.motion);
	st.fadeAfter = eqgfx::settings::Global().GetFloat("SpellFx", "AlwaysOnFadeAfter", st.fadeAfter);
	st.fadeTo    = eqgfx::settings::Global().GetFloat("SpellFx", "AlwaysOnFadeTo", st.fadeTo);
	st.breathe   = eqgfx::settings::Global().GetBool("SpellFx", "AlwaysOnBreathe", st.breathe);
	st.opacity   = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "ClassFxOpacity", st.opacity), 0.05f, 1.0f);
	st.motion    = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "ClassFxMotion", st.motion), 0.0f, 2.0f);
	st.fadeAfter = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "ClassFxFadeAfter", st.fadeAfter), 0.0f, 600.0f);
	st.fadeTo    = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "ClassFxFadeTo", st.fadeTo), 0.0f, 1.0f);
	st.breathe   = eqgfx::settings::Global().GetBool(kSection, "ClassFxBreathe", st.breathe);
	g.gpuEffects       = eqgfx::settings::Global().GetBool(kSection, "GpuEffects", d.gpuEffects);
	SetGfxOptions(g);

	SetNametagBatchEnabled(eqgfx::settings::Global().GetBool(kSection, "NametagBatching", d.nametagBatching));
	SetUiTextBatchEnabled(eqgfx::settings::Global().GetBool(kSection, "UiTextBatching", d.uiTextBatching));

	AnimationLodOptions a = GetAnimationLodOptions();
	a.enabled     = eqgfx::settings::Global().GetBool(kSection, "AnimLod", d.animLod);
	a.distance    = eqgfx::settings::Global().GetFloat(kSection, "AnimLodDistance", d.animLodDistance);
	a.maxInterval = eqgfx::settings::Global().GetInt(kSection, "AnimLodInterval", d.animLodInterval);
	SetAnimationLodOptions(a);
	SetParticleGpuEnabled(eqgfx::settings::Global().GetBool(kSection, "ParticleGpu", d.particleGpu));
	SetParticleGpuBudget(eqgfx::settings::Global().GetInt(kSection, "ParticleGpuBudget", d.particleBudget));

	UnitOptions u = GetUnitOptions();
	u.worldHover    = eqgfx::settings::Global().GetBool(kSection, "WorldHover", d.worldHover);
	u.focusMarker   = eqgfx::settings::Global().GetBool(kSection, "FocusMarker", d.focusMarker);
	u.hoverFallback = eqgfx::settings::Global().GetBool(kSection, "HoverFallback", d.hoverFallback);
	u.buffProbe     = eqgfx::settings::Global().GetBool(kSection, "BuffProbe", d.buffProbe) && BuffProbeAvailable();
	SetUnitOptions(u);

	s_saved = Capture();
	s_loaded = true;
}

void PulseGfxSettings()
{
	if (!s_loaded)
		return;
	const Snapshot now = Capture();
	const uint32_t ms = static_cast<uint32_t>(EQGetTime());
	if (!(now == s_saved))
	{
		s_saved = now;
		s_dirty = true;
		s_changedAt = ms;
		return;
	}
	if (s_dirty && ms - s_changedAt >= 1500)
	{
		s_dirty = false;
		Write(s_saved);
	}
}

void FlushGfxSettings()
{
	if (!s_loaded)
		return;
	const Snapshot now = Capture();
	if (s_dirty || !(now == s_saved))
		Write(now);
	s_dirty = false;
}
