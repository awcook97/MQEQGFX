local Store = require('eqgfx.lib.settings')
local types = require('eqgfx.dmgnums._types')

---@class DmgnumsConfig
local DEFAULTS = {
	enabled     = true,
	radius      = 200,
	targetOnly  = false,
	hideUnderUI = false,
	zOffset     = 5.0,

	pause = {
		whenBackground   = false,
		whenNotRendering = true,
	},

	distScale = {
		enabled  = false,
		near     = 30,
		far      = 200,
		minScale = 0.55,
	},

	show = {
		mine     = { melee = true,  spells = true,  dots = true,  heals = true,  misses = true  },
		pet      = { melee = true,  spells = true,  dots = true,  heals = false, misses = false },
		others   = { melee = false, spells = false, dots = false, heals = false, misses = false },
		incoming = { melee = true,  spells = true,  dots = true,  heals = true,  misses = true  },
	},
	petIncoming      = false,
	incomingOverSelf = true,
	minDamage        = 0,

	text = {
		size            = 16.0,
		shadow          = true,
		outline         = false,
		abbreviate      = true,
		healSign        = true,
		scaleWithDamage = true,
		bigAt           = 1000,
		bigScale        = 1.7,
		showSpellName   = false,
		spellNameSize   = 10.0,
	},

	anim = {
		style      = types.AnimStyle.FLOAT,
		duration   = 1.1,
		riseSpeed  = 46.0,
		spreadX    = 16.0,
		gravity    = 120.0,
		fadeStart  = 0.55,
		popIn      = true,
		popDur     = 0.12,
		stackNudge = true,
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

---@class DmgnumsStore: SettingsStore
---@field data DmgnumsConfig

return Store.new({ section = 'dmgnums', defaults = DEFAULTS }) --[[@as DmgnumsStore]]
