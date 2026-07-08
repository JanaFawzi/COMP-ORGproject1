# Test 03: logic, shifts and comparisons
# Both register shifts and immediate shifts are tested.

.text

start:
    li16 a0, title
    ecall 0x012

    # Register logic operations.
    li16 x3, 0x0F0F
    li16 x4, 0x00F0
    or x3, x4
    li16 x4, 0x0FFF
    beq x3, x4, or_ok
    j logic_fail
or_ok:
    li16 x3, 0xF0FF
    li16 x4, 0x0FF0
    and x3, x4
    li16 x4, 0x00F0
    beq x3, x4, and_ok
    j logic_fail
and_ok:
    li16 x3, 0xAAAA
    li16 x4, 0x0F0F
    xor x3, x4
    li16 x4, 0xA5A5
    beq x3, x4, logic_ok
    j logic_fail
logic_ok:
    li16 a0, logic_pass
    ecall 0x012

    # Register shifts use the low four bits of the second register.
    li x3, 1
    li x4, 15
    sll x3, x4
    li16 x5, 0x8000
    beq x3, x5, sll_ok
    j register_shift_fail
sll_ok:
    li16 x3, 0x8000
    srl x3, x4
    li x5, 1
    beq x3, x5, srl_ok
    j register_shift_fail
srl_ok:
    li16 x3, 0x8000
    sra x3, x4
    li16 x5, 0xFFFF
    beq x3, x5, register_shift_ok
    j register_shift_fail
register_shift_ok:
    li16 a0, register_shift_pass
    ecall 0x012

    # Immediate shifts use an amount from 0 to 15.
    li x3, 1
    slli x3, 4
    li x4, 16
    beq x3, x4, slli_ok
    j immediate_shift_fail
slli_ok:
    li16 x3, 0x8000
    srli x3, 15
    li x4, 1
    beq x3, x4, srli_ok
    j immediate_shift_fail
srli_ok:
    li16 x3, 0x8000
    srai x3, 15
    li16 x4, 0xFFFF
    beq x3, x4, immediate_shift_ok
    j immediate_shift_fail
immediate_shift_ok:
    li16 a0, immediate_shift_pass
    ecall 0x012

    # -1 is less than 1 when signed, but not when unsigned.
    li16 x3, 0xFFFF
    li x4, 1
    slt x3, x4
    li x5, 1
    beq x3, x5, slt_ok
    j compare_fail
slt_ok:
    li16 x3, 0xFFFF
    sltu x3, x4
    li x5, 0
    beq x3, x5, compare_ok
    j compare_fail
compare_ok:
    li16 a0, compare_pass
    ecall 0x012

    li16 a0, all_pass
    ecall 0x012
    ecall 0x3FF

logic_fail:
    li16 a0, logic_bad
    ecall 0x012
    ecall 0x3FF
register_shift_fail:
    li16 a0, register_shift_bad
    ecall 0x012
    ecall 0x3FF
immediate_shift_fail:
    li16 a0, immediate_shift_bad
    ecall 0x012
    ecall 0x3FF
compare_fail:
    li16 a0, compare_bad
    ecall 0x012
    ecall 0x3FF

.data
title:                 .string "TEST 03 - LOGIC SHIFT AND COMPARE\n"
logic_pass:            .string "OR AND XOR: PASS\n"
register_shift_pass:   .string "SLL SRL SRA: PASS\n"
immediate_shift_pass:  .string "SLLI SRLI SRAI: PASS\n"
compare_pass:          .string "SLT AND SLTU: PASS\n"
all_pass:              .string "TEST 03 RESULT: PASS\n"
logic_bad:             .string "TEST 03 RESULT: FAIL AT LOGIC\n"
register_shift_bad:    .string "TEST 03 RESULT: FAIL AT REGISTER SHIFT\n"
immediate_shift_bad:   .string "TEST 03 RESULT: FAIL AT IMMEDIATE SHIFT\n"
compare_bad:           .string "TEST 03 RESULT: FAIL AT COMPARISON\n"
