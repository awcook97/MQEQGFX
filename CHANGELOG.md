# Changelog

## 0.3

- Unit frames for player, target, focus, group, raid, pet and extended targets, with per-window options, gem rotations on clicks and a hotkey, and `/attarget` for casting at any unit without changing target.
- Spell animations for casts, projectiles, impacts and persistent buff auras (`/eqgfx spells`).
- Target ring, auto attack indicator and optional per-class animations on nearby spawns.
- Replacement windows for spell gems, hot bars, spell book, inventory, chat and other stock windows, with per-class profiles and saved layouts (`/eqgfx ui`).
- Performance options: nametag and UI text batching, reduced animation rate for distant actors, GPU particles.
- Lua module (`require('eqgfx')`): world primitives, animated effects, projection, model highlighting, spell geometry, spell animation control, unit frame state, and cast and damage event feeds.
- Bundled Lua scripts: nameplates, spell area indicators, damage numbers and target highlighting.
- Settings saved as JSON (`MQEQGFX.json`, `MQEQGFX_<class>.json`), converted automatically from the previous ini files.
