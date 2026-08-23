/*
 * gpu/LineFX.h - GPU-generated world effects.
 *
 * WHY THIS EXISTS
 * ---------------
 * The Lua path builds every ring, helix and mote segment on the CPU, then hands
 * each one to the engine's DrawLine2D - which, at thickness N, becomes 2N+1
 * separate calls per segment. A few dozen animated spawns is tens of thousands
 * of calls a frame, and it does not hold framerate.
 *
 * Here the CPU writes ONE record per effect (80 bytes) and the GPU generates all
 * of that effect's geometry procedurally in the vertex shader. Cost per effect
 * drops from "hundreds of segments of CPU work plus hundreds of engine calls" to
 * a single struct write, and segment counts stop being a budget problem.
 *
 * WHERE IT DRAWS
 * --------------
 * From MQ's GraphicsSceneRender callback, which MQ drives off its detour on
 * CParticleSystem::Render. In the client's RenderScene that lands AFTER
 * RenderScenePasses - the world is drawn and the depth buffer is populated - so
 * binding the game's DSV gives real occlusion: effects go behind terrain and
 * models instead of over them. MQ wraps the callback in a full DX11
 * ScopedStateBlock (shaders, IA, RS, OM, blend, and the RTV/DSV), so we may set
 * any state we want without restoring it ourselves.
 *
 * GEOMETRY
 * --------
 * No vertex or index buffer. Draw(vertsPerInstance, instanceCount), with the
 * shader deriving everything from SV_VertexID and SV_InstanceID against a
 * StructuredBuffer of effect records. Segments expand into camera-facing ribbons
 * in the VS, so thickness is a real width rather than the old trick of drawing
 * 2N+1 parallel copies of a 1px line.
 *
 * THREADING
 * ---------
 * Render-thread only, from OnSceneRender or MQ's Create/InvalidateDeviceObjects
 * callbacks. Producers hand work in as a flat array; this class owns no scene.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11BlendState;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;

namespace eqgfx::gpu {

/// Procedural forms the vertex shader can generate. The catalogue composes
/// these with parameters to build every per-class, per-element and per-buff
/// effect, so adding a look is a table entry rather than new shader code.
enum FxForm : uint32_t
{
	FxForm_Ring = 0,   ///< circle in a tilted, spinning plane (gyros, halos, ground rings)
	FxForm_Helix,      ///< strand climbing over p0 turns
	FxForm_Orbit,      ///< short dashes on independent orbits (motes / fireflies)
	FxForm_Polygon,    ///< flat n-gon, p0 sides
	FxForm_Bloom,      ///< rings expanding outward and fading
	FxForm_Spire,      ///< vertical rays fanning from the center
	FxForm_Lattice,    ///< two rings joined by verticals (prisms, cages)
	FxForm_Segment,    ///< plain A->B run; the Lua add_line path rides this
	FxForm_Rect,       ///< filled quad from pos..pos2; the Lua add_screen_rect path
	FxForm_Count
};

enum FxFlag : uint32_t
{
	FxFlag_Comet       = 1u << 0,  ///< chase a bright head around the loop
	FxFlag_NoDepth     = 1u << 1,  ///< draw over the world instead of occluding
	FxFlag_ScreenWidth = 1u << 2,  ///< width is in pixels rather than world units
	FxFlag_Additive    = 1u << 3,  ///< additive blend instead of alpha-over
	FxFlag_Reverse     = 1u << 4,  ///< run the animation backwards
	/// pos/pos2 are screen pixels, not world units: skip the view-projection and
	/// go straight to clip space. Screen-space work is always drawn last and
	/// without depth, so overlays land on top of everything.
	FxFlag_ScreenSpace = 1u << 5,
};

/// One effect instance. Mirrors the HLSL struct exactly - keep them in step.
struct FxInstance
{
	float    pos[3];        ///< world anchor, MQ/TLO order (x, y, z up)
	float    radius;

	float    height;
	float    phase;         ///< 0..1, per-spawn so neighbours don't pulse in lockstep
	float    speed;         ///< animation rate multiplier
	float    tilt;          ///< radians, for forms that lean

	float    rgba[4];

	uint32_t form;          ///< FxForm
	uint32_t segments;      ///< segments wanted (clamped to kMaxSegsPerInstance)
	uint32_t flags;         ///< FxFlag bits
	float    width;         ///< ribbon half-width

	float    p0, p1, p2, p3; ///< form-specific (turns, ring count, sides, ...)

	/// Second world point. Only FxForm_Segment reads it, but keeping it in the
	/// common record avoids a second buffer and a second draw call.
	float    pos2[3];
	float    pad;
};

/// Upper bound on segments one instance may generate. The draw always issues
/// this many vertices per instance and the shader collapses the unused tail to
/// a degenerate point - cheaper and simpler than variable vertex counts.
inline constexpr uint32_t kMaxSegsPerInstance = 128;
inline constexpr uint32_t kMaxInstances       = 4096;

class LineFX
{
public:
	~LineFX();

	/// Create device-dependent objects. Safe to call every frame; cheap once it
	/// has succeeded. False means the device isn't ready or a step failed - see
	/// LastError().
	bool EnsureInit(ID3D11Device* device);

	/// Drop everything device-dependent (device reset / plugin shutdown).
	void ReleaseAll();

	/// Upload `instances` and draw them. `viewProj` is row-major and already in
	/// the engine's coordinate convention. camRight/camUp orient the ribbons.
	/// Render-thread only.
	void Render(const FxInstance* instances, size_t count,
		const float viewProj[16], const float camPos[3],
		const float camRight[3], const float camUp[3],
		float timeSec, float viewportW, float viewportH);

	uint32_t    LastSegmentCount() const { return m_lastSegments; }
	uint32_t    LastInstanceCount() const { return m_lastInstances; }
	bool        Ready() const { return m_ready; }
	const char* LastError() const { return m_error; }

private:
	bool CreateShaders();
	bool CreateBuffers();
	bool CreateStates();

	/// Instances sorted into draw buckets. One additive effect must not turn
	/// every other effect additive, and screen-space overlays must not depth
	/// test against the world, so state cannot be global to the frame - the
	/// array is partitioned and each run drawn with its own state.
	std::vector<FxInstance> m_sorted;

	ID3D11Device*             m_device = nullptr;   ///< borrowed (gpD3D11Device)
	ID3D11DeviceContext*      m_ctx = nullptr;      ///< borrowed immediate context
	ID3D11VertexShader*       m_vs = nullptr;
	ID3D11PixelShader*        m_ps = nullptr;
	ID3D11Buffer*             m_cb = nullptr;
	ID3D11Buffer*             m_instBuf = nullptr;  ///< dynamic structured buffer
	ID3D11ShaderResourceView* m_instSrv = nullptr;
	ID3D11BlendState*         m_blendAlpha = nullptr;
	ID3D11BlendState*         m_blendAdd = nullptr;
	ID3D11DepthStencilState*  m_dssTest = nullptr;  ///< test, no write
	ID3D11DepthStencilState*  m_dssNone = nullptr;
	ID3D11RasterizerState*    m_raster = nullptr;

	bool        m_ready = false;
	const char* m_error = "";
	uint32_t    m_lastSegments = 0;
	uint32_t    m_lastInstances = 0;
};

} // namespace eqgfx::gpu
