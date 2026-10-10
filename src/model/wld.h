#pragma once

#include "eqg.h"
#include <cstdint>
#include <string>

namespace eqgfx::wld {

struct BoneXform
{
	float R[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	float T[3] = { 0, 0, 0 };
	int   index = -1;
	bool  valid = false;
};

bool ParseCharacter(const uint8_t* data, size_t size, const std::string& raceCode,
	eqg::Model& out, std::string& err, BoneXform* outHeadBone = nullptr);

bool MergeHeadMesh(const uint8_t* data, size_t size, const std::string& meshName,
	const BoneXform& head, eqg::Model& out, std::string& err,
	float adjYaw = 0.0f, float adjPitch = 0.0f, float adjRoll = 0.0f, float dz = 0.0f);

} // namespace eqgfx::wld
