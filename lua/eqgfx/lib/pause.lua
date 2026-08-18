--- lib/pause.lua - "should this feature draw at all right now?", shared by the
--- eqgfx feature scripts.
---
--- Two independent reasons to stop: EverQuest is not the foreground window, and
--- the frame limiter has stopped drawing the scene (its visible frame rate
--- falls to ~0 while the simulation keeps running). Neither TLO is guaranteed
--- to exist on every build, so an unreadable one counts as "keep drawing".
---
--- ```lua
--- local pause = require('eqgfx.lib.pause')
--- if pause.active(cfg.pause) then return end   -- skip the draw pass
--- ```

local mq = require('mq')

---@class PauseCfg  when a feature stops drawing (one per feature settings tree)
---@field whenBackground boolean # EverQuest is not the foreground window
---@field whenNotRendering boolean # the frame limiter has stopped drawing frames

local M = {}

local RENDER_FLOOR = 1.0   -- visible fps at or below which nothing is being drawn

--- pcall a TLO getter, keep the result only when it is a real boolean (nil =
--- the call failed or the member isn't there, which both mean "don't know").
---@param f fun(): any
---@return boolean|nil
local function getbool(f)
  local ok, v = pcall(f)
  if ok and v ~= nil then return v and true or false end
  return nil
end

--- pcall a TLO getter, keep the result only when it's a number.
---@param f fun(): any
---@return number|nil
local function getn(f)
  local ok, v = pcall(f)
  if ok and type(v) == 'number' then return v end
end

--- Is the EverQuest window in the foreground? Unknown counts as yes.
---@return boolean foreground
function M.foreground()
  return getbool(function() return mq.TLO.EverQuest.Foreground() end) ~= false
end

--- Is the client actually drawing frames? The frame limiter can stop rendering
--- the scene while the simulation keeps running, and reports a visible frame
--- rate of ~0 while it does. No frame limiter counts as yes.
---@return boolean rendering
function M.rendering()
  local limiter = mq.TLO.FrameLimiter
  if getbool(function() return limiter.Enabled() end) ~= true then return true end
  local fps = getn(function() return limiter.RenderFPS() end)
  return fps == nil or fps > RENDER_FLOOR
end

--- Should drawing be skipped this frame?
---
--- ```lua
--- local paused, why = pause.active(cfg.pause)
--- if paused then log.Info('idle: %s', why) return end
--- ```
---@param cfg PauseCfg|nil # nil = never pause
---@return boolean paused
---@return string|nil reason # 'background' | 'not rendering'
function M.active(cfg)
  if not cfg then return false end
  if cfg.whenBackground and not M.foreground() then return true, 'background' end
  if cfg.whenNotRendering and not M.rendering() then return true, 'not rendering' end
  return false
end

return M
