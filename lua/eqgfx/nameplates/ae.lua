local mq    = require('mq')
local eqgfx = require('eqgfx')
local casts = require('eqgfx.casts')
local core  = require('eqgfx.core._types')

local TT              = eqgfx.TargetType
local CASTER_CENTERED = core.CasterCentered
local TARGET_CENTERED = core.TargetCentered

local GROUP_ONLY = { [TT.Group_v1] = true, [TT.Group_v2] = true }

local BEN_TYPES = { [TT.Group_v1] = true, [TT.Group_v2] = true, [TT.AEPC_v1] = true,
                    [TT.AEPC_v2] = true, [TT.CasterAreaPC] = true }

local M = {}

---@class AeState
---@field kind string        'det' | 'ben' (highlight color)
---@field marks string       'pc' | 'npc' - which plate side this area marks
---@field shape string|nil   'circle' | 'cone' | 'beam' (always set on a returned state)
---@field cx number          area origin (caster, or the resolved victim)
---@field cy number
---@field cz number
---@field r2 number|nil      squared radius (circle + cone reach test)
---@field mid number|nil     cone center angle (rad, world atan2 frame)
---@field half number|nil    cone half angle (rad)
---@field len number|nil     beam length
---@field hw number|nil      beam half width
---@field fx number|nil      facing unit vector (beam)
---@field fy number|nil
---@field groupOnly boolean  marks only my group (caster is me / in my group)

local perCaster = {}

---@param deg number|nil # Spawn.Heading.Degrees()
---@return number rad # angle usable with math.cos/sin in world X/Y
local function facing_rad(deg)
  return math.rad(90 + (deg or 0))
end

---@return table<integer, boolean> ids
local function my_group_ids()
  local ids = {}
  pcall(function()
    local n = mq.TLO.Group.Members() or 0
    for gi = 1, n do
      local gid = mq.TLO.Group.Member(gi).ID()
      if gid and gid > 0 then ids[gid] = true end
    end
  end)
  return ids
end

---@param id integer # caster spawn ID
---@param castInfo CastInfo # from the shared tracker
---@param myID integer # my spawn ID
---@param meX number # my position (already read by the caller)
---@param meY number
---@param meZ number
---@param aehl NpAehlCfg # cfg.aehl (source filters)
---@param groups fun(): table<integer, boolean> # lazy my-group-IDs lookup
---@return AeState|nil
local function state_for(id, castInfo, myID, meX, meY, meZ, aehl, groups)
  if castInfo.interrupted or (castInfo.isSelf and castInfo.done) then return nil end
  if not castInfo.spellID then return nil end
  local isSelf = castInfo.isSelf or id == myID

  local cx, cy, cz, headingDeg, side, src
  local spawn
  if isSelf then
    cx, cy, cz = meX, meY, meZ
    side, src = 'pc', 'me'
  else
    spawn = mq.TLO.Spawn(id)
    if not (spawn.ID() == id and spawn.X()) then return nil end
    cx, cy, cz = spawn.X(), spawn.Y(), spawn.Z()
    local cached = perCaster[id]
    if cached and cached.side then
      side, src = cached.side, cached.src
    else
      side = (spawn.Type() == 'NPC') and 'npc' or 'pc'
      src  = (side == 'npc') and 'npcs' or 'pcs'
    end
  end
  if src == 'me'   and not aehl.fromMe   then return nil end
  if src == 'pcs'  and not aehl.fromPCs  then return nil end
  if src == 'npcs' and not aehl.fromNPCs then return nil end

  local cc = perCaster[id]
  if not cc or cc.spellID ~= castInfo.spellID then
    cc = { spellID = castInfo.spellID, side = side, src = src,
           geom = eqgfx.spell_geom(castInfo.spellID) }
    local okk, ben = pcall(function() return mq.TLO.Spell(castInfo.spellID).Beneficial() end)
    if okk and type(ben) == 'boolean' then cc.ben = ben end
    perCaster[id] = cc
  end
  local geom = cc.geom
  if not geom then return nil end

  local tt = geom.targetType
  local ben = cc.ben
  if ben == nil then ben = BEN_TYPES[tt] or false end
  local radius = (geom.aeRange and geom.aeRange > 0) and geom.aeRange or geom.range

  local groupOnly = false
  if GROUP_ONLY[tt] then
    if side ~= 'pc' then return nil end
    if not isSelf and not groups()[id] then return nil end
    groupOnly = true
  end

  ---@type AeState
  local st = {
    kind      = ben and 'ben' or 'det',
    marks     = ben and side or ((side == 'pc') and 'npc' or 'pc'),
    groupOnly = groupOnly,
    cx = cx, cy = cy, cz = cz,
  }

  if tt == TT.DirectionalCone then
    if radius <= 0 then return nil end
    st.r2 = radius * radius
    if math.abs(geom.coneEnd - geom.coneStart) < 0.5 then
      st.shape = 'circle'
    else
      headingDeg = isSelf and mq.TLO.Me.Heading.Degrees() or spawn.Heading.Degrees()
      local facing = facing_rad(headingDeg)
      local a0, a1 = facing + math.rad(geom.coneStart), facing + math.rad(geom.coneEnd)
      st.shape = 'cone'
      st.mid   = (a0 + a1) * 0.5
      st.half  = math.abs(a1 - a0) * 0.5
    end
  elseif tt == TT.Beam then
    local len = (geom.range and geom.range > 0) and geom.range or radius
    if len <= 0 then return nil end
    headingDeg = isSelf and mq.TLO.Me.Heading.Degrees() or spawn.Heading.Degrees()
    local facing = facing_rad(headingDeg)
    st.shape = 'beam'
    st.len   = len
    st.hw    = (geom.aeRange and geom.aeRange > 0) and geom.aeRange or 5
    st.fx, st.fy = math.cos(facing), math.sin(facing)
  elseif CASTER_CENTERED[tt] then
    if radius <= 0 then return nil end
    st.shape = 'circle'
    st.r2    = radius * radius
  elseif TARGET_CENTERED[tt] then
    if radius <= 0 then return nil end
    local vx, vy, vz
    local tgt = mq.TLO.Target
    local tid = tgt.ID() or 0
    if isSelf then
      if tid > 0 and tgt.X() then vx, vy, vz = tgt.X(), tgt.Y() or 0, tgt.Z() or 0 end
    elseif tid == id then
      local ahid = tgt.AggroHolder.ID()
      if ahid and ahid > 0 then
        local vsp = mq.TLO.Spawn(ahid)
        if vsp.ID() == ahid and vsp.X() then vx, vy, vz = vsp.X(), vsp.Y(), vsp.Z() end
      end
    end
    if not vx then return nil end
    st.shape = 'circle'
    st.r2    = radius * radius
    st.cx, st.cy, st.cz = vx, vy, vz
  else
    return nil
  end

  return st
end

---@param cfg NameplatesConfig # the nameplates settings (reads cfg.aehl)
---@param meX number # my position (the caller already read it)
---@param meY number
---@param meZ number
---@return AeState[] states # empty when nothing area-shaped is casting
function M.states(cfg, meX, meY, meZ)
  local aehl = cfg.aehl
  local out = {}
  local myID = mq.TLO.Me.ID() or 0
  local all = casts.all()
  for id in pairs(perCaster) do
    if not all[id] then perCaster[id] = nil end
  end
  local groupIDs
  local function groups()
    groupIDs = groupIDs or my_group_ids()
    return groupIDs
  end
  for id, castInfo in pairs(all) do
    local st = state_for(id, castInfo, myID, meX, meY, meZ, aehl, groups)
    if st then out[#out + 1] = st end
  end
  return out
end

---@param st AeState # one area from states()
---@param plate Plate # the plate being tested (kind/isSelf/inGroup matter)
---@param x number # the spawn's live position (TLO order)
---@param y number
---@param z number
---@return string|nil kind # st.kind ('det'|'ben') when affected, else nil
function M.affects(st, plate, x, y, z)
  if st.marks == 'npc' then
    if plate.kind ~= 'npc' then return nil end
  else
    if not (plate.isSelf or plate.kind == 'pc') then return nil end
    if st.groupOnly and not (plate.isSelf or plate.inGroup) then return nil end
  end

  local dx, dy, dz = x - st.cx, y - st.cy, z - st.cz
  if st.shape == 'circle' then
    if dx * dx + dy * dy + dz * dz > st.r2 then return nil end
  elseif st.shape == 'cone' then
    if dx * dx + dy * dy > st.r2 then return nil end
    local diff = (math.atan2(dy, dx) - st.mid + math.pi) % (2 * math.pi) - math.pi
    if math.abs(diff) > st.half then return nil end
  else
    local along = dx * st.fx + dy * st.fy
    if along < 0 or along > st.len then return nil end
    local perp = dy * st.fx - dx * st.fy
    if math.abs(perp) > st.hw then return nil end
  end
  return st.kind
end

return M
