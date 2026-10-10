#include "pfs.h"

#include <zlib.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

namespace eqgfx::pfs {

static std::string lower(std::string s)
{
	for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	return s;
}

static constexpr uint32_t kMaxEntrySize = 256u << 20;

bool Archive::ExtractEntry(const Entry& e, std::vector<uint8_t>& out) const
{
	out.clear();
	if (e.size > kMaxEntrySize)
		return false;
	out.reserve(e.size);
	size_t pos = e.offset;
	while (out.size() < e.size)
	{
		if (pos + 8 > m_file.size())
			return false;
		uint32_t deflatedLen, inflatedLen;
		std::memcpy(&deflatedLen, &m_file[pos], 4);
		std::memcpy(&inflatedLen, &m_file[pos + 4], 4);
		pos += 8;
		if (pos + deflatedLen > m_file.size() || out.size() + inflatedLen > e.size)
			return false;
		if (inflatedLen == 0)
		{
			pos += deflatedLen;
			continue;
		}
		size_t oldSize = out.size();
		out.resize(oldSize + inflatedLen);
		uLongf destLen = inflatedLen;
		if (uncompress(out.data() + oldSize, &destLen, &m_file[pos], deflatedLen) != Z_OK || destLen != inflatedLen)
			return false;
		pos += deflatedLen;
	}
	return out.size() == e.size;
}

bool Archive::Open(const std::string& path)
{
	m_file.clear();
	m_entries.clear();
	m_names.clear();
	m_nameToIdx.clear();

	auto fail = [this]() {
		std::vector<uint8_t>().swap(m_file);
		m_entries.clear();
		return false;
	};

	FILE* f = nullptr;
	if (fopen_s(&f, path.c_str(), "rb") != 0 || !f)
		return false;
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (sz < 12) { fclose(f); return false; }
	m_file.resize(static_cast<size_t>(sz));
	size_t rd = fread(m_file.data(), 1, m_file.size(), f);
	fclose(f);
	if (rd != m_file.size())
		return fail();

	uint32_t dirOffset, magic;
	std::memcpy(&dirOffset, &m_file[0], 4);
	std::memcpy(&magic, &m_file[4], 4);
	if (magic != 0x20534650u)
		return fail();
	if (static_cast<size_t>(dirOffset) + 4 > m_file.size())
		return fail();

	uint32_t count;
	std::memcpy(&count, &m_file[dirOffset], 4);
	size_t p = dirOffset + 4;
	if (count > (m_file.size() - p) / 12)
		return fail();
	m_entries.reserve(count);
	for (uint32_t i = 0; i < count; ++i)
	{
		if (p + 12 > m_file.size())
			return fail();
		Entry e;
		std::memcpy(&e.offset, &m_file[p + 4], 4);
		std::memcpy(&e.size, &m_file[p + 8], 4);
		m_entries.push_back(e);
		p += 12;
	}
	if (m_entries.empty())
		return fail();

	std::sort(m_entries.begin(), m_entries.end(),
		[](const Entry& a, const Entry& b) { return a.offset < b.offset; });

	std::vector<uint8_t> nb;
	if (ExtractEntry(m_entries.back(), nb) && nb.size() >= 4)
	{
		uint32_t n;
		std::memcpy(&n, nb.data(), 4);
		size_t q = 4;
		m_names.resize(m_entries.size());
		for (uint32_t i = 0; i < n && i < m_entries.size() && q + 4 <= nb.size(); ++i)
		{
			uint32_t len;
			std::memcpy(&len, &nb[q], 4);
			q += 4;
			if (q + len > nb.size())
				break;
			size_t sl = len;
			while (sl > 0 && nb[q + sl - 1] == '\0') --sl;
			std::string nm = lower(std::string(reinterpret_cast<const char*>(&nb[q]), sl));
			m_names[i] = nm;
			m_nameToIdx[nm] = i;
			q += len;
		}
	}
	return true;
}

bool Archive::Contains(const std::string& name) const
{
	return m_nameToIdx.find(lower(name)) != m_nameToIdx.end();
}

bool Archive::Extract(const std::string& name, std::vector<uint8_t>& out) const
{
	auto it = m_nameToIdx.find(lower(name));
	if (it == m_nameToIdx.end())
		return false;
	return ExtractEntry(m_entries[it->second], out);
}

} // namespace eqgfx::pfs
