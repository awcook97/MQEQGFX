// PFS archive reader (.s3d / .eqg) - the classic Verant "PFS " container.
// Reads the whole archive into memory and indexes entries by filename using the
// archive's own name list (the last entry, in offset order - the classic S3D
// convention; the per-entry CRC field is NOT used).
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace eqgfx::pfs {

class Archive
{
public:
	// Open a .s3d/.eqg by absolute path. Returns false if missing / not a PFS file.
	bool Open(const std::string& path);

	bool Contains(const std::string& name) const;                       // by member filename (case-insensitive)
	bool Extract(const std::string& name, std::vector<uint8_t>& out) const;   // decompress a member

	// All member filenames (lowercased), in archive order.
	const std::vector<std::string>& MemberNames() const { return m_names; }
	size_t EntryCount() const { return m_entries.size(); }

private:
	struct Entry { uint32_t offset, size; };
	bool ExtractEntry(const Entry& e, std::vector<uint8_t>& out) const;

	std::vector<uint8_t>                     m_file;      // whole archive
	std::vector<Entry>                       m_entries;   // sorted by offset (name-list order)
	std::vector<std::string>                 m_names;     // lowercased, parallel to m_entries
	std::unordered_map<std::string, size_t>  m_nameToIdx;
};

} // namespace eqgfx::pfs
