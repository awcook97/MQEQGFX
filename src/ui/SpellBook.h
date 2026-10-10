#pragma once

#include <mq/Plugin.h>

#include <string>
#include <vector>

namespace eqgfx::ui {

struct BookSpell
{
	EQ_Spell* spell;
	int       level;
};

struct BookGroup
{
	std::string            name;
	std::vector<BookSpell> spells;
};

struct BookCategory
{
	std::string            name;
	std::vector<BookGroup> groups;
};

const std::vector<BookCategory>& BookTree();

void MemorizeSpell(int gem, const EQ_Spell* spell);

constexpr const char* kSpellPayload = "EQGFX_SPELL";

void MemorizeMenu(int gem, char* search, size_t searchSize);

} // namespace eqgfx::ui
