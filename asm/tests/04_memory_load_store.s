# Test 04: memory loads and stores
# It checks bytes, words, odd word access, and tile-map memory.

.text

start:
    li16 a0, title
    ecall 0x012

    li16 x3, test_memory

    # SW/LW use little-endian 16-bit words.
    li16 x4, 0xABCD
    sw x4, 0(x3)
    lw x5, 0(x3)
    beq x5, x4, word_ok
    j word_fail
word_ok:
    lbu x5, 0(x3)
    li16 x4, 0x00CD
    beq x5, x4, low_byte_ok
    j endian_fail
low_byte_ok:
    lbu x5, 1(x3)
    li16 x4, 0x00AB
    beq x5, x4, endian_ok
    j endian_fail
endian_ok:
    li16 a0, word_pass
    ecall 0x012

    # LB sign-extends 0x80. LBU zero-extends 0x80.
    li16 x4, 0x0080
    sb x4, 2(x3)
    lb x5, 2(x3)
    li16 x4, 0xFF80
    beq x5, x4, lb_ok
    j byte_fail
lb_ok:
    lbu x5, 2(x3)
    li16 x4, 0x0080
    beq x5, x4, byte_ok
    j byte_fail
byte_ok:
    li16 a0, byte_pass
    ecall 0x012

    # Odd-address SW stores little-endian bytes at the odd address.
    li x4, 0x11
    sb x4, 3(x3)
    li x4, 0x22
    sb x4, 4(x3)
    mv x5, x3
    addi x5, 3
    li16 x4, 0xBEEF
    sw x4, 0(x5)
    lbu x4, 0(x5)
    li16 x0, 0x00EF
    beq x4, x0, odd_low_ok
    j align_fail
odd_low_ok:
    lbu x4, 1(x5)
    li16 x0, 0x00BE
    beq x4, x0, odd_store_ok
    j align_fail
odd_store_ok:

    # Odd-address LW reads the same little-endian word back.
    li x4, 0
    lw x4, 0(x5)
    li16 x0, 0xBEEF
    beq x4, x0, align_ok
    j align_fail
align_ok:
    li16 a0, align_pass
    ecall 0x012

    # CPU stores can write directly to the tile map at 0xF000.
    li16 x3, 0xF000
    li16 x4, 0x0201
    sw x4, 0(x3)
    lw x5, 0(x3)
    beq x5, x4, tile_ok
    j tile_fail
tile_ok:
    li16 a0, tile_pass
    ecall 0x012

    li16 a0, all_pass
    ecall 0x012
    ecall 0x3FF

word_fail:
    li16 a0, word_bad
    ecall 0x012
    ecall 0x3FF
endian_fail:
    li16 a0, endian_bad
    ecall 0x012
    ecall 0x3FF
byte_fail:
    li16 a0, byte_bad
    ecall 0x012
    ecall 0x3FF
align_fail:
    li16 a0, align_bad
    ecall 0x012
    ecall 0x3FF
tile_fail:
    li16 a0, tile_bad
    ecall 0x012
    ecall 0x3FF

.data
test_memory: .space 16
title:       .string "TEST 04 - MEMORY LOAD AND STORE\n"
word_pass:   .string "LW SW AND LITTLE ENDIAN: PASS\n"
byte_pass:   .string "LB AND LBU OF 0X80: PASS\n"
align_pass:  .string "ODD WORD ACCESS WORKS: PASS\n"
tile_pass:   .string "TILE MAP WRITE AT 0XF000: PASS\n"
all_pass:    .string "TEST 04 RESULT: PASS\n"
word_bad:    .string "TEST 04 RESULT: FAIL AT LW OR SW\n"
endian_bad:  .string "TEST 04 RESULT: FAIL AT ENDIAN ORDER\n"
byte_bad:    .string "TEST 04 RESULT: FAIL AT LB OR LBU\n"
align_bad:   .string "TEST 04 RESULT: FAIL AT ALIGNMENT\n"
tile_bad:    .string "TEST 04 RESULT: FAIL AT TILE MAP\n"
