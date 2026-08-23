--[[
  highlight/shapes.lua - the line primitives every animation in this feature is
  built from, plus the frame budget that keeps them affordable.

  Nothing here reads a TLO or a setting: callers pass world coordinates in
  MQ/TLO order (x, y, z, z up) and a color, and get lines.

  THE BUDGET
  ----------
  Ambient effects run on every NPC in sight, so the line count is driven by how
  crowded the zone is rather than by anything the script chose. Every primitive
  goes through one emitter that stops accepting lines once the frame's cap is
  reached; callers ask `over()` and bail out of the expensive layers rather
  than trusting themselves to have counted right.

  Note that eqgfx.set_thickness(px) makes the plugin draw 2*px+1 parallel
  copies of each segment, so the real engine cost is the budget times that.
  The cap is in segments, which is what the caller controls.

  COLOR
  -----
  Colors arrive as {r,g,b,a} floats 0..1 (what the settings tree and the menu's
  ColorEdit4 hold) and are packed to ARGB per segment. Shading each segment
  individually is what makes these read as more than wireframe: a bright head
  chasing around a loop costs exactly the same as drawing it flat.
]]

local sin, cos, pi = math.sin, math.cos, math.pi
local floor, min, max = math.floor, math.min, math.max
local TAU = pi * 2

local M = { TAU = TAU }

----------------------------------------------------------------------------
-- Emitter + budget
----------------------------------------------------------------------------

local sink = nil          -- fun(x1,y1,z1,x2,y2,z2,color)
local budget = math.huge
local used = 0

--- Point the primitives at a line sink and start a fresh budget.
---@param addline fun(x1:number,y1:number,z1:number,x2:number,y2:number,z2:number,color:integer)
---@param maxLines integer|nil # nil = unlimited
function M.begin_frame(addline, maxLines)
	sink = addline
	budget = maxLines or math.huge
	used = 0
end

---@return integer # segments emitted this frame
function M.used() return used end

--- True once the frame's cap is reached. Check this before starting an
--- expensive layer, so it drops out whole instead of half-drawn.
---@return boolean
function M.over() return used >= budget end

--- Segments still available this frame.
---@return number
function M.headroom() return max(0, budget - used) end

local function line(x1, y1, z1, x2, y2, z2, color)
	if used >= budget or not sink then return end
	used = used + 1
	sink(x1, y1, z1, x2, y2, z2, color)
end
M.line = line

----------------------------------------------------------------------------
-- Color
----------------------------------------------------------------------------

local argb  -- injected by init.lua so this module stays free of eqgfx

--- Hand in eqgfx.argb once at startup.
---@param fn fun(a:integer,r:integer,g:integer,b:integer): integer
function M.set_packer(fn) argb = fn end

--- Pack an {r,g,b,a} float color to ARGB, scaling alpha by `k` (0..1).
---@param c RGBA
---@param k number|nil # brightness, default 1
---@return integer
local function pack(c, k)
	k = k or 1
	if k < 0 then k = 0 elseif k > 1 then k = 1 end
	return argb(
		floor((c[4] or 1) * 255 * k + 0.5),
		floor(c[1] * 255 + 0.5),
		floor(c[2] * 255 + 0.5),
		floor(c[3] * 255 + 0.5))
end
M.pack = pack

--- Blend two {r,g,b,a} colors; t=0 gives a, t=1 gives b.
---@param a RGBA @param b RGBA @param t number
---@return RGBA
function M.mix(a, b, t)
	if t < 0 then t = 0 elseif t > 1 then t = 1 end
	return {
		a[1] + (b[1] - a[1]) * t,
		a[2] + (b[2] - a[2]) * t,
		a[3] + (b[3] - a[3]) * t,
		(a[4] or 1) + ((b[4] or 1) - (a[4] or 1)) * t,
	}
end

--- Brightness for a point at `t` (0..1 around a loop) given a comet head at
--- `head` (0..1). Falls off both ways from the head and wraps at the seam.
--- Cubed, so the head is tight and the tail is long and dim.
---@param t number @param head number
---@return number # 0..1
function M.comet(t, head)
	local d = (t - head) % 1
	if d > 0.5 then d = 1 - d end
	local u = 1 - d * 2
	return 0.18 + 0.82 * u * u * u
end

----------------------------------------------------------------------------
-- Deterministic per-spawn randomness
--
-- Ambient effects must look random across the crowd but be *stable* for any
-- one spawn - a style or phase that re-rolls every frame is a strobe, not an
-- animation. Everything is derived from the spawn ID through this hash, so it
-- is the same on every frame and on every character watching.
----------------------------------------------------------------------------

--- 32-bit integer hash (xorshift-ish, LuaJIT-safe: stays inside 2^53).
---@param n integer @param salt integer|nil
---@return integer
local function hash(n, salt)
	local h = (n * 2654435761 + (salt or 0) * 40503) % 4294967296
	h = (h + floor(h / 65536)) % 4294967296
	h = (h * 1103515245 + 12345) % 4294967296
	return h
end
M.hash = hash

--- Deterministic 0..1 from a spawn ID and a salt.
---@param n integer @param salt integer|nil
---@return number
function M.rand01(n, salt)
	return hash(n, salt) / 4294967296
end

--- Deterministic value in [lo, hi] from a spawn ID and a salt.
---@param n integer @param salt integer @param lo number @param hi number
---@return number
function M.randrange(n, salt, lo, hi)
	return lo + (hi - lo) * M.rand01(n, salt)
end

--- Deterministic 1..count from a spawn ID and a salt.
---@param n integer @param salt integer @param count integer
---@return integer
function M.randpick(n, salt, count)
	return 1 + (hash(n, salt) % count)
end

----------------------------------------------------------------------------
-- Primitives
----------------------------------------------------------------------------

--- Rotate a vector about the world Z (up) axis.
---@return number, number, number
function M.spin_z(x, y, z, a)
	local s, c = sin(a), cos(a)
	return x * c - y * s, x * s + y * c, z
end

--- A circular arc from `a0` to `a1` radians in the plane spanned by u/v.
--- `head` (0..1, nil for flat) places a comet head along the arc.
---@param style RGBA
function M.arc(cx, cy, cz, r, ux, uy, uz, vx, vy, vz, a0, a1, segs, style, head, gain)
	local px, py, pz
	for i = 0, segs do
		local t = i / segs
		local ang = a0 + (a1 - a0) * t
		local a, b = r * cos(ang), r * sin(ang)
		local x = cx + a * ux + b * vx
		local y = cy + a * uy + b * vy
		local z = cz + a * uz + b * vz
		if i > 0 then
			-- Shade from the segment midpoint so the gradient reads evenly.
			local k = head and M.comet(t - 0.5 / segs, head) or 1
			line(px, py, pz, x, y, z, pack(style, k * (gain or 1)))
		end
		px, py, pz = x, y, z
	end
end

--- A closed circle in the plane spanned by u/v.
function M.ring(cx, cy, cz, r, ux, uy, uz, vx, vy, vz, segs, style, head, gain)
	M.arc(cx, cy, cz, r, ux, uy, uz, vx, vy, vz, 0, TAU, segs, style, head, gain)
end

--- A flat circle in the ground plane.
function M.flat_ring(cx, cy, cz, r, segs, style, head, gain)
	M.ring(cx, cy, cz, r, 1, 0, 0, 0, 1, 0, segs, style, head, gain)
end

--- A flat arc in the ground plane, from a0 to a1 radians.
function M.flat_arc(cx, cy, cz, r, a0, a1, segs, style, head, gain)
	M.arc(cx, cy, cz, r, 1, 0, 0, 0, 1, 0, a0, a1, segs, style, head, gain)
end

--- A tilted, spinning great circle: `tilt` leans it out of the ground plane,
--- `spin` turns the whole plane about the vertical.
function M.gyro_ring(cx, cy, cz, r, tilt, spin, segs, style, head, gain)
	local ux, uy, uz = 1, 0, 0
	local vx, vy, vz = 0, cos(tilt), sin(tilt)
	ux, uy, uz = M.spin_z(ux, uy, uz, spin)
	vx, vy, vz = M.spin_z(vx, vy, vz, spin)
	M.ring(cx, cy, cz, r, ux, uy, uz, vx, vy, vz, segs, style, head, gain)
end

--- A helical strand climbing from cz to cz+height over `turns` revolutions.
---@param phase number # radians; animate to make the twist travel
function M.helix(cx, cy, cz, r, height, turns, phase, segs, style, head, gain)
	local px, py, pz
	local steps = max(1, segs * turns)
	for i = 0, steps do
		local t = i / steps
		local a = TAU * turns * t + phase
		local x, y, z = cx + r * cos(a), cy + r * sin(a), cz + height * t
		if i > 0 then
			line(px, py, pz, x, y, z, pack(style, (head and M.comet(t, head) or 1) * (gain or 1)))
		end
		px, py, pz = x, y, z
	end
end

--- A regular n-gon lying flat, rotated by `spin`.
function M.polygon(cx, cy, cz, r, sides, spin, style, k)
	local px, py
	for i = 0, sides do
		local a = TAU * i / sides + spin
		local x, y = cx + r * cos(a), cy + r * sin(a)
		if i > 0 then line(px, py, cz, x, y, cz, pack(style, k)) end
		px, py = x, y
	end
end

--- A single short dash riding a circular orbit - the building block for the
--- firefly-style ambient effect. `span` is the dash length in radians.
function M.orbit_dash(cx, cy, cz, r, ang, span, segs, style, k)
	M.flat_arc(cx, cy, cz, r, ang - span * 0.5, ang + span * 0.5, segs, style, nil, k)
end

--- A straight run of `segs` segments from A to B, shaded along its length so
--- a bright head travels the line. `head` is 0..1 along A->B.
function M.streak(x1, y1, z1, x2, y2, z2, segs, style, head, gain)
	local px, py, pz = x1, y1, z1
	for i = 1, segs do
		local t = i / segs
		local x = x1 + (x2 - x1) * t
		local y = y1 + (y2 - y1) * t
		local z = z1 + (z2 - z1) * t
		-- Not a loop, so fall off linearly behind the head rather than wrapping.
		local k = 1
		if head then
			local d = head - (t - 0.5 / segs)
			k = (d < 0) and 0.05 or max(0.05, 1 - d * 4)
		end
		line(px, py, pz, x, y, z, pack(style, k * (gain or 1)))
		px, py, pz = x, y, z
	end
end

return M
