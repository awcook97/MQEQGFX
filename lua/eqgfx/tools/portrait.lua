local mq    = require('mq')
local eqgfx = require('eqgfx')

print('\ag[eqgfx]\ax portrait: \ao(in development)\ax')

local SLOT   = 1
local open   = true
local sized  = false

local function xy(a, b)
  if type(a) == 'number' then return a, b end
  return a.x, a.y
end

local function draw()
  if not sized then ImGui.SetNextWindowSize(180, 220); sized = true end

  local show
  open, show = ImGui.Begin('Target Portrait##eqgfx', open, ImGuiWindowFlags.NoBackground)
  if show then
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
