#include "../../MQEQGFX.h"

#include "SpellFxLooks.h"

#include <cctype>
#include <string>

namespace eqgfx::spellfx {

namespace {

std::span<const ClassCatalogue* const> Catalogues()
{
	static const ClassCatalogue* const list[] = {
		&WarriorCatalogue(), &ClericCatalogue(), &PaladinCatalogue(), &RangerCatalogue(),
		&ShadowKnightCatalogue(), &DruidCatalogue(), &MonkCatalogue(), &BardCatalogue(),
		&RogueCatalogue(), &ShamanCatalogue(), &NecromancerCatalogue(), &WizardCatalogue(),
		&MageCatalogue(), &EnchanterCatalogue(), &BeastlordCatalogue(), &BerserkerCatalogue(),
		&GeneralCatalogue(),
	};
	return list;
}

bool IsRoman(std::string_view w)
{
	if (w.empty())
		return false;
	for (char ch : w)
		if (ch != 'i' && ch != 'v' && ch != 'x' && ch != 'l')
			return false;
	return true;
}

std::string BaseName(const char* name)
{
	std::string s = name ? name : "";
	for (char& ch : s)
		ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
	for (;;)
	{
		const size_t sp = s.find_last_of(' ');
		if (sp == std::string::npos)
			break;
		const std::string_view last(s.c_str() + sp + 1);
		if (!IsRoman(last) && last != "rk." && last != "rk.ii" && last != "rk.iii"
			&& last != "azia" && last != "beza" && last != "caza"
			&& last != "(azia)" && last != "(beza)" && last != "(caza)")
			break;
		s.resize(sp);
	}
	return s;
}

bool CastBy(const eqlib::EQ_Spell* sp, int classId)
{
	if (classId <= 0)
		return true;
	const uint8_t lvl = sp->ClassLevel[classId];
	return lvl > 0 && lvl < 255;
}

int Find(std::span<const GroupLine> table, int key)
{
	for (const GroupLine& g : table)
		if (g.key == key)
			return g.line;
	return -1;
}

int ByGroup(const ClassCatalogue& c, int group)
{
	if (group <= 0)
		return -1;
	if (const int line = Find(c.otherLines, group); line >= 0)
		return line;
	if (c.numbered > 0 && group / 1000 == c.numbered && (group / 100) % 10 >= 1)
		if (const int line = Find(c.numberedLines, group % 100); line >= 0)
			return line;
	if (c.classId > 0 && group >= 100000000 && (group / 1000) % 1000 == c.classId * 10)
		if (const int line = Find(c.modernLines, group % 1000); line >= 0)
			return line;
	return -1;
}

int ByAbility(const ClassCatalogue& c, std::string_view base)
{
	for (const NamedLine& a : c.abilities)
		if (base == a.name)
			return a.line;
	return -1;
}

LineRef Ref(const ClassCatalogue& c, int line)
{
	if (line < 0 || line >= static_cast<int>(c.lines.size()))
		return {};
	return { c.cls, &c.lines[line], c.classId };
}

} // namespace

bool EndsWith(std::string_view s, std::string_view suffix)
{
	return s.size() > suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool StartsWith(std::string_view s, std::string_view prefix)
{
	return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

LineRef FindLineLook(const eqlib::EQ_Spell* sp, int ownClass)
{
	if (!sp)
		return {};
	const std::string base = BaseName(sp->Name);

	for (int pass = 0; pass < 3; ++pass)
		for (const ClassCatalogue* c : Catalogues())
			if (pass == 2 || (CastBy(sp, c->classId) && (pass == 1 || c->classId == ownClass)))
				if (const int line = ByAbility(*c, base); line >= 0)
					return Ref(*c, line);

	for (int pass = 0; pass < 2; ++pass)
		for (const ClassCatalogue* c : Catalogues())
		{
			if ((pass == 0) != (c->classId == ownClass) || !CastBy(sp, c->classId))
				continue;
			int line = ByGroup(*c, sp->SpellGroup);
			if (line < 0 && c->bySpell)
				line = c->bySpell(sp, base);
			if (line >= 0)
				return Ref(*c, line);
		}
	return {};
}

int LineLookCount()
{
	int total = 0;
	for (const ClassCatalogue* c : Catalogues())
		total += static_cast<int>(c->lines.size());
	return total;
}

LineRef LineLookAt(int index)
{
	for (const ClassCatalogue* c : Catalogues())
	{
		const int n = static_cast<int>(c->lines.size());
		if (index < n)
			return Ref(*c, index);
		index -= n;
	}
	return {};
}

} // namespace eqgfx::spellfx
