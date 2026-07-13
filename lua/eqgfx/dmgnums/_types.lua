--[[
  dmgnums/_types.lua - types, classes and enums for the damage numbers feature.
]]

local T = {}

---@enum DmgAnimStyle  how a number travels after it spawns
T.AnimStyle = { FLOAT = 1, ARC = 2, FOUNTAIN = 3, STATIC = 4 }
T.AnimStyleLabels = { 'Float up', 'Arc (toss to the side)', 'Fountain (burst)', 'Static (fade in place)' }

-- Defense codes the client reports as DamageCaused <= 0 on the hit path.
-- Labels are user-editable in settings (cfg.missLabels overrides these).
T.MissCodes = {
	[0]  = 'MISS',
	[-1] = 'INVUL',
	[-2] = 'RIPOSTE',
	[-3] = 'PARRY',
	[-4] = 'DODGE',
	[-5] = 'BLOCK',
	[-6] = 'ABSORB',
}

-- Source buckets an event lands in (relative to me).
--   mine     - I dealt it
--   pet      - my pet/merc dealt it
--   others   - anyone else dealt it (to something that isn't me)
--   incoming - dealt TO me (or my pet, when petIncoming is on)
T.Source = { MINE = 'mine', PET = 'pet', OTHERS = 'others', INCOMING = 'incoming' }

-- Kind buckets (the in-game filter categories).
T.Kind = { MELEE = 'melee', SPELLS = 'spells', DOTS = 'dots', HEALS = 'heals', MISSES = 'misses' }

---@class DmgFloater  one live on-screen number
---@field spawnID integer   anchor spawn (screen pos re-read live while it exists)
---@field text string       main text ("1234", "12.3k", "+512", "DODGE")
---@field sub string|nil    optional second line (spell name)
---@field color number[]    {r,g,b,a}
---@field size number       font px at scale 1
---@field bornAt number     frame-time the floater spawned
---@field dx number         horizontal spread offset (px)
---@field vx number         horizontal velocity (px/s, ARC/FOUNTAIN)
---@field vy number         initial vertical velocity (px/s, ARC/FOUNTAIN)
---@field sx number|nil     last projected screen x (fallback when spawn dies)
---@field sy number|nil     last projected screen y

return T
