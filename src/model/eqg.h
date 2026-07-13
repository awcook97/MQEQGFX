// EQG character-model parser: EQGS (.mds, skinned) + EQGM (.mod, static).
// Produces a bind-pose triangle mesh - .mds vertices are already in bind-pose
// model space, so no skinning is needed to draw the default pose.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace eqgfx::eqg {

struct Vertex
{
	float px, py, pz;   // position (EQ world units)
	float nx, ny, nz;   // normal
	float u, v;         // texcoord (as stored on disk)
};

struct SubMesh
{
	std::string name;
	uint32_t    firstIndex = 0;   // into Model::indices
	uint32_t    indexCount = 0;
	int         materialIndex = -1;
};

struct Material
{
	std::string name;
	std::string diffuseTexture;   // "e_TextureDiffuse0" filename ("" / "None" = untextured)
	uint32_t    tint = 0xFFFFFFFF; // 0xRRGGBB dye multiply (0xFFFFFFFF = none); set from equipment
};

// A skeleton bone's BIND-POSE local transform (relative to its parent) + hierarchy,
// for re-posing. Forward kinematics: world = parentWorld ∘ (localR, localT).
struct Bone
{
	int         parent = -1;
	float       localR[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	float       localT[3] = { 0, 0, 0 };
	std::string name;
};

struct Model
{
	std::vector<Vertex>   vertices;   // concatenated; submesh-local indices rebased (WORLD, bind pose)
	std::vector<uint32_t> indices;    // triangle list (engine winding)
	std::vector<SubMesh>  submeshes;
	std::vector<Material> materials;
	int   boneCount = 0;
	float minB[3] = { 0, 0, 0 };
	float maxB[3] = { 0, 0, 0 };

	// Skinning data (parallel arrays; empty on EQG models = not re-poseable). The WLD
	// parser fills these alongside the baked bind-pose 'vertices' so the model can be
	// re-skinned into a new pose without re-reading files.
	std::vector<Vertex> localVertices;   // parallel to 'vertices', in bone-LOCAL space
	std::vector<int>    vertexBone;       // parallel to 'vertices'; bone index (-1 = keep world)
	std::vector<Bone>   skeleton;         // bind-pose local transforms + parents
};

// Parse an EQGS/.mds (skinned) or EQGM/.mod (static) buffer into a bind-pose
// mesh. Returns false with a reason in 'err' on bad magic/format.
bool ParseModel(const uint8_t* data, size_t size, Model& out, std::string& err);

} // namespace eqgfx::eqg
