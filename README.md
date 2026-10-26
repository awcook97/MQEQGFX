# MQEQGFX

MQEQGFX is a MacroQuest plugin that draws in the EverQuest 3D world. It adds unit frames with hover casting, animated spell effects, a target ring and attack indicators, and a Lua module that lets scripts draw in the world, project world positions to the screen, recolor spawn models and read combat events.

![Nameplates marked by an AE spell in flight](docs/img/Main.png)

## Features
* NEW Focus Target window. Track casts, buffs, or just have easy access to any target of secondary importance. (You the tank & MA? Put the boss on your focus, and keep your main target on the adds.)
* Unit frames for you, your target, your focus, your group, your raid, your pet and your extended targets, each with its own options.
* `/attarget` runs any command that acts on your target against a group member, raid member, extended target, your focus or whatever the mouse is over, without changing your target. (Lag making it hard to click & taunt? `/attarget ${Spawn[theboss].ID} /doability taunt`)
* Gem rotations bound to clicks or a hotkey on each unit frame window. Hover healing & multidotting has never been more convenient.
* 70+ spell casting animations, run on the GPU. (No noticeable impact on FPS; does increase VRAM usage.)
* Multiple custom target ring options.
* Primary combat UI windows ported to ImGui for better performance (a 10-20 FPS gain over the default UI in our testing; results vary by machine).
* Fixes for poor graphical optimization:
  * Batched nametag drawing (instead of drawing spawn's nametags one-by-one, do it all in one sweep, noticeable performance boost in zones like the guild lobby)
  * Delayed distant spawn animations (don't care to get animation updates immediately? Delay them! Significant gains if the camp next door is swarming animation heavy mobs)
  * Move those particles off your CPU. Start drawing them on your GPU instead. (Big performance gains if your crew emits lots of particles.)
* Bring your 3D creations to life with the Lua module! `local eqgfx = require('eqgfx')`
* Bundled Lua scripts: nameplates, spell area indicators, damage numbers and target highlighting.

## Getting Started

```txt
/plugin MQEQGFX
```

The group, focus and extended target windows open on load. `/eqgfx help` lists every command and `/eqgfx options` opens the options window.

Copy the `lua/eqgfx` folder into your MacroQuest `lua` folder to use the bundled scripts:

```txt
/lua run eqgfx/nameplates
/lua run eqgfx/indicators
/lua run eqgfx/dmgnums
/lua run eqgfx/highlight
```

### Commands

#### /eqgfx

`/eqgfx [help | <subcommand> [options]]`

Controls unit frames, focus, spell animations, world effects, window replacement and performance options. With no arguments it prints the plugin status. Common subcommands:

```txt
/eqgfx uf <window> [on|off|options]          player, target, focus, group, raid, pet, xtarget
/eqgfx bind <window> <hotkey|left|right> 1|2|3
/eqgfx cast                                  cast the hotkey rotation at the hovered unit
/eqgfx focus [<name>|clear|target]
/eqgfx spells [on|off|self|group|all]
/eqgfx ui <window> [on|off]
/eqgfx options
```

#### /attarget

`/attarget <unit> <command>`

Runs a command against `2Tar`, `Group1`-`Group6`, `Raid1`-`Raid72`, `XTar1`-`XTar20`, `MouseHover`, `Me`, `Target`, a spawn ID or a name, without changing your target.

```txt
/attarget MouseHover /cast 3
/attarget 2Tar /alt act 1234
/attarget Group2 /useitem "Rod of Mending"
```

### Configuration File

Settings are saved in `config/MQEQGFX/`: `MQEQGFX.json` for plugin-wide settings, `classes/<class>.json` for per-class window layouts and hot buttons, and `layouts/` for saved layouts. `shaders.bin` is a compiled shader cache and safe to delete. The bundled scripts save to `config/MQEQGFX/lua/`. Files from older versions are moved there on first load.

## Other Notes

Its vcpkg dependencies are listed in `vcpkg_mq.txt`. It requires the 64-bit DirectX 11 client.

### Documentation

* [Overview, commands and settings](docs/index.md)
* [/eqgfx](docs/cmd-eqgfx.md) and [/attarget](docs/cmd-attarget.md)
* [Unit frames](docs/unit-frames.md)
* [Stock windows](docs/native-ui.md)
* [Lua API](docs/lua-eqgfx.md)
* [Lua scripts](docs/lua-scripts.md)
