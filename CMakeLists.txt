cmake_minimum_required(VERSION 3.16)

project(zx16_web LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hello_wasm
    hello/hello.cpp
)

set_target_properties(hello_wasm PROPERTIES
    SUFFIX ".html"
)

target_link_options(hello_wasm PRIVATE
    "-sWASM=1"
)

add_executable(zx16_backend
    backend_api.cpp
    ../src/core/memory.cpp
    ../src/core/register_file.cpp
    ../src/core/cpu.cpp
    ../src/core/ecall.cpp
    ../src/core/instruction_decoder.cpp
    ../src/core/graphics_memory.cpp
)

target_include_directories(zx16_backend PRIVATE
    ../src/core
)

set_target_properties(zx16_backend PROPERTIES
    SUFFIX ".js"
)

target_link_options(zx16_backend PRIVATE
    "-sWASM=1"
    "-sSINGLE_FILE=1"
    "-sMODULARIZE=1"
    "-sEXPORT_NAME=createZx16Backend"
    "-sENVIRONMENT=web,node"
    "-sEXPORTED_RUNTIME_METHODS=['cwrap','ccall','UTF8ToString']"
    "-sEXPORTED_FUNCTIONS=['_zx16_reset','_zx16_seed_rng','_zx16_load_byte','_zx16_read8','_zx16_write8','_zx16_read16','_zx16_write16','_zx16_set_pc','_zx16_get_pc','_zx16_set_sp','_zx16_get_sp','_zx16_get_register','_zx16_set_register','_zx16_step','_zx16_step_with_breakpoints','_zx16_step_over','_zx16_run_to_cursor','_zx16_run','_zx16_is_halted','_zx16_get_last_instruction','_zx16_get_output','_zx16_clear_output','_zx16_toggle_breakpoint','_zx16_has_breakpoint','_zx16_get_breakpoint_count','_zx16_clear_breakpoints','_zx16_has_breakpoint_hit','_zx16_get_breakpoint_hit_address','_zx16_clear_breakpoint_hit','_zx16_set_keyboard_key','_zx16_get_keyboard_key','_zx16_clear_keyboard_key','_zx16_has_pending_tone','_zx16_get_tone_frequency','_zx16_get_tone_duration_ms','_zx16_clear_tone_request','_zx16_has_pending_stop_audio','_zx16_clear_stop_audio_request','_zx16_get_volume_percent']"
)
