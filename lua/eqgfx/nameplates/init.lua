local mq    = require('mq')
local eqgfx = require('eqgfx')
local ImGui = require('ImGui')
local log   = require('eqgfx.lib.lwlogger')

local types     = require('eqgfx.nameplates._types')
local anim      = require('eqgfx.nameplates.anim')
local settings  = require('eqgfx.nameplates.settings')
local render    = require('eqgfx.nameplates.render')
local menu      = require('eqgfx.nameplates.menu')
local ae        = require('eqgfx.nameplates.ae')
local casts     = require('eqgfx.casts')
local uirects   = require('eqgfx.lib.uirects')
local distscale = require('eqgfx.lib.distscale')
local pause     = require('eqgfx.lib.pause')

log.SetAppName('eqgfx')
log.SetModuleName('nameplates')
log.SetColors(true)
log.SetIncludeCharacter('Server.Character.Zone')
log.SetIncludeTime('milliseconds')
log.SetLevel(log.INFO)
log.SetOutputFile(mq.configDir .. '/MQEQGFX/eqgfx.log')

pause.wait_for_game()

local ok, err = eqgfx.init()
if not ok then log.Error('init failed: %s', err) return end

settings.load()
local bootCfg = settings.data or {}
uirects.add_names(bootCfg.extraWindows)
casts.init{ log = log, trackSelf = true, radius = bootCfg.radius,
            interruptDetect = bootCfg.castbar.interruptDetect }

---@type table<integer, Plate>
local plates = {}

---@type table<integer, { at: number, ok: boolean }>
local losCache = {}

local uiRects = {}
local showUiRects = false

---@param names string[]|nil # out-param: human-readable rect descriptions
local function scan_ui_rects(names)
  uiRects = uirects.get()
  if names then
    for idx, rect in ipairs(uiRects) do
      names[idx] = string.format('%s %d,%d %dx%d', rect.name or 'rect',
                                 rect[1], rect[2],
                                 rect[3] - rect[1], rect[4] - rect[2])
    end
  end
end

local PAD = 12

---@param cfg NameplatesConfig # nameplates settings (hideUnderUI, uiOccludeMode)
---@param plate Plate # needs sx/sy; uses plate.ext when present
---@param hasCast boolean # extend the bbox downward for a cast bar that ext may not cover yet
---@return boolean hide # true = skip the plate entirely (HIDE mode / no clipping support)
---@return number[][]|nil pieces # visible pieces of the padded plate bbox (CLIP mode)
local function occlusion(cfg, plate, hasCast)
  if not cfg.hideUnderUI or #uiRects == 0 then return false end
  local bx0, by0, bx1, by1
  local ext = plate.ext
  local ax, ay = plate.sx + (plate.ox or 0), plate.sy + (plate.oy or 0)
  if ext then
    bx0, by0 = ax + ext[1] - PAD, ay + ext[2] - PAD
    bx1, by1 = ax + ext[3] + PAD, ay + ext[4] + PAD
  else
    local hw = cfg.bar.width * 0.5 + 120
    bx0, by0, bx1, by1 = ax - hw, ay - 90, ax + hw, ay + 110
  end
  if hasCast then
    by1 = by1 + cfg.castbar.gap + cfg.castbar.height + cfg.castbar.textSize + 2
  end
  local pieces = uirects.visible(bx0, by0, bx1, by1, 16)
  if not pieces then return false end
  if cfg.uiOccludeMode == types.UiOccludeMode.HIDE or not render.caps.clipRect then
    return true
  end
  return false, pieces
end

local OVERLAY_FLAGS = bit.bor(
  ImGuiWindowFlags.NoTitleBar, ImGuiWindowFlags.NoResize, ImGuiWindowFlags.NoMove,
  ImGuiWindowFlags.NoScrollbar, ImGuiWindowFlags.NoScrollWithMouse,
  ImGuiWindowFlags.NoCollapse, ImGuiWindowFlags.NoBackground,
  ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoInputs,
  ImGuiWindowFlags.NoFocusOnAppearing, ImGuiWindowFlags.NoBringToFrontOnFocus,
  ImGuiWindowFlags.NoNav)

local drawErrSeen, drawErrCount = {}, 0

local function note_draw_error(problem)
  local msg = tostring(problem)
  if drawErrSeen[msg] or drawErrCount >= 20 then return end
  drawErrSeen[msg] = true
  drawErrCount = drawErrCount + 1
  log.Error('draw error: %s', msg)
end

local MAX_DT = 0.1
local animNow     = casts.now()
local lastFrameAt = animNow

local RS = types.ResScope

---@param scope ResScope
---@param plate Plate
---@return boolean|nil yes # truthy when the bar applies
local function want_res(scope, plate)
  if scope == RS.OFF then return false end
  if scope == RS.SELF then return plate.isSelf end
  if scope == RS.GROUP then return plate.isSelf or plate.inGroup end
  if scope == RS.PCS then return plate.isPC end
  return true
end

local function pct_mana(spawn) return spawn.PctMana() end
local function pct_endurance(spawn) return spawn.PctEndurance() end

---@param plate Plate
---@param spawn spawn # live spawn TLO (already validated by the caller)
---@param cfg NameplatesConfig # nameplates settings
---@param timeNow number # frame timebase
---@param dt number # frame delta, seconds
local function update_live(plate, spawn, cfg, timeNow, dt)
  local hp = (spawn.PctHPs() or 0) / 100

  if cfg.anim.damageFlash and hp < plate.lastHp - cfg.anim.flashThreshold then
    plate.flashAt = timeNow
  end
  plate.lastHp = hp

  if cfg.anim.hpSmoothing then
    plate.dispHp = anim.approach(plate.dispHp, hp, cfg.anim.hpSpeed, dt)
  else
    plate.dispHp = hp
  end

  plate.name  = spawn.CleanName() or plate.name
  plate.alpha = cfg.anim.fadeIn
            and anim.clamp01((timeNow - plate.bornAt) / math.max(cfg.anim.fadeInDur, 0.01))
            or 1
  plate.scale = cfg.anim.appearPop
            and anim.ease_out_back((timeNow - plate.bornAt) / math.max(cfg.anim.popDur, 0.01))
            or 1

  plate.mana, plate.endu = nil, nil
  if want_res(cfg.resources.manaScope, plate) then
    local v
    if plate.isSelf then v = mq.TLO.Me.PctMana()
    else local okk, sv = pcall(pct_mana, spawn); v = okk and sv or nil end
    if type(v) == 'number' and (plate.isSelf or v > 0) then plate.mana = v / 100 end
  end
  if want_res(cfg.resources.enduScope, plate) then
    local v
    if plate.isSelf then v = mq.TLO.Me.PctEndurance()
    else local okk, sv = pcall(pct_endurance, spawn); v = okk and sv or nil end
    if type(v) == 'number' and (plate.isSelf or v > 0) then plate.endu = v / 100 end
  end
end

local OM = types.PlateOverlapMode

---@class PlateSlot  one plate's natural screen rect while decluttering
---@field job PlateJob
---@field x0 number
---@field y0 number
---@field x1 number
---@field y1 number
---@field rank integer # importance: 3 = me, 2 = my target, 0 = everyone else
---@field dist number # squared 3D distance to me

---@param job PlateJob
---@return integer rank
local function plate_rank(job)
  if job.plate.isSelf then return 3 end
  if job.isTarget then return 2 end
  return 0
end

---@param cfg NameplatesConfig # nameplates settings (fallback bar size)
---@param plate Plate # needs sx/sy
---@return number x0, number y0, number x1, number y1
local function plate_rect(cfg, plate)
  local sx, sy = plate.sx or 0, plate.sy or 0
  local ext = plate.ext
  if ext then return sx + ext[1], sy + ext[2], sx + ext[3], sy + ext[4] end
  local hw, hh = cfg.bar.width * 0.5, cfg.bar.height * 0.5
  return sx - hw, sy - hh - cfg.name.size, sx + hw, sy + hh
end

---@param v number # pixels
---@param limit number # cfg.overlap.maxPush
---@return number clamped
local function clamp_push(v, limit)
  if v > limit then return limit end
  if v < -limit then return -limit end
  return v
end

---@param ov NpOverlapCfg # cfg.overlap
---@param slots PlateSlot[] # this frame's rects, mutated in place
local function separate_plates(ov, slots)
  local placed, movable = {}, {}
  for _, slot in ipairs(slots) do
    if slot.rank > 0 then placed[#placed + 1] = slot else movable[#movable + 1] = slot end
  end
  table.sort(movable, function(a, b) return a.y0 < b.y0 end)
  for _, slot in ipairs(movable) do
    local push, again, guard = 0, true, 0
    while again and guard < 4 do
      again, guard = false, guard + 1
      for _, other in ipairs(placed) do
        if slot.x0 < other.x1 + ov.xPad and slot.x1 > other.x0 - ov.xPad
            and slot.y0 + push < other.y1 + ov.gap and slot.y1 + push > other.y0 then
          push = other.y1 + ov.gap - slot.y0
          again = true
        end
      end
    end
    push = clamp_push(push, ov.maxPush)
    slot.y0, slot.y1 = slot.y0 + push, slot.y1 + push
    slot.job.plate.oy = push
    placed[#placed + 1] = slot
  end
end

---@param ov NpOverlapCfg # cfg.overlap
---@param slots PlateSlot[] # this frame's rects, re-sorted in place
local function stack_plates(ov, slots)
  table.sort(slots, function(a, b)
    if a.rank ~= b.rank then return a.rank > b.rank end
    return a.dist < b.dist
  end)
  local heads = {}
  local reach = ov.clusterRadius * ov.clusterRadius
  for _, slot in ipairs(slots) do
    local plate = slot.job.plate
    local sx, sy = plate.sx or 0, plate.sy or 0
    local head
    for _, h in ipairs(heads) do
      local dx, dy = sx - h.sx, sy - h.sy
      if dx * dx + dy * dy <= reach then head = h break end
    end
    if head then
      plate.ox = head.sx - sx
      plate.oy = clamp_push(head.bottom + ov.gap - slot.y0, ov.maxPush)
      head.bottom = head.bottom + ov.gap + (slot.y1 - slot.y0)
    else
      heads[#heads + 1] = { sx = sx, sy = sy, bottom = slot.y1 }
    end
  end
end

---@param cfg NameplatesConfig # nameplates settings (overlap.*)
---@param jobs PlateJob[] # this frame's plates
local function declutter(cfg, jobs)
  local ov = cfg.overlap
  ---@type PlateSlot[]
  local slots = {}
  for i, job in ipairs(jobs) do
    job.plate.ox, job.plate.oy = 0, 0
    if ov.mode ~= OM.OFF then
      local x0, y0, x1, y1 = plate_rect(cfg, job.plate)
      slots[i] = { job = job, x0 = x0, y0 = y0, x1 = x1, y1 = y1,
                   rank = plate_rank(job), dist = job.plate.dist or 0 }
    end
  end
  if ov.mode == OM.OFF or #slots < 2 then return end
  if ov.mode == OM.STACK then stack_plates(ov, slots) else separate_plates(ov, slots) end
end

---@param cfg NameplatesConfig # nameplates settings (clickTarget)
---@param jobs PlateJob[] # this frame's drawn plates, in draw order
local function click_target(cfg, jobs)
  if not cfg.clickTarget then return end
  local okClick, clicked = pcall(ImGui.IsMouseClicked, 0)
  if not (okClick and clicked) then return end
  local okHover, overWindow = pcall(function()
    return ImGui.IsWindowHovered(ImGuiHoveredFlags.AnyWindow)
  end)
  if okHover and overWindow then return end
  local mx, my = render.mouse_pos()
  if not (mx and my) then return end
  for _, rect in ipairs(uiRects) do
    if mx >= rect[1] and mx <= rect[3] and my >= rect[2] and my <= rect[4] then return end
  end
  for i = #jobs, 1, -1 do
    local plate = jobs[i].plate
    local hit = plate.hit
    if hit and not plate.lostAt
        and mx >= hit[1] and mx <= hit[3] and my >= hit[2] and my <= hit[4] then
      mq.TLO.Spawn(plate.id).DoTarget()
      return
    end
  end
end

---@param drawList ImDrawList # the fullscreen overlay window's draw list
---@param cfg NameplatesConfig # nameplates settings
---@param timeNow number # frame timebase
---@param dt number # frame delta, seconds
local function draw_plates(drawList, cfg, timeNow, dt)
  if cfg.hideUnderUI or cfg.clickTarget or showUiRects then scan_ui_rects() end
  local tgtID = mq.TLO.Target.ID() or 0
  local me = mq.TLO.Me
  local mex, mey, mez = me.X() or 0, me.Y() or 0, me.Z() or 0
  local aeStates = cfg.aehl.enabled and ae.states(cfg, mex, mey, mez) or nil
  local jobs = {}

  local function update_plate(id, plate)
    local isTarget = (id == tgtID)
    if not plate.lostAt then
      local spawn = mq.TLO.Spawn(id) --[[@as spawn]]
      local x = spawn.ID() == id and spawn.X()
      if x then
        update_live(plate, spawn, cfg, timeNow, dt)
        local y, z = spawn.Y(), spawn.Z()
        local dx, dy, dz = x - mex, y - mey, z - mez
        plate.dist = dx * dx + dy * dy + dz * dz
        local sx, sy, vis = eqgfx.world_to_screen(x, y, z + cfg.bar.zOffset)
        if vis then
          plate.sx, plate.sy = sx, sy
          plate.dscale = distscale.factor(cfg.distScale, math.sqrt(plate.dist))
          local detN, benN = 0, 0
          if aeStates then
            for si = 1, #aeStates do
              local k = ae.affects(aeStates[si], plate, x, y, z)
              if k == 'det' then detN = detN + 1
              elseif k == 'ben' then benN = benN + 1 end
            end
          end
          local kind = detN > 0 and 'det' or (benN > 0 and 'ben' or nil)
          local target = 0
          if kind then
            local hl = cfg.aehl
            local n = math.min(detN > 0 and detN or benN, hl.stackMax)
            target = math.min(hl.stackBase + hl.stackStep * (n - 1), 1)
            plate.aeKind = kind
          end
          plate.aeAmt = anim.approach(plate.aeAmt or 0, target, cfg.aehl.fadeSpeed, dt)
          if not kind and plate.aeAmt < 0.01 then plate.aeKind = nil end
          local castInfo = nil
          if cfg.castbar.show and (not cfg.castbar.onlyTarget or isTarget) then
            castInfo = casts.get(id)
          end
          jobs[#jobs + 1] = { plate = plate, castInfo = castInfo, isTarget = isTarget }
        end
      else
        plate.lostAt, plate.lossAlpha = timeNow, plate.alpha
      end
    else
      local f = cfg.anim.fadeOut and anim.fade(plate.lostAt, cfg.anim.fadeOutDur, timeNow) or 0
      if f <= 0 then
        plates[id] = nil
      elseif plate.sx then
        plate.aeAmt = anim.approach(plate.aeAmt or 0, 0, cfg.aehl.fadeSpeed, dt)
        plate.alpha = (plate.lossAlpha or 1) * f
        jobs[#jobs + 1] = { plate = plate, castInfo = nil, isTarget = isTarget }
      end
    end
  end

  for id, plate in pairs(plates) do
    local okp, perr = pcall(update_plate, id, plate)
    if not okp then note_draw_error(perr) end
  end

  declutter(cfg, jobs)

  ---@type PlateJob[]
  local drawn = {}
  for _, job in ipairs(jobs) do
    local hide, pieces = occlusion(cfg, job.plate, job.castInfo ~= nil)
    if not hide then
      job.pieces = pieces
      drawn[#drawn + 1] = job
    end
  end

  table.sort(drawn, function(a2, b2)
    local ra, rb = plate_rank(a2), plate_rank(b2)
    if ra ~= rb then return ra < rb end
    return (a2.plate.dist or 0) > (b2.plate.dist or 0)
  end)
  for _, job in ipairs(drawn) do
    if job.pieces then
      for _, sub in ipairs(job.pieces) do
        drawList:PushClipRect(ImVec2(sub[1], sub[2]), ImVec2(sub[3], sub[4]), false)
        local okp, perr = pcall(render.plate, drawList, cfg, job.plate, job.castInfo, timeNow, job.isTarget)
        drawList:PopClipRect()
        if not okp then note_draw_error(perr) end
      end
    else
      local okp, perr = pcall(render.plate, drawList, cfg, job.plate, job.castInfo, timeNow, job.isTarget)
      if not okp then note_draw_error(perr) end
    end
  end

  click_target(cfg, drawn)
end

local function draw()
  animNow = casts.now()
  local timeNow, dt = animNow, math.min(animNow - lastFrameAt, MAX_DT)
  lastFrameAt = animNow
  local cfg = settings.data or {}

  if cfg.enabled and not pause.active(cfg.pause) then
    local sw, sh = eqgfx.get_screen()
    if not sw or sw <= 0 then sw, sh = 4096, 2160 end
    ImGui.SetNextWindowPos(0, 0)
    ImGui.SetNextWindowSize(sw, sh)
    local _, show = ImGui.Begin('##eqgfx_nameplates_overlay', true, OVERLAY_FLAGS)
    if show then
      local okk, drawErr = pcall(function()
        local drawList = ImGui.GetWindowDrawList()
        render.probe_caps(drawList)
        draw_plates(drawList, cfg, timeNow, dt)
        if showUiRects then
          local fg = ImGui.GetForegroundDrawList()
          for n, rect in ipairs(uiRects) do
            fg:AddRect(ImVec2(rect[1], rect[2]), ImVec2(rect[3], rect[4]), 0xffff00ff, 0, 0, 2.0)
            fg:AddText(ImVec2(rect[1] + 3, rect[2] + 3), 0xffff00ff,
                       '#' .. n .. (rect.name and (' ' .. rect.name) or ''))
          end
        end
      end)
      if not okk then note_draw_error(drawErr) end
    end
    ImGui.End()
  end

  menu.draw(render.caps)
end

mq.imgui.init('eqgfx_nameplates', draw)

mq.bind('/npmenu', function() menu.toggle() end)
mq.bind('/npradius', function(n)
  local cfg = settings.data or {}
  local v = tonumber(n)
  if v and v == v then
    v = math.min(math.max(v, 20), 500)
    if v ~= cfg.radius then
      cfg.radius = v
      settings.mark_dirty()
    end
  end
  log.Info('radius=%d', cfg.radius)
end)
mq.bind('/nppcs', function()
  local cfg = settings.data or {}
  cfg.show.pcs = not cfg.show.pcs
  settings.mark_dirty()
  log.Info('show PCs=%s', tostring(cfg.show.pcs))
end)

mq.bind('/npdebug', function()
  local c = render.caps
  log.Info('caps: sizedText=%s multiColor=%s clipRect=%s iconDraw=%s atlasFound=%s',
           tostring(c.sizedText), tostring(c.multiColor), tostring(c.clipRect),
           tostring(c.iconDraw), tostring(c.atlasFound))
  local tid = mq.TLO.Target.ID()
  if tid and tid > 0 then
    local spawn = mq.TLO.Spawn(tid)
    local bc  = select(2, pcall(function() return spawn.BuffCount() end))
    local cbc = select(2, pcall(function() return spawn.CachedBuffCount() end))
    local plate = plates[tid]
    log.Info('target %s: BuffCount=%s CachedBuffCount=%s plate=%s scanned benefits=%d detriments=%d',
             tostring(spawn.CleanName()), tostring(bc), tostring(cbc), tostring(plate ~= nil),
             plate and plate.buffsB and #plate.buffsB or -1, plate and plate.buffsD and #plate.buffsD or -1)
  else
    log.Info('no target - target something buffed and rerun /npdebug')
  end
  local cfg = settings.data or {}
  local paused, why = pause.active(cfg.pause)
  log.Info('idle gate: paused=%s%s (foreground=%s rendering=%s)',
           tostring(paused), why and (' [' .. why .. ']') or '',
           tostring(pause.foreground()), tostring(pause.rendering()))
  local losCached = 0
  for _ in pairs(losCache) do losCached = losCached + 1 end
  log.Info('line of sight: enabled=%s interval=%.2fs cached=%d spawns',
           tostring(cfg.los.enabled), cfg.los.interval, losCached)
  log.Info('overlap: mode=%s | distance scale: enabled=%s %.0f..%.0f -> %.2fx | click-to-target=%s',
           types.PlateOverlapModeLabels[cfg.overlap.mode] or '?',
           tostring(cfg.distScale.enabled), cfg.distScale.near, cfg.distScale.far,
           cfg.distScale.minScale, tostring(cfg.clickTarget))
  log.Info('MQEQGFX build: %s', eqgfx.build and eqgfx.build() or '?')
  local names = {}
  scan_ui_rects(names)
  local scanned, found = eqgfx.ui_stats()
  log.Info('hide under windows: %s | %d window names scanned, %d open: %s',
           tostring((settings.data or {}).hideUnderUI), scanned, found,
           #names > 0 and table.concat(names, ', ') or '(none)')
  log.Info('use /npui show to draw detected occluder rects on screen')
  local sts = ae.states(settings.data or {}, mq.TLO.Me.X() or 0, mq.TLO.Me.Y() or 0, mq.TLO.Me.Z() or 0)
  if #sts > 0 then
    for _, st in ipairs(sts) do
      log.Info('AE area in flight: %s %s marks %s plates%s', st.kind, tostring(st.shape),
               st.marks, st.groupOnly and ' (my group only)' or '')
    end
  else
    log.Info('AE highlight: idle (no area casts in flight)')
  end
end)

mq.bind('/npui', function(...)
  local args = { ... }
  if args[1] == 'show' then
    showUiRects = not showUiRects
    log.Info('occluder rect overlay = %s (%d rects)', tostring(showUiRects), #uiRects)
    return
  end
  if args[1] == 'add' and args[2] then
    local name = table.concat(args, ' ', 2)
    local list = (settings.data or {}).extraWindows
    for _, existing in ipairs(list) do
      if existing == name then
        log.Info('occlusion window already listed: "%s"', name)
        return
      end
    end
    table.insert(list, name)
    uirects.add_names({ name })
    settings.mark_dirty()
    log.Info('occlusion window added: "%s"', name)
  else
    local names = {}
    scan_ui_rects(names)
    log.Info('open EQ windows detected: %s', #names > 0 and table.concat(names, ', ') or '(none)')
    log.Info('missing one? find its name with /windows, then:  /npui add <Name>')
  end
end)

log.Info('nameplates running. /npmenu, /npradius N, /nppcs, /npdebug, /npui')

local buffScanWarned = false
local BUFF_INTERVAL = 1.0

---@param f fun(): any
---@return number|nil
local function getn(f)
  local okk, v = pcall(f)
  if okk and type(v) == 'number' then return v end
end

---@param f fun(): any
---@return boolean|nil
local function getbool(f)
  local okk, v = pcall(f)
  if okk then return v and true or false end
  return nil
end

---@param f fun(): any
---@return string|nil
local function getstr(f)
  local okk, v = pcall(f)
  if okk and type(v) == 'string' and v ~= '' then return v end
end

---@param list string[]|nil
---@param key string # ALREADY lowercased
---@return boolean
local function list_has(list, key)
  for _, v in ipairs(list or {}) do
    if tostring(v):lower() == key then return true end
  end
  return false
end

---@param buffCfg NpBuffsCfg # cfg.buffs
---@param entry BuffEntry
---@return boolean keep # false = drop the buff
local function apply_buff_rules(buffCfg, entry)
  local key = (entry.name or ''):lower()
  local ov = buffCfg.overrides[key]
  if ov and ov.hide then return false end
  if buffCfg.mineOnly and not entry.mine then return false end
  if buffCfg.filterMode == types.BuffFilterMode.WHITELIST and not list_has(buffCfg.whitelist, key) then
    return false
  end
  if buffCfg.filterMode == types.BuffFilterMode.BLACKLIST and list_has(buffCfg.blacklist, key) then
    return false
  end
  entry.scale = (ov and ov.scale) or 1
  entry.prio  = (ov and ov.priority) or 0
  return true
end

---@param lst BuffEntry[]
local function sort_buffs(lst)
  for n, entry in ipairs(lst) do entry._i = n end
  table.sort(lst, function(x, y)
    if x.prio ~= y.prio then return x.prio > y.prio end
    return x._i < y._i
  end)
end

---@param plate Plate # buffsB/buffsD replaced
---@param spawn spawn # live spawn TLO
---@param isSelf boolean
local function refresh_buffs(plate, spawn, isSelf)
  local buffCfg = (settings.data or {}).buffs
  local myName = mq.TLO.Me.CleanName() or ''
  local seen = {}
  for _, lst in ipairs({ plate.buffsB, plate.buffsD }) do
    for _, entry in ipairs(lst or {}) do seen[entry.id] = entry.addedAt end
  end
  local benefits, detriments = {}, {}
  local function add(buff)
    if not buff then return false end
    local id = getn(function() return buff.SpellID() end)
            or getn(function() return buff.Spell.ID() end)
    if not id or id <= 0 then return false end
    local icon = getn(function() return buff.SpellIcon() end) or 0
    local ben = getbool(function() return buff.Beneficial() end)
    local name = getstr(function() return buff.Name() end) or ''
    local caster = getstr(function() return buff.Caster() end)
    local dur = getn(function() return buff.Duration.TotalSeconds() end)
    local entry = { id = id, icon = icon, ben = ben and true or false,
                name = name, mine = (caster == myName),
                caster = caster, dur = dur, addedAt = seen[id] or animNow }
    if apply_buff_rules(buffCfg, entry) then
      if entry.ben then benefits[#benefits + 1] = entry else detriments[#detriments + 1] = entry end
    end
    return true
  end

  if isSelf then
    pcall(function()
      local me = mq.TLO.Me
      local total = me.CountBuffs() or 0
      local found, slot = 0, 1
      while found < total and slot <= 97 do
        if add(me.Buff(slot)) then found = found + 1 end
        slot = slot + 1
      end
    end)
  else
    local ok1 = pcall(function()
      local count = spawn.BuffCount() or 0
      for idx = 1, math.min(count, 40) do add(spawn.Buff(idx)) end
    end)
    if #benefits + #detriments == 0 then
      local ok2 = pcall(function()
        local n = spawn.CachedBuffCount() or -1
        for idx = 1, math.min(n, 40) do add(spawn.CachedBuff('*' .. idx)) end
      end)
      if not ok1 and not ok2 and not buffScanWarned then
        buffScanWarned = true
        log.Warn('buff scan failed: neither Spawn.Buff nor Spawn.CachedBuff works here')
      end
    end
  end
  sort_buffs(benefits)
  sort_buffs(detriments)
  plate.buffsB, plate.buffsD = benefits, detriments
end

local slowExempt, slowExemptAt = {}, 0

local function slow_exempt_ids(ex)
  local out = {}
  if ex.xtarget then
    local slots = getn(function() return mq.TLO.Me.XTargetSlots() end)
               or getn(function() return mq.TLO.Me.XTarget() end) or 0
    pcall(function()
      for slot = 1, slots do
        local xid = mq.TLO.Me.XTarget(slot).ID()
        if xid and xid > 0 then out[xid] = true end
      end
    end)
  end
  if ex.raid then
    pcall(function()
      for idx = 1, mq.TLO.Raid.Members() or 0 do
        local rid = mq.TLO.Raid.Member(idx).Spawn.ID()
        if rid and rid > 0 then out[rid] = true end
      end
    end)
  end
  if ex.fellowship then
    pcall(function()
      local fellow = mq.TLO.Me.Fellowship
      for idx = 1, fellow.Members() or 0 do
        local name = fellow.Member(idx).Name()
        local fid = name and name ~= '' and mq.TLO.Spawn('=' .. name).ID()
        if fid and fid > 0 then out[fid] = true end
      end
    end)
  end
  return out
end

---@param cfg NameplatesConfig # nameplates settings (los.exempt)
---@param groupIDs table<integer, boolean> # my group's spawn IDs
---@return table<integer, boolean> exempt
local function los_exempt_ids(cfg, groupIDs)
  local ex = cfg.los.exempt
  local out = {}
  if ex.group then
    for gid in pairs(groupIDs) do out[gid] = true end
  end
  if ex.target then
    local tid = mq.TLO.Target.ID()
    if tid and tid > 0 then out[tid] = true end
  end
  if ex.pet then
    local pid = mq.TLO.Me.Pet.ID()
    if pid and pid > 0 then out[pid] = true end
  end
  local clock = casts.now()
  if clock - slowExemptAt >= 1 then
    slowExempt, slowExemptAt = slow_exempt_ids(ex), clock
  end
  for sid in pairs(slowExempt) do out[sid] = true end
  return out
end

---@param spawn spawn # candidate spawn TLO
---@param id integer # its spawn ID
---@param interval number # seconds between re-checks
---@param timeNow number # casts.now()
---@return boolean visible
local function in_los(spawn, id, interval, timeNow)
  local cached = losCache[id]
  if cached and (timeNow - cached.at) < interval then return cached.ok end
  local okk, v = pcall(function() return spawn.LineOfSight() end)
  local visible = (not okk) or (v and true or false)
  losCache[id] = { at = timeNow, ok = visible }
  return visible
end

---@param cfg NameplatesConfig # nameplates settings (show.*)
---@param spawn spawn # candidate spawn TLO
---@param id integer # its spawn ID
---@param myID integer|nil # my spawn ID
---@return boolean allow # show a plate for it?
---@return string kind # spawn category: 'npc'|'pc'|'pet'|'merc'|'other' (the AE highlight's eligibility key)
local function allowed(cfg, spawn, id, myID)
  if id == myID then return cfg.show.self, 'pc' end
  local t = spawn.Type()
  if t == 'NPC' then return cfg.show.npcs, 'npc' end
  if t == 'PC'  then return cfg.show.pcs, 'pc' end
  if t == 'Pet'       then return cfg.show.pets, 'pet' end
  if t == 'Mercenary' then return cfg.show.pets, 'merc' end
  return false, 'other'
end

local function discover()
  local cfg = settings.data or {}
  local myID = mq.TLO.Me.ID()
  local spec = 'radius ' .. cfg.radius
  local present = {}

  local groupIDs = {}
  pcall(function()
    local n = mq.TLO.Group.Members() or 0
    for gi = 1, n do
      local gid = mq.TLO.Group.Member(gi).ID()
      if gid and gid > 0 then groupIDs[gid] = true end
    end
  end)

  local tgtID = mq.TLO.Target.ID() or 0
  local clock = casts.now()

  local losCfg = cfg.los
  local losExempt = losCfg.enabled and los_exempt_ids(cfg, groupIDs) or nil
  local losNow, losSeen = clock, {}
  if not losCfg.enabled and next(losCache) then losCache = {} end

  for _, sid in ipairs(eqgfx.spawn_search(spec)) do
    local spawn = mq.TLO.Spawn(sid) --[[@as spawn]]
    if spawn() and spawn.X() then
      local id = spawn.ID()
      local allow, kind = allowed(cfg, spawn, id, myID)
      if allow and losExempt and id ~= myID and not losExempt[id] then
        losSeen[id] = true
        allow = in_los(spawn, id, losCfg.interval, losNow)
      end
      if allow then
        present[id] = true
        local plate = plates[id]
        if plate then
          if plate.lostAt then plate.lostAt, plate.lossAlpha, plate.bornAt = nil, nil, animNow - 1 end
        else
          plate = types.Plate.new(id, spawn.CleanName() or '?',
                              (spawn.PctHPs() or 0) / 100, animNow)
          plates[id] = plate
        end
        plate.kind    = kind
        plate.isSelf  = (id == myID)
        plate.inGroup = groupIDs[id] or false
        if plate.isPC == nil then
          plate.isPC = (spawn.Type() == 'PC')
          if plate.isPC then
            plate.cls      = getstr(function() return spawn.Class.Name() end)
                      or getstr(function() return spawn.Class() end)
            plate.clsShort = getstr(function() return spawn.Class.ShortName() end)
          end
        end
        if cfg.buffs.enabled then
          if cfg.buffs.onlyTarget and id ~= tgtID then
            plate.buffsB, plate.buffsD = nil, nil
          elseif id == tgtID or id == myID or clock >= (plate.buffDue or 0) then
            plate.buffDue = clock + BUFF_INTERVAL
            refresh_buffs(plate, spawn, id == myID)
          end
        end
      end
    end
  end
  for id, plate in pairs(plates) do
    if not present[id] and not plate.lostAt then
      plate.lostAt, plate.lossAlpha = animNow, plate.alpha
    end
  end
  if losExempt then
    for id in pairs(losCache) do
      if not losSeen[id] then losCache[id] = nil end
    end
  end
end

local DISCOVER_INTERVAL = 0.2
local nextDiscoverAt = 0

while true do
  mq.doevents()
  local cfg = settings.data or {}

  local idle = pause.active(cfg.pause)

  if cfg.enabled and not idle then
    if cfg.castbar.show or cfg.aehl.enabled then
      casts.config{ interruptDetect = cfg.castbar.interruptDetect, radius = cfg.radius }
      casts.update()
    end
    local clock = casts.now()
    if clock >= nextDiscoverAt then
      nextDiscoverAt = clock + (cfg.los.enabled and math.min(DISCOVER_INTERVAL, cfg.los.interval) or DISCOVER_INTERVAL)
      discover()
    end
  elseif not cfg.enabled and next(plates) then
    plates = {}
  end

  settings.maybe_save()
  mq.delay(50)
end
