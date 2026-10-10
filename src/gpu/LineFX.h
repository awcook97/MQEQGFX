#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <thread>
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

enum FxForm : uint32_t
{
	FxForm_Ring = 0,
	FxForm_Helix,
	FxForm_Orbit,
	FxForm_Polygon,
	FxForm_Bloom,
	FxForm_Spire,
	FxForm_Lattice,
	FxForm_Segment,
	FxForm_Rect,

	FxForm_Flame,
	FxForm_Vines,
	FxForm_Thorns,
	FxForm_Glyph,
	FxForm_Wings,
	FxForm_Lightning,
	FxForm_Bolt,
	FxForm_Rain,
	FxForm_Sparkle,
	FxForm_Rune,
	FxForm_Vortex,
	FxForm_Dome,
	FxForm_Crystal,
	FxForm_Drain,
	FxForm_Bubbles,
	FxForm_Wind,
	FxForm_Burst,
	FxForm_Cracks,
	FxForm_Count
};

enum FxFlag : uint32_t
{
	FxFlag_Comet       = 1u << 0,
	FxFlag_NoDepth     = 1u << 1,
	FxFlag_ScreenWidth = 1u << 2,
	FxFlag_Additive    = 1u << 3,
	FxFlag_Reverse     = 1u << 4,
	FxFlag_ScreenSpace = 1u << 5,
	FxFlag_NoEnvelope  = 1u << 6,
	FxFlag_Drift       = 1u << 7,
};

enum GlyphAnim : uint32_t
{
	GlyphAnim_Rise    = 1u << 0,
	GlyphAnim_Bob     = 1u << 1,
	GlyphAnim_Spin    = 1u << 2,
	GlyphAnim_Pop     = 1u << 3,
	GlyphAnim_Wiggle  = 1u << 4,
	GlyphAnim_Ascend  = 1u << 5,
	GlyphAnim_Descend = 1u << 6,
	GlyphAnim_Roll    = 1u << 7,
};

struct FxInstance
{
	float    pos[3];
	float    radius;

	float    height;
	float    phase;
	float    speed;
	float    tilt;

	float    rgba[4];

	uint32_t form;
	uint32_t segments;
	uint32_t flags;
	float    width;

	float    p0, p1, p2, p3;

	float    pos2[3];
	float    yaw;

	float    rgba2[4];

	float    birth;
	float    life;
	float    p4;
	float    p5;
};
static_assert(sizeof(FxInstance) == 128, "FxInstance must match the HLSL struct");

inline constexpr uint32_t kMaxSegsPerInstance = 128;
inline constexpr uint32_t kMaxInstances       = 4096;

class LineFX
{
public:
	~LineFX();

	bool EnsureInit(ID3D11Device* device);

	void ReleaseAll();
	void ReleaseDeviceObjects();

	void SetShaderCacheFile(std::string path) { m_cacheFile = std::move(path); }

	void Render(const FxInstance* instances, size_t count,
		const float viewProj[16], const float camPos[3],
		const float camRight[3], const float camUp[3],
		double timeSec, float viewportW, float viewportH);

	uint32_t    LastSegmentCount() const { return m_lastSegments; }
	uint32_t    LastInstanceCount() const { return m_lastInstances; }
	bool        Ready() const { return m_ready; }
	const char* LastError() const { return m_error; }

private:
	bool CreateShaders();
	void StartCompile();
	bool LoadCache(uint64_t hash);
	void SaveCache(uint64_t hash) const;
	bool CreateBuffers();
	bool EnsureInstanceCapacity(uint32_t count);
	bool CreateStates();

	ID3D11Device*             m_device = nullptr;
	ID3D11DeviceContext*      m_ctx = nullptr;
	ID3D11VertexShader*       m_vs = nullptr;
	ID3D11PixelShader*        m_ps = nullptr;
	ID3D11Buffer*             m_cb = nullptr;
	ID3D11Buffer*             m_instBuf = nullptr;
	ID3D11ShaderResourceView* m_instSrv = nullptr;
	uint32_t                  m_instCap = 0;
	ID3D11BlendState*         m_blendAlpha = nullptr;
	ID3D11BlendState*         m_blendAdd = nullptr;
	ID3D11DepthStencilState*  m_dssTest = nullptr;
	ID3D11DepthStencilState*  m_dssNone = nullptr;
	ID3D11RasterizerState*    m_raster = nullptr;

	std::thread       m_compileThread;
	std::atomic<bool> m_compileDone{ false };
	bool              m_compileStarted = false;
	uint64_t          m_sourceHash = 0;
	std::string       m_vsCode;
	std::string       m_psCode;
	std::string       m_compileError;
	std::string       m_cacheFile;

	bool        m_ready = false;
	const char* m_error = "";
	char        m_errorBuf[256] = {};
	uint32_t    m_lastSegments = 0;
	uint32_t    m_lastInstances = 0;
};

} // namespace eqgfx::gpu
