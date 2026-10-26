local mq    = require('mq')
local eqgfx = require('eqgfx')
local log   = require('eqgfx.lib.lwlogger')
local pause = require('eqgfx.lib.pause')

log.SetAppName('eqgfx')
log.SetModuleName('debug')
log.SetColors(true)
log.SetIncludeTime('seconds')
log.SetLevel(log.DEBUG)

pause.wait_for_game()

local ok, err = eqgfx.init()
if not ok then log.Error('init failed: %s', err) return end

local w, h = eqgfx.get_screen()
log.Info('display = %d x %d  (should be your resolution)', w, h)

log.Info('running - look for giant red/green/blue 3D lines through you')

local RED   = eqgfx.argb(255, 255, 0, 0)
local GREEN = eqgfx.argb(255, 0, 255, 0)
local BLUE  = eqgfx.argb(255, 80, 160, 255)
local last  = mq.gettime()
local D     = 3000

while true do
  local me = mq.TLO.Me
  local x, y, z = me.X(), me.Y(), me.Z()
  eqgfx.clear()
  if x then
    eqgfx.add_line(x - D, y, z, x + D, y, z, RED)
    eqgfx.add_line(x, y - D, z, x, y + D, z, GREEN)
    eqgfx.add_line(x, y, z - D, x, y, z + D, BLUE)
  end

  if mq.gettime() - last >= 1000 then
    local scene, draws = eqgfx.stats()
    log.Debug('scene=%d draws=%d  me=(%.1f, %.1f, %.1f)',
              scene, draws, x or -1, y or -1, z or -1)
    last = mq.gettime()
  end
  mq.delay(50)
end
