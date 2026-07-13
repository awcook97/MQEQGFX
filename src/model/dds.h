// Minimal DDS reader for EQ character textures. EQ ships DXT1/3/5 (BC1/2/3)
// compressed DDS with a standard 128-byte header and no mips - the GPU consumes
// the block data directly, so we just parse the header and hand off the bytes.
// Also handles plain 32-bit uncompressed DDS as a fallback. See model_format_spec.md.
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
	uint32_t          rowPitch = 0;      // bytes per row (per 4x4 block-row for BC)
	std::vector<uint8_t> pixels;         // top mip only
	bool valid() const { return format != Unknown && width && height && !pixels.empty(); }
};

// Parse a DDS buffer (top mip only). Returns false on bad magic / unsupported format.
bool ParseDDS(const uint8_t* data, size_t size, Image& out);

// Parse an 8-bit palettized Windows BMP (EQ's global armor textures) into a
// top-down RGBA8 Image. Returns false on bad magic / unsupported bit depth.
bool ParseBMP(const uint8_t* data, size_t size, Image& out);

// Dispatch on magic: "DDS " -> ParseDDS, "BM" -> ParseBMP.
bool ParseTexture(const uint8_t* data, size_t size, Image& out);

} // namespace eqgfx::dds
