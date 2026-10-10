local sin, cos, pi = math.sin, math.cos, math.pi
local floor, min, max = math.floor, math.min, math.max
local TAU = pi * 2

local M = { TAU = TAU }

local sink = nil
local budget = math.huge
local used = 0

---@param addline fun(x1:number,y1:number,z1:number,x2:number,y2:number,z2:number,color:integer)
---@param maxLines integer|nil # nil = unlimited
function M.begin_frame(addline, maxLines)
	sink = addline
	budget = maxLines or math.huge
	used = 0
end

---@return integer # segments emitted this frame
function M.used() return used end

---@return boolean
function M.over() return used >= budget end

---@return number
function M.headroom() return max(0, budget - used) end

local function line(x1, y1, z1, x2, y2, z2, color)
	if used >= budget or not sink then return end
	used = used + 1
	sink(x1, y1, z1, x2, y2, z2, color)
end
M.line = line

local argb

---@param fn fun(a:integer,r:integer,g:integer,b:integer): integer
function M.set_packer(fn) argb = fn end

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

---@param t number @param head number
---@return number # 0..1
function M.comet(t, head)
	local d = (t - head) % 1
	if d > 0.5 then d = 1 - d end
	local u = 1 - d * 2
	return 0.18 + 0.82 * u * u * u
end

---@param n integer @param salt integer|nil
---@return integer
local function hash(n, salt)
	local h = (n * 2654435761 + (salt or 0) * 40503) % 4294967296
	h = (h + floor(h / 65536)) % 4294967296
	local hi, lo = floor(h / 65536), h % 65536
	return ((hi * 1103515245) % 65536 * 65536 + lo * 1103515245 + 12345) % 4294967296
end
M.hash = hash

---@param n integer @param salt integer|nil
---@return number
function M.rand01(n, salt)
	return hash(n, salt) / 4294967296
end

---@param n integer @param salt integer @param lo number @param hi number
---@return number
function M.randrange(n, salt, lo, hi)
	return lo + (hi - lo) * M.rand01(n, salt)
end

---@param n integer @param salt integer @param count integer
---@return integer
function M.randpick(n, salt, count)
	return 1 + (hash(n, salt) % count)
end

---@return number, number, number
function M.spin_z(x, y, z, a)
	local s, c = sin(a), cos(a)
	return x * c - y * s, x * s + y * c, z
end

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
			local k = head and M.comet(t - 0.5 / segs, head) or 1
			line(px, py, pz, x, y, z, pack(style, k * (gain or 1)))
		end
		px, py, pz = x, y, z
	end
end

function M.ring(cx, cy, cz, r, ux, uy, uz, vx, vy, vz, segs, style, head, gain)
	M.arc(cx, cy, cz, r, ux, uy, uz, vx, vy, vz, 0, TAU, segs, style, head, gain)
end

function M.flat_ring(cx, cy, cz, r, segs, style, head, gain)
	M.ring(cx, cy, cz, r, 1, 0, 0, 0, 1, 0, segs, style, head, gain)
end

function M.flat_arc(cx, cy, cz, r, a0, a1, segs, style, head, gain)
	M.arc(cx, cy, cz, r, 1, 0, 0, 0, 1, 0, a0, a1, segs, style, head, gain)
end

function M.gyro_ring(cx, cy, cz, r, tilt, spin, segs, style, head, gain)
	local ux, uy, uz = 1, 0, 0
	local vx, vy, vz = 0, cos(tilt), sin(tilt)
	ux, uy, uz = M.spin_z(ux, uy, uz, spin)
	vx, vy, vz = M.spin_z(vx, vy, vz, spin)
	M.ring(cx, cy, cz, r, ux, uy, uz, vx, vy, vz, segs, style, head, gain)
end

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

function M.polygon(cx, cy, cz, r, sides, spin, style, k)
	local px, py
	for i = 0, sides do
		local a = TAU * i / sides + spin
		local x, y = cx + r * cos(a), cy + r * sin(a)
		if i > 0 then line(px, py, cz, x, y, cz, pack(style, k)) end
		px, py = x, y
	end
end

function M.orbit_dash(cx, cy, cz, r, ang, span, segs, style, k)
	M.flat_arc(cx, cy, cz, r, ang - span * 0.5, ang + span * 0.5, segs, style, nil, k)
end

function M.streak(x1, y1, z1, x2, y2, z2, segs, style, head, gain)
	local px, py, pz = x1, y1, z1
	for i = 1, segs do
		local t = i / segs
		local x = x1 + (x2 - x1) * t
		local y = y1 + (y2 - y1) * t
		local z = z1 + (z2 - z1) * t
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
