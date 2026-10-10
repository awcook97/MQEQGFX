---@alias HighlightRole
---| '"assistTarget"' # the raid/group main assist is on it
---| '"pc"'           # player character
---| '"pet"'          # somebody's pet
---| '"hostile"'      # NPC that is currently aggressive
---| '"neutral"'      # NPC that is not

---@class MarkerStyle
---@field color RGBA
---@field radius number    # base radius, world units
---@field height number    # vertical extent, world units
---@field speed number     # animation rate multiplier (1.0 = as authored)
---@field segments integer # line segments per ring / helix turn - cost knob
---@field comet boolean    # chase a bright head around each loop
---@field groundRing boolean
---@field rings integer|nil # tank only: how many orbiting great circles (1-3)
---@field turns integer|nil # assist only: helix revolutions

---@class HighlightTintCfg
---@field enabled boolean
---@field mode integer      # 1 tint, 2 ambient, 3 both
---@field myTarget boolean
---@field assistTarget boolean

---@class HighlightMarkerCfg
---@field enabled boolean
---@field mainTank boolean
---@field mainAssist boolean
---@field thickness integer # line half-width in px
---@field testMode boolean  # draw both markers on me, for tuning without a group

---@class HighlightAmbientCfg
---@field enabled boolean
---@field mode '"class"'|'"random"'   # per-class animation, or a randomized style
---@field radius integer      # scan radius, world units
---@field maxSpawns integer
---@field losOnly boolean
---@field includePCs boolean
---@field radius3d number     # effect radius (kept apart from the scan radius)
---@field height number
---@field speed number
---@field segsPerNpc integer  # upper bound; the budget may hand out fewer
---@field moteCount integer
---@field speedJitter number
---@field sizeJitter number
---@field fadeFar boolean
---@field classColors boolean
---@field styles table<string, boolean>
---@field palette RGBA[]
---@field classColorTable table<string, RGBA>

---@class HighlightCastCfg
---@field enabled boolean
---@field radius integer      # detection range, world units
---@field selfCasts boolean
---@field charge boolean      # animate the bar filling, not just the landing
---@field byResist boolean
---@field releaseLife number  # seconds a burst lasts
---@field fizzleLife number   # seconds an interrupted cast takes to unwind
---@field maxActive integer
---@field segsPerCast integer
---@field radius3d number     # effect radius when the spell has no AE range
---@field height number
---@field maxReach integer
---@field colors table<string, RGBA>

---@class HighlightConfig
---@field enabled boolean
---@field tints HighlightTintCfg
---@field colors table<HighlightRole, RGBA>
---@field markers HighlightMarkerCfg
---@field tank MarkerStyle
---@field assist MarkerStyle
---@field ambient HighlightAmbientCfg
---@field castfx HighlightCastCfg
---@field budget integer      # total world segments per frame, all layers
---@field pause PauseCfg      # stop drawing while backgrounded / not rendering

local M = {}

---@type table<string, boolean>
M.PlayerTypes = { PC = true, Mercenary = true }

---@type { key: HighlightRole, label: string, hint: string }[]
M.RoleRows = {
	{ key = 'assistTarget', label = 'Main assist target', hint = 'what the MA is on - outranks the rest' },
	{ key = 'pc',           label = 'Player',             hint = 'PC or mercenary' },
	{ key = 'pet',          label = 'Pet',                hint = "anyone's pet" },
	{ key = 'hostile',      label = 'NPC (fighting)',     hint = 'combat/aggro bits are set' },
	{ key = 'neutral',      label = 'NPC (idle)',         hint = 'everything else' },
}

---@type { value: integer, label: string, hint: string }[]
M.Modes = {
	{ value = 1, label = 'Tint only',   hint = 'multiply the color into every material' },
	{ value = 2, label = 'Glow only',   hint = 'constant ambient; keeps the original colors' },
	{ value = 3, label = 'Tint + glow', hint = 'recolored and lit' },
}

return M
