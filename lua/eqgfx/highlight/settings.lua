local Store = require('eqgfx.lib.settings')
local classanim = require('eqgfx.highlight.classanim')

---@class HighlightConfig
local DEFAULTS = {
	enabled = true,

	tints = {
		enabled     = true,
		mode        = 3,
		myTarget    = true,
		assistTarget= true,
	},

	---@type table<string, RGBA>
	colors = {
		assistTarget = { 0.75, 0.35, 1.00, 1.00 },
		pc           = { 0.24, 0.55, 1.00, 1.00 },
		pet          = { 1.00, 0.78, 0.24, 1.00 },
		hostile      = { 1.00, 0.24, 0.24, 1.00 },
		neutral      = { 0.94, 0.94, 0.94, 1.00 },
	},

	markers = {
		enabled    = true,
		mainTank   = true,
		mainAssist = true,
		thickness  = 2,
		testMode   = false,
	},

	---@type MarkerStyle
	tank = {
		color    = { 0.35, 0.78, 1.00, 0.90 },
		radius   = 4.5,
		height   = 7.0,
		speed    = 1.0,
		segments = 28,
		rings    = 3,
		comet    = true,
		groundRing = true,
	},

	---@type MarkerStyle
	assist = {
		color    = { 1.00, 0.65, 0.16, 0.90 },
		radius   = 3.6,
		height   = 9.0,
		speed    = 1.0,
		segments = 26,
		turns    = 2,
		comet    = true,
		groundRing = true,
	},

	ambient = {
		enabled    = false,
		radius     = 150,
		maxSpawns  = 20,
		losOnly    = true,
		includePCs = false,

		radius3d   = 3.0,
		height     = 6.0,
		speed      = 1.0,
		segsPerNpc = 22,
		moteCount  = 4,
		speedJitter= 0.35,
		sizeJitter = 0.30,
		fadeFar    = true,

		mode       = 'class',

		classColors = true,

		---@type table<string, boolean>
		styles = {
			orbit = true, halo = true, motes = true, spiral = true, bloom = true,
		},

		---@type RGBA[]  used by 'random' mode, and by classless spawns
		palette = {
			{ 0.45, 0.85, 1.00, 0.55 },
			{ 0.75, 0.55, 1.00, 0.55 },
			{ 0.45, 1.00, 0.70, 0.55 },
			{ 1.00, 0.80, 0.45, 0.55 },
			{ 1.00, 0.55, 0.75, 0.55 },
		},

		---@type table<string, RGBA>
		classColorTable = classanim.COLORS,
	},

	castfx = {
		enabled     = true,
		alongsideNative = false,
		radius      = 150,
		selfCasts   = true,
		charge      = true,
		byResist    = true,
		releaseLife = 0.9,
		fizzleLife  = 0.5,
		maxActive   = 12,
		segsPerCast = 40,
		radius3d    = 3.0,
		height      = 6.0,
		maxReach    = 80,

		---@type table<string, RGBA>
		colors = {
			beneficial  = { 0.45, 1.00, 0.60, 0.85 },
			detrimental = { 1.00, 0.35, 0.30, 0.85 },
			fire        = { 1.00, 0.50, 0.15, 0.85 },
			cold        = { 0.45, 0.85, 1.00, 0.85 },
			magic       = { 0.65, 0.45, 1.00, 0.85 },
			poison      = { 0.55, 1.00, 0.30, 0.85 },
			disease     = { 0.70, 0.85, 0.35, 0.85 },
			corruption  = { 0.55, 0.25, 0.65, 0.85 },
			physical    = { 0.85, 0.85, 0.85, 0.85 },
			arcane      = { 1.00, 1.00, 1.00, 0.85 },
		},
	},

	budget = 1200,

	pause = {
		whenBackground   = false,
		whenNotRendering = true,
	},
}

---@class HighlightStore: SettingsStore
---@field data HighlightConfig

return Store.new({ section = 'highlight', defaults = DEFAULTS }) --[[@as HighlightStore]]
