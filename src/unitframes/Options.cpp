#include "UnitWindow.h"

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace eqgfx::unitframes {

const char* const kAnonLabels[Anon::Count] = {
	"real names", "class (Magician)", "class short (MAG)", "scrambled",
	"asterisks", "asterisks x8", "first+last (C****e)",
};

const char* const kAlignLabels[Align::Count] = { "left", "center", "right" };

const char* const kHpTextLabels[HpText::Count] = {
	"off", "percent", "current", "current / max", "deficit",
};

const char* const kBarColorLabels[BarColor::Count] = {
	"health gradient", "class colour", "one fixed colour", "con colour",
};

const char* const kBarFillLabels[BarFill::Count] = {
	"flat", "gradient", "glass", "stripes", "segmented",
};

const char* const kBuffFilterLabels[BuffFilter::Count] = {
	"everything", "beneficial only", "detrimental only",
};

const char* const kSortLabels[Sort::Count] = {
	"list order", "nearest first", "lowest health first", "highest health first",
	"by name", "by class",
};

const char* const kLayoutLabels[Layout::Count] = { "rows", "columns" };

const char* const kCastTextLabels[CastText::Count] = {
	"no label", "inside the bar", "above the bar", "below the bar",
};

const char* const kTextEdgeLabels[TextEdge::Count] = {
	"none", "drop shadow", "outline",
};

namespace {

void FormatColor(const ImVec4& c, char* out, size_t size)
{
	auto q = [](float v) { return static_cast<int>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f); };
	std::snprintf(out, size, "%02X%02X%02X%02X", q(c.x), q(c.y), q(c.z), q(c.w));
}

bool ParseColor(const char* text, ImVec4& out)
{
	if (!text || !*text)
		return false;
	if (*text == '#')
		++text;

	unsigned int v = 0;
	const size_t len = std::strlen(text);
	if (len != 6 && len != 8)
		return false;
	if (std::sscanf(text, "%x", &v) != 1)
		return false;

	if (len == 6)
		v = (v << 8) | 0xFFu;

	out.x = ((v >> 24) & 0xFF) / 255.0f;
	out.y = ((v >> 16) & 0xFF) / 255.0f;
	out.z = ((v >> 8) & 0xFF) / 255.0f;
	out.w = (v & 0xFF) / 255.0f;
	return true;
}

bool MatchesFilter(const char* label, const char* filter)
{
	if (!filter || !*filter)
		return true;
	if (!label)
		return false;

	const size_t n = std::strlen(filter);
	for (const char* p = label; *p; ++p)
	{
		if (_strnicmp(p, filter, n) == 0)
			return true;
	}
	return false;
}

void KeyFor(char* out, size_t size, const char* prefix, const char* key)
{
	std::snprintf(out, size, "%s%s", prefix, key);
}

} // namespace

void SettingsPanel::Header(const char* label)
{
	SettingDef d;
	d.kind = SettingDef::Kind::Header;
	d.label = label;
	m_defs.push_back(d);
}

void SettingsPanel::Bool(const char* key, const char* label, bool* v, const char* tip,
	bool sameLine)
{
	SettingDef d;
	d.kind = SettingDef::Kind::Bool;
	d.key = key;
	d.label = label;
	d.tip = tip;
	d.ptr = v;
	d.sameLine = sameLine;
	m_defs.push_back(d);
}

void SettingsPanel::Int(const char* key, const char* label, int* v, int lo, int hi,
	const char* tip)
{
	SettingDef d;
	d.kind = SettingDef::Kind::Int;
	d.key = key;
	d.label = label;
	d.tip = tip;
	d.ptr = v;
	d.lo = static_cast<float>(lo);
	d.hi = static_cast<float>(hi);
	m_defs.push_back(d);
}

void SettingsPanel::Float(const char* key, const char* label, float* v, float lo, float hi,
	const char* tip, const char* fmt)
{
	SettingDef d;
	d.kind = SettingDef::Kind::Float;
	d.key = key;
	d.label = label;
	d.tip = tip;
	d.ptr = v;
	d.lo = lo;
	d.hi = hi;
	d.fmt = fmt;
	m_defs.push_back(d);
}

void SettingsPanel::Color(const char* key, const char* label, ImVec4* v, const char* tip,
	bool sameLine)
{
	SettingDef d;
	d.kind = SettingDef::Kind::Color;
	d.key = key;
	d.label = label;
	d.tip = tip;
	d.ptr = v;
	d.sameLine = sameLine;
	m_defs.push_back(d);
}

void SettingsPanel::Enum(const char* key, const char* label, int* v, const char* const* names,
	int count, const char* tip)
{
	SettingDef d;
	d.kind = SettingDef::Kind::Enum;
	d.key = key;
	d.label = label;
	d.tip = tip;
	d.ptr = v;
	d.names = names;
	d.nameCount = count;
	m_defs.push_back(d);
}

bool SettingsPanel::Draw(const char* filter)
{
	const bool filtering = filter && *filter;
	bool dirty = false;

	std::vector<bool> visible(m_defs.size(), true);
	if (filtering)
	{
		for (size_t i = 0; i < m_defs.size(); ++i)
		{
			if (m_defs[i].kind == SettingDef::Kind::Header)
				visible[i] = false;
			else
				visible[i] = MatchesFilter(m_defs[i].label, filter);
		}

		for (size_t i = 0; i < m_defs.size(); ++i)
		{
			if (m_defs[i].kind != SettingDef::Kind::Header)
				continue;
			for (size_t j = i + 1; j < m_defs.size(); ++j)
			{
				if (m_defs[j].kind == SettingDef::Kind::Header)
					break;
				if (visible[j])
				{
					visible[i] = true;
					break;
				}
			}
		}
	}

	int id = 0;
	bool lastWasVisibleControl = false;

	for (size_t i = 0; i < m_defs.size(); ++i)
	{
		SettingDef& d = m_defs[i];
		if (!visible[i])
		{
			if (d.kind != SettingDef::Kind::Header)
				++id;
			continue;
		}

		if (d.kind == SettingDef::Kind::Header)
		{
			ImGui::SeparatorText(d.label);
			lastWasVisibleControl = false;
			continue;
		}

		if (d.sameLine && lastWasVisibleControl)
			ImGui::SameLine();

		ImGui::PushID(id++);

		switch (d.kind)
		{
		case SettingDef::Kind::Bool:
			dirty |= ImGui::Checkbox(d.label, static_cast<bool*>(d.ptr));
			break;

		case SettingDef::Kind::Int:
			ImGui::SetNextItemWidth(150.0f);
			dirty |= ImGui::SliderInt(d.label, static_cast<int*>(d.ptr),
				static_cast<int>(d.lo), static_cast<int>(d.hi), "%d", ImGuiSliderFlags_AlwaysClamp);
			break;

		case SettingDef::Kind::Float:
			ImGui::SetNextItemWidth(150.0f);
			dirty |= ImGui::SliderFloat(d.label, static_cast<float*>(d.ptr), d.lo, d.hi,
				d.fmt ? d.fmt : "%.2f", ImGuiSliderFlags_AlwaysClamp);
			break;

		case SettingDef::Kind::Color:
			dirty |= ImGui::ColorEdit4(d.label, &static_cast<ImVec4*>(d.ptr)->x,
				ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreviewHalf);
			break;

		case SettingDef::Kind::Enum:
		{
			int* v = static_cast<int*>(d.ptr);
			*v = std::clamp(*v, 0, d.nameCount - 1);
			ImGui::SetNextItemWidth(190.0f);
			if (ImGui::BeginCombo(d.label, d.names[*v]))
			{
				for (int n = 0; n < d.nameCount; ++n)
				{
					if (ImGui::Selectable(d.names[n], n == *v))
					{
						*v = n;
						dirty = true;
					}
				}
				ImGui::EndCombo();
			}
			break;
		}

		default:
			break;
		}

		ImGui::PopID();

		if (d.tip && ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", d.tip);

		lastWasVisibleControl = true;
	}

	return dirty;
}

void SettingsPanel::Load(const char* prefix)
{
	char key[160];

	for (SettingDef& d : m_defs)
	{
		if (!d.key)
			continue;

		KeyFor(key, sizeof(key), prefix, d.key);

		switch (d.kind)
		{
		case SettingDef::Kind::Bool:
		{
			bool* v = static_cast<bool*>(d.ptr);
			*v = SettingGetBool(key, *v);
			break;
		}
		case SettingDef::Kind::Int:
		case SettingDef::Kind::Enum:
		{
			int* v = static_cast<int*>(d.ptr);
			*v = SettingGetInt(key, *v);
			if (d.kind == SettingDef::Kind::Enum)
				*v = std::clamp(*v, 0, d.nameCount - 1);
			else
				*v = std::clamp(*v, static_cast<int>(d.lo), static_cast<int>(d.hi));
			break;
		}
		case SettingDef::Kind::Float:
		{
			float* v = static_cast<float*>(d.ptr);
			*v = std::clamp(SettingGetFloat(key, *v),
				d.lo, d.hi);
			break;
		}
		case SettingDef::Kind::Color:
		{
			ParseColor(SettingGetString(key, "").c_str(), *static_cast<ImVec4*>(d.ptr));
			break;
		}
		default:
			break;
		}
	}
}

void SettingsPanel::Save(const char* prefix) const
{
	char key[160];
	char value[32];

	for (const SettingDef& d : m_defs)
	{
		if (!d.key)
			continue;

		KeyFor(key, sizeof(key), prefix, d.key);

		switch (d.kind)
		{
		case SettingDef::Kind::Bool:
			SettingPutBool(key, *static_cast<const bool*>(d.ptr));
			break;
		case SettingDef::Kind::Int:
		case SettingDef::Kind::Enum:
			SettingPutInt(key, *static_cast<const int*>(d.ptr));
			break;
		case SettingDef::Kind::Float:
			SettingPutFloat(key, *static_cast<const float*>(d.ptr));
			break;
		case SettingDef::Kind::Color:
			FormatColor(*static_cast<const ImVec4*>(d.ptr), value, sizeof(value));
			SettingPutString(key, value);
			break;
		default:
			break;
		}
	}
}

void BuildSharedPanel(SettingsPanel& p, WindowSettings& c)
{
	p.Header("frame");
	p.Bool("Show", "shown", &c.show);
	p.Bool("Locked", "locked", &c.locked,
		"no title bar, no moving, no resizing", true);
	p.Bool("NoTitleBar", "hide title bar", &c.noTitleBar,
		"drop the bar but keep the window movable from its edges");
	p.Bool("NoBackground", "no background", &c.noBackground,
		"only the rows draw; the frame itself is invisible", true);
	p.Bool("Scrollbar", "scrollbar", &c.scrollbar, nullptr, true);
	p.Float("Opacity", "opacity", &c.opacity, 0.0f, 1.0f, nullptr, "%.2f");
	p.Bool("AutoResize", "shrink to fit", &c.autoResize,
		"height follows the row count. width is yours either way");
	p.Float("Width", "width", &c.width, 80.0f, 1200.0f,
		"the row width; obeyed in both modes", "%.0f");
	p.Float("Height", "height", &c.height, 24.0f, 1400.0f,
		"ignored while 'shrink to fit' is on", "%.0f");
	p.Bool("HideWhenEmpty", "hide when empty", &c.hideWhenEmpty,
		"nothing to show: drop the frame entirely rather than leave it blank");
	p.Bool("ClickThrough", "click through", &c.clickThrough,
		"the frame only takes the mouse near a bar. alt grabs it anywhere");
	p.Float("HitPad", "click margin", &c.hitPad, 0.0f, 40.0f,
		"how far outside a bar still counts as clicking that bar", "%.0f px");
	p.Bool("ShowHitRects", "show what takes the mouse", &c.showHitRects,
		"paints every rectangle that currently takes the mouse");

	p.Header("bars");
	p.Float("BarHeight", "bar height", &c.bar.height, 8.0f, 64.0f, nullptr, "%.0f px");
	p.Float("AuxHeight", "aux bar height", &c.bar.auxHeight, 1.0f, 20.0f,
		"mana, endurance and pet strips", "%.0f px");
	p.Float("AuxGap", "aux bar gap", &c.bar.auxGap, 0.0f, 8.0f, nullptr, "%.0f px");
	p.Float("AuxInset", "aux bar inset", &c.bar.auxInset, 0.0f, 20.0f, nullptr, "%.0f px");
	p.Float("Rounding", "corner rounding", &c.bar.rounding, 0.0f, 12.0f, nullptr, "%.1f");
	p.Float("RowSpacing", "row spacing", &c.bar.rowSpacing, 0.0f, 20.0f, nullptr, "%.0f px");
	p.Float("ClassStrip", "class spine width", &c.bar.classStrip, 0.0f, 16.0f,
		"the class-coloured bar down the left edge; 0 turns it off", "%.0f px");
	p.Float("Inset", "fill inset", &c.bar.inset, 0.0f, 6.0f, nullptr, "%.0f px");
	p.Enum("BarColorMode", "bar colour", &c.bar.colorMode, kBarColorLabels, BarColor::Count);
	p.Enum("BarFill", "fill style", &c.bar.fill, kBarFillLabels, BarFill::Count);
	p.Bool("BarReverse", "drain to the left", &c.bar.reverse);
	p.Bool("BarBackdrop", "dark plate behind the fill", &c.bar.hpBackdrop, nullptr, true);
	p.Float("BorderThickness", "border thickness", &c.bar.borderThickness, 0.0f, 6.0f,
		nullptr, "%.1f");
	p.Bool("BorderAlways", "always draw a border", &c.bar.borderAlways,
		"not just on hover, target and focus");

	p.Header("text");
	p.Bool("TextName", "name", &c.text.name);
	p.Enum("TextEdge", "text edge", &c.text.edge, kTextEdgeLabels, TextEdge::Count);
	p.Float("TextEdgeOffset", "edge offset", &c.text.edgeOffset, 0.0f, 3.0f,
		"how far the shadow or outline sits from the glyph", "%.0f px");
	p.Bool("TextSnap", "pixel-snap text", &c.text.pixelSnap,
		"rounds glyphs to a whole pixel at a whole size. off looks soft");
	p.Enum("TextAlign", "name alignment", &c.text.nameAlign, kAlignLabels, Align::Count);
	p.Float("TextScale", "text size", &c.text.scale, 0.5f, 2.5f, nullptr, "%.2fx");
	p.Bool("TextClassColor", "name in class colour", &c.text.classColorName);
	p.Bool("TextConColor", "name in con colour", &c.text.conColorName,
		"grey/green/blue/white/yellow/red, the way the game cons a mob", true);
	p.Enum("HpText", "health text", &c.text.hpText, kHpTextLabels, HpText::Count);
	p.Enum("HpAlign", "health text side", &c.text.hpAlign, kAlignLabels, Align::Count);
	p.Bool("TextClass", "class in the bar", &c.text.showClass,
		"off by default: the class colour already says it, and the tooltip spells it out");
	p.Bool("TextLevel", "level in the bar", &c.text.showLevel, nullptr, true);
	p.Bool("TextDistance", "distance in the bar", &c.text.showDistance, nullptr, true);
	p.Bool("TextGuild", "guild in the bar", &c.text.showGuild);
	p.Bool("TextTag", "role letters / slot tags", &c.text.showTag, nullptr, true);
	p.Int("TextMaxChars", "truncate names to", &c.text.maxNameChars, 0, 32,
		"0 leaves names alone");
	p.Enum("Anon", "name anonymisation", &c.text.anonMode, kAnonLabels, Anon::Count);
	p.Bool("AnonNPCs", "anonymise NPCs too", &c.text.anonNPCs);

	p.Header("cast bar");
	p.Bool("CastShow", "cast bar", &c.cast.show);
	p.Bool("CastOverlay", "inside the health bar", &c.cast.overlay, nullptr, true);
	p.Bool("CastOnlyTarget", "only on my target", &c.cast.onlyTarget, nullptr, true);
	p.Float("CastHeight", "cast bar height", &c.cast.height, 2.0f, 24.0f, nullptr, "%.0f px");
	p.Bool("CastSpellName", "spell name", &c.cast.spellName);
	p.Bool("CastTime", "time remaining", &c.cast.timeText, nullptr, true);
	p.Enum("CastTextPlace", "label position", &c.cast.textPlace, kCastTextLabels, CastText::Count);
	p.Float("CastTextScale", "label size", &c.cast.textScale, 0.4f, 1.5f,
		"as a fraction of the row's text size", "%.2fx");

	p.Header("buff icons");
	p.Bool("Buffs", "buff icons", &c.buffs.show,
		"yours from your profile, everyone else's from MQ's buff cache");
	p.Bool("BuffBorders", "icon borders", &c.buffs.borders, nullptr, true);
	p.Bool("BuffTooltip", "icon tooltips", &c.buffs.tooltip, nullptr, true);
	p.Float("BuffIconSize", "icon size", &c.buffs.iconSize, 8.0f, 48.0f, nullptr, "%.0f px");
	p.Float("BuffSpacing", "icon spacing", &c.buffs.spacing, 0.0f, 12.0f, nullptr, "%.0f px");
	p.Int("BuffMax", "max icons", &c.buffs.maxIcons, 1, 60);
	p.Int("BuffPerRow", "icons per row", &c.buffs.perRow, 1, 30);
	p.Bool("BuffTimers", "timer text on icons", &c.buffs.timers);
	p.Bool("BuffMineOnly", "only what I cast", &c.buffs.mineOnly, nullptr, true);
	p.Bool("BuffDim", "dim what others cast", &c.buffs.dimOthers, nullptr, true);
	p.Enum("BuffFilter", "show", &c.buffs.filter, kBuffFilterLabels, BuffFilter::Count);
	p.Int("BuffRefreshMs", "rebuild the list every", &c.buffs.refreshMs, 33, 2000,
		"how often a row's buff list is rebuilt. lower costs more");
	p.Bool("BuffPetFromWindow", "pet buffs from the pet window", &c.buffs.petFromWindow,
		"the pet info window is the client's only store for pet buffs.\n"
		"off uses the buff cache, which needs the pet targeted first");

	p.Header("tooltip");
	p.Bool("TipEnabled", "tooltip", &c.tip.enabled);
	p.Bool("TipName", "name", &c.tip.name, nullptr, true);
	p.Bool("TipRealName", "real name when anonymised", &c.tip.realName, nullptr, true);
	p.Bool("TipClass", "class and level", &c.tip.classLevel);
	p.Bool("TipRace", "race", &c.tip.race, nullptr, true);
	p.Bool("TipCon", "con", &c.tip.con, nullptr, true);
	p.Bool("TipHp", "health numbers", &c.tip.hp);
	p.Bool("TipDistance", "distance", &c.tip.distance, nullptr, true);
	p.Bool("TipGuild", "guild", &c.tip.guild, nullptr, true);
	p.Bool("TipAggro", "aggro", &c.tip.aggro);
	p.Bool("TipPet", "pet", &c.tip.pet, nullptr, true);
	p.Bool("TipTargetOf", "what it is targeting", &c.tip.targetOf, nullptr, true);
	p.Bool("TipRole", "role", &c.tip.role);
	p.Bool("TipCasting", "what it is casting", &c.tip.casting, nullptr, true);
	p.Bool("TipBinds", "my click bindings", &c.tip.binds,
		"the reminder of what left, right and the hotkey would cast", true);

	p.Header("animation");
	p.Bool("AnimHpSmooth", "smooth health changes", &c.anim.hpSmooth);
	p.Bool("AnimUseImAnim", "smooth with imanim", &c.anim.useImAnim,
		"off falls back to a plain exponential step");
	p.Float("AnimHpSpeed", "smoothing speed", &c.anim.hpSpeed, 1.0f, 40.0f, nullptr, "%.0f");
	p.Bool("AnimDamageFlash", "flash on damage", &c.anim.damageFlash);
	p.Bool("AnimHealFlash", "flash on heals too", &c.anim.healFlash, nullptr, true);
	p.Float("AnimFlashThreshold", "flash threshold", &c.anim.flashThreshold, 0.0f, 0.5f,
		"the fraction of health that has to move before it flashes", "%.2f");
	p.Float("AnimFlashDur", "flash length", &c.anim.flashDur, 0.05f, 2.0f, nullptr, "%.2fs");
	p.Bool("AnimLowHpPulse", "pulse at low health", &c.anim.lowHpPulse);
	p.Float("AnimLowHpThreshold", "low health at", &c.anim.lowHpThreshold, 0.05f, 0.9f,
		nullptr, "%.2f");
	p.Float("AnimLowHpSpeed", "low health pulse speed", &c.anim.lowHpSpeed, 0.2f, 6.0f,
		nullptr, "%.1f Hz");
	p.Bool("AnimTargetGlow", "glow on my target", &c.anim.targetGlow);
	p.Float("AnimGlowSpeed", "glow speed", &c.anim.glowSpeed, 0.2f, 6.0f, nullptr, "%.1f Hz");

	p.Header("colours");
	p.Color("ColBg", "row background", &c.colors.bg);
	p.Color("ColDead", "dead", &c.colors.dead, nullptr, true);
	p.Color("ColHpHigh", "health high", &c.colors.hpHigh);
	p.Color("ColHpMid", "health mid", &c.colors.hpMid, nullptr, true);
	p.Color("ColHpLow", "health low", &c.colors.hpLow, nullptr, true);
	p.Color("ColHpFixed", "health fixed", &c.colors.hpFixed, nullptr, true);
	p.Color("ColMana", "mana", &c.colors.mana);
	p.Color("ColEndurance", "endurance", &c.colors.endurance, nullptr, true);
	p.Color("ColPetHp", "pet health", &c.colors.petHp, nullptr, true);
	p.Color("ColAuxBack", "aux backing", &c.colors.auxBack, nullptr, true);
	p.Color("ColName", "name", &c.colors.name);
	p.Color("ColNameShadow", "name shadow", &c.colors.nameShadow, nullptr, true);
	p.Color("ColHpText", "health text", &c.colors.hpTextColor, nullptr, true);
	p.Color("ColTag", "role tags", &c.colors.tag, nullptr, true);
	p.Color("ColCastFill", "cast fill", &c.colors.castFill);
	p.Color("ColCastBack", "cast backing", &c.colors.castBack, nullptr, true);
	p.Color("ColCastText", "cast text", &c.colors.castText, nullptr, true);
	p.Color("ColHoverBorder", "hover border", &c.colors.hoverBorder);
	p.Color("ColTargetBorder", "target border", &c.colors.targetBorder, nullptr, true);
	p.Color("ColFocusBorder", "focus border", &c.colors.focusBorder, nullptr, true);
	p.Color("ColIdleBorder", "idle border", &c.colors.idleBorder, nullptr, true);
	p.Color("ColFlash", "damage flash", &c.colors.flash);
	p.Color("ColLowHp", "low health pulse", &c.colors.lowHp, nullptr, true);
	p.Color("ColAggroHigh", "aggro 100%", &c.colors.aggroHigh);
	p.Color("ColAggroMid", "aggro 75%", &c.colors.aggroMid, nullptr, true);
	p.Color("ColAggroLow", "aggro low", &c.colors.aggroLow, nullptr, true);
	p.Color("ColBuffBen", "beneficial border", &c.colors.buffBorderBen);
	p.Color("ColBuffDet", "detrimental border", &c.colors.buffBorderDet, nullptr, true);
	p.Color("ColBuffMine", "my buff border", &c.colors.buffBorderMine, nullptr, true);
}

} // namespace eqgfx::unitframes
