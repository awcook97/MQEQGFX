local mq    = require('mq')
local eqgfx = require('eqgfx')
local ImGui = require('ImGui')
local log   = require('eqgfx.lib.lwlogger')

local types     = require('eqgfx.dmgnums._types')
local settings  = require('eqgfx.dmgnums.settings')
local menu      = require('eqgfx.dmgnums.menu')
local uirects   = require('eqgfx.lib.uirects')
local distscale = require('eqgfx.lib.distscale')
local pause     = require('eqgfx.lib.pause')

log.SetAppName('eqgfx')
log.SetModuleName('dmgnums')
log.SetColors(true)
log.SetIncludeCharacter('Server.Character.Zone')
log.SetIncludeTime('milliseconds')
log.SetLevel(log.INFO)
log.SetOutputFile(mq.configDir .. '/MQEQGFX/eqgfx.log')

pause.wait_for_game()

local ok, err = eqgfx.init()
if not ok then log.Error('init failed: %s', err) return end
if not eqgfx.poll_damage then
  log.Error('this MQEQGFX build has no damage tap (poll_damage missing) - rebuild/update the plugin')
  return
end

settings.load()

local function clamp01(v) return v < 0 and 0 or (v > 1 and 1 or v) end

local function ease_out_back(t)
  t = clamp01(t)
  local c1, c3 = 1.70158, 2.70158
  local u = t - 1
  return 1 + c3 * u * u * u + c1 * u * u
end

---@type DmgFloater[]
local floaters = {}

local lastBurst = {}

local dotCache = {}

local MAX_FLOATERS = 500

local animNow       = mq.gettime() / 1000

local function is_dot(spellID)
  local hit = dotCache[spellID]
  if hit ~= nil then return hit end
  local dur = 0
  pcall(function()
    dur = mq.TLO.Spell(spellID).Duration.TotalSeconds() or 0
  end)
  dotCache[spellID] = dur > 0
  return dotCache[spellID]
end

---@return string|nil source, string|nil kind, integer|nil anchorID
local function classify(ev, cfg, myID, petID)
  local source
  if ev.attackerID == myID then source = types.Source.MINE
  elseif petID > 0 and ev.attackerID == petID then source = types.Source.PET
  elseif ev.targetID == myID or (cfg.petIncoming and petID > 0 and ev.targetID == petID) then
    source = types.Source.INCOMING
  else source = types.Source.OTHERS end

  local kind
  if ev.heal then
    kind = types.Kind.HEALS
  elseif ev.isDotTick then
    kind = types.Kind.DOTS
  elseif ev.spellID and ev.spellID > 0 then
    if ev.damage <= 0 then return nil end
    kind = is_dot(ev.spellID) and types.Kind.DOTS or types.Kind.SPELLS
  elseif ev.damage <= 0 then
    kind = types.Kind.MISSES
  else
    kind = types.Kind.MELEE
  end

  local show = cfg.show[source]
  if not (show and show[kind]) then return nil end
  if kind ~= types.Kind.MISSES and kind ~= types.Kind.HEALS
      and ev.damage < (cfg.minDamage or 0) then return nil end

  local anchor = ev.targetID
  if source == types.Source.INCOMING and cfg.incomingOverSelf then anchor = myID end
  return source, kind, anchor
end

local function color_for(cfg, source, kind)
  local c = cfg.colors
  if kind == types.Kind.MISSES then return c.miss end
  if kind == types.Kind.HEALS then
    return source == types.Source.INCOMING and c.incomingHeal or c.heal
  end
  if source == types.Source.INCOMING then return c.incoming end
  if source == types.Source.PET then
    if kind == types.Kind.SPELLS then return c.petSpell end
    if kind == types.Kind.DOTS then return c.petDot end
    return c.petMelee
  end
  if source == types.Source.OTHERS then
    if kind == types.Kind.SPELLS then return c.otherSpell end
    if kind == types.Kind.DOTS then return c.otherDot end
    return c.otherMelee
  end
  if kind == types.Kind.SPELLS then return c.mySpell end
  if kind == types.Kind.DOTS then return c.myDot end
  return c.myMelee
end

local function fmt_amount(cfg, n)
  if cfg.text.abbreviate and n >= 10000 then
    local s = string.format('%.1fk', n / 1000)
    return (s:gsub('%.0k$', 'k'))
  end
  return tostring(n)
end

local function spawn_floater(cfg, source, kind, anchorID, ev, dist)
  local text, sub
  if kind == types.Kind.MISSES then
    text = cfg.missLabels[ev.damage] or types.MissCodes[ev.damage] or 'MISS'
  elseif kind == types.Kind.HEALS then
    text = (cfg.text.healSign and '+' or '') .. fmt_amount(cfg, ev.damage)
  else
    text = fmt_amount(cfg, ev.damage)
  end
  if cfg.text.showSpellName and ev.spellID and ev.spellID > 0
      and (kind == types.Kind.SPELLS or kind == types.Kind.DOTS or kind == types.Kind.HEALS) then
    pcall(function() sub = mq.TLO.Spell(ev.spellID).Name() end)
  end

  local size = cfg.text.size
  if cfg.text.scaleWithDamage and kind ~= types.Kind.MISSES and ev.damage > 0 then
    local f = clamp01(ev.damage / math.max(cfg.text.bigAt, 1))
    size = size * (1 + (cfg.text.bigScale - 1) * f)
  end
  local dscale = distscale.factor(cfg.distScale, dist)
  size = size * dscale

  local nudge = 0
  if cfg.anim.stackNudge then
    local b = lastBurst[anchorID]
    if b and (animNow - b.at) < 0.35 then
      b.n, b.at = b.n + 1, animNow
      nudge = math.min(b.n, 5) * (size * 0.8)
    else
      lastBurst[anchorID] = { at = animNow, n = 0 }
    end
  end

  local style = cfg.anim.style
  local vx, vy = 0, 0
  if style == types.AnimStyle.ARC then
    vx = (math.random() < 0.5 and -1 or 1) * cfg.anim.riseSpeed * (0.5 + math.random() * 0.5)
    vy = cfg.anim.riseSpeed
  elseif style == types.AnimStyle.FOUNTAIN then
    vx = (math.random() * 2 - 1) * cfg.anim.riseSpeed * 1.2
    vy = cfg.anim.riseSpeed * (0.8 + math.random() * 0.6)
  end

  if #floaters >= MAX_FLOATERS then table.remove(floaters, 1) end
  floaters[#floaters + 1] = {
    spawnID = anchorID,
    text = text, sub = sub,
    color = color_for(cfg, source, kind),
    size = size, dscale = dscale,
    bornAt = animNow,
    dx = (math.random() * 2 - 1) * cfg.anim.spreadX,
    dy0 = nudge,
    vx = vx, vy = vy,
  }
end

local function handle_event(ev, cfg, myID, petID, tgtID)
  local source, kind, anchorID = classify(ev, cfg, myID, petID)
  if not (source and anchorID and anchorID > 0) then return end
  if cfg.targetOnly and (source == types.Source.MINE or source == types.Source.PET)
      and anchorID ~= tgtID then return end
  local spawn = mq.TLO.Spawn(anchorID)
  local dist = (spawn.ID() == anchorID) and spawn.Distance3D() or nil
  if dist and dist <= cfg.radius then
    spawn_floater(cfg, source, kind, anchorID, ev, dist)
  end
end

local function intake(cfg)
  local events = eqgfx.poll_damage()
  if #events == 0 then return end
  local myID = mq.TLO.Me.ID() or 0
  local petID = mq.TLO.Me.Pet.ID() or 0
  local tgtID = mq.TLO.Target.ID() or 0
  for _, ev in ipairs(events) do
    handle_event(ev, cfg, myID, petID, tgtID)
  end
end

local function to_dmg(s) return tonumber((tostring(s):gsub('[^%d]', ''))) end

local function spawn_id(name, myID, myName)
  if not name or name == '' then return 0 end
  if name == 'You' or name == 'you' or name == myName then return myID end
  local id = mq.TLO.Spawn('=' .. name).ID()
  return id or 0
end

local function feed_dot(cfg, casterID, targetID, dmg)
  if not cfg.enabled or not dmg or dmg <= 0 or targetID == 0 then return end
  if pause.active(cfg.pause) then return end
  local myID = mq.TLO.Me.ID() or 0
  local petID = mq.TLO.Me.Pet.ID() or 0
  local tgtID = mq.TLO.Target.ID() or 0
  handle_event({ attackerID = casterID, targetID = targetID, damage = dmg,
                 spellID = 0, skill = 0, heal = false, isDotTick = true },
               cfg, myID, petID, tgtID)
end

local OVERLAY_FLAGS = bit.bor(
  ImGuiWindowFlags.NoTitleBar, ImGuiWindowFlags.NoResize, ImGuiWindowFlags.NoMove,
  ImGuiWindowFlags.NoScrollbar, ImGuiWindowFlags.NoScrollWithMouse,
  ImGuiWindowFlags.NoCollapse, ImGuiWindowFlags.NoBackground,
  ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoInputs,
  ImGuiWindowFlags.NoFocusOnAppearing, ImGuiWindowFlags.NoBringToFrontOnFocus,
  ImGuiWindowFlags.NoNav)

local function u32(c, alphaMult)
  return ImGui.ColorConvertFloat4ToU32({ c[1], c[2], c[3], c[4] * (alphaMult or 1) })
end

local sizedText = nil
local function probe_sized(drawList)
  if sizedText ~= nil then return end
  sizedText = pcall(function()
    drawList:AddText(ImGui.GetFont(), 13, ImVec2(-4096, -4096), 0x01FFFFFF, 'x')
  end)
end

local function add_text(drawList, tx, y, size, col, str)
  if sizedText then
    drawList:AddText(ImGui.GetFont(), size, ImVec2(tx, y), col, str)
  else
    drawList:AddText(ImVec2(tx, y), col, str)
  end
end

local function draw_text(drawList, cfg, x, y, size, c, alpha, str)
  local w = ImGui.CalcTextSize(str)
  if sizedText then w = w * (size / (ImGui.GetFontSize() or 13)) end
  local tx = x - w / 2
  local sc = cfg.colors.shadow
  if cfg.text.outline then
    local oc = u32(sc, alpha)
    add_text(drawList, tx - 1, y, size, oc, str)
    add_text(drawList, tx + 1, y, size, oc, str)
    add_text(drawList, tx, y - 1, size, oc, str)
    add_text(drawList, tx, y + 1, size, oc, str)
  elseif cfg.text.shadow then
    add_text(drawList, tx + 1, y + 1, size, u32(sc, alpha), str)
  end
  add_text(drawList, tx, y, size, u32(c, alpha), str)
end

local uiRects = {}

local function under_ui(x, y)
  for _, rect in ipairs(uiRects) do
    if x >= rect[1] and x <= rect[3] and y >= rect[2] and y <= rect[4] then return true end
  end
  return false
end

local function draw_floaters(drawList, cfg)
  if #floaters == 0 then return end
  if cfg.hideUnderUI then uiRects = uirects.get() end

  local anchored, anchorX, anchorY = {}, {}, {}
  local alive = 0
  for i = 1, #floaters do
    local f = floaters[i]
    local age = animNow - f.bornAt
    if age <= cfg.anim.duration then
      alive = alive + 1
      floaters[alive] = f

      local sid = f.spawnID
      if not anchored[sid] then
        anchored[sid] = true
        local spawn = mq.TLO.Spawn(sid)
        local px = spawn.ID() == sid and spawn.X()
        if px then
          local sx, sy, vis = eqgfx.world_to_screen(px, spawn.Y(), (spawn.Z() or 0) + cfg.zOffset)
          if vis then anchorX[sid], anchorY[sid] = sx, sy end
        end
      end
      if anchorX[sid] then f.sx, f.sy = anchorX[sid], anchorY[sid] end

      if f.sx then
        local ox, oy = f.dx, -(f.dy0 or 0)
        local style = cfg.anim.style
        if style == types.AnimStyle.FLOAT then
          oy = oy - cfg.anim.riseSpeed * age
        elseif style == types.AnimStyle.ARC or style == types.AnimStyle.FOUNTAIN then
          ox = ox + f.vx * age
          oy = oy - (f.vy * age - 0.5 * cfg.anim.gravity * age * age)
        end

        local alpha = 1.0
        local fadeFrom = cfg.anim.fadeStart * cfg.anim.duration
        if age > fadeFrom then
          alpha = 1 - (age - fadeFrom) / math.max(cfg.anim.duration - fadeFrom, 0.01)
        end

        local size = f.size
        if cfg.anim.popIn and age < cfg.anim.popDur then
          size = size * ease_out_back(age / cfg.anim.popDur)
        end

        local x, y = f.sx + ox, f.sy + oy
        if not (cfg.hideUnderUI and under_ui(x, y)) then
          draw_text(drawList, cfg, x, y, size, f.color, alpha, f.text)
          if f.sub then
            draw_text(drawList, cfg, x, y + size + 1, cfg.text.spellNameSize * (f.dscale or 1),
              cfg.colors.spellName, alpha, f.sub)
          end
        end
      end
    end
  end
  for i = #floaters, alive + 1, -1 do floaters[i] = nil end
end

local drawErrLogged = false

local function draw()
  animNow = mq.gettime() / 1000
  local cfg = settings.data or {}

  if cfg.enabled and not pause.active(cfg.pause) then
    local sw, sh = eqgfx.get_screen()
    if not sw or sw <= 0 then sw, sh = 4096, 2160 end
    ImGui.SetNextWindowPos(0, 0)
    ImGui.SetNextWindowSize(sw, sh)
    local _, show = ImGui.Begin('##eqgfx_dmgnums_overlay', true, OVERLAY_FLAGS)
    if show then
      local okk, drawErr = pcall(function()
        local drawList = ImGui.GetWindowDrawList()
        probe_sized(drawList)
        intake(cfg)
        draw_floaters(drawList, cfg)
      end)
      if not okk and not drawErrLogged then
        drawErrLogged = true
        log.Error('draw error (numbers disabled this frame): %s', tostring(drawErr))
      end
    end
    ImGui.End()
  else
    if #floaters > 0 then floaters = {} end
    eqgfx.poll_damage()
  end

  menu.draw()
end

mq.imgui.init('eqgfx_dmgnums', draw)

mq.event('dn_dot_mine', '#1# has taken #2# damage from your #3#.#*#', function(_, target, dmgStr)
  local cfg = settings.data
  local myID, myName = mq.TLO.Me.ID() or 0, mq.TLO.Me.CleanName() or ''
  feed_dot(cfg, myID, spawn_id(target, myID, myName), to_dmg(dmgStr))
end)

mq.event('dn_dot_other', '#1# has taken #2# damage from #3# by #4#.#*#', function(_, target, dmgStr, caster)
  local cfg = settings.data
  local myID, myName = mq.TLO.Me.ID() or 0, mq.TLO.Me.CleanName() or ''
  feed_dot(cfg, spawn_id(caster, myID, myName), spawn_id(target, myID, myName), to_dmg(dmgStr))
end)

mq.bind('/dnmenu', function() menu.toggle() end)

mq.bind('/dmgnums', function()
  local cfg = settings.data
  cfg.enabled = not cfg.enabled
  settings.mark_dirty()
  log.Info('damage numbers %s', cfg.enabled and 'ON' or 'OFF')
end)

mq.bind('/dntest', function()
  local cfg = settings.data
  local anchorID = mq.TLO.Target.ID() or 0
  if anchorID == 0 then anchorID = mq.TLO.Me.ID() or 0 end
  if anchorID == 0 then return end
  local samples = {
    { source = types.Source.MINE,     kind = types.Kind.MELEE,  ev = { damage = 187 } },
    { source = types.Source.MINE,     kind = types.Kind.SPELLS, ev = { damage = 1204, spellID = 0 } },
    { source = types.Source.MINE,     kind = types.Kind.DOTS,   ev = { damage = 316, spellID = 0 } },
    { source = types.Source.MINE,     kind = types.Kind.MISSES, ev = { damage = -4 } },
    { source = types.Source.INCOMING, kind = types.Kind.MELEE,  ev = { damage = 92 } },
    { source = types.Source.MINE,     kind = types.Kind.HEALS,  ev = { damage = 512 } },
  }
  for n, s in ipairs(samples) do
    local born = animNow + (n - 1) * 0.12
    local saved = animNow
    animNow = born
    spawn_floater(cfg, s.source, s.kind, anchorID, s.ev)
    animNow = saved
  end
  log.Info('spawned %d test numbers', #samples)
end)

log.Info('damage numbers up - /dnmenu to customize, /dntest to preview')

while true do
  mq.doevents()
  settings.maybe_save()
  mq.delay(100)
end
