--[[
  highlight/markers.lua - the main tank and main assist role markers.

  Two deliberately different silhouettes, so you can tell which is which out
  of the corner of your eye without reading anything:

    tank    counter-rotating great circles on staggered tilts inside a
            breathing ground ring. Slow and heavy. Reads as a shield.
    assist  a double helix climbing out of a fast spinning triangle, twist
            and brightness both flowing upward. Reads as a spear.

  Both are built from shapes.lua, so they share the frame budget with the
  ambient and cast layers.
]]

local S = require('eqgfx.highlight.shapes')

local sin, pi = math.sin, math.pi
local floor, max, min = math.floor, math.max, math.min

local M = {}

-- Tilt, rate and comet rate for each of the tank's orbiting rings. The second
-- runs backwards so the figure never collapses into looking like a single
-- spin. Fixed table, not settings: the *relationship* between the rings is the
-- design; the menu scales the whole thing.
local TANK_RINGS = {
	{ tilt = 0.0,     spin =  0.55, head =  0.20, scale = 1.00 },
	{ tilt =  pi / 3, spin = -0.80, head = -0.27, scale = 0.92 },
	{ tilt = -pi / 3, spin =  1.05, head =  0.33, scale = 0.92 },
}

--- Main tank marker.
---@param x number @param y number @param z number # spawn feet position
---@param style MarkerStyle
---@param now number # animation clock, seconds
function M.tank(x, y, z, style, now)
	local t = now * style.speed
	local r = style.radius
	local mid = z + style.height * 0.45
	local segs = style.segments

	for i = 1, min(style.rings or 3, #TANK_RINGS) do
		local rg = TANK_RINGS[i]
		S.gyro_ring(x, y, mid, r * rg.scale, rg.tilt, t * rg.spin, segs, style.color,
			style.comet and (t * rg.head) % 1 or nil)
		if S.over() then return end
	end

	if style.groundRing then
		-- Breathes between 0.90x and 1.25x.
		local pulse = 0.5 + 0.5 * sin(t * 1.6)
		S.flat_ring(x, y, z + 0.2, r * (0.90 + 0.35 * pulse), segs, style.color, nil)
	end
end

--- Main assist marker.
---@param x number @param y number @param z number # spawn feet position
---@param style MarkerStyle
---@param now number
function M.assist(x, y, z, style, now)
	local t = now * style.speed
	local r = style.radius
	local base = z + 0.3
	local turns = style.turns or 2
	local flow = -t * 2.2                       -- negative: the twist climbs
	local head = style.comet and (t * 0.55) % 1 or nil
	local strandSegs = max(4, floor(style.segments / 2))

	S.helix(x, y, base, r, style.height, turns, flow,      strandSegs, style.color, head)
	S.helix(x, y, base, r, style.height, turns, flow + pi, strandSegs, style.color, head)
	if S.over() then return end

	-- Spinning triangle at the feet, and a smaller counter-spun one capping
	-- the top so the helix terminates instead of trailing off. The cap flashes
	-- on the same period the brightness takes to climb, so they land together.
	if style.groundRing then
		S.polygon(x, y, z + 0.2, r * 1.5, 3, t * 1.9, style.color, 0.85)
	end
	S.polygon(x, y, base + style.height, r * 0.55, 3, -t * 2.6, style.color,
		head and (0.45 + 0.55 * head) or 0.85)
end

return M
