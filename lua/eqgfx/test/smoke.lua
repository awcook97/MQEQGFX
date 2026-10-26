local feature, root, tmp = arg[1], arg[2], arg[3]
local mode = arg[4]
local OCC = mode and mode:match('^occlude') and true or false

local t0 = 0
rawset(os, 'clock', function() t0 = t0 + 0.05; return t0 end)
package.path = root .. '/../?.lua;' .. root .. '/../?/init.lua;' .. package.path

local function proxy()
  return setmetatable({}, {
    __index = function(self, k)
      local p = proxy()
      rawset(self, k, p)
      return p
    end,
    __call = function(_, a)
      if a ~= nil then return proxy() end
      return nil
    end,
  })
end

local draws, binds, events = {}, {}, {}
local loops = 0
local pumps, frameNo = 0, 0
local mqstub = {
  configDir = tmp,
  gettime = function() return os.clock() * 1000 end,
  TLO = proxy(),
  imgui = { init = function(_, fn) draws[#draws + 1] = fn end },
  event = function(name)
    assert(not events[name], 'duplicate event name: ' .. name)
    events[name] = true
  end,
  bind = function(cmd, fn) binds[cmd] = fn end,
  doevents = function()
    pumps = pumps + 1
    if pumps >= 50 then error('__SMOKE_DONE__', 0) end
  end,
  delay = function()
    loops = loops + 1
    if loops >= 3 then error('__SMOKE_DONE__', 0) end
  end,
  cmdf = function() end,
  cmd = function() end,
  ExtractLinks = function() return {} end,
  ParseSpellLink = function() return nil end,
  FormatSpellLink = function() return '' end,
  ExecuteTextLink = function() end,
  StripTextLinks = function(s) return s end,
  LinkTypes = { Spell = 1 },
  FindTextureAnimation = function() return nil end,
  pickle = function() end,
  unpickle = function() error('no file', 0) end,
}
package.preload['mq'] = function() return mqstub end
mqstub.TLO.MacroQuest.GameState = function() return 'INGAME' end
mqstub.TLO.Me.CleanName = function() return 'Smoke' end

local fills = 0
local dl = setmetatable({}, { __index = function(_, k)
  if k == 'AddRectFilled' then return function() fills = fills + 1 end end
  return function() end
end })
local ImGuiStub = setmetatable({
  GetForegroundDrawList = function() return dl end,
  GetBackgroundDrawList = function() return dl end,
  GetWindowDrawList = function() return dl end,
  Begin = function() return true, true end,
  End = function() end,
  CalcTextSize = function() return 10, 13 end,
  GetFontSize = function() return 13 end,
  GetFont = function() return {} end,
  ColorConvertFloat4ToU32 = function() return 0x7f7f7f7f end,
  GetMousePos = function() return 50, 50 end,
  IsMouseDown = function() return false end,
  IsMouseClicked = function() return false end,
  IsWindowHovered = function() return false end,
  Text = function() end, TextDisabled = function() end,
  Separator = function() end, SameLine = function() end,
  SetNextWindowPos = function() end, SetNextWindowSize = function() end,
  SetCursorScreenPos = function() end,
  DrawTextureAnimation = function() end,
  TreePop = function() end,
  Button = function() return false end,
  SmallButton = function() return false end,
  RadioButton = function() return false end,
  TreeNode = function() return false end,
}, {
  __index = function()
    return function(_, v) return v, false end
  end,
})
package.preload['ImGui'] = function() return ImGuiStub end
rawset(_G, 'ImGui', ImGuiStub)
rawset(_G, 'ImVec2', function(x, y) return { x = x, y = y } end)
rawset(_G, 'ImGuiWindowFlags', setmetatable({}, { __index = function() return 1 end }))
rawset(_G, 'ImGuiMouseButton', setmetatable({}, { __index = function() return 1 end }))
rawset(_G, 'ImGuiHoveredFlags', setmetatable({}, { __index = function() return 4 end }))
do
  local mt = getmetatable(_G) or {}
  local prev = mt.__index
  mt.__index = function(t, k)
    if type(k) == 'string' and k:match('^ImGui%u') then
      local enum = setmetatable({}, { __index = function() return 1 end })
      rawset(_G, k, enum)
      return enum
    end
    if type(prev) == 'function' then return prev(t, k) end
    if type(prev) == 'table' then return prev[k] end
  end
  setmetatable(_G, mt)
end

local W2S = { x = 300, y = 300 }
if OCC then
  local function fn(v) return function() return v end end
  local spawn = setmetatable({
    ID = fn(42), X = fn(10), Y = fn(10), Z = fn(5),
    PctHPs = fn(100), CleanName = fn('Dummy'), Type = fn('NPC'),
    Animation = fn(0),
  }, {
    __index = function() return fn(nil) end,
    __call = function() return true end,
  })
  mqstub.TLO.Spawn = function() return spawn end

  local invWindow = setmetatable({
    Open = fn(true), Minimized = fn(false),
    X = fn(100), Y = fn(100), Width = fn(600), Height = fn(800),
    Parent = setmetatable({}, { __index = function() return fn(nil) end }),
  }, { __index = function() return fn(nil) end })
  local nullWindow = setmetatable({}, { __index = function() return fn(nil) end })
  mqstub.TLO.Window = function(name)
    if name == 'InventoryWindow' then return invWindow end
    return nullWindow
  end
end

local function stub_rects()
  if mode == 'occlude-native' then return { { 100, 100, 700, 900 } } end
  if mode == 'occlude-native-empty' then return {} end
  return { { 100, 100, 400, 300 }, { 500, 50, 800, 200 } }
end

package.preload['eqgfx'] = function()
  return {
    init = function() return true end,
    clear = function() end,
    get_screen = function() return 1920, 1080 end,
    ui_stats = function() return 0, 0 end,
    build = function() return 'smoke-stub' end,
    cast_feed_live = function() return false end,
    set_ui_names = function() end,
    get_ui_rects = function() return stub_rects() end,
    spawn_search = function() return OCC and { 42 } or {} end,
    ui_subtract = function(x0, y0, x1, y1)
      local overlap = nil
      for _, w in ipairs(stub_rects()) do
        if x0 >= w[1] and y0 >= w[2] and x1 <= w[3] and y1 <= w[4] then return {} end
        if w[1] < x1 and w[3] > x0 and w[2] < y1 and w[4] > y0 then overlap = { { x0, y0, x1, y1 } } end
      end
      return overlap
    end,
    world_to_screen = function() return W2S.x, W2S.y, true end,
    project = function() return 320, 240, true end,
    spell_geom = function() return nil end,
    set_thickness = function() end,
    add_line = function() end,
    stats = function() frameNo = frameNo + 1; return frameNo, 0 end,
    argb = function() return 0 end,
    poll_damage = function() return {} end,
    highlight = function() end, unhighlight = function() end, clear_highlights = function() end,
    blank = function() end,
    TargetType = require('eqgfx.core._types').TargetType,
  }
end

local okk, err = pcall(dofile, root .. '/' .. feature .. '/init.lua')
if not okk and not tostring(err):find('__SMOKE_DONE__') then
  io.stderr:write('STARTUP/LOOP FAIL: ' .. tostring(err) .. '\n')
  os.exit(1)
end
assert(#draws > 0, 'no imgui draw callback registered')

for frame = 1, 3 do
  for _, fn in ipairs(draws) do
    local ok2, e2 = pcall(fn)
    if not ok2 then
      io.stderr:write('DRAW FAIL (frame ' .. frame .. '): ' .. tostring(e2) .. '\n')
      os.exit(1)
    end
  end
end

local menu = package.loaded['eqgfx.' .. feature .. '.menu']
if menu then
  menu.open = true
  for _, fn in ipairs(draws) do
    local ok2, e2 = pcall(fn)
    if not ok2 then
      io.stderr:write('MENU DRAW FAIL: ' .. tostring(e2) .. '\n')
      os.exit(1)
    end
  end
end

for cmd, fn in pairs(binds) do
  local ok2, e2 = pcall(fn)
  if not ok2 then
    io.stderr:write('BIND FAIL ' .. cmd .. ': ' .. tostring(e2) .. '\n')
    os.exit(1)
  end
end

if OCC and feature == 'nameplates' then
  if mode == 'occlude-native-empty' then
    W2S.x, W2S.y = 300, 300
    fills = 0
    for _ = 1, 3 do for _, fn2 in ipairs(draws) do assert(pcall(fn2)) end end
    assert(fills > 0, mode .. ': plate should draw when occlusion is off')
  else
    W2S.x, W2S.y = 300, 300
    fills = 0
    for _ = 1, 3 do for _, fn2 in ipairs(draws) do assert(pcall(fn2)) end end
    assert(fills == 0, mode .. ': plate drew INSIDE window rect (fills=' .. fills .. ')')

    W2S.x, W2S.y = 1500, 300
    fills = 0
    for _ = 1, 3 do for _, fn2 in ipairs(draws) do assert(pcall(fn2)) end end
    assert(fills > 0, mode .. ': plate did NOT draw outside window rect')
  end
  print('OCCLUSION OK: ' .. mode)
end

print('SMOKE OK: ' .. feature .. ' (loops=' .. loops .. ', draws=' .. #draws .. ')')
