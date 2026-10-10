#include "dds.h"

#include <cstring>

namespace eqgfx::dds {

static uint32_t rd32(const uint8_t* p, size_t o) { uint32_t v; std::memcpy(&v, p + o, 4); return v; }
static int32_t  rdi32(const uint8_t* p, size_t o) { int32_t v; std::memcpy(&v, p + o, 4); return v; }
static uint16_t rd16(const uint8_t* p, size_t o) { uint16_t v; std::memcpy(&v, p + o, 2); return v; }

static constexpr uint32_t kMaxDim = 16384;

bool ParseDDS(const uint8_t* data, size_t size, Image& out)
{
	out = Image{};
	if (size < 128 || std::memcmp(data, "DDS ", 4) != 0) return false;

	uint32_t height = rd32(data, 12);
	uint32_t width  = rd32(data, 16);
	uint32_t pfFlags = rd32(data, 80);
	const uint8_t* fourcc = data + 84;
	if (width == 0 || height == 0 || width > kMaxDim || height > kMaxDim) return false;

	uint32_t blockBytes = 0;
	Image::Format fmt = Image::Unknown;
	if (pfFlags & 0x4)
	{
		if (std::memcmp(fourcc, "DXT1", 4) == 0)      { fmt = Image::BC1; blockBytes = 8; }
		else if (std::memcmp(fourcc, "DXT3", 4) == 0) { fmt = Image::BC2; blockBytes = 16; }
		else if (std::memcmp(fourcc, "DXT5", 4) == 0) { fmt = Image::BC3; blockBytes = 16; }
		else return false;

		uint32_t blocksW = (width + 3) / 4;
		uint32_t blocksH = (height + 3) / 4;
		out.rowPitch = blocksW * blockBytes;
		size_t need = static_cast<size_t>(out.rowPitch) * blocksH;
		if (128 + need > size) return false;
		out.pixels.assign(data + 128, data + 128 + need);
	}
	else if (pfFlags & 0x40)
	{
		uint32_t bits = rd32(data, 88);
		if (bits != 32) return false;
		out.rowPitch = width * 4;
		size_t need = static_cast<size_t>(out.rowPitch) * height;
		if (128 + need > size) return false;
		fmt = Image::RGBA8;
		out.pixels.assign(data + 128, data + 128 + need);
		uint32_t aMask = rd32(data, 104);
		bool bgra = (rd32(data, 92) == 0x00FF0000);
		if (bgra)
			for (size_t i = 0; i + 4 <= out.pixels.size(); i += 4)
				std::swap(out.pixels[i], out.pixels[i + 2]);
		(void)aMask;
	}
	else
		return false;

	out.width = width;
	out.height = height;
	out.format = fmt;
	return true;
}

bool ParseBMP(const uint8_t* data, size_t size, Image& out)
{
	out = Image{};
	if (size < 54 || data[0] != 'B' || data[1] != 'M') return false;
	uint32_t dataOff = rd32(data, 10);
	uint32_t hdrSize = rd32(data, 14);
	int32_t  width   = rdi32(data, 18);
	int32_t  height  = rdi32(data, 22);
	uint16_t bpp     = rd16(data, 28);
	uint32_t compression = rd32(data, 30);
	uint32_t clrUsed = rd32(data, 46);
	if (bpp != 8 || compression != 0 || width <= 0 || height == 0) return false;

	bool bottomUp = height > 0;
	uint32_t h = bottomUp ? static_cast<uint32_t>(height) : 0u - static_cast<uint32_t>(height);
	uint32_t w = static_cast<uint32_t>(width);
	if (w > kMaxDim || h > kMaxDim) return false;

	size_t palOff = 14 + static_cast<size_t>(hdrSize);
	uint32_t palCount = clrUsed ? clrUsed : 256;
	if (palOff + static_cast<size_t>(palCount) * 4 > size) return false;

	uint32_t rowStride = (w + 3) & ~3u;
	if (dataOff + static_cast<size_t>(rowStride) * h > size) return false;

	out.pixels.resize(static_cast<size_t>(w) * h * 4);
	out.rowPitch = w * 4;
	for (uint32_t y = 0; y < h; ++y)
	{
		uint32_t srcRow = bottomUp ? (h - 1 - y) : y;
		const uint8_t* src = data + dataOff + static_cast<size_t>(srcRow) * rowStride;
		uint8_t* dst = out.pixels.data() + static_cast<size_t>(y) * w * 4;
		for (uint32_t x = 0; x < w; ++x)
		{
			uint32_t idx = src[x];
			const uint8_t* pal = data + palOff + (idx < palCount ? idx : 0) * 4;
			dst[x * 4 + 0] = pal[2];
			dst[x * 4 + 1] = pal[1];
			dst[x * 4 + 2] = pal[0];
			dst[x * 4 + 3] = 255;
		}
	}
	out.width = w;
	out.height = h;
	out.format = Image::RGBA8;
	return true;
}

bool ParseTexture(const uint8_t* data, size_t size, Image& out)
{
	if (size >= 4 && std::memcmp(data, "DDS ", 4) == 0) return ParseDDS(data, size, out);
	if (size >= 2 && data[0] == 'B' && data[1] == 'M')  return ParseBMP(data, size, out);
	return false;
}

} // namespace eqgfx::dds
