# Test 10: full integration corner cases
# One program combines registers, memory, control flow, stack, services, and graphics.

.text

start:
    li16 a0, title
    ecall 0x012

    # Register corner cases: writable x0 and 16-bit wraparound.
    li x0, 9
    li x3, 1
    add x0, x3
    li x3, 10
    beq x0, x3, writable_x0_ok
    li16 a0, reg_bad
    ecall 0x012
    ecall 0x3FF
writable_x0_ok:
    li16 x3, 0xFFFF
    li x4, 1
    add x3, x4
    li x4, 0
    beq x3, x4, register_ok
    li16 a0, reg_bad
    ecall 0x012
    ecall 0x3FF
register_ok:
    li16 a0, reg_pass
    ecall 0x012

    # Memory corner cases: odd-address word access and sign extension.
    li16 x3, work_area
    li16 x4, 0xCAFE
    sw x4, 1(x3)
    lw x5, 1(x3)
    beq x5, x4, odd_word_ok
    li16 a0, memory_bad
    ecall 0x012
    ecall 0x3FF
odd_word_ok:
    lbu x5, 1(x3)
    li16 x4, 0x00FE
    beq x5, x4, low_byte_ok
    li16 a0, memory_bad
    ecall 0x012
    ecall 0x3FF
low_byte_ok:
    lb x5, 2(x3)
    li16 x4, 0xFFCA
    beq x5, x4, memory_ok
    li16 a0, memory_bad
    ecall 0x012
    ecall 0x3FF
memory_ok:
    li16 a0, memory_pass
    ecall 0x012

    # Control flow: loop, signed compare, and unsigned compare.
    li x3, 0
    li x4, 1
sum_loop:
    add x3, x4
    addi x4, 1
    li x5, 5
    blt x4, x5, sum_loop
    li x5, 10
    beq x3, x5, loop_ok
    li16 a0, control_bad
    ecall 0x012
    ecall 0x3FF
loop_ok:
    li16 x3, 0xFFFF
    li x4, 1
    slt x3, x4
    li x5, 1
    beq x3, x5, signed_compare_ok
    li16 a0, control_bad
    ecall 0x012
    ecall 0x3FF
signed_compare_ok:
    li16 x3, 0xFFFF
    li x4, 1
    sltu x3, x4
    li x5, 0
    beq x3, x5, control_ok
    li16 a0, control_bad
    ecall 0x012
    ecall 0x3FF
control_ok:
    li16 a0, control_pass
    ecall 0x012

    # Stack and subroutines: push/pop plus nested call preserving ra.
    li16 x3, 0xBEEF
    push x3
    li16 x4, 0xEFFC
    beq sp, x4, push_sp_ok
    li16 a0, stack_bad
    ecall 0x012
    ecall 0x3FF
push_sp_ok:
    pop x4
    beq x4, x3, pop_value_ok
    li16 a0, stack_bad
    ecall 0x012
    ecall 0x3FF
pop_value_ok:
    li x3, 7
    call outer_function
    li x4, 10
    beq x3, x4, call_value_ok
    li16 a0, stack_bad
    ecall 0x012
    ecall 0x3FF
call_value_ok:
    li16 x4, 0xEFFE
    beq sp, x4, stack_ok
    li16 a0, stack_bad
    ecall 0x012
    ecall 0x3FF
stack_ok:
    li16 a0, stack_pass
    ecall 0x012
    j after_functions

outer_function:
    push ra
    addi x3, 1
    call inner_function
    pop ra
    ret

inner_function:
    addi x3, 2
    ret

after_functions:
    # Services: deterministic RNG, keyboard no-key, and print_char.
    li16 a0, service_mark
    ecall 0x012
    li a0, '!'
    ecall 0x001
    li a0, 10
    ecall 0x001
    li16 a0, 0x1234
    ecall 0x031
    ecall 0x032
    li16 x3, 0x3830
    beq a0, x3, rng_ok
    li16 a0, service_bad
    ecall 0x012
    ecall 0x3FF
rng_ok:
    ecall 0x030
    li x3, 0
    beq a0, x3, services_ok
    li16 a0, service_bad
    ecall 0x012
    ecall 0x3FF
services_ok:
    li16 a0, service_pass
    ecall 0x012

    # Graphics edge addresses: last tile-map cell, last tile byte, last palette byte.
    li16 x3, 0xF128
    li x4, 15
    sb x4, 3(x3)
    lbu x5, 3(x3)
    beq x5, x4, graphics_tile_ok
    li16 a0, graphics_bad
    ecall 0x012
    ecall 0x3FF
graphics_tile_ok:
    li16 x3, 0xF9F8
    li16 x4, 0x00AB
    sb x4, 7(x3)
    lbu x5, 7(x3)
    beq x5, x4, graphics_def_ok
    li16 a0, graphics_bad
    ecall 0x012
    ecall 0x3FF
graphics_def_ok:
    li16 x3, 0xFA08
    li16 x4, 0x00E3
    sb x4, 7(x3)
    lbu x5, 7(x3)
    beq x5, x4, graphics_ok
    li16 a0, graphics_bad
    ecall 0x012
    ecall 0x3FF
graphics_ok:
    li16 a0, graphics_pass
    ecall 0x012

    li16 a0, all_pass
    ecall 0x012
    ecall 0x3FF

.data
work_area:     .space 8
title:         .string "TEST 10 - FULL INTEGRATION CORNER CASES\n"
reg_pass:      .string "REGISTER WRAP AND X0: PASS\n"
memory_pass:   .string "ODD MEMORY AND SIGN EXTEND: PASS\n"
control_pass:  .string "CONTROL FLOW COMPARES: PASS\n"
stack_pass:    .string "STACK AND NESTED CALL: PASS\n"
service_mark:  .string "SERVICE PRINT CHAR SAMPLE: "
service_pass:  .string "RNG KEYBOARD SERVICES: PASS\n"
graphics_pass: .string "GRAPHICS EDGE VRAM: PASS\n"
all_pass:      .string "TEST 10 RESULT: PASS\n"
reg_bad:       .string "TEST 10 RESULT: FAIL AT REGISTERS\n"
memory_bad:    .string "TEST 10 RESULT: FAIL AT MEMORY\n"
control_bad:   .string "TEST 10 RESULT: FAIL AT CONTROL FLOW\n"
stack_bad:     .string "TEST 10 RESULT: FAIL AT STACK OR CALL\n"
service_bad:   .string "TEST 10 RESULT: FAIL AT SERVICES\n"
graphics_bad:  .string "TEST 10 RESULT: FAIL AT GRAPHICS\n"
