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
struct ID3D11BlendState;
struct ID3D11SamplerState;

namespace eqgfx::render {

class ModelView
{
public:
	~ModelView();

	bool EnsureInit(ID3D11Device* device, int width, int height);

	bool SetModel(const eqg::Model& model, const std::vector<dds::Image>& textures);
	bool HasModel() const { return m_indexCount > 0; }

	void UpdatePose(const std::vector<eqg::Vertex>& worldVertices);

	void RenderFrame(float yawRadians, float pitchRadians,
		float zoom = 1.0f, float panX = 0.0f, float panY = 0.0f,
		float timeSec = 0.0f, bool breathe = true,
		float breathRate = 3.2f, float breathAmp = 1.0f);

	void* TextureId() const { return m_srv; }
	int   Width() const { return m_width; }
	int   Height() const { return m_height; }

	void ReleaseAll();

private:
	bool CreateTargets(int width, int height);
	bool CreateShaders();

	ID3D11Device*             m_device = nullptr;
	ID3D11DeviceContext*      m_ctx = nullptr;
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
	ID3D11BlendState*         m_blend = nullptr;
	ID3D11SamplerState*       m_sampler = nullptr;

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
	void ReleaseShaders();
	void ReleaseTargets();
	ID3D11ShaderResourceView* CreateTextureSrv(const dds::Image& img) const;
};

} // namespace eqgfx::render
