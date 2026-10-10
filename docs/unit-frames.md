# Unit frames

MQEQGFX provides unit frame windows for you, your target, your focus, your group, your raid, your pet and your extended targets. Clicking a row, or pressing a bound key while hovering it, casts at that unit without changing your target.

## Windows

| Window | Command | Shows |
|--------|---------|-------|
| Player | `/eqgfx uf player` | You: health, mana, endurance, pet health, aggro. |
| Target | `/eqgfx uf target` | Your target, its target, and your hate on it. |
| Focus | `/eqgfx uf focus` | Your focus (secondary target), its cast bar and buffs. |
| Group | `/eqgfx uf group` | Your group, with roles, aggro, pet health and a role menu. |
| Raid | `/eqgfx uf raid` | The raid, by group. |
| Pet | `/eqgfx uf pet` | Your pet, what it is attacking, and your pet buttons. |
| Extended Targets | `/eqgfx uf xtarget` | Your extended target list, with the role of each slot and per-slot aggro. |

The group, focus and extended target windows are open by default. `/eqgfx uf` lists every window with its state, size and bindings; `/eqgfx uf all on|off` shows or hides them together.

Every row has a health bar, name, class-colored stripe, and a cast bar while the unit casts. The border is white while hovered, purple for your focus and gold for your current target. Group rows add thin mana, endurance and pet health bars, role letters and aggro percentage.

## Clicks and gem rotations

Each window has three bindings, so a group window can heal while the extended target window nukes:

```eqcommand
/eqgfx bind <window> <hotkey|left|right> <rotation>
/eqgfx bind group hotkey 1|2|3
/eqgfx bind xtarget left 5|6
/eqgfx bind group right off
```

A rotation is a list of gems in fallback order: `1|2|3` casts gem 1, or gem 2 if gem 1 is not ready, or gem 3 if neither is. Nothing is cast when no gem in the rotation is ready.

| Input | Action |
|-------|--------|
| Left click | Casts the left rotation; targets the unit when nothing is bound. |
| Middle click | Targets the unit. |
| Right click (quick) | Casts the right rotation. |
| Right click (hold) | Opens the unit menu. |
| `/eqgfx cast` | Casts the hotkey rotation of the window under the mouse. Bind it to a key. |

Hovering a row never casts anything on its own; it only sets the hover unit. The unit menu can target the unit, set it as focus, open the window's options, and in the group window assign Main Tank, Main Assist, Puller and Mark NPC.

For anything other than a gem, use [/attarget](cmd-attarget.md), for example `/attarget MouseHover /alt act 1234`.

## Focus

The focus is a second target that you keep while targeting something else. The focus window shows its name, class, health, distance, cast bar and buffs, and a purple ring is drawn under it in the world.

```eqcommand
/eqgfx focus              set focus to your current target
/eqgfx focus <name>       set focus by name
/eqgfx focus clear        clear focus
/eqgfx focus target       target your focus
/eqgfx focus marker       toggle the world ring
/eqgfx focus window       toggle the focus window
```

Use `/attarget 2Tar <command>` to act on your focus.

## Hover unit

The hover unit is set by pointing at a unit frame row, or at a spawn in the 3D world (`/eqgfx frames worldhover` toggles the world part). `/attarget MouseHover` uses it directly. `/eqgfx cast` falls back to your focus, then your target, when nothing is hovered; `/eqgfx frames fallback` turns that off.

## Window behavior

* **Shrink to fit**: on by default. The window height follows the number of rows; the width is always yours. `/eqgfx uf <window> size <w> <h>` sets a fixed size, `size auto` switches back.
* **Empty windows**: a window with nothing to show stays empty. The `hide when empty` option hides it entirely.
* **Click-through**: a frame only takes the mouse over its bars and buttons (plus the `click margin`); everywhere else, clicks go to the game. The title bar stays grabbable, and holding Alt makes an unlocked frame fully clickable so it can be moved. The `show what takes the mouse` option outlines the clickable areas.
* **Background clients**: `draw only while EverQuest has focus` (plugin options, on by default) stops drawing the frames while the client is in the background.

## Options

Each window has its own options panel: hold right-click on one of its rows and pick **Options...**, or use `/eqgfx uf <window> options`. The panel has a filter box and these sections:

* **frame**: shown, locked, hidden title bar, background, scrollbar, opacity, size, hide when empty, click-through, click margin.
* **bars**: bar heights, gaps, rounding, row spacing, class stripe width, bar color (health gradient, class, fixed or con), fill style (flat, gradient, glass, stripes, segmented), drain direction, border.
* **text**: name, alignment, shadow, size, class or con coloring, health text (percent, current, current and max, deficit), class, level, distance, guild, role tags, name truncation and name anonymization.
* **cast bar**: shown, inside or under the health bar, height, spell name, time remaining, target only.
* **buff icons**: shown, size, spacing, count, icons per row, borders, tooltips, timers, only mine, dim others, beneficial or detrimental filter.
* **tooltip**: each line individually, including class and level, race, con, health, distance, guild, aggro, pet, target, role, current cast and your bindings.
* **animation**: health smoothing, damage and heal flash, low health pulse, target glow.
* **colours**: backgrounds, health stops, mana, endurance, pet, text, cast bar, borders, flash, low health, aggro tiers and buff borders.
* **rotations**: the window's three gem rotations.

Some windows add their own options, such as mana and endurance bars, pet health, aggro, which members to include, out of zone members, sort order, range and slot limits. **Reset this window** restores the defaults but keeps your gem rotations.

### Pet buttons

The pet window has a row of buttons you define. Each button has a label, a command and an optional toggle expression:

| Label | Command | Toggle |
|-------|---------|--------|
| Attack | `/pet attack` | |
| Back | `/pet back off` | |
| GHold | `/pet ghold` | `${Pet.GHold}` |

A button with a toggle expression is lit while the expression is `TRUE`; the `${ }` is optional. The editor at the bottom of the pet window's options panel adds, reorders and removes buttons and offers presets for the common `/pet` commands.

## Plugin options

`/eqgfx options`, or **Plugin options...** at the bottom of any window's panel, opens the settings that are not tied to one window:

* **Windows**: which windows are open, locked and click-through, their sizes, and a shortcut to each options panel.
* **Native UI** and **UI layouts**: see [Stock windows](native-ui.md).
* **Attack indicator**: how auto attack is shown, from a simple icon or badge, through a flashing border or pulsing ring, to animated effects between you and your target.
* **Target ring**: the ring under your target: shown, size, color, red while attacking, pulse, glow, visible through walls, hidden on yourself.
* **Targeting**: world hover, focus marker and fallback.
* **World effects**: class animations and their range, spawn limit, opacity and motion; GPU effects; the test ring.
* **Performance**: nametag batching, UI text batching, animation distance reduction, GPU particles and their budget.
* **Spell animations**: the settings behind `/eqgfx spells`.

## Buffs on other units

Your own buffs are always available. Your pet's buffs are read from the pet window while `pet buffs from the pet window` is on (the default). Other units' buffs come from MacroQuest's buff cache, which is filled when the server sends a unit's buffs, normally after you have targeted it. Buff lists are refreshed on a timer (`rebuild the list every`, 250 ms by default).


## Lua

The unit frame state is also available to Lua scripts:

```lua
local mq    = require('mq')
local eqgfx = require('eqgfx')

local focusID = eqgfx.focus()          -- current focus spawn ID, 0 for none
eqgfx.focus(mq.TLO.Target.ID())        -- set focus
local hoverID = eqgfx.hover()          -- spawn ID under the mouse
local unit    = eqgfx.cast_unit()      -- hover, then focus, then target
eqgfx.cast_at(unit, 1)                 -- cast gem 1 at it without changing target
```

See [Lua API](lua-eqgfx.md#unit-frames-and-hover-casting).
