local Store = require('eqgfx.lib.settings')
local types = require('eqgfx.nameplates._types')

---@type NameplatesConfig
local DEFAULTS = {
  enabled     = true,
  radius      = 200,
  hideUnderUI = true,
  uiOccludeMode = types.UiOccludeMode.CLIP,
  extraWindows = {},
  clickTarget = false,

  pause = {
    whenBackground   = false,
    whenNotRendering = true,
  },

  los = {
    enabled  = false,
    interval = 0.25,
    exempt = {
      group      = true,
      raid       = true,
      xtarget    = true,
      pet        = true,
      fellowship = true,
      target     = true,
    },
  },

  distScale = {
    enabled  = false,
    near     = 30,
    far      = 200,
    minScale = 0.55,
  },

  overlap = {
    mode          = types.PlateOverlapMode.OFF,
    gap           = 2,
    xPad          = 4,
    maxPush       = 160,
    clusterRadius = 45,
  },

  show = {
    npcs = true,
    pcs  = true,
    pets = true,
    self = false,
  },

  bar = {
    width           = 80,
    height          = 9,
    rounding        = 2.0,
    zOffset         = 4.0,
    opacity         = 1.0,
    borderThickness = 1.0,
    texture         = types.BarTexture.GLASS,
  },

  hp = {
    gradient = true,
    showPct  = false,
    textPos  = types.HpTextPos.IN_RIGHT,
    textSize = 11.0,
  },

  resources = {
    manaScope = types.ResScope.SELF,
    enduScope = types.ResScope.OFF,
    height    = 4,
  },

  name = {
    show      = true,
    position  = types.NamePosition.ABOVE,
    size      = 13.0,
    offsetX   = 0,
    offsetY   = 0,
    anonMode  = types.AnonMode.OFF,
    background = false,
    bgPadding  = 2,
    shadow     = true,
    anim       = types.NameAnim.NONE,
    animSpeed  = 1.0,
    animAmount = 2.0,
  },

  castbar = {
    show            = true,
    height          = 7,
    gap             = 3,
    widthScale      = 1.0,
    showIcon        = true,
    iconSize        = 18,
    showSpellName   = true,
    showTime        = true,
    showTotal       = false,
    textSize        = 12.0,
    onlyTarget      = false,
    interruptDetect = true,
  },

  buffs = {
    enabled    = true,
    onlyTarget = false,
    iconSize   = 16,
    spacing    = 1,
    maxIcons   = 20,
    maxPerRow  = 8,
    borders    = true,
    combine    = false,
    beneficial  = { position = types.BuffPosition.TOP,   direction = types.BuffDirection.HORIZONTAL },
    detrimental = { position = types.BuffPosition.RIGHT, direction = types.BuffDirection.VERTICAL },

    filterMode = types.BuffFilterMode.ALL,
    whitelist  = {},
    blacklist  = {},
    mineOnly   = false,

    borderMode  = types.BuffBorderMode.BY_TYPE,
    mineBorder  = { 1.00, 0.85, 0.20, 1.00 },
    otherBorder = { 0.55, 0.55, 0.55, 0.90 },
    dimOthers   = 0.0,

    overrides = {},

    tooltip          = true,
    rightClickInspect = true,
    appearFlash      = true,
    detPulse         = false,
  },

  target = {
    distinguish     = true,
    scale           = 1.25,
    border          = true,
    borderColor     = { 1.00, 0.95, 0.25, 1.00 },
    borderThickness = 2.0,
    glow            = true,
  },

  aehl = {
    enabled     = true,
    fromMe      = true,
    fromPCs     = true,
    fromNPCs    = true,
    tintBar     = true,  strength    = 0.65,
    tintBorder  = true,
    glow        = true,
    pulse       = true,  pulseSpeed  = 1.2,   pulseAmount = 0.35,
    fadeSpeed   = 8.0,
    stackBase   = 0.5,
    stackStep   = 0.1,
    stackMax    = 5,
  },

  anim = {
    hpSmoothing    = true,  hpSpeed        = 10.0,
    fadeIn         = true,  fadeInDur      = 0.20,
    fadeOut        = true,  fadeOutDur     = 0.50,
    damageFlash    = true,  flashThreshold = 0.05,  flashDur = 0.30,
    appearPop      = true,  popDur         = 0.25,
    castPulse      = true,

    sheen          = true,  sheenPeriod    = 3.0,
    stripeScroll   = true,  stripeSpeed    = 24.0,
    lowHpPulse     = true,  lowHpThreshold = 0.30,  lowHpSpeed = 1.5,
    breathe        = false, breatheAmount  = 0.25,  breatheSpeed = 0.5,
    borderGlow     = false, glowSpeed      = 1.0,
    bob            = false, bobAmount      = 2.0,   bobSpeed = 0.5,
  },

  colors = {
    barHigh       = { 0.16, 1.00, 0.16, 1.00 },
    barMid        = { 1.00, 1.00, 0.16, 1.00 },
    barLow        = { 1.00, 0.16, 0.16, 1.00 },
    barFixed      = { 0.20, 0.80, 0.20, 1.00 },
    barBack       = { 0.11, 0.11, 0.11, 1.00 },
    border        = { 0.00, 0.00, 0.00, 1.00 },
    name          = { 1.00, 1.00, 1.00, 1.00 },
    hpText        = { 1.00, 1.00, 1.00, 0.90 },
    flash         = { 1.00, 1.00, 1.00, 0.65 },
    lowHp         = { 1.00, 0.10, 0.10, 0.90 },
    glow          = { 1.00, 0.85, 0.30, 0.50 },
    nameBg        = { 0.00, 0.00, 0.00, 0.55 },
    nameShadow    = { 0.00, 0.00, 0.00, 0.90 },
    manaFill      = { 0.25, 0.45, 1.00, 1.00 },
    enduFill      = { 1.00, 0.80, 0.20, 1.00 },
    resourceBack  = { 0.08, 0.08, 0.08, 0.90 },
    castFill      = { 1.00, 0.65, 0.15, 0.95 },
    castBack      = { 0.08, 0.08, 0.08, 0.85 },
    castText      = { 1.00, 1.00, 1.00, 1.00 },
    castInterrupt = { 1.00, 0.20, 0.20, 0.90 },
    aeDet         = { 1.00, 0.55, 0.10, 1.00 },
    aeBen         = { 0.45, 0.80, 1.00, 1.00 },
  },
}

---@class NameplatesStore: SettingsStore
---@field data NameplatesConfig

return Store.new({ section = 'nameplates', defaults = DEFAULTS }) --[[@as NameplatesStore]]
