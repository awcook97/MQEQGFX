/*
 * gpu/FxCamera.h - build the engine's exact view-projection for our shader.
 *
 * The GPU path needs a matrix that puts world points on screen in EXACTLY the
 * same place the engine does, or effects drift away from the things they are
 * attached to. Rather than guess a convention (handedness, Z range, FOV
 * derivation) we reconstruct the matrix from the client's own numbers.
 *
 * CRender::TransformWorldToCamera (EQGraphics 1800bf630) is:
 *
 *     p    = world + eyeOffset                     // CRender +0xbcc8/cc/d0
 *     out.x = dot(camera->worldToEyeXAxisCot,       p)
 *     out.y = dot(camera->worldToEyeYAxisCotAspect, p)
 *     out.z = dot((coef[0].z, coef[1].z, coef[2].z), p)
 *
 * The first two rows already have the projection scale (cot, cot/aspect) folded
 * in, and the third is the view depth - so those three dots ARE clip x, y and w.
 * All that is missing is clip z, which is the standard D3D mapping of view
 * depth into [0,1] using the camera's near and far planes.
 *
 * Everything here reads engine fields directly, so it stays correct across
 * patches as long as eqlib's CCamera layout does.
 */
#pragma once

namespace eqlib { class CCamera; class CRender; }

namespace eqgfx::gpu {

/// Fill `outViewProj` (row-major, for HLSL `mul(float4(p,1), m)` with the
/// matrix declared row_major) plus the camera basis the ribbon expansion needs.
/// Positions fed to the shader are MQ/TLO order (x, y, z-up); the engine's
/// (Y, X, Z) swap is folded into the matrix so callers never deal with it.
///
/// Returns false when the camera or render handler isn't available yet.
bool BuildViewProj(float outViewProj[16], float outCamPos[3],
	float outCamRight[3], float outCamUp[3]);

} // namespace eqgfx::gpu
