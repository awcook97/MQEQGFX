#include "render.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <cmath>
#include <cstring>

using namespace DirectX;

namespace eqgfx::render {

namespace {

struct CB { XMFLOAT4X4 mvp; XMFLOAT4 lightDir; XMFLOAT4 tint; XMFLOAT4 anim; XMFLOAT4 anim2; };

template <class T> void Rel(T*& p) { if (p) { p->Release(); p = nullptr; } }

// Textured diffuse × soft normal shading. Untextured materials bind a 1x1 white
// texture, so this reduces to the old flat-gray look for them. The VS adds a
// procedural BREATHING idle (anim = {timeSec, centerZ, radius, enabled}): the torso
// band expands along its horizontal normals on a slow sine, with a gentle vertical
// bob and micro sway so a static bind-pose reads as alive.
const char* kShader = R"HLSL(
cbuffer CB : register(b0) { float4x4 mvp; float4 lightDir; float4 tint; float4 anim; float4 anim2; };
Texture2D    gTex : register(t0);
SamplerState gSmp : register(s0);
struct VSIn  { float3 pos:POSITION; float3 nrm:NORMAL; float2 uv:TEXCOORD0; };
struct VSOut { float4 pos:SV_POSITION; float3 nrm:NORMAL; float2 uv:TEXCOORD0; };
VSOut VSMain(VSIn i) {
    VSOut o;
    float3 p = i.pos;
    if (anim.w > 0.5f) {
        float t = anim.x, cz = anim.y, rad = max(anim.z, 0.001f);
        float rate = anim2.x, amp = anim2.y;                 // user speed / depth
        float breath = sin(t * rate);
        float h = (p.z - cz) / rad;                          // height, ~[-1,1]
        float chest = saturate(1.0f - abs(h - 0.30f) / 0.45f); // torso band, peak upper chest
        float3 nh = float3(i.nrm.x, i.nrm.y, 0.0f);          // expand horizontally only
        float nl = length(nh); if (nl > 1e-4f) nh /= nl;
        p += nh * (chest * (breath * 0.5f + 0.5f) * rad * 0.022f * amp);
        p.z += breath * rad * 0.008f * amp;                  // gentle bob
        p.x += sin(t * rate * 0.5f) * rad * 0.004f * amp;    // micro sway
    }
    o.pos = mul(float4(p, 1.0f), mvp); o.nrm = i.nrm; o.uv = i.uv; return o;
}
float4 PSMain(VSOut i) : SV_TARGET {
    float d = saturate(dot(normalize(i.nrm), normalize(lightDir.xyz))) * 0.5f + 0.5f;
    float3 c = gTex.Sample(gSmp, i.uv).rgb * tint.rgb;
    return float4(c * d, 1.0f);
}
)HLSL";

} // namespace

ModelView::~ModelView() { ReleaseAll(); }

void ModelView::ReleaseTextures()
{
	for (auto*& s : m_texSrvs) Rel(s);
	m_texSrvs.clear();
	m_draws.clear();
}

void ModelView::ReleaseAll()
{
	ReleaseTextures();
	Rel(m_whiteSrv); Rel(m_whiteTex); Rel(m_sampler);
	Rel(m_ib); Rel(m_vb); Rel(m_cb);
	Rel(m_layout); Rel(m_ps); Rel(m_vs);
	Rel(m_dss); Rel(m_raster);
	Rel(m_dsv); Rel(m_depthTex);
	Rel(m_srv); Rel(m_rtv); Rel(m_colorTex);
	if (m_ctx) { m_ctx->Release(); m_ctx = nullptr; }
	m_device = nullptr;
	m_indexCount = 0;
	m_width = m_height = 0;
}

bool ModelView::CreateTargets(int w, int h)
{
	Rel(m_srv); Rel(m_rtv); Rel(m_colorTex);
	Rel(m_dsv); Rel(m_depthTex);

	D3D11_TEXTURE2D_DESC td = {};
	td.Width = w; td.Height = h; td.MipLevels = 1; td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM; td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (FAILED(m_device->CreateTexture2D(&td, nullptr, &m_colorTex))) return false;
	if (FAILED(m_device->CreateRenderTargetView(m_colorTex, nullptr, &m_rtv))) return false;
	if (FAILED(m_device->CreateShaderResourceView(m_colorTex, nullptr, &m_srv))) return false;

	D3D11_TEXTURE2D_DESC dd = td;
	dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	if (FAILED(m_device->CreateTexture2D(&dd, nullptr, &m_depthTex))) return false;
	if (FAILED(m_device->CreateDepthStencilView(m_depthTex, nullptr, &m_dsv))) return false;

	m_width = w; m_height = h;
	return true;
}

bool ModelView::CreateShaders()
{
	ID3DBlob* vsb = nullptr; ID3DBlob* psb = nullptr; ID3DBlob* err = nullptr;
	if (FAILED(D3DCompile(kShader, std::strlen(kShader), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsb, &err)))
	{ if (err) err->Release(); return false; }
	if (FAILED(D3DCompile(kShader, std::strlen(kShader), nullptr, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psb, &err)))
	{ if (err) err->Release(); vsb->Release(); return false; }

	bool ok = SUCCEEDED(m_device->CreateVertexShader(vsb->GetBufferPointer(), vsb->GetBufferSize(), nullptr, &m_vs))
		&& SUCCEEDED(m_device->CreatePixelShader(psb->GetBufferPointer(), psb->GetBufferSize(), nullptr, &m_ps));
	if (ok)
	{
		D3D11_INPUT_ELEMENT_DESC il[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		ok = SUCCEEDED(m_device->CreateInputLayout(il, 3, vsb->GetBufferPointer(), vsb->GetBufferSize(), &m_layout));
	}
	vsb->Release(); psb->Release();
	if (!ok) return false;

	D3D11_BUFFER_DESC cbd = {};
	cbd.ByteWidth = sizeof(CB); cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER; cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(m_device->CreateBuffer(&cbd, nullptr, &m_cb))) return false;

	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID; rd.CullMode = D3D11_CULL_NONE; rd.DepthClipEnable = TRUE;
	if (FAILED(m_device->CreateRasterizerState(&rd, &m_raster))) return false;

	D3D11_DEPTH_STENCIL_DESC dsd = {};
	dsd.DepthEnable = TRUE; dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; dsd.DepthFunc = D3D11_COMPARISON_LESS;
	if (FAILED(m_device->CreateDepthStencilState(&dsd, &m_dss))) return false;

	// Wrap sampler - EQ UVs run outside [0,1] and tile.
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	if (FAILED(m_device->CreateSamplerState(&sd, &m_sampler))) return false;

	// 1x1 white fallback for untextured materials.
	const uint32_t white = 0xFFFFFFFFu;
	D3D11_TEXTURE2D_DESC wd = {};
	wd.Width = wd.Height = 1; wd.MipLevels = 1; wd.ArraySize = 1;
	wd.Format = DXGI_FORMAT_R8G8B8A8_UNORM; wd.SampleDesc.Count = 1;
	wd.Usage = D3D11_USAGE_IMMUTABLE; wd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA wsd = {}; wsd.pSysMem = &white; wsd.SysMemPitch = 4;
	if (FAILED(m_device->CreateTexture2D(&wd, &wsd, &m_whiteTex))) return false;
	if (FAILED(m_device->CreateShaderResourceView(m_whiteTex, nullptr, &m_whiteSrv))) return false;
	return true;
}

ID3D11ShaderResourceView* ModelView::CreateTextureSrv(const dds::Image& img) const
{
	if (!m_device || !img.valid()) return nullptr;
	DXGI_FORMAT fmt;
	switch (img.format)
	{
	case dds::Image::BC1:   fmt = DXGI_FORMAT_BC1_UNORM; break;
	case dds::Image::BC2:   fmt = DXGI_FORMAT_BC2_UNORM; break;
	case dds::Image::BC3:   fmt = DXGI_FORMAT_BC3_UNORM; break;
	case dds::Image::RGBA8: fmt = DXGI_FORMAT_R8G8B8A8_UNORM; break;
	default: return nullptr;
	}
	D3D11_TEXTURE2D_DESC td = {};
	td.Width = img.width; td.Height = img.height; td.MipLevels = 1; td.ArraySize = 1;
	td.Format = fmt; td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_IMMUTABLE; td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = img.pixels.data(); sd.SysMemPitch = img.rowPitch;
	ID3D11Texture2D* tex = nullptr;
	if (FAILED(m_device->CreateTexture2D(&td, &sd, &tex))) return nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	HRESULT hr = m_device->CreateShaderResourceView(tex, nullptr, &srv);
	tex->Release();
	return SUCCEEDED(hr) ? srv : nullptr;
}

bool ModelView::EnsureInit(ID3D11Device* device, int width, int height)
{
	if (!device) return false;
	if (m_device && m_device != device) ReleaseAll();
	m_device = device;
	if (!m_ctx) m_device->GetImmediateContext(&m_ctx);
	if (!m_vs && !CreateShaders()) return false;
	if ((!m_colorTex || m_width != width || m_height != height) && !CreateTargets(width, height)) return false;
	return m_vs && m_colorTex;
}

bool ModelView::SetModel(const eqg::Model& model, const std::vector<dds::Image>& textures)
{
	Rel(m_vb); Rel(m_ib);
	ReleaseTextures();
	m_indexCount = 0;
	if (!m_device || model.vertices.empty() || model.indices.empty()) return false;

	// One SRV per material (null => white fallback at draw time).
	m_texSrvs.assign(model.materials.size(), nullptr);
	for (size_t i = 0; i < model.materials.size() && i < textures.size(); ++i)
		m_texSrvs[i] = CreateTextureSrv(textures[i]);

	// Draw ranges: one per submesh, selecting its material's texture + tint.
	m_draws.reserve(model.submeshes.size());
	for (const eqg::SubMesh& sm : model.submeshes)
		if (sm.indexCount > 0)
		{
			uint32_t tint = 0xFFFFFFFF;
			if (sm.materialIndex >= 0 && static_cast<size_t>(sm.materialIndex) < model.materials.size())
				tint = model.materials[sm.materialIndex].tint;
			m_draws.push_back({ sm.firstIndex, sm.indexCount, sm.materialIndex, tint });
		}

	// Dynamic VB so posing can rewrite vertices in place (UpdatePose) without a full
	// SetModel (which would re-upload every texture).
	D3D11_BUFFER_DESC vd = {};
	vd.ByteWidth = static_cast<UINT>(model.vertices.size() * sizeof(eqg::Vertex));
	vd.Usage = D3D11_USAGE_DYNAMIC; vd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA vsd = {}; vsd.pSysMem = model.vertices.data();
	if (FAILED(m_device->CreateBuffer(&vd, &vsd, &m_vb))) return false;
	m_vertexCount = static_cast<uint32_t>(model.vertices.size());

	D3D11_BUFFER_DESC id = {};
	id.ByteWidth = static_cast<UINT>(model.indices.size() * sizeof(uint32_t));
	id.Usage = D3D11_USAGE_IMMUTABLE; id.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA isd = {}; isd.pSysMem = model.indices.data();
	if (FAILED(m_device->CreateBuffer(&id, &isd, &m_ib))) { Rel(m_vb); return false; }

	m_indexCount = static_cast<uint32_t>(model.indices.size());
	for (int i = 0; i < 3; ++i) m_center[i] = (model.minB[i] + model.maxB[i]) * 0.5f;
	float r = 0;
	for (int i = 0; i < 3; ++i) { float e = (model.maxB[i] - model.minB[i]) * 0.5f; if (e > r) r = e; }
	m_radius = r > 0 ? r : 1.0f;
	return true;
}

void ModelView::UpdatePose(const std::vector<eqg::Vertex>& verts)
{
	if (!m_ctx || !m_vb || verts.size() != m_vertexCount || verts.empty()) return;
	D3D11_MAPPED_SUBRESOURCE ms;
	if (SUCCEEDED(m_ctx->Map(m_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms)))
	{
		std::memcpy(ms.pData, verts.data(), verts.size() * sizeof(eqg::Vertex));
		m_ctx->Unmap(m_vb, 0);
	}
}

void ModelView::RenderFrame(float yaw, float pitch, float zoom, float panX, float panY,
	float timeSec, bool breathe, float breathRate, float breathAmp)
{
	if (!m_device || !m_ctx || !m_rtv || m_indexCount == 0) return;

	// Save the game's render target + viewport so we don't leave ours bound.
	ID3D11RenderTargetView* oldRTV = nullptr;
	ID3D11DepthStencilView* oldDSV = nullptr;
	m_ctx->OMGetRenderTargets(1, &oldRTV, &oldDSV);
	UINT numVP = 1;
	D3D11_VIEWPORT oldVP = {};
	m_ctx->RSGetViewports(&numVP, &oldVP);

	const float clear[4] = { 0.10f, 0.10f, 0.12f, 0.0f };
	m_ctx->ClearRenderTargetView(m_rtv, clear);
	m_ctx->ClearDepthStencilView(m_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_ctx->OMSetRenderTargets(1, &m_rtv, m_dsv);
	D3D11_VIEWPORT vp = { 0, 0, static_cast<float>(m_width), static_cast<float>(m_height), 0, 1 };
	m_ctx->RSSetViewports(1, &vp);

	// Orbit camera around the model center (EQ is Z-up). Frame the model's bounding
	// sphere in whichever FOV is narrower - for a portrait window the horizontal FOV
	// is tighter, so distance from the vertical FOV alone would clip wide poses
	// (e.g. a T-pose character's arms).
	const float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
	const float vHalf = XM_PIDIV4 * 0.5f;
	const float hHalf = std::atan(aspect * std::tan(vHalf));
	const float minHalf = (hHalf < vHalf) ? hHalf : vHalf;
	float baseDist = m_radius / std::sin(minHalf) * 1.1f;   // +10% margin (fit)
	float dist = baseDist / (zoom > 0.05f ? zoom : 0.05f);
	XMVECTOR worldUp = XMVectorSet(0, 0, 1, 0);
	float cp = std::cos(pitch), sp = std::sin(pitch);
	XMVECTOR center0 = XMVectorSet(m_center[0], m_center[1], m_center[2], 1);
	XMVECTOR orbit = XMVectorSet(dist * cp * std::cos(yaw), dist * cp * std::sin(yaw), dist * sp, 0);
	XMVECTOR eye0 = XMVectorAdd(center0, orbit);
	// Pan the look target in the view plane (right = up x forward, up = forward x right).
	XMVECTOR fwd = XMVector3Normalize(XMVectorSubtract(center0, eye0));
	XMVECTOR right = XMVector3Normalize(XMVector3Cross(worldUp, fwd));
	XMVECTOR camUp = XMVector3Cross(fwd, right);
	XMVECTOR pan = XMVectorAdd(XMVectorScale(right, panX * m_radius), XMVectorScale(camUp, panY * m_radius));
	XMVECTOR center = XMVectorAdd(center0, pan);
	XMVECTOR eye = XMVectorAdd(eye0, pan);
	XMMATRIX view = XMMatrixLookAtLH(eye, center, worldUp);
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.05f, dist + m_radius * 8);
	XMMATRIX mvp = XMMatrixTranspose(XMMatrixMultiply(view, proj));

	CB cbBase;
	XMStoreFloat4x4(&cbBase.mvp, mvp);
	cbBase.lightDir = XMFLOAT4(0.3f, 0.5f, 0.8f, 0);
	cbBase.tint = XMFLOAT4(1, 1, 1, 1);
	cbBase.anim = XMFLOAT4(timeSec, m_center[2], m_radius, breathe ? 1.0f : 0.0f);
	cbBase.anim2 = XMFLOAT4(breathRate, breathAmp, 0.0f, 0.0f);

	auto uploadCB = [&](uint32_t tint) {
		cbBase.tint = XMFLOAT4(((tint >> 16) & 0xFF) / 255.0f, ((tint >> 8) & 0xFF) / 255.0f,
			(tint & 0xFF) / 255.0f, 1.0f);
		D3D11_MAPPED_SUBRESOURCE ms;
		if (SUCCEEDED(m_ctx->Map(m_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms)))
		{
			std::memcpy(ms.pData, &cbBase, sizeof(cbBase));
			m_ctx->Unmap(m_cb, 0);
		}
	};

	UINT stride = sizeof(eqg::Vertex), off = 0;
	m_ctx->IASetInputLayout(m_layout);
	m_ctx->IASetVertexBuffers(0, 1, &m_vb, &stride, &off);
	m_ctx->IASetIndexBuffer(m_ib, DXGI_FORMAT_R32_UINT, 0);
	m_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ctx->VSSetShader(m_vs, nullptr, 0);
	m_ctx->VSSetConstantBuffers(0, 1, &m_cb);
	m_ctx->PSSetShader(m_ps, nullptr, 0);
	m_ctx->PSSetConstantBuffers(0, 1, &m_cb);
	m_ctx->PSSetSamplers(0, 1, &m_sampler);
	m_ctx->RSSetState(m_raster);
	m_ctx->OMSetDepthStencilState(m_dss, 0);

	// Draw each submesh with its material's diffuse texture + dye tint (white if none).
	if (m_draws.empty())
	{
		uploadCB(0xFFFFFFFF);
		m_ctx->PSSetShaderResources(0, 1, &m_whiteSrv);
		m_ctx->DrawIndexed(m_indexCount, 0, 0);
	}
	else for (const Draw& d : m_draws)
	{
		ID3D11ShaderResourceView* srv = m_whiteSrv;
		if (d.materialIndex >= 0 && static_cast<size_t>(d.materialIndex) < m_texSrvs.size() && m_texSrvs[d.materialIndex])
			srv = m_texSrvs[d.materialIndex];
		uploadCB(d.tint);
		m_ctx->PSSetShaderResources(0, 1, &srv);
		m_ctx->DrawIndexed(d.indexCount, d.firstIndex, 0);
	}

	// Restore the game's targets/viewport (rest of the pipeline gets reset by the
	// next draw's own setup / ImGui's backup-restore).
	m_ctx->OMSetRenderTargets(1, &oldRTV, oldDSV);
	if (numVP) m_ctx->RSSetViewports(1, &oldVP);
	if (oldRTV) oldRTV->Release();
	if (oldDSV) oldDSV->Release();
}

} // namespace eqgfx::render
