local mq = require('mq')
local S  = require('eqgfx.highlight.shapes')
local core = require('eqgfx.core._types')

local sin, cos, pi = math.sin, math.cos, math.pi
local floor, max, min = math.floor, math.max, math.min
local TAU = pi * 2

local M = {}

---@type fun(spellID: integer): SpellGeom|nil
local geom_source = function() return nil end

---@param fn fun(spellID: integer): SpellGeom|nil
function M.set_geom_source(fn) geom_source = fn end

---@alias CastShape '"beam"'|'"cone"'|'"casterArea"'|'"targetArea"'|'"single"'|'"self"'|'"unknown"'

local TT = core.TargetType

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

---@param x number
---@param y number
---@param z number # caster feet
---@param class CastClass
---@param color RGBA
---@param cfg table # settings.data.castfx
---@param pct number # 0..1 cast progress
---@param now number
---@param segs integer # segment allowance
function M.draw_charge(x, y, z, class, color, cfg, pct, now, segs)
	local r = cfg.radius
	local t = now * (1 + pct * 2)
	local mid = z + cfg.height * 0.5

	local collapse = r * (1.6 - 1.1 * pct)
	S.flat_ring(x, y, z + 0.25, collapse, max(8, floor(segs * 0.4)), color,
		(t * 0.3) % 1, 0.5 + 0.5 * pct)

	local per = max(6, floor(segs * 0.3))
	S.gyro_ring(x, y, mid, collapse * 0.6, pi / 5, t * 0.9, per, color, (t * 0.25) % 1, 0.8)
	S.gyro_ring(x, y, mid, collapse * 0.6, -pi / 5, -t * 1.15, per, color, (0.5 - t * 0.25) % 1, 0.8)
end

function M.draw_fizzle(x, y, z, color, cfg, age, life, now, segs)
	local u = min(1, age / life)
	local fade = (1 - u) * (1 - u)
	local r = cfg.radius * (0.5 + u * 1.4)
	local wob = 1 + 0.12 * sin(now * 9 + u * 6)
	S.flat_ring(x, y, z + 0.25 + u * 1.5, r * wob, max(8, floor(segs * 0.5)),
		color, nil, fade * 0.8)
end

local release = {}

function release.casterArea(fx, color, cfg, u, now, segs)
	local reach = fx.reach
	local fade = (1 - u) * (1 - u)
	local per = max(10, floor(segs * 0.5))
	local ease = 1 - (1 - u) * (1 - u) * (1 - u)
	S.flat_ring(fx.x, fx.y, fx.z + 0.3 + ease * 1.2, reach * ease, per, color, nil, fade)
	if u > 0.15 then
		local u2 = (u - 0.15) / 0.85
		local e2 = 1 - (1 - u2) * (1 - u2) * (1 - u2)
		S.flat_ring(fx.x, fx.y, fx.z + 0.3, reach * e2 * 0.8, per, color, nil, fade * 0.5)
	end
end

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

function release.cone(fx, color, cfg, u, now, segs)
	local reach = fx.reach
	local fade = (1 - u) * (1 - u)
	local a0 = fx.heading + math.rad(fx.coneStart)
	local a1 = fx.heading + math.rad(fx.coneEnd)
	local bands = 3
	local per = max(4, floor(segs / (bands + 1)))
	for i = 1, bands do
		local lag = (i - 1) * 0.12
		local v = min(1, max(0, (u - lag) / (1 - lag)))
		if v > 0 then
			local ease = 1 - (1 - v) * (1 - v)
			S.flat_arc(fx.x, fx.y, fx.z + 0.4 + i * 0.35,
				reach * ease * (0.55 + 0.15 * i), a0, a1, per, color, nil, fade)
		end
	end
	local ease = 1 - (1 - u) * (1 - u)
	for _, a in ipairs({ a0, a1 }) do
		S.streak(fx.x, fx.y, fx.z + 0.4,
			fx.x + cos(a) * reach * ease, fx.y + sin(a) * reach * ease, fx.z + 0.4,
			per, color, nil, fade * 0.8)
	end
end

function release.beam(fx, color, cfg, u, now, segs)
	local reach = fx.reach
	local fade = 1 - u
	local a = fx.heading
	S.streak(fx.x, fx.y, fx.z + 1.0,
		fx.x + cos(a) * reach, fx.y + sin(a) * reach, fx.z + 1.0,
		max(8, floor(segs * 0.6)), color, u, fade)
	local hx, hy = fx.x + cos(a) * reach * u, fx.y + sin(a) * reach * u
	S.gyro_ring(hx, hy, fx.z + 1.0, cfg.radius * 0.5, pi / 2, a, max(6, floor(segs * 0.25)),
		color, nil, fade)
end

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
