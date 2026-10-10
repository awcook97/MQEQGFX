# Stock windows

MQEQGFX can draw selected EverQuest windows with its own versions. A replaced window is hidden and passes clicks through, but stays open and keeps working as the game's own, so keybinds, page keys and anything that reads it continue to work. No window is replaced until you turn it on.

## Replacing a window

```eqcommand
/eqgfx ui                     list the windows and which are replaced
/eqgfx ui CastSpellWnd on     replace the spell gem bar
/eqgfx ui CastSpellWnd off    restore the stock window
/eqgfx ui all on              replace every listed window
/eqgfx ui all off             restore every stock window
```

The same list is in the plugin options window (`/eqgfx options`, **Native UI**).

| Window | Replacement |
|--------|-------------|
| `PlayerWindow`, `TargetWindow`, `TargetOfTargetWindow`, `GroupWindow`, `PetInfoWindow`, `ExtendedTargetWnd`, `RaidWindow`, `CastingWindow`, `BuffWindow`, `ShortDurationBuffWindow` | Hidden; the [unit frames](unit-frames.md) show the same information. Additions other plugins make to these windows, such as MQ2TargetInfo and MQ2XTarInfo, stay on the hidden window. |
| `CastSpellWnd` | Spell gem bar. |
| `HotButtonWnd`, `HotButtonWnd2` ... | Hot bars with the plugin's own hot buttons. |
| `SpellBookWnd` | Spell book. |
| `ActionsWindow` | Actions window. |
| `CombatAbilityWnd` | Combat abilities. |
| `AAWindow` | Alternate abilities. |
| `InventoryWindow` | Inventory. |
| `Bags` | Open bags. |
| `ChatWindows` | Chat, with its own tabs. |
| `BandolierWnd` | Bandolier. |
| `KeyRingWnd` | Key rings. |
| `MMGW_ManageWnd` | Mercenary management. |
| `CompassWindow`, `SelectorWnd` | Redrawn in place; clicks still go to the stock window. |

Other window names can be given to `/eqgfx ui <name> on`; they are redrawn in place from their own controls. Use MacroQuest's `/windows` command to find names.

`/eqgfx ui native on` keeps EverQuest drawing the stock windows underneath the replacements, to check that they line up.

## Button bars

The gem bar and hot bars share one layout: the stock window's arrangement or a grid, with button size, spacing, columns, background opacity, labels, key labels and a lock. Right-click a bar's background for these options. In grid mode the bar resizes freely and the number of columns follows its width.

Hot buttons are the plugin's own: each has a label, an icon and a list of commands. There are as many bars, pages and slots as EverQuest has, the page shown is EverQuest's current page, and EverQuest's hot bar keys run the plugin's buttons. The first time a class has no hot buttons, EverQuest's current bars are imported. Buttons can be made by dragging from the cursor, a spell gem or the spell book, and edited from the button's context menu.

## Per-class profiles

Window positions, bar layouts, hot buttons, button sets and chat tabs are kept per class in `MQEQGFX_<class>.json` (for example `MQEQGFX_shm.json`) in the MacroQuest `config` folder. Every character of that class shares it, and the profile switches when you log in a character of another class.

## Layouts

A layout is a snapshot of the class profile, the unit frame settings and the list of replaced windows, saved to `MQEQGFX_layouts/<name>.json`:

```eqcommand
/eqgfx ui layout list
/eqgfx ui layout save raid
/eqgfx ui layout savebuttons raid-full
/eqgfx ui layout load raid
/eqgfx ui layout delete raid
```

`save` leaves hot buttons and button sets out; `savebuttons` includes them. Loading a layout replaces the matching parts of the current class profile. Layouts can also be managed from the plugin options window under **UI layouts**.
