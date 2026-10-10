#pragma once

#include <cstdint>

namespace eqgfx::sig {
inline constexpr const char* CRenderStateCache__FlushToDevice =
	"40 53 55 56 57 41 56 48 83 EC 30 FF 81 CC 33 00 00 33 ED 48 8B 05";
inline constexpr const char* CRenderStateCache__SetStreamSource =
	"48 89 5C 24 08 44 8B 5C 24 28 44 8B D2 49 8D 5A 6E 48 03 DB";
inline constexpr const char* CRenderStateCache__SetTexture =
	"8B C2 4C 39 84 C1 E0 07 00 00 74 26 4C 89 84 C1 E0 07 00 00";
inline constexpr const char* CRenderStateCache__SetFVF =
	"39 11 74 24 33 C0 89 11 FF 81 D8 33 00 00 48 89 81 A0 10 00 00";
inline constexpr const char* CRenderStateCache__SetIndices =
	"48 39 51 10 74 14 FF 81 D8 33 00 00 33 C0 C6 81 AC 32 00 00 01";
inline constexpr const char* CFontManager__GetFont =
	"48 63 81 B0 00 00 00 4C 8B D9 0F B6 49 7C 4C 8B C8 49 D3 F9";
inline constexpr const char* CRender__EQG_CreateVertexBuffer =
	"40 53 55 56 57 41 56 48 83 EC 60 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 44 24 58 48 8B 89 08 0F 00 00";
inline constexpr const char* CRender__EQG_ReleaseVertexBuffer =
	"48 89 5C 24 08 57 48 83 EC 40 48 8B 05 ?? ?? ?? ?? 48 33 C4";
inline constexpr const char* CTextObjectBase__PrepareVB =
	"48 89 5C 24 20 57 48 83 EC 40 80 79 40 00 48 8B D9 75 0B 48 83 79 38 00";
inline constexpr const char* CTextObjectBase__Render =
	"40 57 48 83 EC 30 48 8B 41 08 48 8B F9 48 85 C0 74 7E 83 78 08 00";
inline constexpr const char* CStringSpriteText__BuildGlyphs =
	"40 55 56 41 56 48 83 EC 40 48 8B EA 48 8B F1 8B 51 10 48 8B 0D ?? ?? ?? ??";
inline constexpr const char* CTextObject__BuildGlyphs =
	"40 55 41 56 41 57 48 83 EC 50 4C 8B F2 48 8B E9 8B 51 10 48 8B 0D ?? ?? ?? ??";
inline constexpr const char* CCachedFont__DrawPageRun =
	"48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 40 48 8B D9 49 63 F0 48 8B 0D ?? ?? ?? ??";
inline constexpr const char* CActor__Update =
	"4C 8B DC 53 48 81 EC 90 00 00 00 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 44 24 70 80 B9 F0 01 00 00 00";
inline constexpr const char* CParticleSystem__ProcessFrame =
	"48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 56 41 57 48 8D 68 A1 48 81 EC 00 01 00 00";
inline constexpr const char* CParticlePoint__GetAlpha =
	"48 8B C1 48 8B 49 50 48 85 C9 74 0A 48 8B 01 48 FF A0 58 01 00 00 48 8B 48 58 48 85 C9 74 0A 48";
inline constexpr const char* CParticlePoint__GetParticleScaleFactor =
	"48 83 EC 28 48 8B 49 60 48 85 C9 74 17 48 8B 01 FF 50 28 48 8B C8 48 8B 10 48 83 C4 28 48 FF A2 38 04 00 00";
inline constexpr const char* CParticleSystem__DrawGate =
	"48 8B 55 F8 39 8A 14 01 00 00 0F 85 ?? ?? ?? ?? F3 44 0F 10 35";
inline constexpr const char* C2DPrimitiveManager__InsertDeferredText =
	"4C 8B D2 4C 8B C9 48 85 D2 0F 84 FB 00 00 00 48 83 7A 68 00 74 50 4C 8D 42 58";
inline constexpr const char* C2DPrimitiveManager__Instance =
	"40 53 48 83 EC 30 8B 0D ?? ?? ?? ?? 65 48 8B 04 25 58 00 00 00 BA ?? ?? ?? ?? "
	"48 8B 0C C8 8B 04 0A 39 05 ?? ?? ?? ?? 7F ?? 48 8D 05";

struct RipRef { uint32_t insn, disp, len; };

inline constexpr RipRef pinstCFontManager    { 0x12, 3, 7 };
inline constexpr RipRef pinstCRender         { 0x1B, 3, 7 };
inline constexpr RipRef __DebugLineBuffer    { 0x29, 3, 7 };

inline constexpr const char* CharacterZoneClient__CastSpell =
	"48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 F8 FE FF FF 48 81 EC C8 01 00 00 "
	"0F 29 70 A8 0F 29 78 98 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 85 90 00 00 00";

} // namespace eqgfx::sig
