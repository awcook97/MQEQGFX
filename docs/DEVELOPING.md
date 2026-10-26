# Developing eqgfx

How the pieces fit, how the native plugin is built, and the engine facts that
were expensive to learn. Read this before touching `src/` or the
occlusion path.

---

## Layout

```
src/                the MacroQuest plugin implementing the core module:
                      MQEQGFX.cpp        lifecycle, scene + render callback, /eqgfx, Lua module
                      CombatNumbers.cpp  poll_damage() event feed
                      Spells.cpp         poll_casts() event feed + cast tracking
                      Models.cpp         model viewer + appearance capture (experimental)
                      MQEQGFX.h          state shared between those files
                      model/             standalone EQ model-file readers + DX11 view
CMakeLists.txt      plugin build target
lua/eqgfx/          the Lua package, deployed to <MQ>/lua/eqgfx:
  init.lua            entry shim: require('eqgfx') -> require('plugin.MQEQGFX')
  core/               _types.lua: TargetType enums + LuaLS annotations
  casts/              shared cast tracker: plugin per-spawn feed when live, else
                      "begins to cast" chat fallback, plus Me.Casting for self
  nameplates/         plates: init (loop/occlusion), render, anim, ae, menu, settings, _types
  indicators/         ground AE drawing: init (loop/EQBC), render, menu, settings, _types
  dmgnums/            floating combat numbers: init, menu, settings, _types
  lib/                settings store, uirects (occluder rects), ui_windows (name list),
                      lwlogger, EQBC helpers
  tools/              calibrate.lua, debug.lua - projection/render diagnostics;
                      portrait.lua - runnable ObjectPreview overlay (/lua run eqgfx/tools/portrait)
  test/               smoke.lua
```

Each feature keeps its enums/classes in a `_types.lua`. The codebase is
LuaLS-clean. A `.luarc.json` is not tracked in the repo (a fresh clone won't
have one), so set up your own targeting LuaJIT plus the mq-definitions
library, and keep `lua-language-server --check .` at zero problems.

---

## The three core problems

1. **World → screen.** The plugin calls the engine's own
   `CCamera::ProjectWorldCoordinatesToScreen` via `pDisplay->pCamera`. EQ
   stores positions as `(Y, X, Z)` — every world-space entry point swaps the
   first two components before projecting. `eqgfx.world_to_screen` clamps to
   the viewport and returns `visible`; `eqgfx.project` returns raw pixels
   plus a reliable in-front-of-camera bool (use it when triangulating rings
   you may be standing inside).

2. **Drawing.** World geometry (indicators) goes through MQ's
   `AddRenderCallbacks` → `GraphicsSceneRender` and the engine's 2D overlay
   (`CRenderInterface::DrawLine2D` on `g_pDrawHandler`); the legacy DX9
   `DrawLine3D` path fires but never composites on the DX11 client — don't
   try to revive it. Nameplates draw through MQ's ImGui instead
   (double-buffered, animatable), inside a fullscreen pass-through window
   pinned to the back of the ImGui stack.

3. **Lua ↔ native.** MQ2Lua resolves `require('plugin.MQEQGFX')` by calling
   the plugin's exported `CreateLuaModule(sol::this_state, sol::object&)` —
   one sol2 table per Lua state, every table wrapping the same shared native
   scene. MQ2Lua registers the module as a dependency of the requiring
   script, so scripts stop cleanly when the plugin unloads.

---

## Engine facts (learned the hard way)

- **`CXWnd::GetScreenRect` returns render pixels** on this client — verified
  in-game. Do not "calibrate" them: `EQMainWnd` is *not* the fullscreen
  canvas (it's the small EQ-button window), and two separate attempts to
  scale rects against it corrupted perfectly good data. `lib/uirects.lua`
  applies the identity transform, clamps to the screen, and drops only empty
  leftovers and near-fullscreen layers.
- **The engine globals go null at character select.** `pDisplay`,
  `g_pDrawHandler` and friends are eqlib `ForeignPointer`s — check `.get()`
  per frame, never cache across frames. The render callback additionally
  gates on `GetGameState() == GAMESTATE_INGAME`.

---

## Building the plugin

A normal MacroQuest plugin plus sol2/LuaJIT for the Lua module. On
Linux/msvc-wine, `./build-linux.sh` (its `MQEQGFX` case block adds the
sol2/LuaJIT flags).

**No game restart needed to update**: `/plugin MQEQGFX unload`, replace the
DLL, `/plugin MQEQGFX`. MQ2Lua stops dependent scripts on unload.

---

## Compatibility

MQEQGFX is developed and verified only on the DirectX 11 live client.
MacroQuest's emu (ROF2) branch carries the same Lua-module handoff
(`CreateLuaModule`) and the same 2D-overlay/projection interfaces, so the
core drawing path likely ports — but that is untested and unverified in
game. Known gaps on emu/ROF2:

- **Cast and damage feeds compile out.** `poll_casts()` and `poll_damage()`
  depend on client hooks the MacroQuest build's eqlib may or may not export.
  On a build whose eqlib doesn't export them (e.g. an emu/ROF2 eqlib without
  the cast-start / hit-heal hooks) the feeds are compiled away — `poll_casts()`
  returns empty, no `CastTimeLeft` is added, and the damage feed is absent.
  The `eqgfx.casts` tracker works on any build (plugin feed when live, chat
  "begins to cast" fallback otherwise) and is the portable alternative for
  cast tracking.
- **The model viewer is DX11-only.** It uses raw D3D11 and has no DX9 path,
  so it would not compile against a DX9 tree without guarding.
- **3D portraits and appearance capture are x64-specific and unproven** off
  the live client.
- **Cone/beam facing uses EQ heading** and may need a sign tweak per client
  (the directional shapes derive their facing from the caster's heading).

---

## Threading and timing rules

- The DLL's UI window sweep runs on the **render thread** (inside the
  `GraphicsSceneRender` callback) where engine calls are legal; Lua only
  snapshots the cached result (`get_ui_rects`) under a small mutex.
- **Never put a Window-TLO sweep in script loops.** A full TLO window scan
  was measured stalling the Lua loop for hundreds of milliseconds, starving
  `mq.doevents()` and lagging cast detection by seconds. The native scan is
  the single source of occluder rects.
- Every script runs on one clock, `mq.gettime()` (monotonic milliseconds);
  `casts.now()` is that in seconds. Nameplate and damage-number animation,
  the cast tracker and highlight all read it, so animation speed does not
  depend on frame rate.
- The main loops pace with `mq.delay(50)`. Spawn discovery and buff scans are
  time-gated (discovery every 0.2 s, buffs once a second per plate); the
  per-frame draw callback does the position reads so plates track movement
  exactly.

---

## Nameplates internals

- **Render caps probing** (`render.probe_caps`): the MQ ImGui binding's
  fancier calls (sized text, multi-color rects, clip rects, texture
  animations) vary by build, so each is pcall-probed once and the renderer
  degrades per-feature. Check `R.caps` before using any of them.
- **Occlusion** (`init.lua`): each plate records its real drawn footprint
  into `plate.ext` (geometry cursors + text/cast-bar extents). Next frame,
  that bbox is tested against the occluder rects; overlapping plates are
  either skipped (HIDE) or drawn once per visible sub-rectangle with
  `PushClipRect` (CLIP) — `uirects.visible` (the native `ui_subtract`)
  punches the windows out of the bbox. Over-estimating the bbox is harmless; under-estimating leaks pixels
  for one frame.
- **Draw order**: plates are gathered, then painted far → near; your target
  ranks above everything except your own plate.
- **AE highlight** (`ae.lua`): every active cast becomes an `AeState`
  (circle/cone/beam + who it `marks`); `draw_plates` counts how many areas
  cover each plate and maps the count onto the stack alpha curve
  (`aehl.stackBase/Step/Max`), smoothed by `anim.approach`. Detrimental
  marks the caster's enemies, beneficial its allies; group-target spells
  resolve only for my own group; target-centered casts from others need the
  caster targeted (`Target.AggroHolder`). Spell geometry and the
  beneficial flag are cached per caster+spell.

## Indicators internals

- Shapes follow `eqgfx.spell_geom` (target type, range, aeRange, cone
  angles); the caster-centered/target-centered sets live in
  `core/_types.lua` and are shared with the nameplates highlight so both
  always agree.
- Cross-box target resolution rides EQBC: whoever has the caster targeted
  broadcasts `eqgfx_cast=<casterID>=<spellID>=<targetID>`, and every box
  draws the target ring (stub entries handle casters outside local
  detection range).
- World drawing is clipped to `uirects.regions(sw, sh)` — the screen
  minus the open windows — so areas render under the native UI.

---

## Settings

`lib/settings.lua` is a tiny scoped store: one file per scope
(char/server/global) shared by all features, each feature under its own
section key. `merge_defaults` fills in new fields on upgrade, so adding a
setting is just: add the default in `<feature>/settings.lua`, read it where
needed, add a widget in `<feature>/menu.lua` (the helpers mark the store
dirty; the main loop saves when dirty).

## Conventions

- `_types.lua` per feature for enums/classes; LuaLS annotations everywhere;
  zero `lua-language-server --check` problems.
- TLO reads that can fail are pcall-wrapped (see the `getn/getstr/getbool`
  helpers); members differ across MQ builds more than you'd hope.
- New symbols are resolved by name at runtime, with a fallback and a way to
  see the failure (`/npdebug`).
