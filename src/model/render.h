// Standalone DX11 model renderer: uploads a parsed eqg::Model and renders it into
// an offscreen texture (reusing MQ's ID3D11Device, gpD3D11Device), exposing an
// ID3D11ShaderResourceView for ImGui.Image. Flat/normal shading for now (no
// textures yet). All DX calls must run on the render thread.
#pragma once

#include "eqg.h"
#include "dds.h"
#include <cstdint>
#include <vector>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11SamplerState;

namespace eqgfx::render {

class ModelView
{
public:
	~ModelView();

	// Lazily create device-dependent objects (shaders, RT) using MQ's D3D11
	// device. Returns false if the device isn't available. Safe to call per frame.
	bool EnsureInit(ID3D11Device* device, int width, int height);

	// Upload a parsed model's geometry + per-material diffuse textures (parallel to
	// model.materials; invalid/empty entries render untextured white). Replaces any
	// previous model.
	bool SetModel(const eqg::Model& model, const std::vector<dds::Image>& textures);
	bool HasModel() const { return m_indexCount > 0; }

	// Rewrite just the vertex positions/normals (a re-skinned pose) in the existing
	// dynamic VB - same vertex count, textures/indices/draws unchanged. Keeps the
	// camera framing fixed so the model doesn't jump around while posing.
	void UpdatePose(const std::vector<eqg::Vertex>& worldVertices);

	// Render the current model into the offscreen RT. yaw/pitch orbit the camera;
	// zoom scales the framing distance (1 = fit, >1 = closer); panX/panY shift the
	// look target in the view plane (units of the model radius). timeSec drives the
	// procedural breathing idle; breathe=false freezes it at the bind pose. breathRate
	// = angular speed (rad/s), breathAmp = depth multiplier.
	void RenderFrame(float yawRadians, float pitchRadians,
		float zoom = 1.0f, float panX = 0.0f, float panY = 0.0f,
		float timeSec = 0.0f, bool breathe = true,
		float breathRate = 3.2f, float breathAmp = 1.0f);

	// ID3D11ShaderResourceView* (as a void*/ImTextureID) for ImGui.Image.
	void* TextureId() const { return m_srv; }
	int   Width() const { return m_width; }
	int   Height() const { return m_height; }

	void ReleaseAll();   // drop everything (device reset / shutdown)

private:
	bool CreateTargets(int width, int height);
	bool CreateShaders();

	ID3D11Device*             m_device = nullptr;   // borrowed (gpD3D11Device)
	ID3D11DeviceContext*      m_ctx = nullptr;      // borrowed (immediate context)
	ID3D11Texture2D*          m_colorTex = nullptr;
	ID3D11RenderTargetView*   m_rtv = nullptr;
	ID3D11ShaderResourceView* m_srv = nullptr;
	ID3D11Texture2D*          m_depthTex = nullptr;
	ID3D11DepthStencilView*   m_dsv = nullptr;
	ID3D11VertexShader*       m_vs = nullptr;
	ID3D11PixelShader*        m_ps = nullptr;
	ID3D11InputLayout*        m_layout = nullptr;
	ID3D11Buffer*             m_cb = nullptr;
	ID3D11Buffer*             m_vb = nullptr;
	ID3D11Buffer*             m_ib = nullptr;
	ID3D11RasterizerState*    m_raster = nullptr;
	ID3D11DepthStencilState*  m_dss = nullptr;
	ID3D11SamplerState*       m_sampler = nullptr;

	// Per-material diffuse SRVs (parallel to the model's materials) + a 1x1 white
	// fallback, and the submesh draw ranges that select among them.
	std::vector<ID3D11ShaderResourceView*> m_texSrvs;
	ID3D11Texture2D*          m_whiteTex = nullptr;
	ID3D11ShaderResourceView* m_whiteSrv = nullptr;
	struct Draw { uint32_t firstIndex; uint32_t indexCount; int materialIndex; uint32_t tint; };
	std::vector<Draw>         m_draws;

	int      m_width = 0, m_height = 0;
	uint32_t m_indexCount = 0;
	uint32_t m_vertexCount = 0;
	float    m_center[3] = { 0, 0, 0 };
	float    m_radius = 1.0f;

	void ReleaseTextures();
	ID3D11ShaderResourceView* CreateTextureSrv(const dds::Image& img) const;
};

} // namespace eqgfx::render
