#include "Skins.h"
#include "ClassProfile.h"
#include "Widgets.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace eqgfx::ui {

namespace {

constexpr float kPad = 4.0f;
constexpr float kLongPress = 0.45f;
constexpr float kTapSlop = 6.0f;


std::unordered_map<std::string, BarLayout> s_layouts;
std::unordered_set<std::string>            s_resize;
int                                        s_layoutSerial = -1;

void LoadLayouts()
{
	s_layouts.clear();
	const settings::json& layouts = ProfileRoot()["Layout"];
	if (!layouts.is_object())
		return;
	for (const auto& [id, v] : layouts.items())
	{
		if (!v.is_object())
			continue;
		BarLayout l;
		auto get = [&](const char* key) -> const settings::json& {
			static const settings::json none;
			const auto it = v.find(key);
			return it != v.end() ? *it : none;
		};
		l.size = settings::AsFloat(get("size"), l.size);
		l.spacing = settings::AsFloat(get("spacing"), l.spacing);
		l.columns = settings::AsInt(get("columns"), l.columns);
		l.opacity = settings::AsFloat(get("opacity"), l.opacity);
		l.size = l.size > 0.0f ? std::clamp(l.size, 16.0f, 96.0f) : 0.0f;
		l.spacing = std::clamp(l.spacing, 0.0f, 16.0f);
		l.labels = settings::AsBool(get("labels"), l.labels);
		l.keys = settings::AsBool(get("keys"), l.keys);
		l.locked = settings::AsBool(get("locked"), l.locked);
		s_layouts[id] = l;
		s_resize.insert(id);
	}
}

void SaveLayouts()
{
	settings::json layouts = settings::json::object();
	for (const auto& [id, l] : s_layouts)
		layouts[id] = { { "size", l.size }, { "spacing", l.spacing }, { "columns", l.columns },
			{ "opacity", l.opacity }, { "labels", l.labels }, { "keys", l.keys }, { "locked", l.locked } };
	ProfileRoot()["Layout"] = std::move(layouts);
	MarkProfileDirty();
}

BarLayout& LayoutFor(const char* id)
{
	if (s_layoutSerial != ProfileSerial())
	{
		s_layoutSerial = ProfileSerial();
		LoadLayouts();
	}
	return s_layouts[id];
}

struct OpenBar
{
	std::string       id;
	BarLayout*        layout = nullptr;
	std::vector<Cell> cells;
	float             nativeSize = 40.0f;
	int               count = 0;
	int               stockColumns = 1;
};

OpenBar s_bar;

float CellSize(const BarLayout& l, float nativeSize)
{
	return l.size > 0.0f ? l.size : nativeSize;
}

int StockColumns(const std::vector<NativeSlot>& natives)
{
	std::vector<int> xs;
	for (const NativeSlot& s : natives)
	{
		const int x = static_cast<int>(s.offset.x + 0.5f);
		if (std::find(xs.begin(), xs.end(), x) == xs.end())
			xs.push_back(x);
	}
	return std::max(1, static_cast<int>(xs.size()));
}

int SnapColumns(float width, float cell, float gap, int n)
{
	int best = 1;
	for (int rows = n; rows >= 1; --rows)
	{
		const int cols = (n + rows - 1) / rows;
		if (cols * cell + (cols - 1) * gap <= width + 0.5f)
			best = std::max(best, cols);
	}
	return best;
}

void LayoutMenu()
{
	BarLayout& l = *s_bar.layout;
	bool changed = false;

	float size = CellSize(l, s_bar.nativeSize);
	ImGui::SetNextItemWidth(160.0f);
	if (ImGui::SliderFloat("size", &size, 16.0f, 96.0f, "%.0f px"))
	{
		l.size = size;
		changed = true;
		s_resize.insert(s_bar.id);
	}
	ImGui::SetNextItemWidth(160.0f);
	if (ImGui::SliderFloat("spacing", &l.spacing, 0.0f, 16.0f, "%.0f px"))
	{
		changed = true;
		s_resize.insert(s_bar.id);
	}

	bool grid = l.columns > 0;
	if (ImGui::Checkbox("grid", &grid))
	{
		l.columns = grid ? s_bar.count : 0;
		changed = true;
		s_resize.insert(s_bar.id);
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("a grid follows the window's width; off, the bar keeps the stock window's shape");
	if (grid)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.0f);
		if (ImGui::SliderInt("columns", &l.columns, 1, std::max(1, s_bar.count)))
		{
			changed = true;
			s_resize.insert(s_bar.id);
		}
	}

	ImGui::SetNextItemWidth(160.0f);
	changed |= ImGui::SliderFloat("background", &l.opacity, 0.0f, 1.0f, "%.2f");
	changed |= ImGui::Checkbox("labels", &l.labels);
	ImGui::SameLine();
	changed |= ImGui::Checkbox("keys", &l.keys);
	ImGui::SameLine();
	changed |= ImGui::Checkbox("lock", &l.locked);

	if (ImGui::Button("reset"))
	{
		l = BarLayout{};
		l.columns = s_bar.stockColumns;
		changed = true;
		s_resize.insert(s_bar.id);
	}

	if (changed)
		SaveLayouts();
}

} // namespace

const std::vector<Cell>& BeginBar(const char* id, const CXWnd* native, const std::vector<NativeSlot>& natives)
{
	s_bar.id = id;
	s_bar.layout = &LayoutFor(id);
	s_bar.count = static_cast<int>(natives.size());
	s_bar.nativeSize = natives.empty() ? 40.0f : std::max(natives[0].size.x, 8.0f);
	s_bar.stockColumns = StockColumns(natives);
	s_bar.cells.clear();

	BarLayout& l = *s_bar.layout;
	if (l.columns < 0)
		l.columns = StockColumns(natives);
	const float cell = CellSize(l, s_bar.nativeSize);
	const float gap = l.spacing;
	const int n = std::max(s_bar.count, 1);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing
		| ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings;
	if (l.locked)
		flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

	const bool grid = l.columns > 0;
	if (grid)
	{
		const int cols = std::clamp(l.columns, 1, n);
		const int rows = (n + cols - 1) / cols;
		const float height = kPad * 2.0f + rows * cell + (rows - 1) * gap;
		ImGui::SetNextWindowSizeConstraints(ImVec2(kPad * 2.0f + cell, height),
			ImVec2(kPad * 2.0f + n * cell + (n - 1) * gap, height));
		if (s_resize.erase(s_bar.id))
			ImGui::SetNextWindowSize(ImVec2(kPad * 2.0f + cols * cell + (cols - 1) * gap, height), ImGuiCond_Always);
	}
	else
	{
		flags |= ImGuiWindowFlags_AlwaysAutoResize;
		s_resize.erase(s_bar.id);
	}

	const CXRect r = native->GetScreenRect();
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(r.left), static_cast<float>(r.top)), ImGuiCond_FirstUseEver);
	PlaceWindow(id);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(kPad, kPad));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.0f, 1.0f));
	ImGui::Begin(id, nullptr, flags);
	ImGui::PopStyleVar(2);

	const Palette& p = Colors();
	const ImVec2 pos = ImGui::GetWindowPos();
	const ImVec2 end(pos.x + ImGui::GetWindowWidth(), pos.y + ImGui::GetWindowHeight());
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const uint8_t alpha = static_cast<uint8_t>(std::clamp(l.opacity, 0.0f, 1.0f) * 255.0f);
	dl->AddRectFilled(pos, end, WithAlpha(p.panel, alpha), p.rounding);
	if (alpha > 0)
		dl->AddRect(pos, end, WithAlpha(p.panelBorder, alpha), p.rounding);

	const ImVec2 origin(pos.x + kPad, pos.y + kPad);
	if (grid)
	{
		const float width = ImGui::GetWindowWidth() - kPad * 2.0f;
		const int cols = std::clamp(SnapColumns(width, cell, gap, n), 1, n);
		if (cols != l.columns && !l.locked)
		{
			l.columns = cols;
			SaveLayouts();
		}
		for (int k = 0; k < s_bar.count; ++k)
		{
			const ImVec2 mn(origin.x + (k % cols) * (cell + gap), origin.y + (k / cols) * (cell + gap));
			s_bar.cells.push_back({ mn, ImVec2(mn.x + cell, mn.y + cell) });
		}
	}
	else
	{
		const float scale = cell / s_bar.nativeSize;
		ImVec2 lo(1e9f, 1e9f);
		for (const NativeSlot& s : natives)
		{
			lo.x = std::min(lo.x, s.offset.x);
			lo.y = std::min(lo.y, s.offset.y);
		}
		for (const NativeSlot& s : natives)
		{
			const ImVec2 mn(origin.x + (s.offset.x - lo.x) * scale, origin.y + (s.offset.y - lo.y) * scale);
			s_bar.cells.push_back({ mn, ImVec2(mn.x + s.size.x * scale, mn.y + s.size.y * scale) });
		}
	}
	return s_bar.cells;
}

const BarLayout& CurrentBar()
{
	return *s_bar.layout;
}

void EndBar()
{
	if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("##barlayout");
	if (ImGui::BeginPopup("##barlayout"))
	{
		LayoutMenu();
		ImGui::EndPopup();
	}
	NoteWindow(s_bar.id.c_str());
	ImGui::End();
}

bool LeftTap()
{
	const ImGuiIO& io = ImGui::GetIO();
	return ImGui::IsItemDeactivated() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsItemHovered()
		&& io.MouseDragMaxDistanceSqr[ImGuiMouseButton_Left] < kTapSlop * kTapSlop;
}

Press RightPress()
{
	static ImGuiID s_item = 0;
	static bool    s_held = false;

	const ImGuiID item = ImGui::GetItemID();
	if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
	{
		s_item = item;
		s_held = false;
	}
	if (s_item != item)
		return Press::None;

	const ImGuiIO& io = ImGui::GetIO();
	if (io.MouseDown[ImGuiMouseButton_Right])
	{
		if (!s_held && io.MouseDownDuration[ImGuiMouseButton_Right] >= kLongPress)
		{
			s_held = true;
			return Press::Long;
		}
		return Press::None;
	}

	s_item = 0;
	return !s_held && ImGui::IsItemHovered() ? Press::Tap : Press::None;
}

void SpellTooltip(const EQ_Spell* spell, int msLeft, bool buff, const char* caster)
{
	if (!spell)
		return;

	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 26.0f);

	const bool good = spell->IsBeneficialSpell();
	ImGui::TextColored(good ? ImVec4(0.55f, 0.9f, 0.55f, 1.0f) : ImVec4(0.95f, 0.5f, 0.45f, 1.0f), "%s", spell->Name);

	const int cls = pLocalPlayer ? pLocalPlayer->GetClass() : 0;
	const int level = cls > 0 && cls <= MAX_CLASSES ? spell->ClassLevel[cls] : 0;
	if (level > 0 && level < 255)
		ImGui::TextDisabled("level %d", level);

	char left[32];
	if (buff)
	{
		if (msLeft < 0)
			ImGui::TextUnformatted("permanent");
		else
		{
			FormatCooldown(static_cast<uint32_t>(msLeft), left, sizeof(left));
			ImGui::Text("%s left", left);
		}
		if (caster && *caster)
			ImGui::TextDisabled("cast by %s", caster);
	}
	else
	{
		if (msLeft > 0)
		{
			FormatCooldown(static_cast<uint32_t>(msLeft), left, sizeof(left));
			ImGui::Text("ready in %s", left);
		}
		ImGui::TextDisabled("mana %d   cast %.1fs   recast %.1fs", spell->ManaCost, spell->CastTime / 1000.0f,
			spell->RecastTime / 1000.0f);
	}

	if (pDBStr)
		if (const char* desc = pDBStr->GetString(spell->DescriptionIndex, eSpellDescription); desc && *desc)
		{
			ImGui::Separator();
			ImGui::TextUnformatted(desc);
		}

	char effect[MAX_STRING];
	bool first = true;
	for (int i = 0; i < spell->GetNumEffects(); ++i)
	{
		effect[0] = 0;
		ParseSpellEffect(const_cast<EQ_Spell*>(spell), i, effect, sizeof(effect));
		if (!effect[0])
			continue;
		if (first)
			ImGui::Separator();
		first = false;
		ImGui::TextUnformatted(effect);
	}

	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}

void FormatCooldown(uint32_t ms, char* out, size_t size)
{
	const uint32_t s = (ms + 999) / 1000;
	if (ms < 10000)
		std::snprintf(out, size, "%.1f", ms / 1000.0f);
	else if (s < 60)
		std::snprintf(out, size, "%us", s);
	else if (s < 3600)
		std::snprintf(out, size, "%um", (s + 59) / 60);
	else
		std::snprintf(out, size, "%uh", (s + 3599) / 3600);
}

void IconText(ImDrawList* dl, ImVec2 center, const char* text, ImU32 color)
{
	const ImVec2 size = ImGui::CalcTextSize(text);
	const ImVec2 at(center.x - size.x * 0.5f, center.y - size.y * 0.5f);
	dl->AddText(ImVec2(at.x + 1.0f, at.y + 1.0f), IM_COL32(0, 0, 0, 220), text);
	dl->AddText(at, color, text);
}

void DrawCooldownOverlay(ImDrawList* dl, ImVec2 min, ImVec2 max, uint32_t leftMs, uint32_t totalMs)
{
	if (!leftMs || !totalMs)
		return;
	CooldownSweep(dl, min, max, static_cast<float>(leftMs) / totalMs, Colors().cooldown);
	char text[16];
	FormatCooldown(leftMs, text, sizeof(text));
	IconText(dl, ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f), text, IM_COL32(255, 255, 255, 255));
}

uint32_t Countdown(const std::string& key, uint32_t readingMs, uint32_t& totalMs, uint32_t toleranceMs)
{
	struct Clock
	{
		double   end = 0.0;
		uint32_t total = 0;
	};
	static std::unordered_map<std::string, Clock> s_clocks;

	Clock& c = s_clocks[key];
	const double now = ImGui::GetTime();
	const double predicted = std::max(0.0, (c.end - now) * 1000.0);

	if (readingMs == 0)
	{
		if (predicted > toleranceMs)
			c.end = now;
	}
	else if (predicted == 0.0 || std::fabs(readingMs - predicted) > toleranceMs)
	{
		if (predicted == 0.0)
			c.total = readingMs;
		c.end = now + readingMs / 1000.0;
	}

	const uint32_t left = static_cast<uint32_t>(std::max(0.0, (c.end - now) * 1000.0));
	if (!left)
		c.total = 0;
	else if (left > c.total)
		c.total = left;
	totalMs = c.total;
	return left;
}

namespace {

struct CastPrediction
{
	int    spellId = -1;
	int    gem = -1;
	double start = 0.0;
	double lands = 0.0;
	double recoveryEnd = 0.0;
	double gemEnd = 0.0;
	bool   checked = false;
	bool   gemSynced = false;
};

CastPrediction s_cast;
uint32_t       s_lastRecoveryEta = 0;
int            s_predictedFrame = -1;

uint32_t GameRecovery()
{
	if (!pLocalPlayer || !pDisplay)
		return 0;
	const uint32_t eta = pLocalPlayer->GetSpellCooldownETA();
	return eta > pDisplay->TimeStamp ? eta - pDisplay->TimeStamp : 0;
}

bool CanCastNow()
{
	return pLocalPlayer && pLocalPlayer->CastingData.SpellID <= 0 && GameRecovery() == 0;
}

void Predict(const EQ_Spell* spell, int gem, uint32_t castMs, uint32_t recoveryMs)
{
	const double now = ImGui::GetTime();
	s_cast = CastPrediction{};
	s_cast.spellId = spell ? spell->ID : -1;
	s_cast.gem = gem;
	s_cast.start = now;
	s_cast.lands = now + castMs / 1000.0;
	s_cast.recoveryEnd = s_cast.lands + recoveryMs / 1000.0;
	s_cast.gemEnd = spell && gem >= 0 && spell->RecastTime > recoveryMs ? s_cast.lands + spell->RecastTime / 1000.0 : 0.0;
}

void UpdatePrediction()
{
	const int frame = ImGui::GetFrameCount();
	if (frame == s_predictedFrame || !pLocalPlayer || !pDisplay)
		return;
	s_predictedFrame = frame;
	const double now = ImGui::GetTime();

	const LaunchSpellData& casting = pLocalPlayer->CastingData;
	if (casting.SpellID > 0 && casting.SpellID != s_cast.spellId)
	{
		const EQ_Spell* spell = GetSpellByID(casting.SpellID);
		const uint32_t castMs = casting.SpellETA > pDisplay->TimeStamp ? casting.SpellETA - pDisplay->TimeStamp
			: (spell ? spell->CastTime : 0);
		const int gem = casting.SpellSlot < NUM_SPELL_GEMS ? casting.SpellSlot : -1;
		Predict(spell, gem, castMs, spell ? spell->RecoveryTime : 1500);
	}
	else if (casting.SpellID <= 0 && now > s_cast.lands)
		s_cast.spellId = -1;

	const uint32_t eta = pLocalPlayer->GetSpellCooldownETA();
	if (eta > s_lastRecoveryEta + 250 && casting.SpellID <= 0 && now >= s_cast.recoveryEnd)
		Predict(nullptr, -1, 0, GameRecovery());
	s_lastRecoveryEta = eta;

	if (!s_cast.checked && now - s_cast.start >= 1.0)
	{
		s_cast.checked = true;
		if (CanCastNow())
			s_cast.recoveryEnd = std::min(s_cast.recoveryEnd, now);
	}

	if (s_cast.gem >= 0 && !s_cast.gemSynced && now > s_cast.lands + 0.3)
	{
		s_cast.gemSynced = true;
		const uint32_t reading = GemReading(s_cast.gem);
		s_cast.gemEnd = reading ? now + reading / 1000.0 : 0.0;
	}
}

} // namespace

uint32_t RecoveryLeft(uint32_t& totalMs)
{
	UpdatePrediction();
	const double now = ImGui::GetTime();
	if (s_cast.recoveryEnd > now)
	{
		totalMs = static_cast<uint32_t>((s_cast.recoveryEnd - s_cast.start) * 1000.0);
		return static_cast<uint32_t>((s_cast.recoveryEnd - now) * 1000.0);
	}
	return Countdown("recovery", GameRecovery(), totalMs, 150);
}

uint32_t GemReading(int gem)
{
	if (gem < 0 || gem >= NUM_SPELL_GEMS || !pLocalPlayer || !pLocalPC || !pDisplay)
		return 0;
	const EQ_Spell* spell = GetSpellByID(GetMemorizedSpell(gem));
	if (!spell)
		return 0;

	const uint32_t now = pDisplay->TimeStamp;
	const uint32_t eta = pLocalPlayer->SpellGemETA[gem];
	uint32_t left = eta > now ? eta - now : 0;
	if (spell->ReuseTimerIndex > 0 && spell->ReuseTimerIndex < 25)
	{
		const uint32_t linked = pLocalPC->GetLinkedSpellReuseTimer(spell->ReuseTimerIndex);
		const uint32_t fast = static_cast<uint32_t>(GetFastTime());
		if (linked > fast)
			left = std::max(left, (linked - fast) * 1000);
	}
	return left;
}

uint32_t GemCooldown(int gem, uint32_t& totalMs)
{
	totalMs = 0;
	if (gem < 0 || gem >= NUM_SPELL_GEMS)
		return 0;
	UpdatePrediction();
	const double now = ImGui::GetTime();
	if (gem == s_cast.gem && s_cast.gemEnd > now)
	{
		totalMs = static_cast<uint32_t>((s_cast.gemEnd - s_cast.start) * 1000.0);
		return static_cast<uint32_t>((s_cast.gemEnd - now) * 1000.0);
	}
	return Countdown("gem" + std::to_string(gem), GemReading(gem), totalMs, 1100);
}

} // namespace eqgfx::ui
