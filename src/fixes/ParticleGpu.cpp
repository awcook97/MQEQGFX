#include "../MQEQGFX.h"

#include "../PatternScan.h"
#include "../Signatures.h"

#include "eqlib/graphics/Actors.h"
#include "eqlib/graphics/Bones.h"
#include "eqlib/graphics/CameraInterface.h"
#include "eqlib/graphics/MathInterface.h"
#include "eqlib/graphics/ParticleSystemInterface.h"
#include "eqlib/graphics/Render.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace {

template <class T> void Rel(T*& p) { if (p) { p->Release(); p = nullptr; } }

struct GpuEmitter
{
	float    renderScale, alphaMul, axisAngle, nearClip;
	float    farClip, depthBias, sizeScale, maxAlpha;
	float    fadeIn, fadeOut, growIn, shrinkOut;
	float    heightShrink, widthShrink, spriteFps, dt;
	int32_t  colorStart[3];
	uint32_t spriteCells;
	int32_t  colorEnd[3];
	uint32_t renderMode;
	uint32_t flags, pad0, pad1, pad2;
};
static_assert(sizeof(GpuEmitter) == 112);
static_assert(sizeof(CParticle) == 128);

enum EmitterFlag : uint32_t
{
	Emitter_NegativeRadius = 1u << 0,
	Emitter_FlipV          = 1u << 1,
	Emitter_Fog            = 1u << 2,
	Emitter_ConstantColor  = 1u << 3,
	Emitter_ScreenAlign    = 1u << 4,
};

struct FrameCB
{
	float left, cx, right, top;
	float cy, bottom, focal, zA;
	float zB, vpX, vpY, vpW;
	float vpH, fogStart, fogRange, fogDensity;
	float camPos[3], pad0;
	float eyeX[3], pad1;
	float eyeY[3], pad2;
	float eyeW[3], pad3;
};

struct DrawCB
{
	uint32_t first, pad[3];
};

struct SlotDraw
{
	ID3D11ShaderResourceView* srv;
	uint32_t first, count;
	bool     additive, noZWrite;
	int      group;
};

struct Camera
{
	float left, cx, right, top, cy, bottom, focal, zA, zB, farClip;
	float pos[3];
	float ex[3], ey[3], ew[3];
};

struct GroupInfo
{
	SParticleGroup* group;
	float nearClip, farClip;
	bool  fog, flipV;
};

constexpr int kGateSentinel = 0x7EEDFACE;

const char* kShader = R"(
struct Particle
{
	float3 origin; float3 axis0; float3 axis1; float3 axis2;
	float angle; float radius; float height; float radialCos; float radialSin;
	float vel0; float vel1; float vel2; float velRadial; float velAngle; float velGravity;
	float lifetime; float remaining; float rotInit; float rotSpeed; float width; float heightSize;
	float3 drift;
};

struct Emitter
{
	float renderScale; float alphaMul; float axisAngle; float nearClip;
	float farClip; float depthBias; float sizeScale; float maxAlpha;
	float fadeIn; float fadeOut; float growIn; float shrinkOut;
	float heightShrink; float widthShrink; float spriteFps; float dt;
	int3 colorStart; uint spriteCells;
	int3 colorEnd; uint renderMode;
	uint flags; uint pad0; uint pad1; uint pad2;
};

cbuffer Frame : register(b0)
{
	float gLeft, gCx, gRight, gTop;
	float gCy, gBottom, gFocal, gZA;
	float gZB, gVpX, gVpY, gVpW;
	float gVpH, gFogStart, gFogRange, gFogDensity;
	float3 gCamPos; float gPad0;
	float3 gEyeX; float gPad1;
	float3 gEyeY; float gPad2;
	float3 gEyeW; float gPad3;
};

cbuffer Draw : register(b1)
{
	uint gFirst; uint3 gPadD;
};

StructuredBuffer<Particle> gParticles : register(t0);
StructuredBuffer<uint>     gEmitterOf : register(t1);
StructuredBuffer<Emitter>  gEmitters  : register(t2);
Buffer<float>              gTrig      : register(t3);
Texture2D                  gTex       : register(t4);
SamplerState               gSamp      : register(s0);

struct VSOut
{
	float4 pos : SV_Position;
	float4 col : COLOR0;
	float2 uv  : TEXCOORD0;
};

float CosT(float a) { return gTrig[(uint)((int)a & 511)]; }
float SinT(float a) { return gTrig[512 + (uint)((int)a & 511)]; }

float3 ToEye(float3 d) { return float3(dot(d, gEyeX), dot(d, gEyeY), dot(d, gEyeW)); }

float4 ToClip(float sx, float sy, float z, float w)
{
	float nx = (sx + 0.5 - gVpX) / gVpW * 2.0 - 1.0;
	float ny = 1.0 - (sy + 0.5 - gVpY) / gVpH * 2.0;
	return float4(nx * w, ny * w, z * w, w);
}

float Recip(float t) { return t > 0.0 ? 1.0 / t : 0.0; }

VSOut VSMain(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
	static const uint kCorner[6] = { 0, 1, 2, 0, 2, 3 };
	VSOut o;
	o.pos = float4(2.0, 2.0, 2.0, 1.0);
	o.col = 0;
	o.uv = 0;

	const uint index = gFirst + iid;
	const Particle p = gParticles[index];
	const Emitter e = gEmitters[gEmitterOf[index]];
	const uint corner = kCorner[vid];

	if ((e.flags & 1) == 0 && p.radius < 0.0)
		return o;

	const float rem = p.remaining + e.dt;
	const float age = p.lifetime - rem;

	const float rc = CosT(p.angle) * p.radius;
	const float rs = SinT(p.angle) * p.radius;
	float3 world = p.axis0 * p.height + p.origin + p.drift + p.axis1 * rc + p.axis2 * rs;
	const float streak = e.renderScale * p.heightSize;
	if (e.renderMode == 1)
		world -= p.axis0 * (streak * 0.5);

	const float3 v = ToEye(world - gCamPos);
	const float w = v.z - e.depthBias;
	if (w < e.nearClip || w > e.farClip)
		return o;

	float3 v1 = 0;
	float w1 = 1.0;
	if (e.renderMode == 1)
	{
		v1 = ToEye(world + p.axis0 * streak - gCamPos);
		w1 = v1.z - e.depthBias;
		if (w1 < e.nearClip || w1 > e.farClip)
			return o;
	}

	const float rhw = 1.0 / w;
	const float sx = gFocal * v.x * rhw + gCx;
	const float sy = gCy - gFocal * v.y * rhw;
	const float z = gZA - rhw * (gZA * gZB);

	float size = e.renderScale * e.sizeScale;
	if (age < e.growIn)
		size *= age * Recip(e.growIn);
	if (rem < e.shrinkOut)
		size *= rem * Recip(e.shrinkOut);
	const float hs = rem < e.heightShrink ? rem * Recip(e.heightShrink) : 1.0;
	const float ws = rem < e.widthShrink ? rem * Recip(e.widthShrink) : 1.0;
	size *= 0.5;

	float ct = 1.0, st = 0.0;
	if (p.rotSpeed != 0.0 || (e.flags & 16) != 0 || e.renderMode == 1 || e.renderMode == 2)
	{
		const float theta = (p.rotSpeed != 0.0 ? age * p.rotSpeed + p.rotInit : 0.0) + e.axisAngle;
		ct = CosT(theta);
		st = SinT(theta);
	}

	float outX, outY, outZ, outW;
	if (e.renderMode == 1)
	{
		const float rhw1 = 1.0 / w1;
		const float sx1 = gFocal * v1.x * rhw1 + gCx;
		const float sy1 = gCy - gFocal * v1.y * rhw1;
		const float z1 = gZA - rhw1 * (gZA * gZB);
		const float dx = sx1 - sx, dy = sy1 - sy;
		const float hw = size * p.width * gFocal * rhw * hs;
		const float len = sqrt(dx * dx + dy * dy) * ws;
		if (!(len >= 0.95 && hw >= 0.95 && gLeft <= sx + hw && sx - hw <= gRight && gTop <= len + sy && sy - len <= gBottom))
			return o;
		const float k = hw / len;
		const float off = ct * (k * dy) - st * (-k * dx);
		outW = w;
		if (corner == 0)      { outX = sx1 - off; outY = sy1; outZ = z1; }
		else if (corner == 1) { outX = sx1 + off; outY = sy1; outZ = z1; }
		else if (corner == 2) { outX = sx + off;  outY = sy;  outZ = z; }
		else                  { outX = sx - off;  outY = sy;  outZ = z; }
	}
	else
	{
		const float hA = size * p.heightSize * gFocal * rhw * hs;
		const float hB = size * p.width * gFocal * rhw * ws;
		if (!(hB >= 0.95 && hA >= 0.95 && gLeft <= hB + sx && sx - hB <= gRight && gTop <= hA + sy && sy - hA <= gBottom))
			return o;

		const float sa = (corner == 0 || corner == 1) ? 1.0 : -1.0;
		const float sb = (corner == 1 || corner == 2) ? 1.0 : -1.0;
		if (e.renderMode == 2)
		{
			const float3 w2 = float3(ct, st, 0.0) * (size * p.heightSize * hs);
			const float3 w1v = float3(st, -ct, 0.0) * (size * p.width * ws);
			const float3 vo = ToEye(w2 * sa + w1v * sb);
			const float wc = w + vo.z;
			const float rc2 = 1.0 / wc;
			outX = (v.x + vo.x) * gFocal * rc2 + gCx;
			outY = gCy - (v.y + vo.y) * gFocal * rc2;
			outZ = gZA - rc2 * (gZA * gZB);
			outW = wc;
		}
		else
		{
			const float2 a = float2(st * hA, -ct * hA);
			const float2 b = float2(ct * hB, st * hB);
			const float2 s = float2(sx, sy) + a * sa + b * sb;
			outX = s.x;
			outY = s.y;
			outZ = z;
			outW = w;
		}
	}

	float alpha = 1.0;
	if ((e.flags & 4) != 0 && gFogRange != 0.0)
	{
		float t = (w - gFogStart) / gFogRange;
		if (t >= 1.0)
			t = 1.0;
		float fog = t >= 0.0 ? t : 0.0;
		const float u = fog * gFogDensity;
		if (u < 0.2)
		{
			if (u > 0.0)
				fog = u * 5.0;
		}
		else
			fog = 1.0;
		alpha = 1.0 - fog;
	}
	if (age < e.fadeIn)
		alpha *= age * Recip(e.fadeIn);
	if (rem < e.fadeOut)
		alpha *= rem * Recip(e.fadeOut);
	if (!(alpha <= e.maxAlpha))
		alpha = e.maxAlpha;

	int3 rgb = e.colorStart;
	if ((e.flags & 8) == 0)
	{
		const float t = age / p.lifetime;
		rgb = (int3)((float3)(e.colorEnd - e.colorStart) * t) + e.colorStart;
	}
	const int ai = (int)(alpha * e.alphaMul * 255.0);
	const uint argb = ((((uint)ai << 8 | (uint)rgb.x) << 8 | (uint)rgb.y) << 8) | (uint)rgb.z;
	o.col = float4((argb >> 16) & 255, (argb >> 8) & 255, argb & 255, argb >> 24) / 255.0;

	float u0 = 0.0, u1 = 1.0, vt = 0.0, vb = 1.0;
	if (e.spriteCells >= 2)
	{
		const int cells = (int)e.spriteCells;
		const int frame = (int)(age * e.spriteFps) % cells;
		if (cells < 5)
		{
			u0 = (frame % 2) * 0.5; vt = (frame / 2) * 0.5; u1 = u0 + 0.5; vb = vt + 0.5;
		}
		else if (cells < 9)
		{
			u0 = (frame % 4) * 0.25; vt = (frame / 4) * 0.5; u1 = u0 + 0.25; vb = vt + 0.5;
		}
		else if (cells < 17)
		{
			u0 = (frame % 4) * 0.25; vt = (frame / 4) * 0.25; u1 = u0 + 0.25; vb = vt + 0.25;
		}
	}
	if ((e.flags & 2) != 0)
	{
		const float t = vt; vt = vb; vb = t;
	}
	o.uv = float2((corner == 1 || corner == 2) ? u1 : u0, (corner == 0 || corner == 1) ? vt : vb);
	o.pos = ToClip(outX, outY, outZ, outW);
	return o;
}

float4 PSMain(VSOut i) : SV_Target
{
	float4 c = gTex.Sample(gSamp, i.uv) * i.col;
	if (c.a < 1.0 / 255.0)
		discard;
	return c;
}
)";

std::atomic<bool> s_enabled{ false };
uintptr_t s_processFrameAddr = 0;
bool      s_scanned = false;
const char* s_failReason = "";

constexpr const char* kUnsupported = "not supported on this client version";
constexpr const char* kConflict    = "conflicts with another plugin";

void Unsupported(const char* detail)
{
	DebugSpewAlwaysFile("MQEQGFX: gpu particles: %s", detail);
	s_failReason = kUnsupported;
}

void Conflict(const char* detail)
{
	DebugSpewAlwaysFile("MQEQGFX: gpu particles: %s", detail);
	s_failReason = kConflict;
}

bool     s_frameValid = false;
uint32_t s_frameDtMs = 0;
Camera   s_cam = {};
float    s_fogStart = 0.0f, s_fogRange = 0.0f, s_fogDensity = 0.0f;

std::vector<CParticle>  s_particles;
std::vector<uint32_t>   s_emitterOf;
std::vector<GpuEmitter> s_emitters;
std::vector<SlotDraw>   s_draws;
bool s_needNativeFrame = false;

ID3D11Device*             s_device = nullptr;
ID3D11DeviceContext*      s_ctx = nullptr;
ID3D11VertexShader*       s_vs = nullptr;
ID3D11PixelShader*        s_ps = nullptr;
ID3D11Buffer*             s_frameCB = nullptr;
ID3D11Buffer*             s_drawCB = nullptr;
ID3D11Buffer*             s_particleBuf = nullptr;
ID3D11ShaderResourceView* s_particleSrv = nullptr;
ID3D11Buffer*             s_indexBuf = nullptr;
ID3D11ShaderResourceView* s_indexSrv = nullptr;
ID3D11Buffer*             s_emitterBuf = nullptr;
ID3D11ShaderResourceView* s_emitterSrv = nullptr;
ID3D11Buffer*             s_trigBuf = nullptr;
ID3D11ShaderResourceView* s_trigSrv = nullptr;
ID3D11BlendState*         s_blendAlpha = nullptr;
ID3D11BlendState*         s_blendAdd = nullptr;
ID3D11DepthStencilState*  s_depthRead = nullptr;
ID3D11DepthStencilState*  s_depthWrite = nullptr;
ID3D11RasterizerState*    s_raster = nullptr;
ID3D11SamplerState*       s_sampler = nullptr;
uint32_t s_particleCap = 0, s_emitterCap = 0;
constexpr uint32_t kMinCapacity = 1024;
constexpr uint64_t kShrinkAfterMs = 5000;
constexpr uint64_t kOverBudgetMs = 3000;
constexpr uint64_t kDrawFailedMs = 3000;
constexpr uint64_t kTextureRetryMs = 2000;
constexpr size_t   kMaxTextureRetries = 4096;
std::unordered_map<const CEmitterDefSlot*, uint64_t> s_textureRetryAt;
std::atomic<int>  s_budget{ kDefaultParticleBudget };
std::atomic<bool> s_trimRequested{ false };
uint64_t s_lowSinceMs = 0;
uint64_t s_overBudgetUntilMs = 0;
uint64_t s_drawFailedUntilMs = 0;
uint64_t s_lastUploadMs = 0;
bool     s_ready = false;
ID3D11Device* s_failedDevice = nullptr;

std::thread       s_compileThread;
std::atomic<bool> s_compileDone{ false };
bool              s_compileStarted = false;
std::string       s_vsCode, s_psCode, s_compileError;

CMath* MathTables()
{
	SGraphicsEngine* engine = pGraphicsEngine.get();
	return engine ? static_cast<CMath*>(engine->pMath) : nullptr;
}

bool TablesMatchEngine(CMath* math)
{
	for (int i = 0; i < 512; ++i)
	{
		const float a = static_cast<float>(i);
		if (math->FastCos(a) != math->cosTable[i] || math->FastSin(a) != math->sinTable[i])
			return false;
	}
	return true;
}

Camera BuildCamera(const CCamera* c)
{
	Camera cam;
	cam.left = c->left;
	cam.cx = c->halfRenderWidth + c->left;
	cam.right = c->halfRenderWidth + c->halfRenderWidth + c->left;
	cam.top = c->top;
	cam.cy = c->halfRenderHeight + c->top;
	cam.bottom = c->halfRenderHeight + c->halfRenderHeight + c->top;
	cam.focal = c->halfRenderWidth
		/ static_cast<float>(std::tan(static_cast<double>(c->halfViewAngle * 0.703125f * 0.017453292f)));
	cam.zA = c->nearClipRangeConst;
	cam.zB = c->nearClip;
	cam.farClip = c->farClip;
	cam.pos[0] = c->position.x;
	cam.pos[1] = c->position.y;
	cam.pos[2] = c->position.z;
	const glm::vec3* we = c->worldToEyeCoef;
	for (int i = 0; i < 3; ++i)
	{
		cam.ex[i] = we[i].x;
		cam.ey[i] = we[i].y;
		cam.ew[i] = we[i].z;
	}
	return cam;
}

float Dot(const float* a, const float* b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
float Length(const float* v) { return std::sqrt(Dot(v, v)); }

void GroupsOf(CParticleSystem* ps, const Camera& cam, float fogRange, GroupInfo out[4])
{
	const float nearFar = std::min(cam.farClip, 250.0f);
	out[0] = { &ps->spells, ps->nearClipSpells, nearFar, false, false };
	out[1] = { &ps->environment, ps->nearClipFogged, cam.farClip, fogRange > 0.0f, false };
	out[2] = { &ps->actors, ps->nearClipActors, nearFar, false, false };
	out[3] = { &ps->spellEmitters, ps->nearClipFogged, cam.farClip, fogRange > 0.0f, true };
}

ID3D11ShaderResourceView* SlotSrv(const CEmitterDefSlot* slot)
{
	auto* texture = static_cast<eqlib::Direct3DTexture9*>(slot->texture);
	return texture ? texture->GetShaderResourceView() : nullptr;
}

bool HasParticles(const CEmitterDefSlot* slot)
{
	for (const CEmitter* em = slot->emitters; em; em = em->pNext)
		if (em->head != em->tail)
			return true;
	return false;
}

bool AwaitingTexture(const CEmitterDefSlot* slot, uint64_t now)
{
	uint64_t& retryAt = s_textureRetryAt[slot];
	if (now < retryAt)
		return false;
	retryAt = now + kTextureRetryMs;
	return true;
}

bool AllTexturesReady(CParticleSystem* ps)
{
	if (s_textureRetryAt.size() > kMaxTextureRetries)
		s_textureRetryAt.clear();
	const uint64_t now = GetTickCount64();
	bool ready = true;
	SParticleGroup* groups[] = { &ps->spells, &ps->environment, &ps->actors, &ps->spellEmitters };
	for (SParticleGroup* g : groups)
	{
		for (int i = 0; i < g->count; ++i)
		{
			const CEmitterDefSlot* slot = g->slots ? g->slots[i] : nullptr;
			if (slot && slot->enabled && slot->def && HasParticles(slot) && !SlotSrv(slot) && AwaitingTexture(slot, now))
				ready = false;
		}
	}
	return ready;
}

struct EmitterScalars { float renderScale = 1.0f, actorAlpha = 1.0f; };

using PointFloatFn = float(*)(const CParticlePointInterface*);
PointFloatFn s_pointAlpha = nullptr;
PointFloatFn s_pointScale = nullptr;

float Length(const CVector4& row)
{
	return std::sqrt(row.X * row.X + row.Y * row.Y + row.Z * row.Z);
}

EmitterScalars ScalarsFor(const CEmitter* em, const SEmitterDefinition* def)
{
	EmitterScalars r;
	const bool scaleByActor = def->scaleByActor == 1;
	if (scaleByActor && !em->pActor && !em->pAttachObj)
		r.renderScale = em->scale;

	if (CActor* actor = em->pActor)
	{
		r.actorAlpha = actor->GetAlpha();
		if (scaleByActor)
			if (CActor* root = actor->GetRootActor())
				r.renderScale = root->GetScaleFactor() * actor->GetParticleScaleFactor();
	}

	if (!em->pBone)
	{
		if (!em->pActor || !em->pActor->IsParticleCloud())
		{
			if (CParticlePointInterface* point = em->pAttachObj)
			{
				r.actorAlpha = s_pointAlpha(point);
				if (scaleByActor)
					if (const CMatrix44* m = point->GetWorldMatrix())
						r.renderScale = s_pointScale(point) * Length(m->row[0]);
			}
		}
		else if (CActor* root = em->pActor->GetRootActor())
			r.actorAlpha = root->GetAlpha();
	}
	else
	{
		if (CActor* parent = em->pBone->GetParentActor())
			if (CActor* root = parent->GetRootActor())
				r.actorAlpha = root->GetAlpha();
		if (scaleByActor)
			r.renderScale = Length(em->pBone->m_attachmentMatrix.row[0]);
	}
	return r;
}

bool AttachRows(const CEmitter* em, float rows[3][3])
{
	const CMatrix44* m = nullptr;
	if (em->pAttachObj)
		m = em->pAttachObj->GetWorldMatrix();
	else if (em->pBone)
		m = &em->pBone->m_attachmentMatrix;
	else if (em->pActor)
		m = em->pActor->GetPositionMatrix();
	if (!m)
		return false;
	for (int r = 0; r < 3; ++r)
	{
		rows[r][0] = m->row[r].X;
		rows[r][1] = m->row[r].Y;
		rows[r][2] = m->row[r].Z;
	}
	return true;
}

float AxisAngle(const CEmitter* em, int mode, const Camera& cam)
{
	float rows[3][3] = {};
	if (!AttachRows(em, rows))
		return 0.0f;
	float dir[3];
	const int row = (mode == 1 || mode == 4) ? 0 : (mode == 2 || mode == 5) ? 1 : 2;
	const float sign = mode >= 4 && mode <= 6 ? -1.0f : 1.0f;
	for (int k = 0; k < 3; ++k)
		dir[k] = rows[row][k] * sign;
	const float dl = Length(dir);
	if (dl > 0.0f)
		for (float& d : dir)
			d /= dl;

	const float d0[3] = { em->position[0] - cam.pos[0], em->position[1] - cam.pos[1], em->position[2] - cam.pos[2] };
	const float d1[3] = { d0[0] + dir[0], d0[1] + dir[1], d0[2] + dir[2] };
	const float r0 = 1.0f / Dot(d0, cam.ew);
	const float r1 = 1.0f / Dot(d1, cam.ew);
	const float dx = (Dot(d1, cam.ex) * cam.focal * r1 + cam.cx) - (Dot(d0, cam.ex) * cam.focal * r0 + cam.cx);
	const float dy = (cam.cy - Dot(d1, cam.ey) * cam.focal * r1) - (cam.cy - Dot(d0, cam.ey) * cam.focal * r0);
	const float len = std::sqrt(dx * dx + dy * dy);
	float nx = 0.0f, ny = 0.0f;
	if (len <= -1.1920929e-07f || len >= 1.1920929e-07f)
	{
		nx = dx * (1.0f / len);
		ny = dy * (1.0f / len);
	}
	float angle = std::acos(ny);
	if (nx > 0.0f)
		angle = -angle;
	return (angle / 6.2831855f) * 512.0f;
}

bool EmitterVisible(const CEmitter* em, const SEmitterDefinition* def, const Camera& cam, float nearClip, float farClip)
{
	if (!(def->particleLifetime <= 2000.0f && def->particleLifetime != 2000.0f))
		return true;
	const float d[3] = { em->position[0] - cam.pos[0], em->position[1] - cam.pos[1], em->position[2] - cam.pos[2] };
	const float w = Dot(d, cam.ew);
	const float r = em->boundRadius;
	if (!(nearClip <= r + w + 10.0f) || !((w - 10.0f) - r <= farClip))
		return false;
	const float vx = Dot(d, cam.ex);
	const float sx = vx * cam.focal * (1.0f / w) + cam.cx;
	const float allowance = cam.cx * 0.35f;
	const float spread = std::fabs(((r + vx) * cam.focal * (1.0f / w) + cam.cx) - sx);
	return cam.left <= allowance + sx + spread && (sx - allowance) - spread <= cam.right;
}

void AppendParticles(const CParticle* first, const CParticle* last, uint32_t emitterIndex, size_t budget, bool& truncated)
{
	if (last <= first)
		return;
	size_t count = static_cast<size_t>(last - first);
	const size_t room = budget - s_particles.size();
	if (count > room)
	{
		count = room;
		truncated = true;
	}
	s_particles.insert(s_particles.end(), first, first + count);
	s_emitterOf.insert(s_emitterOf.end(), count, emitterIndex);
}

void CopyRing(const CEmitter* em, uint32_t emitterIndex, size_t budget, bool& truncated)
{
	if (em->head < em->tail)
	{
		AppendParticles(em->head, em->tail, emitterIndex, budget, truncated);
		return;
	}
	AppendParticles(em->head, em->ringLast + 1, emitterIndex, budget, truncated);
	AppendParticles(em->ringBegin, em->tail, emitterIndex, budget, truncated);
}

void BuildFrame(CParticleSystem* ps, int viewId, float dt, float fogRange)
{
	s_particles.clear();
	s_emitterOf.clear();
	s_emitters.clear();
	s_draws.clear();

	const size_t budget = static_cast<size_t>(s_budget.load(std::memory_order_relaxed));
	bool truncated = false;

	GroupInfo groups[4];
	GroupsOf(ps, s_cam, fogRange, groups);
	for (int gi = 0; gi < 4; ++gi)
	{
		const GroupInfo& g = groups[gi];
		for (int i = 0; i < g.group->count; ++i)
		{
			CEmitterDefSlot* slot = g.group->slots ? g.group->slots[i] : nullptr;
			if (!slot || !slot->enabled || !slot->def)
				continue;
			const SEmitterDefinition* def = slot->def;
			const uint32_t first = static_cast<uint32_t>(s_particles.size());

			for (const CEmitter* em = slot->emitters; em; em = em->pNext)
			{
				if (em->head == em->tail || em->viewId != viewId)
					continue;
				const float nearClip = em->ignoreNearClip ? 0.0f : g.nearClip;
				if (!EmitterVisible(em, def, s_cam, nearClip, g.farClip))
					continue;

				const EmitterScalars sc = ScalarsFor(em, def);
				GpuEmitter ge = {};
				ge.renderScale = sc.renderScale;
				ge.alphaMul = sc.actorAlpha * em->alpha;
				ge.axisAngle = def->screenAlignAxis != 0 ? AxisAngle(em, def->screenAlignAxis, s_cam) : 0.0f;
				ge.nearClip = nearClip;
				ge.farClip = g.farClip;
				ge.depthBias = def->depthBias;
				ge.sizeScale = def->sizeScale;
				ge.maxAlpha = def->maxAlpha;
				ge.fadeIn = def->alphaFadeIn;
				ge.fadeOut = def->alphaFadeOut;
				ge.growIn = def->sizeGrowIn;
				ge.shrinkOut = def->sizeShrinkOut;
				ge.heightShrink = def->heightShrinkOut;
				ge.widthShrink = def->widthShrinkOut;
				ge.spriteFps = def->spriteFps;
				ge.dt = dt;
				std::memcpy(ge.colorStart, def->colorStart, sizeof(ge.colorStart));
				std::memcpy(ge.colorEnd, def->colorEnd, sizeof(ge.colorEnd));
				ge.spriteCells = static_cast<uint32_t>(def->spriteCells);
				ge.renderMode = static_cast<uint32_t>(def->renderMode);
				ge.flags = (def->renderNegativeRadius == 1 ? Emitter_NegativeRadius : 0)
					| (g.flipV ? Emitter_FlipV : 0)
					| (g.fog ? Emitter_Fog : 0)
					| (std::memcmp(def->colorStart, def->colorEnd, sizeof(def->colorStart)) == 0 ? Emitter_ConstantColor : 0)
					| (def->screenAlignAxis != 0 ? Emitter_ScreenAlign : 0);

				const uint32_t index = static_cast<uint32_t>(s_emitters.size());
				s_emitters.push_back(ge);
				CopyRing(em, index, budget, truncated);
			}

			const uint32_t count = static_cast<uint32_t>(s_particles.size()) - first;
			if (!count)
				continue;
			ID3D11ShaderResourceView* srv = SlotSrv(slot);
			if (!srv)
			{
				if (AwaitingTexture(slot, GetTickCount64()))
					s_needNativeFrame = true;
				continue;
			}
			s_draws.push_back({ srv, first, count, def->additive == 1, def->noZWrite == 1, gi });
		}
	}
	if (truncated)
		s_overBudgetUntilMs = GetTickCount64() + kOverBudgetMs;
	s_frameValid = true;
}

class ParticleSystemHook
{
public:
	DETOUR_TRAMPOLINE_DEF_MEMBER(ParticleSystemHook, int, ProcessFrame_Trampoline, (CCamera*))
	int ProcessFrame_Detour(CCamera* cam)
	{
		CParticleSystem* ps = reinterpret_cast<CParticleSystem*>(this);
		s_frameValid = false;

		const char* reason = nullptr;
		CDisplay* display = pDisplay.get();
		const bool mainCamera = cam && display && static_cast<void*>(display->pCamera) == static_cast<void*>(cam);
		if (!s_enabled.load(std::memory_order_relaxed))
			reason = "off";
		else if (GetGameState() != GAMESTATE_INGAME)
			reason = "not in game";
		else if (!s_ready)
			reason = "device objects not ready";
		else if (!mainCamera)
			reason = "not the main camera";
		else if (s_needNativeFrame)
			reason = "loading textures";
		else if (GetTickCount64() < s_overBudgetUntilMs)
			reason = "over particle budget";
		else if (GetTickCount64() < s_drawFailedUntilMs)
			reason = "draw failed";
		else if (!AllTexturesReady(ps))
			reason = "loading textures";
		else if (!g_pDrawHandler)
			reason = "no renderer";

		if (reason)
		{
			if (mainCamera)
			{
				s_needNativeFrame = false;
				NoteParticleGpuFrame(false, reason);
			}
			return ProcessFrame_Trampoline(cam);
		}

		CRender* render = g_pDrawHandler.get();
		s_cam = BuildCamera(cam);
		s_fogStart = render->GetFogStart();
		s_fogRange = render->GetFogEnd() - s_fogStart;
		if (!render->bFogEnabled || s_fogRange <= 0.0f)
			s_fogRange = 0.0f;
		s_fogDensity = render->fFogDensity;

		const int viewId = ps->viewId;
		const uint32_t before = static_cast<uint32_t>(ps->lastTickMs);
		ps->viewId = kGateSentinel;
		const int result = ProcessFrame_Trampoline(cam);
		ps->viewId = viewId;
		const float dt = std::min(static_cast<float>(static_cast<uint32_t>(ps->lastTickMs) - before) * 0.001f, 1.0f);

		BuildFrame(ps, viewId, dt, s_fogRange);
		NoteParticleGpuFrame(true, "");
		return result;
	}
};

bool Compile(const char* entry, const char* target, std::string& code)
{
	ID3DBlob* blob = nullptr;
	ID3DBlob* err = nullptr;
	const bool ok = SUCCEEDED(D3DCompile(kShader, std::strlen(kShader), nullptr, nullptr, nullptr, entry, target,
		D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &blob, &err));
	if (ok)
		code.assign(static_cast<const char*>(blob->GetBufferPointer()), blob->GetBufferSize());
	else
		s_compileError = std::string(entry) + ": "
			+ (err ? std::string(static_cast<const char*>(err->GetBufferPointer())).substr(0, 200) : std::string("compile failed"));
	Rel(blob);
	Rel(err);
	return ok;
}

bool CreateStructured(uint32_t stride, uint32_t count, ID3D11Buffer*& buf, ID3D11ShaderResourceView*& srv)
{
	Rel(srv);
	Rel(buf);
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = stride * count;
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = stride;
	if (FAILED(s_device->CreateBuffer(&bd, nullptr, &buf)))
		return false;
	D3D11_SHADER_RESOURCE_VIEW_DESC sd = {};
	sd.Format = DXGI_FORMAT_UNKNOWN;
	sd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	sd.Buffer.NumElements = count;
	return SUCCEEDED(s_device->CreateShaderResourceView(buf, &sd, &srv));
}

bool CreateConstant(uint32_t bytes, ID3D11Buffer*& buf)
{
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = bytes;
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	return SUCCEEDED(s_device->CreateBuffer(&bd, nullptr, &buf));
}

bool CreateTrig(CMath* math)
{
	float table[1024];
	std::memcpy(table, math->cosTable, sizeof(math->cosTable));
	std::memcpy(table + 512, math->sinTable, sizeof(math->sinTable));
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(table);
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA init = { table, 0, 0 };
	if (FAILED(s_device->CreateBuffer(&bd, &init, &s_trigBuf)))
		return false;
	D3D11_SHADER_RESOURCE_VIEW_DESC sd = {};
	sd.Format = DXGI_FORMAT_R32_FLOAT;
	sd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	sd.Buffer.NumElements = 1024;
	return SUCCEEDED(s_device->CreateShaderResourceView(s_trigBuf, &sd, &s_trigSrv));
}

bool CreateStates()
{
	D3D11_BLEND_DESC bd = {};
	D3D11_RENDER_TARGET_BLEND_DESC& rt = bd.RenderTarget[0];
	rt.BlendEnable = TRUE;
	rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rt.BlendOp = D3D11_BLEND_OP_ADD;
	rt.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	rt.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (FAILED(s_device->CreateBlendState(&bd, &s_blendAlpha)))
		return false;
	rt.DestBlend = D3D11_BLEND_ONE;
	rt.DestBlendAlpha = D3D11_BLEND_ONE;
	if (FAILED(s_device->CreateBlendState(&bd, &s_blendAdd)))
		return false;

	D3D11_DEPTH_STENCIL_DESC dd = {};
	dd.DepthEnable = TRUE;
	dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	if (FAILED(s_device->CreateDepthStencilState(&dd, &s_depthRead)))
		return false;
	dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	if (FAILED(s_device->CreateDepthStencilState(&dd, &s_depthWrite)))
		return false;

	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE;
	rd.DepthClipEnable = TRUE;
	if (FAILED(s_device->CreateRasterizerState(&rd, &s_raster)))
		return false;

	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	return SUCCEEDED(s_device->CreateSamplerState(&sd, &s_sampler));
}

void ReleaseDevice()
{
	Rel(s_vs); Rel(s_ps); Rel(s_frameCB); Rel(s_drawCB);
	Rel(s_particleSrv); Rel(s_particleBuf); Rel(s_indexSrv); Rel(s_indexBuf);
	Rel(s_emitterSrv); Rel(s_emitterBuf); Rel(s_trigSrv); Rel(s_trigBuf);
	Rel(s_blendAlpha); Rel(s_blendAdd); Rel(s_depthRead); Rel(s_depthWrite);
	Rel(s_raster); Rel(s_sampler); Rel(s_ctx);
	s_device = nullptr;
	s_particleCap = s_emitterCap = 0;
	s_ready = false;
}

bool EnsureDevice()
{
	if (s_ready && s_device == gpD3D11Device)
		return true;
	if (!gbDeviceAcquired || !gpD3D11Device)
	{
		s_failReason = "no D3D11 device";
		return false;
	}
	if (s_failedDevice == gpD3D11Device)
		return false;
	if (s_device && s_device != gpD3D11Device)
		ReleaseDevice();
	if (gpD3D11Device->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
	{
		s_failedDevice = gpD3D11Device;
		s_failReason = "device feature level below 11_0";
		return false;
	}

	if (!s_compileStarted)
	{
		s_compileStarted = true;
		s_compileThread = std::thread([] {
			if (Compile("VSMain", "vs_5_0", s_vsCode) && Compile("PSMain", "ps_5_0", s_psCode))
				s_compileError.clear();
			s_compileDone = true;
		});
	}
	if (!s_compileDone)
	{
		s_failReason = "compiling shaders";
		return false;
	}
	if (s_compileThread.joinable())
		s_compileThread.join();
	if (!s_compileError.empty())
	{
		s_failReason = s_compileError.c_str();
		return false;
	}

	CMath* math = MathTables();
	if (!math)
	{
		s_failReason = "graphics engine not ready";
		return false;
	}
	if (!TablesMatchEngine(math))
	{
		s_failedDevice = gpD3D11Device;
		Unsupported("trig tables unavailable on this client");
		return false;
	}

	s_device = gpD3D11Device;
	s_device->GetImmediateContext(&s_ctx);
	const bool ok = s_ctx
		&& SUCCEEDED(s_device->CreateVertexShader(s_vsCode.data(), s_vsCode.size(), nullptr, &s_vs))
		&& SUCCEEDED(s_device->CreatePixelShader(s_psCode.data(), s_psCode.size(), nullptr, &s_ps))
		&& CreateConstant(sizeof(FrameCB), s_frameCB)
		&& CreateConstant(sizeof(DrawCB), s_drawCB)
		&& CreateTrig(math)
		&& CreateStates();
	if (!ok)
	{
		ReleaseDevice();
		s_failedDevice = gpD3D11Device;
		s_failReason = "device object creation failed";
		return false;
	}
	s_failReason = "";
	s_ready = true;
	return true;
}

uint32_t CapacityFor(uint32_t need)
{
	uint32_t n = kMinCapacity;
	while (n < need)
		n *= 2;
	return n;
}

void ReleaseBuffers()
{
	Rel(s_particleSrv); Rel(s_particleBuf); Rel(s_indexSrv); Rel(s_indexBuf);
	Rel(s_emitterSrv); Rel(s_emitterBuf);
	s_particleCap = s_emitterCap = 0;
	s_lowSinceMs = 0;
}

void ReleaseStaging()
{
	std::vector<CParticle>().swap(s_particles);
	std::vector<uint32_t>().swap(s_emitterOf);
	std::vector<GpuEmitter>().swap(s_emitters);
	std::vector<SlotDraw>().swap(s_draws);
}

void ReleaseIdleBuffers()
{
	if (s_trimRequested.exchange(false)
		|| ((s_particleCap || s_emitterCap) && GetTickCount64() - s_lastUploadMs >= kShrinkAfterMs))
	{
		ReleaseBuffers();
		ReleaseStaging();
	}
}

bool EnsureCapacity()
{
	if (s_trimRequested.exchange(false))
		ReleaseBuffers();
	s_lastUploadMs = GetTickCount64();

	const uint32_t particles = static_cast<uint32_t>(s_particles.size());
	const uint32_t emitters = static_cast<uint32_t>(s_emitters.size());
	const uint32_t wantParticles = CapacityFor(particles * 2);
	const uint32_t wantEmitters = CapacityFor(emitters * 2);

	const uint64_t now = GetTickCount64();
	const bool oversized = s_particleCap > wantParticles * 2 || s_emitterCap > wantEmitters * 2;
	if (!oversized)
		s_lowSinceMs = 0;
	else if (!s_lowSinceMs)
		s_lowSinceMs = now;
	const bool shrink = oversized && now - s_lowSinceMs >= kShrinkAfterMs;

	if (particles > s_particleCap || (shrink && s_particleCap > wantParticles * 2))
	{
		const uint32_t cap = particles > s_particleCap ? CapacityFor(particles) : wantParticles;
		if (!CreateStructured(sizeof(CParticle), cap, s_particleBuf, s_particleSrv)
			|| !CreateStructured(sizeof(uint32_t), cap, s_indexBuf, s_indexSrv))
		{
			s_particleCap = 0;
			return false;
		}
		s_particleCap = cap;
	}
	if (emitters > s_emitterCap || (shrink && s_emitterCap > wantEmitters * 2))
	{
		const uint32_t cap = emitters > s_emitterCap ? CapacityFor(emitters) : wantEmitters;
		if (!CreateStructured(sizeof(GpuEmitter), cap, s_emitterBuf, s_emitterSrv))
		{
			s_emitterCap = 0;
			return false;
		}
		s_emitterCap = cap;
	}
	if (shrink)
		s_lowSinceMs = 0;
	return true;
}

bool Upload(ID3D11Resource* res, const void* data, size_t bytes)
{
	D3D11_MAPPED_SUBRESOURCE map = {};
	if (FAILED(s_ctx->Map(res, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
		return false;
	std::memcpy(map.pData, data, bytes);
	s_ctx->Unmap(res, 0);
	return true;
}

void FallBackToNative()
{
	s_drawFailedUntilMs = GetTickCount64() + kDrawFailedMs;
	ReleaseBuffers();
	ReleaseStaging();
}

} // namespace

void InstallParticleGpu()
{
	if (s_processFrameAddr || s_scanned || !eqlib::EQGraphicsBaseAddress)
		return;
	s_scanned = true;
	const eqgfx::scan::Module gfx = eqgfx::scan::Graphics();
	if (!eqgfx::scan::Find(gfx, eqgfx::sig::CParticleSystem__DrawGate))
	{
		Unsupported("particle draw gate unavailable on this client");
		return;
	}
	s_pointAlpha = reinterpret_cast<PointFloatFn>(eqgfx::scan::Find(gfx, eqgfx::sig::CParticlePoint__GetAlpha));
	s_pointScale = reinterpret_cast<PointFloatFn>(eqgfx::scan::Find(gfx, eqgfx::sig::CParticlePoint__GetParticleScaleFactor));
	if (!s_pointAlpha || !s_pointScale)
	{
		Unsupported("particle point helpers unavailable on this client");
		return;
	}
	s_processFrameAddr = eqgfx::scan::Find(gfx, eqgfx::sig::CParticleSystem__ProcessFrame);
	if (!s_processFrameAddr)
	{
		Unsupported("particle processing unavailable on this client");
		return;
	}
	if (!EzDetour(s_processFrameAddr, &ParticleSystemHook::ProcessFrame_Detour, &ParticleSystemHook::ProcessFrame_Trampoline))
	{
		s_processFrameAddr = 0;
		Conflict("could not hook particle processing");
	}
}

void RemoveParticleGpu()
{
	if (s_processFrameAddr)
		RemoveDetour(s_processFrameAddr);
	s_processFrameAddr = 0;
	s_scanned = false;
	s_frameValid = false;
	ReleaseDevice();
	if (s_compileThread.joinable())
		s_compileThread.join();
}

void ReleaseParticleGpuDevice()
{
	s_frameValid = false;
	ReleaseDevice();
}

void DrawParticleGpu()
{
	if (!s_enabled.load(std::memory_order_relaxed) || !s_processFrameAddr)
	{
		s_failedDevice = nullptr;
		ReleaseIdleBuffers();
		return;
	}
	if (!EnsureDevice())
		return;
	if (!s_frameValid || s_draws.empty())
	{
		s_frameValid = false;
		ReleaseIdleBuffers();
		return;
	}
	s_frameValid = false;
	if (!EnsureCapacity()
		|| !Upload(s_particleBuf, s_particles.data(), s_particles.size() * sizeof(CParticle))
		|| !Upload(s_indexBuf, s_emitterOf.data(), s_emitterOf.size() * sizeof(uint32_t))
		|| !Upload(s_emitterBuf, s_emitters.data(), s_emitters.size() * sizeof(GpuEmitter)))
	{
		FallBackToNative();
		return;
	}

	D3D11_VIEWPORT vp = {};
	UINT vpCount = 1;
	s_ctx->RSGetViewports(&vpCount, &vp);
	if (!vpCount || vp.Width <= 0.0f || vp.Height <= 0.0f)
	{
		FallBackToNative();
		return;
	}

	FrameCB cb = {};
	cb.left = s_cam.left; cb.cx = s_cam.cx; cb.right = s_cam.right; cb.top = s_cam.top;
	cb.cy = s_cam.cy; cb.bottom = s_cam.bottom; cb.focal = s_cam.focal; cb.zA = s_cam.zA;
	cb.zB = s_cam.zB; cb.vpX = vp.TopLeftX; cb.vpY = vp.TopLeftY; cb.vpW = vp.Width;
	cb.vpH = vp.Height; cb.fogStart = s_fogStart; cb.fogRange = s_fogRange; cb.fogDensity = s_fogDensity;
	std::memcpy(cb.camPos, s_cam.pos, sizeof(cb.camPos));
	std::memcpy(cb.eyeX, s_cam.ex, sizeof(cb.eyeX));
	std::memcpy(cb.eyeY, s_cam.ey, sizeof(cb.eyeY));
	std::memcpy(cb.eyeW, s_cam.ew, sizeof(cb.eyeW));
	if (!Upload(s_frameCB, &cb, sizeof(cb)))
	{
		FallBackToNative();
		return;
	}

	ID3D11RenderTargetView* rtv = nullptr;
	ID3D11DepthStencilView* dsv = nullptr;
	s_ctx->OMGetRenderTargets(1, &rtv, &dsv);
	if (rtv)
		s_ctx->OMSetRenderTargets(1, &rtv, dsv);

	ID3D11Buffer* oldVsCb = nullptr;
	ID3D11ShaderResourceView* oldVsSrvs[4] = {};
	ID3D11ShaderResourceView* oldPsSrv = nullptr;
	s_ctx->VSGetConstantBuffers(1, 1, &oldVsCb);
	s_ctx->VSGetShaderResources(0, 4, oldVsSrvs);
	s_ctx->PSGetShaderResources(4, 1, &oldPsSrv);

	s_ctx->IASetInputLayout(nullptr);
	s_ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	s_ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	s_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	s_ctx->VSSetShader(s_vs, nullptr, 0);
	s_ctx->PSSetShader(s_ps, nullptr, 0);
	s_ctx->GSSetShader(nullptr, nullptr, 0);
	ID3D11Buffer* cbs[] = { s_frameCB, s_drawCB };
	s_ctx->VSSetConstantBuffers(0, 2, cbs);
	ID3D11ShaderResourceView* vsSrvs[] = { s_particleSrv, s_indexSrv, s_emitterSrv, s_trigSrv };
	s_ctx->VSSetShaderResources(0, 4, vsSrvs);
	s_ctx->PSSetSamplers(0, 1, &s_sampler);
	s_ctx->RSSetState(s_raster);

	const float blendFactor[4] = {};
	int group = -1;
	bool zwrite = false;
	bool drawFailed = false;
	for (const SlotDraw& d : s_draws)
	{
		if (d.group != group)
		{
			group = d.group;
			zwrite = false;
		}
		DrawCB dcb = { d.first, {} };
		if (!Upload(s_drawCB, &dcb, sizeof(dcb)))
		{
			drawFailed = true;
			break;
		}
		ID3D11ShaderResourceView* tex = d.srv;
		s_ctx->PSSetShaderResources(4, 1, &tex);
		s_ctx->OMSetBlendState(d.additive ? s_blendAdd : s_blendAlpha, blendFactor, 0xFFFFFFFF);
		s_ctx->OMSetDepthStencilState(zwrite && !d.noZWrite ? s_depthWrite : s_depthRead, 0);
		s_ctx->DrawInstanced(6, d.count, 0, 0);
		if (d.noZWrite)
			zwrite = true;
	}
	NoteParticleGpuDraw(static_cast<uint32_t>(s_particles.size()), static_cast<uint32_t>(s_emitters.size()),
		static_cast<uint32_t>(s_draws.size()));

	s_ctx->VSSetConstantBuffers(1, 1, &oldVsCb);
	s_ctx->VSSetShaderResources(0, 4, oldVsSrvs);
	s_ctx->PSSetShaderResources(4, 1, &oldPsSrv);
	Rel(oldVsCb);
	for (ID3D11ShaderResourceView*& srv : oldVsSrvs)
		Rel(srv);
	Rel(oldPsSrv);

	if (drawFailed)
		FallBackToNative();

	Rel(rtv);
	Rel(dsv);
}

bool ParticleGpuInstalled() { return s_processFrameAddr != 0; }
bool ParticleGpuEnabled() { return s_enabled.load(std::memory_order_relaxed); }
void SetParticleGpuEnabled(bool on) { s_enabled.store(on, std::memory_order_relaxed); }
const char* ParticleGpuFailReason() { return s_failReason; }
void TrimParticleGpu() { s_trimRequested.store(true, std::memory_order_relaxed); }
int GetParticleGpuBudget() { return s_budget.load(std::memory_order_relaxed); }
void SetParticleGpuBudget(int particles)
{
	s_budget.store(std::clamp(particles, kMinParticleBudget, kMaxParticleBudget), std::memory_order_relaxed);
}
size_t ParticleGpuBufferBytes()
{
	return static_cast<size_t>(s_particleCap) * (sizeof(CParticle) + sizeof(uint32_t))
		+ static_cast<size_t>(s_emitterCap) * sizeof(GpuEmitter);
}

bool ParticleGpuCommand(const char* szLine)
{
	char verb[MAX_STRING] = { 0 };
	GetArg(verb, szLine, 2);
	if (ci_equals(verb, "budget"))
	{
		char value[MAX_STRING] = { 0 };
		GetArg(value, szLine, 3);
		if (value[0])
			SetParticleGpuBudget(GetIntFromString(value, GetParticleGpuBudget()));
		WriteChatf("\ag[MQEQGFX]\ax gpu particle budget %d", GetParticleGpuBudget());
		return true;
	}
	if (ci_equals(verb, "on"))
		SetParticleGpuEnabled(true);
	else if (ci_equals(verb, "off"))
		SetParticleGpuEnabled(false);
	else if (verb[0])
	{
		WriteChatf("\ag[MQEQGFX]\ax usage: /eqgfx particles [on|off|budget <n>]");
		return true;
	}
	const ParticleGpuStats st = GetParticleGpuStats();
	WriteChatf("\ag[MQEQGFX]\ax gpu particles %s%s%s",
		ParticleGpuEnabled() ? "\agon\ax" : "\aroff\ax",
		*s_failReason ? " - " : "", s_failReason);
	WriteChatf("\ag[MQEQGFX]\ax last pulse: %u particles, %u emitters, %u draws, %u frames on the gpu%s%s",
		st.particles, st.emitters, st.draws, st.gatedFrames,
		*st.ungatedReason ? "; drawn by EQ: " : "", st.ungatedReason);
	WriteChatf("\ag[MQEQGFX]\ax budget %d particles, buffers %.1f MB",
		GetParticleGpuBudget(), static_cast<double>(ParticleGpuBufferBytes()) / (1024.0 * 1024.0));
	return true;
}
