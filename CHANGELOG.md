# Changelog

### Added

- **Nameplates: line-of-sight filter.** `/npmenu` → "Line of sight" only
  plates spawns you can actually see. Your group, raid, extended targets,
  pet, fellowship and current target each have their own exemption toggle so
  the things you track *because* they're out of sight keep their plate; your
  own plate is never hidden. The raycast answer is cached per spawn
  (0.25s by default) so a crowd in range doesn't stall the discovery loop.
- **Distance scaling** for nameplates and damage numbers: full size out to a
  near distance, shrinking to a floor at a far one. Plates scale as a whole -
  bar, name, HP %, buff icons and cast bar together.
- **Nameplates: click a plate to target that spawn** (`/npmenu` → "Click a
  plate to target that spawn", off by default). The topmost plate under the
  cursor wins; clicks owned by an ImGui window or landing on a native EQ
  window are left alone.
- **Nameplates: overlap handling** for clustered spawns (`/npmenu` →
  "Overlap"). *Separate* pushes colliding plates apart vertically; *Stack*
  collapses each cluster into one column under its most important member.
  Your own plate and your target's never move.
- **Idle gate** in all three example scripts (`/npmenu`, `/dnmenu`, `/aemenu`
  → "Idle"): stop drawing while the frame limiter isn't rendering the scene
  (on by default) or while EverQuest isn't the foreground window (off by
  default, since a backgrounded window may still be one you're watching).
  Nameplates also skip the spawn scan while paused; damage numbers keep
  draining the combat feed so resuming doesn't replay a backlog.

### Changed

- Heals and hits now go through separate report paths. `poll_damage()` events
  now track crit/lucky/twincast damage events and healing events so test the
  `/eqgfx dmgdebug` now prints hits and heals with their own fields and
  decodes the modifier tags.

### [0.3.0] - 2026-07-07

- **Damage numbers** (`/lua run eqgfx/dmgnums`): floating combat numbers over
  spawns. Melee, spells, DoT ticks, heals, and misses each get their own
  color, size, and animation style (float, arc, fountain, static), filterable
  by source (you, your pet, others, incoming) and by kind. `/dnmenu` opens the
  settings window, `/dmgnums` toggles the numbers, and `/dntest` pops a sample
  of every category over your target so you can tune the look without
  fighting.
- `eqgfx.poll_damage()`: a combat event feed for your own scripts. Every hit,
  heal, and miss the client reports, as plain Lua tables (attackerID,
  targetID, damage, spellID, skill, heal, totalHeal, timestamp). This is what
  dmgnums is built on.
- `${Spawn[...].CastTimeLeft}`: time remaining on any spawn's current cast,
  usable from macros and Lua. The plugin provides the member itself when the
  MacroQuest build doesn't, so it works either way.
- **3D portraits** (experimental): render a live character model into a UI
  window rect, WoW-style. `/eqgfx portrait self|target [on|off]` for a quick
  look; `set_portrait(slot, opts)` / `clear_portrait(slot)` /
  `portrait_slots()` from Lua. Two slots (self + target). Models currently
  render in a neutral pose without equipment; that part is still in progress.

### Changed

- Repo layout cleanup.

## [0.2.0] - 2026-07-02

### Changed

- **The native side is now a real MacroQuest plugin (`MQEQGFX.dll`)** built
  inside the MacroQuest tree, replacing the standalone `eqgfx.dll` +
  LuaJIT-FFI bridge. The core module is implemented natively and reached via
  `require('plugin.MQEQGFX')`; `require('eqgfx')` still returns the same API,
  so existing scripts are unchanged. The plugin links MacroQuest directly, so
  there is nothing game-version-specific to resolve or configure at runtime.


### Added

- `eqgfx.poll_casts()`: a cast event feed for your own scripts. Every cast
  start in the zone (spellID, spawnID, castTimeMs, timestamp), including
  instant casts and your own.
- `/eqgfx` plugin command: build stamp, render-callback health, and event
  queue counters. Diagnostics that work without any Lua running.

### Removed

- The old FFI interface, the hot-copy loader (`eqgfx_run_*.dll`), and the
  `dll_stale` machinery (the field is kept as a constant `false` for
  compatibility).

## [0.1.0] - 2026-06-11

### FFI Pass - Pre-Linux Compile, Proof of Concept

- **Nameplates** (`/lua run eqgfx/nameplates`): animated HP bars with five
  fill textures, names with anonymization modes and per-letter animations,
  buff icon rows with filters/overrides/tooltips, cast bars with spell icons
  and interrupt detection, mana/endurance bars, target styling, and a full
  in-game menu (`/npmenu`) with character/server/global save scopes.
- **AE cast highlight**: plates light up when an in-flight AE will affect
  them (orange for harmful, light blue for helpful) for your casts, other
  players', and NPCs'. Overlapping AEs deepen the color (configurable
  stacking curve, sources, colors, and animation).
- **Native UI occlusion**: plates clip around (or hide behind) open EQ
  windows instead of drawing over them; `/npui` manages and visualizes the
  detected windows.
- **Indicators** (`/lua run eqgfx/indicators`): the area of spells being
  cast drawn on the ground. Caster rings, target rings, cones, beams and
  cast lines, colored by friend/enemy, with EQBC target sharing across
  boxes and its own menu (`/aemenu`).
- Documentation: [getting started](docs/GETTING_STARTED.md),
  [core API](docs/CORE_API.md), [developing](docs/DEVELOPING.md).
