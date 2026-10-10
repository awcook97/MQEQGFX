local S = require('eqgfx.highlight.shapes')

local sin, pi = math.sin, math.pi
local floor, max, min = math.floor, math.max, math.min

local M = {}

local TANK_RINGS = {
	{ tilt = 0.0,     spin =  0.55, head =  0.20, scale = 1.00 },
	{ tilt =  pi / 3, spin = -0.80, head = -0.27, scale = 0.92 },
	{ tilt = -pi / 3, spin =  1.05, head =  0.33, scale = 0.92 },
}

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
		local pulse = 0.5 + 0.5 * sin(t * 1.6)
		S.flat_ring(x, y, z + 0.2, r * (0.90 + 0.35 * pulse), segs, style.color, nil)
	end
end

---@param x number @param y number @param z number # spawn feet position
---@param style MarkerStyle
---@param now number
function M.assist(x, y, z, style, now)
	local t = now * style.speed
	local r = style.radius
	local base = z + 0.3
	local turns = style.turns or 2
	local flow = -t * 2.2
	local head = style.comet and (t * 0.55) % 1 or nil
	local strandSegs = max(4, floor(style.segments / 2))

	S.helix(x, y, base, r, style.height, turns, flow,      strandSegs, style.color, head)
	S.helix(x, y, base, r, style.height, turns, flow + pi, strandSegs, style.color, head)
	if S.over() then return end

	if style.groundRing then
		S.polygon(x, y, z + 0.2, r * 1.5, 3, t * 1.9, style.color, 0.85)
	end
	S.polygon(x, y, base + style.height, r * 0.55, 3, -t * 2.6, style.color,
		head and (0.45 + 0.55 * head) or 0.85)
end

return M
