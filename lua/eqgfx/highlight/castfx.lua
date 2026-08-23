--[[
  highlight/castfx.lua - a distinct animation per spell, colored by what the
  spell does.

  SHAPE COMES FROM THE SPELL, NOT A GUESS
  ---------------------------------------
  eqgfx.spell_geom(spellID) returns the client's own targetType, AE radius and
  cone angles, and core/_types.lua already groups the target types into
  caster-centered and target-centered sets (the indicators script uses the same
  tables). So the animation is driven by the spell's real footprint:

    beam          a streak racing out along the caster's facing
    cone          a fan of arcs sweeping through the real cone angles
    caster area   a shockwave ring expanding from the caster to the AE radius
    target area   a ring collapsing onto the victim, then snapping outward
    single        a bolt travelling caster -> victim
    self          a column of rings climbing the caster
    unknown       the self column, at a default radius

  COLOR COMES FROM WHAT IT DOES
  -----------------------------
  Beneficial spells take the configured heal color. Detrimental ones are
  colored by resist type (fire orange, cold cyan, poison green, ...), which
  reads instantly and costs one TLO lookup, cached per spell ID. Turn
  byResist off in the menu for a single flat detrimental color.

  TWO PHASES
  ----------
  While a cast is in flight it gets a *charge* animation that tightens as the
  cast bar fills. When it completes, the charge is replaced by a *release*
  burst with its own short lifetime. Interrupted casts fizzle instead: the
  charge unwinds and fades. init.lua owns the event bookkeeping; this module
  owns what each phase looks like.
]]

local mq = require('mq')
local S  = require('eqgfx.highlight.shapes')
local core = require('eqgfx.core._types')

local sin, cos, pi = math.sin, math.cos, math.pi
local floor, max, min = math.floor, math.max, math.min
local TAU = pi * 2

local M = {}

--- eqgfx.spell_geom, injected at startup so this module has no dependency on
--- the plugin binding and stays unit-testable.
---@type fun(spellID: integer): SpellGeom|nil
local geom_source = function() return nil end

--- Hand in eqgfx.spell_geom once at startup.
---@param fn fun(spellID: integer): SpellGeom|nil
function M.set_geom_source(fn) geom_source = fn end

----------------------------------------------------------------------------
-- Classification
----------------------------------------------------------------------------

--- The shape families a cast can animate as.
---@alias CastShape '"beam"'|'"cone"'|'"casterArea"'|'"targetArea"'|'"single"'|'"self"'|'"unknown"'

local TT = core.TargetType

--- What the spell looks like in the world.
---@param geom SpellGeom|nil
---@return CastShape
local function shape_of(geom)
	if not geom or geom.targetType < 0 then return 'unknown' end
	local tt = geom.targetType
	if tt == TT.Beam then return 'beam' end
	if tt == TT.DirectionalCone then return 'cone' end
	if tt == TT.Self then return 'self' end
	if core.CasterCentered[tt] then return 'casterArea' end
	if core.TargetCentered[tt] then return 'targetArea' end
	return 'single'
end

--- Resist-type palette keys. The strings are what Spell.ResistType returns;
--- anything unlisted falls back to the generic detrimental color.
---@type table<string, string>
local RESIST_KEY = {
	Fire = 'fire', Cold = 'cold', Magic = 'magic', Poison = 'poison',
	Disease = 'disease', Corruption = 'corruption', Physical = 'physical',
	Chromatic = 'arcane', Prismatic = 'arcane', Unresistable = 'arcane',
}

---@class CastClass
---@field shape CastShape
---@field geom SpellGeom|nil
---@field beneficial boolean
---@field resist string|nil  # palette key, detrimental only
---@field aeRange number
---@field coneStart number
---@field coneEnd number

---@type table<integer, CastClass>
local classCache = {}

--- Classify a spell once and remember it. Spell data never changes mid-session,
--- and this runs on every cast start in a raid, so caching matters.
---@param spellID integer|nil
---@return CastClass
function M.classify(spellID)
	if not spellID or spellID <= 0 then
		return { shape = 'unknown', beneficial = false, aeRange = 0, coneStart = 0, coneEnd = 0 }
	end
	local hit = classCache[spellID]
	if hit then return hit end

	local okGeom, geom = pcall(geom_source, spellID)
	if not okGeom then geom = nil end

	local beneficial = false
	pcall(function() beneficial = mq.TLO.Spell(spellID).Beneficial() and true or false end)

	local resist
	if not beneficial then
		local rt
		pcall(function() rt = mq.TLO.Spell(spellID).ResistType() end)
		resist = RESIST_KEY[rt or ''] or nil
	end

	local c = {
		shape      = shape_of(geom),
		geom       = geom,
		beneficial = beneficial,
		resist     = resist,
		aeRange    = (geom and geom.aeRange) or 0,
		coneStart  = (geom and geom.coneStart) or 0,
		coneEnd    = (geom and geom.coneEnd) or 0,
	}
	classCache[spellID] = c
	return c
end

--- The color a cast draws in.
---@param class CastClass
---@param cfg table  # settings.data.castfx
---@return RGBA
function M.color(class, cfg)
	if class.beneficial then return cfg.colors.beneficial end
	if cfg.byResist and class.resist and cfg.colors[class.resist] then
		return cfg.colors[class.resist]
	end
	return cfg.colors.detrimental
end

----------------------------------------------------------------------------
-- Charge phase - while the cast bar is filling.
--
-- `pct` is 0..1 through the cast. Every charge animation tightens or speeds up
-- as it approaches 1 so a cast about to land reads as urgent.
----------------------------------------------------------------------------

--- @param x number @param y number @param z number # caster feet
--- @param class CastClass
--- @param color RGBA
--- @param cfg table       # settings.data.castfx
--- @param pct number      # 0..1 cast progress
--- @param now number
--- @param segs integer    # segment allowance
function M.draw_charge(x, y, z, class, color, cfg, pct, now, segs)
	local r = cfg.radius
	local t = now * (1 + pct * 2)                 -- spins up as it fills
	local mid = z + cfg.height * 0.5

	-- A ring collapsing inward as the cast completes: unmistakable at a
	-- glance, and the radius alone tells you how long is left.
	local collapse = r * (1.6 - 1.1 * pct)
	S.flat_ring(x, y, z + 0.25, collapse, max(8, floor(segs * 0.4)), color,
		(t * 0.3) % 1, 0.5 + 0.5 * pct)

	-- Two counter-rotating tilted rings on the body, tightening with it.
	local per = max(6, floor(segs * 0.3))
	S.gyro_ring(x, y, mid, collapse * 0.6, pi / 5, t * 0.9, per, color, (t * 0.25) % 1, 0.8)
	S.gyro_ring(x, y, mid, collapse * 0.6, -pi / 5, -t * 1.15, per, color, (0.5 - t * 0.25) % 1, 0.8)
end

--- Interrupted: the charge unwinds outward and fades. `age` is seconds since
--- the interrupt, `life` the configured fizzle duration.
function M.draw_fizzle(x, y, z, color, cfg, age, life, now, segs)
	local u = min(1, age / life)
	local fade = (1 - u) * (1 - u)
	-- Expanding, wobbling, dimming - the opposite of the charge collapse.
	local r = cfg.radius * (0.5 + u * 1.4)
	local wob = 1 + 0.12 * sin(now * 9 + u * 6)
	S.flat_ring(x, y, z + 0.25 + u * 1.5, r * wob, max(8, floor(segs * 0.5)),
		color, nil, fade * 0.8)
end

----------------------------------------------------------------------------
-- Release phase - the burst when the cast lands.
--
-- `u` is 0..1 through the burst's lifetime. Each shape has its own motion;
-- they all fade out on (1-u)^2 so nothing lingers as a hard edge.
----------------------------------------------------------------------------

local release = {}

--- Shockwave: a ring racing out to the spell's real AE radius, with a second
--- ring chasing it a beat behind.
function release.casterArea(fx, color, cfg, u, now, segs)
	local reach = fx.reach
	local fade = (1 - u) * (1 - u)
	local per = max(10, floor(segs * 0.5))
	local ease = 1 - (1 - u) * (1 - u) * (1 - u)          -- fast out, then settle
	S.flat_ring(fx.x, fx.y, fx.z + 0.3 + ease * 1.2, reach * ease, per, color, nil, fade)
	if u > 0.15 then
		local u2 = (u - 0.15) / 0.85
		local e2 = 1 - (1 - u2) * (1 - u2) * (1 - u2)
		S.flat_ring(fx.x, fx.y, fx.z + 0.3, reach * e2 * 0.8, per, color, nil, fade * 0.5)
	end
end

--- Target area: a ring collapsing onto the victim, then snapping back out.
--- Reads as "this is landing HERE" rather than "something happened".
function release.targetArea(fx, color, cfg, u, now, segs)
	local reach = fx.reach
	local per = max(10, floor(segs * 0.6))
	if u < 0.45 then
		local v = u / 0.45
		S.flat_ring(fx.tx, fx.ty, fx.tz + 0.3 + (1 - v) * 2.0,
			reach * (1.5 - v * 0.5), per, color, (now * 0.6) % 1, 0.5 + 0.5 * v)
	else
		local v = (u - 0.45) / 0.55
		local fade = (1 - v) * (1 - v)
		S.flat_ring(fx.tx, fx.ty, fx.tz + 0.3, reach * (1.0 + v * 0.6), per, color, nil, fade)
		S.flat_ring(fx.tx, fx.ty, fx.tz + 0.3 + v * 1.5, reach * (0.6 + v * 0.5),
			max(8, floor(per * 0.6)), color, nil, fade * 0.6)
	end
end

--- Cone: a fan of arcs sweeping through the spell's real cone angles.
function release.cone(fx, color, cfg, u, now, segs)
	local reach = fx.reach
	local fade = (1 - u) * (1 - u)
	local a0 = fx.heading + math.rad(fx.coneStart)
	local a1 = fx.heading + math.rad(fx.coneEnd)
	local bands = 3
	local per = max(4, floor(segs / (bands + 1)))
	for i = 1, bands do
		-- Each band leaves a beat after the one inside it, so the fan unfurls.
		local lag = (i - 1) * 0.12
		local v = min(1, max(0, (u - lag) / (1 - lag)))
		if v > 0 then
			local ease = 1 - (1 - v) * (1 - v)
			S.flat_arc(fx.x, fx.y, fx.z + 0.4 + i * 0.35,
				reach * ease * (0.55 + 0.15 * i), a0, a1, per, color, nil, fade)
		end
	end
	-- Edge rays so the cone's extent is unambiguous.
	local ease = 1 - (1 - u) * (1 - u)
	for _, a in ipairs({ a0, a1 }) do
		S.streak(fx.x, fx.y, fx.z + 0.4,
			fx.x + cos(a) * reach * ease, fx.y + sin(a) * reach * ease, fx.z + 0.4,
			per, color, nil, fade * 0.8)
	end
end

--- Beam: a streak firing out along the caster's facing, head first.
function release.beam(fx, color, cfg, u, now, segs)
	local reach = fx.reach
	local fade = 1 - u
	local a = fx.heading
	S.streak(fx.x, fx.y, fx.z + 1.0,
		fx.x + cos(a) * reach, fx.y + sin(a) * reach, fx.z + 1.0,
		max(8, floor(segs * 0.6)), color, u, fade)
	-- A ring riding the head.
	local hx, hy = fx.x + cos(a) * reach * u, fx.y + sin(a) * reach * u
	S.gyro_ring(hx, hy, fx.z + 1.0, cfg.radius * 0.5, pi / 2, a, max(6, floor(segs * 0.25)),
		color, nil, fade)
end

--- Single target: a bolt travelling caster -> victim, with a small burst at
--- the far end once it arrives.
function release.single(fx, color, cfg, u, now, segs)
	local fade = 1 - u
	S.streak(fx.x, fx.y, fx.z + 1.2, fx.tx, fx.ty, fx.tz + 1.2,
		max(8, floor(segs * 0.5)), color, u, 1.0)
	if u > 0.6 then
		local v = (u - 0.6) / 0.4
		S.flat_ring(fx.tx, fx.ty, fx.tz + 0.3, cfg.radius * (0.3 + v * 1.2),
			max(8, floor(segs * 0.4)), color, nil, (1 - v) * (1 - v))
	end
end

--- Self: a column of rings climbing the caster and fading off the top.
release['self'] = function(fx, color, cfg, u, now, segs)
	local rings = 3
	local per = max(8, floor(segs / rings))
	for i = 1, rings do
		local lag = (i - 1) * 0.15
		local v = min(1, max(0, (u - lag) / (1 - lag)))
		if v > 0 then
			local fade = (1 - v) * (1 - v)
			S.flat_ring(fx.x, fx.y, fx.z + 0.3 + v * cfg.height * 1.4,
				cfg.radius * (1.0 - v * 0.45), per, color, (now * 0.5 + i * 0.3) % 1, fade)
		end
	end
end

release.unknown = release['self']

--- Draw a release burst.
---@param fx CastFx
---@param color RGBA
---@param cfg table    # settings.data.castfx
---@param u number     # 0..1 through the burst lifetime
---@param now number
---@param segs integer
function M.draw_release(fx, color, cfg, u, now, segs)
	local fn = release[fx.shape] or release.unknown
	fn(fx, color, cfg, u, now, segs)
end

--- The world radius a release should reach: the spell's own AE range where it
--- has one, otherwise a sensible default so single-target and unknown spells
--- still read.
---@param class CastClass
---@param cfg table
---@return number
function M.reach(class, cfg)
	if class.aeRange and class.aeRange > 1 then
		return min(class.aeRange, cfg.maxReach)
	end
	return cfg.radius * 2.5
end

return M
