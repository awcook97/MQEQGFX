local mq    = require('mq')
local eqgfx = require('eqgfx')
local types = require('eqgfx.casts._types')

local CAST_ANIMS = types.CAST_ANIMS

local M = { CAST_ANIMS = CAST_ANIMS }

---@class CastTrackerCfg
---@field log table|nil # lwlogger-style module (Info/Warn/Debug); nil = silent
---@field trackSelf boolean|nil # include your own casts (via Me.Casting)
---@field interruptDetect boolean|nil # detect interrupts (early-ended casts / animation drop / chat lines)
---@field radius number|nil # detection radius (units) for the plugin-fed nearby-spawn scan
---@field grace number|nil # seconds a finished cast lingers (finish pulses)
---@field interruptLinger number|nil # seconds an interrupted cast lingers (fade-outs)
local trackerCfg = {
  log             = nil,
  trackSelf       = true,
  interruptDetect = true,
  radius          = 300,
  grace           = 1.0,
  interruptLinger = 0.8,
}

local SCAN_INTERVAL = 0.2

local active        = {}
local pendingByName = {}
local registered    = false
local selfSpellID   = nil
local lastScanAt    = 0

local function dbg(fmt, ...)
  if trackerCfg.log then trackerCfg.log.Debug(fmt, ...) end
end

---@return boolean live
local function feed_live()
  return eqgfx.cast_feed_live()
end
M.feed_live = feed_live

---@param spellID integer|nil
---@return integer|nil icon # cell index in the A_SpellIcons atlas
local function spell_icon(spellID)
  if not spellID then return nil end
  local ic = mq.TLO.Spell(spellID).SpellIcon()
  if type(ic) == 'number' and ic >= 0 then return ic end
  return nil
end

---@param spellID integer|nil
---@return number seconds # MyCastTime, else CastTime, else 3
local function cast_seconds(spellID)
  if not spellID then return 3 end
  local ms = mq.TLO.Spell(spellID).MyCastTime()
  if not ms or ms <= 0 then ms = mq.TLO.Spell(spellID).CastTime() end
  if type(ms) == 'number' and ms > 0 then return ms / 1000 end
  return 3
end
M.cast_seconds = cast_seconds

---@param id integer
---@param casterName string
---@param spellID integer|nil
---@param spellName string|nil
---@param isSelf boolean
---@param timeNow number
---@return CastInfo
local function make_cast(id, casterName, spellID, spellName, isSelf, timeNow)
  local castInfo = types.CastInfo(id, casterName, spellID, spellName,
                                  cast_seconds(spellID), isSelf, timeNow)
  castInfo.spellIcon = spell_icon(spellID)
  active[id] = castInfo
  return castInfo
end

---@param castInfo CastInfo
---@param timeNow number
local function finish(castInfo, timeNow)
  castInfo._ended = true
  local early = timeNow < castInfo.startedAt + castInfo.duration - 0.25
  if early and (castInfo.isSelf or trackerCfg.interruptDetect) then
    castInfo.interrupted, castInfo.interruptedAt = true, timeNow
    dbg('cast interrupted: %s (%s)', castInfo.casterName, tostring(castInfo.spellName))
  elseif early then
    active[castInfo.spawnID] = nil
  end
  if castInfo.isSelf then castInfo.done = true end
end

local function update_self(timeNow)
  local myid = mq.TLO.Me.ID()
  if not myid or myid <= 0 then return end
  local sc = mq.TLO.Me.Casting.ID()
  if sc and sc > 0 then
    if selfSpellID ~= sc then
      selfSpellID = sc
      make_cast(myid, mq.TLO.Me.CleanName() or 'me', sc, mq.TLO.Spell(sc).Name(), true, timeNow)
    end
  else
    selfSpellID = nil
    local cast = active[myid]
    if cast and cast.isSelf and not cast._ended then
      finish(cast, timeNow)
    end
  end
end

---@param spawn spawn # spawn TLO of the caster
---@return number|nil seconds
local function remain_seconds(spawn)
  ---@diagnostic disable-next-line: undefined-field
  local ok, v = pcall(function() return spawn.CastTimeLeft() end)
  if not ok or type(v) ~= 'number' or v <= 0 then return nil end
  local s = v / 1000
  if s > 60 then return nil end
  return s
end

---@param castInfo CastInfo
---@param spawn spawn
---@param timeNow number
local function anchor(castInfo, spawn, timeNow)
  local remain = remain_seconds(spawn)
  if not remain then return end
  if remain > castInfo.duration then castInfo.duration = remain end
  local started = timeNow - (castInfo.duration - remain)
  if started > timeNow then started = timeNow end
  castInfo.startedAt = started
end

---@param id integer # caster spawn ID (not you)
---@param spawn spawn # its spawn TLO
---@param timeNow number
local function poll_spawn(id, spawn, timeNow)
  local sid = spawn.Casting.ID()
  ---@type integer|nil
  local cur = (type(sid) == 'number' and sid > 0) and sid or nil
  local cast = active[id]

  if cur then
    if cast and cast.spellID == cur and not cast._ended then
      return
    end
    cast = make_cast(id, spawn.CleanName() or '?', cur, mq.TLO.Spell(cur).Name(), false, timeNow)
    anchor(cast, spawn, timeNow)
  elseif cast and not cast._ended then
    finish(cast, timeNow)
  end
end

---@param timeNow number
local function update_plugin(timeNow)
  for id, cast in pairs(active) do
    if not cast.isSelf then
      local spawn = mq.TLO.Spawn(id) --[[@as spawn]]
      if spawn.ID() == id then
        poll_spawn(id, spawn, timeNow)
      elseif not cast._ended then
        finish(cast, timeNow)
      end
    end
  end

  if timeNow - lastScanAt < SCAN_INTERVAL then return end
  lastScanAt = timeNow

  local myID = mq.TLO.Me.ID() or 0
  for _, id in ipairs(eqgfx.spawn_search('radius ' .. (trackerCfg.radius or 300))) do
    if id ~= myID and not active[id] then
      poll_spawn(id, mq.TLO.Spawn(id) --[[@as spawn]], timeNow)
    end
  end
end

---@param name string # clean caster name from the chat line
---@return spawn|nil spawn # the spawn TLO, or nil when nothing matches
local function resolve_caster(name)
  local fallback = nil
  for _, id in ipairs(eqgfx.spawn_search('npc ' .. name)) do
    local spawn = mq.TLO.Spawn(id) --[[@as spawn]]
    if spawn() then
      if CAST_ANIMS[spawn.Animation()] then return spawn end
      fallback = fallback or spawn
    end
  end
  if fallback then return fallback end
  local spawn = mq.TLO.Spawn('npc ' .. name) --[[@as spawn]]
  if spawn() and spawn.ID() and spawn.ID() > 0 then return spawn end
  spawn = mq.TLO.Spawn('pc =' .. name) --[[@as spawn]]
  if spawn() and spawn.ID() and spawn.ID() > 0 then return spawn end
  return nil
end

---@param spawn spawn # spawn TLO of the caster
---@param casterName string
---@param spellID integer|nil
---@param spellName string|nil
---@param timeNow number
local function activate_chat(spawn, casterName, spellID, spellName, timeNow)
  local id = spawn.ID()
  local castInfo = make_cast(id, casterName, spellID, spellName, false, timeNow)
  if CAST_ANIMS[spawn.Animation()] then castInfo.sawAnim, castInfo.lastAnimAt = true, timeNow end
end

local function on_cast(line, casterName)
  if feed_live() then return end
  if not casterName or casterName == '' then return end
  if casterName == (mq.TLO.Me.CleanName() or '') then return end

  local spellID, spellName
  local okk, links = pcall(mq.ExtractLinks, line)
  if okk and links then
    for _, lnk in ipairs(links) do
      if lnk.type == mq.LinkTypes.Spell then
        local ok2, s = pcall(mq.ParseSpellLink, lnk.link)
        if ok2 and s then spellID, spellName = s.spellID, s.spellName end
        break
      end
    end
  end

  local spawn = resolve_caster(casterName)
  if spawn and spawn.ID() and spawn.ID() > 0 then
    activate_chat(spawn, casterName, spellID, spellName, M.now())
    dbg('cast evt (instant): "%s" -> %s', casterName, tostring(spellName))
  else
    pendingByName[casterName] = { spellID = spellID, spellName = spellName, at = M.now() }
    dbg('cast evt (pending): "%s" -> %s', casterName, tostring(spellName))
  end
end

local function on_interrupt(name)
  if feed_live() then return end
  if not name or name == '' then return end
  local timeNow = M.now()
  for _, cast in pairs(active) do
    if not cast.isSelf and not cast.interrupted and cast.casterName == name then
      cast.interrupted, cast.interruptedAt = true, timeNow
      dbg('cast interrupted (event): %s', name)
    end
  end
  pendingByName[name] = nil
end

---@param timeNow number
local function update_chat(timeNow)
  for name, pending in pairs(pendingByName) do
    local spawn = resolve_caster(name)
    if spawn and spawn.ID() and spawn.ID() > 0 then
      activate_chat(spawn, name, pending.spellID, pending.spellName, timeNow)
      pendingByName[name] = nil
    elseif timeNow - pending.at > 3 then
      pendingByName[name] = nil
    end
  end

  for id, cast in pairs(active) do
    if not cast.isSelf and not cast.interrupted and trackerCfg.interruptDetect then
      local spawn = mq.TLO.Spawn(id)
      if spawn.ID() == id then
        local anim = spawn.Animation()
        if CAST_ANIMS[anim] then
          cast.sawAnim, cast.lastAnimAt = true, timeNow
        elseif cast.sawAnim and cast.lastAnimAt
           and timeNow - cast.lastAnimAt > 0.3
           and timeNow < cast.startedAt + cast.duration - 0.2 then
          cast.interrupted, cast.interruptedAt = true, timeNow
          dbg('cast interrupted: %s (%s)', cast.casterName, tostring(cast.spellName))
        end
      end
    end
  end
end

---@param opts CastTrackerCfg|nil # overrides: log, trackSelf, interruptDetect, radius, grace, interruptLinger
function M.init(opts)
  if opts then for k, v in pairs(opts) do trackerCfg[k] = v end end
  if registered then return end
  registered = true
  mq.event('eqgfx_casts1', "#1# begins to cast #2#.#*#", function(l, n) on_cast(l, n) end, { keepLinks = true })
  mq.event('eqgfx_casts2', "#1# begins casting #2#.#*#", function(l, n) on_cast(l, n) end, { keepLinks = true })
  mq.event('eqgfx_casts_i1', "#1#'s casting is interrupted#*#",     function(_, n) on_interrupt(n) end)
  mq.event('eqgfx_casts_i2', "#1#'s spell is interrupted#*#",        function(_, n) on_interrupt(n) end)
  mq.event('eqgfx_casts_i3', "#1#'s #2# spell is interrupted#*#",    function(_, n) on_interrupt(n) end)
  mq.event('eqgfx_casts_i4', "#1#'s #2# casting is interrupted#*#",  function(_, n) on_interrupt(n) end)
end

---@param opts table # any CastTrackerCfg fields to override
function M.config(opts)
  for k, v in pairs(opts) do trackerCfg[k] = v end
end

---@return number now # seconds on the mq.gettime() clock
function M.now()
  return mq.gettime() / 1000
end

function M.update()
  local timeNow = M.now()
  if trackerCfg.trackSelf then update_self(timeNow) end

  if feed_live() then
    update_plugin(timeNow)
  else
    update_chat(timeNow)
  end

  for id, cast in pairs(active) do
    if cast.interrupted then
      if timeNow > (cast.interruptedAt or timeNow) + trackerCfg.interruptLinger then active[id] = nil end
    elseif timeNow > cast.startedAt + cast.duration + trackerCfg.grace then
      active[id] = nil
    end
  end
end

---@param spawnID integer # caster spawn ID
---@return CastInfo|nil castInfo # nil when that spawn isn't casting
function M.get(spawnID)
  local cast = active[spawnID]
  if cast then return cast end
  if not spawnID or spawnID <= 0 or not feed_live() then return nil end
  if spawnID == (mq.TLO.Me.ID() or -1) then return nil end
  local spawn = mq.TLO.Spawn(spawnID) --[[@as spawn]]
  if spawn.ID() == spawnID then
    poll_spawn(spawnID, spawn, M.now())
  end
  return active[spawnID]
end

---@return table<integer, CastInfo> active
function M.all() return active end

---@param castInfo CastInfo
---@param timeNow number # casts.now() - NOT a frame counter
---@return number pct # fraction 0..1, clamped
---@return number remain # seconds remaining, >= 0
function M.progress(castInfo, timeNow)
  local elapsed = timeNow - castInfo.startedAt
  local pct = (castInfo.duration > 0) and (elapsed / castInfo.duration) or 1
  if pct < 0 then pct = 0 elseif pct > 1 then pct = 1 end
  local remain = castInfo.duration - elapsed
  if remain < 0 then remain = 0 end
  return pct, remain
end

return M
