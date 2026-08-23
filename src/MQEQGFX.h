/*
 * MQEQGFX.h - state shared between the plugin's translation units.
 * Anything used by only one file stays in that file's anonymous namespace.
 */
#pragma once

#include <mq/Plugin.h>

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

// ---------------------------------------------------------------------------
// Cast-start events (Spells.cpp), buffered until Lua drains them via
// poll_casts(). One consumer by design.

struct CastEvent
{
	int spellId;
	int spawnId;
	uint32_t castTimeMs;
	uint32_t timestampMs;                       // pDisplay->TimeStamp at arrival
};

constexpr size_t kMaxCastEvents = 64;

extern std::mutex             s_castMutex;
extern std::vector<CastEvent> s_castEvents;

void InstallCastTap();
void RemoveCastTap();
void PulseCastTap();

#ifdef CEverQuest__StartCasting_x
extern bool s_castTapInstalled;                 // one of the two modes is active
extern bool s_castTapFailed;                    // detour conflict - don't retry every pulse
extern bool s_castTrackerOwned;                 // mode 3: detour + sweep + extension are ours
#endif

// ---------------------------------------------------------------------------
// Combat damage/heal events (CombatNumbers.cpp), buffered until Lua drains
// them via poll_damage(). One consumer by design.

struct DamageEvent
{
	int attackerId;         // heal: the healer
	int targetId;           // heal: the spawn healed
	int damage;             // >0 dealt; <=0 miss/defense code (heal: amount landed)
	int spellId;            // >0 for spell/DoT damage (heal: the healing spell)
	int skill;              // attack skill for melee
	bool heal;
	int totalHeal;          // heal only: pre-cap amount (overheal = total - damage)
	uint32_t flags;         // hit: EQSuccessfulHit.Flags; heal: HealFlag_* bits
	uint32_t timestampMs;   // pDisplay->TimeStamp at arrival
};

// EQSuccessfulHeal.EffectBitmask bits
enum : uint32_t
{
	HealFlag_AltMessage = 0x01,   // alternate message strings; with 0x20/0x40 suppresses spell-landed
	HealFlag_OverTime   = 0x02,   // heal-over-time chat channels
	HealFlag_Critical   = 0x04,
	HealFlag_Lucky      = 0x08,
	HealFlag_Twincast   = 0x10,
	HealFlag_NoLandMsg1 = 0x20,   // suppresses the spell-landed message
	HealFlag_NoLandMsg2 = 0x40,   // suppresses the spell-landed message
};

// EQSuccessfulHit.Flags bits. Note these differ from the heal bits above.
// The named modifiers only fire on spell damage (Skill 0xe7); melee crit/
// crippling-blow flags are decoded downstream in the chat formatter, not here.
enum : uint32_t
{
	HitFlag_Critical = 0x80,
	HitFlag_Lucky    = 0x100,
	HitFlag_Twincast = 0x20000,
	HitFlag_Locked   = 0x100000,  // overrides Lucky; the client clears the other bits when set
};

constexpr size_t kMaxDamageEvents = 256;

extern std::mutex               s_dmgMutex;
extern std::vector<DamageEvent> s_dmgEvents;
extern std::atomic<bool>        s_dmgDebug;     // /eqgfx dmgdebug chat-logging
extern bool                     s_dmgTapInstalled;

void InstallDamageTap();
void RemoveDamageTap();

// ---------------------------------------------------------------------------
// Live-captured appearance, owned by Models.cpp: the ordered material-rename
// stream + worn head recorded from the game's own appearance calls.

struct MatEvent { std::string a; std::string b; uint32_t tint; };   // normalized, no _mdf

constexpr size_t kMaxMatEvents = 4096;          // ring cap (replay is self-correcting)

extern std::mutex            s_capMutex;
extern std::vector<MatEvent> s_matEvents;       // guarded - ordered rename log
extern std::string           s_headMesh;        // guarded - worn head mesh (lower)
extern uint32_t              s_headTint;        // guarded - head dye (0xRRGGBB)
extern bool                  s_haveCapture;     // guarded
extern std::string           s_captureActorDef; // guarded - whose look this is
extern std::atomic<bool>     s_captureDirty;    // unsaved changes since last flush
extern bool                  s_captureInstalled;// detours live?
extern bool                  s_captureVerbose;  // /eqgfx capture -> also chat-log each call

bool InstallSwapCapture(bool auto_ = false);
void RemoveSwapCapture();
void SaveCapture();
bool LoadCapture(const std::string& actorDef);

// ---------------------------------------------------------------------------
// Self-parsed model view (Models.cpp owns the pipeline; the /eqgfx command and
// OnPulse in MQEQGFX.cpp drive it).

extern eqgfx::render::ModelView s_modelView;    // render thread only
extern std::atomic<bool>        s_showModel;
extern std::atomic<bool>        s_reloadModel;  // ImGui asks main thread to rebuild

bool BuildModelView(const std::string& armor, std::string& src, std::string& note);
void DumpCharCommand();
void DumpModelCommand();

// ---------------------------------------------------------------------------
// Model highlighting (Highlight.cpp): recolor a spawn's actual 3D model via
// the engine's per-actor material state. Driven from OnPulse; Lua setters
// only record intent. See Highlight.cpp for why the vtable is called by hand.

enum : uint32_t
{
	Highlight_Tint    = 0x1,   // multiply a color into every material
	Highlight_Ambient = 0x2,   // add a constant ambient - reads as a glow
};

constexpr size_t kMaxHighlights = 256;

void SetSpawnHighlight(int spawnId, uint32_t color, uint32_t mode);
void ClearSpawnHighlight(int spawnId);
void ClearAllHighlights();
int  HighlightCount();
void PulseHighlights();               // apply/reap - every pulse
void RefreshHighlights();             // rewrite after material rebuilds - slow cadence
void OnHighlightSpawnRemoved(int spawnId);
void ForgetHighlights();              // zoning: drop without restoring

// id, color, mode, applied 
std::vector<std::tuple<int, uint32_t, uint32_t, bool>> HighlightList();

// ---------------------------------------------------------------------------
// Unit frames (Units.cpp): a WoW-style secondary target ("focus") plus hover
// healing - cast at whoever the mouse is over without giving up your target.
//
// The whole feature rests on one fact about the client, established by reading
// CharacterZoneClient::CastSpell (eqgame 0x1400DD130): the outgoing cast packet
// (opcode 0xead6) carries the target's spawn ID, snapshotted from the global
// pTarget at the moment of the call -
//
//     if (pinstTarget == NULL) hdr.targetId = 0;
//     else                     hdr.targetId = pinstTarget->SpawnID;
//
// - and nothing afterwards re-reads it. So a cast can be aimed at any spawn by
// swapping pTarget, invoking the cast, and swapping straight back: the packet
// is already on the wire by the time the call returns. No /target round trip,
// no flicker, and the player keeps whatever they had selected.

struct UnitCast
{
	int      spellId;
	uint32_t startMs;      // EQGetTime() when the cast started
	uint32_t castMs;       // total cast time reported by the client
};

void InitUnits();
void ShutdownUnits();
void PulseUnits();                       // expire cast bars, deferred retarget
void DrawUnitsUI();                      // render thread, from OnUpdateImGui
void OnUnitSpawnRemoved(int spawnId);

/// Fed from the cast tap in Spells.cpp so the focus/hover cast bars start the
/// instant the packet lands rather than a pulse later.
void UnitsNoteCastStart(int spellId, int spawnId, uint32_t castTimeMs);

/// World markers for the focus target, appended by PulseClassFx before it
/// publishes. Keeping the submit vector owned in one place avoids a second
/// producer racing the swap.
void BuildUnitMarkers(std::vector<eqgfx::gpu::FxInstance>& out);

/// Handles the /eqgfx sub-commands this file owns. Returns false if szCmd
/// isn't one of ours, so the main dispatcher can carry on.
bool UnitsCommand(const char* szCmd, const char* szLine);

int  GetSecondaryTarget();
void SetSecondaryTarget(int spawnId);
int  GetHoverUnit();
void SetHoverUnit(int spawnId);
bool GetSpawnCast(int spawnId, UnitCast& out);

/// Cast memorised gem `gem` (1-based) at `spawnId`. See the note above for why
/// this does not disturb the player's target.
bool CastAtSpawn(int spawnId, int gem);

/// hover -> focus -> target, the WoW mouseover-macro fallback chain.
int  ResolveCastUnit();

/// /attarget <unit> <command> - run any target-consuming command against a
/// chosen unit. The binding is deliberately the player's to create.
void AtTargetCommand(PlayerClient* pChar, const char* szLine);
