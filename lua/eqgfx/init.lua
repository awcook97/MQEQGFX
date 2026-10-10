local mq = require('mq')

local ok, mod = pcall(require, 'plugin.MQEQGFX')
if not ok and mq.TLO.Plugin('mqeqgfx').IsLoaded() ~= true then
  mq.cmd('/squelch /plugin MQEQGFX noauto')
  ok, mod = pcall(require, 'plugin.MQEQGFX')
end
if not ok then
  error('eqgfx: the MQEQGFX plugin is not loaded (/plugin MQEQGFX)', 2)
end

mod.TargetType = require('eqgfx.core._types').TargetType

return mod
