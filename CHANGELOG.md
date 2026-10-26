# Changelog

## Unreleased

### Added

- Spell animations for every class: spell lines, AAs, pets, swarm pets. Scale with spell level.
- Heals, instant casts and your own AAs animate. Buff auras stay up while the buff lasts.
- Stock window replacements (`/eqgfx ui`): spell gems, hot bars, spell book, chat, actions, combat abilities, inventory, bags, AAs, bandolier, key rings, mercenary.
- Hot bars: twelve, any size or shape, drag anything onto them.
- Per-class profiles and saved UI layouts (`/eqgfx ui layout`).
- Unit frame menus: group roles, xtarget menu, click buffs to remove.
- Target ring, attack indicators, attack flash on your health bar.
- GPU particles, nametag batching, UI text batching, animation LOD.
- Plugin options in MacroQuest's settings window.
- Lua: `get_eye`, `world_to_camera`, `dump_matrix`, `spellfx_*`, new `fx_add` forms and glyphs.
- Highlight script: per-class ambient animations, cast animations, shared frame budget.
- Nameplates: line of sight, click to target, overlap handling, distance scaling.

### Changed

- Settings live in `config/MQEQGFX/`. Old files move there on first load.
- Settings are JSON, held in memory, written in the background. Old ini files convert.
- Chat tabs use EQ's own filters. Chat input only sends slash commands.
- Buff auras and class animations have separate settings.
- Cooldowns read straight from the game; smooth sweeps.
- Shaders compile off the game thread and are cached. Reload no longer freezes.
- Scene is double buffered: no flicker.
- World lines clip at the camera instead of vanishing.
- Hits and heals are separate events in `poll_damage()`.

### Fixed

- Chat wraps long lines with links.
- AA window lists every ability.
- `fx_add` effects no longer vanish after one pulse.

## 0.3.0 - 2026-07-07

- Damage numbers (`/lua run eqgfx/dmgnums`).
- `eqgfx.poll_damage()` combat feed.
- `${Spawn[...].CastTimeLeft}`.
- 3D portraits (experimental).

## 0.2.0 - 2026-07-02

- Native MacroQuest plugin (`MQEQGFX.dll`) replaces `eqgfx.dll` and the LuaJIT-FFI bridge. Scripts unchanged.
- `eqgfx.poll_casts()` cast feed.
- `/eqgfx` status command.
- Removed: FFI interface, hot-copy loader, `dll_stale` (kept as `false`).

## 0.1.0 - 2026-06-11

- Nameplates, AE cast highlight, native UI occlusion.
- Indicators (`/lua run eqgfx/indicators`).
- Docs: [getting started](docs/GETTING_STARTED.md), [core API](docs/CORE_API.md), [developing](docs/DEVELOPING.md).
