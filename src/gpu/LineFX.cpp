/*
 * gpu/LineFX.cpp - the procedural line renderer. See LineFX.h for why.
 *
 * The whole design rests on one idea: a vertex shader can compute where a
 * vertex belongs from nothing but its own index. Given SV_VertexID and
 * SV_InstanceID we look up the effect record, work out which segment and which
 * corner of that segment's ribbon we are, evaluate the form's parametric
 * equation at that point, and expand to a camera-facing quad. The CPU never
 * touches a vertex.
 */

#include "LineFX.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#include <cstring>

namespace eqgfx::gpu {

namespace {

template <class T> void Rel(T*& p) { if (p) { p->Release(); p = nullptr; } }

/// Six vertices per segment (two triangles), so a full instance is
/// kMaxSegsPerInstance * 6 vertices. Unused segments collapse to a degenerate
/// point and cost a handful of clipped verts.
constexpr uint32_t kVertsPerSeg = 6;

struct CB
{
	float viewProj[16];
	float camPos[4];
	float camRight[4];
	float camUp[4];
	float timeSec;
	float viewportW;
	float viewportH;
	float pad;
};

const char* kShader = R"HLSL(
cbuffer CB : register(b0)
{
    row_major float4x4 gViewProj;
    float4 gCamPos;
    float4 gCamRight;
    float4 gCamUp;
    float  gTime;
    float  gViewW;
    float  gViewH;
    float  gPad;
};

struct FxInstance
{
    float3 pos;    float radius;
    float  height; float phase; float speed; float tilt;
    float4 rgba;
    uint   form;   uint segments; uint flags; float width;
    float  p0, p1, p2, p3;
    float3 pos2;   float pad;
};
StructuredBuffer<FxInstance> gInst : register(t0);

#define FORM_RING 0u
#define FORM_HELIX 1u
#define FORM_ORBIT 2u
#define FORM_POLY 3u
#define FORM_BLOOM 4u
#define FORM_SPIRE 5u
#define FORM_LATTICE 6u
#define FORM_SEGMENT 7u
#define FORM_RECT 8u

#define F_COMET   (1u<<0)
#define F_NODEPTH (1u<<1)
#define F_SCREENW (1u<<2)
#define F_ADD     (1u<<3)
#define F_REVERSE (1u<<4)
#define F_SCREEN  (1u<<5)

static const float TAU = 6.28318530718f;

// Rotate about world Z (up).
float3 spinZ(float3 v, float a)
{
    float s = sin(a), c = cos(a);
    return float3(v.x * c - v.y * s, v.x * s + v.y * c, v.z);
}

// Brightness for a point at t (0..1 around a loop) with a comet head at `head`.
// Cubed falloff: a tight bright head and a long dim tail. Same curve the Lua
// version used, so the look carries over.
float comet(float t, float head)
{
    float d = frac(t - head);
    d = min(d, 1.0f - d);
    float u = 1.0f - d * 2.0f;
    return 0.18f + 0.82f * u * u * u;
}

// Evaluate a form at parameter t in [0,1]. Returns the world position and
// writes a brightness multiplier. This is the whole animation catalogue: every
// per-class and per-element look is one of these with different parameters.
float3 evalForm(FxInstance ix, float t, float tm, out float gain)
{
    gain = 1.0f;
    float3 o = ix.pos;
    float r = ix.radius;
    uint  f = ix.form;

    if (f == FORM_SEGMENT)
    {
        gain = 1.0f;
        return lerp(ix.pos, ix.pos2, t);
    }
    else if (f == FORM_RING)
    {
        // p0 = ring index within a gyro stack, p1 = spin rate, p2 = radius scale
        float a = TAU * t;
        float3 u = spinZ(float3(1, 0, 0), tm * ix.p1);
        float3 v = spinZ(float3(0, cos(ix.tilt), sin(ix.tilt)), tm * ix.p1);
        float rr = r * (ix.p2 > 0.0f ? ix.p2 : 1.0f);
        if (ix.flags & F_COMET) gain = comet(t, frac(ix.phase + tm * ix.p3));
        return o + float3(0, 0, ix.height) + (u * cos(a) + v * sin(a)) * rr;
    }
    else if (f == FORM_HELIX)
    {
        float turns = max(ix.p0, 1.0f);
        float a = TAU * turns * t + tm * ix.p1 + ix.phase * TAU;
        if (ix.flags & F_COMET) gain = comet(t, frac(ix.phase + tm * ix.p3));
        return o + float3(cos(a) * r, sin(a) * r, ix.height * t);
    }
    else if (f == FORM_ORBIT)
    {
        // p0 = dash count. Each dash gets its own radius/height/rate from its
        // index, so they never line up into a visible ring.
        float n = max(ix.p0, 1.0f);
        float seg = floor(t * n);
        float local = frac(t * n);
        float h1 = frac(sin(seg * 12.9898f + ix.phase * 78.233f) * 43758.5453f);
        float h2 = frac(sin(seg * 39.3468f + ix.phase * 11.135f) * 24634.6345f);
        float rr = r * lerp(0.45f, 1.15f, h1);
        float hh = ix.height * lerp(0.15f, 1.0f, h2);
        float rate = lerp(0.6f, 1.8f, h1);
        float span = 0.55f;
        float a = TAU * frac(ix.phase + seg / n + tm * 0.13f * rate)
                + (local - 0.5f) * span;
        gain = 0.55f + 0.45f * (0.5f + 0.5f * sin(tm * 2.3f + seg * 2.1f));
        return o + float3(cos(a) * rr, sin(a) * rr,
                          hh + sin(tm * 1.7f + seg) * ix.height * 0.08f);
    }
    else if (f == FORM_POLY)
    {
        float sides = max(ix.p0, 3.0f);
        float a = TAU * (floor(t * sides) / sides) + tm * ix.p1 + ix.phase * TAU;
        float a2 = TAU * ((floor(t * sides) + 1.0f) / sides) + tm * ix.p1 + ix.phase * TAU;
        float k = frac(t * sides);
        float3 p1 = o + float3(cos(a) * r, sin(a) * r, ix.height);
        float3 p2 = o + float3(cos(a2) * r, sin(a2) * r, ix.height);
        return lerp(p1, p2, k);
    }
    else if (f == FORM_BLOOM)
    {
        // p0 = wave count. Waves are staggered so one is always mid-flight.
        float waves = max(ix.p0, 1.0f);
        float w = floor(t * waves);
        float local = frac(t * waves);
        float u = frac(tm * 0.5f * ix.speed + ix.phase + w / waves);
        float grow = 1.0f - (1.0f - u) * (1.0f - u);
        gain = (1.0f - u) * (1.0f - u);
        float a = TAU * local;
        float rr = r * (0.25f + grow * 1.35f);
        return o + float3(cos(a) * rr, sin(a) * rr, 0.15f + u * ix.height * 0.25f);
    }
    else if (f == FORM_SPIRE)
    {
        // p0 = ray count, p1 = spin. Each ray runs r..r*p2 outward.
        float n = max(ix.p0, 1.0f);
        float ray = floor(t * n);
        float local = frac(t * n);
        float a = TAU * ray / n + tm * ix.p1 + ix.phase * TAU;
        float r0 = r, r1 = r * (ix.p2 > 0.0f ? ix.p2 : 1.5f);
        float rr = lerp(r0, r1, local);
        gain = 1.0f - local * 0.35f;
        return o + float3(cos(a) * rr, sin(a) * rr, ix.height);
    }
    else // FORM_LATTICE
    {
        // Two rings joined by verticals: first half the rings, second the posts.
        float sides = max(ix.p0, 3.0f);
        if (t < 0.5f)
        {
            float tt = t * 2.0f;
            float ring = floor(tt * 2.0f);           // 0 = lower, 1 = upper
            float k = frac(tt * 2.0f);
            float a = TAU * k + tm * ix.p1 * (ring > 0.5f ? -1.0f : 1.0f) + ix.phase * TAU;
            float rr = r * (ring > 0.5f ? 0.7f : 1.0f);
            return o + float3(cos(a) * rr, sin(a) * rr, ring * ix.height);
        }
        float tt = (t - 0.5f) * 2.0f;
        float post = floor(tt * sides);
        float k = frac(tt * sides);
        float aL = TAU * post / sides + tm * ix.p1 + ix.phase * TAU;
        float aU = TAU * post / sides - tm * ix.p1 + ix.phase * TAU;
        float3 lo = o + float3(cos(aL) * r, sin(aL) * r, 0);
        float3 hi = o + float3(cos(aU) * r * 0.7f, sin(aU) * r * 0.7f, ix.height);
        gain = 0.55f;
        return lerp(lo, hi, k);
    }
}

struct VSOut { float4 pos : SV_POSITION; float4 col : COLOR0; };

// Pixels -> clip. Screen-space work bypasses the view-projection entirely so
// overlays land exactly where the caller asked, at any resolution.
float4 screenToClip(float2 px)
{
    return float4(px.x / max(gViewW, 1.0f) * 2.0f - 1.0f,
                  1.0f - px.y / max(gViewH, 1.0f) * 2.0f,
                  0.0f, 1.0f);
}

VSOut VSMain(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
    VSOut o;
    FxInstance ix = gInst[iid];

    uint seg    = vid / 6u;
    uint corner = vid % 6u;

    // Two triangles over the quad (A-, A+, B-, B+):
    //   tri1 = A- A+ B-   (corners 0,1,2)
    //   tri2 = B- A+ B+   (corners 3,4,5)
    bool atB  = (corner == 2u || corner == 3u || corner == 5u);
    bool plus = (corner == 1u || corner == 4u || corner == 5u);

    // A filled rect is a single quad, not a swept ribbon.
    if (ix.form == FORM_RECT)
    {
        if (seg >= 1u) { o.pos = float4(0,0,0,0); o.col = 0; return o; }
        float2 p = float2(atB  ? ix.pos2.x : ix.pos.x,
                          plus ? ix.pos2.y : ix.pos.y);
        o.pos = (ix.flags & F_SCREEN) ? screenToClip(p)
                                      : mul(float4(p, ix.pos.z, 1.0f), gViewProj);
        o.col = ix.rgba;
        return o;
    }

    uint segCount = max(ix.segments, 1u);
    if (seg >= segCount)                    // collapse the unused tail
    {
        o.pos = float4(0, 0, 0, 0);
        o.col = 0;
        return o;
    }

    float tm = gTime * ix.speed * ((ix.flags & F_REVERSE) ? -1.0f : 1.0f);
    float t0 = float(seg)      / float(segCount);
    float t1 = float(seg + 1u) / float(segCount);

    // Screen-space segments: expand the ribbon in 2D pixels. No projection, no
    // camera basis - the perpendicular is just the 2D normal of the segment.
    if (ix.flags & F_SCREEN)
    {
        float2 a2 = lerp(ix.pos.xy,  ix.pos2.xy, t0);
        float2 b2 = lerp(ix.pos.xy,  ix.pos2.xy, t1);
        float2 d2 = b2 - a2;
        float  l2 = length(d2);
        float2 n2 = (l2 > 1e-5f) ? float2(-d2.y, d2.x) / l2 : float2(0.0f, 1.0f);
        float2 p  = (atB ? b2 : a2) + n2 * (plus ? ix.width : -ix.width);
        o.pos = screenToClip(p);
        o.col = ix.rgba;
        return o;
    }

    float g0, g1;
    float3 a = evalForm(ix, t0, tm, g0);
    float3 b = evalForm(ix, t1, tm, g1);

    // Ribbon: expand across the segment using the camera-facing perpendicular,
    // so width is a real thickness instead of N stacked 1px lines.
    float3 dir = b - a;
    float  len = length(dir);
    float3 view = normalize(gCamPos.xyz - (a + b) * 0.5f);
    float3 side = (len > 1e-5f) ? normalize(cross(normalize(dir), view))
                                : gCamRight.xyz;

    float w = ix.width;
    if (ix.flags & F_SCREENW)
    {
        // Keep a constant pixel width: scale by distance / viewport height.
        float dist = max(length(gCamPos.xyz - a), 0.001f);
        w = ix.width * dist / max(gViewH, 1.0f);
    }

    float3 base = atB ? b : a;
    float  gain = atB ? g1 : g0;
    float3 world = base + side * (plus ? w : -w);

    o.pos = mul(float4(world, 1.0f), gViewProj);
    o.col = float4(ix.rgba.rgb, ix.rgba.a * gain);
    return o;
}

float4 PSMain(VSOut i) : SV_TARGET
{
    return float4(i.col.rgb * i.col.a, i.col.a);   // premultiplied
}
)HLSL";

} // namespace

LineFX::~LineFX() { ReleaseAll(); }

void LineFX::ReleaseAll()
{
	Rel(m_vs); Rel(m_ps); Rel(m_cb);
	Rel(m_instSrv); Rel(m_instBuf);
	Rel(m_blendAlpha); Rel(m_blendAdd);
	Rel(m_dssTest); Rel(m_dssNone); Rel(m_raster);
	Rel(m_ctx);
	m_device = nullptr;
	m_ready = false;
}

bool LineFX::CreateShaders()
{
	ID3DBlob* vsb = nullptr;
	ID3DBlob* psb = nullptr;
	ID3DBlob* err = nullptr;

	// Structured buffers need at least shader model 5.
	if (FAILED(D3DCompile(kShader, std::strlen(kShader), nullptr, nullptr, nullptr,
		"VSMain", "vs_5_0", 0, 0, &vsb, &err)))
	{
		m_error = "vertex shader compile failed";
		Rel(err); Rel(vsb);
		return false;
	}
	if (FAILED(D3DCompile(kShader, std::strlen(kShader), nullptr, nullptr, nullptr,
		"PSMain", "ps_5_0", 0, 0, &psb, &err)))
	{
		m_error = "pixel shader compile failed";
		Rel(err); Rel(vsb); Rel(psb);
		return false;
	}

	bool ok = SUCCEEDED(m_device->CreateVertexShader(
			vsb->GetBufferPointer(), vsb->GetBufferSize(), nullptr, &m_vs))
		&& SUCCEEDED(m_device->CreatePixelShader(
			psb->GetBufferPointer(), psb->GetBufferSize(), nullptr, &m_ps));

	Rel(vsb); Rel(psb); Rel(err);
	if (!ok) m_error = "shader object creation failed";
	return ok;
}

bool LineFX::CreateBuffers()
{
	D3D11_BUFFER_DESC cbd = {};
	cbd.ByteWidth = sizeof(CB);
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(m_device->CreateBuffer(&cbd, nullptr, &m_cb)))
	{
		m_error = "constant buffer creation failed";
		return false;
	}

	D3D11_BUFFER_DESC ibd = {};
	ibd.ByteWidth = sizeof(FxInstance) * kMaxInstances;
	ibd.Usage = D3D11_USAGE_DYNAMIC;
	ibd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ibd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	ibd.StructureByteStride = sizeof(FxInstance);
	if (FAILED(m_device->CreateBuffer(&ibd, nullptr, &m_instBuf)))
	{
		m_error = "instance buffer creation failed";
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srv = {};
	srv.Format = DXGI_FORMAT_UNKNOWN;
	srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv.Buffer.FirstElement = 0;
	srv.Buffer.NumElements = kMaxInstances;
	if (FAILED(m_device->CreateShaderResourceView(m_instBuf, &srv, &m_instSrv)))
	{
		m_error = "instance SRV creation failed";
		return false;
	}
	return true;
}

bool LineFX::CreateStates()
{
	// Premultiplied alpha, and an additive variant for glow-style effects.
	D3D11_BLEND_DESC bd = {};
	bd.RenderTarget[0].BlendEnable = TRUE;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (FAILED(m_device->CreateBlendState(&bd, &m_blendAlpha)))
	{
		m_error = "blend state creation failed";
		return false;
	}
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	if (FAILED(m_device->CreateBlendState(&bd, &m_blendAdd)))
	{
		m_error = "additive blend state creation failed";
		return false;
	}

	// Test against the world's depth but never write: effects occlude correctly
	// without punching holes in the depth buffer for anything drawn after us.
	D3D11_DEPTH_STENCIL_DESC dd = {};
	dd.DepthEnable = TRUE;
	dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	if (FAILED(m_device->CreateDepthStencilState(&dd, &m_dssTest)))
	{
		m_error = "depth state creation failed";
		return false;
	}
	dd.DepthEnable = FALSE;
	if (FAILED(m_device->CreateDepthStencilState(&dd, &m_dssNone)))
	{
		m_error = "no-depth state creation failed";
		return false;
	}

	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE;      // ribbons are two-sided
	rd.DepthClipEnable = TRUE;
	if (FAILED(m_device->CreateRasterizerState(&rd, &m_raster)))
	{
		m_error = "rasterizer state creation failed";
		return false;
	}
	return true;
}

bool LineFX::EnsureInit(ID3D11Device* device)
{
	if (m_ready && m_device == device)
		return true;
	if (!device)
	{
		m_error = "no d3d11 device";
		return false;
	}
	if (m_device && m_device != device)
		ReleaseAll();          // device swapped under us

	m_device = device;
	m_device->GetImmediateContext(&m_ctx);
	if (!m_ctx)
	{
		m_error = "no immediate context";
		return false;
	}

	// Structured buffers require feature level 11. Fail loudly rather than
	// producing nothing at runtime with no explanation.
	if (m_device->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
	{
		m_error = "device feature level below 11_0";
		return false;
	}

	if (!CreateShaders() || !CreateBuffers() || !CreateStates())
		return false;

	m_error = "";
	m_ready = true;
	return true;
}

void LineFX::Render(const FxInstance* instances, size_t count,
	const float viewProj[16], const float camPos[3],
	const float camRight[3], const float camUp[3],
	float timeSec, float viewportW, float viewportH)
{
	m_lastSegments = 0;
	m_lastInstances = 0;
	if (!m_ready || !instances || count == 0)
		return;
	if (count > kMaxInstances)
		count = kMaxInstances;

	// Partition into draw buckets: bit0 = additive, bit1 = no depth. Screen-space
	// always implies no depth, so overlays sit on top of the world pass.
	m_sorted.clear();
	m_sorted.reserve(count);
	UINT bucketCount[4] = { 0, 0, 0, 0 };
	for (int b = 0; b < 4; ++b)
	{
		for (size_t i = 0; i < count; ++i)
		{
			const FxInstance& ix = instances[i];
			const bool additive = (ix.flags & FxFlag_Additive) != 0;
			const bool noDepth  = (ix.flags & (FxFlag_NoDepth | FxFlag_ScreenSpace)) != 0;
			const int bucket = (additive ? 1 : 0) | (noDepth ? 2 : 0);
			if (bucket != b)
				continue;
			m_sorted.push_back(ix);
			++bucketCount[b];
		}
	}

	// Upload the instance records.
	D3D11_MAPPED_SUBRESOURCE map = {};
	if (FAILED(m_ctx->Map(m_instBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
		return;
	std::memcpy(map.pData, m_sorted.data(), sizeof(FxInstance) * m_sorted.size());
	m_ctx->Unmap(m_instBuf, 0);

	CB cb = {};
	std::memcpy(cb.viewProj, viewProj, sizeof(cb.viewProj));
	cb.camPos[0] = camPos[0];   cb.camPos[1] = camPos[1];   cb.camPos[2] = camPos[2];
	cb.camRight[0] = camRight[0]; cb.camRight[1] = camRight[1]; cb.camRight[2] = camRight[2];
	cb.camUp[0] = camUp[0];     cb.camUp[1] = camUp[1];     cb.camUp[2] = camUp[2];
	cb.timeSec = timeSec;
	cb.viewportW = viewportW;
	cb.viewportH = viewportH;

	if (FAILED(m_ctx->Map(m_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
		return;
	std::memcpy(map.pData, &cb, sizeof(cb));
	m_ctx->Unmap(m_cb, 0);

	// Re-bind the game's current targets. MQ's ScopedStateBlock captured them on
	// the way in and will restore them on the way out, so borrowing them here is
	// safe; taking the DSV is what buys correct occlusion against the world.
	ID3D11RenderTargetView* rtv = nullptr;
	ID3D11DepthStencilView* dsv = nullptr;
	m_ctx->OMGetRenderTargets(1, &rtv, &dsv);
	if (rtv)
		m_ctx->OMSetRenderTargets(1, &rtv, dsv);

	const float blendFactor[4] = { 0, 0, 0, 0 };
	m_ctx->IASetInputLayout(nullptr);
	m_ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	m_ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	m_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ctx->VSSetShader(m_vs, nullptr, 0);
	m_ctx->PSSetShader(m_ps, nullptr, 0);
	m_ctx->GSSetShader(nullptr, nullptr, 0);
	m_ctx->VSSetConstantBuffers(0, 1, &m_cb);
	m_ctx->VSSetShaderResources(0, 1, &m_instSrv);
	m_ctx->RSSetState(m_raster);
	// One draw per (depth, blend) bucket. Doing this globally would mean a
	// single additive effect turns every other effect additive, and a single
	// screen-space overlay disables depth for the whole world pass.
	// m_sorted is already partitioned in bucket order, so each run is contiguous
	// and addressable with StartInstanceLocation.
	UINT start = 0;
	for (int b = 0; b < 4; ++b)
	{
		UINT n = bucketCount[b];
		if (n == 0)
			continue;
		const bool additive = (b & 1) != 0;
		const bool noDepth  = (b & 2) != 0;
		m_ctx->OMSetBlendState(additive ? m_blendAdd : m_blendAlpha, blendFactor, 0xFFFFFFFF);
		m_ctx->OMSetDepthStencilState(noDepth ? m_dssNone : m_dssTest, 0);
		m_ctx->DrawInstanced(kMaxSegsPerInstance * kVertsPerSeg, n, 0, start);
		start += n;
	}

	for (size_t i = 0; i < count; ++i)
	{
		uint32_t s = instances[i].segments;
		m_lastSegments += (s > kMaxSegsPerInstance) ? kMaxSegsPerInstance : s;
	}
	m_lastInstances = static_cast<uint32_t>(count);

	Rel(rtv);
	Rel(dsv);
}

} // namespace eqgfx::gpu
