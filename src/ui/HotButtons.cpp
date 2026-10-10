#include "HotButtons.h"
#include "ClassProfile.h"
#include "Skins.h"

#include <imgui.h>

#include <cstdio>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string_view>
#include <unordered_map>

namespace eqgfx::ui {

namespace {

HotButton s_buttons[kHotBars][kHotPages][kHotSlots];
int       s_loadedSerial = -1;
bool      s_needImport = false;

constexpr double kReadingAge = 0.25;

struct Reading
{
	double   at = -100.0;
	uint32_t ms = 0;
};
std::unordered_map<std::string, Reading> s_readings;

struct CachedAA
{
	CAltAbilityData* ability = nullptr;
	double           checked = -100.0;
};
std::unordered_map<std::string, CachedAA> s_aa;

std::string TimerId(const HotButton& b)
{
	return std::to_string(static_cast<int>(b.timer)) + '|' + b.timerKey;
}

HotButton Decode(const std::string& in)
{
	std::string fields[6];
	int f = 0;
	for (size_t i = 0; i < in.size() && f < 6; ++i)
	{
		const char c = in[i];
		if (c == '|')
			++f;
		else if (c == '\\' && i + 1 < in.size())
		{
			const char n = in[++i];
			fields[f] += n == 'n' ? '\n' : n == 'p' ? '|' : n;
		}
		else
			fields[f] += c;
	}
	HotButton b;
	b.iconType = static_cast<ButtonIcon>(std::atoi(fields[0].c_str()));
	b.icon = std::atoi(fields[1].c_str());
	b.timer = static_cast<ButtonTimer>(std::atoi(fields[2].c_str()));
	b.timerKey = std::move(fields[3]);
	b.label = std::move(fields[4]);
	b.cmd = std::move(fields[5]);
	return b;
}

settings::json& HotBarsNode()
{
	settings::json& bars = ProfileRoot()["HotBars"];
	if (!bars.is_array())
		bars = settings::json::array();
	return bars;
}

void SavePage(int bar, int page)
{
	settings::json& bars = HotBarsNode();
	while (static_cast<int>(bars.size()) <= bar)
		bars.push_back(settings::json::array());
	settings::json& pages = bars[bar];
	if (!pages.is_array())
		pages = settings::json::array();
	while (static_cast<int>(pages.size()) <= page)
		pages.push_back(settings::json::array());
	settings::json slots = settings::json::array();
	for (int s = 0; s < kHotSlots; ++s)
	{
		const HotButton& b = s_buttons[bar][page][s];
		slots.push_back(b.Empty() ? settings::json(nullptr) : ButtonJson(b));
	}
	pages[page] = std::move(slots);
	MarkProfileDirty();
}

void LoadAll()
{
	bool any = false;
	const settings::json& bars = HotBarsNode();
	for (int bar = 0; bar < kHotBars; ++bar)
		for (int page = 0; page < kHotPages; ++page)
		{
			for (HotButton& b : s_buttons[bar][page])
				b = HotButton{};
			if (bar >= static_cast<int>(bars.size()) || !bars[bar].is_array() || page >= static_cast<int>(bars[bar].size()))
				continue;
			const settings::json& slots = bars[bar][page];
			if (!slots.is_array())
				continue;
			for (int s = 0; s < kHotSlots && s < static_cast<int>(slots.size()); ++s)
				if (slots[s].is_object())
				{
					s_buttons[bar][page][s] = ButtonFromJson(slots[s]);
					any = true;
				}
		}

	const settings::json& root = ProfileRoot();
	const auto imported = root.find("ImportedEqBars");
	s_needImport = !any && (imported == root.end() || !settings::AsBool(*imported, false));
}

void SetSpell(HotButton& b, const EQ_Spell* spell)
{
	b.label = spell->Name;
	b.cmd = std::string("/cast \"") + spell->Name + "\"";
	b.iconType = ButtonIcon::Spell;
	b.icon = spell->SpellIcon;
	b.timer = ButtonTimer::SpellGem;
	b.timerKey = spell->Name;
}

void SetItem(HotButton& b, const char* name, int iconId)
{
	b.label = name;
	b.cmd = std::string("/useitem \"") + name + "\"";
	if (iconId >= 500)
	{
		b.iconType = ButtonIcon::Item;
		b.icon = iconId - 500;
	}
	b.timer = ButtonTimer::Item;
	b.timerKey = name;
}

void SetSocial(HotButton& b, int index, const char* label)
{
	b.label = label;
	constexpr int kSocials = NUM_SOCIAL_PAGES * SOCIALS_PER_PAGE;
	if (index >= kSocials)
	{
		char cmd[32];
		std::snprintf(cmd, sizeof(cmd), "/alt act %d", index - kSocials);
		b.cmd = cmd;
		b.timer = ButtonTimer::AA;
		b.timerKey = label;
		return;
	}
	if (index < 0 || !pEverQuestInfo)
		return;
	const EQSocial& social = pEverQuestInfo->socials[index / SOCIALS_PER_PAGE][index % SOCIALS_PER_PAGE];
	if (social.Name[0])
		b.label = social.Name;
	for (const char* line : social.Line)
	{
		if (!line[0])
			continue;
		if (!b.cmd.empty())
			b.cmd += '\n';
		b.cmd += line;
	}
}

void SetAbility(HotButton& b, const char* label, bool disc)
{
	b.label = label;
	if (disc)
	{
		b.cmd = std::string("/disc ") + label;
		b.timer = ButtonTimer::Disc;
		b.timerKey = label;
		if (EQ_Spell* spell = GetSpellByName(label))
		{
			b.iconType = ButtonIcon::Spell;
			b.icon = spell->SpellIcon;
		}
	}
	else
		b.cmd = std::string("/doability \"") + label + "\"";
}

HotButton FromEQ(const HotButtonData& d)
{
	HotButton b;
	const char* label = d.Label[0] ? d.Label : d.ItemName;
	switch (d.Type)
	{
	case HotButtonType_SpellGem:
		if (EQ_Spell* spell = GetSpellByID(GetMemorizedSpell(d.Slot)))
			SetSpell(b, spell);
		else
			b.label = label;
		break;
	case HotButtonType_InventorySlot:
	case HotButtonType_ItemLink:
	case HotButtonType_KronoSlot:
	case HotButtonType_ActivatedItemLink:
		SetItem(b, d.ItemName[0] ? d.ItemName : label, d.IconId);
		break;
	case HotButtonType_Social:
		SetSocial(b, d.Slot, label);
		break;
	case HotButtonType_CombatAbility:
		SetAbility(b, label, true);
		break;
	case HotButtonType_Skill:
	case HotButtonType_MeleeAbility:
		SetAbility(b, label, false);
		break;
	default:
		b.label = label;
		break;
	}
	return b;
}

void ImportFromEQ()
{
	s_needImport = false;
	if (!pEverQuestInfo)
		return;
	for (int bar = 0; bar < std::min(kHotBars, NUM_HOTBUTTON_WINDOWS); ++bar)
		for (int page = 0; page < kHotPages; ++page)
		{
			bool any = false;
			for (int s = 0; s < kHotSlots; ++s)
			{
				const HotButtonData& d = pEverQuestInfo->hotButtons[bar][page][s];
				if (d.Type == HotButtonType_None)
					continue;
				s_buttons[bar][page][s] = FromEQ(d);
				any = true;
			}
			if (any)
				SavePage(bar, page);
		}
	ProfileRoot()["ImportedEqBars"] = true;
	MarkProfileDirty();
}

constexpr const char* kButtonPayload = "EQGFX_BUTTON";

struct ButtonPayload
{
	char    label[64];
	char    cmd[2048];
	char    timerKey[64];
	int     icon;
	uint8_t iconType;
	uint8_t timer;
};

int GemWithSpell(const std::string& name)
{
	for (int gem = 0; gem < NUM_SPELL_GEMS; ++gem)
		if (EQ_Spell* spell = GetSpellByID(GetMemorizedSpell(gem)))
			if (ci_equals(spell->Name, name))
				return gem;
	return -1;
}

CAltAbilityData* FindAA(const std::string& name)
{
	CachedAA& cache = s_aa[name];
	const double now = ImGui::GetTime();
	if (now - cache.checked < 10.0)
		return cache.ability;
	cache.checked = now;
	cache.ability = nullptr;
	if (!pLocalPC || !pLocalPlayer || !pDBStr)
		return nullptr;
	for (int i = 0; i < AA_CHAR_MAX_REAL; ++i)
		if (CAltAbilityData* ability = GetAAById(pLocalPC->GetAlternateAbilityId(i), pLocalPlayer->Level))
			if (const char* aaName = pDBStr->GetString(ability->nName, eAltAbilityName))
				if (ci_equals(aaName, name))
					return cache.ability = ability;

	if (!pAltAdvManager || !pAltAdvManager->abilities)
		return nullptr;
	const int cls = pLocalPlayer->GetClass();
	for (CAltAbilityData** it = pAltAdvManager->abilities->WalkFirst(); it; it = pAltAdvManager->abilities->WalkNext(it))
		if (CAltAbilityData* ab = *it; ab && (ab->Classes & (1 << cls)))
			if (const char* aaName = pDBStr->GetString(ab->nName, eAltAbilityName); aaName && ci_equals(aaName, name))
				return cache.ability = pAltAdvManager->GetOwnedAbilityFromGroupID(pLocalPC, ab->GroupID);
	return nullptr;
}

uint32_t LeftMs(const HotButton& b)
{
	switch (b.timer)
	{
	case ButtonTimer::Item:
		if (ItemClient* item = FindItemByName(b.timerKey.c_str(), true))
			return GetItemTimer(item) * 1000;
		return 0;
	case ButtonTimer::AA:
	{
		if (CAltAbilityData* ability = FindAA(b.timerKey))
		{
			int reuse = 0;
			pAltAdvManager->IsAbilityReady(pLocalPC, ability, &reuse);
			return reuse > 0 ? static_cast<uint32_t>(reuse) * 1000 : 0;
		}
		return 0;
	}
	case ButtonTimer::Disc:
		return DiscCooldown(GetSpellByName(b.timerKey));
	default:
		return 0;
	}
}

struct StoredSet
{
	std::vector<HotButton> buttons;
	bool                   stored = false;
};

std::unordered_map<std::string, StoredSet> s_sets;
int s_setsSerial = -1;

} // namespace

std::vector<HotButton>& ButtonSet(const std::string& name, bool* stored)
{
	if (s_setsSerial != ProfileSerial())
	{
		s_setsSerial = ProfileSerial();
		s_sets.clear();
	}
	auto [it, added] = s_sets.try_emplace(name);
	StoredSet& set = it->second;
	if (added && !ProfileClass().empty())
	{
		const settings::json& sets = ProfileRoot()["ButtonSets"];
		const auto list = sets.is_object() ? sets.find(name) : sets.end();
		if (sets.is_object() && list != sets.end() && list->is_array())
		{
			set.stored = true;
			for (const settings::json& b : *list)
				set.buttons.push_back(b.is_object() ? ButtonFromJson(b) : HotButton{});
		}
	}
	if (stored)
		*stored = set.stored;
	return set.buttons;
}

void SaveButtonSet(const std::string& name)
{
	auto it = s_sets.find(name);
	if (it == s_sets.end())
		return;
	it->second.stored = true;
	settings::json list = settings::json::array();
	for (const HotButton& b : it->second.buttons)
		list.push_back(b.Empty() ? settings::json(nullptr) : ButtonJson(b));
	settings::json& sets = ProfileRoot()["ButtonSets"];
	if (!sets.is_object())
		sets = settings::json::object();
	sets[name] = std::move(list);
	MarkProfileDirty();
}

settings::json ButtonJson(const HotButton& b)
{
	return { { "label", b.label }, { "cmd", b.cmd }, { "iconType", static_cast<int>(b.iconType) },
		{ "icon", b.icon }, { "timer", static_cast<int>(b.timer) }, { "timerKey", b.timerKey } };
}

HotButton ButtonFromJson(const settings::json& j)
{
	HotButton b;
	if (!j.is_object())
		return b;
	auto str = [&](const char* key) {
		const auto it = j.find(key);
		return it != j.end() ? settings::AsString(*it, "") : std::string();
	};
	auto num = [&](const char* key) {
		const auto it = j.find(key);
		return it != j.end() ? settings::AsInt(*it, 0) : 0;
	};
	b.label = str("label");
	b.cmd = str("cmd");
	b.iconType = static_cast<ButtonIcon>(num("iconType"));
	b.icon = num("icon");
	b.timer = static_cast<ButtonTimer>(num("timer"));
	b.timerKey = str("timerKey");
	return b;
}

HotButton ButtonFromLegacy(const std::string& text)
{
	return Decode(text);
}

void UpdateHotButtons()
{
	if (s_loadedSerial != ProfileSerial())
	{
		s_loadedSerial = ProfileSerial();
		if (ProfileClass().empty())
			for (auto& bar : s_buttons)
				for (auto& page : bar)
					for (HotButton& b : page)
						b = HotButton{};
		else
			LoadAll();
	}
	if (s_needImport && !ProfileClass().empty())
		ImportFromEQ();
}

int HotBarPage(int bar)
{
	const CHotButtonWnd* wnd = bar >= 0 && bar < kHotBars ? pHotButtonWnds[bar].get() : nullptr;
	const int page = wnd ? wnd->Page : 0;
	return page >= 0 && page < kHotPages ? page : 0;
}

HotButton& ButtonAt(int bar, int page, int slot)
{
	return s_buttons[bar][page][slot];
}

void ButtonChanged(int bar, int page)
{
	SavePage(bar, page);
}

void SwapButtons(int bar, int page, int slot, int bar2, int page2, int slot2)
{
	std::swap(s_buttons[bar][page][slot], s_buttons[bar2][page2][slot2]);
	ButtonChanged(bar, page);
	if (bar2 != bar || page2 != page)
		ButtonChanged(bar2, page2);
}

bool CursorHasButton()
{
	return pCursorAttachment && pCursorAttachment->Type != eCursorAttachment_None;
}

bool AssignFromCursor(HotButton& b)
{
	CCursorAttachment* ca = pCursorAttachment.get();
	if (!ca || ca->Type == eCursorAttachment_None || !pLocalPC)
		return false;

	std::string text = ca->ButtonText.c_str();
	for (char& c : text)
		if (c == '\n' || c == '\r')
			c = ' ';

	HotButton made;
	switch (ca->Type)
	{
	case eCursorAttachment_Item:
		if (ItemPtr item = pLocalPC->GetInventorySlot(InvSlot_Held))
			SetItem(made, item->GetName(), item->GetIconID());
		break;
	case eCursorAttachment_InvSlot:
		if (ItemPtr item = pLocalPC->GetInventorySlot(ca->Index))
			SetItem(made, item->GetName(), item->GetIconID());
		break;
	case eCursorAttachment_ItemLink:
	case eCursorAttachment_KronoSlot:
	case eCursorAttachment_MountKeyRingLink:
	case eCursorAttachment_IllusionKeyRingLink:
	case eCursorAttachment_FamiliarKeyRingLink:
	case eCursorAttachment_TeleportationKeyRingLink:
	case eCursorAttachment_ActivatedKeyRingLink:
	case eCursorAttachment_EquipmentKeyRingLink:
		if (ItemClient* item = FindItemByID(ca->ItemID))
			SetItem(made, item->GetName(), item->GetIconID());
		else if (!text.empty())
			SetItem(made, text.c_str(), ca->IconID);
		break;
	case eCursorAttachment_SpellGem:
		if (EQ_Spell* spell = GetSpellByID(GetMemorizedSpell(ca->Index)))
			SetSpell(made, spell);
		break;
	case eCursorAttachment_MemorizeSpell:
		if (PcProfile* profile = GetPcProfile())
			if (EQ_Spell* spell = GetSpellByID(profile->GetSpellBook(ca->Index)))
				SetSpell(made, spell);
		break;
	case eCursorAttachment_SkillID:
	case eCursorAttachment_Ability:
		SetAbility(made, text.c_str(), false);
		break;
	case eCursorAttachment_MeleeAbility:
	case eCursorAttachment_CombatAbility:
		SetAbility(made, text.c_str(), true);
		break;
	case eCursorAttachment_Social:
		SetSocial(made, ca->Index, text.c_str());
		break;
	default:
		made.label = text;
		break;
	}
	if (made.Empty())
		return false;

	b = std::move(made);
	if (ca->Type != eCursorAttachment_Item)
		ca->RemoveAttachment();
	return true;
}

uint32_t DiscCooldown(const EQ_Spell* spell)
{
	if (!spell || !pLocalPC)
		return 0;
	const uint32_t ready = pLocalPC->GetCombatAbilityTimer(spell->ReuseTimerIndex, spell->SpellGroup);
	const uint32_t now = static_cast<uint32_t>(std::time(nullptr));
	return ready > now ? (ready - now) * 1000 : 0;
}

void DragButtonSource(const HotButton& b)
{
	if (!ImGui::BeginDragDropSource())
		return;
	ButtonPayload p{};
	strncpy_s(p.label, sizeof(p.label), b.label.c_str(), _TRUNCATE);
	strncpy_s(p.cmd, sizeof(p.cmd), b.cmd.c_str(), _TRUNCATE);
	strncpy_s(p.timerKey, sizeof(p.timerKey), b.timerKey.c_str(), _TRUNCATE);
	p.icon = b.icon;
	p.iconType = static_cast<uint8_t>(b.iconType);
	p.timer = static_cast<uint8_t>(b.timer);
	ImGui::SetDragDropPayload(kButtonPayload, &p, sizeof(p));
	ImGui::TextUnformatted(p.label);
	ImGui::EndDragDropSource();
}

bool AcceptDraggedButton(HotButton& into)
{
	const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kButtonPayload);
	if (!payload)
		return false;
	const ButtonPayload& p = *static_cast<const ButtonPayload*>(payload->Data);
	into = HotButton{};
	into.label = p.label;
	into.cmd = p.cmd;
	into.timerKey = p.timerKey;
	into.icon = p.icon;
	into.iconType = static_cast<ButtonIcon>(p.iconType);
	into.timer = static_cast<ButtonTimer>(p.timer);
	return true;
}

void MakeSpellButton(HotButton& b, const EQ_Spell* spell)
{
	if (spell)
	{
		b = HotButton{};
		SetSpell(b, spell);
	}
}

void RunButton(const HotButton& b)
{
	size_t start = 0;
	while (start < b.cmd.size())
	{
		size_t end = b.cmd.find('\n', start);
		if (end == std::string::npos)
			end = b.cmd.size();
		std::string line = b.cmd.substr(start, end - start);
		start = end + 1;
		while (!line.empty() && (line.back() == ' ' || line.back() == '\r'))
			line.pop_back();
		if (!line.empty())
			DoCommand(line.c_str());
	}
}

uint32_t ButtonCooldown(const HotButton& b, uint32_t& totalMs)
{
	totalMs = 0;
	if (b.timer == ButtonTimer::None)
		return 0;
	if (b.timer == ButtonTimer::SpellGem)
	{
		const int gem = GemWithSpell(b.timerKey);
		uint32_t gemTotal = 0, recoveryTotal = 0;
		const uint32_t own = gem >= 0 ? GemCooldown(gem, gemTotal) : 0;
		const uint32_t recovery = RecoveryLeft(recoveryTotal);
		totalMs = own >= recovery ? gemTotal : recoveryTotal;
		return std::max(own, recovery);
	}

	const std::string id = TimerId(b);
	uint32_t reading;
	{
		Reading& r = s_readings[id];
		const double now = ImGui::GetTime();
		if (now - r.at >= kReadingAge)
		{
			r.ms = LeftMs(b);
			r.at = now;
		}
		reading = r.ms;
	}
	return Countdown(id, reading, totalMs, 1100);
}

} // namespace eqgfx::ui
