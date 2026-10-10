#include "wld.h"

#include <cfloat>
#include <cmath>
#include <cstring>
#include <vector>

namespace eqgfx::wld {

namespace {

const uint8_t kKey[8] = { 0x95, 0x3A, 0xC5, 0x2A, 0x95, 0x7A, 0x95, 0x6A };

uint32_t ru32(const uint8_t* b, size_t o) { uint32_t v; std::memcpy(&v, b + o, 4); return v; }
int32_t  ri32(const uint8_t* b, size_t o) { int32_t v; std::memcpy(&v, b + o, 4); return v; }
uint16_t ru16(const uint8_t* b, size_t o) { uint16_t v; std::memcpy(&v, b + o, 2); return v; }
int16_t  ri16(const uint8_t* b, size_t o) { int16_t v; std::memcpy(&v, b + o, 2); return v; }
float    rf32(const uint8_t* b, size_t o) { float v; std::memcpy(&v, b + o, 4); return v; }
int8_t   ri8(const uint8_t* b, size_t o)  { return static_cast<int8_t>(b[o]); }

struct Frag { uint32_t id; const uint8_t* d; uint32_t sz; };

struct Bone { float R[9]; float T[3]; std::string name; };

void matmul(const float A[9], const float B[9], float out[9])
{
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			out[i * 3 + j] = A[i * 3 + 0] * B[0 * 3 + j] + A[i * 3 + 1] * B[1 * 3 + j] + A[i * 3 + 2] * B[2 * 3 + j];
}
void matvec(const float A[9], const float v[3], float out[3])
{
	for (int i = 0; i < 3; ++i)
		out[i] = A[i * 3 + 0] * v[0] + A[i * 3 + 1] * v[1] + A[i * 3 + 2] * v[2];
}

std::string upper(std::string s)
{
	for (char& c : s) if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 32);
	return s;
}

} // namespace

bool ParseCharacter(const uint8_t* data, size_t size, const std::string& raceCode,
	eqg::Model& out, std::string& err, BoneXform* outHeadBone)
{
	out = eqg::Model{};
	for (int i = 0; i < 3; ++i) { out.minB[i] = FLT_MAX; out.maxB[i] = -FLT_MAX; }
	if (outHeadBone) *outHeadBone = BoneXform{};

	if (size < 28) { err = "truncated WLD header"; return false; }
	if (ru32(data, 0) != 0x54503D02u) { err = "not a WLD file"; return false; }
	uint32_t version     = ru32(data, 4);
	uint32_t fragCount   = ru32(data, 8);
	uint32_t strHashSize = ru32(data, 20);
	const bool newWorld  = (version == 0x1000C800u);

	if (28 + static_cast<size_t>(strHashSize) > size) { err = "bad string-hash size"; return false; }
	std::vector<char> sh(data + 28, data + 28 + strHashSize);
	for (size_t i = 0; i < sh.size(); ++i) sh[i] = static_cast<char>(sh[i] ^ kKey[i & 7]);

	auto getName = [&](int32_t ref) -> std::string {
		if (ref >= 0) return std::string();
		size_t off = 0u - static_cast<uint32_t>(ref);
		if (off >= sh.size()) return std::string();
		size_t e = off;
		while (e < sh.size() && sh[e] != '\0') ++e;
		return std::string(&sh[off], e - off);
	};

	std::vector<Frag> frags;
	frags.push_back({ 0, nullptr, 0 });
	size_t p = 28 + strHashSize;
	for (uint32_t i = 0; i < fragCount; ++i)
	{
		if (p + 8 > size) break;
		uint32_t fsz = ru32(data, p);
		uint32_t fid = ru32(data, p + 4);
		if (p + 8 + static_cast<size_t>(fsz) > size) break;
		frags.push_back({ fid, data + p + 8, fsz });
		p += 8 + fsz;
	}

	const std::string code = upper(raceCode);

	std::vector<Bone> bones;
	{
		uint32_t skelRef = 0;
		for (size_t i = 1; i < frags.size(); ++i)
		{
			if (frags[i].id != 0x10 || frags[i].sz < 12) continue;
			std::string nm = getName(ri32(frags[i].d, 0));
			if (skelRef == 0) skelRef = static_cast<uint32_t>(i);
			if (upper(nm).compare(0, code.size(), code) == 0) { skelRef = static_cast<uint32_t>(i); break; }
		}
		if (skelRef)
		{
			const Frag& sf = frags[skelRef];
			uint32_t flags   = ru32(sf.d, 4);
			int32_t  numDags = ri32(sf.d, 8);
			size_t o = 12 + 4;
			if (flags & 0x1) o += 12;
			if (flags & 0x2) o += 4;

			struct Dag { int32_t trackRef; std::string name; std::vector<int32_t> kids; };
			std::vector<Dag> dags;
			bool ok = numDags > 0;
			for (int b = 0; b < numDags && ok; ++b)
			{
				if (o + 20 > sf.sz) { ok = false; break; }
				int32_t nameRef  = ri32(sf.d, o + 0);
				int32_t trackRef = ri32(sf.d, o + 8);
				int32_t cc       = ri32(sf.d, o + 16);
				o += 20;
				Dag dag; dag.trackRef = trackRef; dag.name = getName(nameRef);
				for (int k = 0; k < cc; ++k)
				{
					if (o + 4 > sf.sz) { ok = false; break; }
					dag.kids.push_back(ri32(sf.d, o));
					o += 4;
				}
				dags.push_back(std::move(dag));
			}

			if (ok && !dags.empty())
			{
				std::vector<int> parent(dags.size(), -1);
				for (size_t b = 0; b < dags.size(); ++b)
					for (int k : dags[b].kids)
						if (k >= 0 && static_cast<size_t>(k) < dags.size()) parent[k] = static_cast<int>(b);

				auto localBind = [&](int32_t trackRef, float R[9], float T[3]) {
					R[0] = R[4] = R[8] = 1; R[1] = R[2] = R[3] = R[5] = R[6] = R[7] = 0;
					T[0] = T[1] = T[2] = 0;
					if (trackRef <= 0 || static_cast<uint32_t>(trackRef) >= frags.size()) return;
					const Frag& ti = frags[trackRef];
					if (ti.id != 0x13 || ti.sz < 8) return;
					int32_t defRef = ri32(ti.d, 4);
					if (defRef <= 0 || static_cast<uint32_t>(defRef) >= frags.size()) return;
					const Frag& td = frags[defRef];
					if (td.id != 0x12 || td.sz < 28) return;
					float qx = ri16(td.d, 14), qy = ri16(td.d, 16), qz = ri16(td.d, 18), qw = ri16(td.d, 12);
					float n = std::sqrt(qx * qx + qy * qy + qz * qz + qw * qw);
					if (n <= 0) { n = 1; qw = 1; }
					qx /= n; qy /= n; qz /= n; qw /= n;
					R[0] = 1 - 2 * (qy * qy + qz * qz); R[1] = 2 * (qx * qy - qz * qw);     R[2] = 2 * (qx * qz + qy * qw);
					R[3] = 2 * (qx * qy + qz * qw);     R[4] = 1 - 2 * (qx * qx + qz * qz); R[5] = 2 * (qy * qz - qx * qw);
					R[6] = 2 * (qx * qz - qy * qw);     R[7] = 2 * (qy * qz + qx * qw);     R[8] = 1 - 2 * (qx * qx + qy * qy);
					int16_t sDen = ri16(td.d, 26);
					if (sDen != 0)
					{
						T[0] = ri16(td.d, 20) / static_cast<float>(sDen);
						T[1] = ri16(td.d, 22) / static_cast<float>(sDen);
						T[2] = ri16(td.d, 24) / static_cast<float>(sDen);
					}
				};

				bones.resize(dags.size());
				out.skeleton.assign(dags.size(), eqg::Bone{});
				std::vector<bool> queued(dags.size(), false);
				std::vector<int> order; order.reserve(dags.size());
				for (size_t b = 0; b < dags.size(); ++b) if (parent[b] == -1) { order.push_back(static_cast<int>(b)); queued[b] = true; }
				for (size_t qi = 0; qi < order.size(); ++qi)
				{
					int b = order[qi];
					float lR[9], lT[3];
					localBind(dags[b].trackRef, lR, lT);
					eqg::Bone& sb = out.skeleton[b];
					sb.parent = parent[b];
					std::memcpy(sb.localR, lR, sizeof(lR));
					std::memcpy(sb.localT, lT, sizeof(lT));
					sb.name = dags[b].name;
					if (parent[b] == -1)
					{
						std::memcpy(bones[b].R, lR, sizeof(lR));
						std::memcpy(bones[b].T, lT, sizeof(lT));
					}
					else
					{
						const Bone& pb = bones[parent[b]];
						matmul(pb.R, lR, bones[b].R);
						float rt[3]; matvec(pb.R, lT, rt);
						bones[b].T[0] = rt[0] + pb.T[0];
						bones[b].T[1] = rt[1] + pb.T[1];
						bones[b].T[2] = rt[2] + pb.T[2];
					}
					bones[b].name = dags[b].name;
					for (int k : dags[b].kids)
						if (k >= 0 && static_cast<size_t>(k) < dags.size() && !queued[k]) { queued[k] = true; order.push_back(k); }
				}
			}
		}

		if (outHeadBone)
		{
			auto stripped = [&](size_t b) -> std::string {
				std::string bn = upper(bones[b].name);
				if (bn.compare(0, code.size(), code) == 0) bn.erase(0, code.size());
				if (bn.size() > 4 && bn.compare(bn.size() - 4, 4, "_DAG") == 0) bn.erase(bn.size() - 4);
				return bn;
			};
			int headIdx = -1;
			for (size_t b = 0; b < bones.size() && headIdx < 0; ++b)
			{
				std::string s = stripped(b);
				if (s == "HE" || s == "HEHEAD") headIdx = static_cast<int>(b);
			}
			for (size_t b = 0; b < bones.size() && headIdx < 0; ++b)
			{
				std::string bn = upper(bones[b].name);
				if (bn.find("HEAD") != std::string::npos && bn.find("POINT") == std::string::npos)
					headIdx = static_cast<int>(b);
			}
			if (headIdx < 0 && !bones.empty())
			{
				float bestZ = -FLT_MAX;
				for (size_t b = 0; b < bones.size(); ++b)
					if (bones[b].T[2] > bestZ) { bestZ = bones[b].T[2]; headIdx = static_cast<int>(b); }
			}
			if (headIdx >= 0)
			{
				std::memcpy(outHeadBone->R, bones[headIdx].R, sizeof(outHeadBone->R));
				std::memcpy(outHeadBone->T, bones[headIdx].T, sizeof(outHeadBone->T));
				outHeadBone->index = headIdx;
				outHeadBone->valid = true;
			}
		}
	}

	auto texFromFrag03 = [&](uint32_t ref) -> std::string {
		if (ref == 0 || ref >= frags.size() || frags[ref].id != 0x03) return std::string();
		const Frag& f = frags[ref];
		if (f.sz < 10) return std::string();
		int16_t ln = ri16(f.d, 8);
		if (ln <= 0 || 10 + static_cast<size_t>(ln) > f.sz) return std::string();
		std::string s(reinterpret_cast<const char*>(f.d) + 10, static_cast<size_t>(ln));
		for (size_t k = 0; k < s.size(); ++k) s[k] = static_cast<char>(s[k] ^ kKey[k & 7]);
		while (!s.empty() && s.back() == '\0') s.pop_back();
		return s;
	};

	auto texFromFrag05 = [&](int32_t ref05) -> std::string {
		if (ref05 <= 0 || static_cast<uint32_t>(ref05) >= frags.size()) return std::string();
		const Frag& f5 = frags[ref05];
		if (f5.id != 0x05 || f5.sz < 8) return std::string();
		for (size_t co = 0; co < 8; co += 4)
		{
			int32_t cand = ri32(f5.d, co);
			if (cand <= 0 || static_cast<uint32_t>(cand) >= frags.size() || frags[cand].id != 0x04) continue;
			const Frag& f4 = frags[cand];
			if (f4.sz < 12) continue;
			int32_t c4 = ri32(f4.d, 8);
			for (int j = 0; j < c4; ++j)
			{
				size_t off = 12 + static_cast<size_t>(j) * 4;
				if (off + 4 > f4.sz) break;
				std::string t = texFromFrag03(static_cast<uint32_t>(ri32(f4.d, off)));
				if (!t.empty()) return t;
			}
		}
		return std::string();
	};

	auto resolveMaterials = [&](uint32_t listRef) -> int {
		int base = static_cast<int>(out.materials.size());
		if (listRef == 0 || listRef >= frags.size()) return base;
		const Frag& lf = frags[listRef];
		if (lf.id != 0x31 || lf.sz < 12) return base;
		int32_t cnt = ri32(lf.d, 8);
		for (int i = 0; i < cnt; ++i)
		{
			size_t off = 12 + static_cast<size_t>(i) * 4;
			if (off + 4 > lf.sz) break;
			int32_t mref = ri32(lf.d, off);
			eqg::Material mat;
			if (mref > 0 && static_cast<uint32_t>(mref) < frags.size() && frags[mref].id == 0x30)
			{
				const Frag& mf = frags[mref];
				if (mf.sz >= 4) mat.name = getName(ri32(mf.d, 0));
				if (mf.sz >= 28) mat.diffuseTexture = texFromFrag05(ri32(mf.d, 24));
			}
			out.materials.push_back(std::move(mat));
		}
		return base;
	};

	auto addMesh = [&](const Frag& f) -> bool {
		const uint8_t* d = f.d;
		if (f.sz < 24 + 12 + 12 + 4 + 24 + 20) return false;
		int32_t nameRef    = ri32(d, 0);
		int32_t matListRef = ri32(d, 8);
		float cx = rf32(d, 24), cy = rf32(d, 28), cz = rf32(d, 32);
		size_t o = 24 + 12 + 12 + 4 + 24;
		uint16_t vc  = ru16(d, o);
		uint16_t tc  = ru16(d, o + 2);
		uint16_t nc  = ru16(d, o + 4);
		uint16_t cc  = ru16(d, o + 6);
		uint16_t pc  = ru16(d, o + 8);
		uint16_t vpc = ru16(d, o + 10);
		uint16_t ptc = ru16(d, o + 12);
		int16_t  rawScale = ri16(d, o + 18);
		o += 20;
		if (rawScale < 0 || rawScale > 30) return false;
		float scale = 1.0f / static_cast<float>(1 << rawScale);

		size_t vptr  = o;
		size_t tptr  = vptr + static_cast<size_t>(vc) * 6;
		size_t nptr  = tptr + static_cast<size_t>(tc) * (newWorld ? 8 : 4);
		size_t cptr  = nptr + static_cast<size_t>(nc) * 3;
		size_t pptr  = cptr + static_cast<size_t>(cc) * 4;
		size_t vpptr = pptr + static_cast<size_t>(pc) * 8;
		size_t gptr  = vpptr + static_cast<size_t>(vpc) * 4;
		if (gptr + static_cast<size_t>(ptc) * 4 > f.sz) return false;

		int matBase = resolveMaterials(static_cast<uint32_t>(matListRef));

		std::vector<int> vbone(vc, -1);
		{
			uint32_t vi = 0;
			for (uint16_t i = 0; i < vpc; ++i)
			{
				uint16_t rcount = ru16(d, vpptr + static_cast<size_t>(i) * 4);
				uint16_t bidx   = ru16(d, vpptr + static_cast<size_t>(i) * 4 + 2);
				for (uint16_t k = 0; k < rcount && vi < vc; ++k, ++vi) vbone[vi] = bidx;
			}
		}

		uint32_t base = static_cast<uint32_t>(out.vertices.size());
		for (uint16_t i = 0; i < vc; ++i)
		{
			const uint8_t* q = d + vptr + static_cast<size_t>(i) * 6;
			float lp[3] = { cx + ri16(q, 0) * scale, cy + ri16(q, 2) * scale, cz + ri16(q, 4) * scale };
			float ln[3] = { 0, 0, 1 };
			if (i < nc)
			{
				const uint8_t* nq = d + nptr + static_cast<size_t>(i) * 3;
				ln[0] = ri8(nq, 0) / 127.0f; ln[1] = ri8(nq, 1) / 127.0f; ln[2] = ri8(nq, 2) / 127.0f;
			}
			float u = 0, v = 0;
			if (i < tc)
			{
				const uint8_t* tq = d + tptr + static_cast<size_t>(i) * (newWorld ? 8 : 4);
				if (newWorld) { u = ri32(tq, 0) / 256.0f; v = ri32(tq, 4) / 256.0f; }
				else          { u = ri16(tq, 0) / 256.0f; v = ri16(tq, 2) / 256.0f; }
			}
			int b = vbone[i];
			eqg::Vertex loc{};
			loc.px = lp[0]; loc.py = lp[1]; loc.pz = lp[2];
			loc.nx = ln[0]; loc.ny = ln[1]; loc.nz = ln[2];
			loc.u = u; loc.v = v;
			out.localVertices.push_back(loc);
			out.vertexBone.push_back(b);

			if (b >= 0 && static_cast<size_t>(b) < bones.size())
			{
				float wp[3]; matvec(bones[b].R, lp, wp);
				lp[0] = wp[0] + bones[b].T[0]; lp[1] = wp[1] + bones[b].T[1]; lp[2] = wp[2] + bones[b].T[2];
				float wn[3]; matvec(bones[b].R, ln, wn);
				ln[0] = wn[0]; ln[1] = wn[1]; ln[2] = wn[2];
			}
			eqg::Vertex vx{};
			vx.px = lp[0]; vx.py = lp[1]; vx.pz = lp[2];
			vx.nx = ln[0]; vx.ny = ln[1]; vx.nz = ln[2];
			vx.u = u; vx.v = v;
			out.vertices.push_back(vx);
			const float pos[3] = { vx.px, vx.py, vx.pz };
			for (int k = 0; k < 3; ++k)
			{
				if (pos[k] < out.minB[k]) out.minB[k] = pos[k];
				if (pos[k] > out.maxB[k]) out.maxB[k] = pos[k];
			}
		}

		uint16_t polyCursor = 0;
		for (uint16_t g = 0; g < ptc; ++g)
		{
			uint16_t gcount  = ru16(d, gptr + static_cast<size_t>(g) * 4);
			uint16_t gmatidx = ru16(d, gptr + static_cast<size_t>(g) * 4 + 2);
			eqg::SubMesh sm;
			sm.name = getName(nameRef);
			sm.materialIndex = static_cast<int>(gmatidx) + matBase;
			sm.firstIndex = static_cast<uint32_t>(out.indices.size());
			for (uint16_t k = 0; k < gcount && polyCursor < pc; ++k, ++polyCursor)
			{
				const uint8_t* pq = d + pptr + static_cast<size_t>(polyCursor) * 8;
				uint16_t i0 = ru16(pq, 2), i1 = ru16(pq, 4), i2 = ru16(pq, 6);
				if (i0 >= vc || i1 >= vc || i2 >= vc) continue;
				out.indices.push_back(base + i0);
				out.indices.push_back(base + i1);
				out.indices.push_back(base + i2);
			}
			sm.indexCount = static_cast<uint32_t>(out.indices.size()) - sm.firstIndex;
			out.submeshes.push_back(std::move(sm));
		}
		return true;
	};

	const std::string hePrefix = code + "HE";
	int meshCount = 0;
	for (size_t i = 1; i < frags.size(); ++i)
	{
		if (frags[i].id != 0x36 || frags[i].sz < 4) continue;
		std::string nm = getName(ri32(frags[i].d, 0));
		std::string un = upper(nm);
		if (un.compare(0, code.size(), code) != 0) continue;
		if (un.size() > hePrefix.size() && un.compare(0, hePrefix.size(), hePrefix) == 0
			&& un[hePrefix.size()] >= '0' && un[hePrefix.size()] <= '9') continue;
		if (addMesh(frags[i])) ++meshCount;
	}

	if (meshCount == 0 || out.vertices.empty())
	{
		err = "no matching 0x36 mesh for race '" + raceCode + "'";
		if (outHeadBone) *outHeadBone = BoneXform{};
		return false;
	}
	out.boneCount = static_cast<int>(bones.size());
	return true;
}

bool MergeHeadMesh(const uint8_t* data, size_t size, const std::string& meshName,
	const BoneXform& head, eqg::Model& out, std::string& err,
	float adjYaw, float adjPitch, float adjRoll, float dz)
{
	if (!head.valid) { err = "no head bone"; return false; }

	float cYaw = std::cos(adjYaw), sYaw = std::sin(adjYaw);
	float cPit = std::cos(adjPitch), sPit = std::sin(adjPitch);
	float cRol = std::cos(adjRoll), sRol = std::sin(adjRoll);
	float Rz[9] = { cYaw, -sYaw, 0,  sYaw, cYaw, 0,  0, 0, 1 };
	float Rx[9] = { 1, 0, 0,  0, cPit, -sPit,  0, sPit, cPit };
	float Ry[9] = { cRol, 0, sRol,  0, 1, 0,  -sRol, 0, cRol };
	float Rzx[9]; matmul(Rz, Rx, Rzx);
	float adjR[9]; matmul(Rzx, Ry, adjR);
	float R[9]; matmul(head.R, adjR, R);
	if (size < 28 || ru32(data, 0) != 0x54503D02u) { err = "not a WLD file"; return false; }
	uint32_t version     = ru32(data, 4);
	uint32_t fragCount   = ru32(data, 8);
	uint32_t strHashSize = ru32(data, 20);
	const bool newWorld  = (version == 0x1000C800u);
	if (28 + static_cast<size_t>(strHashSize) > size) { err = "bad string-hash size"; return false; }

	std::vector<char> sh(data + 28, data + 28 + strHashSize);
	for (size_t i = 0; i < sh.size(); ++i) sh[i] = static_cast<char>(sh[i] ^ kKey[i & 7]);
	auto getName = [&](int32_t ref) -> std::string {
		if (ref >= 0) return std::string();
		size_t off = 0u - static_cast<uint32_t>(ref);
		if (off >= sh.size()) return std::string();
		size_t e = off; while (e < sh.size() && sh[e] != '\0') ++e;
		return std::string(&sh[off], e - off);
	};

	std::vector<Frag> frags;
	frags.push_back({ 0, nullptr, 0 });
	size_t p = 28 + strHashSize;
	for (uint32_t i = 0; i < fragCount; ++i)
	{
		if (p + 8 > size) break;
		uint32_t fsz = ru32(data, p);
		uint32_t fid = ru32(data, p + 4);
		if (p + 8 + static_cast<size_t>(fsz) > size) break;
		frags.push_back({ fid, data + p + 8, fsz });
		p += 8 + fsz;
	}

	auto texFromFrag03 = [&](uint32_t ref) -> std::string {
		if (ref == 0 || ref >= frags.size() || frags[ref].id != 0x03) return std::string();
		const Frag& f = frags[ref];
		if (f.sz < 10) return std::string();
		int16_t ln = ri16(f.d, 8);
		if (ln <= 0 || 10 + static_cast<size_t>(ln) > f.sz) return std::string();
		std::string s(reinterpret_cast<const char*>(f.d) + 10, static_cast<size_t>(ln));
		for (size_t k = 0; k < s.size(); ++k) s[k] = static_cast<char>(s[k] ^ kKey[k & 7]);
		while (!s.empty() && s.back() == '\0') s.pop_back();
		return s;
	};
	auto texFromFrag05 = [&](int32_t ref05) -> std::string {
		if (ref05 <= 0 || static_cast<uint32_t>(ref05) >= frags.size()) return std::string();
		const Frag& f5 = frags[ref05];
		if (f5.id != 0x05 || f5.sz < 8) return std::string();
		for (size_t co = 0; co < 8; co += 4)
		{
			int32_t cand = ri32(f5.d, co);
			if (cand <= 0 || static_cast<uint32_t>(cand) >= frags.size() || frags[cand].id != 0x04) continue;
			const Frag& f4 = frags[cand];
			if (f4.sz < 12) continue;
			int32_t c4 = ri32(f4.d, 8);
			for (int j = 0; j < c4; ++j)
			{
				size_t off = 12 + static_cast<size_t>(j) * 4;
				if (off + 4 > f4.sz) break;
				std::string t = texFromFrag03(static_cast<uint32_t>(ri32(f4.d, off)));
				if (!t.empty()) return t;
			}
		}
		return std::string();
	};

	const std::string want = upper(meshName);
	size_t mi = 0;
	for (size_t i = 1; i < frags.size(); ++i)
	{
		if (frags[i].id != 0x36 || frags[i].sz < 4) continue;
		std::string nm = upper(getName(ri32(frags[i].d, 0)));
		if (nm.size() >= want.size() && nm.compare(0, want.size(), want) == 0) { mi = i; break; }
	}
	if (mi == 0) { err = "head mesh '" + meshName + "' not found"; return false; }

	const Frag& f = frags[mi];
	const uint8_t* d = f.d;
	if (f.sz < 24 + 12 + 12 + 4 + 24 + 20) { err = "head mesh truncated"; return false; }
	int32_t matListRef = ri32(d, 8);
	float cx = rf32(d, 24), cy = rf32(d, 28), cz = rf32(d, 32);
	size_t o = 24 + 12 + 12 + 4 + 24;
	uint16_t vc  = ru16(d, o);
	uint16_t tc  = ru16(d, o + 2);
	uint16_t nc  = ru16(d, o + 4);
	uint16_t cc  = ru16(d, o + 6);
	uint16_t pc  = ru16(d, o + 8);
	uint16_t vpc = ru16(d, o + 10);
	uint16_t ptc = ru16(d, o + 12);
	int16_t  rawScale = ri16(d, o + 18);
	o += 20;
	if (rawScale < 0 || rawScale > 30) { err = "head mesh bad scale"; return false; }
	float scale = 1.0f / static_cast<float>(1 << rawScale);

	size_t vptr  = o;
	size_t tptr  = vptr + static_cast<size_t>(vc) * 6;
	size_t nptr  = tptr + static_cast<size_t>(tc) * (newWorld ? 8 : 4);
	size_t cptr  = nptr + static_cast<size_t>(nc) * 3;
	size_t pptr  = cptr + static_cast<size_t>(cc) * 4;
	size_t vpptr = pptr + static_cast<size_t>(pc) * 8;
	size_t gptr  = vpptr + static_cast<size_t>(vpc) * 4;
	if (gptr + static_cast<size_t>(ptc) * 4 > f.sz) { err = "head mesh overrun"; return false; }

	int matBase = static_cast<int>(out.materials.size());
	if (matListRef > 0 && static_cast<uint32_t>(matListRef) < frags.size() && frags[matListRef].id == 0x31)
	{
		const Frag& lf = frags[matListRef];
		int32_t cnt = (lf.sz >= 12) ? ri32(lf.d, 8) : 0;
		for (int i = 0; i < cnt; ++i)
		{
			size_t off = 12 + static_cast<size_t>(i) * 4;
			if (off + 4 > lf.sz) break;
			int32_t mref = ri32(lf.d, off);
			eqg::Material mat;
			if (mref > 0 && static_cast<uint32_t>(mref) < frags.size() && frags[mref].id == 0x30)
			{
				const Frag& mf = frags[mref];
				if (mf.sz >= 4)  mat.name = getName(ri32(mf.d, 0));
				if (mf.sz >= 28) mat.diffuseTexture = texFromFrag05(ri32(mf.d, 24));
			}
			out.materials.push_back(std::move(mat));
		}
	}

	float dzOff[3] = { head.R[6] * dz, head.R[7] * dz, head.R[8] * dz };
	uint32_t base = static_cast<uint32_t>(out.vertices.size());
	for (uint16_t i = 0; i < vc; ++i)
	{
		const uint8_t* q = d + vptr + static_cast<size_t>(i) * 6;
		float lp[3] = { cx + ri16(q, 0) * scale, cy + ri16(q, 2) * scale, cz + ri16(q, 4) * scale };
		float ln[3] = { 0, 0, 1 };
		if (i < nc)
		{
			const uint8_t* nq = d + nptr + static_cast<size_t>(i) * 3;
			ln[0] = ri8(nq, 0) / 127.0f; ln[1] = ri8(nq, 1) / 127.0f; ln[2] = ri8(nq, 2) / 127.0f;
		}
		float u = 0, v = 0;
		if (i < tc)
		{
			const uint8_t* tq = d + tptr + static_cast<size_t>(i) * (newWorld ? 8 : 4);
			if (newWorld) { u = ri32(tq, 0) / 256.0f; v = ri32(tq, 4) / 256.0f; }
			else          { u = ri16(tq, 0) / 256.0f; v = ri16(tq, 2) / 256.0f; }
		}
		float alp[3]; matvec(adjR, lp, alp);
		float aln[3]; matvec(adjR, ln, aln);
		eqg::Vertex loc{};
		loc.px = alp[0] + dzOff[0]; loc.py = alp[1] + dzOff[1]; loc.pz = alp[2] + dzOff[2];
		loc.nx = aln[0]; loc.ny = aln[1]; loc.nz = aln[2];
		loc.u = u; loc.v = v;
		out.localVertices.push_back(loc);
		out.vertexBone.push_back(head.index);

		float wp[3]; matvec(R, lp, wp);
		float wn[3]; matvec(R, ln, wn);
		eqg::Vertex vx{};
		vx.px = wp[0] + head.T[0]; vx.py = wp[1] + head.T[1]; vx.pz = wp[2] + head.T[2] + dz;
		vx.nx = wn[0]; vx.ny = wn[1]; vx.nz = wn[2];
		vx.u = u; vx.v = v;
		out.vertices.push_back(vx);
		const float pos[3] = { vx.px, vx.py, vx.pz };
		for (int k = 0; k < 3; ++k)
		{
			if (pos[k] < out.minB[k]) out.minB[k] = pos[k];
			if (pos[k] > out.maxB[k]) out.maxB[k] = pos[k];
		}
	}

	uint16_t polyCursor = 0;
	for (uint16_t g = 0; g < ptc; ++g)
	{
		uint16_t gcount  = ru16(d, gptr + static_cast<size_t>(g) * 4);
		uint16_t gmatidx = ru16(d, gptr + static_cast<size_t>(g) * 4 + 2);
		eqg::SubMesh sm;
		sm.name = getName(ri32(d, 0));
		sm.materialIndex = static_cast<int>(gmatidx) + matBase;
		sm.firstIndex = static_cast<uint32_t>(out.indices.size());
		for (uint16_t k = 0; k < gcount && polyCursor < pc; ++k, ++polyCursor)
		{
			const uint8_t* pq = d + pptr + static_cast<size_t>(polyCursor) * 8;
			uint16_t i0 = ru16(pq, 2), i1 = ru16(pq, 4), i2 = ru16(pq, 6);
			if (i0 >= vc || i1 >= vc || i2 >= vc) continue;
			out.indices.push_back(base + i0);
			out.indices.push_back(base + i1);
			out.indices.push_back(base + i2);
		}
		sm.indexCount = static_cast<uint32_t>(out.indices.size()) - sm.firstIndex;
		out.submeshes.push_back(std::move(sm));
	}
	return true;
}

} // namespace eqgfx::wld
