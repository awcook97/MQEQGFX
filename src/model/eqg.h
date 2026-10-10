#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace eqgfx::eqg {

struct Vertex
{
	float px, py, pz;
	float nx, ny, nz;
	float u, v;
};

struct SubMesh
{
	std::string name;
	uint32_t    firstIndex = 0;
	uint32_t    indexCount = 0;
	int         materialIndex = -1;
};

struct Material
{
	std::string name;
	std::string diffuseTexture;
	uint32_t    tint = 0xFFFFFFFF;
};

struct Bone
{
	int         parent = -1;
	float       localR[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	float       localT[3] = { 0, 0, 0 };
	std::string name;
};

struct Model
{
	std::vector<Vertex>   vertices;
	std::vector<uint32_t> indices;
	std::vector<SubMesh>  submeshes;
	std::vector<Material> materials;
	int   boneCount = 0;
	float minB[3] = { 0, 0, 0 };
	float maxB[3] = { 0, 0, 0 };

	std::vector<Vertex> localVertices;
	std::vector<int>    vertexBone;
	std::vector<Bone>   skeleton;
};

bool ParseModel(const uint8_t* data, size_t size, Model& out, std::string& err);

} // namespace eqgfx::eqg
