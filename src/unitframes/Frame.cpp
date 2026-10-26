#include "UnitWindow.h"

#include "../MQEQGFX.h"
#include "../gpu/FxCatalog.h"
#include "../ui/Actions.h"
#include "../ui/Skins.h"
#include "../ui/Widgets.h"

#include <imgui.h>
#include <imgui/imanim/im_anim.h>
#include <mq/imgui/Widgets.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace eqgfx::unitframes {

namespace {

std::atomic<int> s_hoverWindow{ -1 };

constexpr float kMenuHoldSeconds = 0.35f;

ImGuiID s_rightPressRow = 0;
bool    s_rightPressHandled = false;

std::vector<ImVec4> s_rowRects[kWindowCount];

std::vector<int> s_rowIds[kWindowCount];

int s_menuWindow = -1;

struct RowAnim
{
	float  last = -1.0f;
	float  fallback = 0.0f;
	double flashAt = -1.0;
	bool   flashHeal = false;
	double seen = 0.0;
};

std::unordered_map<int, RowAnim> s_anim;
double s_lastSweep = 0.0;

RowAnim& AnimFor(int spawnId, double now)
{
	if (now - s_lastSweep > 5.0)
	{
		s_lastSweep = now;
		for (auto it = s_anim.begin(); it != s_anim.end(); )
			it = (now - it->second.seen > 20.0) ? s_anim.erase(it) : std::next(it);
	}

	RowAnim& a = s_anim[spawnId];
	a.seen = now;
	return a;
}

constexpr ImGuiID kChanHp = 0x48500001u;

iam_context* s_animCtx = nullptr;
iam_context* s_animPrev = nullptr;

} // namespace

void BeginAnimContext()
{
	if (!s_animCtx)
		s_animCtx = iam_context_create();

	s_animPrev = iam_context_set_current(s_animCtx);
}

void EndAnimContext()
{
	iam_context_set_current(s_animPrev);
	s_animPrev = nullptr;
}

namespace {

float Ratio(int64_t current, int64_t max)
{
	if (max <= 0)
		return 0.0f;
	return std::clamp(static_cast<float>(current) / static_cast<float>(max), 0.0f, 1.0f);
}

ImVec4 Lerp(const ImVec4& a, const ImVec4& b, float t)
{
	t = std::clamp(t, 0.0f, 1.0f);
	return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
		a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t};
}

ImVec4 HealthColor(float pct, const FrameColors& c)
{
	if (pct > 0.5f)
		return Lerp(c.hpMid, c.hpHigh, (pct - 0.5f) * 2.0f);
	return Lerp(c.hpLow, c.hpMid, pct * 2.0f);
}

ImVec4 U32ToVec4(ImU32 col)
{
	return ImGui::ColorConvertU32ToFloat4(col);
}

float CastProgress(int spawnId, int& spellIdOut, float& secondsLeft)
{
	spellIdOut = 0;
	secondsLeft = 0.0f;

	UnitCast uc;
	if (!GetSpawnCast(spawnId, uc) || uc.castMs == 0)
		return -1.0f;

	const uint32_t now = static_cast<uint32_t>(EQGetTime());
	if (now < uc.startMs)
		return -1.0f;
	const uint32_t elapsed = now - uc.startMs;
	if (elapsed >= uc.castMs)
		return -1.0f;

	spellIdOut = uc.spellId;
	secondsLeft = (uc.castMs - elapsed) / 1000.0f;
	return static_cast<float>(elapsed) / static_cast<float>(uc.castMs);
}

void DrawAuxBar(ImDrawList* dl, const ImVec2& lo, float width, float height, float pct,
	const ImVec4& color, const ImVec4& back, bool reverse)
{
	const ImVec2 hi(lo.x + width, lo.y + height);
	dl->AddRectFilled(lo, hi, ImGui::GetColorU32(back), 1.0f);
	if (pct <= 0.0f)
		return;

	if (reverse)
		dl->AddRectFilled(ImVec2(hi.x - width * pct, lo.y), hi, ImGui::GetColorU32(color), 1.0f);
	else
		dl->AddRectFilled(lo, ImVec2(lo.x + width * pct, hi.y), ImGui::GetColorU32(color), 1.0f);
}

void DrawFill(ImDrawList* dl, const ImVec2& lo, const ImVec2& hi, const ImVec4& color,
	int style, float rounding)
{
	const ImU32 base = ImGui::GetColorU32(color);
	if (hi.x <= lo.x)
		return;

	switch (style)
	{
	case BarFill::Gradient:
	{
		const ImU32 top = ImGui::GetColorU32(
			ImVec4(std::min(color.x * 1.35f, 1.0f), std::min(color.y * 1.35f, 1.0f),
				std::min(color.z * 1.35f, 1.0f), color.w));
		const ImU32 bot = ImGui::GetColorU32(
			ImVec4(color.x * 0.65f, color.y * 0.65f, color.z * 0.65f, color.w));
		dl->AddRectFilledMultiColor(lo, hi, top, top, bot, bot);
		break;
	}

	case BarFill::Glass:
	{
		dl->AddRectFilled(lo, hi, base, rounding);
		const float mid = lo.y + (hi.y - lo.y) * 0.45f;
		dl->AddRectFilled(lo, ImVec2(hi.x, mid),
			ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.18f * color.w)), rounding);
		break;
	}

	case BarFill::Stripes:
	{
		dl->AddRectFilled(lo, hi, base, rounding);
		const ImU32 dark = ImGui::GetColorU32(ImVec4(0, 0, 0, 0.18f));
		for (float x = lo.x; x < hi.x; x += 8.0f)
		{
			dl->AddRectFilled(ImVec2(x, lo.y), ImVec2(std::min(x + 3.0f, hi.x), hi.y), dark);
		}
		break;
	}

	case BarFill::Segments:
	{
		dl->AddRectFilled(lo, hi, base, rounding);
		const ImU32 dark = ImGui::GetColorU32(ImVec4(0, 0, 0, 0.45f));
		const float step = (hi.x - lo.x) / 10.0f;
		for (int i = 1; i < 10; ++i)
		{
			const float x = lo.x + step * i;
			if (x >= hi.x)
				break;
			dl->AddLine(ImVec2(x, lo.y), ImVec2(x, hi.y), dark, 1.0f);
		}
		break;
	}

	case BarFill::Flat:
	default:
		dl->AddRectFilled(lo, hi, base, rounding);
		break;
	}
}

float TextX(float lo, float hi, float textWidth, int align)
{
	switch (align)
	{
	case Align::Center: return lo + ((hi - lo) - textWidth) * 0.5f;
	case Align::Right:  return hi - textWidth;
	case Align::Left:
	default:            return lo;
	}
}

float SnapSize(float size, const TextStyle& ts)
{
	if (!ts.pixelSnap)
		return size;
	return std::max(1.0f, std::floor(size + 0.5f));
}

ImVec2 SnapPos(const ImVec2& p, const TextStyle& ts)
{
	if (!ts.pixelSnap)
		return p;
	return {std::floor(p.x + 0.5f), std::floor(p.y + 0.5f)};
}

void DrawLabel(ImDrawList* dl, const ImVec2& rawPos, ImU32 col, const char* text,
	float size, const TextStyle& ts, ImU32 edgeCol)
{
	ImFont* font = ImGui::GetFont();
	const ImVec2 pos = SnapPos(rawPos, ts);
	const float d = ts.pixelSnap ? std::max(1.0f, std::floor(ts.edgeOffset + 0.5f))
		: ts.edgeOffset;

	if (d > 0.0f)
	{
		switch (ts.edge)
		{
		case TextEdge::Shadow:
			dl->AddText(font, size, ImVec2(pos.x + d, pos.y + d), edgeCol, text);
			break;

		case TextEdge::Outline:
			dl->AddText(font, size, ImVec2(pos.x - d, pos.y), edgeCol, text);
			dl->AddText(font, size, ImVec2(pos.x + d, pos.y), edgeCol, text);
			dl->AddText(font, size, ImVec2(pos.x, pos.y - d), edgeCol, text);
			dl->AddText(font, size, ImVec2(pos.x, pos.y + d), edgeCol, text);
			break;

		default:
			break;
		}
	}

	dl->AddText(font, size, pos, col, text);
}

void FormatDuration(int ms, char* out, size_t size)
{
	if (ms < 0)
	{
		std::snprintf(out, size, "perm");
		return;
	}
	if (ms >= 60000)
		std::snprintf(out, size, "%.1fm", ms / 60000.0);
	else
		std::snprintf(out, size, "%ds", ms / 1000);
}

enum class BuffOwner
{
	None,
	Me,
	Pet,
};

struct RowBuff
{
	int  spellId = 0;
	int  icon = 0;
	bool beneficial = false;
	bool mine = false;
	int  ms = -1;
	char caster[64] = { 0 };
	int  slot = -1;
	BuffOwner owner = BuffOwner::None;
};

struct BuffCache
{
	std::vector<RowBuff> list;
	uint32_t             built = 0;
	uint32_t             seen = 0;
};

std::unordered_map<int, BuffCache> s_buffCache;
uint32_t s_buffSweep = 0;

constexpr size_t kMaxPendingDrops = 256;
std::atomic<bool> s_flushAll{ false };
std::mutex        s_dropMutex;
std::vector<int>  s_dropIds;

PlayerClient* LiveSpawn(PlayerClient* sp)
{
	if (!sp)
		return nullptr;
	for (PlayerClient* p = pSpawnList; p; p = p->pNext)
		if (p == sp)
			return sp;
	return nullptr;
}

bool IsMyPet(const PlayerClient* sp)
{
	const PlayerClient* me = pLocalPlayer.get();
	return me && sp && me->PetID > 0 && static_cast<int>(me->PetID) == sp->SpawnID;
}

void AppendAffect(const EQ_Affect& affect, int slot, std::vector<RowBuff>& out)
{
	if (affect.SpellID <= 0)
		return;

	EQ_Spell* spell = GetSpellByID(affect.SpellID);
	if (!spell)
		return;

	RowBuff b;
	b.spellId = affect.SpellID;
	b.icon = spell->SpellIcon;
	b.beneficial = spell->IsBeneficialSpell();
	b.mine = true;
	b.slot = slot;
	b.owner = BuffOwner::Me;

	b.ms = affect.Duration > 0 ? affect.Duration * 6000 : -1;
	out.push_back(b);
}

void AppendMyBuffs(std::vector<RowBuff>& out)
{
	PcProfile* profile = GetPcProfile();
	if (!profile)
		return;

	const int total = profile->GetMaxEffects();
	for (int i = 0; i < total; ++i)
		AppendAffect(profile->GetEffect(i), i, out);
}

void AppendCachedBuffs(PlayerClient* sp, std::vector<RowBuff>& out)
{
	const char* myName = pLocalPlayer ? pLocalPlayer->Name : "";

	for (const CachedBuff& cb : FilterCachedBuffs(sp, [](const CachedBuff&) { return true; }))
	{
		EQ_Spell* spell = GetSpellByID(cb.spellId);
		if (!spell)
			continue;

		RowBuff b;
		b.spellId = cb.spellId;
		b.icon = spell->SpellIcon;
		b.beneficial = spell->IsBeneficialSpell();
		b.ms = cb.duration < 0 ? -1 : static_cast<int>(cb.Duration());
		std::strncpy(b.caster, cb.casterName, sizeof(b.caster) - 1);
		b.mine = ci_equals(cb.casterName, myName);
		out.push_back(b);
	}
}

void AppendPetBuffs(std::vector<RowBuff>& out)
{
	CPetInfoWnd* wnd = pPetInfoWnd.get();
	if (!wnd)
		return;

	const char* myName = pLocalPlayer ? pLocalPlayer->Name : "";
	const int total = wnd->GetMaxBuffs();

	for (int i = 0; i < total; ++i)
	{
		const int spellId = wnd->GetBuff(i);
		if (spellId <= 0)
			continue;

		EQ_Spell* spell = GetSpellByID(spellId);
		if (!spell)
			continue;

		RowBuff b;
		b.spellId = spellId;
		b.icon = spell->SpellIcon;
		b.beneficial = spell->IsBeneficialSpell();
		b.ms = wnd->GetBuffTimer(i);
		b.slot = i;
		b.owner = BuffOwner::Pet;
		if (const char* caster = wnd->GetCasterName(i))
		{
			std::strncpy(b.caster, caster, sizeof(b.caster) - 1);
			b.mine = ci_equals(caster, myName);
		}
		out.push_back(b);
	}
}

void BuildBuffs(PlayerClient* sp, const BuffStyle& bs, std::vector<RowBuff>& out)
{
	out.clear();
	if (!sp)
		return;

	if (pLocalPlayer && sp->SpawnID == pLocalPlayer->SpawnID)
	{
		AppendMyBuffs(out);
		return;
	}

	if (IsMyPet(sp))
	{
		if (bs.petFromWindow)
		{
			AppendPetBuffs(out);
			if (!out.empty())
				return;
		}

		AppendCachedBuffs(sp, out);
		if (out.empty() && !bs.petFromWindow)
			UnitsWantBuffs(sp->SpawnID);
		return;
	}

	AppendCachedBuffs(sp, out);
	if (out.empty())
		UnitsWantBuffs(sp->SpawnID);
}

const std::vector<RowBuff>& BuffsFor(PlayerClient* sp, const BuffStyle& bs)
{
	static const std::vector<RowBuff> empty;
	if (!sp)
		return empty;

	const uint32_t now = static_cast<uint32_t>(EQGetTime());

	if (now - s_buffSweep > 10000)
	{
		s_buffSweep = now;
		for (auto it = s_buffCache.begin(); it != s_buffCache.end(); )
			it = (now - it->second.seen > 30000) ? s_buffCache.erase(it) : std::next(it);
	}

	BuffCache& c = s_buffCache[sp->SpawnID];
	c.seen = now;

	if (c.built == 0 || now - c.built >= static_cast<uint32_t>(std::max(bs.refreshMs, 16)))
	{
		BuildBuffs(sp, bs, c.list);
		c.built = now;
	}

	return c.list;
}

std::string Scramble(const char* name)
{
	std::string s(name ? name : "");
	uint32_t h = 2166136261u;
	for (char ch : s)
		h = (h ^ static_cast<unsigned char>(ch)) * 16777619u;

	for (size_t i = s.size(); i > 1; --i)
	{
		h = h * 1664525u + 1013904223u;
		std::swap(s[i - 1], s[h % i]);
	}
	return s;
}

} // namespace

float PctHP(const PlayerClient* sp)
{
	return sp ? Ratio(sp->HPCurrent, sp->HPMax) : 0.0f;
}

float PctMana(const PlayerClient* sp)
{
	return sp ? Ratio(sp->ManaCurrent, sp->ManaMax) : 0.0f;
}

float PctEndurance(const PlayerClient* sp)
{
	return sp ? Ratio(sp->EnduranceCurrent, sp->EnduranceMax) : 0.0f;
}

ImU32 ClassColor(const PlayerClient* sp, float alpha)
{
	if (sp)
	{
		if (const eqgfx::gpu::ClassFx* fx = eqgfx::gpu::FindClassFxById(sp->GetClass()))
		{
			const float* c = fx->defs[0].rgba;
			return ImGui::GetColorU32(ImVec4(c[0], c[1], c[2], alpha));
		}
	}
	return ImGui::GetColorU32(ImVec4(0.75f, 0.75f, 0.75f, alpha));
}

ImU32 ConColorOf(const PlayerClient* sp, float alpha)
{
	if (!sp)
		return ImGui::GetColorU32(ImVec4(0.75f, 0.75f, 0.75f, alpha));

	const uint32_t argb = ConColorToARGB(ConColor(const_cast<PlayerClient*>(sp)));
	return ImGui::GetColorU32(ImVec4(((argb >> 16) & 0xFF) / 255.0f,
		((argb >> 8) & 0xFF) / 255.0f, (argb & 0xFF) / 255.0f, alpha));
}

const char* ClassShort(const PlayerClient* sp)
{
	if (!sp)
		return "";
	const int cls = sp->GetClass();
	if (cls > 0 && cls < static_cast<int>(std::size(eqlib::ClassInfo)))
		return eqlib::ClassInfo[cls].UCShortName;
	return "";
}

std::string DisplayName(const PlayerClient* sp, const TextStyle& text)
{
	if (!sp)
		return {};

	std::string name = sp->DisplayedName;

	const bool isPC = sp->Type == SPAWN_PLAYER;
	if (text.anonMode != Anon::Off && (isPC || text.anonNPCs))
	{
		switch (text.anonMode)
		{
		case Anon::ClassName:
		{
			const int cls = sp->GetClass();
			if (cls > 0 && cls < static_cast<int>(std::size(eqlib::ClassInfo)))
				name = eqlib::ClassInfo[cls].LongName;
			break;
		}
		case Anon::ClassShort:
		{
			const char* s = ClassShort(sp);
			if (s && *s)
				name = s;
			break;
		}
		case Anon::Scramble:
			name = Scramble(name.c_str());
			break;
		case Anon::Stars:
			name.assign(name.size(), '*');
			break;
		case Anon::Stars8:
			name = "********";
			break;
		case Anon::FirstLast:
			if (name.size() > 2)
				name = name.substr(0, 1) + std::string(name.size() - 2, '*') + name.back();
			break;
		default:
			break;
		}
	}

	if (text.maxNameChars > 0 && static_cast<int>(name.size()) > text.maxNameChars)
		name = name.substr(0, static_cast<size_t>(text.maxNameChars));

	return name;
}

int GroupAggroPct(int groupIndex)
{
	if (!pAggroInfo || groupIndex < 0 || groupIndex >= MAX_GROUP_SIZE)
		return -1;

	const int slot = (groupIndex == 0) ? AD_Player : (AD_Group1 + groupIndex - 1);
	if (slot >= MAX_AGGRO_METER_SIZE)
		return -1;

	return pAggroInfo->aggroData[slot].AggroPct;
}

int XTargetAggroPct(int slotIndex)
{
	if (!pAggroInfo || slotIndex < 0)
		return -1;

	const int slot = AD_xTarget1 + slotIndex;
	if (slot >= MAX_AGGRO_METER_SIZE)
		return -1;

	return pAggroInfo->aggroData[slot].AggroPct;
}

int HoverWindowIndex()
{
	return s_hoverWindow.load(std::memory_order_relaxed);
}

void SetHoverWindow(Window w)
{
	s_hoverWindow.store(static_cast<int>(w), std::memory_order_relaxed);
}

void ClearHoverWindow()
{
	s_hoverWindow.store(-1, std::memory_order_relaxed);
}

void ForgetUnits()
{
	s_flushAll.store(true, std::memory_order_relaxed);
}

void ForgetUnit(int spawnId)
{
	if (spawnId == 0 || s_flushAll.load(std::memory_order_relaxed))
		return;

	std::scoped_lock lock(s_dropMutex);
	if (s_dropIds.size() >= kMaxPendingDrops)
		s_flushAll.store(true, std::memory_order_relaxed);
	else
		s_dropIds.push_back(spawnId);
}

void ApplyForgets()
{
	if (s_flushAll.exchange(false, std::memory_order_relaxed))
	{
		s_anim.clear();
		s_buffCache.clear();

		std::scoped_lock lock(s_dropMutex);
		s_dropIds.clear();
		return;
	}

	std::vector<int> drops;
	{
		std::scoped_lock lock(s_dropMutex);
		if (s_dropIds.empty())
			return;
		drops.swap(s_dropIds);
	}

	for (int id : drops)
	{
		s_anim.erase(id);
		s_buffCache.erase(id);
	}
}

PlayerClient* ValidateSpawn(PlayerClient* sp)
{
	return LiveSpawn(sp);
}

ImVec2 MousePos()
{
	if (MQMouseInfo* m = EQADDR_MOUSE)
		return {static_cast<float>(m->X), static_cast<float>(m->Y)};
	return ImGui::GetIO().MousePos;
}

void BeginRowCapture(Window w)
{
	const int i = static_cast<int>(w);
	if (i >= 0 && i < kWindowCount)
	{
		s_rowRects[i].clear();
		s_rowIds[i].clear();
	}
	if (s_menuWindow == i)
		s_menuWindow = -1;
}

void NoteHotRect(Window w, const ImVec2& lo, const ImVec2& hi)
{
	const int i = static_cast<int>(w);
	if (i >= 0 && i < kWindowCount)
		s_rowRects[i].emplace_back(lo.x, lo.y, hi.x, hi.y);
}

void NoteHotItem(Window w)
{
	NoteHotRect(w, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

int MenuOpenWindow()
{
	return s_menuWindow;
}

const std::vector<ImVec4>& RowRects(Window w)
{
	const int i = static_cast<int>(w);
	static const std::vector<ImVec4> empty;
	return (i >= 0 && i < kWindowCount) ? s_rowRects[i] : empty;
}

void DebugDrawHotRects(Window w, const ImVec4& colour)
{
	const int i = static_cast<int>(w);
	if (i < 0 || i >= kWindowCount)
		return;

	ImDrawList* dl = ImGui::GetForegroundDrawList();
	const ImU32 line = ImGui::GetColorU32(colour);
	ImVec4 wash = colour;
	wash.w *= 0.15f;
	const ImU32 fill = ImGui::GetColorU32(wash);

	for (const ImVec4& r : s_rowRects[i])
	{
		dl->AddRectFilled(ImVec2(r.x, r.y), ImVec2(r.z, r.w), fill);
		dl->AddRect(ImVec2(r.x, r.y), ImVec2(r.z, r.w), line, 0.0f, 0, 1.0f);
	}
}

bool IsGemReady(int gem)
{
	if (gem < 1 || gem > NUM_SPELL_GEMS)
		return false;

	PlayerClient* me = pLocalPlayer.get();
	if (!me || !pDisplay)
		return false;

	if (!GetSpellByID(GetMemorizedSpell(gem - 1)))
		return false;

	return pDisplay->TimeStamp > me->SpellGemETA[gem - 1]
		&& pDisplay->TimeStamp > me->GetSpellCooldownETA();
}

int PickRotationGem(const Rotation& r)
{
	for (int i = 0; i < r.count; ++i)
	{
		if (IsGemReady(r.gems[i]))
			return r.gems[i];
	}
	return 0;
}

bool FireRotation(const Rotation& r, int spawnId)
{
	if (r.empty() || spawnId == 0)
		return false;

	const int gem = PickRotationGem(r);
	if (gem == 0)
		return false;

	return CastAtSpawn(spawnId, gem);
}

void DrawBuffIcons(PlayerClient* sp, const WindowSettings& cfg)
{
	if (!sp || !cfg.buffs.show)
		return;

	const BuffStyle& bs = cfg.buffs;
	const std::vector<RowBuff>& buffs = BuffsFor(sp, bs);
	if (buffs.empty())
		return;

	CTextureAnimation* icons = ui::SpellIconSheet();

	int drawn = 0;
	int inRow = 0;

	for (const RowBuff& b : buffs)
	{
		if (drawn >= bs.maxIcons)
			break;
		if (bs.mineOnly && !b.mine)
			continue;
		if (bs.filter == BuffFilter::Beneficial && !b.beneficial)
			continue;
		if (bs.filter == BuffFilter::Detrimental && b.beneficial)
			continue;

		if (inRow > 0 && inRow < bs.perRow)
			ImGui::SameLine(0.0f, bs.spacing);
		else if (inRow >= bs.perRow)
			inRow = 0;

		ImGui::PushID(drawn);

		const ImVec2 lo = ImGui::GetCursorScreenPos();
		const ImVec2 hi(lo.x + bs.iconSize, lo.y + bs.iconSize);

		if (icons && icons->GetWidth() > 0)
		{
			icons->SetCurCell(b.icon);
			mq::imgui::DrawTextureAnimation(icons,
				CXSize(static_cast<int>(bs.iconSize), static_cast<int>(bs.iconSize)));
		}
		else
		{
			ImGui::ColorButton("##b",
				b.beneficial ? cfg.colors.buffBorderBen : cfg.colors.buffBorderDet,
				ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop,
				ImVec2(bs.iconSize, bs.iconSize));
		}

		ImDrawList* dl = ImGui::GetWindowDrawList();

		if (bs.dimOthers && !b.mine)
			dl->AddRectFilled(lo, hi, ImGui::GetColorU32(ImVec4(0, 0, 0, 0.45f)));

		if (bs.borders)
		{
			const ImVec4& col = b.mine ? cfg.colors.buffBorderMine
				: (b.beneficial ? cfg.colors.buffBorderBen : cfg.colors.buffBorderDet);
			dl->AddRect(lo, hi, ImGui::GetColorU32(col), 0.0f, 0, 1.5f);
		}

		if (bs.timers && b.ms >= 0)
		{
			char dur[16];
			FormatDuration(b.ms, dur, sizeof(dur));
			const float size = SnapSize(std::max(8.0f, bs.iconSize * 0.45f), cfg.text);
			DrawLabel(dl, ImVec2(lo.x + 1.0f, hi.y - size - 1.0f),
				ImGui::GetColorU32(cfg.colors.hpTextColor), dur, size, cfg.text,
				ImGui::GetColorU32(cfg.colors.nameShadow));
		}

		if (b.owner != BuffOwner::None && b.beneficial && ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			if (b.owner == BuffOwner::Me)
				ui::RemoveMyBuff(b.slot, b.spellId);
			else
				ui::RemovePetBuff(b.slot, b.spellId);
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			ui::InspectSpell(b.spellId);

		if (bs.tooltip && ImGui::IsItemHovered())
			ui::SpellTooltip(GetSpellByID(b.spellId), b.ms, true, b.caster);

		ImGui::PopID();

		++drawn;
		++inRow;
	}
}

RowResult DrawUnitRow(PlayerClient* sp, Window src, const RowOptions& opts, float rawWidth,
	const MenuItems& extra)
{
	RowResult result;
	if (!sp)
		return result;

	const float width = std::max(rawWidth, 1.0f);

	const WindowSettings& cfg = SettingsFor(src);
	const FrameColors& col = cfg.colors;
	const BarStyle& bar = cfg.bar;

	const int id = sp->SpawnID;
	const int wi = static_cast<int>(src);

	int seq = 0;
	if (wi >= 0 && wi < kWindowCount)
	{
		for (int prev : s_rowIds[wi])
			seq += (prev == id);
		s_rowIds[wi].push_back(id);
	}
	ImGui::PushID(id);
	ImGui::PushID(seq);

	const double now = ImGui::GetTime();
	const float dt = ImGui::GetIO().DeltaTime;

	PlayerClient* pet = opts.pet && sp->PetID > 0 ? GetSpawnByID(sp->PetID) : nullptr;

	const float bandH = opts.height > 0.0f ? opts.height : bar.height;
	const int auxBars = opts.nameOnly ? 0
		: ((opts.mana ? 1 : 0) + (opts.endurance ? 1 : 0) + (pet ? 1 : 0));

	const bool castUnder = opts.castBar && cfg.cast.show && !cfg.cast.overlay && !opts.nameOnly;
	float rowH = bandH + auxBars * (bar.auxHeight + bar.auxGap);
	if (castUnder)
		rowH += cfg.cast.height + bar.auxGap;

	const ImVec2 start = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##row", ImVec2(std::max(width, 1.0f), std::max(rowH, 1.0f)));

	const ImGuiID rowId = ImGui::GetItemID();
	const bool hovered = ImGui::IsItemHovered();
	const bool clickedL = ImGui::IsItemClicked(ImGuiMouseButton_Left);
	const bool clickedM = ImGui::IsItemClicked(ImGuiMouseButton_Middle);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	const ImVec2 end(start.x + width, start.y + rowH);
	const ImVec2 bandEnd(end.x, start.y + bandH);

	{
		if (wi >= 0 && wi < kWindowCount)
			s_rowRects[wi].emplace_back(start.x, start.y, end.x, end.y);
	}

	const float rawPct = PctHP(sp);
	RowAnim& anim = AnimFor(id, now);

	if (anim.last < 0.0f)
	{
		anim.last = rawPct;
		anim.fallback = rawPct;
	}

	if (cfg.anim.damageFlash && anim.last >= 0.0f)
	{
		const float delta = rawPct - anim.last;
		if (-delta >= cfg.anim.flashThreshold)
		{
			anim.flashAt = now;
			anim.flashHeal = false;
		}
		else if (cfg.anim.healFlash && delta >= cfg.anim.flashThreshold)
		{
			anim.flashAt = now;
			anim.flashHeal = true;
		}
	}
	anim.last = rawPct;

	float display = rawPct;
	if (cfg.anim.hpSmooth)
	{
		if (cfg.anim.useImAnim && iam_context_get_current())
		{
			const float dur = 1.0f / std::max(cfg.anim.hpSpeed, 0.1f);
			display = iam_tween_float(ImGui::GetID("##hp"), kChanHp, rawPct, dur,
				iam_ease_preset(iam_ease_out_cubic), iam_policy_crossfade, dt, rawPct);
		}
		else
		{
			anim.fallback += (rawPct - anim.fallback) * std::clamp(dt * cfg.anim.hpSpeed, 0.0f, 1.0f);
			display = anim.fallback;
		}
	}

	const float pct = std::clamp(display, 0.0f, 1.0f);
	const bool dead = sp->HPCurrent <= 0 && sp->HPMax > 0;

	dl->AddRectFilled(start, end,
		ImGui::GetColorU32(dead ? col.dead : col.bg), bar.rounding);

	ImVec4 fillColor;
	switch (bar.colorMode)
	{
	case BarColor::Class: fillColor = U32ToVec4(ClassColor(sp, col.hpFixed.w)); break;
	case BarColor::Con:   fillColor = U32ToVec4(ConColorOf(sp, col.hpFixed.w)); break;
	case BarColor::Fixed: fillColor = col.hpFixed; break;
	case BarColor::Health:
	default:              fillColor = HealthColor(pct, col); break;
	}

	if (cfg.anim.lowHpPulse && rawPct > 0.0f && rawPct <= cfg.anim.lowHpThreshold)
	{
		const float t = 0.5f + 0.5f * std::sin(static_cast<float>(now)
			* cfg.anim.lowHpSpeed * 6.2831853f);
		fillColor = Lerp(fillColor, col.lowHp, t * 0.6f);
	}

	const float fx0 = start.x + bar.inset;
	const float fx1 = bandEnd.x - bar.inset;
	const float fy0 = start.y + bar.inset;
	const float fy1 = bandEnd.y - bar.inset;

	if (bar.hpBackdrop)
		dl->AddRectFilled(ImVec2(fx0, fy0), ImVec2(fx1, fy1),
			ImGui::GetColorU32(col.auxBack), bar.rounding);

	if (fx1 > fx0 && pct > 0.0f)
	{
		const float w = (fx1 - fx0) * pct;
		if (bar.reverse)
			DrawFill(dl, ImVec2(fx1 - w, fy0), ImVec2(fx1, fy1), fillColor, bar.fill, bar.rounding);
		else
			DrawFill(dl, ImVec2(fx0, fy0), ImVec2(fx0 + w, fy1), fillColor, bar.fill, bar.rounding);
	}

	if (src == Window::Player && sp == pLocalPlayer.get())
		DrawAttackHealthBar(dl, start.x, start.y, bandEnd.x, bandEnd.y, bar.rounding);

	if (anim.flashAt > 0.0 && now - anim.flashAt < cfg.anim.flashDur)
	{
		const float t = 1.0f - static_cast<float>((now - anim.flashAt) / cfg.anim.flashDur);
		ImVec4 flash = col.flash;
		if (anim.flashHeal)
			flash = ImVec4(0.4f, 1.0f, 0.5f, flash.w);
		flash.w *= t;
		dl->AddRectFilled(start, bandEnd, ImGui::GetColorU32(flash), bar.rounding);
	}

	if (bar.classStrip > 0.0f)
		dl->AddRectFilled(start, ImVec2(start.x + bar.classStrip, end.y),
			ClassColor(sp), bar.rounding);

	float barY = bandEnd.y + bar.auxGap;
	const float barX = start.x + bar.auxInset;
	const float barW = std::max(width - bar.auxInset * 2.0f, 1.0f);

	if (!opts.nameOnly)
	{
		if (opts.mana)
		{
			DrawAuxBar(dl, ImVec2(barX, barY), barW, bar.auxHeight, PctMana(sp),
				col.mana, col.auxBack, bar.reverse);
			barY += bar.auxHeight + bar.auxGap;
		}
		if (opts.endurance)
		{
			DrawAuxBar(dl, ImVec2(barX, barY), barW, bar.auxHeight, PctEndurance(sp),
				col.endurance, col.auxBack, bar.reverse);
			barY += bar.auxHeight + bar.auxGap;
		}
		if (pet)
		{
			DrawAuxBar(dl, ImVec2(barX, barY), barW, bar.auxHeight, PctHP(pet),
				col.petHp, col.auxBack, bar.reverse);
			barY += bar.auxHeight + bar.auxGap;
		}
	}

	const int focusId = GetSecondaryTarget();
	const bool isFocus = (id == focusId);
	const bool isTarget = (pTarget && static_cast<int>(pTarget->SpawnID) == id);

	if (hovered)
	{
		dl->AddRect(start, end, ImGui::GetColorU32(col.hoverBorder), bar.rounding, 0,
			bar.borderThickness + 0.5f);
	}
	else if (isFocus)
	{
		dl->AddRect(start, end, ImGui::GetColorU32(col.focusBorder), bar.rounding, 0,
			bar.borderThickness + 0.5f);
	}
	else if (isTarget)
	{
		ImVec4 tb = col.targetBorder;
		if (cfg.anim.targetGlow)
		{
			const float t = 0.5f + 0.5f * std::sin(static_cast<float>(now)
				* cfg.anim.glowSpeed * 6.2831853f);
			tb.w *= 0.45f + 0.55f * t;
		}
		dl->AddRect(start, end, ImGui::GetColorU32(tb), bar.rounding, 0, bar.borderThickness);
	}
	else if (bar.borderAlways)
	{
		dl->AddRect(start, end, ImGui::GetColorU32(col.idleBorder), bar.rounding, 0,
			bar.borderThickness);
	}

	const float fontSize = SnapSize(ImGui::GetFontSize() * cfg.text.scale, cfg.text);
	const float textY = start.y + (bandH - fontSize) * 0.5f;
	const float padX = std::max(bar.classStrip, 2.0f) + 3.0f;

	dl->PushClipRect(ImVec2(start.x + 1.0f, start.y), ImVec2(bandEnd.x - 1.0f, bandEnd.y), true);

	char tagLine[16] = { 0 };
	if (cfg.text.showTag && opts.tag && *opts.tag && !opts.nameOnly)
		std::snprintf(tagLine, sizeof(tagLine), "%s", opts.tag);

	char nameLine[192] = { 0 };
	if (cfg.text.name)
	{
		const std::string shown = DisplayName(sp, cfg.text);
		int n = 0;

		if (cfg.text.showLevel && !opts.nameOnly)
			n += std::snprintf(nameLine + n, sizeof(nameLine) - n, "%d ", sp->Level);

		n += std::snprintf(nameLine + n, sizeof(nameLine) - n, "%s", shown.c_str());

		if (cfg.text.showClass && !opts.nameOnly)
			n += std::snprintf(nameLine + n, sizeof(nameLine) - n, " %s", ClassShort(sp));
		if (cfg.text.showDistance && !opts.nameOnly)
			n += std::snprintf(nameLine + n, sizeof(nameLine) - n, " %.0fm", GetDistance3D(pLocalPlayer.get(), sp));
		if (cfg.text.showGuild && !opts.nameOnly
			&& sp->GuildID != 0 && sp->GuildID != static_cast<uint64_t>(-1))
		{
			if (const char* g = GetGuildByID(sp->GuildID))
				std::snprintf(nameLine + n, sizeof(nameLine) - n, " <%s>", g);
		}
	}

	char hpLine[48] = { 0 };
	switch (cfg.text.hpText)
	{
	case HpText::Percent:    std::snprintf(hpLine, sizeof(hpLine), "%.0f%%", rawPct * 100.0f); break;
	case HpText::Current:    std::snprintf(hpLine, sizeof(hpLine), "%lld", static_cast<long long>(sp->HPCurrent)); break;
	case HpText::CurrentMax: std::snprintf(hpLine, sizeof(hpLine), "%lld/%lld",
		static_cast<long long>(sp->HPCurrent), static_cast<long long>(sp->HPMax)); break;
	case HpText::Deficit:    std::snprintf(hpLine, sizeof(hpLine), "-%lld",
		static_cast<long long>(sp->HPMax - sp->HPCurrent)); break;
	default: break;
	}

	char aggroLine[16] = { 0 };
	if (opts.aggro && opts.aggroPct >= 0)
		std::snprintf(aggroLine, sizeof(aggroLine), "%d%%", opts.aggroPct);

	const ImU32 shadowCol = ImGui::GetColorU32(col.nameShadow);

	float rightEdge = bandEnd.x - 4.0f;

	if (aggroLine[0])
	{
		const ImVec2 sz = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, aggroLine);
		const ImVec4 ac = opts.aggroPct >= 100 ? col.aggroHigh
			: opts.aggroPct >= 75 ? col.aggroMid : col.aggroLow;
		DrawLabel(dl, ImVec2(rightEdge - sz.x, textY), ImGui::GetColorU32(ac), aggroLine,
			fontSize, cfg.text, shadowCol);
		rightEdge -= sz.x + 5.0f;
	}

	float leftEdge = start.x + padX;

	if (tagLine[0])
	{
		const ImVec2 sz = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, tagLine);
		DrawLabel(dl, ImVec2(leftEdge, textY), ImGui::GetColorU32(col.tag), tagLine,
			fontSize, cfg.text, shadowCol);
		leftEdge += sz.x + 4.0f;
	}

	if (hpLine[0])
	{
		const ImVec2 sz = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, hpLine);
		const float x = (cfg.text.hpAlign == Align::Left) ? leftEdge
			: (cfg.text.hpAlign == Align::Center)
			? TextX(start.x + padX, bandEnd.x - 4.0f, sz.x, Align::Center)
			: rightEdge - sz.x;
		DrawLabel(dl, ImVec2(x, textY), ImGui::GetColorU32(col.hpTextColor), hpLine,
			fontSize, cfg.text, shadowCol);

		if (cfg.text.hpAlign == Align::Left)
			leftEdge += sz.x + 5.0f;
		else if (cfg.text.hpAlign == Align::Right)
			rightEdge -= sz.x + 5.0f;
	}

	if (nameLine[0])
	{
		const ImVec2 sz = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, nameLine);
		const float x = TextX(leftEdge, std::max(rightEdge, leftEdge + sz.x),
			sz.x, cfg.text.nameAlign);

		ImU32 nameCol = ImGui::GetColorU32(col.name);
		if (cfg.text.conColorName)
			nameCol = ConColorOf(sp, col.name.w);
		else if (cfg.text.classColorName)
			nameCol = ClassColor(sp, col.name.w);

		DrawLabel(dl, ImVec2(x, textY), nameCol, nameLine, fontSize, cfg.text, shadowCol);
	}

	dl->PopClipRect();

	int spellId = 0;
	float castLeft = 0.0f;
	const float progress = (opts.castBar && cfg.cast.show && !opts.nameOnly
		&& (!cfg.cast.onlyTarget || isTarget))
		? CastProgress(id, spellId, castLeft) : -1.0f;

	if (progress >= 0.0f)
	{
		const float cy1 = cfg.cast.overlay ? bandEnd.y - 1.0f : barY + cfg.cast.height;
		const float cy0 = cy1 - cfg.cast.height;
		const float cx0 = start.x + bar.auxInset;
		const float cx1 = bandEnd.x - bar.auxInset;

		dl->AddRectFilled(ImVec2(cx0, cy0), ImVec2(cx1, cy1),
			ImGui::GetColorU32(col.castBack), 2.0f);
		dl->AddRectFilled(ImVec2(cx0, cy0),
			ImVec2(cx0 + (cx1 - cx0) * progress, cy1),
			ImGui::GetColorU32(col.castFill), 2.0f);

		char castText[128] = { 0 };
		int n = 0;
		if (cfg.cast.spellName)
		{
			if (EQ_Spell* spell = GetSpellByID(spellId))
				n += std::snprintf(castText, sizeof(castText), "%s", spell->Name);
		}
		if (cfg.cast.timeText)
			std::snprintf(castText + n, sizeof(castText) - n, "%s%.1fs", n ? "  " : "", castLeft);

		if (castText[0] && cfg.cast.textPlace != CastText::Off)
		{
			const float size = SnapSize(
				std::max(7.0f, fontSize * cfg.cast.textScale), cfg.text);

			float ty = cy0 + (cfg.cast.height - size) * 0.5f;
			ImVec2 clipLo(cx0, cy0);
			ImVec2 clipHi(cx1, cy1);

			if (cfg.cast.textPlace == CastText::Above)
			{
				ty = cy0 - size - 1.0f;
				clipLo.y = ty;
			}
			else if (cfg.cast.textPlace == CastText::Below)
			{
				ty = cy1 + 1.0f;
				clipHi.y = ty + size;
			}

			dl->PushClipRect(clipLo, clipHi, true);
			DrawLabel(dl, ImVec2(cx0 + 3.0f, ty), ImGui::GetColorU32(col.castText),
				castText, size, cfg.text, shadowCol);
			dl->PopClipRect();
		}
	}

	const Rotation& leftBind = cfg.binds[static_cast<int>(Action::Left)];
	const Rotation& rightBind = cfg.binds[static_cast<int>(Action::Right)];
	const Rotation& hotkeyBind = cfg.binds[static_cast<int>(Action::Hotkey)];

	if (hovered)
	{
		SetHoverUnit(id);
		SetHoverWindow(src);
		result.hovered = true;

		if (cfg.tip.enabled)
		{
			const TooltipStyle& t = cfg.tip;
			std::string tip;
			char buf[256];

			auto line = [&tip](const char* text) {
				if (!tip.empty())
					tip += '\n';
				tip += text;
			};

			if (t.name)
				line(DisplayName(sp, cfg.text).c_str());
			if (t.realName && cfg.text.anonMode != Anon::Off)
				line(sp->DisplayedName);

			if (t.classLevel)
			{
				std::snprintf(buf, sizeof(buf), "level %d %s", sp->Level, sp->GetClassString());
				line(buf);
			}
			if (t.race)
				line(sp->GetRaceString());
			if (t.con && sp->Type != SPAWN_PLAYER)
			{
				static const char* kCon[] = { "grey", "green", "light blue", "blue",
					"white", "yellow", "red" };
				const int c = ConColor(sp);
				std::snprintf(buf, sizeof(buf), "cons %s",
					(c >= 0 && c < static_cast<int>(std::size(kCon))) ? kCon[c] : "?");
				line(buf);
			}
			if (t.hp)
			{
				if (sp->HPMax > 0 && (pLocalPlayer && sp->SpawnID == pLocalPlayer->SpawnID))
					std::snprintf(buf, sizeof(buf), "%lld / %lld hp  (%.0f%%)",
						static_cast<long long>(sp->HPCurrent),
						static_cast<long long>(sp->HPMax), rawPct * 100.0f);
				else
					std::snprintf(buf, sizeof(buf), "%.0f%% hp", rawPct * 100.0f);
				line(buf);
			}
			if (t.distance)
			{
				std::snprintf(buf, sizeof(buf), "%.0f away", GetDistance3D(pLocalPlayer.get(), sp));
				line(buf);
			}
			if (t.guild && sp->GuildID != 0 && sp->GuildID != static_cast<uint64_t>(-1))
			{
				if (const char* g = GetGuildByID(sp->GuildID))
				{
					std::snprintf(buf, sizeof(buf), "<%s>", g);
					line(buf);
				}
			}
			if (t.role && opts.role && *opts.role)
				line(opts.role);
			if (t.aggro && opts.aggroPct >= 0)
			{
				std::snprintf(buf, sizeof(buf), "aggro %d%%", opts.aggroPct);
				line(buf);
			}
			if (t.pet && pet)
			{
				std::snprintf(buf, sizeof(buf), "pet: %s  (%.0f%%)",
					pet->DisplayedName, PctHP(pet) * 100.0f);
				line(buf);
			}
			if (t.targetOf && pLocalPlayer && sp == pTarget)
			{
				if (PlayerClient* tot = GetSpawnByID(pLocalPlayer->TargetOfTarget))
				{
					std::snprintf(buf, sizeof(buf), "targeting %s", tot->DisplayedName);
					line(buf);
				}
			}
			if (t.casting && progress >= 0.0f)
			{
				if (EQ_Spell* spell = GetSpellByID(spellId))
				{
					std::snprintf(buf, sizeof(buf), "casting %s  (%.1fs)", spell->Name, castLeft);
					line(buf);
				}
			}
			if (t.binds)
			{
				std::snprintf(buf, sizeof(buf), "left %s   right %s   hotkey %s",
					leftBind.empty() ? "target" : FormatRotation(leftBind).c_str(),
					rightBind.empty() ? "-" : FormatRotation(rightBind).c_str(),
					hotkeyBind.empty() ? "-" : FormatRotation(hotkeyBind).c_str());
				line(buf);
			}

			if (!tip.empty())
				ImGui::SetTooltip("%s", tip.c_str());
		}
	}

	if (clickedL)
	{
		if (!FireRotation(leftBind, id) && leftBind.empty())
			pTarget = sp;
	}
	if (clickedM)
		pTarget = sp;

	if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		s_rightPressRow = rowId;
		s_rightPressHandled = false;
	}

	if (s_rightPressRow == rowId)
	{
		const ImGuiIO& io = ImGui::GetIO();

		if (!s_rightPressHandled && ImGui::IsMouseDown(ImGuiMouseButton_Right)
			&& io.MouseDownDuration[ImGuiMouseButton_Right] >= kMenuHoldSeconds)
		{
			s_rightPressHandled = true;
			result.menuRequested = true;
		}

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		{
			if (!s_rightPressHandled && hovered)
				FireRotation(rightBind, id);
			s_rightPressRow = 0;
			s_rightPressHandled = false;
		}
	}

	if (result.menuRequested)
		ImGui::OpenPopup("##unitmenu");

	if (ImGui::BeginPopup("##unitmenu"))
	{
		s_menuWindow = static_cast<int>(src);
		ImGui::TextDisabled("%s", DisplayName(sp, cfg.text).c_str());
		ImGui::Separator();
		if (ImGui::MenuItem("Target"))
			pTarget = sp;
		if (ImGui::MenuItem("Set focus"))
			SetSecondaryTarget(id);
		if (isFocus && ImGui::MenuItem("Clear focus"))
			SetSecondaryTarget(0);
		if (extra)
		{
			ImGui::Separator();
			extra(sp);
		}
		ImGui::EndPopup();
	}

	if (opts.buffs)
	{
		const ImVec2 top = ImGui::GetCursorScreenPos();
		DrawBuffIcons(sp, cfg);
		const float bottom = ImGui::GetCursorScreenPos().y;
		if (bottom > top.y)
			NoteHotRect(src, top, ImVec2(top.x + width, bottom));
	}

	if (bar.rowSpacing > 0.0f)
		ImGui::Dummy(ImVec2(1.0f, bar.rowSpacing));

	ImGui::PopID();
	ImGui::PopID();
	return result;
}

void ShutdownIcons()
{
	s_anim.clear();
	s_buffCache.clear();

	if (s_animCtx && iam_context_get_current() == s_animCtx)
		iam_context_set_current(iam_context_get_default_context());
	s_animPrev = nullptr;
}

} // namespace eqgfx::unitframes
