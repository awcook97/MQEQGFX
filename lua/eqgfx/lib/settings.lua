local mq = require('mq')

local M = {}

M.SCOPES = { 'char', 'class', 'global' }
M.SCOPE_LABELS = {
  char   = 'This character',
  class  = 'This class',
  global = 'All characters',
}

local SAVE_QUIET_MS = 500

local function sanitize(s)
  return (tostring(s or 'unknown'):gsub('[^%w_%-]', ''))
end

---@param scope string # 'char' | 'class' | 'global'
---@return string file
local function file_for(scope)
  if scope == 'global' then return 'eqgfx/global_settings.lua' end
  if scope == 'class' then return 'eqgfx/class_' .. sanitize(mq.TLO.Me.Class.ShortName()) .. '_settings.lua' end
  return 'eqgfx/' .. sanitize(mq.TLO.EverQuest.Server()) .. '_' .. sanitize(mq.TLO.Me.CleanName()) .. '_settings.lua'
end

---@param path string # absolute path
---@return table|nil settings
local function load_table(path)
  local chunk = loadfile(path, 't', {})
  if not chunk then return nil end
  local ok, t = pcall(chunk)
  if ok and type(t) == 'table' then return t end
  return nil
end

local function read_file(file)
  return load_table(mq.configDir .. '/' .. file)
end

---@param dst table|nil # loaded (possibly partial) config
---@param def table # the defaults to backfill from
---@return table dst # the merged table (same identity as dst when it was a table)
local function merge_defaults(dst, def)
  dst = (type(dst) == 'table') and dst or {}
  for k, v in pairs(def) do
    if type(v) == 'table' then
      dst[k] = merge_defaults(dst[k], v)
    elseif type(dst[k]) ~= type(v) or dst[k] ~= dst[k] then
      dst[k] = v
    end
  end
  return dst
end
M.merge_defaults = merge_defaults

---@class SettingsStore
---@field section string
---@field defaults table
---@field legacy string|nil
---@field data table          live settings (defaults until load() merges files)
---@field scope string        'char' | 'class' | 'global'
---@field load fun(): table
---@field save fun()
---@field mark_dirty fun()
---@field maybe_save fun()
---@field set_scope fun(scope: string, log?: table)

---@param opts { section: string, defaults: table, legacy: string|nil } # legacy = old single-file config to import once
---@return SettingsStore store
function M.new(opts)
  local self = {
    section  = assert(opts.section, 'settings store needs a section name'),
    defaults = assert(opts.defaults, 'settings store needs defaults'),
    legacy   = opts.legacy,
    scope    = 'char',
  }
  self.data = merge_defaults({}, self.defaults)
  local dirty, changedAt = false, 0

  ---@return table data # the live settings table (also self.data)
  function self.load()
    local found, level
    for _, scope in ipairs(M.SCOPES) do
      local t = read_file(file_for(scope))
      if t and type(t[self.section]) == 'table' then
        found, level = t[self.section], scope
        break
      end
    end
    if not found and self.legacy then
      found = load_table(self.legacy)
    end
    self.data  = merge_defaults(found or {}, self.defaults)
    self.scope = level or 'char'
    return self.data
  end

  function self.save()
    local file = file_for(self.scope)
    local whole = read_file(file) or {}
    whole[self.section] = self.data
    mq.pickle(file, whole)
  end

  function self.mark_dirty()
    dirty, changedAt = true, mq.gettime()
  end

  function self.maybe_save()
    if dirty and mq.gettime() - changedAt >= SAVE_QUIET_MS then
      dirty = false
      self.save()
    end
  end

  ---@param scope string # 'char' | 'class' | 'global'
  ---@param log table|nil # lwlogger-style module
  function self.set_scope(scope, log)
    if scope == self.scope then return end
    self.scope = scope
    for _, sc in ipairs(M.SCOPES) do
      if sc == scope then break end
      local whole = read_file(file_for(sc))
      if whole and whole[self.section] ~= nil then
        whole[self.section] = nil
        mq.pickle(file_for(sc), whole)
      end
    end
    self.save()
    dirty = false
    if log then log.Info('settings scope -> %s (%s)', scope, file_for(scope)) end
  end

  return self
end

return M
