--[[
  eqgfx/tools/portrait.lua - minimal ImGui overlay showing the current target's
  live 3D portrait, rendered by the MQEQGFX plugin's ObjectPreview pipeline.

  Run:  /lua run eqgfx/tools/portrait        (requires /plugin MQEQGFX)

  The window is drawn with NoBackground, so the engine composites the 3D model
  into the window's content area. Drag / resize it anywhere; close it to stop.
]]
local mq    = require('mq')
local eqgfx = require('eqgfx')

local SLOT   = 1        -- ObjectPreview slot 1 = target (slot 0 = self)
local open   = true
local sized  = false

-- MQ's ImGui binding returns some geometry as an ImVec2 (with .x/.y) and some as
-- two plain numbers; accept either.
local function xy(a, b)
  if type(a) == 'number' then return a, b end
  return a.x, a.y
end

local function draw()
  if not sized then ImGui.SetNextWindowSize(180, 220); sized = true end

  local show
  open, show = ImGui.Begin('Target Portrait##eqgfx', open, ImGuiWindowFlags.NoBackground)
  if show then
    -- Content rect in SCREEN pixels -> hand it to the plugin, which blits the
    -- engine-rendered portrait RT into exactly this rectangle.
    local px, py = xy(ImGui.GetCursorScreenPos())
    local aw, ah = xy(ImGui.GetContentRegionAvail())
    if px and aw and aw > 4 and ah > 4 then
      eqgfx.set_portrait(SLOT, {
        subject = 'target',
        x0 = px,       y0 = py,
        x1 = px + aw,  y1 = py + ah,
      })
    else
      eqgfx.clear_portrait(SLOT)
    end
  else
    eqgfx.clear_portrait(SLOT)
  end
  ImGui.End()
end

mq.imgui.init('eqgfx_portrait', draw)
print('\aw[eqgfx]\ax target-portrait overlay running (close the window or /lua stop to end)')
while open do mq.delay(100) end
eqgfx.clear_portrait(SLOT)
