--[[
  eqgfx/init.lua - entry shim.

  The core module is implemented natively by the MQEQGFX MacroQuest plugin
  and reached through MQ2Lua's plugin-module loader; this keeps
  require('eqgfx') working for every feature script and external user.
  MQ2Lua registers the module as a dependency of the requiring script, so
  scripts are stopped cleanly if the plugin unloads.
]]

local ok, mod = pcall(require, 'plugin.MQEQGFX')
if not ok then
  -- One auto-load attempt before giving up.
  local mq = require('mq')
  mq.cmd('/squelch /plugin MQEQGFX noauto')
  ok, mod = pcall(require, 'plugin.MQEQGFX')
end
if not ok then
  error('eqgfx: the MQEQGFX plugin is not loaded (/plugin MQEQGFX)', 2)
end

-- eSpellTargetType values + LuaLS annotations stay Lua-side in core/_types.lua.
mod.TargetType = require('eqgfx.core._types').TargetType

return mod
