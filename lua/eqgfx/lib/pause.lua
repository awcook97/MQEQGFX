local mq = require('mq')

---@class PauseCfg  when a feature stops drawing (one per feature settings tree)
---@field whenBackground boolean # EverQuest is not the foreground window
---@field whenNotRendering boolean # the frame limiter has stopped drawing frames

local M = {}

local RENDER_FLOOR = 1.0

---@param f fun(): any
---@return boolean|nil
local function getbool(f)
  local ok, v = pcall(f)
  if ok and v ~= nil then return v and true or false end
  return nil
end

---@param f fun(): any
---@return number|nil
local function getn(f)
  local ok, v = pcall(f)
  if ok and type(v) == 'number' then return v end
end

---@return boolean foreground
function M.foreground()
  return getbool(function() return mq.TLO.EverQuest.Foreground() end) ~= false
end

---@return boolean rendering
function M.rendering()
  local limiter = mq.TLO.FrameLimiter
  if getbool(function() return limiter.Enabled() end) ~= true then return true end
  local fps = getn(function() return limiter.RenderFPS() end)
  return fps == nil or fps > RENDER_FLOOR
end

---@param cfg PauseCfg|nil # nil = never pause
---@return boolean paused
---@return string|nil reason # 'background' | 'not rendering'
function M.active(cfg)
  if not cfg then return false end
  if cfg.whenBackground and not M.foreground() then return true, 'background' end
  if cfg.whenNotRendering and not M.rendering() then return true, 'not rendering' end
  return false
end

function M.wait_for_game()
  while mq.TLO.MacroQuest.GameState() ~= 'INGAME' or not mq.TLO.Me.CleanName() do
    mq.delay(1000)
  end
end

return M
