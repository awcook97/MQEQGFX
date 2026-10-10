---@class SpellGeom
---@field targetType integer # eSpellTargetType
---@field range number # base range, world units
---@field aeRange number # area radius (0 when not an area spell)
---@field coneStart integer # cone start angle, degrees relative to facing
---@field coneEnd integer # cone end angle, degrees

---@alias RGBA number[] # {r, g, b, a} floats 0..1

local T = {}

T.TargetType = {
  AEPC_v1         = 2,   Group_v1        = 3,   PBAE            = 4,
  Single          = 5,   Self            = 6,   TargetArea      = 8,
  TargetAEDrain   = 20,  TargetAEUndead  = 24,  TargetAESummoned= 25,
  CasterAreaPC    = 36,  CasterAreaNPC   = 37,  AEPC_v2         = 40,
  Group_v2        = 41,  DirectionalCone = 42,  Beam            = 44,
  FreeTarget      = 45,  AreaDetrimental = 50,
}

local TT = T.TargetType
---@type table<integer, boolean> # target types whose area sits on the caster
T.CasterCentered = {
  [TT.PBAE]=true, [TT.AEPC_v1]=true, [TT.Group_v1]=true, [TT.AEPC_v2]=true,
  [TT.Group_v2]=true, [TT.CasterAreaPC]=true, [TT.CasterAreaNPC]=true,
}
---@type table<integer, boolean> # target types whose area sits on the caster's target
T.TargetCentered = {
  [TT.TargetArea]=true, [TT.AreaDetrimental]=true, [TT.TargetAEDrain]=true,
  [TT.TargetAEUndead]=true, [TT.TargetAESummoned]=true, [TT.FreeTarget]=true,
}

return T
