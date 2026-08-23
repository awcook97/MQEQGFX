--[[
  highlight/classanim.lua - a signature animation and color for each of the
  sixteen EverQuest classes.

  Keyed on Spawn.Class.ShortName() (WAR/CLR/PAL/... - the codes in eqlib's
  ClassInfo table), which NPCs carry too, so this drives the ambient layer for
  anything with a class. Spawns whose class can't be read fall back to the
  randomized styles in ambient.lua.

  COLORS
  ------
  Defaults are World of Warcraft's class colors, mapped onto the closest EQ
  analogue. Thirteen of them land exactly:

    Warrior->Warrior  Cleric->Priest  Paladin->Paladin  Ranger->Hunter
    Shadow Knight->Death Knight  Druid->Druid  Monk->Monk  Rogue->Rogue
    Shaman->Shaman  Necromancer->Warlock  Wizard->Mage  Magician->Evoker
    Enchanter->Demon Hunter

  EQ has three classes WoW has no counterpart for - Bard, Beastlord and
  Berserker. Those take WoW's item-quality colors (heirloom gold, uncommon
  green, epic purple) so the whole palette still comes from one source. All of
  them are settings values; override any of them in /hlmenu.

  SHAPES
  ------
  Every class reads differently at a glance, and the shape says something about
  the class rather than being decoration: plate classes get heavy grounded
  figures, casters get orbiting arcane geometry, priests get rising light,
  rogues and monks get fast small motion. They are all built from shapes.lua,
  so they share the frame budget with everything else.

  This lives in Lua rather than the plugin on purpose: it is pure presentation
  and you can retune a class without a rebuild.
]]

local S = require('eqgfx.highlight.shapes')

local sin, cos, pi, abs = math.sin, math.cos, math.pi, math.abs
local floor, max, min = math.floor, math.max, math.min
local TAU = pi * 2

local M = {}

--- Hex -> {r,g,b,a} floats, so the table below can be read against WoW's
--- published values without conversion by hand.
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

--- Default per-class colors. Keys are Spawn.Class.ShortName().
---@type table<string, RGBA>
M.COLORS = {
	WAR = rgb(0xC69B6D),  -- WoW Warrior
	CLR = rgb(0xFFFFFF),  -- WoW Priest
	PAL = rgb(0xF48CBA),  -- WoW Paladin
	RNG = rgb(0xAAD372),  -- WoW Hunter
	SHD = rgb(0xC41E3A),  -- WoW Death Knight
	DRU = rgb(0xFF7C0A),  -- WoW Druid
	MNK = rgb(0x00FF98),  -- WoW Monk
	ROG = rgb(0xFFF468),  -- WoW Rogue
	SHM = rgb(0x0070DD),  -- WoW Shaman
	NEC = rgb(0x8788EE),  -- WoW Warlock
	WIZ = rgb(0x3FC7EB),  -- WoW Mage
	MAG = rgb(0x33937F),  -- WoW Evoker
	ENC = rgb(0xA330C9),  -- WoW Demon Hunter
	-- No WoW class analogue; WoW item-quality colors instead.
	BRD = rgb(0xE6CC80),  -- heirloom gold
	BST = rgb(0x1EFF00),  -- uncommon green
	BER = rgb(0xA335EE),  -- epic purple
	MER = rgb(0xBBBBBB),  -- mercenary: neutral grey, not a WoW color
}

--- Display order for the menu's color list.
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

----------------------------------------------------------------------------
-- Local helpers used by more than one class.
----------------------------------------------------------------------------

--- `n` rays fanning out from a center in the ground plane.
local function rays(x, y, z, r0, r1, n, spin, segs, color, k)
	for i = 0, n - 1 do
		local a = TAU * i / n + spin
		S.streak(x + cos(a) * r0, y + sin(a) * r0, z,
			x + cos(a) * r1, y + sin(a) * r1, z, segs, color, nil, k)
	end
end

--- A vertical bar through the figure - the spine of the "standing" classes.
local function pillar(x, y, z0, z1, segs, color, head, k)
	S.streak(x, y, z0, x, y, z1, segs, color, head, k)
end

----------------------------------------------------------------------------
-- The animations.
--
-- Each takes (x, y, z) at the spawn's feet, a color, the shared ambient draw
-- config (radius/height/speed), a segment allowance, a per-spawn phase 0..1
-- and the animation clock. Phase is what keeps two adjacent warriors from
-- pulsing in lockstep; everything else about a class is identical by design,
-- because the point is to recognize the class.
----------------------------------------------------------------------------

local A = {}

--- Warrior: two heavy ground rings and four cardinal spikes. Grounded, slow,
--- immovable - a shield wall seen from above.
function A.WAR(x, y, z, c, cfg, segs, ph, t)
	local r, per = cfg.radius, max(8, floor(segs * 0.35))
	local spin = t * 0.25 + ph * TAU
	S.flat_ring(x, y, z + 0.2, r * 1.15, per, c, nil, 0.9)
	S.flat_ring(x, y, z + 0.55, r * 0.85, per, c, nil, 0.6)
	rays(x, y, z + 0.35, r * 0.85, r * 1.45, 4, spin, max(2, floor(segs * 0.06)), c, 0.85)
end

--- Cleric: a column of rings rising and fading off the top, under a steady
--- halo. Light lifting off the body.
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

--- Paladin: a halo plus an upright cross - the two bars turning slowly.
function A.PAL(x, y, z, c, cfg, segs, ph, t)
	local r = cfg.radius
	local spin = t * 0.35 + ph * TAU
	local per = max(6, floor(segs * 0.2))
	S.flat_ring(x, y, z + cfg.height, r * 0.6, max(8, floor(segs * 0.3)), c, (t * 0.25) % 1, 0.95)
	pillar(x, y, z + 0.2, z + cfg.height * 1.05, per, c, nil, 0.8)
	-- Crossbar, turning about the pillar.
	local a = spin
	S.streak(x - cos(a) * r, y - sin(a) * r, z + cfg.height * 0.72,
		x + cos(a) * r, y + sin(a) * r, z + cfg.height * 0.72, per, c, nil, 0.8)
end

--- Ranger: two opposed arcs like a drawn bow, with an arrow streaking the gap.
function A.RNG(x, y, z, c, cfg, segs, ph, t)
	local r = cfg.radius
	local mid = z + cfg.height * 0.5
	local spin = t * 0.5 + ph * TAU
	local per = max(6, floor(segs * 0.3))
	S.flat_arc(x, y, mid, r, spin - 0.9, spin + 0.9, per, c, nil, 0.9)
	S.flat_arc(x, y, mid, r, spin + pi - 0.9, spin + pi + 0.9, per, c, nil, 0.9)
	-- The loosed arrow: crosses the figure, resets, crosses again.
	local u = (t * 0.7 + ph) % 1
	local a = spin + pi / 2
	S.streak(x - cos(a) * r, y - sin(a) * r, mid,
		x + cos(a) * r, y + sin(a) * r, mid, max(4, floor(segs * 0.25)), c, u, 1.0)
end

--- Shadow Knight: rings sinking INTO the ground instead of rising, over a
--- slow dark gyro. The inverse of the cleric, deliberately.
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

--- Druid: three interlocking tilted rings, like leaves turning around a stem.
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

--- Monk: four chi motes on a tight fast orbit, bobbing out of phase.
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

--- Bard: concentric rings marching outward on a steady beat - sound leaving
--- the body. Staggered so there are always three in flight.
function A.BRD(x, y, z, c, cfg, segs, ph, t)
	local waves = 3
	local per = max(6, floor(segs / waves))
	for i = 1, waves do
		local u = ((t * 0.55 + ph + (i - 1) / waves) % 1)
		S.flat_ring(x, y, z + 0.3 + u * cfg.height * 0.5,
			cfg.radius * (0.3 + u * 1.5), per, c, nil, (1 - u) * (1 - u))
	end
end

--- Rogue: one thin low fast orbit that fades almost to nothing and comes back.
--- The only class whose effect regularly disappears - that IS the read.
function A.ROG(x, y, z, c, cfg, segs, ph, t)
	local flicker = 0.5 + 0.5 * sin(t * 1.3 + ph * TAU)
	local k = flicker * flicker * flicker          -- long dark, brief bright
	S.flat_ring(x, y, z + 0.35, cfg.radius * 0.8, max(8, floor(segs * 0.6)),
		c, (t * 0.5 + ph) % 1, 0.15 + 0.85 * k)
end

--- Shaman: a totem - a wobbling helix whose radius breathes as it climbs.
function A.SHM(x, y, z, c, cfg, segs, ph, t)
	local turns = 2
	local wob = 0.8 + 0.25 * sin(t * 1.1 + ph * TAU)
	S.helix(x, y, z + 0.25, cfg.radius * wob, cfg.height * 0.9, turns,
		-t * 0.9 + ph * TAU, max(5, floor(segs / (turns + 1))), c, (t * 0.28 + ph) % 1)
	S.flat_ring(x, y, z + 0.2, cfg.radius * 1.1 * wob,
		max(6, floor(segs * 0.25)), c, nil, 0.55)
end

--- Necromancer: rings dragged down into the ground, plus a low tight gyro.
--- Kin to the shadow knight, but faster and smaller - a drain, not a doom.
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

--- Wizard: three fast rings on near-orthogonal tilts - an arcane sphere.
--- The most "geometric" of the set, and the fastest.
function A.WIZ(x, y, z, c, cfg, segs, ph, t)
	local mid = z + cfg.height * 0.5
	local per = max(6, floor(segs / 3))
	local base = ph * TAU
	S.gyro_ring(x, y, mid, cfg.radius, 0,        base + t * 1.3, per, c, (t * 0.4) % 1)
	S.gyro_ring(x, y, mid, cfg.radius, pi / 2,   base - t * 1.1, per, c, (0.33 - t * 0.4) % 1)
	S.gyro_ring(x, y, mid, cfg.radius, pi / 2.0, base + t * 0.9 + pi / 2, per, c, (0.66 + t * 0.4) % 1)
end

--- Magician: a rotating square prism - two offset quads, elemental and blocky.
function A.MAG(x, y, z, c, cfg, segs, ph, t)
	local spin = t * 0.6 + ph * TAU
	local r = cfg.radius
	S.polygon(x, y, z + 0.3, r, 4, spin, c, 0.9)
	S.polygon(x, y, z + cfg.height * 0.85, r * 0.7, 4, -spin + pi / 4, c, 0.9)
	-- Verticals joining the two, so it reads as a solid rather than two rings.
	local per = max(2, floor(segs * 0.06))
	for i = 0, 3 do
		local a0, a1 = TAU * i / 4 + spin, TAU * i / 4 - spin + pi / 4
		S.streak(x + cos(a0) * r, y + sin(a0) * r, z + 0.3,
			x + cos(a1) * r * 0.7, y + sin(a1) * r * 0.7, z + cfg.height * 0.85,
			per, c, nil, 0.55)
	end
end

--- Enchanter: two counter-rotating hexagons that scale in and out against
--- each other - interference, illusion.
function A.ENC(x, y, z, c, cfg, segs, ph, t)
	local mid = z + cfg.height * 0.5
	local pulse = 0.5 + 0.5 * sin(t * 0.9 + ph * TAU)
	S.polygon(x, y, mid, cfg.radius * (0.55 + 0.5 * pulse), 6, t * 0.5 + ph * TAU, c, 0.95)
	S.polygon(x, y, mid, cfg.radius * (1.05 - 0.5 * pulse), 6, -t * 0.7 - ph * TAU, c, 0.75)
end

--- Beastlord: two orbits at different heights - the warder circling with you.
function A.BST(x, y, z, c, cfg, segs, ph, t)
	local per = max(3, floor(segs * 0.2))
	local a1 = TAU * ((ph + t * 0.22) % 1)
	local a2 = TAU * ((ph + 0.5 - t * 0.3) % 1)
	S.orbit_dash(x, y, z + cfg.height * 0.75, cfg.radius * 0.75, a1, 0.7, per, c, 0.95)
	S.orbit_dash(x, y, z + cfg.height * 0.25, cfg.radius * 1.05, a2, 0.7, per, c, 0.8)
	S.flat_ring(x, y, z + 0.2, cfg.radius * 1.05, max(6, floor(segs * 0.3)), c, nil, 0.4)
end

--- Berserker: a ring whose radius jitters - rage. The noise is a fixed sum of
--- sines rather than random(), so it is violent but never strobes.
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

--- Mercenary: a plain slow ring. Deliberately the least interesting thing on
--- screen - mercs are background, not information.
function A.MER(x, y, z, c, cfg, segs, ph, t)
	S.flat_ring(x, y, z + 0.25, cfg.radius * 0.9, max(8, floor(segs * 0.5)),
		c, nil, 0.5)
end

M.ANIMS = A

--- True when this class has an animation.
---@param short string|nil
---@return boolean
function M.has(short) return short ~= nil and A[short] ~= nil end

--- Draw one spawn's class animation.
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
