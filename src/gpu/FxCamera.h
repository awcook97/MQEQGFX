#pragma once

namespace eqlib { class CCamera; class CRender; }

namespace eqgfx::gpu {

bool BuildViewProj(float outViewProj[16], float outCamPos[3],
	float outCamRight[3], float outCamUp[3]);

} // namespace eqgfx::gpu
