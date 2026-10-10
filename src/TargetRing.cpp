#include "MQEQGFX.h"
#include "SettingsFile.h"
#include "gpu/FxCatalog.h"

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <mutex>

using eqgfx::gpu::FxInstance;

namespace {

constexpr const char* kSection = "TargetRing";

enum Style
{
	Style_Ring, Style_Double, Style_Comet, Style_Sparks, Style_Rune, Style_Pulse,
	Style_Triangle, Style_Diamond, Style_Hexagon, Style_Count
};
constexpr const char* kStyles[] = {
	"ring", "double ring", "comet chase", "orbiting sparks", "rune circle", "pulsing waves",
	"triangle", "diamond", "hexagon",
};

enum ColorMode { Color_Con, Color_Class, Color_Custom, Color_Count };
constexpr const char* kColorModes[] = { "con color", "class color", "custom" };

struct Settings
{
	bool  enabled     = true;
	int   style       = Style_Comet;
	int   colorMode   = Color_Con;
	float custom[4]   = { 1.0f, 0.85f, 0.25f, 1.0f };
	float opacity     = 0.85f;
	float size        = 1.0f;
	float thickness   = 0.18f;
	float spin        = 0.6f;
	float height      = 0.2f;
	bool  pulse       = false;
	float pulseRate   = 1.0f;
	bool  glow        = true;
	bool  throughWalls = false;
	bool  attackRed   = true;
	bool  hideSelf    = true;

	bool operator==(const Settings&) const = default;
};

std::mutex s_mutex;
Settings   s_cfg;

Settings Cfg()
{
	std::scoped_lock lock(s_mutex);
	return s_cfg;
}

float FeetZ(const PlayerClient* sp)
{
	const float h = sp->AvatarHeight > 0.5f ? sp->AvatarHeight : 6.0f;
	const float gap = sp->Z - sp->FloorHeight;
	return (gap >= -1.0f && gap <= h) ? sp->FloorHeight : sp->Z;
}

void ColorOf(const Settings& c, const PlayerClient* sp, float out[4])
{
	if (c.attackRed && pEverQuestInfo && pEverQuestInfo->bAutoAttack)
	{
		out[0] = 1.0f; out[1] = 0.15f; out[2] = 0.1f; out[3] = 1.0f;
		return;
	}
	out[3] = 1.0f;
	if (c.colorMode == Color_Con)
	{
		UnpackArgb(ConColorToARGB(ConColor(const_cast<PlayerClient*>(sp))), out);
		return;
	}
	if (c.colorMode == Color_Class)
	{
		if (const eqgfx::gpu::ClassFx* fx = eqgfx::gpu::FindClassFxById(sp->GetClass()))
		{
			std::copy(fx->defs[0].rgba, fx->defs[0].rgba + 3, out);
			return;
		}
		out[0] = out[1] = out[2] = 0.75f;
		return;
	}
	std::copy(c.custom, c.custom + 4, out);
}

FxInstance Base(const Settings& c, const PlayerClient* sp, const float rgba[4], float alpha)
{
	FxInstance fx = {};
	fx.pos[0] = sp->X; fx.pos[1] = sp->Y; fx.pos[2] = FeetZ(sp) + c.height;
	fx.radius = std::max(2.5f, (sp->AvatarHeight > 0.5f ? sp->AvatarHeight : 6.0f) * 0.6f) * c.size;
	fx.width = c.thickness;
	fx.speed = 1.0f;
	fx.segments = 64;
	fx.flags = (c.glow ? eqgfx::gpu::FxFlag_Additive : 0u) | (c.throughWalls ? eqgfx::gpu::FxFlag_NoDepth : 0u);
	fx.rgba[0] = rgba[0]; fx.rgba[1] = rgba[1]; fx.rgba[2] = rgba[2]; fx.rgba[3] = rgba[3] * alpha;
	return fx;
}

} // namespace

void BuildTargetRing(std::vector<FxInstance>& out)
{
	const Settings c = Cfg();
	PlayerClient* target = pTarget.get();
	if (!c.enabled || !target || out.size() + 3 > eqgfx::gpu::kMaxInstances)
		return;
	if (c.hideSelf && pLocalPlayer && target->SpawnID == pLocalPlayer->SpawnID)
		return;

	float rgba[4];
	ColorOf(c, target, rgba);
	float alpha = c.opacity;
	if (c.pulse)
		alpha *= 0.45f + 0.55f * (0.5f + 0.5f * std::sin(FxClockNow() * 6.2831853f * c.pulseRate));

	FxInstance fx = Base(c, target, rgba, alpha);
	switch (c.style)
	{
	case Style_Ring:
		fx.form = eqgfx::gpu::FxForm_Ring;
		fx.p1 = c.spin; fx.p2 = 1.0f;
		out.push_back(fx);
		break;
	case Style_Double:
	{
		fx.form = eqgfx::gpu::FxForm_Ring;
		fx.p1 = c.spin; fx.p2 = 1.0f;
		out.push_back(fx);
		FxInstance inner = fx;
		inner.p1 = -c.spin;
		inner.p2 = 0.75f;
		inner.width = c.thickness * 0.7f;
		out.push_back(inner);
		break;
	}
	case Style_Comet:
		fx.form = eqgfx::gpu::FxForm_Ring;
		fx.flags |= eqgfx::gpu::FxFlag_Comet;
		fx.p1 = 0.0f; fx.p2 = 1.0f; fx.p3 = c.spin;
		out.push_back(fx);
		break;
	case Style_Sparks:
		fx.form = eqgfx::gpu::FxForm_Orbit;
		fx.p0 = 14.0f;
		fx.height = 0.8f;
		fx.speed = std::max(0.1f, std::fabs(c.spin) * 1.5f);
		out.push_back(fx);
		break;
	case Style_Rune:
		fx.form = eqgfx::gpu::FxForm_Rune;
		fx.p0 = 5.0f; fx.p1 = 2.0f; fx.p2 = 10.0f; fx.p3 = 0.0f;
		fx.speed = c.spin;
		fx.segments = 96;
		out.push_back(fx);
		break;
	case Style_Pulse:
		fx.form = eqgfx::gpu::FxForm_Bloom;
		fx.p0 = 3.0f;
		fx.speed = std::max(0.2f, c.spin);
		out.push_back(fx);
		break;
	case Style_Triangle:
	case Style_Diamond:
	case Style_Hexagon:
		fx.form = eqgfx::gpu::FxForm_Polygon;
		fx.p0 = c.style == Style_Triangle ? 3.0f : c.style == Style_Diamond ? 4.0f : 6.0f;
		fx.p1 = c.spin;
		fx.p2 = 1.0f;
		out.push_back(fx);
		break;
	default:
		break;
	}
}

void DrawTargetRingOptions()
{
	Settings c = Cfg();
	bool changed = false;

	changed |= ImGui::Checkbox("show a ring under my target", &c.enabled);
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::Combo("style", &c.style, kStyles, Style_Count);
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::Combo("color", &c.colorMode, kColorModes, Color_Count);
	if (c.colorMode == Color_Custom)
	{
		ImGui::SameLine();
		changed |= ImGui::ColorEdit4("##custom", c.custom, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
	}
	changed |= ImGui::Checkbox("red while attacking", &c.attackRed);

	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("opacity", &c.opacity, 0.05f, 1.0f, "%.2f");
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("size", &c.size, 0.3f, 3.0f, "%.2f");
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("thickness", &c.thickness, 0.03f, 0.6f, "%.2f");
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("spin", &c.spin, -3.0f, 3.0f, "%.2f");
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("height", &c.height, 0.0f, 6.0f, "%.1f");

	changed |= ImGui::Checkbox("pulse", &c.pulse);
	if (c.pulse)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(120.0f);
		changed |= ImGui::SliderFloat("##rate", &c.pulseRate, 0.2f, 4.0f, "%.1f / s");
	}
	changed |= ImGui::Checkbox("glow", &c.glow);
	ImGui::SameLine();
	changed |= ImGui::Checkbox("through walls", &c.throughWalls);
	ImGui::SameLine();
	changed |= ImGui::Checkbox("not on myself", &c.hideSelf);

	if (changed)
	{
		{
			std::scoped_lock lock(s_mutex);
			s_cfg = c;
		}
		FlushTargetRingSettings();
	}
}

void LoadTargetRingSettings()
{
	Settings c;
	c.enabled      = eqgfx::settings::Global().GetBool(kSection, "Enabled", c.enabled);
	c.style        = std::clamp(eqgfx::settings::Global().GetInt(kSection, "Style", c.style), 0, Style_Count - 1);
	c.colorMode    = std::clamp(eqgfx::settings::Global().GetInt(kSection, "Color", c.colorMode), 0, Color_Count - 1);
	c.custom[0]    = eqgfx::settings::Global().GetFloat(kSection, "CustomR", c.custom[0]);
	c.custom[1]    = eqgfx::settings::Global().GetFloat(kSection, "CustomG", c.custom[1]);
	c.custom[2]    = eqgfx::settings::Global().GetFloat(kSection, "CustomB", c.custom[2]);
	c.custom[3]    = eqgfx::settings::Global().GetFloat(kSection, "CustomA", c.custom[3]);
	c.opacity      = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "Opacity", c.opacity), 0.05f, 1.0f);
	c.size         = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "Size", c.size), 0.3f, 3.0f);
	c.thickness    = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "Thickness", c.thickness), 0.03f, 0.6f);
	c.spin         = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "Spin", c.spin), -3.0f, 3.0f);
	c.height       = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "Height", c.height), 0.0f, 6.0f);
	c.pulse        = eqgfx::settings::Global().GetBool(kSection, "Pulse", c.pulse);
	c.pulseRate    = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "PulseRate", c.pulseRate), 0.2f, 4.0f);
	c.glow         = eqgfx::settings::Global().GetBool(kSection, "Glow", c.glow);
	c.throughWalls = eqgfx::settings::Global().GetBool(kSection, "ThroughWalls", c.throughWalls);
	c.attackRed    = eqgfx::settings::Global().GetBool(kSection, "AttackRed", c.attackRed);
	c.hideSelf     = eqgfx::settings::Global().GetBool(kSection, "HideSelf", c.hideSelf);
	std::scoped_lock lock(s_mutex);
	s_cfg = c;
}

void FlushTargetRingSettings()
{
	const Settings c = Cfg();
	eqgfx::settings::Section sec;
	sec.PutBool("Enabled", c.enabled);
	sec.PutInt("Style", c.style);
	sec.PutInt("Color", c.colorMode);
	sec.PutFloat("CustomR", c.custom[0]);
	sec.PutFloat("CustomG", c.custom[1]);
	sec.PutFloat("CustomB", c.custom[2]);
	sec.PutFloat("CustomA", c.custom[3]);
	sec.PutFloat("Opacity", c.opacity);
	sec.PutFloat("Size", c.size);
	sec.PutFloat("Thickness", c.thickness);
	sec.PutFloat("Spin", c.spin);
	sec.PutFloat("Height", c.height);
	sec.PutBool("Pulse", c.pulse);
	sec.PutFloat("PulseRate", c.pulseRate);
	sec.PutBool("Glow", c.glow);
	sec.PutBool("ThroughWalls", c.throughWalls);
	sec.PutBool("AttackRed", c.attackRed);
	sec.PutBool("HideSelf", c.hideSelf);
	eqgfx::settings::Global().SetSection(kSection, std::move(sec));
}
