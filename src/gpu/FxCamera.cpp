/*
 * gpu/FxCamera.cpp - reconstruct the engine's clip transform. See FxCamera.h.
 */

#include "FxCamera.h"

#include <mq/Plugin.h>

#include "eqlib/graphics/CameraInterface.h"
#include "eqlib/graphics/Render.h"

#include <cmath>

namespace eqgfx::gpu {

namespace {

/// The engine's rows are indexed in its own (Y, X, Z) order. Our shader works
/// in MQ/TLO order (x, y, z), so swap the first two components once here rather
/// than making every caller remember.
struct Row3 { float x, y, z; };

inline Row3 ToMQOrder(const glm::vec3& v)
{
	return Row3{ v.y, v.x, v.z };
}

inline float Dot(const Row3& r, const float p[3])
{
	return r.x * p[0] + r.y * p[1] + r.z * p[2];
}

} // namespace

bool BuildViewProj(float outViewProj[16], float outCamPos[3],
	float outCamRight[3], float outCamUp[3])
{
	CDisplay* display = pDisplay.get();
	if (!display || !display->pCamera)
		return false;

	// pCamera is a CCameraInterface; the concrete CCamera carries the fields.
	eqlib::CCamera* cam = display->pCamera->AsCamera();
	if (!cam)
		return false;

	CRender* render = g_pDrawHandler.get();
	if (!render)
		return false;

	// eyeOffset is added to world positions before the transform. The engine
	// keeps it on CRender rather than the camera.
	CVector3 eye(0.0f, 0.0f, 0.0f);
	render->GetEyeOffset(eye);

	// Rows, converted to MQ order so the shader can feed raw TLO coordinates.
	const Row3 rx = ToMQOrder(cam->worldToEyeXAxisCot);
	const Row3 ry = ToMQOrder(cam->worldToEyeYAxisCotAspect);
	const Row3 rw = ToMQOrder(glm::vec3(cam->worldToEyeCoef[0].z,
		cam->worldToEyeCoef[1].z, cam->worldToEyeCoef[2].z));

	// The offset is in engine order; convert it the same way so the dots agree.
	const float offMQ[3] = { eye.Y, eye.X, eye.Z };
	const float dx = Dot(rx, offMQ);
	const float dy = Dot(ry, offMQ);
	const float dw = Dot(rw, offMQ);

	// clip.z maps view depth into [0,1]: z = w * far/(far-near) - near*far/(far-near)
	const float nearZ = cam->nearClip;
	const float farZ = (cam->farClip > nearZ + 1.0f) ? cam->farClip : (nearZ + 1.0f);
	const float A = farZ / (farZ - nearZ);
	const float B = nearZ * farZ / (farZ - nearZ);

	// Row-major, for HLSL row_major + mul(float4(p,1), m):
	//   result.j = sum_i p[i] * M[i][j]
	// so column j holds the row that produces clip component j.
	float* m = outViewProj;
	m[0]  = rx.x;  m[1]  = ry.x;  m[2]  = rw.x * A;      m[3]  = rw.x;
	m[4]  = rx.y;  m[5]  = ry.y;  m[6]  = rw.y * A;      m[7]  = rw.y;
	m[8]  = rx.z;  m[9]  = ry.z;  m[10] = rw.z * A;      m[11] = rw.z;
	m[12] = dx;    m[13] = dy;    m[14] = dw * A - B;    m[15] = dw;

	// Camera position in MQ order. `position` is engine order.
	outCamPos[0] = cam->position.y;
	outCamPos[1] = cam->position.x;
	outCamPos[2] = cam->position.z;

	// Basis for the ribbon fallback when a segment is degenerate. The X/Y rows
	// are the eye axes scaled by cot/aspect; normalising recovers the direction.
	auto norm = [](const Row3& r, float out[3]) {
		float len = std::sqrt(r.x * r.x + r.y * r.y + r.z * r.z);
		if (len < 1e-6f) { out[0] = 1.0f; out[1] = 0.0f; out[2] = 0.0f; return; }
		out[0] = r.x / len; out[1] = r.y / len; out[2] = r.z / len;
	};
	norm(rx, outCamRight);
	norm(ry, outCamUp);
	return true;
}

} // namespace eqgfx::gpu
