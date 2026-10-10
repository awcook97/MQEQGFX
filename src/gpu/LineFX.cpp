#include "LineFX.h"
#include "Glyphs.h"

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

namespace eqgfx::gpu {

namespace {

template <class T> void Rel(T*& p) { if (p) { p->Release(); p = nullptr; } }

constexpr uint32_t kVertsPerSeg = 6;
constexpr double   kTimeWrap = 16384.0;

struct CB
{
	float viewProj[16];
	float camPos[4];
	float camRight[4];
	float camUp[4];
	float timeSec;
	float viewportW;
	float viewportH;
	uint32_t baseInstance;
	float viewportX;
	float viewportY;
	float pad[2];
};

int BucketOf(const FxInstance& ix)
{
	const bool additive = (ix.flags & FxFlag_Additive) != 0;
	const bool noDepth  = (ix.flags & (FxFlag_NoDepth | FxFlag_ScreenSpace)) != 0;
	return (additive ? 1 : 0) | (noDepth ? 2 : 0);
}

uint32_t SegsDrawn(const FxInstance& ix)
{
	if (ix.form == FxForm_Rect)
		return 1;
	return std::clamp(ix.segments, 1u, kMaxSegsPerInstance);
}

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
    uint   gBase;
    float  gViewX;
    float  gViewY;
};

struct FxInstance
{
    float3 pos;    float radius;
    float  height; float phase; float speed; float tilt;
    float4 rgba;
    uint   form;   uint segments; uint flags; float width;
    float  p0, p1, p2, p3;
    float3 pos2;   float yaw;
    float4 rgba2;
    float  birth;  float life; float p4; float p5;
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
#define FORM_FLAME 9u
#define FORM_VINES 10u
#define FORM_THORNS 11u
#define FORM_GLYPH 12u
#define FORM_WINGS 13u
#define FORM_LIGHTNING 14u
#define FORM_BOLT 15u
#define FORM_RAIN 16u
#define FORM_SPARKLE 17u
#define FORM_RUNE 18u
#define FORM_VORTEX 19u
#define FORM_DOME 20u
#define FORM_CRYSTAL 21u
#define FORM_DRAIN 22u
#define FORM_BUBBLES 23u
#define FORM_WIND 24u
#define FORM_BURST 25u
#define FORM_CRACKS 26u

#define F_COMET   (1u<<0)
#define F_NODEPTH (1u<<1)
#define F_SCREENW (1u<<2)
#define F_ADD     (1u<<3)
#define F_REVERSE (1u<<4)
#define F_SCREEN  (1u<<5)
#define F_NOENV   (1u<<6)
#define F_DRIFT   (1u<<7)

#define GA_RISE   (1u<<0)
#define GA_BOB    (1u<<1)
#define GA_SPIN   (1u<<2)
#define GA_POP    (1u<<3)
#define GA_WIGGLE (1u<<4)
#define GA_ASCEND (1u<<5)
#define GA_DESCEND (1u<<6)
#define GA_ROLL   (1u<<7)

static const float TAU = 6.28318530718f;
static const float PI  = 3.14159265359f;
static const float3 UP = float3(0, 0, 1);

float hash2(float a, float b)
{
    return frac(sin(a * 12.9898f + b * 78.233f) * 43758.5453f);
}

float easeOut(float x)
{
    x = saturate(x);
    return 1.0f - (1.0f - x) * (1.0f - x) * (1.0f - x);
}

float easeOutBack(float x)
{
    x = saturate(x);
    const float c = 1.70158f;
    float y = x - 1.0f;
    return 1.0f + (c + 1.0f) * y * y * y + c * y * y;
}

struct FormOut
{
    float3 p;
    float  gain;
    float  grad;
    float  wmul;
};

FormOut mk(float3 p, float gain, float grad, float wmul)
{
    FormOut r; r.p = p; r.gain = gain; r.grad = grad; r.wmul = wmul; return r;
}

FormOut dead() { return mk(float3(0, 0, 0), 0.0f, 0.0f, -1.0f); }

void itemOf(uint seg, uint n, uint K, bool atB, out uint i, out float s)
{
    uint spi = max(n / max(K, 1u), 1u);
    i = seg / spi;
    s = float(seg % spi + (atB ? 1u : 0u)) / float(spi);
}

float3 fwdOf(float yaw)   { return float3(cos(yaw), sin(yaw), 0); }
float3 rightOf(float yaw) { return float3(sin(yaw), -cos(yaw), 0); }

void basisOf(float3 dir, out float3 e1, out float3 e2)
{
    float3 ref = abs(dir.z) < 0.9f ? UP : float3(1, 0, 0);
    e1 = normalize(cross(dir, ref));
    e2 = cross(dir, e1);
}

float3 spinZ(float3 v, float a)
{
    float s = sin(a), c = cos(a);
    return float3(v.x * c - v.y * s, v.x * s + v.y * c, v.z);
}

float comet(float t, float head)
{
    float d = frac(t - head);
    d = min(d, 1.0f - d);
    float u = 1.0f - d * 2.0f;
    return 0.18f + 0.82f * u * u * u;
}

float3 evalLegacy(FxInstance ix, float t, float tm, out float gain)
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
        float n = max(ix.p0, 1.0f);
        float ray = floor(t * n);
        float local = frac(t * n);
        float a = TAU * ray / n + tm * ix.p1 + ix.phase * TAU;
        float r0 = r, r1 = r * (ix.p2 > 0.0f ? ix.p2 : 1.5f);
        float rr = lerp(r0, r1, local);
        gain = 1.0f - local * 0.35f;
        return o + float3(cos(a) * rr, sin(a) * rr, ix.height);
    }
    else
    {
        float sides = max(ix.p0, 3.0f);
        if (t < 0.5f)
        {
            float tt = t * 2.0f;
            float ring = floor(tt * 2.0f);
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

float3 vinePos(FxInstance ix, uint i, float v, float K)
{
    float a0  = TAU * float(i) / K + hash2(float(i), 3.3f + ix.phase) * 0.5f;
    float dir = (i % 2u == 0u) ? 1.0f : -1.0f;
    float turns = ix.p1 > 0.0f ? ix.p1 : 1.0f;
    float hTop = ix.height * (0.75f + 0.25f * hash2(float(i), 7.7f));
    float rr, z;
    if (ix.p3 > 0.5f)
    {
        rr = lerp(ix.radius * 1.9f, ix.radius * 0.55f, v);
        z  = hTop * sin(v * PI * 0.5f);
    }
    else
    {
        rr = ix.radius * (1.05f - 0.2f * v);
        z  = hTop * v;
    }
    float tm = gTime * ix.speed;
    rr += 0.1f * ix.radius * sin(v * 19.0f + float(i) * 2.3f + tm * 1.3f);
    float a = a0 + dir * turns * TAU * v + 0.12f * v * sin(tm * 0.9f + float(i));
    return ix.pos + float3(cos(a) * rr, sin(a) * rr, z + 0.05f);
}

float2 wingBone(float t)
{
    float2 p1 = float2(0.45f, 0.45f), p2 = float2(1.0f, 0.12f);
    float k = 1.0f - t;
    return 2.0f * k * t * p1 + t * t * p2;
}

FormOut evalSpell(FxInstance ix, uint seg, uint n, bool atB, float tm, float u)
{
    float3 o = ix.pos;
    float  r = ix.radius;
    float  H = ix.height;
    uint   f = ix.form;
    uint   i = 0u; float s = 0.0f;

    if (f == FORM_FLAME)
    {
        float K = max(ix.p0, 1.0f);
        itemOf(seg, n, uint(K), atB, i, s);
        if (float(i) >= K) return dead();
        float fi = float(i);
        float hA = hash2(fi, ix.phase * 91.0f + 1.0f), hB = hash2(fi, 2.7f);
        float hC = hash2(fi, 5.3f), hD = hash2(fi, 8.1f);
        float spread = ix.p1 > 0.0f ? ix.p1 : 0.9f;
        float lenS = ix.p2 > 0.0f ? ix.p2 : 1.0f;
        float cyc = frac(tm * (1.1f + hB * 1.3f) + hA);
        float z01 = hC * spread;
        float zb  = H * z01 + cyc * H * 0.12f;
        float a   = TAU * hD + tm * 0.35f * (hB - 0.5f);
        float prof = (z01 > 0.82f) ? 0.55f : (0.72f + 0.28f * sin(PI * saturate(z01 / 0.82f)));
        float3 nrm = float3(cos(a), sin(a), 0), tang = float3(-sin(a), cos(a), 0);
        float L = H * (0.10f + 0.16f * hB) * lenS * (1.0f - 0.6f * cyc) * smoothstep(0.0f, 0.12f, u);
        float sway = sin(s * 4.0f + tm * 9.0f + fi * 1.7f) * 0.22f * r * s;
        float3 p = o + nrm * (r * prof + 0.12f * r * s) + tang * sway + UP * (zb + L * s);
        return mk(p, pow(1.0f - cyc, 0.6f) * (0.65f + 0.35f * (1.0f - s)), s,
                  (1.0f - s) * min(s * 4.0f + 0.25f, 1.0f) * 1.6f);
    }
    if (f == FORM_VINES)
    {
        float K = max(ix.p0, 1.0f);
        itemOf(seg, n, uint(K), atB, i, s);
        float g = smoothstep(0.0f, 0.45f, u);
        if (float(i) >= K || g <= 0.001f) return dead();
        float v = s * g;
        return mk(vinePos(ix, i, v, K), 1.0f, v, 1.0f - 0.55f * v);
    }
    if (f == FORM_THORNS)
    {
        float K = max(ix.p0, 1.0f), M = max(ix.p2, 1.0f);
        uint j = 0u;
        itemOf(seg, n, uint(K * M), atB, j, s);
        if (float(j) >= K * M) return dead();
        uint vi = j / uint(M), k = j % uint(M);
        float g  = smoothstep(0.0f, 0.45f, u);
        float vj = (float(k) + 0.5f) / M;
        float grow = saturate((g - vj) * 8.0f);
        if (grow <= 0.0f) return dead();
        float3 P = vinePos(ix, vi, vj, K);
        float3 T = normalize(vinePos(ix, vi, vj + 0.01f, K) - P);
        float3 N = normalize(float3(P.x - o.x, P.y - o.y, 0.001f));
        float side = (k % 2u == 0u) ? 1.0f : -1.0f;
        float3 d = normalize(N * 0.8f + UP * 0.45f + cross(T, N) * 0.35f * side);
        float len = r * 0.45f * grow * (0.7f + 0.6f * hash2(float(j), 4.4f));
        return mk(P + d * len * s, 1.0f, 0.3f + 0.7f * s, (1.0f - s) * 1.8f);
    }
    if (f == FORM_GLYPH)
    {
        uint gid = min(uint(ix.p0), GLYPH_COUNT - 1u);
        uint2 range = GRANGE[gid];
        uint cnt = max(range.y, 1u);
        float C = max(ix.p2, 1.0f);
        uint ci = seg / cnt + uint(ix.p5);
        uint j  = seg % cnt;
        if (float(ci) >= C || j >= range.y) return dead();
        float4 e = GSEG[range.x + j];
        float2 q = atB ? e.zw : e.xy;
        uint anim = uint(ix.p3);
        float size = ix.p4 > 0.0f ? ix.p4 : 1.0f;
        float gain = 1.0f;
        if (anim & GA_POP) size *= easeOutBack(u * 5.0f);
        if (anim & GA_ROLL)
        {
            float ra = tm * 1.6f + float(ci) * 0.7f;
            q = float2(q.x * cos(ra) - q.y * sin(ra), q.x * sin(ra) + q.y * cos(ra));
        }
        if (anim & GA_WIGGLE)
        {
            float k = saturate(abs(q.x) * 1.5f) * saturate(q.y + 0.2f);
            q.x += sin(tm * 7.0f + float(ci) * 2.1f + q.y * 3.0f) * 0.14f * k;
            q.y += cos(tm * 5.0f + float(ci) * 1.3f) * 0.06f * k;
        }
        float ang = TAU * float(ci) / C + ix.phase * TAU + ((anim & GA_SPIN) ? tm * 0.6f : 0.0f);
        float ringR = (C > 1.0f || ix.p5 > 0.0f) ? r : 0.0f;
        float3 c = o + float3(cos(ang), sin(ang), 0) * ringR + UP * H;
        if (anim & GA_RISE)   c.z -= (1.0f - easeOut(u * 2.0f + hash2(float(ci), 1.9f) * 0.3f)) * size * 2.4f;
        if (anim & GA_BOB)    c.z += sin(tm * 2.0f + float(ci)) * size * 0.15f;
)HLSL"
R"HLSL(        if (anim & GA_ASCEND) { c.z += u * size * 3.0f; gain *= 1.0f - u; }
        if (anim & GA_DESCEND) { float d = saturate(u * 3.0f); c.z += (1.0f - d * d) * size * 4.0f; }
        float3 ax, ay;
        uint mode = uint(ix.p1);
        if (mode == 1u)      { ax = rightOf(ix.yaw); ay = UP; }
        else if (mode == 2u) { ax = fwdOf(ix.yaw);   ay = float3(-sin(ix.yaw), cos(ix.yaw), 0); c.z = o.z + H + 0.1f; }
        else if (mode == 3u) { ax = float3(-sin(ang), cos(ang), 0); ay = UP; }
        else if (mode == 4u)
        {
            float3 v = normalize(gCamPos.xyz - c);
            ax = normalize(cross(UP, float3(v.x, v.y, 0.0001f)));
            ay = UP;
        }
        else { ax = gCamRight.xyz; ay = gCamUp.xyz; }
        return mk(c + ax * q.x * size + ay * q.y * size, gain, q.y * 0.5f + 0.5f, 1.0f);
    }
    if (f == FORM_WINGS)
    {
        uint F = uint(clamp(ix.p0, 3.0f, 12.0f));
        uint E = 8u;
        uint per = E + F * 3u;
        uint w = seg / per, j = seg % per;
        if (w > 1u) return dead();
        float span = (ix.p2 > 0.0f ? ix.p2 : r * 2.5f) * easeOut(u * 3.0f);
        float rate = ix.p1 > 0.0f ? ix.p1 : 0.8f;
        float flap = 0.25f + 0.55f * sin(tm * rate * TAU + ix.phase * TAU);
        float2 q; float grad, wm;
        if (j < E)
        {
            float t = (float(j) + (atB ? 1.0f : 0.0f)) / float(E);
            q = wingBone(t); grad = t; wm = 1.3f - 0.8f * t;
        }
        else
        {
            uint k = (j - E) / 3u, m = (j - E) % 3u;
            float fs = (float(m) + (atB ? 1.0f : 0.0f)) / 3.0f;
            float tk = 0.12f + 0.88f * float(k) / float(F - 1u);
            float len = (0.35f + 0.45f * sin(PI * min(tk * 1.15f, 1.0f))) * (1.0f + 0.2f * tk);
            float2 d = normalize(float2(0.35f * tk + 0.1f, -1.0f));
            d.x += sin(tm * 6.0f + float(k)) * 0.03f;
            q = wingBone(tk) + d * len * fs;
            q.x += 0.06f * fs * fs * tk;
            grad = 0.4f + 0.6f * fs; wm = (1.0f - fs * 0.85f) * 0.9f;
        }
        q *= span;
        float sideSign = (w == 0u) ? 1.0f : -1.0f;
        float3 fwd = fwdOf(ix.yaw);
        float3 out3 = rightOf(ix.yaw) * sideSign;
        float3 wOut = out3 * cos(flap) + UP * sin(flap);
        float3 wUp  = -out3 * sin(flap) + UP * cos(flap);
        float3 root = o + UP * H - fwd * r * 0.35f + out3 * r * 0.2f;
        return mk(root + wOut * q.x + wUp * q.y - fwd * 0.3f * q.x, 1.0f, grad, wm);
    }
    if (f == FORM_LIGHTNING || f == FORM_BOLT || f == FORM_DRAIN)
    {
        float3 A = o, B = ix.pos2;
        if (f == FORM_LIGHTNING && ix.p3 > 0.5f) { A = ix.pos2 + UP * H; B = ix.pos2; }
        float3 d = B - A;
        float  L = max(length(d), 0.001f);
        float3 dir = d / L, e1, e2;
        basisOf(dir, e1, e2);

        if (f == FORM_LIGHTNING)
        {
            float K = max(ix.p0, 1.0f);
            itemOf(seg, n, uint(K), atB, i, s);
            if (float(i) >= K) return dead();
            uint spi = max(n / uint(K), 1u);
            float node = float(seg % spi + (atB ? 1u : 0u));
            float q = floor(gTime * (ix.p2 > 0.0f ? ix.p2 : 12.0f));
            float jag = (ix.p1 > 0.0f ? ix.p1 : 1.0f) * L * 0.07f;
            float env = sin(PI * s);
            float fi = float(i);
            float3 off = (hash2(node + fi * 37.0f, q) - 0.5f) * 2.0f * e1
                       + (hash2(node * 1.7f + fi * 11.0f, q + 3.0f) - 0.5f) * 2.0f * e2;
            float gain = (0.55f + 0.45f * hash2(q, fi + 1.0f)) * step(0.15f, hash2(q, 9.1f));
            return mk(lerp(A, B, s) + off * jag * env * (1.0f + 0.5f * fi), gain,
                      abs(0.5f - s) * 2.0f, 1.0f - 0.5f * fi / K);
        }
        if (f == FORM_BOLT)
        {
            float K = max(ix.p0, 1.0f);
            itemOf(seg, n, uint(K), atB, i, s);
            if (float(i) >= K || u >= 0.7f) return dead();
            float head = saturate(u / 0.7f);
            float tail = ix.p2 > 0.0f ? ix.p2 : 0.35f;
            float w = head - s * tail;
            if (w < 0.0f) return dead();
            float3 path = lerp(A, B, w) + UP * ix.p3 * sin(PI * w);
            float ang = tm * 10.0f + float(i) * TAU / K + w * 25.0f;
            float rad = (i == 0u) ? 0.0f : (ix.p1 > 0.0f ? ix.p1 : 0.6f) * (1.0f - s);
            return mk(path + (e1 * cos(ang) + e2 * sin(ang)) * rad, pow(1.0f - s, 1.5f), s,
                      (1.0f - s) * 1.5f + 0.2f);
        }
        float K = max(ix.p0, 1.0f), D = max(ix.p2, 4.0f);
        uint j = 0u;
        itemOf(seg, n, uint(K * D), false, j, s);
        if (float(j) >= K * D) return dead();
        uint si = j / uint(D), mi = j % uint(D);
        float w = frac(tm * 0.6f + float(mi) / D + hash2(float(si), 2.2f));
        float wp = atB ? min(w + 0.05f, 1.0f) : w;
        float ang = float(si) * TAU / K + wp * 12.0f + tm * 3.0f;
        float3 p = lerp(A, B, wp) + UP * (ix.p1 > 0.0f ? ix.p1 : 1.5f) * sin(PI * wp)
                 + (e1 * cos(ang) + e2 * sin(ang)) * 0.5f * sin(PI * wp);
        return mk(p, sin(PI * w), wp, 1.0f);
    }
    if (f == FORM_RAIN)
    {
        float K = max(ix.p0, 1.0f);
        if (float(seg) >= K) return dead();
        float fi = float(seg) + ix.phase * 17.0f;
        float h1 = hash2(fi, 1.1f), h2 = hash2(fi, 2.3f), h3 = hash2(fi, 3.7f), h4 = hash2(fi, 4.9f);
        float rr = sqrt(h1) * r, a = TAU * h2;
        float c = frac(tm * (0.9f + 0.5f * h3) + h4);
        float len = ix.p1 > 0.0f ? ix.p1 : H * 0.12f;
        float3 top = o + float3(cos(a) * rr, sin(a) * rr, H * (1.0f - c) + len);
        float3 bot = top - float3(ix.p2 * len, 0, len);
        return mk(atB ? bot : top, smoothstep(0.0f, 0.1f, c) * (1.0f - smoothstep(0.9f, 1.0f, c)),
                  atB ? 1.0f : 0.0f, 1.0f);
    }
    if (f == FORM_SPARKLE)
    {
        float K = max(ix.p0, 1.0f);
        uint m = seg / 2u, arm = seg % 2u;
        if (float(m) >= K) return dead();
        float fm = float(m) + ix.phase * 13.0f;
        float h1 = hash2(fm, 1.3f), h2 = hash2(fm, 2.9f), h3 = hash2(fm, 4.1f), h4 = hash2(fm, 5.7f);
        float c = frac(tm * (0.25f + 0.35f * h1) + h2);
        float a = TAU * h4 + tm * 0.4f * (h1 - 0.5f);
        float rr = r * (0.35f + 0.8f * h3), z = H * c;
        uint mode = uint(ix.p2);
        if (mode == 1u) z = H * (1.0f - c);
        else if (mode == 2u) { rr = r * 1.8f * (1.0f - c); z = H * (0.2f + 0.6f * h3); a += c * 2.0f; }
        float3 center = o + float3(cos(a) * rr, sin(a) * rr, z);
        float size = (ix.p1 > 0.0f ? ix.p1 : 0.12f) * (0.6f + 0.8f * h1) * (0.6f + 0.4f * sin(tm * 9.0f + float(m) * 3.0f));
        float3 axis = (arm == 0u) ? gCamRight.xyz : gCamUp.xyz;
        float sc = sin(PI * c);
        return mk(center + axis * size * (atB ? 1.0f : -1.0f),
                  sc * sc * (0.5f + 0.5f * sin(tm * 13.0f + float(m) * 5.0f)), c, 1.0f);
    }
    if (f == FORM_RUNE)
    {
        uint nS = uint(clamp(ix.p0, 3.0f, 16.0f));
        uint kS = max(uint(ix.p1), 1u);
        uint nT = uint(clamp(ix.p2, 0.0f, 24.0f));
        float e = atB ? 1.0f : 0.0f;
        float2 q;
        if (seg < 28u)
        {
            float a = TAU * (float(seg) + e) / 28.0f + tm * 0.25f;
            q = float2(cos(a), sin(a));
        }
        else if (seg < 52u)
        {
            float a = TAU * (float(seg - 28u) + e) / 24.0f - tm * 0.35f;
            q = 0.84f * float2(cos(a), sin(a));
        }
        else if (seg < 52u + nS)
        {
            float vtx = float(seg - 52u) + e;
            float a = TAU * vtx * float(kS) / float(nS) + tm * 0.5f + PI * 0.5f;
            q = 0.8f * float2(cos(a), sin(a));
        }
        else if (seg < 52u + nS + nT)
        {
            float a = TAU * float(seg - 52u - nS) / float(nT) - tm * 0.35f;
            q = (atB ? 0.98f : 0.87f) * float2(cos(a), sin(a));
        }
        else return dead();
        float R = r * easeOut(u * 3.0f);
        float3 c = o + UP * 0.12f, ax = float3(1, 0, 0), ay = float3(0, 1, 0);
        uint orient = uint(ix.p3);
        if (orient == 1u)      { c = o + UP * H; ax = gCamRight.xyz; ay = gCamUp.xyz; }
        else if (orient == 2u) { c = o + fwdOf(ix.yaw) * r * 0.3f + UP * H; ax = rightOf(ix.yaw); ay = UP; }
        return mk(c + ax * q.x * R + ay * q.y * R, 0.8f + 0.2f * sin(tm * 3.0f), length(q), 1.0f);
    }
    if (f == FORM_VORTEX)
    {
        float K = max(ix.p0, 1.0f);
        itemOf(seg, n, uint(K), atB, i, s);
        if (float(i) >= K) return dead();
        float flare = ix.p1 > 0.0f ? ix.p1 : 1.5f;
        float turns = ix.p2 > 0.0f ? ix.p2 : 2.5f;
        float spin  = ix.p3 != 0.0f ? ix.p3 : 2.0f;
        float rad = r * (0.2f + 0.8f * pow(s, flare));
        float a = TAU * float(i) / K + s * turns * TAU - tm * spin;
        return mk(o + float3(cos(a) * rad, sin(a) * rad, H * s * easeOut(u * 2.0f)),
                  0.5f + 0.5f * s, s, 0.6f + 0.8f * s);
    }
    if (f == FORM_DOME)
    {
        uint M = uint(clamp(ix.p0, 2.0f, 10.0f)), P = uint(clamp(ix.p1, 1.0f, 4.0f));
        uint S = min(8u, (128u - P * 16u) / M);
        float g = easeOut(u * 4.0f);
        float el, a;
        float e = atB ? 1.0f : 0.0f;
        if (seg < M * S)
        {
            el = (float(seg % S) + e) / float(S) * PI * 0.5f;
            a  = TAU * float(seg / S) / float(M) + tm * 0.2f;
        }
        else
        {
            uint j = seg - M * S, pi = j / 16u;
            if (pi >= P) return dead();
            el = float(pi + 1u) / float(P + 1u) * PI * 0.5f;
            a  = TAU * (float(j % 16u) + e) / 16.0f - tm * 0.15f;
        }
        float3 p = o + float3(cos(a) * cos(el) * r * g, sin(a) * cos(el) * r * g, sin(el) * H * g);
        float ripple = 0.5f + 0.5f * sin(tm * 3.0f - el * 6.0f);
        return mk(p, 0.45f + 0.55f * ripple * ripple, sin(el), 1.0f);
    }
    if (f == FORM_CRYSTAL)
    {
        float K = max(ix.p0, 1.0f);
        uint ci = seg / 5u, ed = seg % 5u;
        if (float(ci) >= K) return dead();
        float fc = float(ci) + ix.phase * 7.0f;
        float h1 = hash2(fc, 1.7f), h2 = hash2(fc, 2.1f), h3 = hash2(fc, 3.3f), h4 = hash2(fc, 4.7f);
        bool ground = ix.p1 > 0.5f;
        float a = TAU * (float(ci) + h1 * 0.7f) / K;
        float3 nrm = float3(cos(a), sin(a), 0), tang = float3(-sin(a), cos(a), 0);
        float3 B = o + nrm * r * (0.85f + 0.35f * h2) + UP * (ground ? 0.0f : H * (0.05f + 0.7f * h3));
        float3 axis = normalize(nrm * (ground ? 0.55f : 0.9f) + UP * (ground ? 1.0f : 0.5f) + tang * (h4 - 0.5f) * 0.4f);
        float grow = easeOut(u * 4.0f - h4 * 0.8f);
        float L = H * (0.18f + 0.22f * h1) * (ix.p2 > 0.0f ? ix.p2 : 1.0f) * grow;
        if (L <= 0.0f) return dead();
        float3 T = B + axis * L;
        float3 Mp = B + axis * L * 0.35f + tang * L * 0.2f;
        float3 Mm = B + axis * L * 0.35f - tang * L * 0.2f;
        float3 p;
        if (ed == 0u)      p = atB ? Mp : B;
        else if (ed == 1u) p = atB ? T : Mp;
        else if (ed == 2u) p = atB ? Mm : T;
        else if (ed == 3u) p = atB ? B : Mm;
        else               p = atB ? T : B;
        return mk(p, 0.7f + 0.3f * sin(tm * 4.0f + fc), saturate(dot(p - B, axis) / L), 1.0f);
    }
    if (f == FORM_BUBBLES)
    {
        float K = max(ix.p0, 1.0f);
        uint bi = seg / 8u;
        if (float(bi) >= K) return dead();
        float fb = float(bi) + ix.phase * 5.0f;
        float h1 = hash2(fb, 1.9f), h2 = hash2(fb, 2.4f), h3 = hash2(fb, 3.8f), h4 = hash2(fb, 4.2f);
        float t = (float(seg % 8u) + (atB ? 1.0f : 0.0f)) / 8.0f;
        float c = frac(tm * (0.3f + 0.3f * h1) + h2);
        float a = TAU * h4;
        float3 center = o + float3(cos(a), sin(a), 0) * r * (0.3f + 0.8f * h3) + UP * H * c;
        float rad = (ix.p1 > 0.0f ? ix.p1 : 0.25f) * (0.5f + 0.8f * h4) * (0.6f + 0.6f * c);
        float gain = smoothstep(0.0f, 0.1f, c);
        if (c > 0.9f) { rad *= 1.0f + (c - 0.9f) * 8.0f; gain *= 1.0f - (c - 0.9f) * 10.0f; }
        float ang = TAU * t;
        return mk(center + (gCamRight.xyz * cos(ang) + gCamUp.xyz * sin(ang)) * rad, gain, c, 1.0f);
    }
    if (f == FORM_WIND)
    {
        float K = max(ix.p0, 1.0f);
        uint si = seg / 6u;
        if (float(si) >= K) return dead();
        float fs = float(si) + ix.phase * 3.0f;
        float h1 = hash2(fs, 1.2f), h2 = hash2(fs, 2.6f), h3 = hash2(fs, 3.1f), h4 = hash2(fs, 4.5f);
        float t = (float(seg % 6u) + (atB ? 1.0f : 0.0f)) / 6.0f;
        float c = frac(tm * (0.9f + 0.6f * h1) + h2);
        float side = h3 > 0.5f ? 1.0f : -1.0f;
        float z = H * (0.15f + 0.75f * h4);
        float lat = r * (0.8f + 0.5f * h1) * side + side * 0.25f * r * t * t;
        float along = r * (1.2f - 4.0f * c) - t * (ix.p1 > 0.0f ? ix.p1 : r * 1.2f);
        return mk(o + fwdOf(ix.yaw) * along + rightOf(ix.yaw) * lat + UP * z,
                  sin(PI * c) * (1.0f - t), t, 1.0f - 0.7f * t);
    }
    if (f == FORM_BURST)
    {
        uint K = uint(clamp(ix.p0, 1.0f, 3.0f)), R = uint(clamp(ix.p2, 0.0f, 24.0f));
        if (seg < K * 32u)
        {
            float ri = float(seg / 32u);
            float v = saturate((u - ri * 0.12f) / 0.6f);
            if (v <= 0.0f) return dead();
            float a = TAU * (float(seg % 32u) + (atB ? 1.0f : 0.0f)) / 32.0f;
            float rad = r * easeOut(v);
            return mk(o + float3(cos(a) * rad, sin(a) * rad, 0.3f + ix.p1 * v),
                      (1.0f - v) * (1.0f - v), v, 1.0f);
        }
        uint j = seg - K * 32u;
        if (j >= R) return dead();
        float v = saturate(u / 0.5f);
        float a = TAU * float(j) / float(R) + ix.phase * TAU;
        float rad = r * easeOut(v) * (atB ? 1.05f : 0.35f);
        return mk(o + float3(cos(a) * rad, sin(a) * rad, 0.3f + ix.p1 * v * 0.5f),
                  (1.0f - v) * (1.0f - v), atB ? 1.0f : 0.0f, atB ? 0.2f : 1.0f);
    }
    if (f == FORM_CRACKS)
    {
        float K = max(ix.p0, 1.0f);
        uint ci = seg / 8u;
)HLSL"
R"HLSL(        if (float(ci) >= K) return dead();
        float fc = float(ci) + ix.phase * 9.0f;
        float node = float(seg % 8u + (atB ? 1u : 0u));
        float fa = node / 8.0f;
        float g = easeOut(u * 2.5f);
        float len = r * (0.6f + 0.5f * hash2(fc, 1.4f));
        float a = TAU * float(ci) / K + (hash2(fc, 2.8f) - 0.5f) * 0.6f;
        float jit = (node > 0.0f) ? (hash2(node, fc) - 0.5f) * (ix.p1 > 0.0f ? ix.p1 : 1.0f) * r * 0.12f : 0.0f;
        float3 p = o + float3(cos(a), sin(a), 0) * len * fa * g + float3(-sin(a), cos(a), 0) * jit + UP * 0.15f;
        return mk(p, 1.0f - fa * 0.5f, fa, 1.2f - fa);
    }
    return dead();
}

float3 drift(FxInstance ix, float3 p)
{
    float a = gTime * ix.p5 + ix.phase * TAU;
    float s = sin(a), c = cos(a);
    float3 d = p - ix.pos;
    d.xy = float2(d.x * c - d.y * s, d.x * s + d.y * c);
    d.z += sin(gTime * 1.1f + ix.phase * TAU) * 0.05f * max(ix.height, 1.0f);
    return ix.pos + d;
}

FormOut evalForm(FxInstance ix, uint seg, uint n, bool atB, float tm, float u)
{
    if (ix.form < FORM_FLAME)
    {
        float t = float(seg + (atB ? 1u : 0u)) / float(n);
        float g = 1.0f;
        float3 p = evalLegacy(ix, t, tm, g);
        return mk(p, g, 0.0f, 1.0f);
    }
    return evalSpell(ix, seg, n, atB, tm, u);
}

struct VSOut { float4 pos : SV_POSITION; float4 col : COLOR0; };

float4 screenToClip(float2 px)
{
    return float4((px.x - gViewX) / max(gViewW, 1.0f) * 2.0f - 1.0f,
                  1.0f - (px.y - gViewY) / max(gViewH, 1.0f) * 2.0f,
                  0.0f, 1.0f);
}

VSOut VSMain(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
    VSOut o;
    FxInstance ix = gInst[iid + gBase];

    uint seg    = vid / 6u;
    uint corner = vid % 6u;

    bool atB  = (corner == 2u || corner == 3u || corner == 5u);
    bool plus = (corner == 1u || corner == 4u || corner == 5u);

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

    uint segCount = clamp(ix.segments, 1u, 128u);
    if (seg >= segCount)
    {
        o.pos = float4(0, 0, 0, 0);
        o.col = 0;
        return o;
    }

    float tm = gTime * ix.speed * ((ix.flags & F_REVERSE) ? -1.0f : 1.0f);
    float t0 = float(seg)      / float(segCount);
    float t1 = float(seg + 1u) / float(segCount);

    float u = 1.0f, env = 1.0f;
    if (ix.life > 0.0f)
    {
        float age = gTime - ix.birth;
        if (age < 0.0f) { o.pos = float4(0, 0, 0, 0); o.col = 0; return o; }
        u = saturate(age / ix.life);
        if (!(ix.flags & F_NOENV))
            env = smoothstep(0.0f, 0.06f, u) * (1.0f - smoothstep(0.75f, 1.0f, u));
    }

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

    FormOut fa = evalForm(ix, seg, segCount, false, tm, u);
    FormOut fb = evalForm(ix, seg, segCount, true,  tm, u);
    if ((ix.flags & F_DRIFT) && ix.form != FORM_GLYPH)
    {
        fa.p = drift(ix, fa.p);
        fb.p = drift(ix, fb.p);
    }
    float3 a = fa.p, b = fb.p;

    float3 dir = b - a;
    float  len = length(dir);
    float3 view = normalize(gCamPos.xyz - (a + b) * 0.5f);
    float3 side = (len > 1e-5f) ? normalize(cross(normalize(dir), view))
                                : gCamRight.xyz;

    float3 hereP    = atB ? fb.p    : fa.p;
    float  hereGain = atB ? fb.gain : fa.gain;
    float  hereGrad = atB ? fb.grad : fa.grad;
    float  hereWmul = atB ? fb.wmul : fa.wmul;
    float w = ix.width * max(hereWmul, 0.0f);
    if (ix.flags & F_SCREENW)
    {
        float dist = max(length(gCamPos.xyz - hereP), 0.001f);
        w = w * dist / max(gViewH, 1.0f);
    }

    float3 world = hereP + side * (plus ? w : -w);

    if (fa.wmul < 0.0f || fb.wmul < 0.0f)
    {
        o.pos = float4(0, 0, 0, 0); o.col = 0; return o;
    }

    float4 c2 = any(ix.rgba2) ? ix.rgba2 : ix.rgba;
    float4 col = lerp(ix.rgba, c2, saturate(hereGrad));

    o.pos = mul(float4(world, 1.0f), gViewProj);
    o.col = float4(col.rgb, col.a * hereGain * env);
    return o;
}

float4 PSMain(VSOut i) : SV_TARGET
{
    return float4(i.col.rgb * i.col.a, i.col.a);
}
)HLSL";

} // namespace

LineFX::~LineFX() { ReleaseAll(); }

void LineFX::ReleaseAll()
{
	if (m_compileThread.joinable())
		m_compileThread.join();
	ReleaseDeviceObjects();
}

void LineFX::ReleaseDeviceObjects()
{
	Rel(m_vs); Rel(m_ps); Rel(m_cb);
	Rel(m_instSrv); Rel(m_instBuf);
	m_instCap = 0;
	Rel(m_blendAlpha); Rel(m_blendAdd);
	Rel(m_dssTest); Rel(m_dssNone); Rel(m_raster);
	Rel(m_ctx);
	m_device = nullptr;
	m_ready = false;
}

namespace {

uint64_t Fnv1a(const std::string& text)
{
	uint64_t h = 1469598103934665603ull;
	for (unsigned char c : text)
		h = (h ^ c) * 1099511628211ull;
	return h;
}

std::string EffectSource()
{
	return GlyphHlsl() + kShader;
}

bool Compile(const std::string& src, const char* entry, const char* target, std::string& code, std::string& error)
{
	ID3DBlob* blob = nullptr;
	ID3DBlob* err = nullptr;
	const bool ok = SUCCEEDED(D3DCompile(src.data(), src.size(), nullptr, nullptr, nullptr, entry, target, 0, 0,
		&blob, &err));
	if (ok)
		code.assign(static_cast<const char*>(blob->GetBufferPointer()), blob->GetBufferSize());
	else
	{
		const char* msg = err ? static_cast<const char*>(err->GetBufferPointer()) : nullptr;
		error = std::string(entry) + ": " + (msg ? std::string(msg).substr(0, 200) : std::string("compile failed"));
	}
	Rel(blob);
	Rel(err);
	return ok;
}

constexpr uint32_t kCacheMagic = 0x58464645;

} // namespace

bool LineFX::LoadCache(uint64_t hash)
{
	if (m_cacheFile.empty())
		return false;
	std::ifstream in(m_cacheFile, std::ios::binary);
	uint32_t magic = 0, vsSize = 0, psSize = 0;
	uint64_t fileHash = 0;
	if (!in.read(reinterpret_cast<char*>(&magic), sizeof(magic)) || magic != kCacheMagic
		|| !in.read(reinterpret_cast<char*>(&fileHash), sizeof(fileHash)) || fileHash != hash
		|| !in.read(reinterpret_cast<char*>(&vsSize), sizeof(vsSize))
		|| !in.read(reinterpret_cast<char*>(&psSize), sizeof(psSize))
		|| vsSize == 0 || psSize == 0 || vsSize > (16u << 20) || psSize > (16u << 20))
		return false;
	m_vsCode.resize(vsSize);
	m_psCode.resize(psSize);
	return static_cast<bool>(in.read(m_vsCode.data(), vsSize)) && static_cast<bool>(in.read(m_psCode.data(), psSize));
}

void LineFX::SaveCache(uint64_t hash) const
{
	if (m_cacheFile.empty())
		return;
	std::ofstream out(m_cacheFile, std::ios::binary | std::ios::trunc);
	const uint32_t vsSize = static_cast<uint32_t>(m_vsCode.size());
	const uint32_t psSize = static_cast<uint32_t>(m_psCode.size());
	out.write(reinterpret_cast<const char*>(&kCacheMagic), sizeof(kCacheMagic));
	out.write(reinterpret_cast<const char*>(&hash), sizeof(hash));
	out.write(reinterpret_cast<const char*>(&vsSize), sizeof(vsSize));
	out.write(reinterpret_cast<const char*>(&psSize), sizeof(psSize));
	out.write(m_vsCode.data(), vsSize);
	out.write(m_psCode.data(), psSize);
}

void LineFX::StartCompile()
{
	m_compileStarted = true;
	std::string src = EffectSource();
	m_sourceHash = Fnv1a(src);
	if (LoadCache(m_sourceHash))
	{
		m_compileDone = true;
		return;
	}
	m_compileThread = std::thread([this, src = std::move(src)]() {
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
		std::string error;
		if (Compile(src, "VSMain", "vs_5_0", m_vsCode, error) && Compile(src, "PSMain", "ps_5_0", m_psCode, error))
		{
			error.clear();
			SaveCache(m_sourceHash);
		}
		m_compileError = std::move(error);
		m_compileDone = true;
	});
}

bool LineFX::CreateShaders()
{
	if (!m_compileStarted)
		StartCompile();
	if (!m_compileDone)
	{
		m_error = "compiling shaders";
		return false;
	}
	if (m_compileThread.joinable())
		m_compileThread.join();
	if (!m_compileError.empty())
	{
		std::snprintf(m_errorBuf, sizeof(m_errorBuf), "%s", m_compileError.c_str());
		m_error = m_errorBuf;
		return false;
	}

	const bool ok = SUCCEEDED(m_device->CreateVertexShader(m_vsCode.data(), m_vsCode.size(), nullptr, &m_vs))
		&& SUCCEEDED(m_device->CreatePixelShader(m_psCode.data(), m_psCode.size(), nullptr, &m_ps));
	if (!ok)
	{
		Rel(m_vs);
		Rel(m_ps);
		m_error = "shader object creation failed";
	}
	return ok;
}

bool LineFX::CreateBuffers()
{
	Rel(m_cb);
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
	return true;
}

bool LineFX::EnsureInstanceCapacity(uint32_t count)
{
	if (count <= m_instCap)
		return true;
	uint32_t cap = m_instCap ? m_instCap : 256;
	while (cap < count)
		cap *= 2;
	cap = std::min(cap, kMaxInstances);
	Rel(m_instSrv);
	Rel(m_instBuf);
	m_instCap = 0;

	D3D11_BUFFER_DESC ibd = {};
	ibd.ByteWidth = sizeof(FxInstance) * cap;
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
	srv.Buffer.NumElements = cap;
	if (FAILED(m_device->CreateShaderResourceView(m_instBuf, &srv, &m_instSrv)))
	{
		m_error = "instance SRV creation failed";
		return false;
	}
	m_instCap = cap;
	return true;
}

bool LineFX::CreateStates()
{
	Rel(m_blendAlpha); Rel(m_blendAdd);
	Rel(m_dssTest); Rel(m_dssNone); Rel(m_raster);
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
	rd.CullMode = D3D11_CULL_NONE;
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
		ReleaseDeviceObjects();

	m_device = device;
	if (!m_ctx)
		m_device->GetImmediateContext(&m_ctx);
	if (!m_ctx)
	{
		m_error = "no immediate context";
		return false;
	}

	if (m_device->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
	{
		m_error = "device feature level below 11_0";
		return false;
	}

	if (!m_vs && !CreateShaders())
		return false;
	if (!CreateBuffers() || !CreateStates())
		return false;

	m_error = "";
	m_ready = true;
	return true;
}

void LineFX::Render(const FxInstance* instances, size_t count,
	const float viewProj[16], const float camPos[3],
	const float camRight[3], const float camUp[3],
	double timeSec, float viewportW, float viewportH)
{
	m_lastSegments = 0;
	m_lastInstances = 0;
	if (!m_ready || !instances || count == 0)
		return;
	if (count > kMaxInstances)
		count = kMaxInstances;

	UINT bucketCount[4] = { 0, 0, 0, 0 };
	uint32_t bucketSegs[4] = { 1, 1, 1, 1 };
	for (size_t i = 0; i < count; ++i)
	{
		const int b = BucketOf(instances[i]);
		++bucketCount[b];
		bucketSegs[b] = std::max(bucketSegs[b], SegsDrawn(instances[i]));
	}

	if (!EnsureInstanceCapacity(static_cast<uint32_t>(count)))
		return;

	D3D11_MAPPED_SUBRESOURCE map = {};
	if (FAILED(m_ctx->Map(m_instBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
		return;
	const double timeBase = std::floor(timeSec / kTimeWrap) * kTimeWrap;
	FxInstance* dst = static_cast<FxInstance*>(map.pData);
	UINT cursor[4] = { 0, bucketCount[0], bucketCount[0] + bucketCount[1], bucketCount[0] + bucketCount[1] + bucketCount[2] };
	for (size_t i = 0; i < count; ++i)
	{
		FxInstance ix = instances[i];
		ix.birth = static_cast<float>(static_cast<double>(ix.birth) - timeBase);
		dst[cursor[BucketOf(ix)]++] = ix;
	}
	m_ctx->Unmap(m_instBuf, 0);

	CB cb = {};
	std::memcpy(cb.viewProj, viewProj, sizeof(cb.viewProj));
	cb.camPos[0] = camPos[0];   cb.camPos[1] = camPos[1];   cb.camPos[2] = camPos[2];
	cb.camRight[0] = camRight[0]; cb.camRight[1] = camRight[1]; cb.camRight[2] = camRight[2];
	cb.camUp[0] = camUp[0];     cb.camUp[1] = camUp[1];     cb.camUp[2] = camUp[2];
	cb.timeSec = static_cast<float>(timeSec - timeBase);
	D3D11_VIEWPORT vp = {};
	UINT vpCount = 1;
	m_ctx->RSGetViewports(&vpCount, &vp);
	const bool vpValid = vpCount && vp.Width > 0.0f && vp.Height > 0.0f;
	cb.viewportX = vpValid ? vp.TopLeftX : 0.0f;
	cb.viewportY = vpValid ? vp.TopLeftY : 0.0f;
	cb.viewportW = vpValid ? vp.Width : viewportW;
	cb.viewportH = vpValid ? vp.Height : viewportH;

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
	ID3D11ShaderResourceView* prevVsSrv = nullptr;
	m_ctx->VSGetShaderResources(0, 1, &prevVsSrv);
	m_ctx->VSSetShaderResources(0, 1, &m_instSrv);
	m_ctx->RSSetState(m_raster);
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
		cb.baseInstance = start;
		if (FAILED(m_ctx->Map(m_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
			break;
		std::memcpy(map.pData, &cb, sizeof(cb));
		m_ctx->Unmap(m_cb, 0);
		m_ctx->DrawInstanced(bucketSegs[b] * kVertsPerSeg, n, 0, 0);
		start += n;
	}
	m_ctx->VSSetShaderResources(0, 1, &prevVsSrv);
	Rel(prevVsSrv);

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
