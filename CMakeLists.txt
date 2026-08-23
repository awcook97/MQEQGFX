# MQEQGFX - MacroQuest plugin exposing the EQ graphics engine to Lua.
#
# Built automatically by the repo-root build-linux.sh plugin loop, which
# discovers this file and takes the DLL name from add_library() below
# (sol2/LuaJIT flags come from the MQEQGFX case block there). The target
# definition is also valid for a regular MacroQuest CMake build.
cmake_minimum_required(VERSION 3.20)
project(MQEQGFX)

add_library(MQEQGFX SHARED
    src/MQEQGFX.cpp
    src/CombatNumbers.cpp
    src/Models.cpp
    src/Spells.cpp
    src/Highlight.cpp
    src/Units.cpp
    src/gpu/LineFX.cpp
    src/gpu/FxCatalog.cpp
    src/gpu/FxCamera.cpp
    src/model/pfs.cpp
    src/model/eqg.cpp
    src/model/wld.cpp
    src/model/dds.cpp
    src/model/render.cpp
)
