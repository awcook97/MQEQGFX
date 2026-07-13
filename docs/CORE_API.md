# Core API: building your own scripts on eqgfx

`require('eqgfx')` gives any MacroQuest Lua script engine-backed world
drawing: project world coordinates to screen pixels, draw primitives the
engine renders every frame, and read spell geometry. The bundled nameplates,
indicators, and dmgnums scripts are just examples built on this API; your
script can do anything they do.

The smallest useful script, a ring under your target:

```lua
local mq    = require('mq')
local eqgfx = require('eqgfx')

local ok, err = eqgfx.init()
if not ok then print('eqgfx: ' .. tostring(err)) return end

while true do
  eqgfx.clear()                                   -- rebuild the scene each pass
  local t = mq.TLO.Target
  if t.ID() and t.ID() > 0 and t.X() then
    eqgfx.add_circle(t.X(), t.Y(), t.Z(), 15,     -- TLO coords, as-is
                     eqgfx.argb(200, 255, 60, 60), 48)
  end
  mq.delay(50)
end
```

---

## Lifecycle

```lua
local eqgfx = require('eqgfx')  -- resolves to the MQEQGFX plugin module
local ok, err = eqgfx.init()    -- false + reason when not in game yet
...
eqgfx.shutdown()                -- on script exit (drops your queued primitives)
```

- The module is implemented by the **MQEQGFX MacroQuest plugin**;
  `require('eqgfx')` auto-loads it if needed. The plugin owns the render
  callback; `init()` just reports whether the engine is ready to draw, so
  check the return value in game and retry later rather than crashing.
- Each running Lua script gets its own independent copy of the module table
  (MQ Lua states are isolated) wrapping one shared native scene, so your
  script coexists with nameplates and indicators without coordination.
  MQ2Lua stops scripts using the module if the plugin unloads.
- Compatibility flag: `eqgfx.ui_native` is always `true` (the native EQ window
  scan is built in).

---

## Two ways to draw

| | Engine primitives (`add_*`) | ImGui overlay (`project` + ImGui) |
|---|---|---|
| Layering | **under** the native EQ UI automatically | on **top** of everything (clip to go under; see below) |
| Shapes | lines, circles, arcs, screen rects | anything ImGui can do: filled polys, text, textures/icons |
| Pacing | engine frame rate, rebuilt from your main loop | every ImGui frame; smooth, animatable |
| Colors | `eqgfx.argb(a,r,g,b)` | ImGui packed u32 (`ImGui.ColorConvertFloat4ToU32{r,g,b,a}`) |
| Used by | `tools/debug.lua`, `tools/calibrate.lua` | `nameplates/`, `indicators/` |

### Engine primitives

Your main loop owns a little retained scene: `clear()` it, `add_*` what should
exist right now, and the engine redraws it every frame until you change it.

```lua
eqgfx.clear()
eqgfx.add_circle(x, y, z, radius, color, segments)        -- ground ring (segments default 48)
eqgfx.add_arc(cx, cy, cz, radius, a0, a1, color, segments) -- a0/a1 in RADIANS
eqgfx.add_line(x1,y1,z1, x2,y2,z2, color)                  -- world-space segment
eqgfx.add_screen_line(x1,y1, x2,y2, color)                 -- raw pixels, no projection
eqgfx.add_screen_rect(x0,y0, x1,y1, color)                 -- filled pixel rect
eqgfx.set_thickness(px)                                    -- line half-width in pixels
```

Colors are `eqgfx.argb(a, r, g, b)` with 0–255 components; `eqgfx.abgr` is the
byte-swapped variant for a client that wants the reversed order. A segment
only draws while both endpoints are in front of the camera.

### ImGui overlay

Project positions yourself and draw with MQ's ImGui from an
`mq.imgui.init` callback. This is the nameplates approach, and the only way
to get filled shapes, text and spell icons:

```lua
local mq    = require('mq')
local eqgfx = require('eqgfx')
local ImGui = require('ImGui')

assert(eqgfx.init())

mq.imgui.init('target_label', function()
  local t = mq.TLO.Target
  if not (t.ID() and t.ID() > 0 and t.X()) then return end
  local sx, sy, vis = eqgfx.world_to_screen(t.X(), t.Y(), t.Z() + 4)
  if not vis then return end
  local fg = ImGui.GetForegroundDrawList()
  fg:AddText(ImVec2(sx, sy), ImGui.ColorConvertFloat4ToU32({1, 1, 0, 1}),
             t.CleanName() or '?')
end)

while true do mq.doevents() mq.delay(100) end
```

The callback runs every frame; reading positions inside it is what makes
drawings stick to moving spawns (the example scripts all do this).

---

## Projection

```lua
local sx, sy, visible = eqgfx.world_to_screen(x, y, z)
local sx, sy, infront = eqgfx.project(x, y, z)
local w, h            = eqgfx.get_screen()
```

- **Pass TLO coordinates directly** (`spawn.X(), spawn.Y(), spawn.Z()`); the
  engine's axis conventions are handled internally.
- `world_to_screen`: `visible` is true only when the point is in front of the
  camera **and** inside the viewport. Use it for "should I draw this at all"
  decisions (labels, plates).
- `project`: raw pixels with no viewport clamp; `infront` is a reliable
  per-vertex front-of-camera test even for off-screen points. Use it when
  triangulating shapes you might be standing inside (see
  `indicators/render.lua` `fill_and_outline` for the pattern).

---

## Spell geometry

```lua
local g = eqgfx.spell_geom(spellID)
-- nil, or { targetType, range, aeRange, coneStart, coneEnd }  (cone angles in degrees)

local TT   = eqgfx.TargetType                       -- PBAE, TargetArea, DirectionalCone, Beam, ...
local core = require('eqgfx.core._types')
core.CasterCentered[g.targetType]                   -- area sits on the caster
core.TargetCentered[g.targetType]                   -- area sits on the caster's target
```

The conventional radius is `aeRange` when it's > 0, else `range`. For worked
examples of every shape (rings, cones, beams, and who they affect), read
`indicators/render.lua` (drawing) and `nameplates/ae.lua` (hit-testing).

---

## Going under the EQ windows (ImGui route)

ImGui draws over the native UI. To make world drawings respect open windows,
clip them to the screen regions the windows don't cover. `lib/uirects.lua`
provides the rects:

```lua
local uirects = require('eqgfx.lib.uirects')

-- inside your imgui callback:
local rects, native = uirects.get()          -- one cheap native call; {x0,y0,x1,y1,name=...}
local drawList = ImGui.GetBackgroundDrawList()
if native and #rects > 0 then
  local sw, sh = eqgfx.get_screen()
  for _, r in ipairs(uirects.regions(rects, sw, sh)) do
    drawList:PushClipRect(ImVec2(r[1], r[2]), ImVec2(r[3], r[4]), false)
    draw_everything(drawList)                -- your drawing, once per region
    drawList:PopClipRect()
  end
else
  draw_everything(drawList)
end
```

`uirects.subtract(bbox, rects)` does the same subtraction against a single
rectangle; nameplates uses it to clip one plate at a time. Engine primitives
never need any of this; they're under the UI already.

---

## Cast tracking (`eqgfx.casts`)

`Spawn.Casting` is native only for *yourself*. For **other** spawns
`eqgfx.casts` picks its source automatically: when the plugin's cast feed is
live (see [Combat event feeds](#combat-event-feeds)) the plugin fills
`Spawn[id].Casting` / `Spawn[id].CastTimeLeft` and the tracker reads those;
when the feed isn't available on this build it falls back to the "begins to
cast" chat line. Either way it gives you start time, duration, progress, and
interrupt/finish state, one tracker per script.

```lua
local casts = require('eqgfx.casts')
casts.init{ trackSelf = true, radius = 300 }   -- do this once

while true do
  mq.doevents()
  casts.update()                               -- call once per main-loop pass
  for spawnID, info in pairs(casts.all()) do
    local pct, remain = casts.progress(info, casts.now())
    -- info.spellName, info.spellID, info.casterName, info.spellIcon,
    -- info.duration, info.isSelf, info.interrupted ...
  end
  mq.delay(50)
end
```

Every Lua state keeps its own tracker (there is no shared queue to drain, unlike
`poll_casts`), so nameplates, indicators and your script can each track casts
independently.

### API

| Call | Returns / effect |
|---|---|
| `casts.init(opts)` | apply config once; safe to call again (ignored after first) |
| `casts.config(opts)` | change config at runtime (settings menus call this each pass) |
| `casts.update()` | advance the tracker; call once per main-loop pass |
| `casts.get(spawnID)` | `CastInfo` for one spawn, or `nil` (lazily polls that spawn) |
| `casts.all()` | `{ [spawnID] = CastInfo }` for every cast in flight |
| `casts.progress(info, timeNow)` | `pct` (0..1) and `remain` (seconds ≥ 0) |
| `casts.now()` | tracker clock (`os.clock()` seconds) — pass to `progress` |
| `casts.cast_seconds(spellID)` | cast time in seconds (`Spell.MyCastTime` → `CastTime` → 3s) |
| `casts.feed_live()` | `true` when the plugin cast feed is live (other spawns read from the feed; `false` = chat-fallback path) |
| `casts.CAST_ANIMS` | exported table of casting-animation ids (informational) |

`init` / `config` options and their defaults:

| Option | Default | Meaning |
|---|---|---|
| `log` | `nil` | lwlogger-style module (`Info`/`Warn`/`Debug`); `nil` = silent |
| `trackSelf` | `true` | include your own casts (via `Me.Casting`) |
| `interruptDetect` | `true` | mark early-ending casts `interrupted` (vs dropping silently) |
| `radius` | `300` | detection radius (units) for the nearby-spawn discovery scan |
| `grace` | `1.0` | seconds a finished cast lingers before expiry |
| `interruptLinger` | `0.8` | seconds an interrupted cast lingers (fade-outs) |

`CastInfo` fields you read: `spawnID`, `casterName`, `spellID` (nil when the
plugin couldn't read it), `spellName`, `spellIcon` (A_SpellIcons atlas cell, or
nil), `startedAt`, `duration` (seconds), `isSelf`, `interrupted` /
`interruptedAt`, and `done` (a self-cast that just finished). All times are on
the `casts.now()` / `os.clock()` base — never mix them with a frame counter.

### `casts` vs. the raw feeds

| Want | Use | Why |
|---|---|---|
| Cast bars, AE/indicator geometry, interrupts, self | `eqgfx.casts` | per-script (no drain), spell names + icons + interrupts, self always works |
| One dedicated aggregator of exact cast-*start* events | `poll_casts()` | single-consumer drain, exact spawnID + start, build-conditional, no names |
| Just "time left" on a spawn from many scripts/macros | `${Spawn[id].CastTimeLeft}` | multi-consumer, remaining-time only, build-conditional |

Notes:

- Other spawns' casts are read from the plugin cast feed when it's live on this
  build (`casts.feed_live()` / `eqgfx.start_casting_addr() ~= 0`); when the feed
  isn't available the tracker detects them from the "begins to cast" chat line
  instead. The two paths differ in what they can know: the feed gives the
  server's real remaining time and needs no chat, while the chat fallback
  depends on the message not being filtered and can misresolve duplicate-name
  casters (below). Your own cast always comes from `Me.Casting` on both paths.
- Discovery scans spawns within `radius` on each `update()`; a caster farther out
  isn't auto-added, though `casts.get(id)` will lazily poll any specific spawn.
- On the chat-fallback path, duplicate-name / out-of-range casters can
  misresolve; the plugin feed keys off the spawn ID directly and avoids that.
- Initial `duration` is *your* client's cast time for that spell
  (`cast_seconds`); when `CastTimeLeft` is readable the tracker re-anchors the
  bar to the server's reported remaining time (also correct for a spawn picked
  up mid-cast).

## Persistent settings (shared lib)

The same scoped store the example scripts use (character / server / global files
under `<config>/eqgfx/`) works for your script:

```lua
local Store = require('eqgfx.lib.settings')
local settings = Store.new{ section = 'myscript', defaults = { radius = 50 } }
settings.load()
settings.data.radius = 75
settings.mark_dirty()
settings.maybe_save()               -- call every loop pass; writes only on change
```

---

## Combat event feeds

The plugin queues combat events straight from the game and hands them to Lua
as plain tables. No chat parsing, no TLO polling. Two feeds:

```lua
-- Every cast START in the zone, including instants and your own:
for _, e in ipairs(eqgfx.poll_casts()) do
  -- e.spellID, e.spawnID, e.castTimeMs, e.timestamp (game clock, ms)
end

-- Every hit, heal, and miss the client reports (this is what the dmgnums example draws):
for _, e in ipairs(eqgfx.poll_damage()) do
  -- e.attackerID, e.targetID, e.damage  (<= 0 means a miss/dodge/parry/...)
  -- e.spellID (> 0 for spell damage), e.skill (melee attack skill)
  -- e.heal (true for heals; e.attackerID = healer, e.targetID = healed,
  --         e.damage = amount landed, e.totalHeal = pre-cap)
  -- e.flags (crit/lucky/twincast bits; masks DIFFER for hits vs heals, see below)
  -- e.timestamp (game clock, ms)
end
```

Each `poll_*()` call **drains** its queue (casts hold up to 64 events, damage up
to 256; on overflow the oldest event is dropped): **one consumer per feed by
design**. If two scripts poll the same feed they split the stream and steal each
other's events — no duplication, no error. Match any `*ID` to a spawn with
`mq.TLO.Spawn(id)`. The cast feed only reports that a cast *started*;
`eqgfx.casts` (above) layers interrupt/finish detection and spell names on top.

> **Coexistence:** the bundled **dmgnums** example drains `poll_damage()` the
> whole time it runs, so any third-party script polling `poll_damage()`
> concurrently will lose events. Stop dmgnums (`/lua stop eqgfx/dmgnums`) or run
> a single aggregator that re-broadcasts. For cast bars, prefer `eqgfx.casts`
> (per-script, no drain) over `poll_casts()`.

### Event fields

`poll_casts()` entries:

| Field | Meaning |
|---|---|
| `spellID` | spell being cast |
| `spawnID` | caster (`mq.TLO.Spawn(spawnID)`) |
| `castTimeMs` | **base** cast time in ms — the client applies its own haste/focus, so treat this as ≈ `Spell(spellID).CastTime()`, **not** the caster's shortened time |
| `timestamp` | game-clock time in ms |

`poll_damage()` entries:

| Field | Meaning |
|---|---|
| `attackerID` / `targetID` | attacker and target (healer / healed on a heal) |
| `damage` | > 0 = amount dealt (or, on a heal, amount landed); ≤ 0 = a miss / dodge / parry / defense code |
| `spellID` | > 0 for spell/DoT damage (or the healing spell); 0 for a plain melee hit |
| `skill` | melee attack skill id for hits; `0` on heals |
| `heal` | `true` for heals — then `totalHeal` is the pre-cap amount, so overheal = `totalHeal - damage` |
| `flags` | crit / lucky / twincast bits — **encoding differs for hits vs heals** (below) |
| `timestamp` | game-clock time in ms |

The `flags` bit for "critical" (and lucky/twincast) is **not** the same value
on a hit as on a heal — hits and heals encode their flag bits differently. Test
the right mask for the event's type:

```lua
local HIT  = { crit = 0x80,  lucky = 0x100, twincast = 0x20000, locked = 0x100000 }
local HEAL = { crit = 0x04,  lucky = 0x08,  twincast = 0x10,    hot = 0x02 }

for _, e in ipairs(eqgfx.poll_damage()) do
  local f = e.heal and HEAL or HIT
  local isCrit = (e.flags & f.crit) ~= 0
end
```

These modifiers only appear on spell damage/heals; a normal melee crit is
decoded elsewhere by the client and isn't in the hit `flags`.

### Feed availability is build-conditional

Each feed is present only when the MacroQuest build the plugin was compiled
against exposes the matching client hook — the cast-start hook for
`poll_casts()`, and the hit/heal report hooks for `poll_damage()`. When a hook
isn't exposed, the plugin still loads normally, but that feed's `poll_*()`
**always returns an empty table**.

Check the cast feed at runtime with `eqgfx.start_casting_addr() ~= 0`, or run
bare `/eqgfx`, which prints the cast feed state (and
`StartCasting offset not available in this build` when the hook is missing).

### `${Spawn[...].CastTimeLeft}`

When the cast feed is available, the plugin also keeps each spawn's cast state
current, so `${Spawn[id].CastTimeLeft}` and `${Target.CastTimeLeft}` (time
remaining, as a timestamp) work from macros and
`mq.TLO.Spawn(id).CastTimeLeft()` works from Lua. This is build-conditional: on
builds where the plugin owns cast tracking it fills the `CastTimeLeft` member
itself; on builds where MacroQuest already tracks casts the plugin defers to it.
Either way your scripts read the same value. When the cast feed isn't available,
`CastTimeLeft` isn't present.

---

## 3D portraits (experimental)

An experimental API renders a spawn into a screen rect via the engine's object
preview. Treat it as unstable — behavior is still being validated in game.

```lua
local slots = eqgfx.portrait_slots()          -- number of portrait slots (2)

eqgfx.set_portrait(0, {                        -- slots are 0-based
  enabled = true,
  subject = 'target',                          -- 'self' | 'target' | 'none'
  window  = 'TargetWindow',                     -- track a window's rect...
  -- ...OR give an explicit pixel rect instead of `window`:
  -- x0 = 100, y0 = 100, x1 = 260, y1 = 300,
  color   = eqgfx.argb(255, 255, 255, 255),
})

eqgfx.clear_portrait(0)                        -- release a slot
```

Out-of-range slot numbers are ignored. Provide either a `window` name to track
(e.g. `'PlayerWindow'` / `'TargetWindow'`) or an explicit `x0,y0,x1,y1` pixel
rect.

