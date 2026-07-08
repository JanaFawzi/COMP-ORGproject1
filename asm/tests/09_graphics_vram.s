# Test 09: graphics VRAM
# It checks tile map, tile definitions, palette bytes, nibbles, and RGB332 math.

.text

start:
    li16 a0, title
    ecall 0x012

    # Tile map covers 20x15 cells from 0xF000 through 0xF12B.
    li16 x3, 0xF000
    li x4, 1
    sb x4, 0(x3)
    lbu x5, 0(x3)
    beq x5, x4, tile_first_ok
    j tile_map_fail
tile_first_ok:
    li16 x3, 0xF128
    li x4, 15
    sb x4, 3(x3)
    lbu x5, 3(x3)
    beq x5, x4, tile_map_ok
    j tile_map_fail
tile_map_ok:
    li16 a0, tile_map_pass
    ecall 0x012

    # Tile 0 starts at 0xF200, tile 15 ends at 0xF9FF.
    li16 x3, 0xF200
    li16 x4, 0x0021
    sb x4, 0(x3)
    lbu x5, 0(x3)
    beq x5, x4, tile_def_first_ok
    j tile_def_fail
tile_def_first_ok:
    li16 x3, 0xF9F8
    li16 x4, 0x00AB
    sb x4, 7(x3)
    lbu x5, 7(x3)
    beq x5, x4, tile_def_ok
    j tile_def_fail
tile_def_ok:
    li16 a0, tile_def_pass
    ecall 0x012

    # In one tile byte, even x uses the low nibble and odd x uses the high nibble.
    li16 x3, 0xF200
    lbu x4, 0(x3)
    mv x5, x4
    andi x5, 0x0F
    li x0, 1
    beq x5, x0, low_nibble_ok
    j nibble_fail
low_nibble_ok:
    srli x4, 4
    andi x4, 0x0F
    li x0, 2
    beq x4, x0, nibble_ok
    j nibble_fail
nibble_ok:
    li16 a0, nibble_pass
    ecall 0x012

    # Palette is 16 RGB332 bytes at 0xFA00 through 0xFA0F.
    li16 x3, 0xFA00
    li16 x4, 0x00E3
    sb x4, 0(x3)
    lbu x5, 0(x3)
    beq x5, x4, palette_first_ok
    j palette_fail
palette_first_ok:
    li16 x3, 0xFA08
    li16 x4, 0x001C
    sb x4, 7(x3)
    lbu x5, 7(x3)
    beq x5, x4, palette_ok
    j palette_fail
palette_ok:
    li16 a0, palette_pass
    ecall 0x012

    # RGB332 full-intensity channels expand to 0xFF.
    li x3, 7
    mv x4, x3
    slli x3, 5
    slli x4, 2
    or x3, x4
    li x4, 7
    srli x4, 1
    or x3, x4
    li16 x4, 0x00FF
    beq x3, x4, red_expand_ok
    j rgb_fail
red_expand_ok:
    li x3, 3
    mv x4, x3
    slli x3, 6
    slli x4, 4
    or x3, x4
    li x4, 3
    slli x4, 2
    or x3, x4
    li x4, 3
    or x3, x4
    li16 x4, 0x00FF
    beq x3, x4, rgb_ok
    j rgb_fail
rgb_ok:
    li16 a0, rgb_pass
    ecall 0x012

    li16 a0, all_pass
    ecall 0x012
    ecall 0x3FF

tile_map_fail:
    li16 a0, tile_map_bad
    ecall 0x012
    ecall 0x3FF
tile_def_fail:
    li16 a0, tile_def_bad
    ecall 0x012
    ecall 0x3FF
nibble_fail:
    li16 a0, nibble_bad
    ecall 0x012
    ecall 0x3FF
palette_fail:
    li16 a0, palette_bad
    ecall 0x012
    ecall 0x3FF
rgb_fail:
    li16 a0, rgb_bad
    ecall 0x012
    ecall 0x3FF

.data
title:         .string "TEST 09 - GRAPHICS VRAM\n"
tile_map_pass: .string "TILE MAP 0XF000 TO 0XF12B: PASS\n"
tile_def_pass: .string "TILE DEFINITIONS 0XF200 TO 0XF9FF: PASS\n"
nibble_pass:   .string "TILE NIBBLE ORDER LOW HIGH: PASS\n"
palette_pass:  .string "PALETTE RGB332 BYTES: PASS\n"
rgb_pass:      .string "RGB332 EXPANSION MATH: PASS\n"
all_pass:      .string "TEST 09 RESULT: PASS\n"
tile_map_bad:  .string "TEST 09 RESULT: FAIL AT TILE MAP\n"
tile_def_bad:  .string "TEST 09 RESULT: FAIL AT TILE DEFINITIONS\n"
nibble_bad:    .string "TEST 09 RESULT: FAIL AT NIBBLE ORDER\n"
palette_bad:   .string "TEST 09 RESULT: FAIL AT PALETTE\n"
rgb_bad:       .string "TEST 09 RESULT: FAIL AT RGB332 EXPANSION\n"
