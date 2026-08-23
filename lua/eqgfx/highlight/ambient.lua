--[[
  highlight/ambient.lua - a randomized but stable idle animation for every NPC
  you can see.

  RANDOM, NOT RANDOMIZED-PER-FRAME
  --------------------------------
  Everything that makes one NPC's effect different from its neighbour's -
  which style it gets, its palette color, phase, speed, radius, tilt - is
  derived from the spawn ID through shapes.hash(). That is the whole trick:
  the crowd looks hand-varied, but any single mob animates smoothly because
  its parameters never change. Rolling math.random() per frame would strobe.

  Each style is deliberately cheap. The per-NPC segment count is the budget
  divided by the crowd size (init.lua does the division), and every style
  honours it, so a busy zone degrades into simpler figures instead of blowing
  the frame.

  Styles are picked from the enabled set in settings, so turning one off
  redistributes the crowd across the rest.
]]

local S = require('eqgfx.highlight.shapes')

local sin, cos, pi = math.sin, math.cos, math.pi
local floor, max, min = math.floor, math.max, math.min
local TAU = pi * 2

local M = {}

--- Style ids in a stable order. The per-spawn pick indexes the *enabled*
--- subset, so a spawn keeps its look as long as the enabled set is unchanged.
---@type string[]
M.STYLES = { 'orbit', 'halo', 'motes', 'spiral', 'bloom' }

M.STYLE_LABELS = {
	orbit  = 'Orbit rings',
	halo   = 'Halo',
	motes  = 'Fireflies',
	spiral = 'Spiral',
	bloom  = 'Ground bloom',
}

M.STYLE_HINTS = {
	orbit  = 'tilted rings circling the body, comet head on each',
	halo   = 'a slowly spinning ring overhead that breathes',
	motes  = 'a handful of short dashes on independent orbits',
	spiral = 'a helix winding up around the body',
	bloom  = 'rings expanding outward from the feet and fading',
}

----------------------------------------------------------------------------
-- Per-spawn parameters. Salts are arbitrary but must stay fixed, or every
-- NPC in the zone changes appearance when this file is edited.
----------------------------------------------------------------------------

---@class AmbientParams
---@field style string
---@field color RGBA
---@field phase number   # 0..1 starting offset around the loop
---@field speed number   # multiplier on the style's base rate
---@field radius number  # multiplier on the configured radius
---@field tilt number    # radians, styles that lean
---@field dir number     # 1 or -1, rotation direction

--- Derive one NPC's stable look.
---@param id integer         # spawn ID
---@param cfg table          # settings.data.ambient
---@param enabled string[]   # style ids currently switched on
---@return AmbientParams
function M.params(id, cfg, enabled)
	local palette = cfg.palette
	return {
		style  = enabled[S.randpick(id, 11, #enabled)],
		color  = palette[S.randpick(id, 23, #palette)],
		phase  = S.rand01(id, 37),
		speed  = S.randrange(id, 41, 1 - cfg.speedJitter, 1 + cfg.speedJitter),
		radius = S.randrange(id, 53, 1 - cfg.sizeJitter, 1 + cfg.sizeJitter),
		tilt   = S.randrange(id, 67, -pi / 3, pi / 3),
		dir    = (S.hash(id, 71) % 2 == 0) and 1 or -1,
	}
end

----------------------------------------------------------------------------
-- Styles
--
-- Each takes the spawn's feet position, its params, the shared config, a
-- segment allowance and the animation clock. `segs` is already the per-NPC
-- share of the frame budget; treat it as the total to spend.
----------------------------------------------------------------------------

local styles = {}

--- Two tilted rings orbiting the torso, counter-rotating, comet head on each.
function styles.orbit(x, y, z, p, cfg, segs, now)
	local r = cfg.radius * p.radius
	local mid = z + cfg.height * 0.5
	local t = now * cfg.speed * p.speed * p.dir
	local per = max(6, floor(segs / 2))
	S.gyro_ring(x, y, mid, r, p.tilt, t * 0.6, per, p.color, (p.phase + t * 0.15) % 1)
	S.gyro_ring(x, y, mid, r * 0.75, p.tilt + pi / 2, -t * 0.85, per,
		p.color, (p.phase + 0.5 - t * 0.21) % 1)
end

--- A ring overhead: slow spin, radius breathing, brightest at the head.
function styles.halo(x, y, z, p, cfg, segs, now)
	local t = now * cfg.speed * p.speed * p.dir
	local breathe = 0.85 + 0.25 * sin(t * 1.1 + p.phase * TAU)
	S.flat_ring(x, y, z + cfg.height * p.radius, cfg.radius * p.radius * breathe,
		max(8, segs), p.color, (p.phase + t * 0.18) % 1)
end

--- Fireflies: short dashes on independent orbits at different radii and
--- heights. The most "alive" of the set and the cheapest per unit of motion.
function styles.motes(x, y, z, p, cfg, segs, now)
	local t = now * cfg.speed * p.speed
	local count = max(2, min(cfg.moteCount, floor(segs / 3)))
	local per = max(2, floor(segs / count))
	for i = 1, count do
		-- Each mote gets its own orbit from the spawn id and its index, so
		-- they never line up into a visible ring.
		local seed = i * 977
		local rr = cfg.radius * p.radius * S.randrange(i, seed, 0.45, 1.15)
		local hh = cfg.height * S.randrange(i, seed + 1, 0.15, 1.0)
		local rate = S.randrange(i, seed + 2, 0.6, 1.8) * p.dir
		local ang = TAU * ((p.phase + i / count + t * 0.13 * rate) % 1)
		local bob = sin(t * 1.7 + i) * cfg.height * 0.08
		S.orbit_dash(x, y, z + hh + bob, rr, ang, 0.55, per, p.color,
			0.55 + 0.45 * (0.5 + 0.5 * sin(t * 2.3 + i * 2.1)))
	end
end

--- A helix winding up the body, twist travelling upward.
function styles.spiral(x, y, z, p, cfg, segs, now)
	local t = now * cfg.speed * p.speed
	local turns = 2
	S.helix(x, y, z + cfg.height * 0.1, cfg.radius * p.radius,
		cfg.height * 0.9, turns, -t * 1.4 * p.dir,
		max(4, floor(segs / turns)), p.color, (p.phase + t * 0.25) % 1)
end

--- Rings expanding out of the feet and fading as they go. Staggered so there
--- is always one mid-flight.
function styles.bloom(x, y, z, p, cfg, segs, now)
	local t = now * cfg.speed * p.speed
	local waves = 2
	local per = max(8, floor(segs / waves))
	for i = 1, waves do
		local u = ((t * 0.5 + p.phase + (i - 1) / waves) % 1)
		-- Ease out: fast at first, then drifting.
		local grow = 1 - (1 - u) * (1 - u)
		S.flat_ring(x, y, z + 0.15 + u * cfg.height * 0.25,
			cfg.radius * p.radius * (0.25 + grow * 1.35),
			per, p.color, nil, (1 - u) * (1 - u))
	end
end

--- Draw one NPC's ambient effect.
---@param x number @param y number @param z number # spawn feet position
---@param p AmbientParams
---@param cfg table       # settings.data.ambient
---@param segs integer    # segment allowance for this spawn
---@param now number      # animation clock, seconds
function M.draw(x, y, z, p, cfg, segs, now)
	local fn = styles[p.style]
	if fn then fn(x, y, z, p, cfg, segs, now) end
end

return M
