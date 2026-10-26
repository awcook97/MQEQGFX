# Unit Frames, Secondary Target, and Hover Casting

WoW-style healing and focus targeting for EverQuest: cast at a group member, a
raid member, an extended target, or whatever your mouse is over — **without
changing your target**.

Three ImGui windows and one command. All of it is bindable rather than
automatic; nothing fires that you did not set up on purpose.

---

## Quick start

```txt
/plugin MQEQGFX
```

The **Group**, **Focus**, and **Extended Targets** windows open on their
own. Then make a hotkey:

```txt
/attarget MouseHover /cast 1
```

Bind that to a key in EQ. Point at a group member — in the frames window or in
the 3D world — press the key, and gem 1 lands on them. Your target never
changes.

---

## `/attarget` — the whole feature in one command

```txt
/attarget <unit> <command>
```

| Unit selector | Resolves to |
| --- | --- |
| `2Tar` | your secondary target (focus) |
| `Group1`–`Group6` | group slot N, matching the group window (1 is you) |
| `Raid1`–`Raid72` | raid roster slot N |
| `XTar1`–`XTar20` | extended target slot N |
| `MouseHover` | whatever the mouse is over |
| `Me` / `Self` | you |
| `Target` | your current target |
| `<id>` / `<name>` | a spawn id or name |

The command is everything after the selector, passed through untouched:

```txt
/attarget MouseHover /cast 3
/attarget 2Tar       /alt act 1234
/attarget Group2     /useitem "Rod of Mending"
/attarget XTar1      /doability 4
/attarget Raid14     /cast "Complete Heal"
```

`Raid` resolves by name (the roster stores names, not spawns), so members who
are out of zone will not resolve. `XTar` is bounded by your live slot count.

### Why your target doesn't change

`/attarget` swaps your target, runs the command, and swaps back within the same
pulse. The action lands on the unit you chose, and neither you nor the server
ever sees a target change. The command runs immediately rather than queued, so
it cannot land after your target is restored.

---

## Windows

There is one window per game window it stands in for, all built from the same
row widget (`src/unitframes/`). Each row is one hit box, so hovering anywhere on
it counts as hovering that unit.

| Window | Command | Shows |
| --- | --- | --- |
| Player | `/eqgfx uf player` | you: health, mana, endurance, pet health, aggro |
| Target | `/eqgfx uf target` | your target, its target, your hate percentage on it |
| Focus | `/eqgfx uf focus` | the secondary target |
| Group | `/eqgfx uf group` | the group, roles, aggro, pet health, role menu |
| Raid | `/eqgfx uf raid` | the raid roster by group |
| Pet | `/eqgfx uf pet` | your pet and what it is attacking |
| Extended Targets | `/eqgfx uf xtarget` | the XTarget list with per-slot aggro |

`/eqgfx uf` alone lists what is open, how big it is and what each window's
clicks are bound to; `/eqgfx uf all [on|off]` moves them together. `/eqgfx frames`
and `/eqgfx xtarget` still toggle the group and extended target windows.
`/eqgfx uf <window> size <w> <h>`, `lock`, `clickthrough` and `reset` are the
same knobs the options panel has. Everything else is in the options window below.

Every row carries: health bar, name, class-coloured stripe, a cast bar when the
unit is casting, and a white border when hovered / purple for focus / gold for
your current target. Group rows add thin bars underneath — blue mana, yellow
endurance, **green pet health** — plus the member's role letters and their aggro
percentage. What else the row spells out is up to you; the defaults keep it to a
name and a health percentage and put the rest in the tooltip, because the class
colour already says the class.

### Size, emptiness and the mouse

Four things every window does, that the options panel can change:

* **Shrink to fit.** On by default: the **height** follows the row count, so
  twenty extended target slots holding four mobs are four rows of window, not
  twenty. **Width is always yours** — it is what rows are laid out at, in both
  modes, deliberately: if rows took their width *from* the window under
  auto-fit the window would be defined in terms of itself and could only ever
  grow. Turn shrink-to-fit off and the height is obeyed exactly too, and a drag
  on the resize grip is remembered. `/eqgfx uf xtarget size auto` and
  `/eqgfx uf xtarget size 320 200` switch between the two.
* **An empty box stays empty.** No target means the target window draws nothing:
  it does not tell you to click on a target, and the group window does not tell
  you that you are not in a group. `hide when empty` goes further and drops the
  frame entirely until there is something to show.
* **The frame is click-through.** It only takes the mouse when the pointer is on
  a bar (or a button), plus the `click margin` slack around it. Everywhere else
  the click goes to the game, so an idle unit frame never eats the world beneath
  it. The title bar stays grabbable, and holding **alt** hands an unlocked frame
  back whole so a frame with no title bar can still be moved.

  If a click is not reaching the game and you want to know why, turn on
  **`show what takes the mouse`**. It paints every rectangle that is currently
  clickable straight over the frame — green while the frame has the mouse,
  orange while it is passing clicks through.
* **Out of sight, out of mind.** `draw only while EverQuest has focus`, in the
  plugin options and on by default, means a background client draws no frames,
  builds no rows and reads no buff lists at all. The rest of the plugin's
  per-pulse work drops to an eighth of its rate in the background too. On a box
  running a lot of characters this is the setting that matters. Options panels
  are exempt, so the switch never hides itself.

Text is pixel-snapped by default — glyph positions and the font size are rounded
to whole pixels, which is the difference between crisp labels and the soft look
you get when a centred label lands on a half pixel. `text edge` picks none, a
drop shadow or a full outline; an outline is the only one that stays readable
over a bright health bar.

### Clicks and gems

Each window has its own three bindings, because what you cast on a group member
is not what you cast on an extended target:

```txt
/eqgfx bind <window> <hotkey|left|right> <rotation>
/eqgfx bind group hotkey 1|2|3
/eqgfx bind xtarget left 5|6
/eqgfx bind group right off
```

A **rotation** is gems in fallback order: `1|2|3` casts gem 1, or gem 2 if 1 is
on cooldown, or gem 3 if both are. Nothing fires if none of them is ready.

* **left** — casts its rotation; targets when nothing is bound
* **middle** — targets
* **right, quick click** — casts its rotation
* **right, held** — opens the unit menu instead (0.35 s)
* **hotkey** — fired by `/eqgfx cast`, which you bind to a key. It uses the
  rotation belonging to the window your mouse is over, so one key does group
  heals over a group row and a nuke over an extended target row

Pointing at a row never casts anything by itself. It records which unit and
which window are under the mouse; a click or the hotkey is what acts.

The menu targets, sets focus, opens **Options...**, and — in the group window —
assigns Main Tank, Main Assist, Puller and Mark NPC by pressing the game's own
group-window role buttons.

### Options

Each window owns its settings and has its own panel: hold right-click on one of
its rows and pick **Options...**, or type `/eqgfx uf <window> options`. Every
setting is declared once, in `src/unitframes/Options.cpp`, and the panel, the
save and the load all walk that one list — so nothing can be drawn
without being saved. There is a filter box at the top, because there are a lot
of them.

Every panel carries the sections every window has:

* **frame** — shown, locked, hidden title bar, no background, scrollbar,
  opacity, width, height, hide when empty, click through, click margin
* **bars** — bar and aux-bar heights, gaps, insets, rounding, row spacing, class
  spine width, bar colour (health gradient / class / fixed / con), fill style
  (flat, gradient, glass, stripes, segmented), drain direction, border thickness
* **text** — name, alignment, shadow, size, class or con colouring, health text
  (percent / current / current+max / deficit) and which side it sits on, class,
  level, distance and guild in the bar, role tags, name truncation, and
  **anonymisation** (class name, class short, scrambled, asterisks, first+last)
* **cast bar** — shown, inside or under the health bar, height, spell name, time
  remaining, target only
* **buff icons** — shown, size, spacing, caps, per row, borders, tooltips, timer
  text, only-mine, dim-others, beneficial/detrimental filter
* **tooltip** — every line it can carry, individually: name, real name behind an
  anonymised one, class and level, race, con, health numbers, distance, guild,
  aggro, pet, what it is targeting, its role, what it is casting, your bindings
* **animation** — health smoothing and its speed, damage flash (and heal flash)
  with threshold and length, low-health pulse with threshold and speed, a glow
  on your current target
* **colours** — all thirty of them: backgrounds, the three health stops, mana,
  endurance, pet, name, shadow, health text, tags, cast fill and text, hover,
  target, focus and idle borders, flash, low health, the three aggro tiers and
  the three buff-icon borders
* the three rotations, edited in place

— and then whatever that window alone offers: mana and endurance bars, pet
health, aggro, who to include, members out of zone, sort order, range and slot
caps, its target as a bar of its own, and the pet's buttons.

**Reset this window** puts every one of them back, keeping your gems.

### The pet's buttons

The pet window's button row is yours to define. A button is a display name, a
command, and optionally a macro-data expression that turns it into a toggle:

| Display | Cmd | Toggles |
| --- | --- | --- |
| Attack | `/pet attack` | |
| Back | `/pet back off` | |
| GHold | `/pet ghold` | `${Pet.GHold}` |

A button with a toggle expression lights up whenever that expression reads
`TRUE`; the braces are optional, so `Pet.GHold` works as well as `${Pet.GHold}`.
The editor at the foot of the pet's options panel adds, reorders and removes
them, and offers every `/pet` command worth a button as a preset. Expressions are
evaluated four times a second on the pulse, and the commands they fire are queued
for the same place — never for the render thread.

The settings that belong to no single window sit behind **Plugin options...**
at the foot of every panel, or `/eqgfx options`:

* **Windows** — what is open, locked and click-through, how big each one is,
  with a shortcut into each one's panel and buttons to lock them all at once
* **Targeting** — world hover, focus marker, the fallback chain
* **World effects** — the passive class animations and who they draw on, their
  radius, spawn cap, alpha and slot mask, the whole GPU pass, the test ring and
  the damage debug log

Settings live in `config/MQEQGFX/MQEQGFX.json`, held in memory: reading one is a lookup, and a
change is written once changes have been idle a moment, whole, off the game
thread, so dragging a slider does not write the file sixty times a second. The
first time, the file is converted from `MQEQGFX.ini`, which is left as it was.

| Sub-command | Effect |
| --- | --- |
| `/eqgfx frames [on\|off]` | toggle the group window |
| `/eqgfx frames raid` | toggle the raid window |
| `/eqgfx frames worldhover` | let hovering a spawn in the 3D world set the hover unit |
| `/eqgfx frames fallback` | `MouseHover` falls back to focus, then target |

### Focus — `/eqgfx focus`

The secondary target: name, class, health, distance, its cast bar and its
buffs. A purple ring is drawn under it in the world.

There is no "who it is targeting" row here. The client tracks one such field,
`pLocalPlayer->TargetOfTarget` (`${Me.TargetOfTarget}`, the same thing MQ calls
`${Target.Target}`), and it answers for your own target only — so that row
belongs to the target window and nowhere else.

```txt
/eqgfx focus              set focus to your current target
/eqgfx focus <name>       set by name
/eqgfx focus clear        clear it
/eqgfx focus target       target your focus
/eqgfx focus marker       toggle the world ring
/eqgfx focus window       toggle the panel
```

The cast bar starts the moment the cast does, so you can watch a mob's cast
without ever selecting it.

### Extended Targets — `/eqgfx xtarget`

Your XTarget list, each row labelled with its role (Auto Hater, My Pet's
Target, …). Same frame widget as the unit frames, so hovering a row feeds the
same hover unit — which is how `MouseHover` works for damage as well as heals:

```txt
/attarget XTar1 /cast 2
```

### Status

```txt
/eqgfx units
```

Prints focus, hover, which windows are open, click gem, fallback mode, and how
many cast bars are live.

---

## Buffs on units you have not targeted

Buffs come from the data, not from the UI. There are exactly three stores:

| Whose | Where it is read from |
|---|---|
| Yours | your own buffs and songs, the same as `${Me.Buff[n]}` and `${Me.Song[n]}` |
| Anyone else's | MacroQuest's cached buffs, the same store as `${Target.Buff[n]}` |
| Your pet's | the pet window — see below |

The cache covers anyone the server has ever sent buffs about, not only your
current target. It sends nothing and costs nothing.

**The pet.** The client keeps pet buffs in the pet window only, and
`${Pet.Buff[n]}` reads them from there too. `pet buffs from the pet window` is on
by default for that reason; turn it off and pet buffs come from the
same targeting cache as a stranger's, which means only after you have targeted
your pet.

Whichever store a row uses, the list is rebuilt on a timer (`rebuild the list
every`, 250 ms by default) rather than every frame. A buff list recomputed a
hundred times a second to show the same eight icons was most of what a row used
to cost.

The server only sends a spawn's buffs while that spawn is your target, so a unit
you have never targeted shows no buffs.

---

## Lua

```lua
local eqgfx = require('eqgfx')

eqgfx.focus()             -- current secondary target spawn id (0 = none)
eqgfx.focus(spawnID)      -- set it
eqgfx.hover()             -- spawn id under the mouse this frame
eqgfx.cast_unit()         -- hover -> focus -> target, the fallback chain

eqgfx.cast_at(spawnID, gem)      -- cast a memorised gem at any spawn

local c = eqgfx.unit_cast(spawnID)   -- nil when not casting
if c then
  print(c.spell, c.start, c.total)   -- spell id, start ms, total cast ms
end
```
