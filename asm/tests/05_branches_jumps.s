# Test 05: branches and jumps
# It checks branch meanings, PC+2, links, and exact offset limits.

.text

start:
    li16 a0, title
    ecall 0x012

    # Equality and zero branches.
    li x3, 5
    li x4, 5
    beq x3, x4, beq_ok
    j branch_fail
beq_ok:
    li x4, 6
    bne x3, x4, bne_ok
    j branch_fail
bne_ok:
    li x3, 0
    bz x3, bz_ok
    j branch_fail
bz_ok:
    li x3, 1
    bnz x3, bnz_ok
    j branch_fail
bnz_ok:

    # These two branches must not be taken.
    li x3, 5
    li x4, 6
    beq x3, x4, unexpected_beq
    j beq_not_taken_ok
unexpected_beq:
    j branch_fail
beq_not_taken_ok:
    li x4, 5
    bne x3, x4, unexpected_bne
    j bne_not_taken_ok
unexpected_bne:
    j branch_fail
bne_not_taken_ok:

    # Signed and unsigned relation branches.
    li x3, -1
    li x4, 1
    blt x3, x4, blt_ok
    j branch_fail
blt_ok:
    bge x4, x3, bge_ok
    j branch_fail
bge_ok:
    li x3, 1
    li16 x4, 0xFFFF
    bltu x3, x4, bltu_ok
    j branch_fail
bltu_ok:
    bgeu x4, x3, branch_ok
    j branch_fail
branch_ok:
    li16 a0, branch_pass
    ecall 0x012

    # Direct J skips the failure jump.
    j direct_j_ok
    j jump_fail
direct_j_ok:

    # JAL stores PC+2 in ra, and JR returns to it.
    jal ra, jal_function
after_jal:

    # JALR also stores PC+2 in ra and jumps through a register.
    li16 x3, jalr_function
    jalr ra, x3
after_jalr:
    li16 a0, jump_pass
    ecall 0x012
    j after_functions

jal_function:
    li16 x4, after_jal
    beq ra, x4, jal_link_ok
    j jump_fail
jal_link_ok:
    jr ra

jalr_function:
    li16 x4, after_jalr
    beq ra, x4, jalr_link_ok
    j jump_fail
jalr_link_ok:
    jr ra

after_functions:
    # Exact positive branch boundary: +14 bytes.
    li x3, 0
    li x5, 0
    bz x3, branch_plus14_target
    addi x5, 1
    addi x5, 1
    addi x5, 1
    addi x5, 1
    addi x5, 1
    addi x5, 1
    addi x5, 1
branch_plus14_target:
    li x4, 0
    beq x5, x4, plus14_ok
    j boundary_fail
plus14_ok:

    # Exact negative branch boundary: -16 bytes.
    li x3, 2
    li x5, 0
branch_minus16_target:
    addi x3, -1
    addi x5, 0
    addi x5, 0
    addi x5, 0
    addi x5, 0
    addi x5, 0
    addi x5, 0
    bnz x3, branch_minus16_target
    li x4, 0
    beq x3, x4, branch_boundaries_ok
    j boundary_fail
branch_boundaries_ok:
    li16 a0, branch_boundary_pass
    ecall 0x012

    # Continue at the fixed J boundary test area.
    li x5, 0
    j jump_plus510_source

branch_fail:
    li16 a0, branch_bad
    ecall 0x012
    ecall 0x3FF
jump_fail:
    li16 a0, jump_bad
    ecall 0x012
    ecall 0x3FF
boundary_fail:
    li16 a0, boundary_bad
    ecall 0x012
    ecall 0x3FF

    # From 0x0200, the target at 0x0400 is exactly PC+2+510.
.org 0x0200
jump_plus510_source:
    j jump_boundary_target

.org 0x0400
jump_boundary_target:
    addi x5, 1
    li x4, 1
    beq x5, x4, go_to_minus512
    j jump_boundaries_done

go_to_minus512:
    j jump_minus512_source

jump_boundaries_done:
    li x4, 2
    beq x5, x4, jump_boundaries_ok
    li16 a0, boundary_bad
    ecall 0x012
    ecall 0x3FF
jump_boundaries_ok:
    li16 a0, jump_boundary_pass
    ecall 0x012
    li16 a0, all_pass
    ecall 0x012
    ecall 0x3FF

    # From 0x05FE, jumping to 0x0400 is exactly PC+2-512.
.org 0x05FE
jump_minus512_source:
    j jump_boundary_target

.data
title:                .string "TEST 05 - BRANCHES AND JUMPS\n"
branch_pass:          .string "ALL BRANCH CONDITIONS: PASS\n"
jump_pass:            .string "J JAL JR AND JALR: PASS\n"
branch_boundary_pass: .string "BRANCH -16 AND +14: PASS\n"
jump_boundary_pass:   .string "JUMP -512 AND +510: PASS\n"
all_pass:             .string "TEST 05 RESULT: PASS\n"
branch_bad:           .string "TEST 05 RESULT: FAIL AT BRANCH\n"
jump_bad:             .string "TEST 05 RESULT: FAIL AT JUMP OR LINK\n"
boundary_bad:         .string "TEST 05 RESULT: FAIL AT BOUNDARY\n"
