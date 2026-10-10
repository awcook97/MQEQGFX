---@class DistScaleCfg  distance-based size falloff
---@field enabled boolean
---@field near number # world units still drawn at full size
---@field far number # world units where minScale is reached
---@field minScale number # size multiplier at (and beyond) far

local M = {}

---@param cfg DistScaleCfg|nil # nil or disabled = no scaling
---@param dist number|nil # 3D distance in world units (nil = full size)
---@return number scale # cfg.minScale .. 1
function M.factor(cfg, dist)
  if not (cfg and cfg.enabled) or not dist then return 1 end
  local near = cfg.near
  local far  = math.max(cfg.far, near + 1)
  if dist <= near then return 1 end
  if dist >= far then return cfg.minScale end
  return 1 - (1 - cfg.minScale) * ((dist - near) / (far - near))
end

return M
