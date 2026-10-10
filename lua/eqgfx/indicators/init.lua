local mq    = require('mq')
local eqgfx = require('eqgfx')
local ImGui = require('ImGui')
local log   = require('eqgfx.lib.lwlogger')

local settings = require('eqgfx.indicators.settings')
local render   = require('eqgfx.indicators.render')
local menu     = require('eqgfx.indicators.menu')
local casts    = require('eqgfx.casts')
local uirects  = require('eqgfx.lib.uirects')
local pause    = require('eqgfx.lib.pause')

log.SetAppName('eqgfx')
log.SetModuleName('indicators')
log.SetColors(true)
log.SetIncludeTime('milliseconds')
log.SetLevel(log.INFO)
log.SetIncludeCharacter('Server.Character.Zone')
log.SetOutputFile(mq.configDir .. '/eqgfx.log')
log.SetIncludeSource('trace')

pause.wait_for_game()

local ok, err = eqgfx.init()
if not ok then log.Error('init failed: %s', err) return end

settings.load()
casts.init{ log = log, trackSelf = true, grace = 0.5, radius = settings.data.radius }

---@type table<integer, ActiveCast>
local active = {}

---@param spawn spawn # caster spawn TLO
---@return boolean friend
local function classify_friend(spawn)
  local t = spawn.Type()
  if t == 'PC' or t == 'Pet' or t == 'Mercenary' then return true end
  if t == 'NPC' then return not spawn.Aggressive() end
  return true
end

---@return boolean|nil ok
local function eqbc_ok()
  return mq.TLO.Plugin('mq2eqbc').IsLoaded() and mq.TLO.EQBC.Connected()
end

---@param casterID integer
---@param spellID integer|nil # 0 when unknown
---@param targetID integer|nil # the resolved victim
local function announce(casterID, spellID, targetID)
  if not eqbc_ok() then return end
  mq.cmdf('/squelch /bc eqgfx_cast=%d=%d=%d', casterID, spellID or 0, targetID or 0)
end

local function valid_id(v)
  if v and v >= 1 and v < 2 ^ 31 and v == math.floor(v) then return v end
end

mq.event('eqgfx_cast_in', "#*#eqgfx_cast=#1#=#2#=#3##*#", function(_, cid, sid, vid)
  cid, sid, vid = valid_id(tonumber(cid)), valid_id(tonumber(sid)), valid_id(tonumber(vid))
  if not cid then return end
  local activeCast = active[cid]
  if not activeCast then
    activeCast = { id = cid, spellID = sid, name = '?',
          isSelf = false, friend = false, stub = true, castStart = casts.now(),
          expireAt = casts.now() + casts.cast_seconds(sid) + 1 }
    activeCast.geom = sid and eqgfx.spell_geom(sid) or nil
    active[cid] = activeCast
  end
  if vid then activeCast.targetID = vid end
end)

local canClip

---@param drawList ImDrawList
local function draw_all(drawList)
  if (settings.data or {}).showDebugRing then render.draw_debug_ring(drawList) end
  for _, activeCast in pairs(active) do
    render.draw_active(drawList, activeCast)
  end
end

local function draw()
  local cfg = settings.data or {}
  if pause.active(cfg.pause) or (next(active) == nil and not cfg.showDebugRing) then
    menu.draw()
    return
  end
  local drawList = ImGui.GetBackgroundDrawList()
  if canClip == nil then
    canClip = pcall(function()
      drawList:PushClipRect(ImVec2(0, 0), ImVec2(1, 1), false)
      drawList:PopClipRect()
    end) and true or false
  end

  local sw, sh = eqgfx.get_screen()
  if canClip and sw > 0 then
    for _, rect in ipairs(uirects.regions(sw, sh)) do
      drawList:PushClipRect(ImVec2(rect[1], rect[2]), ImVec2(rect[3], rect[4]), false)
      draw_all(drawList)
      drawList:PopClipRect()
    end
  else
    draw_all(drawList)
  end

  menu.draw()
end

mq.imgui.init('eqgfx_indicators', draw)

local function clamped(text, lo, hi)
  local n = tonumber(text)
  if not n or n ~= n then return nil end
  return math.max(lo, math.min(hi, n))
end

mq.bind('/aemenu', function() menu.toggle() end)
mq.bind('/aering', function(rect)
  local cfg = settings.data or {}
  if rect and rect ~= '' then cfg.debugRadius = clamped(rect, 1, 400) or cfg.debugRadius end
  cfg.showDebugRing = not cfg.showDebugRing
  settings.mark_dirty()
  log.Info('debug ring = %s (radius %g)', tostring(cfg.showDebugRing), cfg.debugRadius)
end)
mq.bind('/aerad', function(rect)
  local cfg = settings.data or {}
  cfg.debugRadius = clamped(rect, 1, 400) or cfg.debugRadius
  settings.mark_dirty()
  log.Info('debug ring radius = %g', cfg.debugRadius)
end)
mq.bind('/aez', function(z)
  local cfg = settings.data or {}
  cfg.groundOffset = clamped(z, 0, 15) or cfg.groundOffset
  settings.mark_dirty()
  log.Info('ground offset = %g', cfg.groundOffset)
end)

log.Info('spell indicators running. /aemenu = settings, /aering = debug ring.')

while true do
  mq.doevents()
  settings.maybe_save()
  local cfg = settings.data or {}

  if pause.active(cfg.pause) then
    if next(active) then active = {} end
    mq.delay(250)
  else
    casts.config({ radius = cfg.radius })
    casts.update()
    local timeNow = casts.now()

    local seen = {}
    for id, castInfo in pairs(casts.all()) do
      if not (castInfo.interrupted or (castInfo.isSelf and castInfo.done)) then
        seen[id] = true
        local activeCast = active[id]
        if not activeCast or activeCast.spellID ~= castInfo.spellID or activeCast.castStart ~= castInfo.startedAt then
          local spawn = mq.TLO.Spawn(id) --[[@as spawn]]
          active[id] = {
            id        = id,
            spellID   = castInfo.spellID,
            castStart = castInfo.startedAt,
            geom      = castInfo.spellID and eqgfx.spell_geom(castInfo.spellID) or nil,
            name      = castInfo.casterName,
            isSelf    = castInfo.isSelf,
            friend    = castInfo.isSelf or (spawn() and classify_friend(spawn)) or false,
            targetID  = activeCast and activeCast.targetID or nil,
          }
        end
      end
    end
    for id, activeCast in pairs(active) do
      if not seen[id] then
        if activeCast.stub then
          if timeNow > (activeCast.expireAt or 0) then active[id] = nil end
        else
          active[id] = nil
        end
      end
    end

    local tgt = mq.TLO.Target
    local tid = tgt.ID()
    if tid and active[tid] and not active[tid].isSelf then
      local vid = tgt.AggroHolder.ID()
      if vid and vid > 0 and active[tid].targetID ~= vid then
        active[tid].targetID = vid
        announce(tid, active[tid].spellID or 0, vid)
      end
    end

    mq.delay(50)
  end
end
