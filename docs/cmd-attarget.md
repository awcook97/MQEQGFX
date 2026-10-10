---
tags:
  - command
---

# /attarget

## Syntax

<!--cmd-syntax-start-->
```eqcommand
/attarget <unit> <command>
```
<!--cmd-syntax-end-->

## Description

<!--cmd-desc-start-->
Runs any command that acts on your target (`/cast`, `/alt act`, `/useitem`, `/doability`, ...) against another unit, without changing your target. Bind it to a key to heal whatever the mouse is over, or to cast at your focus.
<!--cmd-desc-end-->

The command runs immediately with the chosen unit as its target, and your own target is restored straight afterwards. If the unit does not resolve to a targetable spawn in the zone, nothing runs.

## Options

| Option | Description |
|--------|-------------|
| `2Tar` | Your focus (secondary target). `Focus` and `Secondary` also work. |
| `Group1`-`Group6` | Group slot *N*, in group window order (1 is you). |
| `Raid1`-`Raid72` | Raid roster slot *N*. Members out of zone do not resolve. |
| `XTar1`-`XTar20` | Extended target slot *N*, up to your number of slots. |
| `MouseHover` | The unit under the mouse, in a unit frame or in the 3D world. `Hover` and `MouseOver` also work. |
| `Me` / `Self` | You. |
| `Target` | Your current target. |
| `<id>` | A spawn ID. |
| `<name>` | A spawn name. |
| `<command>` | Everything after the unit. It must start with `/`. |

Macro variables on the line, such as `${Spawn[npc goblin].ID}` or `${Target.ID}`, are expanded before anything runs, so `${Target}` is always your own target.

With no arguments, `/attarget` prints its usage.

## Examples

```eqcommand
/attarget MouseHover /cast 3
/attarget 2Tar /alt act 1234
/attarget Group2 /useitem "Rod of Mending"
/attarget XTar1 /doability 4
/attarget Raid14 /cast "Complete Heal"
```
