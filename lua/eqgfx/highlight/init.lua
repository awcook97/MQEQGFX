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
log.SetOutputFile(mq.configDir .. '/MQEQGFX/eqgfx.log')

pause.wait_for_game()

local ok, err = eqgfx.init()
if not ok then log.Error('init failed: %s', err) return end

settings.load()

S.set_packer(eqgfx.argb)
castfx.set_geom_source(eqgfx.spell_geom)

local floor, max, min, sqrt = math.floor, math.max, math.min, math.sqrt

---@generic T
---@param f fun(): T
---@return T|nil
local function try(f)
  local okCall, v = pcall(f)
  if okCall then return v end
end

local function posid(v)
  if v and v > 0 then return v end
end

---@return integer|nil spawnID
local function assist_target_id()
  local raidSize = try(function() return mq.TLO.Raid.Members() end) or 0
  if raidSize > 0 then
    local id = posid(try(function() return mq.TLO.Me.RaidAssistTarget(1).ID() end))
    if id then return id end
  end
  return posid(try(function() return mq.TLO.Me.GroupAssistTarget.ID() end))
end

---@return integer|nil tankID, integer|nil assistID
local function role_spawn_ids()
  local tank = posid(try(function() return mq.TLO.Group.MainTank.Spawn.ID() end))
  local assist = posid(try(function() return mq.TLO.Raid.MainAssist(1).Spawn.ID() end))
      or posid(try(function() return mq.TLO.Group.MainAssist.Spawn.ID() end))
  return tank, assist
end

---@param id integer
---@param maTargetID integer|nil
---@return HighlightRole|nil
local function classify(id, maTargetID)
  if maTargetID and id == maTargetID then return 'assistTarget' end

  local spawn = mq.TLO.Spawn('id ' .. id)
  local kind = try(function() return spawn.Type() end)
  if not kind then return end

  if kind == 'Pet' then return 'pet' end
  if types.PlayerTypes[kind] then return 'pc' end
  if kind ~= 'NPC' then return end

  return try(function() return spawn.Aggressive() end) and 'hostile' or 'neutral'
end

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

---@type table<integer, integer>  spawn ID -> ARGB last requested
local lit = {}

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

---@type table<integer, string|false>   false = looked up, has no class
local classCache = {}

---@param id integer
---@param spawn spawn|nil # pass one if you already have it
---@return string|nil # Spawn.Class.ShortName(), e.g. 'WIZ'
local function class_of(id, spawn)
  local hit = classCache[id]
  if hit ~= nil then return hit or nil end
  local source = spawn or mq.TLO.Spawn('id ' .. id)
  local short = try(function() return source.Class.ShortName() end)
  if type(short) ~= 'string' or short == '' then short = nil end
  classCache[id] = short or false
  return short
end

---@class AmbientTarget
---@field id integer
---@field x number
---@field y number
---@field z number
---@field dist number
---@field class string|nil # Spawn.Class.ShortName(), nil when unreadable

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
  for _, sid in ipairs(eqgfx.spawn_search(spec)) do
    local spawn = mq.TLO.Spawn(sid) --[[@as spawn]]
    local id = posid(try(function() return spawn.ID() end))
    if id and id ~= myID then
      local kind = try(function() return spawn.Type() end)
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
            if #out >= cfg.maxSpawns then break end
          end
        end
      end
    end
  end
  return out
end

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

---@class CastFx
---@field shape CastShape
---@field color RGBA
---@field x number   # caster position, frozen at release
---@field y number
---@field z number
---@field tx number  # victim position
---@field ty number
---@field tz number
---@field heading number # caster facing, radians
---@field reach number
---@field coneStart number
---@field coneEnd number
---@field bornAt number
---@field life number
---@field fizzle boolean

---@type table<integer, CastFx>   spawnID -> effect in its release/fizzle phase
local bursts = {}
---@type table<integer, table>    spawnID -> last frame's CastInfo
local prevCasts = {}

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

---@param info table       # CastInfo
---@param class CastClass
---@param cfg table        # settings.data.castfx
---@param nowMs number      # mq.gettime() milliseconds
---@param fizzle boolean
---@return CastFx|nil
local function make_burst(info, class, cfg, nowMs, fizzle)
  local x, y, z = spawn_pos(info.spawnID)
  if not x then return end

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

---@param cfg table # settings.data.castfx
---@param nowMs number # mq.gettime() milliseconds
---@return table<integer, table> live
local function update_casts(cfg, nowMs)
  casts.update()
  local live = casts.all()

  for id, info in pairs(prevCasts) do
    if not live[id] then
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
  elseif which == '' then
    cfg.enabled = not cfg.enabled
    if not cfg.enabled then
      clear_tints()
      eqgfx.blank()
    end
    log.Info('highlighting %s', cfg.enabled and 'ON' or 'OFF')
  else
    log.Info('usage: /hl [markers|tints|ambient|casts|test]  (/hl alone toggles everything)')
    return
  end
  settings.mark_dirty()
end)

mq.imgui.init('eqgfx_highlight', function() menu.draw() end)

eqgfx.clear_highlights()

casts.init({ log = log, trackSelf = true, radius = settings.data.castfx.radius })

log.Info('highlighting up - /hlmenu for settings, /hl to toggle')

local gettime = mq.gettime
local startMs = gettime()
local lastZone = nil

local lastFrame = -1

while true do
  mq.doevents()
  settings.maybe_save()
  local cfg = settings.data

  local zoneID = try(function() return mq.TLO.Zone.ID() end)
  if zoneID ~= lastZone then
    lastZone = zoneID
    classCache = {}
    bursts, prevCasts = {}, {}
  end

  local frame = eqgfx.stats()

  if not cfg.enabled or pause.active(cfg.pause) then
    if next(lit) then clear_tints() end
    eqgfx.blank()
    bursts, prevCasts = {}, {}
    menu.status.litCount, menu.status.lineCount = 0, 0
    menu.status.ambientCount, menu.status.castCount = 0, 0
    mq.delay(100)
  elseif frame ~= lastFrame then
    lastFrame = frame
    local nowMs = gettime()
    local now = (nowMs - startMs) / 1000
    local maTargetID = assist_target_id()
    local tankID, assistID = role_spawn_ids()

    ---@type table<integer, integer>
    local want = {}
    if cfg.tints.enabled then
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

    eqgfx.clear()
    eqgfx.set_thickness(cfg.markers.thickness)
    S.begin_frame(eqgfx.add_line, cfg.budget)

    if cfg.markers.enabled then
      if cfg.markers.testMode then
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

    local castCount = 0
    local nativeOn = eqgfx.spellfx_enabled and eqgfx.spellfx_enabled() or false
    if cfg.castfx.enabled and (not nativeOn or cfg.castfx.alongsideNative) then
      local src = cfg.castfx
      local fxcfg = {
        radius = src.radius3d, height = src.height,
        colors = src.colors, byResist = src.byResist,
        releaseLife = src.releaseLife, fizzleLife = src.fizzleLife,
        maxActive = src.maxActive, segsPerCast = src.segsPerCast,
        maxReach = src.maxReach, selfCasts = src.selfCasts, charge = src.charge,
      }
      casts.config({ radius = src.radius })
      local live = update_casts(fxcfg, nowMs)

      if fxcfg.charge then
        for id, info in pairs(live) do
          if S.over() then break end
          if (fxcfg.selfCasts or not info.isSelf) and not bursts[id] then
            local x, y, z = spawn_pos(id)
            if x and y and z then
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

    local ambientCount = 0
    if cfg.ambient.enabled and not S.over() then
      local acfg = cfg.ambient
      local classMode = acfg.mode == 'class'
      local styleList = enabled_styles(acfg)
      if classMode or #styleList > 0 then
        local targets = ambient_targets(acfg)
        if #targets > 0 then
          local share = min(acfg.segsPerNpc, max(6, floor(S.headroom() / #targets)))
          local drawCfg = {
            radius = acfg.radius3d, height = acfg.height, speed = acfg.speed,
            moteCount = acfg.moteCount, speedJitter = acfg.speedJitter,
            sizeJitter = acfg.sizeJitter, palette = acfg.palette,
          }
          for _, t in ipairs(targets) do
            if S.over() then break end

            local fade = 1
            if acfg.fadeFar and acfg.radius > 0 then
              fade = 1 - min(1, (t.dist / acfg.radius)) * 0.6
            end

            local useClass = classMode and classanim.has(t.class)
            if useClass then
              local base = (acfg.classColors and acfg.classColorTable[t.class])
                  or acfg.palette[1]
              local c = { base[1], base[2], base[3], (base[4] or 1) * fade }
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

    local st = menu.status
    st.tankID, st.assistID, st.maTargetID = tankID, assistID, maTargetID
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
  end
end
