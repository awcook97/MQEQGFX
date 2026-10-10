#include "eqg.h"

#include <cfloat>
#include <cstring>

namespace eqgfx::eqg {

static uint32_t rd32(const uint8_t* p) { uint32_t v; std::memcpy(&v, p, 4); return v; }
static float    rdf(const uint8_t* p)  { float v;    std::memcpy(&v, p, 4); return v; }

static std::string strAt(const uint8_t* pool, uint32_t poolSize, uint32_t off)
{
	if (off >= poolSize)
		return std::string();
	const char* s = reinterpret_cast<const char*>(pool + off);
	size_t maxLen = poolSize - off;
	size_t len = 0;
	while (len < maxLen && s[len] != '\0') ++len;
	return std::string(s, len);
}

static bool ParseMaterials(const uint8_t* data, size_t size, const uint8_t* pool, uint32_t poolSize,
	uint32_t count, size_t& p, std::vector<Material>& out)
{
	for (uint32_t m = 0; m < count; ++m)
	{
		if (p + 0x10 > size)
			return false;
		uint32_t nameOff   = rd32(data + p + 0x04);
		uint32_t propCount = rd32(data + p + 0x0C);
		Material mat;
		mat.name = strAt(pool, poolSize, nameOff);
		size_t pp = p + 0x10;
		for (uint32_t j = 0; j < propCount; ++j)
		{
			if (pp + 12 > size)
				return false;
			uint32_t pnameOff = rd32(data + pp);
			uint32_t ptype    = rd32(data + pp + 4);
			uint32_t pval     = rd32(data + pp + 8);
			if (ptype == 2)
			{
				std::string pname = strAt(pool, poolSize, pnameOff);
				if (pname == "e_TextureDiffuse0")
					mat.diffuseTexture = strAt(pool, poolSize, pval);
			}
			pp += 12;
		}
		out.push_back(std::move(mat));
		p = pp;
	}
	return true;
}

static void AddVertexBounds(Model& out, const Vertex& vx)
{
	const float* pos = &vx.px;
	for (int i = 0; i < 3; ++i)
	{
		if (pos[i] < out.minB[i]) out.minB[i] = pos[i];
		if (pos[i] > out.maxB[i]) out.maxB[i] = pos[i];
	}
}

bool ParseModel(const uint8_t* data, size_t size, Model& out, std::string& err)
{
	out = Model{};
	for (int i = 0; i < 3; ++i) { out.minB[i] = FLT_MAX; out.maxB[i] = -FLT_MAX; }

	if (size < 4) { err = "buffer too small"; return false; }

	const bool skinned = std::memcmp(data, "EQGS", 4) == 0;
	const bool staticM = std::memcmp(data, "EQGM", 4) == 0;
	if (!skinned && !staticM) { err = "not an EQGS/EQGM model"; return false; }

	if (skinned)
	{
		if (size < 0x18) { err = "truncated EQGS header"; return false; }
		uint32_t poolSize = rd32(data + 8);
		uint32_t numMat   = rd32(data + 12);
		uint32_t numBone  = rd32(data + 16);
		uint32_t numMesh  = rd32(data + 20);
		out.boneCount = static_cast<int>(numBone);
		if (0x18 + static_cast<size_t>(poolSize) > size) { err = "bad string pool"; return false; }
		const uint8_t* pool = data + 0x18;
		size_t p = 0x18 + poolSize;

		if (!ParseMaterials(data, size, pool, poolSize, numMat, p, out.materials)) { err = "bad material list"; return false; }

		if (p + static_cast<size_t>(numBone) * 0x38 > size) { err = "bad bone list"; return false; }
		p += static_cast<size_t>(numBone) * 0x38;

		for (uint32_t mm = 0; mm < numMesh; ++mm)
		{
			if (p + 0x14 > size) { err = "bad submesh header"; return false; }
			uint32_t nameOff = rd32(data + p + 0x04);
			uint32_t vc  = rd32(data + p + 0x08);
			uint32_t tc  = rd32(data + p + 0x0C);
			uint32_t bac = rd32(data + p + 0x10);
			size_t vptr = p + 0x14;
			size_t tptr = vptr + static_cast<size_t>(vc) * 0x20;
			size_t bptr = tptr + static_cast<size_t>(tc) * 0x14;
			size_t next = bptr + static_cast<size_t>(bac) * 0x24;
			if (next > size) { err = "bad submesh geometry"; return false; }

			uint32_t base = static_cast<uint32_t>(out.vertices.size());
			for (uint32_t v = 0; v < vc; ++v)
			{
				const uint8_t* q = data + vptr + static_cast<size_t>(v) * 0x20;
				Vertex vx;
				vx.px = rdf(q);      vx.py = rdf(q + 4);  vx.pz = rdf(q + 8);
				vx.nx = rdf(q + 12); vx.ny = rdf(q + 16); vx.nz = rdf(q + 20);
				vx.u  = rdf(q + 24); vx.v  = rdf(q + 28);
				out.vertices.push_back(vx);
				AddVertexBounds(out, vx);
			}

			SubMesh sm;
			sm.name = strAt(pool, poolSize, nameOff);
			sm.firstIndex = static_cast<uint32_t>(out.indices.size());
			for (uint32_t t = 0; t < tc; ++t)
			{
				const uint8_t* q = data + tptr + static_cast<size_t>(t) * 0x14;
				uint32_t i0 = rd32(q), i1 = rd32(q + 4), i2 = rd32(q + 8);
				int mat = static_cast<int>(rd32(q + 12));
				if (i0 >= vc || i1 >= vc || i2 >= vc)
					continue;
				out.indices.push_back(base + i0);
				out.indices.push_back(base + i2);
				out.indices.push_back(base + i1);
				if (sm.materialIndex < 0)
					sm.materialIndex = mat;
			}
			sm.indexCount = static_cast<uint32_t>(out.indices.size()) - sm.firstIndex;
			out.submeshes.push_back(std::move(sm));
			p = next;
		}
		return true;
	}

	if (size < 0x1C) { err = "truncated EQGM header"; return false; }
	uint32_t version  = rd32(data + 4);
	uint32_t poolSize = rd32(data + 8);
	uint32_t numMat   = rd32(data + 12);
	uint32_t numVert  = rd32(data + 16);
	uint32_t numTri   = rd32(data + 20);
	out.boneCount = static_cast<int>(rd32(data + 24));
	if (0x1C + static_cast<size_t>(poolSize) > size) { err = "bad string pool"; return false; }
	const uint8_t* pool = data + 0x1C;
	size_t p = 0x1C + poolSize;
	if (!ParseMaterials(data, size, pool, poolSize, numMat, p, out.materials)) { err = "bad material list"; return false; }

	size_t stride = (version > 2) ? 0x2C : 0x20;
	size_t vptr = p;
	size_t tptr = vptr + static_cast<size_t>(numVert) * stride;
	if (tptr + static_cast<size_t>(numTri) * 0x14 > size) { err = "bad EQGM geometry"; return false; }
	for (uint32_t v = 0; v < numVert; ++v)
	{
		const uint8_t* q = data + vptr + static_cast<size_t>(v) * stride;
		Vertex vx;
		vx.px = rdf(q);      vx.py = rdf(q + 4);  vx.pz = rdf(q + 8);
		vx.nx = rdf(q + 12); vx.ny = rdf(q + 16); vx.nz = rdf(q + 20);
		size_t uvOff = (version > 2) ? 0x1C : 0x18;
		vx.u = rdf(q + uvOff); vx.v = rdf(q + uvOff + 4);
		out.vertices.push_back(vx);
		AddVertexBounds(out, vx);
	}
	SubMesh sm;
	sm.firstIndex = 0;
	for (uint32_t t = 0; t < numTri; ++t)
	{
		const uint8_t* q = data + tptr + static_cast<size_t>(t) * 0x14;
		uint32_t i0 = rd32(q), i1 = rd32(q + 4), i2 = rd32(q + 8);
		int mat = static_cast<int>(rd32(q + 12));
		if (i0 >= numVert || i1 >= numVert || i2 >= numVert)
			continue;
		out.indices.push_back(i0);
		out.indices.push_back(i2);
		out.indices.push_back(i1);
		if (sm.materialIndex < 0)
			sm.materialIndex = mat;
	}
	sm.indexCount = static_cast<uint32_t>(out.indices.size());
	out.submeshes.push_back(std::move(sm));
	(void)version;
	return true;
}

} // namespace eqgfx::eqg
