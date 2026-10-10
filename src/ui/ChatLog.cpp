#include "ChatLog.h"

#include <mq/Plugin.h>

#include <imgui.h>

#include <iterator>
#include <mutex>

namespace eqgfx::ui {

namespace {

constexpr size_t kMaxPending = 2000;

std::mutex               s_mutex;
std::vector<ChatLine>    s_pending;
std::vector<std::string> s_links;

enum Legacy : uint32_t
{
	L_Public = 1u << 0, L_Tell = 1u << 1, L_Group = 1u << 2, L_Guild = 1u << 3, L_Channel = 1u << 4,
	L_Npc = 1u << 5, L_Combat = 1u << 6, L_Spells = 1u << 7, L_Loot = 1u << 8, L_Progress = 1u << 9,
	L_System = 1u << 10, L_MQ = 1u << 11,
};

struct FilterDef
{
	ChatFilter filter;
	uint32_t   legacy;
	int        colors[3];
};

constexpr FilterDef kDefs[] = {
	{ { "say", "Say", "Chat" }, L_Public, { USERCOLOR_SAY, USERCOLOR_ECHO_SAY } },
	{ { "tell", "Tells", "Chat" }, L_Tell, { USERCOLOR_TELL, USERCOLOR_ECHO_TELL } },
	{ { "group", "Group", "Chat" }, L_Group, { USERCOLOR_GROUP, USERCOLOR_ECHO_GROUP } },
	{ { "raid", "Raid", "Chat" }, L_Group, { USERCOLOR_RAID } },
	{ { "guild", "Guild", "Chat" }, L_Guild, { USERCOLOR_GUILD, USERCOLOR_ECHO_GUILD } },
	{ { "guildmsg", "Guild messages", "Chat" }, L_Guild, { USERCOLOR_GUILD_MSG } },
	{ { "fellowship", "Fellowship", "Chat" }, L_Guild, { USERCOLOR_FELLOWSHIP } },
	{ { "ooc", "OOC", "Chat" }, L_Public, { USERCOLOR_OOC, USERCOLOR_ECHO_OOC } },
	{ { "auction", "Auction", "Chat" }, L_Public, { USERCOLOR_AUCTION, USERCOLOR_ECHO_AUCTION } },
	{ { "shout", "Shout", "Chat" }, L_Public, { USERCOLOR_SHOUT, USERCOLOR_ECHO_SHOUT } },
	{ { "emote", "Emotes", "Chat" }, L_Public, { USERCOLOR_EMOTE, USERCOLOR_ECHO_EMOTE } },
	{ { "yell", "Yells for help", "Chat" }, L_Public, { USERCOLOR_YELL } },
	{ { "roles", "Group / raid roles", "Chat" }, L_Group, { USERCOLOR_LEADER } },
	{ { "mercenary", "Mercenaries", "Chat" }, L_Group, { USERCOLOR_MERCENARY_GRP } },
	{ { "npcspeech", "NPC dialogue", "Chat" }, L_Npc, { USERCOLOR_NPC_SPEECH } },
	{ { "npctoyou", "NPC dialogue to you", "Chat" }, L_Npc, { USERCOLOR_NPC_SPEECH_TO_YOU, USERCOLOR_DIALOG_LINK } },
	{ { "itemspeech", "Item speech", "Chat" }, L_Npc, { USERCOLOR_ITEM_SPEECH } },

	{ { "channelinfo", "Channel messages", "Chat channels" }, L_Channel, { USERCOLOR_CHAT_CHANNEL } },
	{ { "channel1", "Channel 1", "Chat channels" }, L_Channel, { USERCOLOR_CHAT_1, USERCOLOR_ECHO_CHAT_1 } },
	{ { "channel2", "Channel 2", "Chat channels" }, L_Channel, { USERCOLOR_CHAT_2, USERCOLOR_ECHO_CHAT_2 } },
	{ { "channel3", "Channel 3", "Chat channels" }, L_Channel, { USERCOLOR_CHAT_3, USERCOLOR_ECHO_CHAT_3 } },
	{ { "channel4", "Channel 4", "Chat channels" }, L_Channel, { USERCOLOR_CHAT_4, USERCOLOR_ECHO_CHAT_4 } },
	{ { "channel5", "Channel 5", "Chat channels" }, L_Channel, { USERCOLOR_CHAT_5, USERCOLOR_ECHO_CHAT_5 } },
	{ { "channel6", "Channel 6", "Chat channels" }, L_Channel, { USERCOLOR_CHAT_6, USERCOLOR_ECHO_CHAT_6 } },
	{ { "channel7", "Channel 7", "Chat channels" }, L_Channel, { USERCOLOR_CHAT_7, USERCOLOR_ECHO_CHAT_7 } },
	{ { "channel8", "Channel 8", "Chat channels" }, L_Channel, { USERCOLOR_CHAT_8, USERCOLOR_ECHO_CHAT_8 } },
	{ { "channel9", "Channel 9", "Chat channels" }, L_Channel, { USERCOLOR_CHAT_9, USERCOLOR_ECHO_CHAT_9 } },
	{ { "channel10", "Channel 10", "Chat channels" }, L_Channel, { USERCOLOR_CHAT_10, USERCOLOR_ECHO_CHAT_10 } },

	{ { "yourhits", "Your hits", "Melee" }, L_Combat, { USERCOLOR_YOU_HIT_OTHER } },
	{ { "yourmisses", "Your misses", "Melee" }, L_Combat, { USERCOLOR_YOU_MISS_OTHER } },
	{ { "beinghit", "You being hit", "Melee" }, L_Combat, { USERCOLOR_OTHER_HIT_YOU } },
	{ { "beingmissed", "You being missed", "Melee" }, L_Combat, { USERCOLOR_OTHER_MISS_YOU } },
	{ { "othershits", "Others' hits", "Melee" }, L_Combat, { USERCOLOR_OTHER_HIT_OTHER } },
	{ { "othersmisses", "Others' misses", "Melee" }, L_Combat, { USERCOLOR_OTHER_MISS_OTHER } },
	{ { "yourcrits", "Your critical hits", "Melee" }, L_Combat, { USERCOLOR_MELEE_CRIT } },
	{ { "otherscrits", "Others' critical hits", "Melee" }, L_Combat, { USERCOLOR_MELEE_OTHERS_CRITICAL } },
	{ { "yourflurry", "Your flurries", "Melee" }, L_Combat, { USERCOLOR_FLURRY } },
	{ { "strikethrough", "Strikethrough", "Melee" }, L_Combat, { USERCOLOR_STRIKETHROUGH } },
	{ { "stun", "Stuns", "Melee" }, L_Combat, { USERCOLOR_STUN } },
	{ { "toofar", "Warnings (too far away)", "Melee" }, L_Combat, { USERCOLOR_TOO_FAR_AWAY } },
	{ { "npcrampage", "NPC rampage", "Melee" }, L_Combat, { USERCOLOR_NPC_RAMPAGE } },
	{ { "npcflurry", "NPC flurry", "Melee" }, L_Combat, { USERCOLOR_NPC_FLURRY } },
	{ { "npcenrage", "NPC enrage", "Melee" }, L_Combat, { USERCOLOR_NPC_ENRAGE } },
	{ { "yourdiscs", "Your disciplines", "Melee" }, L_Spells, { USERCOLOR_DISCIPLINES } },
	{ { "othersdiscs", "Others' disciplines", "Melee" }, L_Combat, { USERCOLOR_DISCIPLINES_OTHER } },
	{ { "abilityreuse", "Combat ability reuse", "Melee" }, L_System, { USERCOLOR_ABILITY_COOLDOWN } },
	{ { "taunt", "Taunts", "Melee" }, L_Combat, { USERCOLOR_TAUNT_MESSAGES } },
	{ { "yourdeath", "Your death", "Melee" }, L_Combat, { USERCOLOR_YOUR_DEATH } },
	{ { "otherdeath", "Others' deaths", "Melee" }, L_Combat, { USERCOLOR_OTHER_DEATH } },
	{ { "npcdeath", "NPC deaths", "Melee" }, L_Combat, { USERCOLOR_NPC_DEATH } },

	{ { "yourspells", "Your spells", "Spells" }, L_Spells, { USERCOLOR_SPELLS } },
	{ { "othersspells", "Others' spells", "Spells" }, L_Spells, { USERCOLOR_OTHERS_SPELLS } },
	{ { "failures", "Your spell failures", "Spells" }, L_Spells, { USERCOLOR_SPELL_FAILURE } },
	{ { "othersfailures", "Others' spell failures", "Spells" }, L_Spells, { USERCOLOR_SPELL_FAILURES_OTHER } },
	{ { "spelldamage", "Spell damage", "Spells" }, L_Combat, { USERCOLOR_NON_MELEE } },
	{ { "yourdd", "Your direct damage", "Spells" }, L_Spells, { USERCOLOR_DIRECT_DAMAGE_YOU } },
	{ { "yourddcrits", "Your direct damage crits", "Spells" }, L_Combat, { USERCOLOR_SPELL_CRIT } },
	{ { "othersdd", "Others' direct damage", "Spells" }, L_Spells, { USERCOLOR_DIRECT_DAMAGE_OTHERS } },
	{ { "othersddcrits", "Others' direct damage crits", "Spells" }, L_Spells, { USERCOLOR_DIRECT_DAMAGE_OTHERS_CRITICAL } },
	{ { "yourdots", "Your DoTs", "Spells" }, L_Spells, { USERCOLOR_DOTS_YOURS } },
	{ { "yourdotcrits", "Your DoT crits", "Spells" }, L_Spells, { USERCOLOR_DOT_YOURS_CRITICAL } },
	{ { "othersdots", "Others' DoTs", "Spells" }, L_Spells, { USERCOLOR_DOTS_OTHERS } },
	{ { "othersdotcrits", "Others' DoT crits", "Spells" }, L_Spells, { USERCOLOR_DOT_OTHERS_CRITICAL } },
	{ { "dotsonyou", "DoTs on you", "Spells" }, L_Spells, { USERCOLOR_DOT_DAMAGE_TAKEN } },
	{ { "yourheals", "Your heals", "Spells" }, L_Spells, { USERCOLOR_HEALS_YOU } },
	{ { "yourhealcrits", "Your critical heals", "Spells" }, L_Spells, { USERCOLOR_HEALS_YOURS_CRITICAL } },
	{ { "othersheals", "Others' heals", "Spells" }, L_Spells, { USERCOLOR_HEALS_OTHERS } },
	{ { "othershealcrits", "Others' critical heals", "Spells" }, L_Spells, { USERCOLOR_HEALS_OTHERS_CRITICAL } },
	{ { "healsreceived", "Heals received", "Spells" }, L_Spells, { USERCOLOR_HEALS_RECEIVED } },
	{ { "wornoff", "Spells worn off", "Spells" }, L_Spells, { USERCOLOR_SPELL_WORN_OFF } },
	{ { "overwrittendet", "Overwritten (detrimental)", "Spells" }, L_Spells, { USERCOLOR_OVERWRITTEN_DET } },
	{ { "overwrittenbene", "Overwritten (beneficial)", "Spells" }, L_Spells, { USERCOLOR_OVERWRITTEN_BENE } },
	{ { "spellemotes", "Spell emotes", "Spells" }, L_Spells, { USERCOLOR_SPELL_MESSAGES } },
	{ { "focus", "Focus effects", "Spells" }, L_Spells, { USERCOLOR_FOCUS } },
	{ { "yourauras", "Your auras", "Spells" }, L_Spells, { USERCOLOR_AURAS_YOU } },
	{ { "othersauras", "Others' auras", "Spells" }, L_Spells, { USERCOLOR_AURAS_OTHER } },
	{ { "aareuse", "AA reuse", "Spells" }, L_System, { USERCOLOR_AA_REUSE_TIMER } },

	{ { "petmelee", "Pet melee", "Pets" }, L_Combat, { USERCOLOR_PET } },
	{ { "petrampage", "Pet rampage / flurry", "Pets" }, L_Combat, { USERCOLOR_PETRAMPFLURRY } },
	{ { "petcrits", "Pet critical hits", "Pets" }, L_Combat, { USERCOLOR_PETCRITS } },
	{ { "petspells", "Pet spells", "Pets" }, L_Spells, { USERCOLOR_PET_SPELLS } },
	{ { "petresponses", "Pet responses", "Pets" }, L_System, { USERCOLOR_PET_RESPONSES } },
	{ { "songsonpets", "Bard songs on pets", "Pets" }, L_System, { USERCOLOR_BARD_SONG_PETS } },
	{ { "swarmdeath", "Swarm pet deaths", "Pets" }, L_System, { USERCOLOR_SWARM_PET_DEATH } },

	{ { "dshittingyou", "Damage shields hitting you", "Damage shields" }, L_Combat, { USERCOLOR_DAMAGESHIELD } },
	{ { "dsyours", "Your damage shield", "Damage shields" }, L_System, { USERCOLOR_DAMAGESHIELD_SELF } },
	{ { "dsothers", "Others' damage shields", "Damage shields" }, L_System, { USERCOLOR_DAMAGESHIELD_OTHER } },

	{ { "rollsyours", "Your rolls", "Random" }, L_Public, { USERCOLOR_RANDOM } },
	{ { "rollsgroup", "Group / raid rolls", "Random" }, L_System, { USERCOLOR_DICE_GROUP } },
	{ { "rollsothers", "Others' rolls", "Random" }, L_System, { USERCOLOR_DICE_OTHER } },

	{ { "envyours", "Your environmental damage", "Environmental" }, L_System, { USERCOLOR_FALL_DAMAGE_SELF } },
	{ { "envothers", "Others' environmental damage", "Environmental" }, L_System, { USERCOLOR_FALL_DAMAGE_OTHER } },

	{ { "xp", "Experience", "Other" }, L_Progress, { USERCOLOR_XP } },
	{ { "skills", "Skills", "Other" }, L_Progress, { USERCOLOR_SKILLS } },
	{ { "faction", "Faction", "Other" }, L_Progress, { USERCOLOR_FACTION, USERCOLOR_FACTION_LINK } },
	{ { "achievements", "Your achievements", "Other" }, L_Progress, { USERCOLOR_ACHIEVEMENTS_YOU, USERCOLOR_ACHIEVEMENT } },
	{ { "othersachievements", "Others' achievements", "Other" }, L_Progress, { USERCOLOR_ACHIEVEMENTS_OTHERS } },
	{ { "loot", "Loot", "Other" }, L_Loot, { USERCOLOR_LOOT } },
	{ { "money", "Money splits", "Other" }, L_Loot, { USERCOLOR_MONEY_SPLIT } },
	{ { "merchant", "Merchants", "Other" }, L_Loot, { USERCOLOR_MERCHANT_OFFER, USERCOLOR_MERCHANT_EXCHANGE } },
	{ { "destroyed", "Destroyed items", "Other" }, L_System, { USERCOLOR_DESTROY_ITEM } },
	{ { "food", "Food and drink", "Other" }, L_System, { USERCOLOR_FOOD_AND_DRINK } },
	{ { "who", "Who", "Other" }, L_System, { USERCOLOR_WHO } },
	{ { "system", "System messages", "Other" }, L_System, { USERCOLOR_SYSTEM } },
	{ { "broadcasts", "Broadcasts", "Other" }, L_System, { USERCOLOR_DUELS } },
	{ { "events", "Event messages", "Other" }, L_System, { USERCOLOR_EVENT } },
	{ { "pvp", "PvP", "Other" }, L_System, { USERCOLOR_PVP } },
	{ { "cantuse", "Can't use command", "Other" }, L_System, { USERCOLOR_CANT_USE_COMMAND } },
	{ { "raidvictory", "Raid victory", "Other" }, L_System, { USERCOLOR_RAID_VICTORY } },
	{ { "encounter", "Encounter locks", "Other" }, L_System, { USERCOLOR_ENCOUNTER_LOCK_ATTACKABLE, USERCOLOR_ENCOUNTER_LOCK_UNATTACKABLE } },
	{ { "lockedslots", "Locked inventory slots", "Other" }, L_System, { USERCOLOR_LOCKED_INVENTORY_SLOTS } },
	{ { "other", "Everything else", "Other" }, L_System, { USERCOLOR_DEFAULT } },

	{ { "macroquest", "MacroQuest output", "MacroQuest" }, L_MQ, {} },
};
static_assert(std::size(kDefs) <= kMaxChatFilters);

const std::vector<ChatFilter>& FilterList()
{
	static const std::vector<ChatFilter> list = [] {
		std::vector<ChatFilter> out;
		for (const FilterDef& d : kDefs)
			out.push_back(d.filter);
		return out;
	}();
	return list;
}

uint16_t IndexOfKey(std::string_view key)
{
	for (size_t i = 0; i < std::size(kDefs); ++i)
		if (key == kDefs[i].filter.key)
			return static_cast<uint16_t>(i);
	return 0;
}

uint16_t FilterOf(uint32_t color)
{
	static const std::vector<uint16_t> byColor = [] {
		std::vector<uint16_t> table(USERCOLOR_SAY + NUM_USER_COLORS, IndexOfKey("other"));
		for (size_t i = 0; i < std::size(kDefs); ++i)
			for (int c : kDefs[i].colors)
				if (c > 0 && c < static_cast<int>(table.size()))
					table[c] = static_cast<uint16_t>(i);
		return table;
	}();
	return color < byColor.size() ? byColor[color] : byColor[USERCOLOR_DEFAULT];
}

uint32_t RgbaOf(int color)
{
	if (color >= USERCOLOR_SAY && pChatManager)
	{
		const uint32_t argb = pChatManager->GetRGBAFromIndex(color);
		return IM_COL32((argb >> 16) & 0xff, (argb >> 8) & 0xff, argb & 0xff, 255);
	}
	return IM_COL32(230, 230, 230, 255);
}

void Push(ChatLine&& line)
{
	std::scoped_lock lock(s_mutex);
	if (s_pending.size() >= kMaxPending)
		s_pending.erase(s_pending.begin(), s_pending.begin() + kMaxPending / 4);
	s_pending.push_back(std::move(line));
}

} // namespace

std::span<const ChatFilter> ChatFilters()
{
	return FilterList();
}

int ChatFilterIndex(std::string_view key)
{
	for (size_t i = 0; i < std::size(kDefs); ++i)
		if (key == kDefs[i].filter.key)
			return static_cast<int>(i);
	return -1;
}

ChatFilterSet AllChatFilters()
{
	ChatFilterSet set;
	for (size_t i = 0; i < std::size(kDefs); ++i)
		set.set(i);
	return set;
}

ChatFilterSet ChatFilterGroup(std::string_view group)
{
	ChatFilterSet set;
	for (size_t i = 0; i < std::size(kDefs); ++i)
		if (group == kDefs[i].filter.group)
			set.set(i);
	return set;
}

ChatFilterSet ChatFiltersForLegacyMask(uint32_t mask)
{
	ChatFilterSet set;
	for (size_t i = 0; i < std::size(kDefs); ++i)
		if (kDefs[i].legacy & mask)
			set.set(i);
	return set;
}

void AddEqChat(const char* line, uint32_t color)
{
	if (!line || !*line)
		return;
	Push(ChatLine{ line, RgbaOf(static_cast<int>(color)), FilterOf(color) });
}

void AddMqChat(const char* line, int color)
{
	if (!line || !*line)
		return;
	std::string plain(std::strlen(line) + 1, '\0');
	StripMQChat(line, plain.data());
	plain.resize(std::strlen(plain.c_str()));
	static const uint16_t mq = IndexOfKey("macroquest");
	Push(ChatLine{ std::move(plain), RgbaOf(color), mq });
}

void DrainChat(std::vector<ChatLine>& into, size_t max)
{
	{
		std::scoped_lock lock(s_mutex);
		if (s_pending.empty())
			return;
		for (ChatLine& line : s_pending)
			into.push_back(std::move(line));
		s_pending.clear();
	}
	if (into.size() > max)
		into.erase(into.begin(), into.begin() + (into.size() - max));
}

void QueueChatLink(const std::string& line, size_t linkStart, size_t linkLength)
{
	std::scoped_lock lock(s_mutex);
	s_links.push_back(line.substr(linkStart, linkLength));
}

void RunChatLinks()
{
	std::vector<std::string> links;
	{
		std::scoped_lock lock(s_mutex);
		links.swap(s_links);
	}
	for (const std::string& link : links)
		ExecuteTextLink(ExtractLink(link));
}

} // namespace eqgfx::ui
