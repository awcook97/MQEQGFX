include("${CMAKE_CURRENT_SOURCE_DIR}/../../src/Plugin.cmake")

find_package(luajit REQUIRED)
find_package(sol2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(nlohmann_json REQUIRED)

set(MQEQGFX_HEADERS
    "src/Detours.h"
    "src/gpu/effects/SpellFxLooks.h"
    "src/gpu/FxCamera.h"
    "src/gpu/FxCatalog.h"
    "src/gpu/Glyphs.h"
    "src/gpu/LineFX.h"
    "src/model/dds.h"
    "src/model/eqg.h"
    "src/model/pfs.h"
    "src/model/render.h"
    "src/model/wld.h"
    "src/MQEQGFX.h"
    "src/PatternScan.h"
    "src/SettingsFile.h"
    "src/Signatures.h"
    "src/ui/Actions.h"
    "src/ui/ChatLog.h"
    "src/ui/ClassProfile.h"
    "src/ui/Controls.h"
    "src/ui/HotButtons.h"
    "src/ui/Layouts.h"
    "src/ui/NativeUi.h"
    "src/ui/Skins.h"
    "src/ui/SpellBook.h"
    "src/ui/Suppress.h"
    "src/ui/Widgets.h"
    "src/unitframes/UnitFrames.h"
    "src/unitframes/UnitWindow.h"
)

set(MQEQGFX_RESOURCES
    "resource.h"
    "MQEQGFX.rc"
)

set(MQEQGFX_SOURCES
    "src/CombatNumbers.cpp"
    "src/DebugWindow.cpp"
    "src/fixes/AnimationLod.cpp"
    "src/fixes/debugging/ActorAnimationStats.cpp"
    "src/fixes/debugging/ParticleGpuStats.cpp"
    "src/fixes/NametagBatch.cpp"
    "src/fixes/ParticleGpu.cpp"
    "src/GfxSettings.cpp"
    "src/gpu/effects/AttackFx.cpp"
    "src/gpu/effects/SpellFx.cpp"
    "src/gpu/effects/SpellFxBard.cpp"
    "src/gpu/effects/SpellFxBeastlord.cpp"
    "src/gpu/effects/SpellFxBerserker.cpp"
    "src/gpu/effects/SpellFxCleric.cpp"
    "src/gpu/effects/SpellFxDruid.cpp"
    "src/gpu/effects/SpellFxEnchanter.cpp"
    "src/gpu/effects/SpellFxGeneral.cpp"
    "src/gpu/effects/SpellFxLines.cpp"
    "src/gpu/effects/SpellFxMage.cpp"
    "src/gpu/effects/SpellFxMonk.cpp"
    "src/gpu/effects/SpellFxNecromancer.cpp"
    "src/gpu/effects/SpellFxPaladin.cpp"
    "src/gpu/effects/SpellFxRanger.cpp"
    "src/gpu/effects/SpellFxRogue.cpp"
    "src/gpu/effects/SpellFxShadowKnight.cpp"
    "src/gpu/effects/SpellFxShaman.cpp"
    "src/gpu/effects/SpellFxWarrior.cpp"
    "src/gpu/effects/SpellFxWizard.cpp"
    "src/gpu/FxCamera.cpp"
    "src/gpu/FxCatalog.cpp"
    "src/gpu/Glyphs.cpp"
    "src/gpu/LineFX.cpp"
    "src/Highlight.cpp"
    "src/model/dds.cpp"
    "src/model/eqg.cpp"
    "src/model/pfs.cpp"
    "src/model/render.cpp"
    "src/model/wld.cpp"
    "src/Models.cpp"
    "src/MQEQGFX.cpp"
    "src/SettingsFile.cpp"
    "src/Spells.cpp"
    "src/TargetRing.cpp"
    "src/ui/Actions.cpp"
    "src/ui/Canvas.cpp"
    "src/ui/ChatLog.cpp"
    "src/ui/ClassProfile.cpp"
    "src/ui/Controls.cpp"
    "src/ui/HotButtons.cpp"
    "src/ui/Layouts.cpp"
    "src/ui/Skins.cpp"
    "src/ui/skins/Actions.cpp"
    "src/ui/skins/Chat.cpp"
    "src/ui/skins/Collections.cpp"
    "src/ui/skins/Gems.cpp"
    "src/ui/skins/Hotbars.cpp"
    "src/ui/skins/Inventory.cpp"
    "src/ui/skins/SpellBookWnd.cpp"
    "src/ui/SpellBook.cpp"
    "src/ui/Suppress.cpp"
    "src/ui/UiSettings.cpp"
    "src/ui/Widgets.cpp"
    "src/unitframes/FocusWindow.cpp"
    "src/unitframes/Frame.cpp"
    "src/unitframes/GroupWindow.cpp"
    "src/unitframes/Options.cpp"
    "src/unitframes/PetWindow.cpp"
    "src/unitframes/PlayerWindow.cpp"
    "src/unitframes/PluginOptions.cpp"
    "src/unitframes/RaidWindow.cpp"
    "src/unitframes/Settings.cpp"
    "src/unitframes/TargetWindow.cpp"
    "src/unitframes/UnitWindow.cpp"
    "src/unitframes/XTargetWindow.cpp"
    "src/Units.cpp"
)

add_library(MQEQGFX SHARED
    ${MQEQGFX_HEADERS}
    ${MQEQGFX_SOURCES}
    ${MQEQGFX_RESOURCES}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${MQEQGFX_HEADERS} ${MQEQGFX_SOURCES})
source_group("Resource Files" FILES ${MQEQGFX_RESOURCES})

set_target_properties(MQEQGFX PROPERTIES FOLDER "CustomPlugins")

target_Plugin_props(MQEQGFX)

target_include_directories(MQEQGFX PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(MQEQGFX PRIVATE SOL_LUAJIT=1)
target_compile_options(MQEQGFX PRIVATE /EHa /bigobj /std:c++20)

target_link_libraries(MQEQGFX PRIVATE
    luajit::luajit
    sol2
    ZLIB::ZLIB
    nlohmann_json::nlohmann_json
    d3dcompiler
    imgui
    MQ2Main
)

add_dependencies(MQEQGFX MQ2Main imgui)
