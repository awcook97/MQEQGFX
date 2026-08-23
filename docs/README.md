# eqgfx docs

The **MQEQGFX MacroQuest plugin** exposes the game's own camera and renderer to
Lua. Any script can project world coordinates to screen pixels, draw primitives
in the 3D world, look up spell shapes, and read combat events. `require('eqgfx')`
is all it takes.

Three example scripts ship with it and show what you can build:

- **`nameplates`**: animated, flicker-free nameplates over nearby spawns — HP
  bar, name, a cast bar with spell and time remaining, and an in-game menu
  (`/npmenu`). Plates clip around the native EQ windows instead of drawing over
  them, and in-flight AE casts highlight the plates they will affect.
- **`indicators`**: while you or nearby mobs cast, draws the spell's affected
  area (PBAE or targeted-AE ring, directional cone, or beam) on the ground.
- **`dmgnums`**: floating combat numbers over spawns. Melee, spells, DoTs,
  heals, and misses each get their own colors and animation style (`/dnmenu`).

![nameplates marked by an AE area in flight](../assets/Main.png)

This folder is the reference documentation. For install, quick start, and the
project overview, see the [top-level README](../README.md).

---

## Guides

| Doc | What's in it |
|-----|--------------|
| [CORE_API.md](CORE_API.md) | The API guide: write your own scripts. Projection, the two drawing models, occlusion clipping, combat event feeds, and the shared cast-tracking / settings libs. Runnable examples throughout. |
| [GETTING_STARTED.md](GETTING_STARTED.md) | Install, first run, a tour of the example scripts, settings, troubleshooting. |
| [UNITS.md](UNITS.md) | Unit frames, secondary target, extended target window, and `/attarget` — cast at a group member, XTarget, or whatever the mouse is over without changing your target. |
| [DEVELOPING.md](DEVELOPING.md) | Architecture, building the plugin inside the MacroQuest tree, engine facts, and conventions. Read this before changing plugin sources. |

---

## The shape of the API

A complete script that draws a red ring under your target:

```lua
local mq    = require('mq')
local eqgfx = require('eqgfx')

local ok, err = eqgfx.init()
if not ok then print('eqgfx: ' .. tostring(err)) return end

while true do
  eqgfx.clear()
  local t = mq.TLO.Target
  if t.ID() and t.ID() > 0 and t.X() then
    eqgfx.add_circle(t.X(), t.Y(), t.Z(), 15, eqgfx.argb(200, 255, 60, 60))
  end
  mq.delay(50)
end
```

Two things to know, both covered in full by **[CORE_API.md](CORE_API.md)**:

- **Pass TLO `X(),Y(),Z()` as they are.** The plugin swaps to engine order
  internally, so you never reorder coordinates yourself.
- **Two drawing models.** World primitives (`add_circle` / `add_arc` /
  `add_line`) are drawn by the game's own renderer and land *under* the EQ
  windows automatically. For text, icons, and filled shapes, get pixels from
  `world_to_screen` and draw with MQ's ImGui — which draws *over* the UI (clip
  with the `lib/uirects` helper to go under).

---

## Caveats

- **Client support.** Developed and tested on the DirectX 11 (live) client. The
  core drawing path is expected to work on MacroQuest's emu (ROF2, DX9) branch
  too but is untested there; the model viewer and 3D portraits are DX11-only.
  Building and client compatibility — including the cone/beam heading sign-tweak
  caveat — are covered in [DEVELOPING.md](DEVELOPING.md).
