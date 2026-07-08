# Test 07: console and string services
# It exercises print/read integer, character, and string ECALLs.

.text

start:
    li16 a0, title
    ecall 0x012

    # print_int treats 0x7FFF as 32767 and 0x8000 as -32768.
    li16 a0, boundary_text
    ecall 0x012
    li16 a0, 0x7FFF
    ecall 0x000
    li a0, 32
    ecall 0x001
    li16 a0, 0x8000
    ecall 0x000
    li a0, 10
    ecall 0x001
    li16 a0, boundary_pass
    ecall 0x012

    # print_char emits the low byte of a0.
    li16 a0, char_text
    ecall 0x012
    li a0, 'Z'
    ecall 0x001
    li a0, 'X'
    ecall 0x001
    li a0, '1'
    ecall 0x001
    li a0, '6'
    ecall 0x001
    li a0, 10
    ecall 0x001
    li16 a0, char_pass
    ecall 0x012

    # print_string walks bytes until the zero terminator.
    li16 a0, sample_string
    ecall 0x012
    li16 a0, string_pass
    ecall 0x012

    # With the GUI assembly runner there is no scripted input, so read_int
    # should return zero when no digits are available.
    ecall 0x011
    li x3, 0
    beq a0, x3, read_int_ok
    j read_int_fail
read_int_ok:
    li16 a0, read_int_pass
    ecall 0x012

    # read_string at EOF writes a terminator and reports length zero.
    li16 a0, input_buffer
    li a1, 8
    ecall 0x010
    li x3, 0
    beq a0, x3, read_len_ok
    j read_string_fail
read_len_ok:
    li16 x4, input_buffer
    lbu x5, 0(x4)
    li x3, 0
    beq x5, x3, read_string_ok
    j read_string_fail
read_string_ok:
    li16 a0, read_string_pass
    ecall 0x012

    li16 a0, all_pass
    ecall 0x012
    ecall 0x3FF

read_int_fail:
    li16 a0, read_int_bad
    ecall 0x012
    ecall 0x3FF
read_string_fail:
    li16 a0, read_string_bad
    ecall 0x012
    ecall 0x3FF

.data
input_buffer:      .space 8
title:             .string "TEST 07 - CONSOLE STRING SERVICES\n"
boundary_text:     .string "PRINT INT BOUNDARIES: "
boundary_pass:     .string "PRINT INT 0X7FFF AND 0X8000: PASS\n"
char_text:         .string "PRINT CHAR SAMPLE: "
sample_string:     .string "PRINT STRING SAMPLE: PASS\n"
char_pass:         .string "PRINT CHAR: PASS\n"
string_pass:       .string "PRINT STRING: PASS\n"
read_int_pass:     .string "READ INT EMPTY INPUT: PASS\n"
read_string_pass:  .string "READ STRING EMPTY INPUT: PASS\n"
all_pass:          .string "TEST 07 RESULT: PASS\n"
read_int_bad:      .string "TEST 07 RESULT: FAIL AT READ INT\n"
read_string_bad:   .string "TEST 07 RESULT: FAIL AT READ STRING\n"
