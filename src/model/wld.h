// Classic Verant WLD parser (older races, e.g. globalhaf_chr.wld) - the fragment
// container that predates EQG. Extracts the character BODY mesh(es) (fragment
// type 0x36, DmSpriteDef) and assembles them at bind pose via the 0x10 skeleton
// (0x36 vertices are BONE-LOCAL; without the skeleton the mesh is a collapsed
// blob). Produces the same eqg::Model the DX11 renderer consumes. On old models
// the worn head (hair/helm) is a SEPARATE mesh (HAFHE00..03) living in a
// different archive (global_chr.wld); MergeHeadMesh grafts it on via the body's
// head bone.
#pragma once

#include "eqg.h"
#include <cstdint>
#include <string>

namespace eqgfx::wld {

// The bind-pose world transform of a skeleton bone (world = R*local + T) + its index
// in Model::skeleton (so a grafted head can be skinned to it).
struct BoneXform
{
	float R[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	float T[3] = { 0, 0, 0 };
	int   index = -1;
	bool  valid = false;
};

// Parse a WLD buffer and merge every 0x36 mesh whose name starts with the
// (uppercased) race code - e.g. "haf" -> HAF_DMSPRITEDEF, HAFEYE_R/L - assembling
// bind pose from the 0x10 skeleton. Fills 'out' (Z-up) with geometry + per-mesh
// materials. If 'outHeadBone' is non-null it receives the world transform of the
// bone whose name contains "HEAD" (the neck/head attach point, for MergeHeadMesh).
// Returns false with a reason in 'err' on bad magic / no matching mesh.
bool ParseCharacter(const uint8_t* data, size_t size, const std::string& raceCode,
	eqg::Model& out, std::string& err, BoneXform* outHeadBone = nullptr);

// Parse a single named 0x36 mesh from a (different) WLD buffer - the worn head
// HAFHE00..03 from global_chr.wld - transform every vertex by the body's head-bone
// world transform, and append it (geometry + its own materials) to 'out'. The head
// mesh is rigid (head-local space); 'head' comes from ParseCharacter's outHeadBone.
// adjYaw/Pitch/Roll (radians) rotate the head in place before the bone transform (a
// user tweak to seat it); dz nudges its world height. Returns false with a reason in
// 'err' if the mesh isn't found / can't be parsed.
bool MergeHeadMesh(const uint8_t* data, size_t size, const std::string& meshName,
	const BoneXform& head, eqg::Model& out, std::string& err,
	float adjYaw = 0.0f, float adjPitch = 0.0f, float adjRoll = 0.0f, float dz = 0.0f);

} // namespace eqgfx::wld
