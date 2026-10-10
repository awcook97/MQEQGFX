#pragma once

#include "LineFX.h"

#include <cstdint>

namespace eqgfx::gpu {

struct FxDef
{
	const char* name;
	uint32_t    form;
	float       radius;
	float       height;
	float       speed;
	float       tilt;
	float       width;
	uint32_t    segments;
	uint32_t    flags;
	float       p0, p1, p2, p3;
	float       rgba[4];
};

struct ClassFx
{
	const char*  shortName;
	const char*  fullName;
	const FxDef* defs;
	int          count;
	int          classId;
};

const ClassFx* FindClassFx(const char* shortName);
const ClassFx* FindClassFxById(int classId);

const ClassFx* AllClassFx(int& countOut);

int TotalFxCount();

void MakeInstance(const FxDef& def, const float pos[3], float phase,
	float alphaScale, FxInstance& out);

} // namespace eqgfx::gpu
