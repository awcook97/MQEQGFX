local S = require('eqgfx.highlight.shapes')

local sin, cos, pi, abs = math.sin, math.cos, math.pi, math.abs
local floor, max, min = math.floor, math.max, math.min
local TAU = pi * 2

local M = {}

---@param hex integer @param a number|nil
---@return RGBA
local function rgb(hex, a)
	return {
		floor(hex / 65536) % 256 / 255,
		floor(hex / 256) % 256 / 255,
		hex % 256 / 255,
		a or 0.65,
	}
end
M.rgb = rgb

---@type table<string, RGBA>
M.COLORS = {
	WAR = rgb(0xC69B6D),
	CLR = rgb(0xFFFFFF),
	PAL = rgb(0xF48CBA),
	RNG = rgb(0xAAD372),
	SHD = rgb(0xC41E3A),
	DRU = rgb(0xFF7C0A),
	MNK = rgb(0x00FF98),
	ROG = rgb(0xFFF468),
	SHM = rgb(0x0070DD),
	NEC = rgb(0x8788EE),
	WIZ = rgb(0x3FC7EB),
	MAG = rgb(0x33937F),
	ENC = rgb(0xA330C9),
	BRD = rgb(0xE6CC80),
	BST = rgb(0x1EFF00),
	BER = rgb(0xA335EE),
	MER = rgb(0xBBBBBB),
}

---@type string[]
M.ORDER = { 'WAR', 'CLR', 'PAL', 'RNG', 'SHD', 'DRU', 'MNK', 'BRD',
            'ROG', 'SHM', 'NEC', 'WIZ', 'MAG', 'ENC', 'BST', 'BER', 'MER' }

---@type table<string, string>
M.NAMES = {
	WAR = 'Warrior', CLR = 'Cleric', PAL = 'Paladin', RNG = 'Ranger',
	SHD = 'Shadow Knight', DRU = 'Druid', MNK = 'Monk', BRD = 'Bard',
	ROG = 'Rogue', SHM = 'Shaman', NEC = 'Necromancer', WIZ = 'Wizard',
	MAG = 'Magician', ENC = 'Enchanter', BST = 'Beastlord', BER = 'Berserker',
	MER = 'Mercenary',
}

local function rays(x, y, z, r0, r1, n, spin, segs, color, k)
	for i = 0, n - 1 do
		local a = TAU * i / n + spin
		S.streak(x + cos(a) * r0, y + sin(a) * r0, z,
			x + cos(a) * r1, y + sin(a) * r1, z, segs, color, nil, k)
	end
end

local function pillar(x, y, z0, z1, segs, color, head, k)
	S.streak(x, y, z0, x, y, z1, segs, color, head, k)
end

local A = {}

function A.WAR(x, y, z, c, cfg, segs, ph, t)
	local r, per = cfg.radius, max(8, floor(segs * 0.35))
	local spin = t * 0.25 + ph * TAU
	S.flat_ring(x, y, z + 0.2, r * 1.15, per, c, nil, 0.9)
	S.flat_ring(x, y, z + 0.55, r * 0.85, per, c, nil, 0.6)
	rays(x, y, z + 0.35, r * 0.85, r * 1.45, 4, spin, max(2, floor(segs * 0.06)), c, 0.85)
end

function A.CLR(x, y, z, c, cfg, segs, ph, t)
	local per = max(8, floor(segs * 0.28))
	for i = 1, 2 do
		local u = ((t * 0.35 + ph + (i - 1) * 0.5) % 1)
		S.flat_ring(x, y, z + 0.3 + u * cfg.height, cfg.radius * (1 - u * 0.45),
			per, c, nil, (1 - u) * (1 - u))
	end
	S.flat_ring(x, y, z + cfg.height * 1.05, cfg.radius * 0.55,
		max(8, floor(segs * 0.3)), c, (t * 0.2 + ph) % 1, 0.9)
end

function A.PAL(x, y, z, c, cfg, segs, ph, t)
	local r = cfg.radius
	local spin = t * 0.35 + ph * TAU
	local per = max(6, floor(segs * 0.2))
	S.flat_ring(x, y, z + cfg.height, r * 0.6, max(8, floor(segs * 0.3)), c, (t * 0.25) % 1, 0.95)
	pillar(x, y, z + 0.2, z + cfg.height * 1.05, per, c, nil, 0.8)
	local a = spin
	S.streak(x - cos(a) * r, y - sin(a) * r, z + cfg.height * 0.72,
		x + cos(a) * r, y + sin(a) * r, z + cfg.height * 0.72, per, c, nil, 0.8)
end

function A.RNG(x, y, z, c, cfg, segs, ph, t)
	local r = cfg.radius
	local mid = z + cfg.height * 0.5
	local spin = t * 0.5 + ph * TAU
	local per = max(6, floor(segs * 0.3))
	S.flat_arc(x, y, mid, r, spin - 0.9, spin + 0.9, per, c, nil, 0.9)
	S.flat_arc(x, y, mid, r, spin + pi - 0.9, spin + pi + 0.9, per, c, nil, 0.9)
	local u = (t * 0.7 + ph) % 1
	local a = spin + pi / 2
	S.streak(x - cos(a) * r, y - sin(a) * r, mid,
		x + cos(a) * r, y + sin(a) * r, mid, max(4, floor(segs * 0.25)), c, u, 1.0)
end

function A.SHD(x, y, z, c, cfg, segs, ph, t)
	local per = max(8, floor(segs * 0.3))
	for i = 1, 2 do
		local u = ((t * 0.3 + ph + (i - 1) * 0.5) % 1)
		S.flat_ring(x, y, z + cfg.height * (1 - u) * 0.9 + 0.2,
			cfg.radius * (0.55 + u * 0.6), per, c, nil, u * u)
	end
	S.gyro_ring(x, y, z + cfg.height * 0.45, cfg.radius * 0.9, pi / 6,
		-t * 0.5 + ph * TAU, max(8, floor(segs * 0.35)), c, (t * 0.18) % 1, 0.85)
end

function A.DRU(x, y, z, c, cfg, segs, ph, t)
	local mid = z + cfg.height * 0.45
	local per = max(6, floor(segs / 3))
	for i = 0, 2 do
		local tilt = pi / 3 + i * 0.4
		S.gyro_ring(x, y, mid, cfg.radius * (0.75 + i * 0.12), tilt,
			t * (0.4 + i * 0.15) + ph * TAU + i * TAU / 3, per, c,
			(t * 0.22 + i / 3) % 1)
	end
end

function A.MNK(x, y, z, c, cfg, segs, ph, t)
	local n = 4
	local per = max(2, floor(segs / n))
	for i = 1, n do
		local a = TAU * ((ph + i / n + t * 0.32) % 1)
		local h = cfg.height * (0.35 + 0.3 * sin(t * 2.2 + i * 1.7))
		S.orbit_dash(x, y, z + h, cfg.radius * 0.7, a, 0.5, per, c,
			0.5 + 0.5 * (0.5 + 0.5 * sin(t * 3.1 + i)))
	end
end

function A.BRD(x, y, z, c, cfg, segs, ph, t)
	local waves = 3
	local per = max(6, floor(segs / waves))
	for i = 1, waves do
		local u = ((t * 0.55 + ph + (i - 1) / waves) % 1)
		S.flat_ring(x, y, z + 0.3 + u * cfg.height * 0.5,
			cfg.radius * (0.3 + u * 1.5), per, c, nil, (1 - u) * (1 - u))
	end
end

function A.ROG(x, y, z, c, cfg, segs, ph, t)
	local flicker = 0.5 + 0.5 * sin(t * 1.3 + ph * TAU)
	local k = flicker * flicker * flicker
	S.flat_ring(x, y, z + 0.35, cfg.radius * 0.8, max(8, floor(segs * 0.6)),
		c, (t * 0.5 + ph) % 1, 0.15 + 0.85 * k)
end

function A.SHM(x, y, z, c, cfg, segs, ph, t)
	local turns = 2
	local wob = 0.8 + 0.25 * sin(t * 1.1 + ph * TAU)
	S.helix(x, y, z + 0.25, cfg.radius * wob, cfg.height * 0.9, turns,
		-t * 0.9 + ph * TAU, max(5, floor(segs / (turns + 1))), c, (t * 0.28 + ph) % 1)
	S.flat_ring(x, y, z + 0.2, cfg.radius * 1.1 * wob,
		max(6, floor(segs * 0.25)), c, nil, 0.55)
end

function A.NEC(x, y, z, c, cfg, segs, ph, t)
	local per = max(6, floor(segs * 0.28))
	for i = 1, 2 do
		local u = ((t * 0.5 + ph + (i - 1) * 0.5) % 1)
		S.flat_ring(x, y, z + cfg.height * 0.8 * (1 - u) + 0.15,
			cfg.radius * (0.4 + (1 - u) * 0.5), per, c, nil, u * u * 0.9)
	end
	S.gyro_ring(x, y, z + 0.5, cfg.radius * 0.7, pi / 2.2, t * 1.2 + ph * TAU,
		max(6, floor(segs * 0.3)), c, (t * 0.35) % 1, 0.8)
end

function A.WIZ(x, y, z, c, cfg, segs, ph, t)
	local mid = z + cfg.height * 0.5
	local per = max(6, floor(segs / 3))
	local base = ph * TAU
	S.gyro_ring(x, y, mid, cfg.radius, 0,        base + t * 1.3, per, c, (t * 0.4) % 1)
	S.gyro_ring(x, y, mid, cfg.radius, pi / 2,   base - t * 1.1, per, c, (0.33 - t * 0.4) % 1)
	S.gyro_ring(x, y, mid, cfg.radius, pi / 2.0, base + t * 0.9 + pi / 2, per, c, (0.66 + t * 0.4) % 1)
end

function A.MAG(x, y, z, c, cfg, segs, ph, t)
	local spin = t * 0.6 + ph * TAU
	local r = cfg.radius
	S.polygon(x, y, z + 0.3, r, 4, spin, c, 0.9)
	S.polygon(x, y, z + cfg.height * 0.85, r * 0.7, 4, -spin + pi / 4, c, 0.9)
	local per = max(2, floor(segs * 0.06))
	for i = 0, 3 do
		local a0, a1 = TAU * i / 4 + spin, TAU * i / 4 - spin + pi / 4
		S.streak(x + cos(a0) * r, y + sin(a0) * r, z + 0.3,
			x + cos(a1) * r * 0.7, y + sin(a1) * r * 0.7, z + cfg.height * 0.85,
			per, c, nil, 0.55)
	end
end

function A.ENC(x, y, z, c, cfg, segs, ph, t)
	local mid = z + cfg.height * 0.5
	local pulse = 0.5 + 0.5 * sin(t * 0.9 + ph * TAU)
	S.polygon(x, y, mid, cfg.radius * (0.55 + 0.5 * pulse), 6, t * 0.5 + ph * TAU, c, 0.95)
	S.polygon(x, y, mid, cfg.radius * (1.05 - 0.5 * pulse), 6, -t * 0.7 - ph * TAU, c, 0.75)
end

function A.BST(x, y, z, c, cfg, segs, ph, t)
	local per = max(3, floor(segs * 0.2))
	local a1 = TAU * ((ph + t * 0.22) % 1)
	local a2 = TAU * ((ph + 0.5 - t * 0.3) % 1)
	S.orbit_dash(x, y, z + cfg.height * 0.75, cfg.radius * 0.75, a1, 0.7, per, c, 0.95)
	S.orbit_dash(x, y, z + cfg.height * 0.25, cfg.radius * 1.05, a2, 0.7, per, c, 0.8)
	S.flat_ring(x, y, z + 0.2, cfg.radius * 1.05, max(6, floor(segs * 0.3)), c, nil, 0.4)
end

function A.BER(x, y, z, c, cfg, segs, ph, t)
	local segsN = max(10, floor(segs * 0.7))
	local px, py
	local base = z + cfg.height * 0.35
	for i = 0, segsN do
		local u = i / segsN
		local a = TAU * u
		local n = sin(a * 5 + t * 6 + ph * TAU) * 0.18 + sin(a * 9 - t * 4) * 0.10
		local r = cfg.radius * (1 + n)
		local nx, ny = x + cos(a) * r, y + sin(a) * r
		if i > 0 then
			S.line(px, py, base, nx, ny, base, S.pack(c, 0.55 + 0.45 * abs(n) * 4))
		end
		px, py = nx, ny
	end
	rays(x, y, base, cfg.radius * 1.15, cfg.radius * 1.5, 3,
		t * 2.2 + ph * TAU, max(2, floor(segs * 0.05)), c, 0.9)
end

function A.MER(x, y, z, c, cfg, segs, ph, t)
	S.flat_ring(x, y, z + 0.25, cfg.radius * 0.9, max(8, floor(segs * 0.5)),
		c, nil, 0.5)
end

M.ANIMS = A

---@param short string|nil
---@return boolean
function M.has(short) return short ~= nil and A[short] ~= nil end

---@param short string   # Spawn.Class.ShortName()
---@param x number @param y number @param z number
---@param color RGBA
---@param cfg table      # ambient draw config (radius/height/speed)
---@param segs integer
---@param phase number   # 0..1, per-spawn so neighbours don't pulse in step
---@param now number
function M.draw(short, x, y, z, color, cfg, segs, phase, now)
	local fn = A[short]
	if fn then fn(x, y, z, color, cfg, segs, phase, now * cfg.speed) end
end

return M
