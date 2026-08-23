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

The **Unit Frames**, **Focus**, and **Extended Targets** windows open on their
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

`CharacterZoneClient::CastSpell` (`0x1400DD130`) takes a gem and a spell but
never a target — every targeted action is aimed at the global `pTarget`. What
the decompile shows is that the outgoing packet *snapshots* it:

```c
if (pinstTarget == NULL) hdr.targetId = 0;
else                     hdr.targetId = pinstTarget->SpawnID;   // +0x168
...
CPacketScrambler::hton(pinstCPacketScrambler, 0xead6);
EQNetwork::SendPacket(__gWorld, 4, buf, len);
```

The id is read once, synchronously, into the packet, and copied onto the
character as the in-progress cast's target (`+0x250`). Nothing reads `pTarget`
again afterwards.

Separately, the client only *tells the server* about a target change from a
per-frame diff in `PlayerZoneClient::ChangeTarget` (`0x1400f3b40`):

```c
DAT_140de5b50 = pinstTarget;                       // shadow: last id sent
hton(pinstCPacketScrambler, 0x1994);
<2 bytes opcode><4 bytes spawn id>
EQNetwork::SendPacket(__gWorld, 4, buf, 6);
```

It fires **only when `pTarget` differs from that shadow**. `/attarget` swaps
`pTarget`, runs the command, and swaps back inside a single pulse — so the diff
never sees a change and **nothing goes on the wire**. The action is already
aimed; the swap is invisible to both you and the server.

This is also why the command runs with `DoCommand(..., delayed=false)`. A queued
command would execute *after* the restore and hit the wrong unit.

---

## Windows

### Unit Frames — `/eqgfx frames`

Your group, then the rest of the raid. Each row is one hit box, so hovering
anywhere on it counts as hovering that unit.

* health bar, name, class, distance
* class-coloured stripe (the same World of Warcraft palette the 3D effects use)
* cast bar in the bottom strip when the unit is casting
* buff icons underneath (toggle with the `buffs` checkbox)
* white border = hovered, purple = focus, gold = your current target

Clicks: **left** targets (or casts the click gem, if set), **right** targets,
**middle** sets focus.

| Sub-command | Effect |
| --- | --- |
| `/eqgfx frames` | toggle the window |
| `/eqgfx frames clickgem <n>` | left-click casts gem *n* instead of targeting (0 = off) |
| `/eqgfx frames raid` | include raid members beyond your group |
| `/eqgfx frames worldhover` | let hovering a spawn in the 3D world set the hover unit |
| `/eqgfx frames fallback` | `MouseHover` falls back to focus, then target |
| `/eqgfx frames restore` | hold the target swap until the cast completes (see below) |
| `/eqgfx frames probe` | ask the server for buffs on units you have not targeted |

### Focus — `/eqgfx focus`

The secondary target: name, class, health, distance, **who it is targeting**,
its cast bar, and its buffs. A purple ring is drawn under it in the world.

```txt
/eqgfx focus              set focus to your current target
/eqgfx focus <name>       set by name
/eqgfx focus clear        clear it
/eqgfx focus target       target your focus
/eqgfx focus marker       toggle the world ring
/eqgfx focus window       toggle the panel
```

The cast bar is fed by the `CEverQuest::StartCasting` tap the moment the packet
lands, so you can watch a mob's cast without ever selecting it.

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

Prints focus, hover, which windows are open, click gem, fallback and restore
modes, probe state, and how many cast bars are live.

---

## Buffs on units you have not targeted

By default the buff rows come from MacroQuest's cache, which is filled by its
detour on `CTargetWnd::RefreshTargetBuffs`. That cache is keyed by the spawn id
in the packet header — not "current target" — so it covers anyone the server
has ever sent buffs about. It sends nothing and costs nothing.

The gap: the server only *volunteers* a spawn's buffs when it believes that
spawn is your target, so the cache can never answer "what is on someone I have
never clicked".

`/eqgfx frames probe` closes that gap, and it is **off by default** because it
is the one part of this feature that puts packets on the wire. It sends the
6-byte `0x1994` target notification by hand for the unit you want, then
immediately another for your real target. The server answers the first with a
buff packet (which MQ caches by spawn id) and the second restores its
bookkeeping. `pTarget` is never touched, so nothing flickers locally.

Rate limited to one probe per 750 ms with a 15 s per-spawn cooldown. The focus
and hovered units are kept warm automatically.

**The cost, plainly:** between those two packets the server believes you are
targeting someone you are not. Anything server-side that reads your target in
that window — another player's `/assist`, for instance — sees the probe target.
It is a very small window, but it is not zero, which is why you have to ask for
it.

---

## Target restore modes

`/eqgfx frames restore` toggles how the swap is undone for **frame clicks and
the Lua `cast_at` binding**:

* **immediate** (default) — restore right after the call. Nothing on the wire,
  no visible target change. This is what makes it feel like a WoW mouseover.
* **on complete** — hold the swapped target until the cast finishes. Your
  target visibly changes. Use this only if your server re-validates the
  caster's target when the spell *lands* rather than when it is *sent*.

`/attarget` always restores immediately — correct for `/cast`, `/alt act`,
`/doability` and `/useitem`, all of which send synchronously with the target id
embedded.

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
