#include "../MQEQGFX.h"

#include "../PatternScan.h"
#include "../Signatures.h"

#include "eqlib/graphics/GraphicsEngine.h"
#include "eqlib/graphics/GraphicsResources.h"
#include "eqlib/graphics/Render.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

using fnSetStreamSource = int (*)(CRenderStateCache* cache, uint32_t stream, Direct3DVertexBuffer9* vb,
	uint32_t offset, uint32_t stride);
using fnSetIndices      = int (*)(CRenderStateCache* cache, Direct3DIndexBuffer9* ib);
using fnSetFVF          = int (*)(CRenderStateCache* cache, uint32_t fvf);
using fnSetTexture      = int (*)(CRenderStateCache* cache, uint32_t stage, Direct3DTexture9* tex);
using fnFlush           = void (*)(CRenderStateCache* cache);
using fnGetFont         = CCachedFont* (*)(CFontManager* fontMgr, uint32_t id);
using fnCreateVB        = int (*)(CRender* render, uint32_t bytes, uint32_t usage, uint32_t fvf,
	uint32_t pool, Direct3DVertexBuffer9** out, void* handle, int tag);
using fnReleaseVB       = void (*)(CRender* render, Direct3DVertexBuffer9* vb, int tag);
using fnTextVfn         = void (*)(CTextObjectBase* obj);
using fnBuildGlyphs     = void (*)(CTextObjectBase* obj, FONT_D3DTLVERTEX* verts);

struct Api
{
	fnSetStreamSource     setStream = nullptr;
	fnSetIndices          setIndices = nullptr;
	fnSetFVF              setFVF = nullptr;
	fnSetTexture          setTexture = nullptr;
	fnFlush               flush = nullptr;
	fnGetFont             getFont = nullptr;
	fnCreateVB            createVB = nullptr;
	fnReleaseVB           releaseVB = nullptr;
	fnBuildGlyphs         buildGlyphs = nullptr;
	CFontManager**        fontMgrSlot = nullptr;
	CRender**             renderSlot = nullptr;
	C2DPrimitiveManager*  mgr = nullptr;
	uintptr_t*            vtable = nullptr;
	fnTextVfn             origPrepare = nullptr;
	fnTextVfn             origRender = nullptr;

	uintptr_t*            uiVtable = nullptr;
	fnBuildGlyphs         origUiBuild = nullptr;
};

constexpr size_t kPrepareVBSlot   = 0x28 / sizeof(uintptr_t);
constexpr size_t kRenderSlot      = 0x30 / sizeof(uintptr_t);
constexpr size_t kBuildGlyphsSlot = 0x38 / sizeof(uintptr_t);

constexpr uint32_t kFVF             = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;
constexpr uint32_t kVertsPerGlyph   = 4;
constexpr uint32_t kGlyphBytes      = kVertsPerGlyph * sizeof(FONT_D3DTLVERTEX);
constexpr uint32_t kMinBuildGlyphs  = 10;
constexpr uint32_t kMaxQuadsPerDraw = 10922;
constexpr uint32_t kVBUsage         = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
constexpr int      kVBTag           = 7;
constexpr uint32_t kPrimBatchHead   = 0x80000000;

static_assert(sizeof(FONT_D3DTLVERTEX) == 0x20);
static_assert(kFVF == 0x1C4);

Api  s_api;
bool s_resolved = false;
bool s_resolveFailed = false;
bool s_installed = false;
bool s_installFailed = false;
std::atomic<bool> s_enabled{ true };
const char* s_failReason = "";

constexpr const char* kUnsupported = "not supported on this client version";
constexpr const char* kConflict    = "conflicts with another plugin";

void Unsupported(const char* detail)
{
	DebugSpewAlwaysFile("MQEQGFX: nametag batching: %s", detail);
	s_failReason = kUnsupported;
}

void Conflict(const char* detail)
{
	DebugSpewAlwaysFile("MQEQGFX: nametag batching: %s", detail);
	s_failReason = kConflict;
}

std::atomic<uint32_t> s_statTags{ 0 };
std::atomic<uint32_t> s_statDraws{ 0 };
std::atomic<uint32_t> s_statRebuilds{ 0 };

CRender* Render()                 { return s_api.renderSlot ? *s_api.renderSlot : nullptr; }
CFontManager* FontManager()       { return s_api.fontMgrSlot ? *s_api.fontMgrSlot : nullptr; }
bool HasVtable(const CTextObjectBase* obj, const uintptr_t* vtable)
{
	return *reinterpret_cast<const uintptr_t* const*>(obj) == vtable;
}

using eqgfx::scan::Module;
using eqgfx::scan::Rip;
namespace sig = eqgfx::sig;
inline uintptr_t Scan(const Module& m, const char* sig) { return eqgfx::scan::Find(m, sig); }

uintptr_t* FindTextVtable(const Module& m, uintptr_t build, uintptr_t prepare, uintptr_t render)
{
	for (uintptr_t p = m.base; p + (kBuildGlyphsSlot + 1) * sizeof(uintptr_t) <= m.base + m.size; p += sizeof(uintptr_t))
	{
		if (*reinterpret_cast<const uintptr_t*>(p) != build)
			continue;
		uintptr_t* vt = reinterpret_cast<uintptr_t*>(p) - kBuildGlyphsSlot;
		if (vt[kPrepareVBSlot] == prepare && vt[kRenderSlot] == render)
			return vt;
	}
	return nullptr;
}

bool Resolve()
{
	if (s_resolved) return true;
	if (s_resolveFailed) return false;

	const Module m = eqgfx::scan::Graphics();
	auto fail = [](const char* why) { Unsupported(why); s_resolveFailed = true; return false; };
	if (!m.base || !m.size)
		return fail("EQGraphics not loaded");

	const uintptr_t flush     = Scan(m, sig::CRenderStateCache__FlushToDevice);
	const uintptr_t setStream = Scan(m, sig::CRenderStateCache__SetStreamSource);
	const uintptr_t setTex    = Scan(m, sig::CRenderStateCache__SetTexture);
	const uintptr_t setFVF    = Scan(m, sig::CRenderStateCache__SetFVF);
	const uintptr_t setIdx    = Scan(m, sig::CRenderStateCache__SetIndices);
	const uintptr_t getFont   = Scan(m, sig::CFontManager__GetFont);
	const uintptr_t createVB  = Scan(m, sig::CRender__EQG_CreateVertexBuffer);
	const uintptr_t releaseVB = Scan(m, sig::CRender__EQG_ReleaseVertexBuffer);
	const uintptr_t build     = Scan(m, sig::CStringSpriteText__BuildGlyphs);
	const uintptr_t prepare   = Scan(m, sig::CTextObjectBase__PrepareVB);
	const uintptr_t render    = Scan(m, sig::CTextObjectBase__Render);
	const uintptr_t drawPage  = Scan(m, sig::CCachedFont__DrawPageRun);
	const uintptr_t instance  = Scan(m, sig::C2DPrimitiveManager__Instance);
	if (!flush || !setStream || !setTex || !setFVF || !setIdx || !getFont || !createVB || !releaseVB
		|| !build || !prepare || !render || !drawPage || !instance)
		return fail("nametag batching is unavailable on this client");

	s_api.flush       = reinterpret_cast<fnFlush>(flush);
	s_api.setStream   = reinterpret_cast<fnSetStreamSource>(setStream);
	s_api.setTexture  = reinterpret_cast<fnSetTexture>(setTex);
	s_api.setFVF      = reinterpret_cast<fnSetFVF>(setFVF);
	s_api.setIndices  = reinterpret_cast<fnSetIndices>(setIdx);
	s_api.getFont     = reinterpret_cast<fnGetFont>(getFont);
	s_api.createVB    = reinterpret_cast<fnCreateVB>(createVB);
	s_api.releaseVB   = reinterpret_cast<fnReleaseVB>(releaseVB);
	s_api.buildGlyphs = reinterpret_cast<fnBuildGlyphs>(build);

	auto rip = [&m](uintptr_t fn, const sig::RipRef& r) { return Rip(m, fn + r.insn, r.disp, r.len); };
	const uintptr_t fontMgrSlot = rip(build, sig::pinstCFontManager);
	const uintptr_t renderSlot  = rip(drawPage, sig::pinstCRender);
	const uintptr_t mgr         = rip(instance, sig::__DebugLineBuffer);
	if (!fontMgrSlot || !renderSlot || !mgr)
		return fail("nametag batching is unavailable on this client");
	s_api.fontMgrSlot = reinterpret_cast<CFontManager**>(fontMgrSlot);
	s_api.renderSlot  = reinterpret_cast<CRender**>(renderSlot);
	s_api.mgr         = reinterpret_cast<C2DPrimitiveManager*>(mgr);

	s_api.vtable = FindTextVtable(m, build, prepare, render);
	if (!s_api.vtable)
		return fail("nametag text renderer unavailable on this client");

	if (const uintptr_t uiBuild = Scan(m, sig::CTextObject__BuildGlyphs))
	{
		s_api.uiVtable = FindTextVtable(m, uiBuild, prepare, render);
		if (s_api.uiVtable)
			s_api.origUiBuild = reinterpret_cast<fnBuildGlyphs>(uiBuild);
	}

	s_api.origPrepare = reinterpret_cast<fnTextVfn>(prepare);
	s_api.origRender = reinterpret_cast<fnTextVfn>(render);
	s_resolved = true;
	return true;
}

struct Run { uint32_t page; uint32_t glyphs; };

struct Shadow
{
	uint32_t  length = 0;
	int       font = 0;
	std::vector<FONT_D3DTLVERTEX> verts;
	std::vector<Run>     runs;
	uint32_t  lastFrame = 0;
	bool      valid = false;
};

uint32_t CollectRuns(const CTextObjectBase* obj, std::vector<Run>& runs)
{
	uint32_t total = 0;
	for (const CTextPageRun* r = obj->m_pageRuns; r; r = r->pNext)
	{
		if (r->bActive && r->glyphCount > 0)
		{
			runs.push_back({ r->page, static_cast<uint32_t>(r->glyphCount) });
			total += static_cast<uint32_t>(r->glyphCount);
		}
	}
	return total;
}

std::unordered_map<const CTextObjectBase*, Shadow> s_shadows;
std::vector<const CTextObjectBase*> s_order;
std::vector<FONT_D3DTLVERTEX> s_scratch;
uint32_t s_frame = 0;
const CTextObjectBase* s_batchHead = nullptr;
bool s_batchReady = false;

struct PassVB { Direct3DVertexBuffer9* vb = nullptr; uint32_t bytes = 0; };
PassVB s_tagVB;
PassVB s_uiVB;

bool UpdateShadow(CTextObjectBase* obj)
{
	const uint32_t length = static_cast<uint32_t>(obj->m_text.length());
	if (length == 0)
		return false;

	Shadow& sh = s_shadows[obj];
	sh.lastFrame = s_frame;

	if (sh.valid && !obj->m_dirty && sh.length == length)
		return !sh.verts.empty();

	const size_t need = std::max(length, kMinBuildGlyphs) * kVertsPerGlyph;
	if (s_scratch.size() < need)
		s_scratch.resize(need);
	for (CTextPageRun* r = obj->m_pageRuns; r; r = r->pNext)
		r->bActive = false;

	s_api.buildGlyphs(obj, s_scratch.data());
	obj->m_dirty = false;

	sh.valid = true;
	sh.length = length;
	sh.font = obj->m_font;
	sh.runs.clear();
	sh.verts.clear();
	if (!obj->m_render)
		return false;

	const uint32_t total = CollectRuns(obj, sh.runs);
	const size_t verts = static_cast<size_t>(total) * kVertsPerGlyph;
	if (verts > s_scratch.size())
	{
		sh.runs.clear();
		return false;
	}
	sh.verts.assign(s_scratch.begin(), s_scratch.begin() + static_cast<ptrdiff_t>(verts));
	s_statRebuilds.fetch_add(1, std::memory_order_relaxed);
	return total > 0;
}

Direct3DTexture9* PageTexture(const CCachedFont* font, uint32_t page)
{
	const auto& textures = font->arTextures;
	if (page >= static_cast<uint32_t>(textures.GetLength()))
		return nullptr;
	const CD3DTexturePointerNode* node = textures[static_cast<int>(page)];
	return node ? node->pTexture : nullptr;
}

bool EnsureVB(CRender* render, PassVB& p, uint32_t bytes)
{
	if (p.vb && p.bytes >= bytes)
		return true;
	if (p.vb)
		s_api.releaseVB(render, p.vb, kVBTag);
	p.vb = nullptr;
	p.bytes = 0;
	const uint32_t want = std::max<uint32_t>(bytes + bytes / 2, 2048 * kGlyphBytes);
	if (s_api.createVB(render, want, kVBUsage, 0, D3DPOOL_DEFAULT, &p.vb, nullptr, kVBTag) < 0 || !p.vb)
	{
		p.vb = nullptr;
		return false;
	}
	p.bytes = want;
	return true;
}

std::vector<CDeferredTextNode*> s_sortNodes;
bool s_fallbackDrawn = false;

float SortKey(const CDeferredTextNode* node)
{
	const float d = static_cast<const CStringSpriteText*>(node->pText)->m_distance;
	return std::isnan(d) ? -std::numeric_limits<float>::infinity() : d;
}

void SortDeferred(C2DPrimitiveManager* mgr)
{
	if (s_sortNodes.size() < 2)
		return;
	std::stable_sort(s_sortNodes.begin(), s_sortNodes.end(),
		[](const CDeferredTextNode* a, const CDeferredTextNode* b) { return SortKey(a) > SortKey(b); });
	CDeferredTextNode* prev = nullptr;
	for (CDeferredTextNode* n : s_sortNodes)
	{
		n->pPrev = prev;
		if (prev)
			prev->pNext = n;
		prev = n;
	}
	prev->pNext = nullptr;
	mgr->deferredHead = s_sortNodes.front();
	mgr->deferredTail = prev;
}

void PrepareBatch(const CTextObjectBase* head)
{
	++s_frame;
	s_order.clear();
	s_batchReady = false;
	s_fallbackDrawn = false;
	s_batchHead = head;

	s_sortNodes.clear();
	for (CDeferredTextNode* node = s_api.mgr->deferredHead; node; node = node->pNext)
	{
		if (!node->pText)
			continue;
		if (!HasVtable(node->pText, s_api.vtable))
			return;
		s_sortNodes.push_back(node);
	}
	SortDeferred(s_api.mgr);

	for (CDeferredTextNode* node : s_sortNodes)
		if (UpdateShadow(node->pText))
			s_order.push_back(node->pText);
	s_batchReady = true;

	if ((s_frame & 127) == 0)
	{
		for (auto it = s_shadows.begin(); it != s_shadows.end(); )
		{
			if (s_frame - it->second.lastFrame > 256)
				it = s_shadows.erase(it);
			else
				++it;
		}
	}
}

bool Upload(PassVB& p, const Shadow* const* list, size_t n)
{
	CRender* render = Render();
	if (!render)
		return false;
	uint32_t total = 0;
	for (size_t i = 0; i < n; ++i)
		total += static_cast<uint32_t>(list[i]->verts.size() * sizeof(FONT_D3DTLVERTEX));
	if (total == 0 || !EnsureVB(render, p, total))
		return false;

	void* mapped = nullptr;
	if (FAILED(p.vb->Lock(0, total, &mapped, D3DLOCK_DISCARD)) || !mapped)
		return false;
	auto* dst = static_cast<FONT_D3DTLVERTEX*>(mapped);
	for (size_t i = 0; i < n; ++i)
	{
		std::memcpy(dst, list[i]->verts.data(), list[i]->verts.size() * sizeof(FONT_D3DTLVERTEX));
		dst += list[i]->verts.size();
	}
	p.vb->Unlock();
	return true;
}

uint32_t DrawRange(const PassVB& p, const Shadow* const* list, const int* fonts, size_t n, uint32_t base)
{
	CRender* render = Render();
	CFontManager* fontMgr = FontManager();
	Direct3DDevice9* device = render ? render->pD3DDevice : nullptr;
	if (!device || !fontMgr)
		return 0;
	CRenderStateCache* cache = render->GetStateCache();

	s_api.setStream(cache, 0, p.vb, 0, sizeof(FONT_D3DTLVERTEX));
	s_api.setFVF(cache, kFVF);

	Direct3DTexture9*     curTex = nullptr;
	Direct3DIndexBuffer9* curIB = nullptr;
	uint32_t groupStart = base, groupQuads = 0, vertCursor = base, draws = 0;

	auto flushGroup = [&]() {
		if (groupQuads == 0)
			return;
		s_api.flush(cache);
		device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, static_cast<INT>(groupStart), 0,
			groupQuads * kVertsPerGlyph, 0, groupQuads * 2);
		++draws;
		groupStart += groupQuads * kVertsPerGlyph;
		groupQuads = 0;
	};

	for (size_t i = 0; i < n; ++i)
	{
		const CCachedFont* font = s_api.getFont(fontMgr, static_cast<uint32_t>(fonts[i]));
		for (const Run& r : list[i]->runs)
		{
			Direct3DTexture9* tex = font ? PageTexture(font, r.page) : nullptr;
			Direct3DIndexBuffer9* ib = font && font->pFontManager ? font->pFontManager->pIB : nullptr;
			if (!tex || !ib)
			{
				flushGroup();
				vertCursor += r.glyphs * kVertsPerGlyph;
				groupStart = vertCursor;
				continue;
			}
			if (tex != curTex || ib != curIB || groupQuads + r.glyphs > kMaxQuadsPerDraw)
			{
				flushGroup();
				groupStart = vertCursor;
				if (ib != curIB) { s_api.setIndices(cache, ib); curIB = ib; }
				if (tex != curTex) { s_api.setTexture(cache, 0, tex); curTex = tex; }
			}
			groupQuads += r.glyphs;
			vertCursor += r.glyphs * kVertsPerGlyph;
		}
	}
	flushGroup();
	return draws;
}

bool DrawBatch()
{
	if (s_order.empty())
	{
		s_statTags.store(0, std::memory_order_relaxed);
		s_statDraws.store(0, std::memory_order_relaxed);
		return true;
	}
	std::vector<const Shadow*> list;
	std::vector<int> fonts;
	list.reserve(s_order.size());
	fonts.reserve(s_order.size());
	for (const CTextObjectBase* obj : s_order)
	{
		const Shadow& sh = s_shadows[obj];
		list.push_back(&sh);
		fonts.push_back(sh.font);
	}
	if (!Upload(s_tagVB, list.data(), list.size()))
		return false;
	const uint32_t draws = DrawRange(s_tagVB, list.data(), fonts.data(), list.size(), 0);
	s_statTags.store(static_cast<uint32_t>(s_order.size()), std::memory_order_relaxed);
	s_statDraws.store(draws, std::memory_order_relaxed);
	return true;
}

bool IsHead(const CTextObjectBase* obj)
{
	const CDeferredTextNode* node = s_api.mgr->deferredHead;
	return node && node->pText == obj;
}

void PrepareNative(CTextObjectBase* obj)
{
	auto it = s_shadows.find(obj);
	if (it != s_shadows.end())
	{
		obj->m_dirty = true;
		s_shadows.erase(it);
	}
	s_api.origPrepare(obj);
}

void Prepare_Hook(CTextObjectBase* obj)
{
	if (!s_enabled.load(std::memory_order_relaxed))
		return PrepareNative(obj);

	if (IsHead(obj))
	{
		PrepareBatch(obj);
		if (!s_batchReady)
			return PrepareNative(obj);
		return;
	}
	if (s_batchReady || s_fallbackDrawn)
		return;
	PrepareNative(obj);
}

void Render_Hook(CTextObjectBase* obj)
{
	if (!s_enabled.load(std::memory_order_relaxed))
		return s_api.origRender(obj);

	if (s_batchReady && obj == s_batchHead)
	{
		if (!DrawBatch())
		{
			s_batchReady = false;
			for (const CDeferredTextNode* node = s_api.mgr->deferredHead; node; node = node->pNext)
			{
				CTextObjectBase* o = node->pText;
				if (!o) continue;
				o->m_dirty = true;
				s_api.origPrepare(o);
				s_api.origRender(o);
			}
			s_batchHead = nullptr;
			s_fallbackDrawn = true;
		}
		return;
	}
	if (s_batchReady || s_fallbackDrawn)
		return;
	s_api.origRender(obj);
}

std::atomic<bool> s_uiEnabled{ true };
bool s_uiInstalled = false;

std::unordered_map<const CTextObjectBase*, Shadow> s_uiShadows;
std::vector<FONT_D3DTLVERTEX> s_uiScratch;

bool s_uiLastWasRender = true;
bool s_uiPlanned = false;
uint32_t s_uiPass = 0;

struct UiMerge { size_t first; size_t count; uint32_t base; };
std::unordered_map<const CTextObjectBase*, UiMerge> s_uiHeads;
std::unordered_set<const CTextObjectBase*>          s_uiSkip;
std::vector<const Shadow*>                          s_uiList;
std::vector<int>                                    s_uiFonts;

std::atomic<uint32_t> s_uiStatMerged{ 0 };
std::atomic<uint32_t> s_uiStatDrawsSaved{ 0 };

void UiBuild_Hook(CTextObjectBase* obj, FONT_D3DTLVERTEX* verts)
{
	if (!verts)
		return s_api.origUiBuild(obj, verts);

	if (!s_uiEnabled.load(std::memory_order_relaxed))
	{
		s_uiShadows.erase(obj);
		return s_api.origUiBuild(obj, verts);
	}

	const uint32_t length = static_cast<uint32_t>(obj->m_text.length());
	const size_t need = std::max({ length, static_cast<uint32_t>(std::max(obj->m_maxLength, 0)), kMinBuildGlyphs })
		* kVertsPerGlyph;
	if (s_uiScratch.size() < need)
		s_uiScratch.resize(need);

	s_api.origUiBuild(obj, s_uiScratch.data());

	Shadow& sh = s_uiShadows[obj];
	sh.runs.clear();
	const uint32_t total = CollectRuns(obj, sh.runs);
	const size_t count = std::min(static_cast<size_t>(total) * kVertsPerGlyph, need);

	std::memcpy(verts, s_uiScratch.data(), count * sizeof(FONT_D3DTLVERTEX));

	sh.valid = obj->m_render && count == static_cast<size_t>(total) * kVertsPerGlyph;
	sh.length = length;
	sh.font = obj->m_font;
	sh.lastFrame = s_uiPass;
	sh.verts.assign(s_uiScratch.begin(), s_uiScratch.begin() + static_cast<ptrdiff_t>(count));
}

const Shadow* UiMergeable(const CTextObjectBase* obj)
{
	if (!HasVtable(obj, s_api.uiVtable))
		return nullptr;
	const uint32_t length = static_cast<uint32_t>(obj->m_text.length());
	if (length == 0 || !obj->m_pVertexBuffer || !obj->m_render)
		return nullptr;
	auto it = s_uiShadows.find(obj);
	if (it == s_uiShadows.end())
		return nullptr;
	it->second.lastFrame = s_uiPass;
	if (!it->second.valid || it->second.length != length || it->second.verts.empty())
		return nullptr;
	return &it->second;
}

void UiPrepare_Hook(CTextObjectBase* obj)
{
	if (s_uiLastWasRender)
		s_uiPlanned = false;
	s_uiLastWasRender = false;
	s_api.origPrepare(obj);
}

void PlanUiPass()
{
	s_uiPlanned = true;
	++s_uiPass;
	s_uiHeads.clear();
	s_uiSkip.clear();
	s_uiList.clear();
	s_uiFonts.clear();

	if ((s_uiPass & 127) == 0)
	{
		for (auto it = s_uiShadows.begin(); it != s_uiShadows.end(); )
		{
			if (s_uiPass - it->second.lastFrame > 256)
				it = s_uiShadows.erase(it);
			else
				++it;
		}
	}

	const C2DPrimitiveManager* mgr = s_api.mgr;
	if (!mgr->prims)
		return;
	const int count = mgr->primCount;

	auto textAt = [&](int i) -> CTextObjectBase* {
		const SPrim2D& e = mgr->prims[i];
		return (e.type & ~kPrimBatchHead) == Prim2D_CachedText ? static_cast<CTextObjectBase*>(e.texture) : nullptr;
	};

	uint32_t vertex = 0;
	for (int i = 0; i < count; )
	{
		const size_t first = s_uiList.size();
		const uint32_t base = vertex;
		int j = i;
		for (; j < count; ++j)
		{
			CTextObjectBase* o = textAt(j);
			const Shadow* sh = o ? UiMergeable(o) : nullptr;
			if (!sh)
			{
				if (o && HasVtable(o, s_api.uiVtable) && !s_uiShadows.count(o))
				{
					o->m_dirty = true;
					s_uiShadows[o].lastFrame = s_uiPass;
				}
				break;
			}
			s_uiList.push_back(sh);
			s_uiFonts.push_back(o->m_font);
			vertex += static_cast<uint32_t>(sh->verts.size());
		}

		const size_t n = s_uiList.size() - first;
		if (n >= 2)
		{
			s_uiHeads[textAt(i)] = { first, n, base };
			for (int k = i + 1; k < j; ++k)
				s_uiSkip.insert(textAt(k));
		}
		else
		{
			s_uiList.resize(first);
			s_uiFonts.resize(first);
			vertex = base;
		}
		i = std::max(j, i + 1);
	}

	if (!s_uiHeads.empty() && !Upload(s_uiVB, s_uiList.data(), s_uiList.size()))
	{
		s_uiHeads.clear();
		s_uiSkip.clear();
	}
}

void UiRender_Hook(CTextObjectBase* obj)
{
	s_uiLastWasRender = true;
	if (!s_uiEnabled.load(std::memory_order_relaxed))
		return s_api.origRender(obj);
	if (!s_uiPlanned)
		PlanUiPass();

	if (s_uiSkip.count(obj))
		return;
	auto it = s_uiHeads.find(obj);
	if (it == s_uiHeads.end())
		return s_api.origRender(obj);

	const UiMerge& m = it->second;
	const uint32_t draws = DrawRange(s_uiVB, s_uiList.data() + m.first, s_uiFonts.data() + m.first, m.count, m.base);

	uint32_t wouldHave = 0;
	for (size_t k = 0; k < m.count; ++k)
		wouldHave += static_cast<uint32_t>(s_uiList[m.first + k]->runs.size());
	s_uiStatMerged.fetch_add(static_cast<uint32_t>(m.count), std::memory_order_relaxed);
	if (wouldHave > draws)
		s_uiStatDrawsSaved.fetch_add(wouldHave - draws, std::memory_order_relaxed);
}

template <typename Hook, typename Original>
bool PatchSlot(uintptr_t* vtable, size_t slot, Hook hook, Original original, std::string_view name)
{
	const uintptr_t replacement = reinterpret_cast<uintptr_t>(hook);
	const uintptr_t expected = reinterpret_cast<uintptr_t>(original);
	return mq::AddPatch(reinterpret_cast<uintptr_t>(&vtable[slot]), reinterpret_cast<const uint8_t*>(&replacement),
		sizeof(replacement), reinterpret_cast<const uint8_t*>(&expected), name);
}

void UnpatchSlot(uintptr_t* vtable, size_t slot)
{
	mq::RemovePatch(reinterpret_cast<uintptr_t>(&vtable[slot]));
}

void UnpatchUiSlots()
{
	UnpatchSlot(s_api.uiVtable, kRenderSlot);
	UnpatchSlot(s_api.uiVtable, kPrepareVBSlot);
	UnpatchSlot(s_api.uiVtable, kBuildGlyphsSlot);
}

uintptr_t s_insertAddr = 0;
bool      s_insertScanned = false;

void Unlink(C2DPrimitiveManager* mgr, CDeferredTextNode* node)
{
	if (node == mgr->deferredHead)
		mgr->deferredHead = node->pNext;
	if (node == mgr->deferredTail)
		mgr->deferredTail = node->pPrev;
	if (node->pPrev)
		node->pPrev->pNext = node->pNext;
	if (node->pNext)
		node->pNext->pPrev = node->pPrev;
	node->pPrev = node->pNext = nullptr;
	node->pOwnerList = nullptr;
}

class DeferredTextHook
{
public:
	DETOUR_TRAMPOLINE_DEF_MEMBER(DeferredTextHook, void, Insert_Trampoline, (CStringSpriteText*))
	void Insert_Detour(CStringSpriteText* text)
	{
		if (!text || !s_installed || !s_enabled.load(std::memory_order_relaxed))
			return Insert_Trampoline(text);

		C2DPrimitiveManager* mgr = reinterpret_cast<C2DPrimitiveManager*>(this);
		CDeferredTextNode* node = &text->m_deferred;
		if (node->pOwnerList)
			Unlink(mgr, node);
		node->pPrev = mgr->deferredTail;
		node->pNext = nullptr;
		if (mgr->deferredTail)
			mgr->deferredTail->pNext = node;
		else
			mgr->deferredHead = node;
		mgr->deferredTail = node;
		node->pOwnerList = &mgr->deferredHead;
		++mgr->deferredCount;
	}
};

void InstallDeferredInsert()
{
	if (s_insertScanned || !eqlib::EQGraphicsBaseAddress)
		return;
	s_insertScanned = true;
	s_insertAddr = Scan(eqgfx::scan::Graphics(), sig::C2DPrimitiveManager__InsertDeferredText);
	if (s_insertAddr && !EzDetour(s_insertAddr, &DeferredTextHook::Insert_Detour, &DeferredTextHook::Insert_Trampoline))
		s_insertAddr = 0;
}

void RemoveDeferredInsert()
{
	if (s_insertAddr)
		RemoveDetour(s_insertAddr);
	s_insertAddr = 0;
	s_insertScanned = false;
}

} // namespace

void InstallNametagBatch()
{
	InstallDeferredInsert();
	if (s_installed || s_installFailed || !Resolve())
		return;
	const bool preparePatched = PatchSlot(s_api.vtable, kPrepareVBSlot, &Prepare_Hook, s_api.origPrepare, "CStringSpriteText::PrepareVB");
	if (!preparePatched
		|| !PatchSlot(s_api.vtable, kRenderSlot, &Render_Hook, s_api.origRender, "CStringSpriteText::Render"))
	{
		if (preparePatched)
			UnpatchSlot(s_api.vtable, kPrepareVBSlot);
		Conflict("could not hook the nametag renderer");
		s_installFailed = true;
		return;
	}
	s_installed = true;

	if (!s_api.uiVtable)
		return;
	const bool buildPatched = PatchSlot(s_api.uiVtable, kBuildGlyphsSlot, &UiBuild_Hook, s_api.origUiBuild, "CTextObject::BuildGlyphs");
	const bool uiPreparePatched = buildPatched
		&& PatchSlot(s_api.uiVtable, kPrepareVBSlot, &UiPrepare_Hook, s_api.origPrepare, "CTextObject::PrepareVB");
	if (uiPreparePatched
		&& PatchSlot(s_api.uiVtable, kRenderSlot, &UiRender_Hook, s_api.origRender, "CTextObject::Render"))
	{
		s_uiInstalled = true;
		return;
	}
	if (uiPreparePatched)
		UnpatchSlot(s_api.uiVtable, kPrepareVBSlot);
	if (buildPatched)
		UnpatchSlot(s_api.uiVtable, kBuildGlyphsSlot);
}

void RemoveNametagBatch()
{
	RemoveDeferredInsert();
	if (!s_installed)
		return;
	if (s_uiInstalled)
	{
		UnpatchUiSlots();
		s_uiInstalled = false;
		s_uiShadows.clear();
	}
	UnpatchSlot(s_api.vtable, kPrepareVBSlot);
	UnpatchSlot(s_api.vtable, kRenderSlot);
	s_installed = false;

	ReleaseNametagBatchDevice();
	s_shadows.clear();
	s_order.clear();
	s_batchReady = false;
	s_batchHead = nullptr;
}

void ReleaseNametagBatchDevice()
{
	CRender* render = Render();
	for (PassVB* p : { &s_tagVB, &s_uiVB })
	{
		if (p->vb && render)
			s_api.releaseVB(render, p->vb, kVBTag);
		*p = PassVB{};
	}
}

std::atomic<uint32_t> s_eqDraws{ 0 };
uint32_t              s_lastFlushCount = 0;

void SampleEqDrawCounter()
{
	CRender* render = Resolve() ? Render() : nullptr;
	if (!render)
		return;
	const uint32_t now = static_cast<uint32_t>(render->GetStateCache()->flushCount);
	s_eqDraws.store(now - s_lastFlushCount, std::memory_order_relaxed);
	s_lastFlushCount = now;
}

uint32_t EqDrawsLastPulse() { return s_eqDraws.load(std::memory_order_relaxed); }

void ForgetNametagBatch()
{
	s_shadows.clear();
	s_order.clear();
	s_batchReady = false;
	s_batchHead = nullptr;
}

bool NametagBatchCommand(const char* szLine)
{
	char a1[MAX_STRING] = { 0 };
	GetArg(a1, szLine, 2);
	if (ci_equals(a1, "on"))       s_enabled.store(true);
	else if (ci_equals(a1, "off")) s_enabled.store(false);

	if (!s_installed)
	{
		WriteChatf("\ag[MQEQGFX]\ax nametag batching \arnot active\ax - %s",
			*s_failReason ? s_failReason : "not installed yet (waits for EQGraphics)");
		return true;
	}
	WriteChatf("\ag[MQEQGFX]\ax nametag batching %s | last frame: %u tags in %u draws | %u relayouts since last asked",
		s_enabled.load() ? "\agon\ax" : "\ayoff\ax", s_statTags.load(), s_statDraws.load(),
		s_statRebuilds.exchange(0));
	WriteChatf("\ag[MQEQGFX]\ax the client issued %u draws last pulse (all of it: world, UI, text)",
		s_eqDraws.load());
	WriteChatf("\ag[MQEQGFX]\ax /eqgfx nametags on|off - draw every nametag from one buffer instead of one draw per tag");
	return true;
}

bool UiTextBatchCommand(const char* szLine)
{
	char a1[MAX_STRING] = { 0 };
	GetArg(a1, szLine, 2);
	if (ci_equals(a1, "on"))       s_uiEnabled.store(true);
	else if (ci_equals(a1, "off")) s_uiEnabled.store(false);

	if (!s_uiInstalled)
	{
		WriteChatf("\ag[MQEQGFX]\ax UI text batching \arnot active\ax - %s",
			!s_installed ? (*s_failReason ? s_failReason : "not installed yet")
			             : kUnsupported);
		return true;
	}
	WriteChatf("\ag[MQEQGFX]\ax UI text batching %s | since last asked: %u strings merged, %u draws saved | "
		"%zu strings cached | client draws last pulse: %u",
		s_uiEnabled.load() ? "\agon\ax" : "\ayoff\ax", s_uiStatMerged.exchange(0),
		s_uiStatDrawsSaved.exchange(0), s_uiShadows.size(), s_eqDraws.load());
	WriteChatf("\ag[MQEQGFX]\ax /eqgfx uitext on|off - adjacent strings drawn together instead of one draw each");
	return true;
}

bool NametagBatchEnabled() { return s_installed && s_enabled.load(std::memory_order_relaxed); }
void SetNametagBatchEnabled(bool on) { s_enabled.store(on, std::memory_order_relaxed); }

TextBatchStats GetTextBatchStats()
{
	TextBatchStats t;
	t.resolved = s_resolved;
	t.failReason = s_failReason;
	t.nametagsInstalled = s_installed;
	t.sortInstalled = s_insertAddr != 0;
	t.uiInstalled = s_uiInstalled;
	t.nametagsOn = s_enabled.load(std::memory_order_relaxed);
	t.uiOn = s_uiEnabled.load(std::memory_order_relaxed);
	t.tags = s_statTags.load(std::memory_order_relaxed);
	t.tagDraws = s_statDraws.load(std::memory_order_relaxed);
	t.tagRebuilds = s_statRebuilds.load(std::memory_order_relaxed);
	t.uiMerged = s_uiStatMerged.load(std::memory_order_relaxed);
	t.uiDrawsSaved = s_uiStatDrawsSaved.load(std::memory_order_relaxed);
	t.uiCached = static_cast<uint32_t>(s_uiShadows.size());
	t.uiMergesPlanned = static_cast<uint32_t>(s_uiHeads.size());
	return t;
}

void SetUiTextBatchEnabled(bool on) { s_uiEnabled.store(on, std::memory_order_relaxed); }

void ResetTextBatchCaches()
{
	s_shadows.clear();
	s_order.clear();
	s_batchReady = false;
	s_batchHead = nullptr;
	s_uiShadows.clear();
	s_uiHeads.clear();
	s_uiSkip.clear();
	s_uiPlanned = false;
	s_statRebuilds.store(0);
	s_uiStatMerged.store(0);
	s_uiStatDrawsSaved.store(0);
}
