local eqgfx = require('eqgfx')

local M = {}

local baseNames = require('eqgfx.lib.ui_windows')
local extraNames = {}
local pushed = false

local function push_names()
  local all, seen = {}, {}
  for _, list in ipairs({ baseNames, extraNames }) do
    for _, n in ipairs(list) do
      if not seen[n] then
        seen[n] = true
        all[#all + 1] = n
      end
    end
  end
  eqgfx.set_ui_names(all)
  pushed = true
end

---@param list string[]|nil # window names as shown by /windows
function M.add_names(list)
  for _, n in ipairs(list or {}) do extraNames[#extraNames + 1] = n end
  pushed = false
end

---@return { [1]: number, [2]: number, [3]: number, [4]: number, name: string }[] rects # {x0,y0,x1,y1,name} per visible window, clamped to the screen
function M.get()
  if not pushed then push_names() end
  return eqgfx.get_ui_rects()
end

---@param x0 number
---@param y0 number
---@param x1 number
---@param y1 number
---@param cap integer|nil # most pieces to return (default 24); past that the smallest are dropped
---@return number[][]|nil pieces # visible {x0,y0,x1,y1} pieces; nil when no window overlaps
function M.visible(x0, y0, x1, y1, cap)
  if not pushed then push_names() end
  return eqgfx.ui_subtract(x0, y0, x1, y1, cap)
end

---@param sw number # screen width in pixels
---@param sh number # screen height in pixels
---@param cap integer|nil # most regions to return (default 24); past that the smallest are dropped
---@return number[][] regions # visible {x0,y0,x1,y1} screen pieces
function M.regions(sw, sh, cap)
  return M.visible(0, 0, sw, sh, cap) or { { 0, 0, sw, sh } }
end

return M
