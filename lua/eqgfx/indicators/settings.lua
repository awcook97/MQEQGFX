local mq    = require('mq')
local Store = require('eqgfx.lib.settings')

---@type IndicatorsConfig
local DEFAULTS = {
  showEnemies   = true,
  showFriendly  = false,
  showSelf      = true,
  showAoE       = true,
  showCones     = true,
  showBeams     = true,
  showTargetAoE = true,
  showLines     = true,
  showDebugRing = false,
  genericMarker = true,
  radius        = 250,
  groundOffset  = 5.0,
  debugRadius   = 40,
  pause = {
    whenBackground   = false,
    whenNotRendering = true,
  },
  colors = {
    enemyFill  = { 1.00, 0.51, 0.51, 0.27 },
    enemyLine  = { 1.00, 0.24, 0.24, 0.86 },
    friendFill = { 0.51, 0.71, 1.00, 0.27 },
    friendLine = { 0.30, 0.60, 1.00, 0.86 },
    selfFill   = { 0.51, 1.00, 0.51, 0.27 },
    selfLine   = { 0.24, 1.00, 0.24, 0.86 },
    line       = { 1.00, 0.85, 0.20, 0.90 },
    debugFill  = { 0.00, 1.00, 1.00, 0.20 },
    debugLine  = { 0.00, 1.00, 1.00, 0.86 },
  },
}

---@class IndicatorsStore: SettingsStore
---@field data IndicatorsConfig

return Store.new({
  section  = 'indicators',
  defaults = DEFAULTS,
  legacy   = mq.configDir .. '/eqgfx_indicator.lua',
}) --[[@as IndicatorsStore]]
