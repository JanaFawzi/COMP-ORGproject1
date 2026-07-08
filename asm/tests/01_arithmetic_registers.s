# Test 01: arithmetic and registers
# The program checks its own results and prints PASS or FAIL.

.text

start:
    li16 a0, title
    ecall 0x012

    # ADD: 20 + 22 = 42
    li x3, 20
    li x4, 22
    add x3, x4
    li x4, 42
    beq x3, x4, add_ok
    j add_fail
add_ok:
    li16 a0, add_pass
    ecall 0x012

    # SUB: 50 - 8 = 42
    li x3, 50
    li x4, 8
    sub x3, x4
    li x4, 42
    beq x3, x4, sub_ok
    j sub_fail
sub_ok:
    li16 a0, sub_pass
    ecall 0x012

    # MV copies the complete register value.
    li16 x3, 0xBEEF
    mv x5, x3
    beq x5, x3, mv_ok
    j mv_fail
mv_ok:
    li16 a0, mv_pass
    ecall 0x012

    # x0 is writable and its stored value can be read again.
    li x0, 7
    li x4, 5
    add x0, x4
    li x3, 0
    add x3, x0
    li x4, 12
    beq x3, x4, x0_ok
    j x0_fail
x0_ok:
    li16 a0, x0_pass
    ecall 0x012

    # 16-bit arithmetic wraps: 0xFFFF + 1 = 0.
    li16 x3, 0xFFFF
    li x4, 1
    add x3, x4
    li x4, 0
    beq x3, x4, wrap_ok
    j wrap_fail
wrap_ok:
    li16 a0, wrap_pass
    ecall 0x012

    li16 a0, all_pass
    ecall 0x012
    ecall 0x3FF

add_fail:
    li16 a0, add_bad
    ecall 0x012
    ecall 0x3FF
sub_fail:
    li16 a0, sub_bad
    ecall 0x012
    ecall 0x3FF
mv_fail:
    li16 a0, mv_bad
    ecall 0x012
    ecall 0x3FF
x0_fail:
    li16 a0, x0_bad
    ecall 0x012
    ecall 0x3FF
wrap_fail:
    li16 a0, wrap_bad
    ecall 0x012
    ecall 0x3FF

.data
title:      .string "TEST 01 - ARITHMETIC AND REGISTERS\n"
add_pass:   .string "ADD: PASS\n"
sub_pass:   .string "SUB: PASS\n"
mv_pass:    .string "MV: PASS\n"
x0_pass:    .string "WRITABLE X0: PASS\n"
wrap_pass:  .string "16-BIT WRAP: PASS\n"
all_pass:   .string "TEST 01 RESULT: PASS\n"
add_bad:    .string "TEST 01 RESULT: FAIL AT ADD\n"
sub_bad:    .string "TEST 01 RESULT: FAIL AT SUB\n"
mv_bad:     .string "TEST 01 RESULT: FAIL AT MV\n"
x0_bad:     .string "TEST 01 RESULT: FAIL AT X0\n"
wrap_bad:   .string "TEST 01 RESULT: FAIL AT WRAP\n"
