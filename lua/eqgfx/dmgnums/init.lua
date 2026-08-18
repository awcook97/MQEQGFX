--[[
  dmgnums/init.lua - floating combat damage numbers over spawns.

  Events:     native eqgfx.poll_damage() - the MQEQGFX plugin detours the
              client's own CEverQuest::ReportSuccessfulHit/Heal, so every
              melee hit, spell DD, DoT tick, miss (defense codes) and heal
              arrives here with attacker/target spawn IDs - no chat parsing.
  Projection: native eqgfx world->screen (numbers stick to the spawn).
  Drawing:    ImGui draw list inside the same fullscreen pass-through overlay
              trick nameplates use (numbers stay under other ImGui windows).

  Classification:
    kind    melee | spells | dots | heals | misses
            (spells vs dots = spell duration lookup, cached per spell ID;
             misses = DamageCaused <= 0 defense codes -> label text)
    source  mine | pet | others | incoming   (relative to me)
  Both are filterable per-cell from /dnmenu - the in-game filter grid.

  Run:  /lua run eqgfx/dmgnums
  Stop: /lua stop eqgfx/dmgnums
  Opts: /dnmenu     in-game customization window (auto-saved settings)
        /dmgnums    quick enable/disable toggle
        /dntest     spawn one sample of every category over your target
]]

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
log.SetOutputFile(mq.configDir .. '/eqgfx.log')

local ok, err = eqgfx.init()
if not ok then log.Error('init failed: %s', err) return end
if not eqgfx.poll_damage then
  log.Error('this MQEQGFX build has no damage tap (poll_damage missing) - rebuild/update the plugin')
  return
end

settings.load()

----------------------------------------------------------------------------
-- Small animation helpers (self-contained; mirror nameplates/anim.lua).
----------------------------------------------------------------------------
local function clamp01(v) return v < 0 and 0 or (v > 1 and 1 or v) end

--- Overshoot ease for the spawn pop (eases 0..1 with a little bounce past 1).
local function ease_out_back(t)
  t = clamp01(t)
  local c1, c3 = 1.70158, 2.70158
  local u = t - 1
  return 1 + c3 * u * u * u + c1 * u * u
end

----------------------------------------------------------------------------
-- Live floaters.
----------------------------------------------------------------------------
---@type DmgFloater[]
local floaters = {}

-- Per-spawn burst tracking for stackNudge: rapid consecutive numbers on the
-- same spawn spawn a bit higher so they don't overlap each other.
local lastBurst = {}   -- spawnID -> { at = frameTime, n = count }

-- spellID -> true (DoT) / false (DD); duration lookup done once per spell.
local dotCache = {}

local FRAME_DT = 1 / 35
local animNow  = 0

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

--- Bucket one native damage event: which filter cell it belongs to and which
--- spawn the number floats over. Returns nil when it should be dropped.
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
    kind = types.Kind.DOTS      -- from the "has taken N damage" chat tap; known DoT
  elseif ev.spellID and ev.spellID > 0 then
    -- Spell / DoT. damage <= 0 here is a resist or the DoT's initial landing
    -- (0 immediate damage) - NOT a melee "miss"; drop it so it doesn't floater.
    if ev.damage <= 0 then return nil end
    kind = is_dot(ev.spellID) and types.Kind.DOTS or types.Kind.SPELLS
  elseif ev.damage <= 0 then
    kind = types.Kind.MISSES   -- melee only: defensive codes (miss/dodge/parry/...)
  else
    kind = types.Kind.MELEE
  end

  local show = cfg.show[source]
  if not (show and show[kind]) then return nil end
  if kind ~= types.Kind.MISSES and kind ~= types.Kind.HEALS
      and ev.damage < (cfg.minDamage or 0) then return nil end

  -- Where the number floats: over the victim/healed spawn; incoming
  -- optionally over my own head (classic style).
  local anchor = ev.targetID
  if source == types.Source.INCOMING and cfg.incomingOverSelf then anchor = myID end
  return source, kind, anchor
end

--- Color for a source+kind cell (heals split by direction).
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

--- "12345" -> "12.3k" (only at >= 10000, and only when cfg wants it).
local function fmt_amount(cfg, n)
  if cfg.text.abbreviate and n >= 10000 then
    local s = string.format('%.1fk', n / 1000)
    return (s:gsub('%.0k$', 'k'))
  end
  return tostring(n)
end

--- Turn one classified event into a live floater. `dist` is how far the
--- anchor spawn is, for the distance size falloff (nil = full size).
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

  -- size scaling by damage
  local size = cfg.text.size
  if cfg.text.scaleWithDamage and kind ~= types.Kind.MISSES and ev.damage > 0 then
    local f = clamp01(ev.damage / math.max(cfg.text.bigAt, 1))
    size = size * (1 + (cfg.text.bigScale - 1) * f)
  end
  local dscale = distscale.factor(cfg.distScale, dist)
  size = size * dscale

  -- burst nudge: numbers landing on the same spawn in quick succession start higher
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

----------------------------------------------------------------------------
-- Event intake: classify -> filter -> range -> spawn. Shared by the native
-- poll_damage feed and the DoT-tick chat tap.
----------------------------------------------------------------------------
local function handle_event(ev, cfg, myID, petID, tgtID)
  local source, kind, anchorID = classify(ev, cfg, myID, petID)
  if not (source and anchorID and anchorID > 0) then return end
  -- outgoing target-only filter
  if cfg.targetOnly and (source == types.Source.MINE or source == types.Source.PET)
      and anchorID ~= tgtID then return end
  -- range filter on the anchor spawn
  local spawn = mq.TLO.Spawn(anchorID)
  local dist = (spawn.ID() == anchorID) and spawn.Distance3D() or nil
  if dist and dist <= cfg.radius then
    spawn_floater(cfg, source, kind, anchorID, ev, dist)
  end
end

--- Native combat feed (runs inside the draw callback, before rendering).
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

----------------------------------------------------------------------------
-- DoT-tick chat tap. DoT ticks are periodic and reported via chat, NOT through
-- the native ReportSuccessfulHit path (which only fires for the DoT's landing).
-- The two "has taken N damage" formats (eqstr 9072 / 13327) are distinct from
-- direct-nuke messages, so this doesn't double-count nukes.
----------------------------------------------------------------------------
local function to_dmg(s) return tonumber((tostring(s):gsub('[^%d]', ''))) end

local function spawn_id(name, myID, myName)
  if not name or name == '' then return 0 end
  if name == 'You' or name == 'you' or name == myName then return myID end
  local id = mq.TLO.Spawn('=' .. name).ID()
  return id or 0
end

--- Feed one DoT tick (from either chat format) through the shared handler.
local function feed_dot(cfg, casterID, targetID, dmg)
  if not cfg.enabled or not dmg or dmg <= 0 or targetID == 0 then return end
  local myID = mq.TLO.Me.ID() or 0
  local petID = mq.TLO.Me.Pet.ID() or 0
  local tgtID = mq.TLO.Target.ID() or 0
  handle_event({ attackerID = casterID, targetID = targetID, damage = dmg,
                 spellID = 0, skill = 0, heal = false, isDotTick = true },
               cfg, myID, petID, tgtID)
end

----------------------------------------------------------------------------
-- Rendering.
----------------------------------------------------------------------------
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

-- Sized AddText(font, size, pos, col, str) availability - probed once, same
-- trick as nameplates/render.lua. Without it, numbers draw at the default
-- font size (still fully functional).
local sizedText = nil
local function probe_sized(drawList)
  if sizedText ~= nil then return end
  sizedText = pcall(function()
    drawList:AddText(ImGui.GetFont(), 13, ImVec2(-4096, -4096), 0x01FFFFFF, 'x')
  end)
end

--- One AddText at (tx, y) honoring the sized-text capability.
local function add_text(drawList, tx, y, size, col, str)
  if sizedText then
    drawList:AddText(ImGui.GetFont(), size, ImVec2(tx, y), col, str)
  else
    drawList:AddText(ImVec2(tx, y), col, str)
  end
end

--- Draw text with the configured shadow/outline, centered on x.
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
  if cfg.hideUnderUI and uirects.native then
    local rect = uirects.get()
    if #rect > 0 or uirects.proven then uiRects = rect end
  end

  local alive = 0
  for i = 1, #floaters do
    local f = floaters[i]
    local age = animNow - f.bornAt
    if age <= cfg.anim.duration then
      alive = alive + 1
      floaters[alive] = f

      -- anchor: live spawn position while it exists, else last screen pos
      local spawn = mq.TLO.Spawn(f.spawnID)
      if spawn.ID() == f.spawnID and spawn.X() then
        local sx, sy, vis = eqgfx.world_to_screen(spawn.X(), spawn.Y(), (spawn.Z() or 0) + cfg.zOffset)
        if vis then f.sx, f.sy = sx, sy end
      end

      if f.sx then
        -- motion by style
        local ox, oy = f.dx, -(f.dy0 or 0)
        local style = cfg.anim.style
        if style == types.AnimStyle.FLOAT then
          oy = oy - cfg.anim.riseSpeed * age
        elseif style == types.AnimStyle.ARC or style == types.AnimStyle.FOUNTAIN then
          ox = ox + f.vx * age
          oy = oy - (f.vy * age - 0.5 * cfg.anim.gravity * age * age)
        end

        -- alpha: hold, then fade to 0 across the tail of the lifetime
        local alpha = 1.0
        local fadeFrom = cfg.anim.fadeStart * cfg.anim.duration
        if age > fadeFrom then
          alpha = 1 - (age - fadeFrom) / math.max(cfg.anim.duration - fadeFrom, 0.01)
        end

        -- pop-in scale
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
  animNow = animNow + FRAME_DT
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
    eqgfx.poll_damage()   -- keep draining so re-enable doesn't burst-replay
  end

  menu.draw()
end

mq.imgui.init('eqgfx_dmgnums', draw)

----------------------------------------------------------------------------
-- DoT-tick chat events (processed by mq.doevents() in the main loop).
--   eqstr 9072:  "%1 has taken %2 damage from your %3."   (my DoT)
--   eqstr 13327: "%1 has taken %2 damage from %3 by %4."  (someone's DoT)
----------------------------------------------------------------------------
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

----------------------------------------------------------------------------
-- Binds.
----------------------------------------------------------------------------
mq.bind('/dnmenu', function() menu.toggle() end)

mq.bind('/dmgnums', function()
  local cfg = settings.data
  cfg.enabled = not cfg.enabled
  settings.mark_dirty()
  log.Info('damage numbers %s', cfg.enabled and 'ON' or 'OFF')
end)

-- One fake event per category over the target (self if none) - for tuning
-- looks without needing combat.
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
    -- stagger the spawns a touch so they read as a burst, not a clump
    local born = animNow + (n - 1) * 0.12
    local saved = animNow
    animNow = born
    spawn_floater(cfg, s.source, s.kind, anchorID, s.ev)
    animNow = saved
  end
  log.Info('spawned %d test numbers', #samples)
end)

log.Info('damage numbers up - /dnmenu to customize, /dntest to preview')

----------------------------------------------------------------------------
-- Main loop: settings persistence only (events + drawing live in the draw
-- callback; poll_damage is drained there every frame).
----------------------------------------------------------------------------
while true do
  mq.doevents()
  settings.maybe_save(log)
  mq.delay(100)
end
