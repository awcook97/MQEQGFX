--[[
  highlight/menu.lua - the in-game customization window (/hlmenu).

  Every widget writes straight into settings.data and marks it dirty; the main
  loop debounce-saves through the shared scoped store.

  The Status section is deliberately at the top: the markers only draw on
  spawns holding a group/raid role, and "no role assigned" looks exactly like
  "broken" from the outside. This shows you which it is.
]]

local ImGui = require('ImGui')

local max = math.max
local types = require('eqgfx.highlight._types')
local settings = require('eqgfx.highlight.settings')
local ambient = require('eqgfx.highlight.ambient')
local classanim = require('eqgfx.highlight.classanim')

local M = { open = false }

--- Live status handed in by init.lua each frame, so the menu can report what
--- the role lookups actually resolved to without repeating them.
---@class HighlightStatus
---@field tankID integer|nil
---@field tankName string|nil
---@field assistID integer|nil
---@field assistName string|nil
---@field maTargetID integer|nil
---@field maTargetName string|nil
---@field litCount integer      # models currently tinted
---@field lineCount integer     # world segments emitted last frame
---@field budget integer        # the cap those were drawn against
---@field ambientCount integer  # spawns given an ambient effect
---@field castCount integer     # cast effects drawn
---@field castFeed boolean|nil  # true when the plugin cast feed is live
M.status = { litCount = 0, lineCount = 0, budget = 0, ambientCount = 0, castCount = 0 }

--- Show/hide the settings window (the /hlmenu bind).
function M.toggle()
	M.open = not M.open
end

local function mark()
	settings.mark_dirty()
end

--- Checkbox bound to t[k]; marks the settings dirty on change.
local function check(label, t, k)
	local v, pressed = ImGui.Checkbox(label, t[k])
	if pressed then
		t[k] = v
		mark()
	end
end

--- Integer slider bound to t[k]; marks dirty on change.
local function slideri(label, t, k, lo, hi)
	local v, changed = ImGui.SliderInt(label, t[k], lo, hi)
	if changed then
		t[k] = v
		mark()
	end
end

--- Float slider bound to t[k]; marks dirty on change.
local function sliderf(label, t, k, lo, hi, fmt)
	local v, changed = ImGui.SliderFloat(label, t[k], lo, hi, fmt or '%.1f')
	if changed then
		t[k] = v
		mark()
	end
end

--- Keep colors as plain number tables (pickle-safe) regardless of what the
--- binding hands back (table vs ImVec4).
local function to_color(colorVal)
	if type(colorVal) == 'table' then
		return { colorVal[1], colorVal[2], colorVal[3], colorVal[4] }
	end
	return { colorVal.x, colorVal.y, colorVal.z, colorVal.w }
end

--- Color picker bound to t[k] ({r,g,b,a} floats); marks dirty on change.
local function color(label, t, k)
	local colorVal, changed = ImGui.ColorEdit4(label, t[k])
	if changed then
		t[k] = to_color(colorVal)
		mark()
	end
end

--- A greyed-out one-liner under the widget above it.
local function hint(text)
	ImGui.TextDisabled('(' .. text .. ')')
end

--- One "Tank: Fippy (id 1234)" / "Tank: none assigned" line.
local function role_line(label, id, name)
	if id then
		ImGui.TextColored(0.5, 1.0, 0.5, 1.0, string.format('%s: %s (id %d)', label, name or '?', id))
	else
		ImGui.TextColored(1.0, 0.7, 0.3, 1.0, label .. ': none assigned')
	end
end

--- The marker style block, shared by the tank and assist tabs.
---@param style MarkerStyle
---@param extra fun()|nil  # per-marker widgets (ring count / helix turns)
local function marker_style(style, extra)
	color('Color', style, 'color')
	sliderf('Radius', style, 'radius', 0.5, 15.0, '%.1f')
	sliderf('Height', style, 'height', 0.0, 25.0, '%.1f')
	sliderf('Speed', style, 'speed', 0.0, 4.0, '%.2f x')
	slideri('Segments', style, 'segments', 6, 64)
	hint('line segments per loop - the cost knob')
	if extra then extra() end
	check('Comet head', style, 'comet')
	hint('chase a bright point around each loop instead of drawing it flat')
	check('Ground ring', style, 'groundRing')
end

--- Render the settings window (call every frame from the ImGui callback;
--- no-op while closed).
function M.draw()
	if not M.open then
		return
	end
	local cfg = settings.data or {}
	local st = M.status

	ImGui.SetNextWindowSize(430, 540, ImGuiCond.FirstUseEver)
	local open, show = ImGui.Begin('EQGFX Highlight', M.open)
	M.open = open
	if show then
		ImGui.Text('Save for:')
		ImGui.SameLine()
		if ImGui.RadioButton('This character', settings.scope == 'char') then
			settings.set_scope('char')
		end
		ImGui.SameLine()
		if ImGui.RadioButton('This server', settings.scope == 'server') then
			settings.set_scope('server')
		end
		ImGui.SameLine()
		if ImGui.RadioButton('All characters', settings.scope == 'global') then
			settings.set_scope('global')
		end
		ImGui.Separator()

		check('Enabled', cfg, 'enabled')
		ImGui.Separator()

		if ImGui.CollapsingHeader('Status', ImGuiTreeNodeFlags.DefaultOpen) then
			role_line('Main tank', st.tankID, st.tankName)
			role_line('Main assist', st.assistID, st.assistName)
			role_line('MA target', st.maTargetID, st.maTargetName)
			ImGui.Text(string.format('Tinted %d   Ambient %d   Casts %d',
				st.litCount or 0, st.ambientCount or 0, st.castCount or 0))

			-- The budget bar is the thing to watch: pegged at 100%% means the
			-- ambient layer is being thinned to fit.
			local used, cap = st.lineCount or 0, max(1, st.budget or 1)
			local frac = used / cap
			if frac > 0.98 then
				ImGui.TextColored(1.0, 0.7, 0.3, 1.0,
					string.format('Segments: %d / %d (budget full)', used, cap))
			else
				ImGui.Text(string.format('Segments: %d / %d', used, cap))
			end
			ImGui.ProgressBar(math.min(1, frac), -1, 0)

			if st.castFeed == false then
				ImGui.TextDisabled('cast feed: chat fallback (no spell IDs on some casts)')
			end
			if not st.tankID and not st.assistID then
				ImGui.TextColored(1.0, 0.7, 0.3, 1.0, 'No group roles set - markers have nothing to draw on.')
				hint('assign roles in the group window, or tick Test mode below')
			end
			check('Test mode (draw both markers on me)', cfg.markers, 'testMode')
		end

		if ImGui.CollapsingHeader('Model tinting', ImGuiTreeNodeFlags.DefaultOpen) then
			check('Recolor models', cfg.tints, 'enabled')
			check('My target', cfg.tints, 'myTarget')
			check("Main assist's target", cfg.tints, 'assistTarget')
			hint('lit whether or not it is also your target')

			ImGui.Spacing()
			ImGui.Text('How:')
			for _, m in ipairs(types.Modes) do
				if ImGui.RadioButton(m.label, cfg.tints.mode == m.value) then
					cfg.tints.mode = m.value
					mark()
				end
				if ImGui.IsItemHovered() then
					ImGui.SetTooltip(m.hint)
				end
			end

			ImGui.Spacing()
			for _, row in ipairs(types.RoleRows) do
				color(row.label, cfg.colors, row.key)
				if ImGui.IsItemHovered() then
					ImGui.SetTooltip(row.hint)
				end
			end
			ImGui.TextDisabled('EQ gives the client no "will attack me" flag, so a')
			ImGui.TextDisabled('KOS mob standing idle still reads as NPC (idle).')
		end

		if ImGui.CollapsingHeader('Role markers') then
			check('Draw markers', cfg.markers, 'enabled')
			check('Main tank', cfg.markers, 'mainTank')
			check('Main assist', cfg.markers, 'mainAssist')
			slideri('Thickness', cfg.markers, 'thickness', 1, 6)
			hint('half-width in pixels, applied while the markers draw')

			if ImGui.BeginTabBar('hl_marker_tabs') then
				if ImGui.BeginTabItem('Tank') then
					ImGui.TextDisabled('counter-rotating great circles - reads as a shield')
					marker_style(cfg.tank, function()
						slideri('Rings', cfg.tank, 'rings', 1, 3)
					end)
					ImGui.EndTabItem()
				end
				if ImGui.BeginTabItem('Assist') then
					ImGui.TextDisabled('double helix over a spinning triangle - reads as a spear')
					marker_style(cfg.assist, function()
						slideri('Helix turns', cfg.assist, 'turns', 1, 5)
					end)
					ImGui.EndTabItem()
				end
				ImGui.EndTabBar()
			end
		end

		if ImGui.CollapsingHeader('Ambient NPC animation') then
			check('Animate nearby spawns', cfg.ambient, 'enabled')
			hint('the expensive layer - watch the segment bar above')

			ImGui.Spacing()
			ImGui.Text('Style source:')
			if ImGui.RadioButton('By class', cfg.ambient.mode == 'class') then
				cfg.ambient.mode = 'class'; mark()
			end
			if ImGui.IsItemHovered() then
				ImGui.SetTooltip('each spawn animates as its class; classless spawns fall back to a random style')
			end
			ImGui.SameLine()
			if ImGui.RadioButton('Random', cfg.ambient.mode == 'random') then
				cfg.ambient.mode = 'random'; mark()
			end
			if ImGui.IsItemHovered() then
				ImGui.SetTooltip('every spawn gets a randomized style from the enabled set')
			end

			ImGui.Spacing()
			slideri('Scan radius', cfg.ambient, 'radius', 20, 400)
			slideri('Max spawns', cfg.ambient, 'maxSpawns', 1, 60)
			check('Nearest first when over the cap', cfg.ambient, 'nearestFirst')
			check('Line of sight only', cfg.ambient, 'losOnly')
			check('Players too, not just NPCs', cfg.ambient, 'includePCs')
			check('Fade with distance', cfg.ambient, 'fadeFar')

			ImGui.Spacing()
			sliderf('Effect radius', cfg.ambient, 'radius3d', 0.5, 12.0, '%.1f')
			sliderf('Effect height', cfg.ambient, 'height', 0.5, 20.0, '%.1f')
			sliderf('Speed', cfg.ambient, 'speed', 0.0, 4.0, '%.2f x')
			slideri('Segments per spawn', cfg.ambient, 'segsPerNpc', 6, 60)
			hint('an upper bound - the budget can hand out fewer')

			if cfg.ambient.mode == 'class' then
				if ImGui.CollapsingHeader('Class colors') then
					check('Use class colors', cfg.ambient, 'classColors')
					hint('off = the palette below, as in random mode')
					ImGui.TextDisabled('Defaults are World of Warcraft class colors mapped')
					ImGui.TextDisabled('onto the closest EQ class. Bard, Beastlord and')
					ImGui.TextDisabled('Berserker have no WoW analogue and use its item')
					ImGui.TextDisabled('quality colors instead.')
					ImGui.Spacing()
					for _, short in ipairs(classanim.ORDER) do
						color(classanim.NAMES[short] or short, cfg.ambient.classColorTable, short)
					end
				end
			else
				if ImGui.CollapsingHeader('Random styles') then
					for _, id in ipairs(ambient.STYLES) do
						check(ambient.STYLE_LABELS[id] or id, cfg.ambient.styles, id)
						if ImGui.IsItemHovered() then
							ImGui.SetTooltip(ambient.STYLE_HINTS[id] or '')
						end
					end
					if cfg.ambient.styles.motes then
						slideri('Fireflies per spawn', cfg.ambient, 'moteCount', 2, 10)
					end
					ImGui.Spacing()
					sliderf('Speed variation', cfg.ambient, 'speedJitter', 0.0, 0.9, '+/- %.2f')
					sliderf('Size variation', cfg.ambient, 'sizeJitter', 0.0, 0.9, '+/- %.2f')
					ImGui.Spacing()
					for i = 1, #cfg.ambient.palette do
						color('Palette ' .. i, cfg.ambient.palette, i)
					end
				end
			end
		end

		if ImGui.CollapsingHeader('Cast animations') then
			check('Animate casts', cfg.castfx, 'enabled')
			hint('shape comes from the spell\'s real footprint, color from what it does')
			check('While the cast bar fills', cfg.castfx, 'charge')
			check('My own casts', cfg.castfx, 'selfCasts')
			check('Color detrimental by resist type', cfg.castfx, 'byResist')
			hint('off = one flat detrimental color')

			ImGui.Spacing()
			slideri('Detection radius', cfg.castfx, 'radius', 20, 400)
			slideri('Max concurrent', cfg.castfx, 'maxActive', 1, 40)
			slideri('Segments per cast', cfg.castfx, 'segsPerCast', 8, 120)
			sliderf('Burst duration', cfg.castfx, 'releaseLife', 0.2, 3.0, '%.2f s')
			sliderf('Fizzle duration', cfg.castfx, 'fizzleLife', 0.0, 2.0, '%.2f s')
			sliderf('Base radius', cfg.castfx, 'radius3d', 0.5, 12.0, '%.1f')
			sliderf('Height', cfg.castfx, 'height', 0.5, 20.0, '%.1f')
			slideri('Max AE reach', cfg.castfx, 'maxReach', 10, 300)
			hint('clamp on how far an area burst travels')

			if ImGui.CollapsingHeader('Cast colors') then
				color('Beneficial', cfg.castfx.colors, 'beneficial')
				color('Detrimental', cfg.castfx.colors, 'detrimental')
				ImGui.Spacing()
				ImGui.TextDisabled('by resist type:')
				for _, k in ipairs({ 'fire', 'cold', 'magic', 'poison', 'disease',
					'corruption', 'physical', 'arcane' }) do
					color(k:sub(1, 1):upper() .. k:sub(2), cfg.castfx.colors, k)
				end
				ImGui.TextDisabled('(arcane covers chromatic, prismatic and unresistable)')
			end
		end

		if ImGui.CollapsingHeader('Performance') then
			slideri('Segment budget', cfg, 'budget', 100, 6000)
			hint('total world segments per frame, all layers')
			ImGui.TextDisabled('Markers are drawn first, then casts, then ambient -')
			ImGui.TextDisabled('so a crowded zone thins the ambient layer rather than')
			ImGui.TextDisabled('dropping the thing you needed to see.')
			ImGui.TextDisabled(string.format('Thickness %d means each segment is drawn %d times.',
				cfg.markers.thickness, cfg.markers.thickness * 2 + 1))
		end

		if ImGui.CollapsingHeader('Idle') then
			check('Stop while EverQuest is in the background', cfg.pause, 'whenBackground')
			check('Stop while the client is not drawing frames', cfg.pause, 'whenNotRendering')
		end

		ImGui.Separator()
		ImGui.TextDisabled('/hl toggles all - /hl markers | tints | ambient | casts')
	end
	ImGui.End()
end

return M
