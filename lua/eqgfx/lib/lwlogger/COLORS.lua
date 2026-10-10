---@module 'lwlogger.COLORS'

---@alias lwlogger.ColorName
---| '"black"'
---| '"green"'
---| '"magenta"'
---| '"orange"'
---| '"purple"'
---| '"red"'
---| '"cyan"'
---| '"blue"'
---| '"white"'
---| '"yellow"'
---| '"reset"'

---@alias lwlogger.ColorCode
---| '"b"' # black
---| '"g"' # green
---| '"m"' # magenta
---| '"o"' # orange
---| '"p"' # purple
---| '"r"' # red
---| '"t"' # cyan
---| '"u"' # blue
---| '"w"' # white
---| '"y"' # yellow
---| '"x"' # previous/reset

---@alias lwlogger.DarkColorCode
---| '"-b"' # black (dark)
---| '"-g"' # green (dark)
---| '"-m"' # magenta (dark)
---| '"-o"' # orange (dark)
---| '"-p"' # purple (dark)
---| '"-r"' # red (dark)
---| '"-t"' # cyan (dark)
---| '"-u"' # blue (dark)
---| '"-w"' # white (dark)
---| '"-y"' # yellow (dark)

---@alias lwlogger.Color
---| lwlogger.ColorName
---| lwlogger.ColorCode
---| lwlogger.DarkColorCode

---@alias lwlogger.Role
---| '"app"'       # the application name segment
---| '"module"'    # the module name segment
---| '"time"'      # the timestamp segment
---| '"character"' # the character segment
---| '"source"'    # the caller file:line segment
---| '"var"'       # substituted message variables
---| '"trace"'     # TRACE level label
---| '"debug"'     # DEBUG level label
---| '"info"'      # INFO level label
---| '"warn"'      # WARN level label
---| '"error"'     # ERROR level label
---| '"fatal"'     # FATAL level label

---@class lwlogger.colors
---@field CODES       table<lwlogger.ColorName, lwlogger.ColorCode> # friendly name -> code letter
---@field SCHEME      table<lwlogger.Role, lwlogger.Color>          # role -> the color it's drawn in
---@field VAR_PALETTE lwlogger.Color[]                              # colors cycled for multicolored vars
local color = {}

local ESC   = '\a'
local RESET  = ESC .. 'x'

color.CODES = {
  black = 'b', green = 'g', magenta = 'm', orange = 'o', purple = 'p',
  red   = 'r', cyan  = 't', blue    = 'u', white  = 'w', yellow = 'y',
  reset = 'x',
}

color.SCHEME = {
  app       = 'green',
  module    = 'yellow',
  time      = 'white',
  character = 'cyan',
  source    = '-w',
  var       = 'purple',
  trace = '-w', debug = 'u', info = 'g', warn = 'y', error = 'r', fatal = 'm',
}

color.VAR_PALETTE = { 'p', 't', 'y', 'o', 'u', 'm', 'g' }

---@param name lwlogger.Color|nil
---@param text string
---@return string
function color.wrap(name, text)
  if not name or name == '' then return text end
  local code = color.CODES[name] or name
  return ESC .. code .. text .. RESET
end

---@param role lwlogger.Role
---@param text string
---@return string
function color.ColoredRole(role, text)
  return color.wrap(color.SCHEME[role] or 'white', text)
end

---@param name string
---@return string
function color.ColoredModule(name)   return color.ColoredRole('module', name) end
---@param name string
---@return string
function color.ColoredFunction(name) return color.wrap('cyan', name) end

---@param multicolor boolean         # cycle VAR_PALETTE instead of one fixed color
---@param baseColor  lwlogger.Color   # color used when not multicolor
---@param fmt        string           # printf-style format string
---@param ...        any              # arguments substituted into `fmt`
---@return string
function color.ColoredVars(multicolor, baseColor, fmt, ...)
  local args = { ... }
  local n = 0
  local out = fmt:gsub('%%[%-%+ #0]*%d*%.?%d*[%a%%]', function(spec)
    if spec == '%%' then return '%' end
    n = n + 1
    local ok, formatted = pcall(string.format, spec, args[n])
    if not ok then formatted = tostring(args[n]) end
    local c = baseColor
    if multicolor then
      c = color.VAR_PALETTE[(n - 1) % #color.VAR_PALETTE + 1]
    end
    return color.wrap(c, formatted)
  end)
  return out
end

---@param rules table<lwlogger.Role, lwlogger.Color>
function color.SetColorRules(rules)
  for role, c in pairs(rules or {}) do color.SCHEME[role] = c end
end

---@param s string
---@return string
function color.strip(s)
  return (s:gsub('\a%-?.', ''))
end

return color
