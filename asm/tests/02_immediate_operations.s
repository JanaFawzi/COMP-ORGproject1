# Test 02: immediate instructions
# It covers signed imm7, logical masks, LUI and AUIPC.

.text

start:
    li16 a0, title
    ecall 0x012

    # Lowest signed imm7 value: -64
    li x3, 0
    addi x3, -64
    li x4, -64
    beq x3, x4, neg64_ok
    j signed_fail
neg64_ok:

    # Highest signed imm7 value: +63
    li x3, 0
    addi x3, 63
    li x4, 63
    beq x3, x4, signed_ok
    j signed_fail
signed_ok:
    li16 a0, signed_pass
    ecall 0x012

    # ORI uses the zero-extended 7-bit mask 0x7F.
    li16 x3, 0x1200
    ori x3, 0x7F
    li16 x4, 0x127F
    beq x3, x4, ori_ok
    j mask_fail
ori_ok:

    # ANDI uses the same zero-extended mask.
    li16 x3, 0xFFFF
    andi x3, 0x7F
    li16 x4, 0x007F
    beq x3, x4, andi_ok
    j mask_fail
andi_ok:

    # XORI uses the same zero-extended mask.
    li16 x3, 0xAAAA
    xori x3, 0x7F
    li16 x4, 0xAAD5
    beq x3, x4, mask_ok
    j mask_fail
mask_ok:
    li16 a0, mask_pass
    ecall 0x012

    # Signed and unsigned immediate comparisons are different.
    li x3, -1
    slti x3, 1
    li x4, 1
    beq x3, x4, slti_ok
    j compare_fail
slti_ok:
    li16 x3, 0xFFFF
    sltui x3, 1
    li x4, 0
    beq x3, x4, compare_ok
    j compare_fail
compare_ok:
    li16 a0, compare_pass
    ecall 0x012

    # LUI places imm9 in bits 15..7.
    lui x3, 0x1AB
    li16 x4, 0xD580
    beq x3, x4, lui_ok
    j upper_fail
lui_ok:

    # AUIPC with zero returns this instruction's own address.
auipc_here:
    auipc x3, 0
    li16 x4, auipc_here
    beq x3, x4, upper_ok
    j upper_fail
upper_ok:
    li16 a0, upper_pass
    ecall 0x012

    li16 a0, all_pass
    ecall 0x012
    ecall 0x3FF

signed_fail:
    li16 a0, signed_bad
    ecall 0x012
    ecall 0x3FF
mask_fail:
    li16 a0, mask_bad
    ecall 0x012
    ecall 0x3FF
compare_fail:
    li16 a0, compare_bad
    ecall 0x012
    ecall 0x3FF
upper_fail:
    li16 a0, upper_bad
    ecall 0x012
    ecall 0x3FF

.data
title:         .string "TEST 02 - IMMEDIATE OPERATIONS\n"
signed_pass:   .string "SIGNED IMM7 -64 AND +63: PASS\n"
mask_pass:     .string "ORI ANDI XORI MASKS: PASS\n"
compare_pass:  .string "SLTI AND SLTUI: PASS\n"
upper_pass:    .string "LUI AND AUIPC: PASS\n"
all_pass:      .string "TEST 02 RESULT: PASS\n"
signed_bad:    .string "TEST 02 RESULT: FAIL AT SIGNED IMM7\n"
mask_bad:      .string "TEST 02 RESULT: FAIL AT LOGICAL MASK\n"
compare_bad:   .string "TEST 02 RESULT: FAIL AT COMPARISON\n"
upper_bad:     .string "TEST 02 RESULT: FAIL AT LUI OR AUIPC\n"
