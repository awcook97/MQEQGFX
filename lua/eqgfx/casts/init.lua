--[[
  casts/init.lua - shared cast detection & tracking for eqgfx features.

  Two detection sources, picked automatically per build:

  1. PLUGIN FEED (preferred). When the MQEQGFX cast feed is live
     (eqgfx.start_casting_addr() ~= 0) the plugin keeps every spawn's current
     cast on the spawn itself, so this module reads it with plain TLO members:
       - your own cast:    Me.Casting              (native, always accurate)
       - any other spawn:  Spawn[id].Casting       (the spell being cast) and
                           Spawn[id].CastTimeLeft  (server time remaining, ms)
     Other spawns are found by a radius-bounded scan each update. Nothing here
     depends on chat, spell links, duplicate names or animations.

  2. CHAT FALLBACK. When the plugin feed is NOT available on this build
     (older/emu MacroQuest without the cast-start offset), other spawns' casts
     are detected the old way: the "<name> begins to cast <spell>" chat line
     (keeping the spell link for the spell ID), progress from elapsed-vs-
     CastTime, caster name resolved to a spawn ID preferring one in a casting
     animation, interrupts from chat lines + the animation heuristic. Your own
     cast still comes from Me.Casting.

  The two paths never run at once - update() and the chat handlers both check
  eqgfx.start_casting_addr(), so on a plugin-fed build the chat handlers are
  inert, and on a build without the feed the plugin scan is skipped.

  Each running script keeps its own copy of this state (MQ Lua states are
  isolated), so nameplates and indicators can both track independently.

  Usage:
    local casts = require('eqgfx.casts')
    casts.init{ log = log, trackSelf = true, radius = 300 }
    -- main loop (after mq.doevents(), which the chat fallback needs):
    casts.update()
    local castInfo = casts.get(spawnID)        -- CastInfo or nil
    local pct, remain = casts.progress(castInfo, os.clock())
]]

local mq    = require('mq')
local eqgfx = require('eqgfx')
local types = require('eqgfx.casts._types')

local CAST_ANIMS = types.CAST_ANIMS

local M = { CAST_ANIMS = CAST_ANIMS }

--- Tracker configuration. All fields are optional in init()/config() calls;
--- the defaults below fill the gaps.
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

local active        = {}    -- spawnID -> CastInfo
local pendingByName = {}    -- casterName -> { spellID, spellName, at }  (chat fallback)
local registered    = false
local selfSpellID   = nil

local function dbg(fmt, ...)
  if trackerCfg.log then trackerCfg.log.Debug(fmt, ...) end
end

--- Is the plugin's cast feed live on this build? Decides which detection path
--- runs: the plugin per-spawn read (true) or the chat fallback (false).
---@return boolean live
local function feed_live()
  local ok, addr = pcall(function() return eqgfx.start_casting_addr() end)
  return ok and type(addr) == 'number' and addr ~= 0
end
M.feed_live = feed_live

--- SpellIcon atlas cell for a spell, or nil when unknown.
---@param spellID integer|nil
---@return integer|nil icon # cell index in the A_SpellIcons atlas
local function spell_icon(spellID)
  if not spellID then return nil end
  local ic = mq.TLO.Spell(spellID).SpellIcon()
  if type(ic) == 'number' and ic >= 0 then return ic end
  return nil
end

--- Cast time of a spell in wall-clock seconds (3s fallback when unknown).
---
--- ```lua
--- local dur = casts.cast_seconds(spellID)
--- ```
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

--- Base CastInfo for a spawn, with icon filled in.
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

--- End a cast: interrupted (ended early) or finished. Both linger briefly so
--- the renderer can fade them out; expiry happens in update().
---@param castInfo CastInfo
---@param timeNow number
local function finish(castInfo, timeNow)
  castInfo._ended = true
  local early = timeNow < castInfo.startedAt + castInfo.duration - 0.25
  if early and (castInfo.isSelf or trackerCfg.interruptDetect) then
    castInfo.interrupted, castInfo.interruptedAt = true, timeNow
    dbg('cast interrupted: %s (%s)', castInfo.casterName, tostring(castInfo.spellName))
  elseif early then
    active[castInfo.spawnID] = nil   -- drop without an interrupt fade
  end
  if castInfo.isSelf then castInfo.done = true end
end

-- ---------------------------------------------------------------------------
-- Self cast (both paths): read straight off Me.Casting - native and accurate.
-- ---------------------------------------------------------------------------
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

-- ===========================================================================
-- PLUGIN FEED PATH
-- ===========================================================================

--- Server time remaining on a spawn's cast, in seconds, from the plugin's
--- CastTimeLeft member (milliseconds). Returns nil when it isn't a usable
--- number (feed off, member absent) or is out of a sane range - the caller
--- then falls back to client cast-time pacing.
---@param spawn spawn # spawn TLO of the caster
---@return number|nil seconds
local function remain_seconds(spawn)
  ---@diagnostic disable-next-line: undefined-field
  local ok, v = pcall(function() return spawn.CastTimeLeft() end)
  if not ok or type(v) ~= 'number' or v <= 0 then return nil end
  local s = v / 1000
  if s > 60 then return nil end   -- guard against a unit surprise / stale ETA
  return s
end

--- Anchor a freshly detected cast to the server's reported remaining time, so
--- the bar tracks what the client shows and picks up a spawn already mid-cast
--- at the right point. No-op when CastTimeLeft isn't usable.
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

--- Read one spawn's current cast off the plugin and reconcile it with what we
--- track: start new casts, leave running ones anchored, end vanished ones.
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
      return   -- still the same cast; its start/duration stay anchored
    end
    cast = make_cast(id, spawn.CleanName() or '?', cur, mq.TLO.Spell(cur).Name(), false, timeNow)
    anchor(cast, spawn, timeNow)
  elseif cast and not cast._ended then
    finish(cast, timeNow)
  end
end

--- Plugin path per-update work: refresh tracked casts, discover new casters.
---@param timeNow number
local function update_plugin(timeNow)
  -- Re-check the casts we already track (including lingering _ended ones, so a
  -- quick recast is caught before the old entry expires).
  for id, cast in pairs(active) do
    if not cast.isSelf then
      local spawn = mq.TLO.Spawn(id) --[[@as spawn]]
      if spawn.ID() == id then
        poll_spawn(id, spawn, timeNow)
      elseif not cast._ended then
        finish(cast, timeNow)   -- caster gone (dead / out of range)
      end
    end
  end

  -- Discover new casters within range.
  local myID = mq.TLO.Me.ID() or 0
  local spec = 'radius ' .. (trackerCfg.radius or 300)
  local cnt = mq.TLO.SpawnCount(spec)() or 0
  for i = 1, cnt do
    local spawn = mq.TLO.NearestSpawn(string.format('%d, %s', i, spec)) --[[@as spawn]]
    local id = spawn.ID()
    if id and id > 0 and id ~= myID and not active[id] then
      poll_spawn(id, spawn, timeNow)
    end
  end
end

-- ===========================================================================
-- CHAT FALLBACK PATH  (only when the plugin feed is unavailable)
-- ===========================================================================

--- Resolve a caster name to its spawn, preferring one in a cast animation
--- (several spawns can share a name).
---@param name string # clean caster name from the chat line
---@return spawn|nil spawn # the spawn TLO, or nil when nothing matches
local function resolve_caster(name)
  local cnt = mq.TLO.SpawnCount('npc ' .. name)() or 0
  local fallback = nil
  for i = 1, cnt do
    local spawn = mq.TLO.NearestSpawn(string.format('%d, npc %s', i, name)) --[[@as spawn]]
    if spawn() and spawn.ID() and spawn.ID() > 0 then
      if CAST_ANIMS[spawn.Animation()] then return spawn end
      fallback = fallback or spawn
    end
  end
  if fallback then return fallback end
  local spawn = mq.TLO.Spawn('npc ' .. name) --[[@as spawn]]
  if spawn() and spawn.ID() and spawn.ID() > 0 then return spawn end
  spawn = mq.TLO.Spawn('pc =' .. name) --[[@as spawn]] -- other players cast too
  if spawn() and spawn.ID() and spawn.ID() > 0 then return spawn end
  return nil
end

--- Build + register a chat-detected cast for a resolved spawn.
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

-- Chat event: resolve the caster IMMEDIATELY (handlers run inside
-- mq.doevents, so the cast bar exists the instant the line arrives).
-- Unresolvable names go to pending and get retried from update().
local function on_cast(line, casterName)
  if feed_live() then return end   -- plugin path owns detection on this build
  if not casterName or casterName == '' then return end
  if casterName == (mq.TLO.Me.CleanName() or '') then return end  -- self via Me.Casting

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
    activate_chat(spawn, casterName, spellID, spellName, os.clock())
    dbg('cast evt (instant): "%s" -> %s', casterName, tostring(spellName))
  else
    pendingByName[casterName] = { spellID = spellID, spellName = spellName, at = os.clock() }
    dbg('cast evt (pending): "%s" -> %s', casterName, tostring(spellName))
  end
end

-- Interrupt lines name the caster directly - more reliable than the animation
-- heuristic (which needs the spawn close enough to see the anim).
local function on_interrupt(name)
  if feed_live() then return end
  if not name or name == '' then return end
  local timeNow = os.clock()
  for _, cast in pairs(active) do
    if not cast.isSelf and not cast.interrupted and cast.casterName == name then
      cast.interrupted, cast.interruptedAt = true, timeNow
      dbg('cast interrupted (event): %s', name)
    end
  end
  pendingByName[name] = nil
end

--- Chat path per-update work: retry unresolved names, run the animation
--- interrupt heuristic.
---@param timeNow number
local function update_chat(timeNow)
  -- Retry casts whose caster couldn't be resolved at event time.
  for name, pending in pairs(pendingByName) do
    local spawn = resolve_caster(name)
    if spawn and spawn.ID() and spawn.ID() > 0 then
      activate_chat(spawn, name, pending.spellID, pending.spellName, timeNow)
      pendingByName[name] = nil
    elseif timeNow - pending.at > 3 then
      pendingByName[name] = nil
    end
  end

  -- Animation-based interrupt heuristic for casts still in flight.
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

-- ===========================================================================
-- Public API
-- ===========================================================================

--- Set up the tracker: registers the chat-fallback events (once) and applies
--- config overrides. The chat events stay inert on a build whose plugin feed
--- is live, but are registered either way so detection survives the feed being
--- absent. Your main loop MUST call mq.doevents() (the fallback rides on it).
---
--- ```lua
--- local casts = require('eqgfx.casts')
--- casts.init{ trackSelf = true, radius = 300 }
--- while true do
---   mq.doevents()
---   casts.update()
---   mq.delay(50)
--- end
--- ```
---@param opts CastTrackerCfg|nil # overrides: log, trackSelf, interruptDetect, radius, grace, interruptLinger
function M.init(opts)
  if opts then for k, v in pairs(opts) do trackerCfg[k] = v end end
  if registered then return end
  registered = true
  -- "begins to cast a spell." and "begins casting <spell>." - the named/link
  -- form carries the spell link in the raw line (keepLinks).
  mq.event('eqgfx_casts1', "#1# begins to cast #2#.#*#", function(l, n) on_cast(l, n) end, { keepLinks = true })
  mq.event('eqgfx_casts2', "#1# begins casting #2#.#*#", function(l, n) on_cast(l, n) end, { keepLinks = true })
  mq.event('eqgfx_casts_i1', "#1#'s casting is interrupted#*#",     function(_, n) on_interrupt(n) end)
  mq.event('eqgfx_casts_i2', "#1#'s spell is interrupted#*#",        function(_, n) on_interrupt(n) end)
  -- actual live format: "Soandso's Adamant Stance spell is interrupted."
  mq.event('eqgfx_casts_i3', "#1#'s #2# spell is interrupted#*#",    function(_, n) on_interrupt(n) end)
  mq.event('eqgfx_casts_i4', "#1#'s #2# casting is interrupted#*#",  function(_, n) on_interrupt(n) end)
end

--- Change tracker config at runtime (settings menus call this every pass).
---
--- ```lua
--- casts.config{ interruptDetect = cfg.castbar.interruptDetect, radius = cfg.radius }
--- ```
---@param opts table # any CastTrackerCfg fields to override
function M.config(opts)
  for k, v in pairs(opts) do trackerCfg[k] = v end
end

--- The tracker's clock. Spell durations are wall-clock seconds, so cast math
--- (progress/expiry) must use THIS time base, not a frame counter.
---
--- ```lua
--- local pct, remain = casts.progress(info, casts.now())
--- ```
---@return number now # os.clock() seconds
function M.now()
  return os.clock()
end

--- Advance the tracker: pick up your own cast (Me.Casting), then run whichever
--- detection path this build supports (plugin per-spawn read, else chat
--- fallback), then expire finished casts. Call once per main-loop pass, after
--- mq.doevents().
function M.update()
  local timeNow = os.clock()
  if trackerCfg.trackSelf then update_self(timeNow) end

  if feed_live() then
    update_plugin(timeNow)
  else
    update_chat(timeNow)
  end

  -- Expiry: interrupted casts linger interruptLinger; everything else grace.
  for id, cast in pairs(active) do
    if cast.interrupted then
      if timeNow > (cast.interruptedAt or timeNow) + trackerCfg.interruptLinger then active[id] = nil end
    elseif timeNow > cast.startedAt + cast.duration + trackerCfg.grace then
      active[id] = nil
    end
  end
end

--- The active cast for one spawn, if any. On a plugin-fed build this lazily
--- pulls the spawn's current cast so per-plate callers work even for spawns
--- outside the discovery scan radius; on the chat-fallback build it just
--- returns what the events have populated.
---
--- ```lua
--- local info = casts.get(mq.TLO.Target.ID() or 0)
--- if info and not info.interrupted then ... end
--- ```
---@param spawnID integer # caster spawn ID
---@return CastInfo|nil castInfo # nil when that spawn isn't casting
function M.get(spawnID)
  local cast = active[spawnID]
  if cast then return cast end
  if not spawnID or spawnID <= 0 or not feed_live() then return nil end
  if spawnID == (mq.TLO.Me.ID() or -1) then return nil end   -- self is update_self's job
  local spawn = mq.TLO.Spawn(spawnID) --[[@as spawn]]
  if spawn.ID() == spawnID then
    poll_spawn(spawnID, spawn, os.clock())
  end
  return active[spawnID]
end

--- Every cast currently in flight, keyed by caster spawn ID. Includes
--- recently finished/interrupted entries while they linger for fade-outs.
---@return table<integer, CastInfo> active
function M.all() return active end

--- Progress of a cast on the tracker clock.
---
--- ```lua
--- local pct, remain = casts.progress(info, casts.now())
--- bar_fill(pct); label(string.format('%.1fs', remain))
--- ```
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
