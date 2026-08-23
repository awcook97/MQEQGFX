--[[
  highlight/init.lua - four layers of world decoration, one loop.

  1. MODEL TINT (eqgfx.highlight)
     Your target's actual 3D model is recolored by what it is:
       purple  whatever the raid/group main assist is targeting
       blue    player character (or mercenary)
       gold    a pet
       red     NPC that is currently aggressive
       white   NPC that is not
     The MA's target is lit whether or not it is also yours, and purple
     outranks the other categories on the same spawn.

  2. ROLE MARKERS (markers.lua)
     An animated line figure on the main tank and the main assist. These only
     appear on spawns holding a group/raid role - /hlmenu reports what the
     lookups resolved to and has a test mode that draws both on you.

  3. AMBIENT (ambient.lua)
     Every NPC in line of sight gets a randomized idle effect. "Randomized"
     means derived from the spawn ID, not re-rolled per frame: the crowd looks
     hand-varied while any one mob animates smoothly. Off by default - it is
     the expensive layer.

  4. CAST FX (castfx.lua)
     A distinct animation per spell, shaped by the spell's real footprint
     (beam / cone / caster area / target area / single / self, read from
     eqgfx.spell_geom) and colored by what it does - configured heal color for
     beneficial, resist-type color for detrimental. Casts animate while the
     bar fills, burst when they land, and unwind when interrupted.

  THE FRAME BUDGET
  ----------------
  Layers 2-4 all emit world line segments into one shared cap (settings
  `budget`). They are served in priority order - markers, then casts, then
  ambient - so a crowded zone degrades by thinning the ambient layer rather
  than by dropping the thing you actually needed to see. shapes.lua owns the
  counter; every layer checks S.over() and bails out whole.

  Run:  /lua run eqgfx/highlight
  Stop: /lua stop eqgfx/highlight
  Opts: /hlmenu        the settings window
        /hl            toggle everything off/on (clears tints when off)
        /hl markers | tints | ambient | casts    toggle one layer
        /hl test       draw both role markers on yourself

  NOTE: model tints are plugin state, not script state. Stopping this script
  with /lua stop leaves the last tint applied until you zone or unload the
  plugin. Use /hl to turn it off first, or just re-run the script - it clears
  any orphans on startup.

  NOTE: the drawing here uses the shared retained scene, so this script calls
  eqgfx.clear() every frame. Nameplates, indicators and dmgnums all draw with
  ImGui and are unaffected, but tools/debug.lua and tools/calibrate.lua use
  the same scene and will fight with this one - run them one at a time.
]]

local mq    = require('mq')
local eqgfx = require('eqgfx')
local log   = require('eqgfx.lib.lwlogger')

local types    = require('eqgfx.highlight._types')
local settings = require('eqgfx.highlight.settings')
local menu     = require('eqgfx.highlight.menu')
local S        = require('eqgfx.highlight.shapes')
local markers  = require('eqgfx.highlight.markers')
local ambient  = require('eqgfx.highlight.ambient')
local classanim = require('eqgfx.highlight.classanim')
local castfx   = require('eqgfx.highlight.castfx')
local casts    = require('eqgfx.casts')
local pause    = require('eqgfx.lib.pause')

log.SetAppName('eqgfx')
log.SetModuleName('highlight')
log.SetColors(true)
log.SetIncludeCharacter('Server.Character.Zone')
log.SetIncludeTime('milliseconds')
log.SetLevel(log.INFO)
log.SetOutputFile(mq.configDir .. '/eqgfx.log')

local ok, err = eqgfx.init()
if not ok then log.Error('init failed: %s', err) return end
if not eqgfx.highlight then
  log.Error('this MQEQGFX build has no model highlighting (eqgfx.highlight missing) - rebuild/update the plugin')
  return
end

settings.load()

S.set_packer(eqgfx.argb)
castfx.set_geom_source(eqgfx.spell_geom)

local floor, max, min, sqrt = math.floor, math.max, math.min, math.sqrt

----------------------------------------------------------------------------
-- TLO helpers. Every read is guarded: members go missing between builds and
-- a spawn can vanish between the ID read and the field read.
----------------------------------------------------------------------------

--- pcall a TLO getter and keep only a successful result.
---@generic T
---@param f fun(): T
---@return T|nil
local function try(f)
  local okCall, v = pcall(f)
  if okCall then return v end
end

--- Normalize a TLO spawn ID to nil / a positive integer.
local function posid(v)
  if v and v > 0 then return v end
end

--- The spawn the main assist is on: raid assist 1 when we're in a raid,
--- otherwise the group's main assist target.
---@return integer|nil spawnID
local function assist_target_id()
  local raidSize = try(function() return mq.TLO.Raid.Members() end) or 0
  if raidSize > 0 then
    local id = posid(try(function() return mq.TLO.Me.RaidAssistTarget(1).ID() end))
    if id then return id end
  end
  return posid(try(function() return mq.TLO.Me.GroupAssistTarget.ID() end))
end

--- Main tank / main assist spawn IDs.
---
--- Both Group.MainTank and Group.MainAssist hand back a group *member*, so the
--- spawn is one hop further in. The raid has a MainAssist (up to three; we
--- take the first) but no main tank of its own, so the tank always comes from
--- your group - which is the one you care about in a raid anyway.
---@return integer|nil tankID, integer|nil assistID
local function role_spawn_ids()
  local tank = posid(try(function() return mq.TLO.Group.MainTank.Spawn.ID() end))
  local assist = posid(try(function() return mq.TLO.Raid.MainAssist(1).Spawn.ID() end))
      or posid(try(function() return mq.TLO.Group.MainAssist.Spawn.ID() end))
  return tank, assist
end

--- Which tint bucket a spawn falls into. `maTargetID` is passed in so the
--- caller reads it once per frame rather than per spawn.
---@param id integer
---@param maTargetID integer|nil
---@return HighlightRole|nil
local function classify(id, maTargetID)
  if maTargetID and id == maTargetID then return 'assistTarget' end

  local spawn = mq.TLO.Spawn('id ' .. id)
  local kind = try(function() return spawn.Type() end)
  if not kind then return end

  -- Pet before PC: check it first so a mercenary-owned pet can't fall
  -- through into the PC bucket.
  if kind == 'Pet' then return 'pet' end
  if types.PlayerTypes[kind] then return 'pc' end
  if kind ~= 'NPC' then return end          -- corpses, chests, auras: leave alone

  -- "Aggressive" is PlayerState combat/aggro bits, i.e. it is fighting
  -- something right now. EQ exposes no faction-will-attack-me flag to the
  -- client, so a KOS mob standing idle still reads as neutral here.
  return try(function() return spawn.Aggressive() end) and 'hostile' or 'neutral'
end

--- World position of a spawn. Returns nil unless all three axes read back -
--- callers do arithmetic with them.
---@param id integer
---@return number|nil x, number|nil y, number|nil z
local function spawn_pos(id)
  local spawn = mq.TLO.Spawn('id ' .. id)
  local x = try(function() return spawn.X() end)
  local y = try(function() return spawn.Y() end)
  local z = try(function() return spawn.Z() end)
  if x and y and z then return x, y, z end
end

---@param id integer|nil
---@return string|nil
local function spawn_name(id)
  if not id then return end
  return try(function() return mq.TLO.Spawn('id ' .. id).CleanName() end)
end

----------------------------------------------------------------------------
-- Model tints. The plugin holds the highlights; we hold the diff so we only
-- call across on an actual change.
----------------------------------------------------------------------------

---@type table<integer, integer>  spawn ID -> ARGB we last asked for
local lit = {}

--- Reconcile the plugin's highlight set with `want`.
---@param want table<integer, integer>  # spawn ID -> ARGB
---@param mode integer
local function apply_tints(want, mode)
  for id, color in pairs(want) do
    if lit[id] ~= color then
      eqgfx.highlight(id, color, mode)
      lit[id] = color
    end
  end
  for id in pairs(lit) do
    if not want[id] then
      eqgfx.unhighlight(id)
      lit[id] = nil
    end
  end
end

local function clear_tints()
  for id in pairs(lit) do eqgfx.unhighlight(id) end
  lit = {}
end

----------------------------------------------------------------------------
-- Class lookup. Spawn.Class.ShortName() is a two-hop TLO read and the ambient
-- layer would do it for every spawn every frame, so it is cached for the
-- session. A spawn ID is only reused after the original despawns, and the
-- worst case is one mob wearing a recycled class until you zone.
----------------------------------------------------------------------------

---@type table<integer, string|false>   false = looked up, has no class
local classCache = {}

---@param id integer
---@param spawn spawn|nil # pass one if you already have it
---@return string|nil # Spawn.Class.ShortName(), e.g. 'WIZ'
local function class_of(id, spawn)
  local hit = classCache[id]
  if hit ~= nil then return hit or nil end
  spawn = spawn or mq.TLO.Spawn('id ' .. id)
  local short = try(function() return spawn.Class.ShortName() end)
  if type(short) ~= 'string' or short == '' then short = nil end
  classCache[id] = short or false
  return short
end

----------------------------------------------------------------------------
-- Ambient discovery: nearby spawns, line-of-sight filtered, capped.
--
-- Runs in full every pass - no interval gating, no position cache. A LOS read
-- that errors counts as visible, so the filter never hides everything.
--
-- The per-spawn TLO reads here are the bulk of this script's cost, so the loop
-- is written to do the minimum: one handle per spawn from NearestSpawn, then
-- ID / Type / LineOfSight / X / Y / Z off that handle, and distance computed
-- in Lua from our own position rather than a seventh TLO call.
----------------------------------------------------------------------------

---@class AmbientTarget
---@field id integer
---@field x number @field y number @field z number
---@field dist number
---@field class string|nil # Spawn.Class.ShortName(), nil when unreadable

--- The spawns to animate this frame, sorted nearest-first when the cap might
--- bite so the crowd thins from the back.
---@param cfg table # settings.data.ambient
---@return AmbientTarget[]
local function ambient_targets(cfg)
  local spec = cfg.includePCs
      and ('radius ' .. cfg.radius .. ' zradius 100')
      or ('npc radius ' .. cfg.radius .. ' zradius 100')

  local myID = posid(try(function() return mq.TLO.Me.ID() end))
  local mx = try(function() return mq.TLO.Me.X() end) or 0
  local my = try(function() return mq.TLO.Me.Y() end) or 0
  local mz = try(function() return mq.TLO.Me.Z() end) or 0

  local out = {}
  local cnt = try(function() return mq.TLO.SpawnCount(spec)() end) or 0

  for i = 1, cnt do
    local spawn = mq.TLO.NearestSpawn(i .. ', ' .. spec) --[[@as spawn]]
    local id = posid(try(function() return spawn.ID() end))
    if id and id ~= myID then
      local kind = try(function() return spawn.Type() end)
      -- Corpses and props keep their own Type, so this also filters them out.
      local wanted = (kind == 'NPC') or (cfg.includePCs and types.PlayerTypes[kind])
      if wanted then
        local visible = true
        if cfg.losOnly then
          local okCall, v = pcall(function() return spawn.LineOfSight() end)
          visible = (not okCall) or (v and true or false)
        end
        if visible then
          local x = try(function() return spawn.X() end)
          local y = try(function() return spawn.Y() end)
          local z = try(function() return spawn.Z() end)
          if x and y and z then
            local dx, dy, dz = x - mx, y - my, z - mz
            out[#out + 1] = {
              id = id, x = x, y = y, z = z,
              dist = sqrt(dx * dx + dy * dy + dz * dz),
              class = class_of(id, spawn),
            }
          end
        end
      end
    end
  end

  if cfg.nearestFirst and #out > cfg.maxSpawns then
    table.sort(out, function(a, b) return a.dist < b.dist end)
  end
  while #out > cfg.maxSpawns do table.remove(out) end
  return out
end

--- Cache the style list so the per-spawn pick is stable and we aren't
--- rebuilding a table every frame. Rebuilt when the enabled set changes.
local enabledStyles, enabledKey = {}, nil

---@param cfg table # settings.data.ambient
---@return string[]
local function enabled_styles(cfg)
  local key = ''
  for _, id in ipairs(ambient.STYLES) do
    key = key .. (cfg.styles[id] and '1' or '0')
  end
  if key ~= enabledKey then
    enabledKey = key
    enabledStyles = {}
    for _, id in ipairs(ambient.STYLES) do
      if cfg.styles[id] then enabledStyles[#enabledStyles + 1] = id end
    end
  end
  return enabledStyles
end

----------------------------------------------------------------------------
-- Cast effects.
--
-- eqgfx.casts gives us casts in flight with progress and interrupt state, but
-- not "this one just landed" - a completed cast simply stops being in
-- casts.all(). So we diff the live set against the previous frame's: anything
-- that disappeared without being flagged interrupted completed, and gets a
-- release burst with its own lifetime.
----------------------------------------------------------------------------

---@class CastFx
---@field shape CastShape
---@field color RGBA
---@field x number @field y number @field z number   # caster position, frozen at release
---@field tx number @field ty number @field tz number # victim position
---@field heading number # caster facing, radians
---@field reach number
---@field coneStart number @field coneEnd number
---@field bornAt number
---@field life number
---@field fizzle boolean

---@type table<integer, CastFx>   spawnID -> effect in its release/fizzle phase
local bursts = {}
---@type table<integer, table>    spawnID -> last frame's CastInfo
local prevCasts = {}

--- Caster facing in radians, in the same frame add_line uses. EQ headings run
--- 0..512 counter-clockwise from north; the world math here is standard
--- atan2-style radians about +X, so convert rather than eyeballing it.
---@param id integer
---@return number
local function heading_rad(id)
  local h = try(function() return mq.TLO.Spawn('id ' .. id).Heading.DegreesCCW() end)
  if not h then
    local raw = try(function() return mq.TLO.Spawn('id ' .. id).Heading.Degrees() end)
    h = raw or 0
  end
  return math.rad(h)
end

--- Freeze everything a burst needs so it survives the caster dying mid-flight.
---@param info table       # CastInfo
---@param class CastClass
---@param cfg table        # settings.data.castfx
---@param nowMs number      # mq.gettime() milliseconds
---@param fizzle boolean
---@return CastFx|nil
local function make_burst(info, class, cfg, nowMs, fizzle)
  local x, y, z = spawn_pos(info.spawnID)
  if not x then return end

  -- Target-shaped spells need a victim. Target-of-target is the best the
  -- client offers for someone else's cast; fall back to the caster so the
  -- effect still plays somewhere sensible rather than at the origin.
  local tx, ty, tz = x, y, z
  if class.shape == 'single' or class.shape == 'targetArea' then
    local vid = posid(try(function() return mq.TLO.Spawn('id ' .. info.spawnID).TargetOfTarget.ID() end))
    if vid then
      local vx, vy, vz = spawn_pos(vid)
      if vx then tx, ty, tz = vx, vy, vz end
    end
  end

  return {
    shape     = class.shape,
    color     = castfx.color(class, cfg),
    x = x, y = y, z = z,
    tx = tx, ty = ty, tz = tz,
    heading   = heading_rad(info.spawnID),
    reach     = castfx.reach(class, cfg),
    coneStart = class.coneStart,
    coneEnd   = class.coneEnd,
    bornAt    = nowMs,
    life      = 1000 * (fizzle and cfg.fizzleLife or cfg.releaseLife),
    fizzle    = fizzle,
  }
end

--- Advance cast bookkeeping: spawn bursts for casts that ended, retire expired
--- ones. Returns the live cast set so the draw pass can render charges.
---@param cfg table # settings.data.castfx
---@param nowMs number # mq.gettime() milliseconds
---@return table<integer, table> live
local function update_casts(cfg, nowMs)
  casts.update()
  local live = casts.all()

  for id, info in pairs(prevCasts) do
    if not live[id] then
      -- Gone from the tracker: it either landed or was interrupted. The
      -- tracker lingers interrupted casts, so an interrupt is already flagged
      -- on the copy we kept.
      local fizzled = info.interrupted and true or false
      if (not fizzled) or cfg.fizzleLife > 0 then
        local n = 0
        for _ in pairs(bursts) do n = n + 1 end
        if n < cfg.maxActive then
          local class = castfx.classify(info.spellID)
          local b = make_burst(info, class, cfg, nowMs, fizzled)
          if b then bursts[id] = b end
        end
      end
    end
  end

  -- Snapshot only what we need, so a stale CastInfo can't keep a spawn alive.
  local snap = {}
  for id, info in pairs(live) do
    if cfg.selfCasts or not info.isSelf then
      snap[id] = { spawnID = id, spellID = info.spellID, interrupted = info.interrupted }
    end
  end
  prevCasts = snap

  for id, b in pairs(bursts) do
    if (nowMs - b.bornAt) >= b.life then bursts[id] = nil end
  end
  return live
end

----------------------------------------------------------------------------
-- Commands
----------------------------------------------------------------------------

mq.bind('/hlmenu', function() menu.toggle() end)

local LAYERS = {
  markers = { path = 'markers', label = 'role markers' },
  tints   = { path = 'tints',   label = 'model tints' },
  ambient = { path = 'ambient', label = 'ambient NPC animation' },
  casts   = { path = 'castfx',  label = 'cast animations' },
}

mq.bind('/hl', function(which)
  local cfg = settings.data
  which = (which or ''):lower()

  local layer = LAYERS[which]
  if layer then
    local t = cfg[layer.path]
    t.enabled = not t.enabled
    if which == 'tints' and not t.enabled then clear_tints() end
    log.Info('%s %s', layer.label, t.enabled and 'ON' or 'OFF')
  elseif which == 'test' then
    cfg.markers.testMode = not cfg.markers.testMode
    log.Info('marker test mode %s (both markers on you)', cfg.markers.testMode and 'ON' or 'OFF')
  else
    cfg.enabled = not cfg.enabled
    if not cfg.enabled then
      clear_tints()
      eqgfx.blank()
    end
    log.Info('highlighting %s', cfg.enabled and 'ON' or 'OFF')
  end
  settings.mark_dirty()
end)

mq.imgui.init('eqgfx_highlight', function() menu.draw() end)

----------------------------------------------------------------------------
-- Main loop. Rebuild the whole scene every pass: everything here is animated,
-- so there is nothing worth retaining between frames, and nothing throttles
-- the rate - discovery, line-of-sight and positions are all read fresh.
----------------------------------------------------------------------------

-- Drop anything a previous run left behind before we start tracking our own.
eqgfx.clear_highlights()

casts.init({ log = log, trackSelf = true, radius = settings.data.castfx.radius })

log.Info('highlighting up - /hlmenu for settings, /hl to toggle')

-- One clock for the whole script, read once per frame.
--
-- mq.gettime() is a steady_clock in milliseconds: monotonic, never steps
-- backwards, and unaffected by process CPU time - unlike os.clock(), which
-- this used to mix three separate bases of (animation phase, burst lifetimes,
-- and the cast tracker's own). Everything downstream now shares this one
-- value, so nothing can drift against anything else.
local gettime = mq.gettime or function() return os.clock() * 1000 end
local startMs = gettime()
local lastZone = nil

-- The plugin's render-callback counter, from eqgfx.stats(). It ticks once per
-- rendered frame, which is the only thing that can change what is on screen.
local lastFrame = -1

while true do
  mq.doevents()
  local cfg = settings.data

  -- Spawn IDs are recycled across zones, so the per-ID caches have to go.
  local zoneID = try(function() return mq.TLO.Zone.ID() end)
  if zoneID ~= lastZone then
    lastZone = zoneID
    classCache = {}
    bursts, prevCasts = {}, {}
  end

  -- Without an mq.delay this loop resumes on MQ2Lua's instruction-count hook,
  -- which fires many times per rendered frame. Repeating the work inside one
  -- frame cannot change a single pixel, but it does repeat every spawn scan
  -- and every LineOfSight raycast - which is what was destabilising things.
  -- Gate on the plugin's own frame counter: not a timer and not a delay, just
  -- refusing to do identical work twice against the same frame.
  local frame = eqgfx.stats()

  if not cfg.enabled or pause.active(cfg.pause) then
    if next(lit) then clear_tints() end
    -- blank(), not clear(): clear() publishes the build and starts a new one,
    -- so an empty scene would not reach the screen until the pass after this.
    eqgfx.blank()
    bursts, prevCasts = {}, {}
    menu.status.litCount, menu.status.lineCount = 0, 0
    menu.status.ambientCount, menu.status.castCount = 0, 0
    -- The one delay left in the script, and only on the path where nothing is
    -- being drawn: not consuming a pulse is the entire point of pausing.
    mq.delay(100)
  elseif frame ~= lastFrame then
    lastFrame = frame
    local nowMs = gettime()
    local now = (nowMs - startMs) / 1000        -- seconds, for animation phase
    local maTargetID = assist_target_id()
    local tankID, assistID = role_spawn_ids()

    ---- 1. model tints ----------------------------------------------------
    ---@type table<integer, integer>
    local want = {}
    if cfg.tints.enabled then
      -- The MA's target is lit whether or not it is also ours.
      if cfg.tints.assistTarget and maTargetID then
        want[maTargetID] = S.pack(cfg.colors.assistTarget)
      end
      if cfg.tints.myTarget then
        local myID = posid(try(function() return mq.TLO.Target.ID() end))
        if myID and not want[myID] then
          local role = classify(myID, maTargetID)
          if role then want[myID] = S.pack(cfg.colors[role]) end
        end
      end
    end
    apply_tints(want, cfg.tints.mode)

    ---- world layers ------------------------------------------------------
    eqgfx.clear()
    eqgfx.set_thickness(cfg.markers.thickness)
    S.begin_frame(eqgfx.add_line, cfg.budget)

    -- 2. role markers first: they are the layer you cannot afford to lose.
    if cfg.markers.enabled then
      if cfg.markers.testMode then
        -- Both markers on me, offset apart so they don't overlap - lets you
        -- tune shapes and colors without a group.
        local x = try(function() return mq.TLO.Me.X() end)
        local y = try(function() return mq.TLO.Me.Y() end)
        local z = try(function() return mq.TLO.Me.Z() end)
        if x and y and z then
          markers.tank(x - 6, y, z, cfg.tank, now)
          markers.assist(x + 6, y, z, cfg.assist, now)
        end
      else
        if cfg.markers.mainTank and tankID then
          local x, y, z = spawn_pos(tankID)
          if x then markers.tank(x, y, z, cfg.tank, now) end
        end
        if cfg.markers.mainAssist and assistID then
          local x, y, z = spawn_pos(assistID)
          if x then markers.assist(x, y, z, cfg.assist, now) end
        end
      end
    end

    -- 3. cast effects: transient and information-dense, so they outrank the
    --    ambient layer for what is left of the budget.
    local castCount = 0
    if cfg.castfx.enabled then
      -- castfx wants the effect radius under the name `radius`; building a
      -- view rather than assigning into cfg.castfx, whose `radius` is the
      -- detection range and must not be clobbered.
      local src = cfg.castfx
      local fxcfg = {
        radius = src.radius3d, height = src.height,
        colors = src.colors, byResist = src.byResist,
        releaseLife = src.releaseLife, fizzleLife = src.fizzleLife,
        maxActive = src.maxActive, segsPerCast = src.segsPerCast,
        maxReach = src.maxReach, selfCasts = src.selfCasts, charge = src.charge,
      }
      -- The tracker's own detection radius is a menu value, so push it every
      -- pass rather than only at init.
      casts.config({ radius = src.radius })
      local live = update_casts(fxcfg, nowMs)

      if fxcfg.charge then
        for id, info in pairs(live) do
          if S.over() then break end
          if (fxcfg.selfCasts or not info.isSelf) and not bursts[id] then
            local x, y, z = spawn_pos(id)
            if x then
              local class = castfx.classify(info.spellID)
              local pct = casts.progress(info, casts.now())
              castfx.draw_charge(x, y, z, class, castfx.color(class, fxcfg), fxcfg,
                pct or 0, now, fxcfg.segsPerCast)
              castCount = castCount + 1
            end
          end
        end
      end

      for _, b in pairs(bursts) do
        if S.over() then break end
        local u = (nowMs - b.bornAt) / b.life
        if b.fizzle then
          castfx.draw_fizzle(b.x, b.y, b.z, b.color, fxcfg,
            (nowMs - b.bornAt) / 1000, b.life / 1000, now, fxcfg.segsPerCast)
        else
          castfx.draw_release(b, b.color, fxcfg, min(1, u), now, fxcfg.segsPerCast)
        end
        castCount = castCount + 1
      end
    end

    -- 4. ambient: last, and it absorbs whatever budget is left.
    local ambientCount = 0
    if cfg.ambient.enabled and not S.over() then
      local acfg = cfg.ambient
      local classMode = acfg.mode == 'class'
      local styleList = enabled_styles(acfg)
      -- Class mode still needs the random styles for classless spawns, so a
      -- fully-disabled style set only stops us in random mode.
      if classMode or #styleList > 0 then
        local targets = ambient_targets(acfg)
        if #targets > 0 then
          -- Share the remaining budget evenly, never more than configured.
          local share = min(acfg.segsPerNpc, max(6, floor(S.headroom() / #targets)))
          -- shapes.lua wants the effect radius under the name `radius`.
          local drawCfg = {
            radius = acfg.radius3d, height = acfg.height, speed = acfg.speed,
            moteCount = acfg.moteCount, speedJitter = acfg.speedJitter,
            sizeJitter = acfg.sizeJitter, palette = acfg.palette,
          }
          for _, t in ipairs(targets) do
            if S.over() then break end

            -- Dim with distance so a packed zone reads as depth, not noise.
            local fade = 1
            if acfg.fadeFar and acfg.radius > 0 then
              fade = 1 - min(1, (t.dist / acfg.radius)) * 0.6
            end

            local useClass = classMode and classanim.has(t.class)
            if useClass then
              local base = (acfg.classColors and acfg.classColorTable[t.class])
                  or acfg.palette[1]
              local c = { base[1], base[2], base[3], (base[4] or 1) * fade }
              -- Phase only: two wizards should look like wizards, just not
              -- pulse in lockstep.
              classanim.draw(t.class, t.x, t.y, t.z, c, drawCfg, share,
                S.rand01(t.id, 37), now)
            elseif #styleList > 0 then
              local p = ambient.params(t.id, drawCfg, styleList)
              p.color = { p.color[1], p.color[2], p.color[3], (p.color[4] or 1) * fade }
              ambient.draw(t.x, t.y, t.z, p, drawCfg, share, now)
            end
            ambientCount = ambientCount + 1
          end
        end
      end
    end

    ---- status for the menu -----------------------------------------------
    local st = menu.status
    st.tankID, st.assistID, st.maTargetID = tankID, assistID, maTargetID
    -- CleanName is three more TLO reads a frame; only the status panel wants
    -- them, so skip it unless the menu is actually open.
    if menu.open then
      st.tankName = spawn_name(tankID)
      st.assistName = spawn_name(assistID)
      st.maTargetName = spawn_name(maTargetID)
    end
    st.lineCount = S.used()
    st.budget = cfg.budget
    st.ambientCount = ambientCount
    st.castCount = castCount
    st.castFeed = casts.feed_live()
    local n = 0
    for _ in pairs(lit) do n = n + 1 end
    st.litCount = n

    settings.maybe_save(log)

    -- No delay, deliberately. MQ2Lua force-yields the coroutine on an
    -- instruction-count hook (LuaThread's turbo value, 500 by default), so
    -- this loop already hands control back without asking; an mq.delay here
    -- would only hold the scene at a frame rate below the one the client is
    -- drawing at, which shows up as choppy animation for no gain.
  end
end
