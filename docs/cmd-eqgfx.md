---
tags:
  - command
---

# /eqgfx

## Syntax

<!--cmd-syntax-start-->
```eqcommand
/eqgfx [help | <subcommand> [options]]
```
<!--cmd-syntax-end-->

## Description

<!--cmd-desc-start-->
Controls MQEQGFX: unit frames, focus and hover casting, spell animations, world effects, stock window replacement and performance options. With no arguments it prints the plugin status; `/eqgfx help` lists every subcommand.
<!--cmd-desc-end-->

Most of these settings are also in the MacroQuest settings window under plugins/MQEQGFX (or the plugin options window, `/eqgfx options`) and are saved to `MQEQGFX/MQEQGFX.json`.

## Options

### General

| Option | Description |
|--------|-------------|
| *(none)* | Print status: build, screen size, draw counters, active highlights, GPU effect state and whether the cast feed is available. |
| `help` | List every subcommand. `?` is an alias. |
| `options [off]` | Open (or close) the plugin options window. `opts` is an alias. |
| `debug [on\|off]` | Toggle the test panel: feature toggles, live statistics and previews of every spell look, class animation and effect form. |

### Unit frames and targeting

See [Unit frames](unit-frames.md) for a full description.

| Option | Description |
|--------|-------------|
| `uf` | List every unit frame window: open or closed, size, lock state and click bindings. |
| `uf <window> [on\|off]` | Show, hide or toggle a window. Windows: `player`, `target`, `focus`, `group`, `raid`, `pet`, `xtarget`. |
| `uf all [on\|off]` | Show or hide every unit frame window. |
| `uf <window> options` | Open that window's options panel. `opts` is an alias. |
| `uf <window> size <w> <h>` | Give the window a fixed size (turns shrink-to-fit off). Width 80 to 1200, height 24 to 1400. |
| `uf <window> size auto` | Size the window's height to its rows. |
| `uf <window> lock` | Toggle the window lock (no title bar, move or resize). |
| `uf <window> clickthrough` | Toggle click-through outside the rows. `ct` is an alias. |
| `uf <window> reset` | Reset the window's settings to defaults. |
| `bind <window> <hotkey\|left\|right> [<gems>\|off]` | Set the gem rotation a window casts for an action, for example `1\|2\|3`. With no gems, print the current binding; `off` clears it. |
| `cast [<window>]` | Cast the hotkey rotation of the window under the mouse (or the named window) at the hovered unit. Bind this to a key. |
| `focus` | Set your focus (secondary target) to your current target. |
| `focus <name>` | Set your focus by spawn name. |
| `focus clear` | Clear your focus. |
| `focus target` | Target your focus. |
| `focus marker` | Toggle the ring drawn under your focus. |
| `focus window` | Toggle the focus window. |
| `frames [on\|off]` | Show, hide or toggle the group window. |
| `frames raid` | Toggle the raid window. |
| `frames worldhover` | Toggle whether pointing at a spawn in the 3D world sets the hover unit. |
| `frames fallback` | Toggle whether `/eqgfx cast` falls back to your focus, then your target, when nothing is hovered. |
| `xtarget [on\|off]` | Show, hide or toggle the extended target window. |
| `units` | Print focus, hover unit, targeting options and the number of live cast bars. |

`unitframes` is an alias of `uf`, and `xt` of `xtarget`.

### Spell animations

| Option | Description |
|--------|-------------|
| `spells` | Print the current spell animation settings. |
| `spells on\|off` | Turn spell animations on or off. |
| `spells self\|group\|all` | Animate only your casts; yours, your group's and your target's; or everyone's in range. |
| `spells radius <n>` | Range, in world units, within which casts are animated (at least 10). |
| `spells persist off\|self\|group` | Persistent buff auras: none, your buffs, or your and your group's buffs. |
| `spells duration <x>` | Multiplier on how long each animation lasts (0.5 to 4, default 1.6). |
| `spells intensity <0-1>\|off` | How strongly spell level scales an animation's size and brightness, or `off` for no scaling. |
| `spells motion <0-2>` | How much persistent auras move around the body. |
| `spells fade <seconds>\|off [opacity]` | Fade persistent auras to `opacity` (0 to 1) after this many seconds. |
| `spells breathe` | Toggle a slow breathing pulse on persistent auras. |
| `spells list` | List every spell look. |
| `spells why <spell>` | Show which look a spell gets and why. Takes a spell ID or name. |
| `spells test <spell>` | Play a spell's animation from you at your target. |
| `spells clear` | Stop every running spell animation. |

`spellfx` is an alias of `spells`.

### World effects

| Option | Description |
|--------|-------------|
| `fx` | Print class animation status. |
| `fx on\|off` | Turn the per-class idle animations on nearby spawns on or off (off by default). |
| `fx pcs` / `fx npcs` | Toggle class animations on player characters / NPCs. |
| `fx radius <n>` | Range of the class animations (20 to 600). |
| `fx max <n>` | Maximum number of spawns that get a class animation (1 to 200). |
| `fx slots <mask>` | Bit mask of which of each class's animations are shown; bit 0 is the class's signature animation. |
| `fx list` | List every class's animations. |
| `fx test` | Toggle a large test ring around you. |
| `highlight [<color>] [tint\|ambient\|both]` | Recolor your target's 3D model. `<color>` is `0xAARRGGBB` (red by default); the mode defaults to `tint`. |
| `highlight off` | Remove the highlight from your target. |
| `highlight clear` | Remove every highlight. |

### Stock windows

See [Stock windows](native-ui.md).

| Option | Description |
|--------|-------------|
| `ui` / `ui list` | List the stock windows the plugin can draw and which are replaced. |
| `ui <window> [on\|off]` | Replace a stock window with the plugin's version, restore it, or toggle. The window must exist. |
| `ui all [on\|off]` | Replace or restore every listed window. |
| `ui native [on\|off]` | Keep EverQuest drawing replaced windows underneath, to check alignment. |
| `ui layout list` | List saved layouts. |
| `ui layout save <name>` | Save the current window layout, unit frames and replaced windows as a named layout. |
| `ui layout savebuttons <name>` | As `save`, including hot buttons and button sets. |
| `ui layout load <name>` | Load a saved layout. |
| `ui layout delete <name>` | Delete a saved layout. |

### Performance

| Option | Description |
|--------|-------------|
| `nametags [on\|off]` | Draw all nametags in one batch (on by default). Prints draw statistics. `perf` is an alias. |
| `uitext [on\|off]` | Draw runs of adjacent UI text together (on by default). |
| `animlod [on\|off]` | Update animations of distant actors less often (off by default). |
| `animlod distance <n>` | Distance beyond which animation updates are reduced (10 to 2000, default 100). |
| `animlod interval <n>` | Update distant animations at most every *n* frames (1 to 30, default 4). |
| `particles [on\|off]` | Draw spell and zone particles on the GPU (off by default). |
| `particles budget <n>` | Most particles drawn on the GPU per frame (4096 to 1048576, default 65536). Above it, EQ draws its particles itself for a few seconds. Buffers shrink when particle counts drop, and are released on zoning or after a few seconds unused. |

### Models (experimental)

| Option | Description |
|--------|-------------|
| `showmodel [<armor>]` | Open a model viewer window with your character's model. `<armor>` is `plate`, `chain`, `cloth`, `leather`, `ring` or `scale`. |
| `showmodel off` | Close the model viewer. |
| `portrait self\|target [on\|off]` | Show a 3D portrait in the player or target window. In development. |
| `capture [off\|save\|clear\|status]` | Log, save, clear or report the recorded appearance of your character. |
| `dumpchar` | Print your character's appearance data to chat. |
| `dumpmodel` | Print the loaded model's data to chat. |
| `dmgdebug [off]` | Log each raw hit, heal and miss event to chat. |

## Examples

```eqcommand
/eqgfx uf raid on
/eqgfx bind group hotkey 1|2|3
/eqgfx bind xtarget left 5|6
/eqgfx focus
/eqgfx spells group
/eqgfx spells test Complete Heal
/eqgfx highlight 0xFF40FF40 both
/eqgfx ui CastSpellWnd on
/eqgfx ui layout save raid
/eqgfx animlod on
```
