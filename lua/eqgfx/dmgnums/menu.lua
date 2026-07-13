--[[
  dmgnums/menu.lua - the in-game customization window (/dnmenu).

  Every widget writes straight into settings.data and marks it dirty; the main
  loop debounce-saves through the shared scoped store.
]]

local ImGui = require("ImGui")
local types = require("eqgfx.dmgnums._types")
local settings = require("eqgfx.dmgnums.settings")

local M = { open = false }

--- Show/hide the settings window (the /dnmenu bind).
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
	local v, changed = ImGui.SliderFloat(label, t[k], lo, hi, fmt or "%.1f")
	if changed then
		t[k] = v
		mark()
	end
end

--- Keep colors as plain number tables (pickle-safe) regardless of what the
--- binding hands back (table vs ImVec4).
local function to_color(colorVal)
	if type(colorVal) == "table" then
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

--- Combo box bound to t[k] (1-based index into items); marks dirty on change.
local function combo(label, t, k, items)
	local v, changed = ImGui.Combo(label, t[k], items, #items)
	if changed then
		t[k] = v
		mark()
	end
end

-- The WHO x WHAT filter grid: one row per source, one checkbox per kind.
local GRID_SOURCES = {
	{ key = 'mine',     label = 'Mine' },
	{ key = 'pet',      label = 'My pet' },
	{ key = 'others',   label = 'Others' },
	{ key = 'incoming', label = 'Incoming' },
}
local GRID_KINDS = {
	{ key = 'melee',  label = 'Melee' },
	{ key = 'spells', label = 'Spells' },
	{ key = 'dots',   label = 'DoTs' },
	{ key = 'heals',  label = 'Heals' },
	{ key = 'misses', label = 'Misses' },
}

--- Render the settings window (call every frame from the ImGui callback;
--- no-op while closed).
function M.draw()
	if not M.open then
		return
	end
	local cfg = settings.data or {}
	local open, show = ImGui.Begin("EQGFX Damage Numbers", M.open)
	M.open = open
	if show then
		ImGui.Text("Save for:")
		ImGui.SameLine()
		if ImGui.RadioButton("This character", settings.scope == "char") then
			settings.set_scope("char")
		end
		ImGui.SameLine()
		if ImGui.RadioButton("This server", settings.scope == "server") then
			settings.set_scope("server")
		end
		ImGui.SameLine()
		if ImGui.RadioButton("All characters", settings.scope == "global") then
			settings.set_scope("global")
		end
		ImGui.Separator()

		check("Enabled", cfg, "enabled")
		slideri("Radius", cfg, "radius", 20, 500)
		sliderf("Height above head", cfg, "zOffset", 0.0, 15.0)
		slideri("Minimum damage", cfg, "minDamage", 0, 500)
		check("Outgoing on current target only", cfg, "targetOnly")
		check("Hide numbers behind EQ windows", cfg, "hideUnderUI")

		if ImGui.CollapsingHeader("Filters") then
			ImGui.TextDisabled("who (rows) x what (columns)")
			for _, src in ipairs(GRID_SOURCES) do
				ImGui.Text(string.format("%-9s", src.label))
				for _, kind in ipairs(GRID_KINDS) do
					ImGui.SameLine()
					check(kind.label .. "##" .. src.key, cfg.show[src.key], kind.key)
				end
			end
			ImGui.Separator()
			check("Damage to my pet counts as incoming", cfg, "petIncoming")
			check("Incoming numbers float over my head", cfg, "incomingOverSelf")
		end

		if ImGui.CollapsingHeader("Text") then
			sliderf("Size", cfg.text, "size", 8.0, 40.0)
			check("Drop shadow", cfg.text, "shadow")
			check("Outline (heavier)", cfg.text, "outline")
			check("Abbreviate thousands (12.3k)", cfg.text, "abbreviate")
			check("'+' on heals", cfg.text, "healSign")
			check("Scale with damage", cfg.text, "scaleWithDamage")
			if cfg.text.scaleWithDamage then
				slideri("Full-size damage", cfg.text, "bigAt", 100, 20000)
				sliderf("Max scale", cfg.text, "bigScale", 1.0, 3.0)
			end
			check("Spell name under spell numbers", cfg.text, "showSpellName")
			if cfg.text.showSpellName then
				sliderf("Spell name size", cfg.text, "spellNameSize", 6.0, 20.0)
			end
		end

		if ImGui.CollapsingHeader("Animation") then
			combo("Style", cfg.anim, "style", types.AnimStyleLabels)
			sliderf("Duration (s)", cfg.anim, "duration", 0.3, 4.0)
			sliderf("Rise speed (px/s)", cfg.anim, "riseSpeed", 0.0, 200.0)
			sliderf("Horizontal spread (px)", cfg.anim, "spreadX", 0.0, 80.0)
			if cfg.anim.style == types.AnimStyle.ARC or cfg.anim.style == types.AnimStyle.FOUNTAIN then
				sliderf("Gravity (px/s2)", cfg.anim, "gravity", 0.0, 400.0)
			end
			sliderf("Fade starts at (fraction)", cfg.anim, "fadeStart", 0.0, 1.0, "%.2f")
			check("Pop-in", cfg.anim, "popIn")
			if cfg.anim.popIn then
				sliderf("Pop duration (s)", cfg.anim, "popDur", 0.05, 0.5, "%.2f")
			end
			check("Stack bursts upward", cfg.anim, "stackNudge")
		end

		if ImGui.CollapsingHeader("Colors") then
			ImGui.TextDisabled("mine")
			color("My melee", cfg.colors, "myMelee")
			color("My spells", cfg.colors, "mySpell")
			color("My DoTs", cfg.colors, "myDot")
			ImGui.TextDisabled("my pet")
			color("Pet melee", cfg.colors, "petMelee")
			color("Pet spells", cfg.colors, "petSpell")
			color("Pet DoTs", cfg.colors, "petDot")
			ImGui.TextDisabled("others")
			color("Other melee", cfg.colors, "otherMelee")
			color("Other spells", cfg.colors, "otherSpell")
			color("Other DoTs", cfg.colors, "otherDot")
			ImGui.TextDisabled("special")
			color("Heals", cfg.colors, "heal")
			color("Incoming damage", cfg.colors, "incoming")
			color("Incoming heals", cfg.colors, "incomingHeal")
			color("Misses", cfg.colors, "miss")
			color("Spell name", cfg.colors, "spellName")
			color("Shadow/outline", cfg.colors, "shadow")
		end

		ImGui.Separator()
		ImGui.TextDisabled("/dntest previews one of each over your target")
	end
	ImGui.End()
end

return M
