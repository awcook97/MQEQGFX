local A = {}

---@param t number
---@return number clamped
function A.clamp01(t)
  if t < 0 then return 0 elseif t > 1 then return 1 end
  return t
end

---@param a number # value at t = 0
---@param b number # value at t = 1
---@param t number # blend factor (not clamped)
---@return number
function A.lerp(a, b, t) return a + (b - a) * t end

---@param cur number # current value
---@param target number # value to move toward
---@param speed number # approach rate per second (higher = snappier)
---@param dt number # seconds since the last update
---@return number next # the moved value
function A.approach(cur, target, speed, dt)
  return cur + (target - cur) * math.min(1, dt * speed)
end

---@param t number
---@return number eased
function A.smoothstep(t)
  t = A.clamp01(t)
  return t * t * (3 - 2 * t)
end

---@param t number # progress 0..1 (clamped)
---@return number scale # ~0..1.1..1
function A.ease_out_back(t)
  t = A.clamp01(t)
  local c1, c3 = 1.70158, 2.70158
  local u = t - 1
  return 1 + c3 * u * u * u + c1 * u * u
end

---@param startedAt number|nil # when the event began (nil = no event -> 0)
---@param dur number # fade duration in the caller's timebase
---@param timeNow number # current time, same timebase as startedAt
---@return number f # 1 at start -> 0 at startedAt+dur (0 when expired/nil)
function A.fade(startedAt, dur, timeNow)
  if not startedAt or dur <= 0 then return 0 end
  return 1 - A.clamp01((timeNow - startedAt) / dur)
end

---@param timeNow number
---@param period number|nil # seconds per cycle (default 0.4)
---@return number pulse # 0..1
function A.pulse(timeNow, period)
  return 0.5 + 0.5 * math.sin(timeNow * (2 * math.pi) / (period or 0.4))
end

---@param c1 number[] # {r,g,b,a} floats 0..1, at t = 0
---@param c2 number[] # {r,g,b,a} floats 0..1, at t = 1
---@param t number # blend factor
---@return number[] color # new {r,g,b,a}
function A.lerp_color(c1, c2, t)
  return { A.lerp(c1[1], c2[1], t), A.lerp(c1[2], c2[2], t),
           A.lerp(c1[3], c2[3], t), A.lerp(c1[4], c2[4], t) }
end

return A
