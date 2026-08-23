# Changelog

### Added

- **highlight: per-NPC ambient animation.** Every NPC in line of sight can be
  given its own idle effect. The default mode draws a **signature animation
  per class** — sixteen of them, one for each EQ class, in **World of Warcraft
  class colors** mapped onto the closest EQ analogue (Bard, Beastlord and
  Berserker have no WoW counterpart and take its item-quality colors). A
  random mode instead hashes a style and palette color out of each spawn ID,
  so the crowd varies while any one spawn animates smoothly. Line-of-sight
  filtered, capped, nearest-first, and off by default.
- **highlight: cast animations.** Every cast gets an animation whose shape
  comes from the spell's real footprint via `spell_geom` — beam, cone (through
  its actual angles), caster-centered shockwave, target-centered collapse,
  single-target bolt, self column — and whose color comes from what the spell
  does: the configured heal color for beneficial, resist-type colors for
  detrimental. Casts animate while the bar fills, burst on landing, and unwind
  when interrupted.
- **highlight: a shared frame budget.** All the world drawing shares one
  segment cap, served role markers → cast effects → ambient, so a crowded zone
  degrades by thinning decoration instead of dropping what matters. `/hlmenu`
  shows it as a live bar.
- **New example script: `/lua run eqgfx/highlight`.** Recolors your target's
  3D model by what it is — purple for whatever the main assist is targeting,
  blue for players, gold for pets, red for an NPC that is currently
  aggressive, white for one that is not — and draws an animated line figure
  around the main tank (counter-rotating great circles with comet heads
  chasing around them, inside a breathing ground ring) and the main assist (a
  double helix rising out of a spinning triangle, brightness flowing upward).
  `/hlmenu` has the full settings window: per-role colors, tint/glow mode,
  marker shapes, speeds, segment counts and save scope, plus a status panel
  reporting which spawns the group/raid role lookups resolved to and a test
  mode that draws both markers on you. `/hl`, `/hl markers`, `/hl tints` and
  `/hl test` toggle the pieces from chat.
- **Model highlighting.** `eqgfx.highlight(spawnID, color[, mode])` recolors a
  spawn's actual 3D model through the engine's per-actor material state — a
  tint multiplied into every material (`mode` 1), a constant ambient that
  reads as a glow (2), or both (3). `unhighlight()`, `clear_highlights()` and
  `highlights()` manage them; `/eqgfx highlight` applies one to your target
  for a quick look. Highlights live outside the retained scene, so `clear()`
  leaves them alone; they are dropped on despawn, zone and plugin unload.
  Note that removing one writes back neutral defaults rather than the previous
  values, which resets dyed armor on PCs until their model reloads.
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

- **The retained scene is double buffered.** `add_*` now writes a buffer the
  render thread is not reading, and `clear()` swaps it in. Previously the
  scene mutex was taken and released once per primitive, so the render
  callback could — and routinely did — fire part way through a rebuild and
  draw a half-built scene. That was visible as flicker on anything animated.
  Scripts need no change: `clear()` is the frame boundary, and the scene is
  one build behind, which is not perceptible. `present()` publishes without
  starting a new build (for a scene you set up once), and `blank()` empties
  both buffers immediately (for pausing, where `clear()` alone would leave the
  last frame up for one more pass). Locking dropped from once per line to once
  per frame as a side effect.
- **World segments are clipped at the camera's near plane instead of being
  dropped.** Previously `add_line`/`add_circle`/`add_arc` discarded any
  segment with an endpoint behind the camera, so a ring you were standing
  inside lost the part that passed behind you. The visible portion is now
  drawn, with the cut found by bisecting the segment against the camera's
  own projection test.

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
