# Test 06: stack and subroutines
# It checks stack reset, PUSH/POP, CALL/RET, and nested calls.

.text

start:
    li16 a0, title
    ecall 0x012

    # The stack starts at 0xF000.
    li16 x3, 0xF000
    beq sp, x3, reset_ok
    j reset_fail
reset_ok:
    # PUSH moves down by two and POP restores the value and SP.
    li16 x3, 0x1234
    push x3
    li16 x4, 0xEFFE
    beq sp, x4, push_address_ok
    j stack_fail
push_address_ok:
    lw x5, 0(sp)
    beq x5, x3, push_value_ok
    j stack_fail
push_value_ok:
    li x5, 0
    pop x5
    beq x5, x3, pop_value_ok
    j stack_fail
pop_value_ok:
    li16 x4, 0xF000
    beq sp, x4, stack_ok
    j stack_fail
stack_ok:
    li16 a0, stack_pass
    ecall 0x012

    # A simple CALL adds two and RET comes back here.
    li x3, 5
    call add_two
after_add_two:
    li x4, 7
    beq x3, x4, call_ok
    j call_fail
call_ok:
    li16 a0, call_pass
    ecall 0x012

    # The outer function saves ra before making a nested CALL.
    li x3, 10
    call outer_function
after_outer:
    li x4, 13
    beq x3, x4, nested_value_ok
    j nested_fail
nested_value_ok:
    li16 x4, 0xF000
    beq sp, x4, nested_ok
    j nested_fail
nested_ok:
    li16 a0, nested_pass
    ecall 0x012

    # The empty stack returns to 0xF000, but data was pushed below it.
    li16 x4, 0xF000
    beq sp, x4, safe_sp_ok
    j safe_fail
safe_sp_ok:
    lbu x5, 0(x4)
    li x0, 0
    beq x5, x0, safe_ok
    j safe_fail
safe_ok:
    li16 a0, safe_pass
    ecall 0x012

    li16 a0, all_pass
    ecall 0x012
    ecall 0x3FF

add_two:
    addi x3, 2
    li16 x4, after_add_two
    beq ra, x4, add_two_return_ok
    j call_fail
add_two_return_ok:
    ret

outer_function:
    push ra
    addi x3, 1
    call inner_function
    pop ra
    ret

inner_function:
    addi x3, 2
    ret

reset_fail:
    li16 a0, reset_bad
    ecall 0x012
    ecall 0x3FF
stack_fail:
    li16 a0, stack_bad
    ecall 0x012
    ecall 0x3FF
call_fail:
    li16 a0, call_bad
    ecall 0x012
    ecall 0x3FF
nested_fail:
    li16 a0, nested_bad
    ecall 0x012
    ecall 0x3FF
safe_fail:
    li16 a0, safe_bad
    ecall 0x012
    ecall 0x3FF

.data
title:       .string "TEST 06 - STACK AND SUBROUTINES\n"
stack_pass:  .string "RESET SP PUSH AND POP: PASS\n"
call_pass:   .string "CALL AND RET: PASS\n"
nested_pass: .string "NESTED CALL: PASS\n"
safe_pass:   .string "STACK STORES BELOW 0XF000: PASS\n"
all_pass:    .string "TEST 06 RESULT: PASS\n"
reset_bad:   .string "TEST 06 RESULT: FAIL AT RESET SP\n"
stack_bad:   .string "TEST 06 RESULT: FAIL AT PUSH OR POP\n"
call_bad:    .string "TEST 06 RESULT: FAIL AT CALL OR RET\n"
nested_bad:  .string "TEST 06 RESULT: FAIL AT NESTED CALL\n"
safe_bad:    .string "TEST 06 RESULT: FAIL AT STACK SAFETY\n"
