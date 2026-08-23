--[[
  highlight/settings.lua - defaults for target tinting + role markers,
  persisted via the shared scoped store (eqgfx.lib.settings) exactly like
  nameplates and dmgnums:
      <config>/eqgfx/<Server>_<Name>_settings.lua   (default: per character)
      <config>/eqgfx/<Server>_settings.lua
      <config>/eqgfx/global_settings.lua
  Scope is switchable from /hlmenu.

  Plain tables only. Colors are {r,g,b,a} floats 0..1 so the menu's
  ImGui.ColorEdit4 can bind to them directly; init.lua packs them into the
  ARGB integers the plugin wants at draw time.
]]

local Store = require('eqgfx.lib.settings')
local classanim = require('eqgfx.highlight.classanim')

---@class HighlightConfig
local DEFAULTS = {
	enabled = true,

	-- ---- model tinting ------------------------------------------------
	tints = {
		enabled     = true,
		mode        = 3,      -- 1 tint, 2 ambient, 3 both (see eqgfx.HIGHLIGHT_*)
		myTarget    = true,   -- recolor whatever I have targeted
		assistTarget= true,   -- always recolor what the main assist is on
	},

	---@type table<string, RGBA>
	colors = {
		assistTarget = { 0.75, 0.35, 1.00, 1.00 },  -- purple - the MA is on it
		pc           = { 0.24, 0.55, 1.00, 1.00 },  -- blue
		pet          = { 1.00, 0.78, 0.24, 1.00 },  -- gold
		hostile      = { 1.00, 0.24, 0.24, 1.00 },  -- red
		neutral      = { 0.94, 0.94, 0.94, 1.00 },  -- white
	},

	-- ---- animated role markers ----------------------------------------
	markers = {
		enabled    = true,
		mainTank   = true,
		mainAssist = true,
		thickness  = 2,       -- line half-width in px, applied while we draw
		testMode   = false,   -- draw both markers on ME, for tuning without a group
	},

	---@type MarkerStyle
	tank = {
		color    = { 0.35, 0.78, 1.00, 0.90 },  -- steel cyan
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
		color    = { 1.00, 0.65, 0.16, 0.90 },  -- amber
		radius   = 3.6,
		height   = 9.0,
		speed    = 1.0,
		segments = 26,
		turns    = 2,
		comet    = true,
		groundRing = true,
	},

	-- ---- ambient NPC animation ----------------------------------------
	-- Every NPC in sight gets a randomized-but-stable idle effect. The look
	-- of any one spawn is derived from its spawn ID, so the crowd varies but
	-- each mob animates smoothly (see ambient.lua).
	ambient = {
		enabled    = false,   -- off by default: it is the expensive layer
		radius     = 150,     -- world units to scan
		maxSpawns  = 20,      -- hard cap on how many get an effect
		losOnly    = true,    -- only NPCs you can actually see
		includePCs = false,   -- players too, not just NPCs
		nearestFirst = true,  -- when over the cap, keep the closest

		radius3d   = 3.0,     -- effect radius, world units  (named to avoid clashing with the scan radius)
		height     = 6.0,     -- effect vertical extent
		speed      = 1.0,     -- global rate multiplier
		segsPerNpc = 22,      -- segment allowance per spawn, before budget scaling
		moteCount  = 4,       -- 'motes' style: dashes per spawn
		speedJitter= 0.35,    -- +/- fraction of speed randomized per spawn
		sizeJitter = 0.30,    -- +/- fraction of radius randomized per spawn
		fadeFar    = true,    -- dim with distance so a crowd doesn't scream

		-- 'class'  each spawn animates as its class (classanim.lua), falling
		--          back to a random style when the class can't be read
		-- 'random' every spawn gets a randomized style from the set below
		mode       = 'class',

		classColors = true,   -- class mode: use the per-class color, not the palette

		---@type table<string, boolean>
		styles = {
			orbit = true, halo = true, motes = true, spiral = true, bloom = true,
		},

		---@type RGBA[]  used by 'random' mode, and by classless spawns
		palette = {
			{ 0.45, 0.85, 1.00, 0.55 },   -- ice blue
			{ 0.75, 0.55, 1.00, 0.55 },   -- violet
			{ 0.45, 1.00, 0.70, 0.55 },   -- mint
			{ 1.00, 0.80, 0.45, 0.55 },   -- warm gold
			{ 1.00, 0.55, 0.75, 0.55 },   -- rose
		},

		-- Per-class colors, keyed on Spawn.Class.ShortName(). Defaults are
		-- World of Warcraft's class colors mapped onto the closest EQ class;
		-- see classanim.lua for the mapping and the three EQ classes WoW has
		-- no counterpart for.
		---@type table<string, RGBA>
		classColorTable = classanim.COLORS,
	},

	-- ---- cast animations ----------------------------------------------
	-- One animation per spell shape, colored by what the spell does.
	castfx = {
		enabled     = true,
		radius      = 150,    -- only animate casts within this range
		selfCasts   = true,   -- animate your own casts too
		charge      = true,   -- animate the cast bar filling, not just the landing
		byResist    = true,   -- detrimental spells colored by resist type
		releaseLife = 0.9,    -- seconds a release burst lasts
		fizzleLife  = 0.5,    -- seconds an interrupted cast takes to unwind
		maxActive   = 12,     -- concurrent cast effects
		segsPerCast = 40,     -- segment allowance per effect
		radius3d    = 3.0,    -- base effect radius when the spell has no AE range
		height      = 6.0,
		maxReach    = 80,     -- clamp on how far an AE burst is allowed to travel

		---@type table<string, RGBA>
		colors = {
			beneficial  = { 0.45, 1.00, 0.60, 0.85 },   -- green
			detrimental = { 1.00, 0.35, 0.30, 0.85 },   -- red
			fire        = { 1.00, 0.50, 0.15, 0.85 },
			cold        = { 0.45, 0.85, 1.00, 0.85 },
			magic       = { 0.65, 0.45, 1.00, 0.85 },
			poison      = { 0.55, 1.00, 0.30, 0.85 },
			disease     = { 0.70, 0.85, 0.35, 0.85 },
			corruption  = { 0.55, 0.25, 0.65, 0.85 },
			physical    = { 0.85, 0.85, 0.85, 0.85 },
			arcane      = { 1.00, 1.00, 1.00, 0.85 },   -- chromatic/prismatic/unresistable
		},
	},

	-- Total world segments the whole feature may emit in one frame. The
	-- plugin draws 2*thickness+1 parallel copies of each, so the engine cost
	-- is this times that; markers and cast effects are served before ambient,
	-- which absorbs whatever is left.
	budget = 1200,

	pause = {                   -- stop drawing when nobody can see it
		whenBackground   = false, -- backgrounded but rendering: boxers watch those windows
		whenNotRendering = true,  -- the frame limiter has stopped drawing the scene
	},
}

--- The highlight settings store: a SettingsStore whose .data is typed as
--- the HighlightConfig tree.
---@class HighlightStore: SettingsStore
---@field data HighlightConfig

return Store.new({ section = 'highlight', defaults = DEFAULTS }) --[[@as HighlightStore]]
