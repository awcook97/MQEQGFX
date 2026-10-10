#pragma once

#if !defined(_WIN64)
#error "MQEQGFX requires the 64-bit client"
#endif

#include <mq/Plugin.h>

#include "Detours.h"

#include "model/eqg.h"
#include "model/dds.h"
#include "model/render.h"
#include "gpu/LineFX.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

struct CastEvent
{
	int spellId;
	int spawnId;
	uint32_t castTimeMs;
	uint32_t timestampMs;
};

constexpr size_t kMaxCastEvents = 64;

extern std::mutex             s_castMutex;
extern std::vector<CastEvent> s_castEvents;

void InstallCastTap();
void RemoveCastTap();
void PulseCastTap();

#ifdef CEverQuest__StartCasting_x
extern bool s_castTapInstalled;
extern bool s_castTapFailed;
extern bool s_castTrackerOwned;
#endif

struct DamageEvent
{
	int attackerId;
	int targetId;
	int damage;
	int spellId;
	int skill;
	bool heal;
	int totalHeal;
	uint32_t flags;
	uint32_t timestampMs;
};

enum : uint32_t
{
	HealFlag_AltMessage = 0x01,
	HealFlag_OverTime   = 0x02,
	HealFlag_Critical   = 0x04,
	HealFlag_Lucky      = 0x08,
	HealFlag_Twincast   = 0x10,
	HealFlag_NoLandMsg1 = 0x20,
	HealFlag_NoLandMsg2 = 0x40,
};

enum : uint32_t
{
	HitFlag_Critical = 0x80,
	HitFlag_Lucky    = 0x100,
	HitFlag_Twincast = 0x20000,
	HitFlag_Locked   = 0x100000,
};

constexpr size_t kMaxDamageEvents = 256;

extern std::mutex               s_dmgMutex;
extern std::vector<DamageEvent> s_dmgEvents;
extern std::atomic<bool>        s_dmgDebug;
extern bool                     s_dmgTapInstalled;

void InstallDamageTap();
void RemoveDamageTap();

struct MatEvent { std::string a; std::string b; uint32_t tint; };

constexpr size_t kMaxMatEvents = 4096;

extern std::mutex            s_capMutex;
extern std::vector<MatEvent> s_matEvents;
extern std::string           s_headMesh;
extern uint32_t              s_headTint;
extern bool                  s_haveCapture;
extern std::string           s_captureActorDef;
extern std::atomic<bool>     s_captureDirty;
extern bool                  s_captureInstalled;
extern bool                  s_captureVerbose;

bool InstallSwapCapture(bool auto_ = false);
void RemoveSwapCapture();
void SaveCapture();
bool LoadCapture(const std::string& actorDef);

extern eqgfx::render::ModelView s_modelView;
extern std::atomic<bool>        s_showModel;
extern std::atomic<bool>        s_reloadModel;

bool BuildModelView(const std::string& armor, std::string& src, std::string& note);
void DumpCharCommand();
void DumpModelCommand();

enum : uint32_t
{
	Highlight_Tint    = 0x1,
	Highlight_Ambient = 0x2,
};

constexpr size_t kMaxHighlights = 256;

void SetSpawnHighlight(int spawnId, uint32_t color, uint32_t mode, const void* owner = nullptr);
void ClearSpawnHighlight(int spawnId);
void ClearAllHighlights();
void ClearHighlightsOwnedBy(const void* owner);
int  HighlightCount();
void PulseHighlights();
void RefreshHighlights();
void OnHighlightSpawnRemoved(int spawnId);
void ForgetHighlights();

std::vector<std::tuple<int, uint32_t, uint32_t, bool>> HighlightList();

float FxClockNow();

inline void UnpackArgb(uint32_t argb, float* rgba)
{
	const mq::MQColor color(argb);
	rgba[0] = color.Red / 255.0f;
	rgba[1] = color.Green / 255.0f;
	rgba[2] = color.Blue / 255.0f;
	rgba[3] = color.Alpha / 255.0f;
}

void InstallSelfCastHook();
void RemoveSelfCastHook();
void SpellFxNoteCastStart(int spellId, int spawnId, uint32_t castTimeMs);
void SpellFxNoteLanded(int casterId, int targetId, int spellId, bool heal);
void PulseSpellFx(std::vector<eqgfx::gpu::FxInstance>& out);
void ForgetSpellFx();
void LoadSpellFxSettings();
void FlushSpellFxSettings();
void OnSpellFxSpawnRemoved(int spawnId);
bool SpellFxCommand(const char* szLine);
void DrawSpellFxOptions();
int  SpellFxLookCount();
bool SpellFxEnabled();
void SetSpellFxEnabled(bool on);
void SpellFxPlay(int spellId, int casterId, int targetId, uint32_t castMs);
std::string SpellFxDescribe(int spellId);
const char* SpellFxLookName(int index);
void SpellFxPlayLook(int index, int casterId, int targetId, float delay);
int SpellFxLineCount();
const char* SpellFxLineClass(int index);
const char* SpellFxLineName(int index);
void SpellFxPlayLine(int index, int casterId, int targetId, float delay);
int  SpellFxDemoLevel();
void SetSpellFxDemoLevel(int level);
void SpellFxDemoForm(uint32_t form, int casterId, int targetId);
void SpellFxDemoGlyphs(int spawnId);
uint32_t SpellFxActiveCount();
bool SelfCastHookInstalled();

bool ProjectToScreen(float x, float y, float z, float& sx, float& sy);

void AttackFxNoteHit(int attackerId, int targetId, int damage, uint32_t flags);
void BuildAttackFx(std::vector<eqgfx::gpu::FxInstance>& out);
void DrawAttackOverlay();
void DrawAttackHealthBar(struct ImDrawList* dl, float x0, float y0, float x1, float y1, float rounding);
void DrawAttackOptions();
void LoadAttackSettings();
void FlushAttackSettings();

void BuildTargetRing(std::vector<eqgfx::gpu::FxInstance>& out);
void DrawTargetRingOptions();
void LoadTargetRingSettings();
void FlushTargetRingSettings();

void DrawDebugWindow();
void ShowDebugWindow(bool on);
bool DebugWindowShown();

void LoadGfxSettings();
void PulseGfxSettings();
void FlushGfxSettings();

struct FxStats { uint32_t instances, segments; bool ready; const char* error; };
FxStats GetFxStats();

void InstallNametagBatch();
void RemoveNametagBatch();
void ReleaseNametagBatchDevice();
void ForgetNametagBatch();
bool NametagBatchCommand(const char* szLine);
bool UiTextBatchCommand(const char* szLine);
bool NametagBatchEnabled();
void SetNametagBatchEnabled(bool on);
void SetUiTextBatchEnabled(bool on);
void ResetTextBatchCaches();

struct TextBatchStats
{
	bool        resolved = false;
	const char* failReason = "";
	bool        nametagsInstalled = false, uiInstalled = false, sortInstalled = false;
	bool        nametagsOn = false, uiOn = false;
	uint32_t    tags = 0, tagDraws = 0, tagRebuilds = 0;
	uint32_t    uiMerged = 0, uiDrawsSaved = 0, uiCached = 0, uiMergesPlanned = 0;
};
TextBatchStats GetTextBatchStats();

void SampleEqDrawCounter();
uint32_t EqDrawsLastPulse();

struct AnimationLodOptions
{
	bool  enabled = false;
	float distance = 100.0f;
	int   maxInterval = 4;
};

void InstallAnimationLod();
void RemoveAnimationLod();
void PulseAnimationLod();
bool AnimationLodInstalled();
bool AnimationLodCommand(const char* szLine);
AnimationLodOptions GetAnimationLodOptions();
void SetAnimationLodOptions(const AnimationLodOptions& o);

void InstallParticleGpu();
void RemoveParticleGpu();
void ReleaseParticleGpuDevice();
void DrawParticleGpu();
bool ParticleGpuInstalled();
bool ParticleGpuEnabled();
void SetParticleGpuEnabled(bool on);
const char* ParticleGpuFailReason();
constexpr int kDefaultParticleBudget = 65536;
constexpr int kMinParticleBudget = 4096;
constexpr int kMaxParticleBudget = 1048576;
void TrimParticleGpu();
int GetParticleGpuBudget();
void SetParticleGpuBudget(int particles);
size_t ParticleGpuBufferBytes();
bool ParticleGpuCommand(const char* szLine);

struct ParticleGpuStats
{
	uint32_t    particles = 0, emitters = 0, draws = 0, gatedFrames = 0;
	const char* ungatedReason = "";
};

void NoteParticleGpuFrame(bool gated, const char* ungatedReason);
void NoteParticleGpuDraw(uint32_t particles, uint32_t emitters, uint32_t draws);
void SampleParticleGpuStats();
ParticleGpuStats GetParticleGpuStats();

struct ActorAnimationStats
{
	uint32_t animated = 0, distant = 0, skipped = 0;
};

void NoteActorAnimation(bool distant, bool skipped);
void SampleActorAnimationStats();
ActorAnimationStats GetActorAnimationStats();

struct AlwaysOnStyle
{
	float opacity   = 0.85f;
	float motion    = 1.0f;
	float fadeAfter = 30.0f;
	float fadeTo    = 0.35f;
	bool  breathe   = false;

	bool operator==(const AlwaysOnStyle&) const = default;
};

float AlwaysOnFade(const AlwaysOnStyle& style, float shownFor, float phase);

bool AlwaysOnStyleOptions(AlwaysOnStyle& style);

struct GfxOptions
{
	bool  classFx;
	bool  classFxPCs;
	bool  classFxNPCs;
	float classFxRadius;
	int   classFxMaxSpawns;
	int   classFxSlotMask;
	AlwaysOnStyle classFxStyle;
	bool  testRing;
	bool  gpuEffects;
	bool  damageDebug;
};

GfxOptions GetGfxOptions();
void       SetGfxOptions(const GfxOptions& opts);

struct UnitOptions
{
	bool worldHover;
	bool focusMarker;
	bool hoverFallback;
	bool buffProbe;
};

UnitOptions GetUnitOptions();
void        SetUnitOptions(const UnitOptions& opts);

bool BuffProbeAvailable();

struct UnitCast
{
	int      spellId;
	uint32_t startMs;
	uint32_t castMs;
};

void InitUnits();
void ShutdownUnits();
void PulseUnits();
void DrawUnitsUI();
void OnUnitSpawnRemoved(int spawnId);

void UnitsNoteCastStart(int spellId, int spawnId, uint32_t castTimeMs);

void UnitsWantBuffs(int spawnId);

void BuildUnitMarkers(std::vector<eqgfx::gpu::FxInstance>& out);

bool UnitsCommand(const char* szCmd, const char* szLine);

int  GetSecondaryTarget();
void SetSecondaryTarget(int spawnId);
int  GetHoverUnit();
void SetHoverUnit(int spawnId);
bool GetSpawnCast(int spawnId, UnitCast& out);

bool CastAtSpawn(int spawnId, int gem);

int  ResolveCastUnit();

void AtTargetCommand(PlayerClient* pChar, const char* szLine);
