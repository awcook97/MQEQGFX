#include "SpellBook.h"

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>

namespace eqgfx::ui {

namespace {

std::vector<BookCategory> s_tree;
uint64_t s_bookHash = 0;
double   s_checkedAt = -1.0;

uint64_t HashBook(const PcProfile* profile)
{
	uint64_t h = 1469598103934665603ull;
	for (int i = 0; i < NUM_BOOK_SLOTS; ++i)
		h = (h ^ static_cast<uint32_t>(profile->GetSpellBook(i))) * 1099511628211ull;
	return h;
}

const char* CategoryName(int id)
{
	const char* name = id > 0 && pDBStr ? pDBStr->GetString(id, eSpellCategory) : nullptr;
	return name && *name ? name : "Other";
}

void Build(const PcProfile* profile)
{
	const int cls = pLocalPlayer ? pLocalPlayer->GetClass() : 0;
	std::map<std::string, std::map<std::string, std::vector<BookSpell>>> grouped;
	for (int i = 0; i < NUM_BOOK_SLOTS; ++i)
	{
		EQ_Spell* spell = GetSpellByID(profile->GetSpellBook(i));
		if (!spell)
			continue;
		const int level = cls > 0 && cls <= MAX_CLASSES ? spell->ClassLevel[cls] : 0;
		grouped[CategoryName(spell->Category)][CategoryName(spell->Subcategory)].push_back({ spell, level });
	}

	s_tree.clear();
	for (auto& [catName, groups] : grouped)
	{
		BookCategory& cat = s_tree.emplace_back();
		cat.name = catName;
		for (auto& [groupName, spells] : groups)
		{
			std::sort(spells.begin(), spells.end(), [](const BookSpell& a, const BookSpell& b) {
				return a.level != b.level ? a.level > b.level : std::strcmp(a.spell->Name, b.spell->Name) < 0;
			});
			cat.groups.push_back({ groupName, std::move(spells) });
		}
	}
}

void SpellItem(int gem, const BookSpell& s)
{
	char item[160];
	std::snprintf(item, sizeof(item), "%3d  %s", s.level, s.spell->Name);
	if (ImGui::MenuItem(item))
		MemorizeSpell(gem, s.spell);
}

} // namespace

const std::vector<BookCategory>& BookTree()
{
	const double now = ImGui::GetTime();
	if (now - s_checkedAt < 0.5 && s_checkedAt >= 0.0)
		return s_tree;
	s_checkedAt = now;

	const PcProfile* profile = GetPcProfile();
	if (!profile)
	{
		s_tree.clear();
		s_bookHash = 0;
		return s_tree;
	}
	const uint64_t hash = HashBook(profile);
	if (hash != s_bookHash)
	{
		s_bookHash = hash;
		Build(profile);
	}
	return s_tree;
}

void MemorizeSpell(int gem, const EQ_Spell* spell)
{
	if (!spell || gem < 0 || gem >= NUM_SPELL_GEMS)
		return;
	char line[MAX_STRING];
	std::snprintf(line, sizeof(line), "/memspell %d \"%s\"", gem + 1, spell->Name);
	DoCommand(line);
}

void MemorizeMenu(int gem, char* search, size_t searchSize)
{
	ImGui::SetNextItemWidth(220.0f);
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	ImGui::InputTextWithHint("##search", "search", search, searchSize);

	const std::vector<BookCategory>& tree = BookTree();
	if (search[0])
	{
		int shown = 0;
		for (const BookCategory& cat : tree)
			for (const BookGroup& group : cat.groups)
				for (const BookSpell& s : group.spells)
					if (shown < 60 && ci_find_substr(s.spell->Name, search) != -1)
					{
						SpellItem(gem, s);
						++shown;
					}
		if (!shown)
			ImGui::TextDisabled("no match");
		return;
	}

	for (const BookCategory& cat : tree)
	{
		if (!ImGui::BeginMenu(cat.name.c_str()))
			continue;
		for (const BookGroup& group : cat.groups)
		{
			if (!ImGui::BeginMenu(group.name.c_str()))
				continue;
			for (const BookSpell& s : group.spells)
				SpellItem(gem, s);
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
}

} // namespace eqgfx::ui
