--[[
  dmgnums/settings.lua - damage-number defaults, persisted via the shared
  scoped store (eqgfx.lib.settings) exactly like nameplates:
      <config>/eqgfx/<Server>_<Name>_settings.lua   (default: per character)
      <config>/eqgfx/<Server>_settings.lua
      <config>/eqgfx/global_settings.lua
  Scope is switchable from /dnmenu.

  Plain tables only. Colors are {r,g,b,a} floats 0..1.
]]

local Store = require('eqgfx.lib.settings')
local types = require('eqgfx.dmgnums._types')

---@class DmgnumsConfig
local DEFAULTS = {
	enabled     = true,
	radius      = 200,    -- world units; events on spawns farther than this are ignored
	targetOnly  = false,  -- only float numbers over my current target (outgoing)
	hideUnderUI = false,  -- suppress numbers whose anchor sits behind an open EQ window
	zOffset     = 5.0,    -- world units above the spawn's head

	pause = {                    -- stop drawing when nobody can see the numbers
		whenBackground   = false,  -- backgrounded but still rendering: boxers watch those windows
		whenNotRendering = true,   -- the frame limiter has stopped drawing the scene
	},

	distScale = {          -- numbers shrink with distance to the spawn they float over
		enabled  = false,
		near     = 30,       -- world units still drawn at full size
		far      = 200,      -- world units where minScale is reached
		minScale = 0.55,
	},

	-- WHO x WHAT filters — the in-game filter grid. Sources: mine (me), pet
	-- (my pet/merc), others (everyone else's combat), incoming (dealt TO me).
	show = {
		mine     = { melee = true,  spells = true,  dots = true,  heals = true,  misses = true  },
		pet      = { melee = true,  spells = true,  dots = true,  heals = false, misses = false },
		others   = { melee = false, spells = false, dots = false, heals = false, misses = false },
		incoming = { melee = true,  spells = true,  dots = true,  heals = true,  misses = true  },
	},
	petIncoming      = false, -- damage TO my pet counts as incoming
	incomingOverSelf = true,  -- float incoming numbers over MY head (else over the attacker)
	minDamage        = 0,     -- suppress hits below this (misses/heals unaffected)

	text = {
		size            = 16.0,  -- font px
		shadow          = true,  -- 1px drop shadow
		outline         = false, -- 4-way outline (heavier than shadow)
		abbreviate      = true,  -- 12345 -> "12.3k"
		healSign        = true,  -- prefix heals with "+"
		scaleWithDamage = true,  -- bigger hits draw bigger
		bigAt           = 1000,  -- damage at which the max scale is reached
		bigScale        = 1.7,   -- max size multiplier
		showSpellName   = false, -- spell/DoT numbers get the spell name under them
		spellNameSize   = 10.0,
	},

	anim = {
		style      = types.AnimStyle.FLOAT,
		duration   = 1.1,    -- seconds a number lives
		riseSpeed  = 46.0,   -- px/s (FLOAT; initial vertical speed for ARC/FOUNTAIN)
		spreadX    = 16.0,   -- random horizontal spawn offset (px, +/-)
		gravity    = 120.0,  -- px/s^2 pulling ARC/FOUNTAIN numbers back down
		fadeStart  = 0.55,   -- fraction of duration where fade-out begins
		popIn      = true,   -- overshoot-scale on spawn
		popDur     = 0.12,
		stackNudge = true,   -- consecutive numbers on one spawn nudge up so they don't overlap
	},

	colors = {
		myMelee      = { 1.00, 1.00, 1.00, 1.00 },
		mySpell      = { 1.00, 0.62, 0.15, 1.00 },
		myDot        = { 0.72, 1.00, 0.30, 1.00 },
		petMelee     = { 0.55, 0.85, 1.00, 1.00 },
		petSpell     = { 0.55, 0.70, 1.00, 1.00 },
		petDot       = { 0.55, 1.00, 0.85, 1.00 },
		otherMelee   = { 0.78, 0.78, 0.78, 0.90 },
		otherSpell   = { 0.85, 0.65, 0.45, 0.90 },
		otherDot     = { 0.70, 0.85, 0.50, 0.90 },
		heal         = { 0.30, 1.00, 0.40, 1.00 },
		incoming     = { 1.00, 0.25, 0.25, 1.00 },
		incomingHeal = { 0.40, 1.00, 0.60, 1.00 },
		miss         = { 0.75, 0.75, 0.75, 1.00 },
		spellName    = { 1.00, 1.00, 1.00, 0.80 },
		shadow       = { 0.00, 0.00, 0.00, 0.90 },
	},

	-- Defense-code labels (DamageCaused <= 0). Editable so you can shorten or
	-- restyle them; missing codes fall back to "MISS".
	missLabels = {
		[0]  = 'MISS',
		[-1] = 'INVUL',
		[-2] = 'RIPOSTE',
		[-3] = 'PARRY',
		[-4] = 'DODGE',
		[-5] = 'BLOCK',
		[-6] = 'ABSORB',
	},
}

--- The dmgnums settings store: a SettingsStore whose .data is typed as
--- the DmgnumsConfig tree.
---@class DmgnumsStore: SettingsStore
---@field data DmgnumsConfig

return Store.new({ section = 'dmgnums', defaults = DEFAULTS }) --[[@as DmgnumsStore]]
