# Lua API

MQEQGFX exposes a Lua module for drawing in the 3D world, projecting world positions to the screen, recoloring spawn models, reading spell geometry and reading cast and damage events.

## Loading the module

```lua
local eqgfx = require('eqgfx')           -- bundled lua/eqgfx package
-- or
local eqgfx = require('plugin.MQEQGFX')  -- the plugin module directly
```

`require('eqgfx')` loads the plugin if it is not loaded yet and adds the `TargetType` table (see [Spell geometry](#spell-geometry)). It requires the `lua/eqgfx` folder in `<MQ>/lua/`. `require('plugin.MQEQGFX')` returns the plugin's table without the Lua-side additions.

Each Lua script gets its own copy of the module table. If the plugin is unloaded, scripts that required it are stopped.

### A complete script

```lua
local mq    = require('mq')
local eqgfx = require('eqgfx')

local ok, err = eqgfx.init()
if not ok then print('eqgfx: ' .. tostring(err)) return end

while true do
  eqgfx.clear()
  local t = mq.TLO.Target
  if t.ID() and t.ID() > 0 then
    eqgfx.add_circle(t.X(), t.Y(), t.Z(), 15, eqgfx.argb(200, 255, 60, 60))
  end
  mq.delay(50)
end
```

## Usage notes

### Coordinates

Pass TLO coordinates as they are: `spawn.X(), spawn.Y(), spawn.Z()`. Every function that takes a world position uses the same order as MacroQuest's TLOs; no swapping is needed.

### Colors

Colors are 32-bit ARGB values. Build them with `eqgfx.argb(a, r, g, b)`, each component 0 to 255; values outside that range are clamped. Hex literals such as `0xFFFF3030` also work. `eqgfx.abgr(a, b, g, r)` packs the components in ABGR order instead, the order ImGui colors use.

### Two ways to draw

| | World primitives (`add_*`, `fx_*`) | ImGui overlay (`world_to_screen` + ImGui) |
|---|---|---|
| Layering | Under the EverQuest windows automatically | Over everything; clip with [`eqgfx.lib.uirects`](#eqgfxlibuirects) to go under windows |
| Shapes | Lines, circles, arcs, screen rectangles, animated effects | Anything ImGui can draw: filled shapes, text, icons |
| Updates | A retained scene rebuilt from your main loop | Every ImGui frame |
| Colors | `eqgfx.argb(a, r, g, b)` | ImGui colors, for example `ImGui.GetColorU32(r, g, b, a)` |

The primitive scene and the effect list are shared by all scripts: a `clear()` or `fx_clear()` in one script also clears what another script added. Only one script should draw with them at a time. When the script that last changed the scene or the effect list stops, that scene or list is emptied. The bundled nameplates, indicators and damage numbers scripts draw with ImGui and do not use the shared scene; the highlight script does.

### Drawing with ImGui

Project positions inside an ImGui callback so drawings follow moving spawns:

```lua
local mq    = require('mq')
local eqgfx = require('eqgfx')
local ImGui = require('ImGui')

assert(eqgfx.init())

mq.imgui.init('target_label', function()
  local t = mq.TLO.Target
  if not (t.ID() and t.ID() > 0) then return end
  local sx, sy, visible = eqgfx.world_to_screen(t.X(), t.Y(), t.Z() + 4)
  if not visible then return end
  ImGui.GetForegroundDrawList():AddText(ImVec2(sx, sy), ImGui.GetColorU32(1, 1, 0, 1), t.CleanName())
end)

while true do mq.delay(100) end
```

## Lifecycle

| Function | Returns | Description |
|----------|---------|-------------|
| `init()` | `ok: boolean, err: string\|nil` | Whether the plugin is ready to draw. Returns `false` and a reason before you are in game. |
| `shutdown()` | | Empty the primitive scene immediately. Same as `blank()`. |
| `build()` | `string` | Build date and time of the loaded plugin. |
| `stats()` | `frames: integer, draws: integer` | Rendered frame counter and the number of primitives drawn in the last frame. |

## World primitives

The primitive scene is double buffered. `add_*` calls build the next scene; `clear()` shows it and starts a new one, so the visible scene is always a complete one.

| Function | Description |
|----------|-------------|
| `clear()` | Show the scene built since the last `clear()` and start a new one. Call once per main loop pass. |
| `present()` | Show the scene built so far without starting a new one. For a scene built once and left up. |
| `blank()` | Remove everything now, including the scene on screen. Use when pausing or stopping. |
| `add_circle(x, y, z, radius, color [, segments])` | Horizontal circle centered on a world point. `segments` defaults to 48, 3 to 128. |
| `add_arc(x, y, z, radius, startRad, endRad, color [, segments])` | Pie slice: an arc from `startRad` to `endRad` (radians) closed back to the center. `segments` defaults to 24, 1 to 128. |
| `add_line(x1, y1, z1, x2, y2, z2, color)` | Line between two world points. |
| `add_screen_line(x1, y1, x2, y2, color)` | Line in screen pixels. |
| `add_screen_rect(x0, y0, x1, y1, color)` | Filled rectangle in screen pixels. |
| `set_thickness(px)` | Line width in pixels for world and screen lines (default 3). |

World lines are depth tested: terrain and models in front of them hide them. Primitives and effects together are limited to 4096 per frame.

```lua
-- a scene built once and left up: a 30-unit ring around you
local me = mq.TLO.Me
eqgfx.add_circle(me.X(), me.Y(), me.Z(), 30, eqgfx.argb(255, 80, 200, 255))
eqgfx.present()

-- a quarter-circle wedge around you, 3 pixels wide
eqgfx.set_thickness(3)
eqgfx.clear()
eqgfx.add_arc(me.X(), me.Y(), me.Z(), 40, 0, math.pi / 2, eqgfx.argb(200, 255, 140, 0))
eqgfx.present()
```

## Animated effects

Effects are animated line shapes (rings, helixes, runes, lightning, glyphs and more) generated on the GPU. They are cheaper than primitives and animate on their own, so an effect is added once and stays until `fx_clear()`, until its `life` ends, or until the script stops.

| Function | Returns | Description |
|----------|---------|-------------|
| `fx_clear()` | | Remove every effect added from Lua. |
| `fx_add(form, x, y, z [, opts])` | `boolean` | Add an effect of `form` (an `FX_*` constant) at a world point. Returns `false` when `form` is not a valid form or the list is full. |
| `fx_preset(class, index, x, y, z [, phase [, alpha]])` | `boolean` | Add one of the built-in class animations (see `fx_catalog()`). `class` is a short name such as `"WIZ"`, `index` is 0-based. |
| `fx_catalog()` | `table` | The built-in class animations: `{ WIZ = { "Arcane", ... }, ... }`. |
| `fx_enabled([on])` | `boolean` | Get or set whether effects are drawn at all (the **gpu effects** option). |
| `fx_stats()` | `instances, segments, ready, error` | Effects and segments drawn last frame, whether the effect renderer is ready, and its last error. |
| `fx_now()` | `number` | The clock, in seconds, that `life` and `delay` are measured against. |

Up to 4096 effects can be added.

### `fx_add` options

| Option | Default | Description |
|--------|---------|-------------|
| `radius` | 3 | Size of the shape. |
| `height` | 4 | Height of the shape. |
| `speed` | 1 | Animation speed multiplier. |
| `tilt` | 0 | Tilt in radians, for shapes that lean. |
| `phase` | 0 | Animation offset, 0 to 1. |
| `width` | 0.05 | Line half-width in world units (pixels with `screenwidth`). |
| `segments` | 40 | Line segments to draw, 1 to 128. For `FX_GLYPH` the default draws the whole glyph and every copy. |
| `color`, `color2` | white | Color, and the far end of the color gradient. |
| `yaw` | 0 | Facing in radians, for directional forms. |
| `to` | | Second point `{x, y, z}` for `FX_SEGMENT`, `FX_LIGHTNING`, `FX_BOLT` and `FX_DRAIN`. |
| `life` | | Play once over this many seconds instead of looping. |
| `delay` | 0 | With `life`: seconds before it starts. |
| `comet` | false | A bright head chases around the shape. |
| `additive` | false | Additive blending (glow). |
| `nodepth` | false | Draw over the world instead of being hidden by it. |
| `reverse` | false | Run the animation backwards. |
| `screenwidth` | false | `width` is in pixels. |
| `noenvelope` | false | Skip the default fade in and fade out of a `life` effect. |
| `p0` - `p5` | 0 | Form-specific parameters, listed below. |

### Forms

| Constant | Shape | Parameters |
|----------|-------|------------|
| `FX_RING` | Circle in a tilted, spinning plane | |
| `FX_HELIX` | Strand climbing around a column | `p0` turns |
| `FX_ORBIT` | Short dashes on independent orbits | |
| `FX_POLYGON` | Flat polygon | `p0` sides |
| `FX_BLOOM` | Rings expanding outward and fading | |
| `FX_SPIRE` | Vertical rays fanning from the center | |
| `FX_LATTICE` | Two rings joined by verticals | |
| `FX_SEGMENT` | Line to `to` | |
| `FX_FLAME` | Flames rising around a body | `p0` tongues, `p1` base spread (0-1), `p2` length |
| `FX_VINES` | Vines spiralling up | `p0` vines, `p1` turns, `p3` 1 = curl in from outside |
| `FX_THORNS` | Spikes along vines | `p0` vines, `p1` turns, `p2` thorns per vine, `p3` as vines |
| `FX_GLYPH` | Line-art symbol | `p0` glyph (`eqgfx.GLYPH`), `p1` orientation (0 facing camera, 1 yaw plane, 2 ground, 3 facing out), `p2` copies on a ring, `p3` animation bits (`eqgfx.GLYPH_ANIM`), `p4` size |
| `FX_WINGS` | Flapping wings | `p0` feathers, `p1` flap rate, `p2` span; uses `yaw` |
| `FX_LIGHTNING` | Jagged bolt to `to` | `p0` strands, `p1` jaggedness, `p2` re-strike rate |
| `FX_BOLT` | Projectile to `to` | `p0` tail strands, `p1` spiral radius, `p2` tail length, `p3` arc height |
| `FX_RAIN` | Falling streaks | `p0` drops, `p1` streak length, `p2` slant |
| `FX_SPARKLE` | Twinkling motes | `p0` motes, `p1` size, `p2` mode (0 rise, 1 fall, 2 gather) |
| `FX_RUNE` | Magic circle | `p0`/`p1` star points/step, `p2` ticks, `p3` orientation (0 ground, 1 facing camera, 2 yaw plane) |
| `FX_VORTEX` | Funnel of strands | `p0` strands, `p1` flare, `p2` turns, `p3` spin rate |
| `FX_DOME` | Shield bubble | `p0` meridians, `p1` parallels |
| `FX_CRYSTAL` | Crystals jutting out | `p0` crystals, `p1` 1 = from the ground, `p2` length |
| `FX_DRAIN` | Motes streaming to `to` | `p0` strands, `p1` bulge, `p2` motes per strand |
| `FX_BUBBLES` | Rising bubbles | `p0` bubbles, `p1` size |
| `FX_WIND` | Streaks flowing past | `p0` streaks, `p1` length; uses `yaw` |
| `FX_BURST` | Expanding rings and rays | `p0` rings, `p1` rise, `p2` rays |
| `FX_CRACKS` | Fissures over the ground | `p0` cracks, `p1` jaggedness |

`eqgfx.GLYPH` maps glyph names to ids (`skull`, `cross`, `heart`, `shield`, `flame`, `snowflake`, `note`, ...). `eqgfx.GLYPH_ANIM` holds the animation bits `rise`, `bob`, `spin`, `pop`, `wiggle`, `ascend`, `descend` and `roll`; add them together for `p3`.

```lua
local me = mq.TLO.Me
eqgfx.fx_clear()

-- a spinning rune under you
eqgfx.fx_add(eqgfx.FX_RUNE, me.X(), me.Y(), me.Z(), {
  radius = 8, p0 = 5, p1 = 2, p2 = 24, color = eqgfx.argb(255, 120, 180, 255), additive = true,
})

-- a skull that pops in over your target and fades after 3 seconds
local t = mq.TLO.Target
eqgfx.fx_add(eqgfx.FX_GLYPH, t.X(), t.Y(), t.Z() + 8, {
  p0 = eqgfx.GLYPH.skull, p3 = eqgfx.GLYPH_ANIM.pop + eqgfx.GLYPH_ANIM.bob, p4 = 2,
  life = 3, color = eqgfx.argb(255, 255, 60, 60),
})

-- lightning from you to your target
eqgfx.fx_add(eqgfx.FX_LIGHTNING, me.X(), me.Y(), me.Z() + 4, {
  to = { t.X(), t.Y(), t.Z() + 4 }, p0 = 2, p1 = 1, p2 = 4, life = 1.5,
})
```

## Model highlighting

`highlight()` recolors a spawn's 3D model itself.

| Function | Returns | Description |
|----------|---------|-------------|
| `highlight(spawnID, color [, mode])` | | Recolor a spawn. `mode` defaults to `HIGHLIGHT_TINT`. |
| `unhighlight(spawnID)` | | Remove a spawn's highlight. |
| `clear_highlights()` | | Remove every highlight. |
| `highlights()` | `table` | `{ {id=, color=, mode=, applied=}, ... }`; `applied` is false until the highlight has been drawn. |
| `HIGHLIGHT_TINT` | `1` | Multiply the color into the model; darkens toward the hue. |
| `HIGHLIGHT_AMBIENT` | `2` | Add the color as a glow; keeps the original colors. |

The modes combine: `3` gives a lit, recolored model. Up to 256 spawns can be highlighted at once.

Highlights are not part of the primitive scene: `clear()` does not remove them. They last until `unhighlight()`, the spawn despawns, you zone, the script that set them stops, or the plugin unloads. Removing a highlight restores the default look, so a player's dyed armor shows undyed until the model reloads; prefer highlighting NPCs.

```lua
-- keep your current target highlighted
local lit
while true do
  local id = mq.TLO.Target.ID()
  if id ~= lit then
    if lit then eqgfx.unhighlight(lit) end
    if id and id > 0 then eqgfx.highlight(id, eqgfx.argb(255, 255, 40, 40), 3) end
    lit = id
  end
  mq.delay(100)
end
```

`/eqgfx highlight` does the same for your target from the command line.

## Projection

| Function | Returns | Description |
|----------|---------|-------------|
| `world_to_screen(x, y, z)` | `sx, sy, visible` | Screen pixels of a world point. `visible` is true when the point is in front of the camera and on screen. |
| `project(x, y, z)` | `sx, sy, infront` | Screen pixels without the on-screen check. `infront` is true when the point is in front of the camera, even if off screen. Use it for shapes you may be standing inside. |
| `get_screen()` | `width, height` | Screen size in pixels. |

## Spawn search

| Function | Returns | Description |
|----------|---------|-------------|
| `spawn_search(spec [, max])` | `table` | IDs of the spawns matching a MacroQuest spawn search such as `'npc radius 200 zradius 50'`, nearest first, at most `max`. You are included when you match, as with `NearestSpawn`. |

## Spell geometry

| Function | Returns | Description |
|----------|---------|-------------|
| `spell_geom(spellID)` | `table\|nil` | `{ targetType, range, aeRange, coneStart, coneEnd }`, or `nil` for an unknown spell. Cone angles are in degrees relative to the caster's facing. |
| `TargetType` | `table` | Target type values by name: `PBAE`, `TargetArea`, `DirectionalCone`, `Beam`, `Single`, `Self`, `Group_v2`, ... (added by `require('eqgfx')`). |

The area radius is `aeRange` when it is greater than 0, otherwise `range`. `eqgfx.core._types` says where an area spell is centered:

```lua
local core = require('eqgfx.core._types')
local g = eqgfx.spell_geom(mq.TLO.Me.Casting.ID())
if g then
  local radius = g.aeRange > 0 and g.aeRange or g.range
  if core.CasterCentered[g.targetType] then
    -- area sits on the caster
  elseif core.TargetCentered[g.targetType] then
    -- area sits on the caster's target
  elseif g.targetType == eqgfx.TargetType.DirectionalCone then
    -- cone from coneStart to coneEnd degrees
  end
end
```

## Spell animations

These control the plugin's own spell animations (`/eqgfx spells`).

| Function | Returns | Description |
|----------|---------|-------------|
| `spellfx_enabled([on])` | `boolean` | Get or set whether spell animations are on. |
| `spellfx_play(spellID [, casterID [, targetID [, castMs]]])` | | Play a spell's animation. Defaults to you casting at your target, instantly. |
| `spellfx_look(spellID)` | `string` | The look a spell gets, for example `"fireball + stun"`. |

A script that draws its own cast animations can check `spellfx_enabled()` so both do not animate the same cast.

## Unit frames and hover casting

| Function | Returns | Description |
|----------|---------|-------------|
| `focus([spawnID])` | `integer` | Get or set your focus (secondary target). 0 means none. |
| `hover([spawnID])` | `integer` | Get or set the hover unit (the unit under the mouse). |
| `cast_unit()` | `integer` | The hover unit, or, with fallback on, your focus, then your target. 0 when there is none. |
| `cast_at(spawnID, gem)` | `boolean` | Cast memorized gem `gem` (1-based) at a spawn without changing your target. |
| `unit_cast(spawnID)` | `table\|nil` | The spawn's current cast: `{ spell = spellID, start = startMs, total = castMs }`, or `nil` when it is not casting. |

```lua
-- heal whatever is under the mouse with gem 1
local unit = eqgfx.cast_unit()
if unit ~= 0 then eqgfx.cast_at(unit, 1) end
```

## Event feeds

The plugin queues cast starts and combat events and hands them to Lua as tables.

| Function | Returns | Description |
|----------|---------|-------------|
| `poll_casts()` | `table` | Cast starts since the last call: `{ {spellID, spawnID, castTimeMs, timestamp}, ... }`. |
| `poll_damage()` | `table` | Hits, misses and heals since the last call (fields below). |
| `cast_feed_live()` | `boolean` | Whether the plugin's cast feed is running on this client. |

Each `poll_*()` call empties its queue, so each feed should have one reader. Two scripts polling the same feed split its events between them. The cast queue holds 64 events and the damage queue 256; when full, the oldest event is dropped. The bundled damage numbers script reads `poll_damage()` while it runs. For cast bars, use [`eqgfx.casts`](#eqgfxcasts) instead of `poll_casts()`: every script gets its own tracker.

`poll_casts()` entries:

| Field | Description |
|-------|-------------|
| `spellID` | Spell being cast. |
| `spawnID` | Caster. |
| `castTimeMs` | The spell's base cast time in milliseconds, before the caster's haste and focus effects. |
| `timestamp` | Game time in milliseconds. |

`poll_damage()` entries:

| Field | Description |
|-------|-------------|
| `attackerID`, `targetID` | Attacker and target; healer and healed for a heal. |
| `damage` | Amount dealt or healed. 0 or less is a miss, dodge, parry or similar. |
| `spellID` | Spell for spell damage, DoTs and heals; 0 for melee. |
| `skill` | Melee skill for hits; 0 for heals. |
| `heal` | `true` for heals. |
| `totalHeal` | For heals, the amount before the cap: overheal is `totalHeal - damage`. |
| `flags` | Critical, lucky and twincast bits. The values differ between hits and heals. |
| `timestamp` | Game time in milliseconds. |

```lua
local HIT  = { crit = 0x80, lucky = 0x100, twincast = 0x20000 }
local HEAL = { crit = 0x04, lucky = 0x08,  twincast = 0x10 }

while true do
  for _, e in ipairs(eqgfx.poll_damage()) do
    local f = e.heal and HEAL or HIT
    local crit = bit.band(e.flags, f.crit) ~= 0
    local who = mq.TLO.Spawn(e.attackerID).CleanName() or '?'
    printf('%s %s %d%s', who, e.heal and 'healed' or 'hit', e.damage, crit and ' (crit)' or '')
  end
  mq.delay(50)
end
```

These flags are only set on spell damage and heals.

When the cast feed is available, the plugin also keeps other spawns' cast state current, so `${Spawn[id].CastTimeLeft}` and `mq.TLO.Spawn(id).CastTimeLeft()` work for any spawn. `/eqgfx` with no arguments shows whether the cast feed is available.

## EverQuest window rectangles

The plugin scans a list of window names for open EverQuest windows. Each script has its own list, and `get_ui_rects()` returns the rectangles of that script's windows. The bundled [`eqgfx.lib.uirects`](#eqgfxlibuirects) helper uses this to keep ImGui drawings under those windows.

| Function | Returns | Description |
|----------|---------|-------------|
| `set_ui_names(names)` | | Set this script's list of window names to scan. Names longer than 200 characters are dropped. |
| `get_ui_rects()` | `table` | Screen rectangles of this script's open, visible windows, clamped to the screen: `{ {x0, y0, x1, y1, idx = nameIndex, name = windowName}, ... }`, where `idx` is the position in its `set_ui_names()` list. Near-fullscreen layers are left out. |
| `ui_subtract(x0, y0, x1, y1 [, cap])` | `table\|nil` | The parts of a screen rectangle not covered by this script's windows, as `{ {x0, y0, x1, y1}, ... }`. At most `cap` pieces (default 24, up to 64); past that the smallest are dropped. `nil` when no window overlaps it. |
| `ui_stats()` | `names, found` | Names scanned and rectangles found in the last scan. |

## 3D portraits (in development)

!!! warning "In development"
    Portraits are incomplete. Behaviour and the API may change between releases.

A sample target portrait window ships as `/lua run eqgfx/tools/portrait`.

| Function | Returns | Description |
|----------|---------|-------------|
| `portrait_slots()` | `integer` | Number of portrait slots (2). |
| `set_portrait(slot, opts)` | | Configure a 0-based slot: `enabled`, `subject` (`'self'`, `'target'` or `'none'`), `window` (a window name to follow, such as `'TargetWindow'`) or a pixel rectangle `x0, y0, x1, y1`, and `color`. |
| `clear_portrait(slot)` | | Release a slot. |

```lua
eqgfx.set_portrait(1, { subject = 'target', window = 'TargetWindow' })
```

## Helper libraries

The `lua/eqgfx` package includes these libraries for your own scripts.

### eqgfx.casts

Tracks casts of you and nearby spawns, with spell names, icons, progress and interrupts. Each script has its own tracker. Other spawns' casts come from the plugin's cast feed when it is available, otherwise from the "begins to cast" chat message.

```lua
local mq    = require('mq')
local casts = require('eqgfx.casts')
casts.init({ trackSelf = true, radius = 300 })

while true do
  mq.doevents()
  casts.update()
  for spawnID, info in pairs(casts.all()) do
    local pct, remain = casts.progress(info, casts.now())
    printf('%s: %s %.0f%% (%.1fs)', info.casterName, info.spellName, pct * 100, remain)
  end
  mq.delay(50)
end
```

| Function | Returns | Description |
|----------|---------|-------------|
| `casts.init(opts)` | | Configure the tracker and register its chat events. Calling it again only updates the options, like `casts.config`. |
| `casts.config(opts)` | | Change options at run time. |
| `casts.update()` | | Advance the tracker. Call once per main loop pass. |
| `casts.get(spawnID)` | `CastInfo\|nil` | The cast of one spawn. |
| `casts.all()` | `table` | `{ [spawnID] = CastInfo }` for every cast in progress. |
| `casts.progress(info, now)` | `pct, remain` | Progress 0 to 1 and seconds remaining. |
| `casts.now()` | `number` | The tracker clock, in seconds. Pass it to `progress`. |
| `casts.cast_seconds(spellID)` | `number` | Your cast time for a spell, in seconds. |
| `casts.feed_live()` | `boolean` | `true` when other spawns' casts come from the plugin feed. |

| Option | Default | Description |
|--------|---------|-------------|
| `trackSelf` | `true` | Track your own casts. |
| `interruptDetect` | `true` | Mark casts that end early as interrupted. |
| `radius` | `300` | Range in which nearby casters are found. |
| `grace` | `1.0` | Seconds a finished cast is kept. |
| `interruptLinger` | `0.8` | Seconds an interrupted cast is kept. |
| `log` | `nil` | Logger with `Info`/`Warn`/`Debug` methods for debug output. |

`CastInfo` fields: `spawnID`, `casterName`, `spellID`, `spellName`, `spellIcon`, `startedAt`, `duration` (seconds), `isSelf`, `interrupted`, `interruptedAt` and `done`. Times use the `casts.now()` clock.

### eqgfx.lib.settings

Saves a script's settings with `mq.pickle` in the same files as the bundled scripts (`<MQ>/config/MQEQGFX/lua/`), per character, per class or for everyone.

```lua
local Store = require('eqgfx.lib.settings')
local settings = Store.new({ section = 'myscript', defaults = { radius = 50 } })
settings.load()

settings.data.radius = 75
settings.mark_dirty()

while true do
  settings.maybe_save()      -- writes only when something changed
  mq.delay(100)
end
```

| Function | Description |
|----------|-------------|
| `Store.new({ section, defaults })` | Create a store for one section. |
| `store.load()` | Load the section from the character, class or global file (first found), with defaults filled in. Returns `store.data`. |
| `store.data` | The live settings table. |
| `store.mark_dirty()` | Flag a change. |
| `store.maybe_save()` | Save if a change was flagged and nothing changed for half a second. |
| `store.save()` | Save now. |
| `store.set_scope(scope)` | Save to `'char'`, `'class'` or `'global'` from now on, removing the section from the more specific files. |

### eqgfx.lib.uirects

Keeps ImGui drawings under the EverQuest windows by clipping them to the parts of the screen the windows do not cover.

| Function | Returns | Description |
|----------|---------|-------------|
| `uirects.get()` | `table` | Rectangles `{x0, y0, x1, y1, name = ...}` of the open windows. The result is reused for 100 ms. |
| `uirects.regions(width, height [, cap])` | `table` | The visible parts of the screen, as rectangles. |
| `uirects.visible(x0, y0, x1, y1 [, cap])` | `table\|nil` | The visible parts of one rectangle, or `nil` when no window overlaps it. |
| `uirects.add_names(names)` | | Add window names to the scan (find names with `/windows`). |

```lua
local uirects = require('eqgfx.lib.uirects')

mq.imgui.init('under_windows', function()
  local drawList = ImGui.GetBackgroundDrawList()
  local rects = uirects.get()
  if #rects > 0 then
    local sw, sh = eqgfx.get_screen()
    for _, r in ipairs(uirects.regions(sw, sh)) do
      drawList:PushClipRect(ImVec2(r[1], r[2]), ImVec2(r[3], r[4]), false)
      draw_everything(drawList)
      drawList:PopClipRect()
    end
  else
    draw_everything(drawList)
  end
end)
```

World primitives and effects do not need this; they are drawn under the windows already.
