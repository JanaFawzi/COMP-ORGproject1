# ZX16 Assembly Tests

This file shows the result of the first six assembly tests.

## Running a test

```powershell
python assembler/zx16asm.py asm/tests/01_arithmetic_registers.s -o asm/bin/01_arithmetic_registers.bin -f bin
cmake-build-debug/zx16sim.exe asm/bin/01_arithmetic_registers.bin
```

Each program prints `PASS` when the result is correct. If a check fails, it prints
`FAIL` and stops.

## Test 01 - Arithmetic and registers

File: [`01_arithmetic_registers.s`](../asm/tests/01_arithmetic_registers.s)

Checks: `ADD`, `SUB`, `MV`, 16-bit overflow, and reading/writing `x0`.

Expected: all calculations are correct. `x0` keeps the value written to it and
`0xFFFF + 1` becomes `0x0000`.

Actual: PASS.

![Test 01 output](screenshots/assembly-tests/01_arithmetic_registers.png)

## Test 02 - Immediate instructions

File: [`02_immediate_operations.s`](../asm/tests/02_immediate_operations.s)

Checks: signed immediate limits `-64` and `+63`, logical immediate behavior,
`SLTI`, `SLTUI`, `LUI`, and `AUIPC`.

Expected: the signed limits work, `ORI` uses a zero-extended mask,
`ANDI/XORI` use the sign-extended immediate value, and `LUI 0x1AB` gives
`0xD580`.

Actual: PASS.

![Test 02 output](screenshots/assembly-tests/02_immediate_operations.png)

## Test 03 - Logic, shifts and comparisons

File: [`03_logic_shift_compare.s`](../asm/tests/03_logic_shift_compare.s)

Checks: `OR`, `AND`, `XOR`, register shifts, immediate shifts, `SLT`, and `SLTU`.

Expected: logical shift of `0x8000` by 15 gives `0x0001`, arithmetic shift gives
`0xFFFF`, signed `-1 < 1` is true, and unsigned `65535 < 1` is false.

Actual: PASS.

![Test 03 output](screenshots/assembly-tests/03_logic_shift_compare.png)

## Test 04 - Memory

File: [`04_memory_load_store.s`](../asm/tests/04_memory_load_store.s)

Checks: `LB`, `LBU`, `LW`, `SB`, `SW`, little-endian words, odd word access, and
a write to the tile map at `0xF000`.

Expected: `LB` of `0x80` gives `0xFF80`, `LBU` gives `0x0080`, odd-address
`SW/LW` still use little-endian order, and the tile-map write is stored.

Actual: PASS.

![Test 04 output](screenshots/assembly-tests/04_memory_load_store.png)

## Test 05 - Branches and jumps

File: [`05_branches_jumps.s`](../asm/tests/05_branches_jumps.s)

Checks: all branch conditions, `J`, `JAL`, `JR`, `JALR`, and PC-relative limits.

Expected: branches work at `-16` and `+14`, jumps work at `-512` and `+510`, and
`JAL/JALR` save `PC+2` as the return address.

Actual: PASS.

![Test 05 output](screenshots/assembly-tests/05_branches_jumps.png)

## Test 06 - Stack and functions

File: [`06_stack_subroutines.s`](../asm/tests/06_stack_subroutines.s)

Checks: reset SP, `PUSH`, `POP`, `CALL`, `RET`, and a nested call.

Expected: SP starts at `0xF000`, a push moves it to `0xEFFE`, a pop restores it,
and stack data is stored below `0xF000`.

Actual: PASS.

![Test 06 output](screenshots/assembly-tests/06_stack_subroutines.png)

## My simulator results

| Test | Result |
|---|---|
| 01 Arithmetic and registers | PASS |
| 02 Immediate instructions | PASS |
| 03 Logic, shifts and comparisons | PASS |
| 04 Memory | PASS |
| 05 Branches and jumps | PASS |
| 06 Stack and functions | PASS |

The supplied assembler built all six programs. The programs use `print_string`
to show their results in the console.

## Reference simulator check

I also ran the same `.bin` files with `reference/zx16sim.py`.

The reference simulator only prints with `print_int` and `print_char`. These
tests use `print_string`, so the reference console output is empty. I checked the
final message address in `x6` instead.

| Test | My simulator | Reference simulator | Note |
|---|---|---|---|
| 01 Arithmetic and registers | PASS | PASS | Same result |
| 02 Immediate instructions | PASS | PASS | Same result |
| 03 Logic, shifts and comparisons | PASS | PASS | Same result |
| 04 Memory | PASS | PASS | Same result |
| 05 Branches and jumps | PASS | PASS | Same result |
| 06 Stack and functions | PASS | PASS | Same result |

So the six tests now pass in my simulator and in the provided reference
simulator.
