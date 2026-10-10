#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace eqgfx::pfs {

class Archive
{
public:
	bool Open(const std::string& path);

	bool Contains(const std::string& name) const;
	bool Extract(const std::string& name, std::vector<uint8_t>& out) const;

	const std::vector<std::string>& MemberNames() const { return m_names; }
	size_t EntryCount() const { return m_entries.size(); }

private:
	struct Entry { uint32_t offset, size; };
	bool ExtractEntry(const Entry& e, std::vector<uint8_t>& out) const;

	std::vector<uint8_t>                     m_file;
	std::vector<Entry>                       m_entries;
	std::vector<std::string>                 m_names;
	std::unordered_map<std::string, size_t>  m_nameToIdx;
};

} // namespace eqgfx::pfs
