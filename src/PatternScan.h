#pragma once

#include <mq/Plugin.h>

#include <cstdint>
#include <string>
#include <vector>

namespace eqgfx::scan {

struct Module { uintptr_t base = 0; uintptr_t size = 0; };

inline Module ModuleAt(uintptr_t base)
{
	Module m;
	m.base = base;
	if (!base)
		return m;
	auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
	auto nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
	m.size = nt->OptionalHeader.SizeOfImage;
	return m;
}

inline Module Graphics() { return ModuleAt(eqlib::EQGraphicsBaseAddress); }
inline Module Game()     { return ModuleAt(eqlib::EQGameBaseAddress); }

inline uintptr_t Find(const Module& m, const char* sig)
{
	if (!m.base || !m.size)
		return 0;
	std::vector<uint8_t> bytes;
	std::string mask;
	for (const char* p = sig; *p; )
	{
		while (*p == ' ') ++p;
		if (!*p) break;
		if (p[0] == '?')
		{
			bytes.push_back(0);
			mask.push_back('?');
			p += (p[1] == '?') ? 2 : 1;
		}
		else
		{
			bytes.push_back(static_cast<uint8_t>(strtoul(std::string(p, 2).c_str(), nullptr, 16)));
			mask.push_back('x');
			p += 2;
		}
	}
	const uintptr_t last = m.base + m.size - bytes.size();
	const uintptr_t hit = FindPattern(m.base, last - m.base + 1, bytes.data(), mask.c_str());
	if (hit && hit < last && FindPattern(hit + 1, last - hit, bytes.data(), mask.c_str()))
		return 0;
	return hit;
}

inline uintptr_t Rip(uintptr_t insn, uint32_t dispOff, uint32_t len)
{
	return insn + len + *reinterpret_cast<const int32_t*>(insn + dispOff);
}

inline uintptr_t Rip(const Module& m, uintptr_t insn, uint32_t dispOff, uint32_t len)
{
	if (!m.base || insn < m.base || insn + dispOff + sizeof(int32_t) > m.base + m.size)
		return 0;
	const uintptr_t target = Rip(insn, dispOff, len);
	return target >= m.base && target < m.base + m.size ? target : 0;
}

} // namespace eqgfx::scan
