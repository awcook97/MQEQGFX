#pragma once

#include <cstdint>
#include <vector>

namespace eqgfx::dds {

struct Image
{
	enum Format { Unknown, BC1, BC2, BC3, RGBA8 };
	uint32_t          width = 0;
	uint32_t          height = 0;
	Format            format = Unknown;
	uint32_t          rowPitch = 0;
	std::vector<uint8_t> pixels;
	bool valid() const { return format != Unknown && width && height && !pixels.empty(); }
};

bool ParseDDS(const uint8_t* data, size_t size, Image& out);

bool ParseBMP(const uint8_t* data, size_t size, Image& out);

bool ParseTexture(const uint8_t* data, size_t size, Image& out);

} // namespace eqgfx::dds
