cmake_minimum_required(VERSION 3.16)

project(zx16_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

FetchContent_MakeAvailable(raylib)

add_library(zx16_core
    src/core/memory.cpp
    src/core/register_file.cpp
    src/core/cpu.cpp
    src/core/ecall.cpp
    src/core/program_loader.cpp
    src/core/instruction_decoder.cpp
    src/core/graphics_memory.cpp
    src/core/snake_runtime.cpp
)

target_include_directories(zx16_core PUBLIC
    src/core
)

add_library(zx16_desktop
    src/desktop/gui.cpp
)

target_include_directories(zx16_desktop PUBLIC
    src/desktop
)

target_link_libraries(zx16_desktop PUBLIC
    zx16_core
    raylib
)

add_executable(zx16sim
    src/app/main.cpp
)

target_link_libraries(zx16sim PRIVATE
    zx16_core
    zx16_desktop
)
