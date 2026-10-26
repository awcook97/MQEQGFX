#include "../../MQEQGFX.h"
#include "../../SettingsFile.h"

#include <imgui.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

using eqgfx::gpu::FxInstance;

namespace {

constexpr const char* kSection = "Attack";
constexpr float kPi = 3.14159265f;

constexpr const char* kPlain[] = { "off", "crossed swords", "ATTACKING badge", "swords over the target" };
constexpr const char* kStandard[] = { "off", "flashing border", "edge glow", "ring under the target" };
constexpr const char* kFancy[] = { "off", "blade storm", "lightning duel", "meteor assault" };
constexpr const char* kBar[] = { "off", "flash", "chase", "flash and chase" };

std::atomic<int>   s_plain{ 0 };
std::atomic<int>   s_standard{ 1 };
std::atomic<int>   s_fancy{ 0 };
std::atomic<float> s_intensity{ 1.0f };
std::atomic<float> s_anchorX{ 0.5f };
std::atomic<float> s_anchorY{ 0.12f };
std::atomic<double> s_previewUntil{ 0.0 };

std::atomic<int>      s_barStyle{ 0 };
std::atomic<uint32_t> s_barColorA{ IM_COL32(255, 40, 40, 255) };
std::atomic<uint32_t> s_barColorB{ IM_COL32(255, 255, 255, 255) };
std::atomic<float>    s_barFlashHz{ 4.0f };
std::atomic<bool>     s_barFlashHard{ true };
std::atomic<float>    s_barChaseSpeed{ 160.0f };
std::atomic<float>    s_barChaseLength{ 12.0f };
std::atomic<float>    s_barThickness{ 2.0f };
std::atomic<float>    s_barFill{ 0.25f };

bool Attacking()
{
	if (ImGui::GetCurrentContext() && ImGui::GetTime() < s_previewUntil.load())
		return true;
	return pEverQuestInfo && pEverQuestInfo->bAutoAttack && pTarget;
}

bool AttackingPulse()
{
	return pEverQuestInfo && pEverQuestInfo->bAutoAttack && pTarget;
}

float FeetZ(const PlayerClient* sp)
{
	const float h = sp->AvatarHeight > 0.5f ? sp->AvatarHeight : 6.0f;
	const float gap = sp->Z - sp->FloorHeight;
	return (gap >= -1.0f && gap <= h) ? sp->FloorHeight : sp->Z;
}

float ChestZ(const PlayerClient* sp)
{
	const float h = sp->AvatarHeight > 0.5f ? sp->AvatarHeight : 6.0f;
	return FeetZ(sp) + h * 0.6f;
}

struct Strike
{
	FxInstance fx;
	int        targetId;
	bool       fromMe;
	bool       atFeet;
};

std::mutex          s_strikesMutex;
std::vector<Strike> s_strikes;

void SetColor(FxInstance& fx, float r, float g, float b, float a)
{
	fx.rgba[0] = r; fx.rgba[1] = g; fx.rgba[2] = b; fx.rgba[3] = a;
}

void AddStrike(const FxInstance& fx, int targetId, bool fromMe, bool atFeet)
{
	std::scoped_lock lock(s_strikesMutex);
	if (s_strikes.size() < 256)
		s_strikes.push_back({ fx, targetId, fromMe, atFeet });
}

FxInstance Base(uint32_t form, float life, float birth)
{
	FxInstance fx = {};
	fx.form = form;
	fx.life = life;
	fx.birth = birth;
	fx.speed = 1.0f;
	fx.flags = eqgfx::gpu::FxFlag_Additive;
	return fx;
}

void StrikeFor(int style, int targetId, bool crit, float k)
{
	const float now = FxClockNow();
	const float big = crit ? 1.6f : 1.0f;
	const float hue[3] = { crit ? 1.0f : 1.0f, crit ? 0.8f : 0.25f, crit ? 0.2f : 0.15f };

	switch (style)
	{
	case 1:
	{
		FxInstance slash = Base(eqgfx::gpu::FxForm_Ring, 0.35f, now);
		slash.radius = 3.2f * big * k;
		slash.segments = 48;
		slash.width = 0.18f * big;
		slash.tilt = 0.6f + 1.2f * static_cast<float>(std::rand() % 100) / 100.0f;
		slash.phase = static_cast<float>(std::rand() % 100) / 100.0f;
		slash.p1 = 0.0f;
		slash.p2 = 1.0f;
		slash.p3 = 3.0f;
		slash.flags |= eqgfx::gpu::FxFlag_Comet;
		SetColor(slash, 1.0f, 0.95f, 0.9f, 0.95f);
		slash.rgba2[0] = hue[0]; slash.rgba2[1] = hue[1]; slash.rgba2[2] = hue[2]; slash.rgba2[3] = 0.8f;
		AddStrike(slash, targetId, false, false);

		FxInstance sparks = Base(eqgfx::gpu::FxForm_Sparkle, 0.5f, now);
		sparks.radius = 2.0f * big * k;
		sparks.height = 1.0f;
		sparks.segments = 48;
		sparks.width = 0.08f;
		sparks.p0 = crit ? 24.0f : 12.0f;
		sparks.p1 = 1.0f;
		SetColor(sparks, hue[0], hue[1], hue[2], 0.9f);
		AddStrike(sparks, targetId, false, false);
		break;
	}
	case 2:
	{
		FxInstance flash = Base(eqgfx::gpu::FxForm_Burst, 0.4f, now);
		flash.radius = 3.0f * big * k;
		flash.segments = 64;
		flash.width = 0.12f;
		flash.p0 = 2.0f; flash.p1 = 0.5f; flash.p2 = 10.0f;
		SetColor(flash, 0.6f, 0.8f, 1.0f, 0.95f);
		if (crit)
			SetColor(flash, 1.0f, 0.9f, 0.4f, 1.0f);
		AddStrike(flash, targetId, false, false);

		FxInstance cracks = Base(eqgfx::gpu::FxForm_Cracks, 0.9f, now);
		cracks.radius = 3.5f * big * k;
		cracks.segments = 48;
		cracks.width = 0.08f;
		cracks.p0 = 6.0f; cracks.p1 = 1.2f;
		SetColor(cracks, 0.5f, 0.75f, 1.0f, 0.9f);
		AddStrike(cracks, targetId, false, true);
		break;
	}
	case 3:
	{
		FxInstance bolt = Base(eqgfx::gpu::FxForm_Bolt, 0.6f, now);
		bolt.radius = 1.0f;
		bolt.segments = 96;
		bolt.width = 0.2f * big;
		bolt.p0 = 4.0f; bolt.p1 = 0.5f; bolt.p2 = 0.45f; bolt.p3 = 3.0f;
		SetColor(bolt, 1.0f, 0.55f, 0.15f, 1.0f);
		bolt.rgba2[0] = hue[0]; bolt.rgba2[1] = hue[1]; bolt.rgba2[2] = hue[2]; bolt.rgba2[3] = 0.9f;
		AddStrike(bolt, targetId, true, false);

		FxInstance impact = Base(eqgfx::gpu::FxForm_Burst, 0.6f, now + 0.42f);
		impact.radius = 4.0f * big * k;
		impact.segments = 96;
		impact.width = 0.14f;
		impact.p0 = 3.0f; impact.p1 = 1.5f; impact.p2 = 12.0f;
		SetColor(impact, 1.0f, 0.45f, 0.1f, 1.0f);
		AddStrike(impact, targetId, false, true);

		FxInstance bloom = Base(eqgfx::gpu::FxForm_Bloom, 0.8f, now + 0.42f);
		bloom.radius = 5.0f * big * k;
		bloom.segments = 64;
		bloom.width = 0.1f;
		bloom.p0 = 3.0f;
		SetColor(bloom, 1.0f, 0.3f, 0.05f, 0.8f);
		AddStrike(bloom, targetId, false, true);
		break;
	}
	default:
		break;
	}
}

void Steady(std::vector<FxInstance>& out, PlayerClient* me, PlayerClient* target, float k)
{
	const float now = FxClockNow();
	const int standard = s_standard.load();
	const int fancy = s_fancy.load();

	if (standard == 3)
	{
		FxInstance ring = Base(eqgfx::gpu::FxForm_Ring, 0.0f, 0.0f);
		ring.pos[0] = target->X; ring.pos[1] = target->Y; ring.pos[2] = FeetZ(target) + 0.2f;
		ring.radius = std::max(3.0f, target->AvatarHeight * 0.7f) * k;
		ring.segments = 64;
		ring.width = 0.22f;
		ring.speed = 1.5f;
		ring.p1 = 0.6f; ring.p2 = 1.0f; ring.p3 = 0.3f;
		ring.flags |= eqgfx::gpu::FxFlag_Comet;
		const float pulse = 0.55f + 0.45f * std::sin(now * 5.0f);
		SetColor(ring, 1.0f, 0.15f, 0.1f, pulse);
		out.push_back(ring);
	}

	const float chestMe[3] = { me->X, me->Y, ChestZ(me) };
	const float chestTarget[3] = { target->X, target->Y, ChestZ(target) };

	switch (fancy)
	{
	case 1:
	{
		FxInstance drain = Base(eqgfx::gpu::FxForm_Drain, 0.0f, 0.0f);
		std::copy(chestMe, chestMe + 3, drain.pos);
		std::copy(chestTarget, chestTarget + 3, drain.pos2);
		drain.segments = 96;
		drain.width = 0.08f;
		drain.speed = 1.6f;
		drain.p0 = 3.0f; drain.p1 = 0.8f; drain.p2 = 8.0f;
		SetColor(drain, 1.0f, 0.2f, 0.15f, 0.7f);
		out.push_back(drain);
		break;
	}
	case 2:
	{
		FxInstance arc = Base(eqgfx::gpu::FxForm_Lightning, 0.0f, 0.0f);
		std::copy(chestMe, chestMe + 3, arc.pos);
		std::copy(chestTarget, chestTarget + 3, arc.pos2);
		arc.segments = 96;
		arc.width = 0.14f * k;
		arc.p0 = 3.0f; arc.p1 = 0.7f; arc.p2 = 10.0f;
		SetColor(arc, 0.55f, 0.75f, 1.0f, 0.9f);
		arc.rgba2[0] = 0.9f; arc.rgba2[1] = 0.95f; arc.rgba2[2] = 1.0f; arc.rgba2[3] = 0.9f;
		out.push_back(arc);
		break;
	}
	case 3:
	{
		FxInstance rune = Base(eqgfx::gpu::FxForm_Rune, 0.0f, 0.0f);
		rune.pos[0] = me->X; rune.pos[1] = me->Y; rune.pos[2] = FeetZ(me) + 0.15f;
		rune.radius = 3.5f * k;
		rune.segments = 96;
		rune.width = 0.08f;
		rune.speed = 0.6f;
		rune.p0 = 5.0f; rune.p1 = 2.0f; rune.p2 = 10.0f;
		SetColor(rune, 1.0f, 0.4f, 0.1f, 0.75f);
		out.push_back(rune);
		break;
	}
	default:
		break;
	}
}

void Swords(ImDrawList* dl, ImVec2 c, float size, ImU32 col)
{
	for (int side = -1; side <= 1; side += 2)
	{
		const ImVec2 hilt(c.x + side * size * 0.45f, c.y + size * 0.45f);
		const ImVec2 tip(c.x - side * size * 0.5f, c.y - size * 0.5f);
		dl->AddLine(hilt, tip, col, size * 0.12f);
		const ImVec2 dir((tip.x - hilt.x) / size, (tip.y - hilt.y) / size);
		const ImVec2 guard(hilt.x + dir.x * size * 0.2f, hilt.y + dir.y * size * 0.2f);
		const ImVec2 perp(-dir.y * size * 0.2f, dir.x * size * 0.2f);
		dl->AddLine(ImVec2(guard.x - perp.x, guard.y - perp.y), ImVec2(guard.x + perp.x, guard.y + perp.y), col,
			size * 0.1f);
	}
}

ImU32 Red(float a)
{
	return IM_COL32(230, 40, 30, static_cast<int>(std::clamp(a, 0.0f, 1.0f) * 255.0f));
}

} // namespace

void AttackFxNoteHit(int attackerId, int targetId, int damage, uint32_t flags)
{
	(void)damage;
	const int fancy = s_fancy.load();
	if (!fancy || !pLocalPlayer || attackerId != static_cast<int>(pLocalPlayer->SpawnID) || !AttackingPulse())
		return;
	StrikeFor(fancy, targetId, (flags & HitFlag_Critical) != 0, s_intensity.load());
}

void BuildAttackFx(std::vector<FxInstance>& out)
{
	PlayerClient* me = pLocalPlayer.get();
	PlayerClient* target = pTarget.get();
	const float k = s_intensity.load();

	if (me && target && AttackingPulse())
		Steady(out, me, target, k);

	const float now = FxClockNow();
	std::scoped_lock lock(s_strikesMutex);
	s_strikes.erase(std::remove_if(s_strikes.begin(), s_strikes.end(), [now](const Strike& s) {
		return now > s.fx.birth + s.fx.life;
	}), s_strikes.end());
	for (const Strike& s : s_strikes)
	{
		PlayerClient* victim = GetSpawnByID(s.targetId);
		if (!victim || !me || out.size() >= eqgfx::gpu::kMaxInstances)
			continue;
		FxInstance fx = s.fx;
		if (s.fromMe)
		{
			fx.pos[0] = me->X; fx.pos[1] = me->Y; fx.pos[2] = ChestZ(me);
			fx.pos2[0] = victim->X; fx.pos2[1] = victim->Y; fx.pos2[2] = ChestZ(victim);
		}
		else
		{
			fx.pos[0] = victim->X; fx.pos[1] = victim->Y;
			fx.pos[2] = s.atFeet ? FeetZ(victim) + 0.1f : ChestZ(victim);
		}
		out.push_back(fx);
	}
}

namespace {

ImU32 MixColor(ImU32 a, ImU32 b, float t)
{
	const ImVec4 va = ImGui::ColorConvertU32ToFloat4(a);
	const ImVec4 vb = ImGui::ColorConvertU32ToFloat4(b);
	return ImGui::ColorConvertFloat4ToU32(ImVec4(va.x + (vb.x - va.x) * t, va.y + (vb.y - va.y) * t,
		va.z + (vb.z - va.z) * t, va.w + (vb.w - va.w) * t));
}

ImU32 ScaleAlpha(ImU32 c, float k)
{
	ImVec4 v = ImGui::ColorConvertU32ToFloat4(c);
	v.w *= k;
	return ImGui::ColorConvertFloat4ToU32(v);
}

float FlashMix(double t)
{
	const double phase = std::fmod(t * std::max(s_barFlashHz.load(), 0.1f), 1.0);
	if (s_barFlashHard.load())
		return phase >= 0.5 ? 1.0f : 0.0f;
	return 0.5f + 0.5f * std::sin(static_cast<float>(phase) * 2.0f * kPi);
}

ImVec2 PerimeterAt(const ImVec2& lo, const ImVec2& hi, float s)
{
	const float w = hi.x - lo.x, h = hi.y - lo.y;
	if (s < w)
		return {lo.x + s, lo.y};
	s -= w;
	if (s < h)
		return {hi.x, lo.y + s};
	s -= h;
	if (s < w)
		return {hi.x - s, hi.y};
	s -= w;
	return {lo.x, hi.y - s};
}

void PerimeterRun(ImDrawList* dl, const ImVec2& lo, const ImVec2& hi, float a, float b, ImU32 col, float thick)
{
	const float w = hi.x - lo.x, h = hi.y - lo.y;
	const float corners[] = { w, w + h, w + w + h };
	ImVec2 pts[5];
	int n = 0;
	pts[n++] = PerimeterAt(lo, hi, a);
	for (float c : corners)
		if (c > a && c < b)
			pts[n++] = PerimeterAt(lo, hi, c);
	pts[n++] = PerimeterAt(lo, hi, b);
	dl->AddPolyline(pts, n, col, ImDrawFlags_None, thick);
}

} // namespace

void DrawAttackHealthBar(ImDrawList* dl, float x0, float y0, float x1, float y1, float rounding)
{
	const int style = s_barStyle.load();
	if (!style || !dl || !Attacking() || x1 <= x0 || y1 <= y0)
		return;

	const double t = ImGui::GetTime();
	const ImU32 a = s_barColorA.load(), b = s_barColorB.load();
	const bool flash = style == 1 || style == 3;
	const bool chase = style == 2 || style == 3;
	const float mix = flash ? FlashMix(t) : 0.0f;
	const float thick = std::max(s_barThickness.load(), 0.5f);

	const float fill = s_barFill.load();
	if (fill > 0.0f)
		dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), ScaleAlpha(MixColor(a, b, mix), fill), rounding);

	const ImVec2 lo(x0 + thick * 0.5f, y0 + thick * 0.5f);
	const ImVec2 hi(x1 - thick * 0.5f, y1 - thick * 0.5f);
	if (hi.x <= lo.x || hi.y <= lo.y)
		return;

	if (!chase)
	{
		dl->AddRect(lo, hi, MixColor(a, b, mix), rounding, 0, thick);
		return;
	}

	const float perimeter = 2.0f * ((hi.x - lo.x) + (hi.y - lo.y));
	const float pairs = std::max(1.0f, std::round(perimeter / (2.0f * std::max(s_barChaseLength.load(), 2.0f))));
	const float len = perimeter / (2.0f * pairs);
	const float offset = static_cast<float>(std::fmod(t * s_barChaseSpeed.load(), 2.0f * len));
	const bool swap = flash && mix > 0.5f;
	int k = 0;
	for (float s = offset - 2.0f * len; s < perimeter; s += len, ++k)
	{
		const float s0 = std::max(s, 0.0f), s1 = std::min(s + len, perimeter);
		if (s1 > s0)
			PerimeterRun(dl, lo, hi, s0, s1, ((k & 1) != swap) ? b : a, thick);
	}
}

void DrawAttackOverlay()
{
	if (!Attacking())
		return;

	const ImGuiIO& io = ImGui::GetIO();
	ImDrawList* dl = ImGui::GetForegroundDrawList();
	const double t = ImGui::GetTime();
	const float k = s_intensity.load();
	const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(std::fmod(t * 1.1, 1.0)) * 2.0f * kPi);

	switch (s_standard.load())
	{
	case 1:
	{
		const float w = 6.0f * k;
		dl->AddRect(ImVec2(w * 0.5f, w * 0.5f), ImVec2(io.DisplaySize.x - w * 0.5f, io.DisplaySize.y - w * 0.5f),
			Red(0.25f + 0.65f * pulse), 0.0f, 0, w);
		break;
	}
	case 2:
	{
		const float d = 90.0f * k;
		const ImU32 in = Red(0.45f * (0.4f + 0.6f * pulse));
		const ImU32 out = Red(0.0f);
		const ImVec2 s = io.DisplaySize;
		dl->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(s.x, d), in, in, out, out);
		dl->AddRectFilledMultiColor(ImVec2(0, s.y - d), s, out, out, in, in);
		dl->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(d, s.y), in, out, out, in);
		dl->AddRectFilledMultiColor(ImVec2(s.x - d, 0), s, out, in, in, out);
		break;
	}
	default:
		break;
	}

	const ImVec2 anchor(io.DisplaySize.x * s_anchorX.load(), io.DisplaySize.y * s_anchorY.load());
	switch (s_plain.load())
	{
	case 1:
		Swords(dl, anchor, 28.0f * k, Red(0.7f + 0.3f * pulse));
		break;
	case 2:
	{
		const char* text = "ATTACKING";
		const ImVec2 ts = ImGui::CalcTextSize(text);
		const ImVec2 mn(anchor.x - ts.x * 0.5f - 8.0f, anchor.y - ts.y * 0.5f - 4.0f);
		const ImVec2 mx(anchor.x + ts.x * 0.5f + 8.0f, anchor.y + ts.y * 0.5f + 4.0f);
		dl->AddRectFilled(mn, mx, IM_COL32(120, 10, 10, static_cast<int>(170 + 60 * pulse)), 4.0f);
		dl->AddRect(mn, mx, Red(0.9f), 4.0f, 0, 1.5f);
		dl->AddText(ImVec2(anchor.x - ts.x * 0.5f, anchor.y - ts.y * 0.5f), IM_COL32(255, 230, 220, 255), text);
		break;
	}
	case 3:
		if (PlayerClient* target = pTarget.get())
		{
			float sx = 0.0f, sy = 0.0f;
			const float h = target->AvatarHeight > 0.5f ? target->AvatarHeight : 6.0f;
			if (ProjectToScreen(target->X, target->Y, FeetZ(target) + h * 1.25f, sx, sy))
				Swords(dl, ImVec2(sx, sy - 8.0f * pulse), 22.0f * k, Red(0.75f + 0.25f * pulse));
		}
		break;
	default:
		break;
	}
}

void DrawAttackOptions()
{
	bool changed = false;
	int plain = s_plain.load(), standard = s_standard.load(), fancy = s_fancy.load();
	float intensity = s_intensity.load(), ax = s_anchorX.load(), ay = s_anchorY.load();

	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::Combo("plain", &plain, kPlain, IM_ARRAYSIZE(kPlain));
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::Combo("standard", &standard, kStandard, IM_ARRAYSIZE(kStandard));
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::Combo("fancy", &fancy, kFancy, IM_ARRAYSIZE(kFancy));
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("drawn between you and your target; each of your melee hits strikes");
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::SliderFloat("size", &intensity, 0.3f, 2.5f, "%.2f");
	if (plain == 1 || plain == 2)
	{
		ImGui::SetNextItemWidth(200.0f);
		changed |= ImGui::SliderFloat("icon across", &ax, 0.0f, 1.0f, "%.2f");
		ImGui::SetNextItemWidth(200.0f);
		changed |= ImGui::SliderFloat("icon down", &ay, 0.0f, 1.0f, "%.2f");
	}
	int barStyle = s_barStyle.load();
	ImVec4 barA = ImGui::ColorConvertU32ToFloat4(s_barColorA.load());
	ImVec4 barB = ImGui::ColorConvertU32ToFloat4(s_barColorB.load());
	float flashHz = s_barFlashHz.load(), chaseSpeed = s_barChaseSpeed.load(), chaseLen = s_barChaseLength.load();
	float barThick = s_barThickness.load(), barFill = s_barFill.load();
	bool flashHard = s_barFlashHard.load();
	ImGui::SetNextItemWidth(200.0f);
	changed |= ImGui::Combo("health bar", &barStyle, kBar, IM_ARRAYSIZE(kBar));
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("your health bar in the player window while you attack");
	if (barStyle)
	{
		changed |= ImGui::ColorEdit4("bar color 1", &barA.x, ImGuiColorEditFlags_NoInputs);
		ImGui::SameLine();
		changed |= ImGui::ColorEdit4("bar color 2", &barB.x, ImGuiColorEditFlags_NoInputs);
		if (barStyle == 1 || barStyle == 3)
		{
			ImGui::SetNextItemWidth(200.0f);
			changed |= ImGui::SliderFloat("flash speed", &flashHz, 0.5f, 20.0f, "%.1f / s");
			ImGui::SameLine();
			changed |= ImGui::Checkbox("hard flash", &flashHard);
		}
		if (barStyle == 2 || barStyle == 3)
		{
			ImGui::SetNextItemWidth(200.0f);
			changed |= ImGui::SliderFloat("chase speed", &chaseSpeed, 10.0f, 1000.0f, "%.0f px/s");
			ImGui::SetNextItemWidth(200.0f);
			changed |= ImGui::SliderFloat("color length", &chaseLen, 2.0f, 80.0f, "%.0f px");
		}
		ImGui::SetNextItemWidth(200.0f);
		changed |= ImGui::SliderFloat("border width", &barThick, 0.5f, 8.0f, "%.1f px");
		ImGui::SetNextItemWidth(200.0f);
		changed |= ImGui::SliderFloat("fill tint", &barFill, 0.0f, 1.0f, "%.2f");
	}

	if (ImGui::Button("preview overlays for 3 s"))
		s_previewUntil = ImGui::GetTime() + 3.0;
	ImGui::SameLine();
	ImGui::BeginDisabled(!fancy || !pTarget);
	if (ImGui::Button("preview a strike"))
		if (pTarget)
			StrikeFor(fancy, pTarget->SpawnID, false, intensity);
	ImGui::EndDisabled();

	if (changed)
	{
		s_plain = plain;
		s_standard = standard;
		s_fancy = fancy;
		s_intensity = intensity;
		s_anchorX = ax;
		s_anchorY = ay;
		s_barStyle = barStyle;
		s_barColorA = ImGui::ColorConvertFloat4ToU32(barA);
		s_barColorB = ImGui::ColorConvertFloat4ToU32(barB);
		s_barFlashHz = flashHz;
		s_barFlashHard = flashHard;
		s_barChaseSpeed = chaseSpeed;
		s_barChaseLength = chaseLen;
		s_barThickness = barThick;
		s_barFill = barFill;
		FlushAttackSettings();
	}
}

namespace {

std::string FormatColor(ImU32 c)
{
	char buf[16];
	std::snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF, (c >> 24) & 0xFF);
	return buf;
}

ImU32 ParseColor(const std::string& text, ImU32 fallback)
{
	const char* p = text.c_str();
	if (*p == '#')
		++p;
	if (std::strlen(p) != 8)
		return fallback;
	char* end = nullptr;
	const unsigned long v = std::strtoul(p, &end, 16);
	if (!end || *end)
		return fallback;
	return IM_COL32((v >> 24) & 0xFF, (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
}

} // namespace

void LoadAttackSettings()
{
	s_plain = std::clamp(eqgfx::settings::Global().GetInt(kSection, "Plain", s_plain.load()), 0, 3);
	s_standard = std::clamp(eqgfx::settings::Global().GetInt(kSection, "Standard", s_standard.load()), 0, 3);
	s_fancy = std::clamp(eqgfx::settings::Global().GetInt(kSection, "Fancy", s_fancy.load()), 0, 3);
	s_intensity = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "Size", s_intensity.load()), 0.3f, 2.5f);
	s_anchorX = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "IconX", s_anchorX.load()), 0.0f, 1.0f);
	s_anchorY = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "IconY", s_anchorY.load()), 0.0f, 1.0f);
	s_barStyle = std::clamp(eqgfx::settings::Global().GetInt(kSection, "Bar", s_barStyle.load()), 0, 3);
	s_barColorA = ParseColor(eqgfx::settings::Global().GetString(kSection, "BarColor1", ""), s_barColorA.load());
	s_barColorB = ParseColor(eqgfx::settings::Global().GetString(kSection, "BarColor2", ""), s_barColorB.load());
	s_barFlashHz = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "BarFlashSpeed", s_barFlashHz.load()), 0.1f, 60.0f);
	s_barFlashHard = eqgfx::settings::Global().GetBool(kSection, "BarFlashHard", s_barFlashHard.load());
	s_barChaseSpeed = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "BarChaseSpeed", s_barChaseSpeed.load()), 0.0f, 5000.0f);
	s_barChaseLength = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "BarColorLength", s_barChaseLength.load()), 2.0f, 500.0f);
	s_barThickness = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "BarBorderWidth", s_barThickness.load()), 0.5f, 20.0f);
	s_barFill = std::clamp(eqgfx::settings::Global().GetFloat(kSection, "BarFillTint", s_barFill.load()), 0.0f, 1.0f);
}

void FlushAttackSettings()
{
	eqgfx::settings::Section sec;
	sec.PutInt("Plain", s_plain.load());
	sec.PutInt("Standard", s_standard.load());
	sec.PutInt("Fancy", s_fancy.load());
	sec.PutFloat("Size", s_intensity.load());
	sec.PutFloat("IconX", s_anchorX.load());
	sec.PutFloat("IconY", s_anchorY.load());
	sec.PutInt("Bar", s_barStyle.load());
	sec.Put("BarColor1", FormatColor(s_barColorA.load()));
	sec.Put("BarColor2", FormatColor(s_barColorB.load()));
	sec.PutFloat("BarFlashSpeed", s_barFlashHz.load());
	sec.PutBool("BarFlashHard", s_barFlashHard.load());
	sec.PutFloat("BarChaseSpeed", s_barChaseSpeed.load());
	sec.PutFloat("BarColorLength", s_barChaseLength.load());
	sec.PutFloat("BarBorderWidth", s_barThickness.load());
	sec.PutFloat("BarFillTint", s_barFill.load());
	eqgfx::settings::Global().SetSection(kSection, std::move(sec));
}
