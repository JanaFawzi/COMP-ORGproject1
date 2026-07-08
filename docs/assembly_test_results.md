# ZX16 Assembly Test Results

This report covers the first three ZX16 assembly tests.

Each program checks its own answers. If an answer is wrong, it prints the name of
the failed part and stops. If every answer is correct, it prints a final PASS line.

## How to assemble and run

Assemble one test:

```powershell
python assembler/zx16asm.py asm/tests/01_arithmetic_registers.s -o asm/bin/01_arithmetic_registers.bin -f bin
```

Run the generated test image:

```powershell
cmake-build-debug/zx16sim.exe asm/bin/01_arithmetic_registers.bin
```

The simulator loads the image, runs it until `halt`, and opens the debugger with
the console result visible.

## Test 01: Arithmetic and registers

Source: [`01_arithmetic_registers.s`](../asm/tests/01_arithmetic_registers.s)

### What this program tests

- `ADD`: 20 + 22 must give 42.
- `SUB`: 50 - 8 must give 42.
- `MV`: the complete value `0xBEEF` must be copied.
- `x0`: writing 7, adding 5, and reading it again must give 12.
- 16-bit wrap: `0xFFFF + 1` must give `0x0000`.

The `x0` check is important because ZX16 does not have a hardwired zero register.

### Expected console output

```text
TEST 01 - ARITHMETIC AND REGISTERS
ADD: PASS
SUB: PASS
MV: PASS
WRITABLE X0: PASS
16-BIT WRAP: PASS
TEST 01 RESULT: PASS
```

### Actual result

The actual output matched the expected output. The final result was PASS.

![Test 01 simulator output](screenshots/assembly-tests/01_arithmetic_registers.png)

## Test 02: Immediate operations

Source: [`02_immediate_operations.s`](../asm/tests/02_immediate_operations.s)

### What this program tests

- Signed imm7 lowest value: `-64`.
- Signed imm7 highest value: `+63`.
- `ORI`, `ANDI`, and `XORI` use the zero-extended mask `0x7F`.
- `SLTI` compares signed values.
- `SLTUI` compares unsigned values.
- `LUI 0x1AB` must give `0xD580`.
- `AUIPC` uses the address of its own instruction.

### Expected values

| Check | Expected result |
|---|---:|
| `0 + (-64)` | `0xFFC0` |
| `0 + 63` | `0x003F` |
| `0x1200 OR 0x007F` | `0x127F` |
| `0xFFFF AND 0x007F` | `0x007F` |
| `0xAAAA XOR 0x007F` | `0xAAD5` |
| `LUI 0x1AB` | `0xD580` |

### Expected console output

```text
TEST 02 - IMMEDIATE OPERATIONS
SIGNED IMM7 -64 AND +63: PASS
ORI ANDI XORI MASKS: PASS
SLTI AND SLTUI: PASS
LUI AND AUIPC: PASS
TEST 02 RESULT: PASS
```

### Actual result

The actual output matched the expected output. The final result was PASS.

![Test 02 simulator output](screenshots/assembly-tests/02_immediate_operations.png)

## Test 03: Logic, shifts and comparisons

Source: [`03_logic_shift_compare.s`](../asm/tests/03_logic_shift_compare.s)

### What this program tests

- Register logic: `OR`, `AND`, and `XOR`.
- Register shifts: `SLL`, `SRL`, and `SRA`.
- Immediate shifts: `SLLI`, `SRLI`, and `SRAI`.
- Shift amount boundary of 15.
- `SRL` fills the left side with zero.
- `SRA` keeps the negative sign bit.
- `SLT` treats `0xFFFF` as signed -1.
- `SLTU` treats `0xFFFF` as unsigned 65535.

### Expected values

| Check | Expected result |
|---|---:|
| `0x0F0F OR 0x00F0` | `0x0FFF` |
| `0xF0FF AND 0x0FF0` | `0x00F0` |
| `0xAAAA XOR 0x0F0F` | `0xA5A5` |
| `1 << 15` | `0x8000` |
| logical `0x8000 >> 15` | `0x0001` |
| arithmetic `0x8000 >> 15` | `0xFFFF` |
| signed `-1 < 1` | `1` |
| unsigned `65535 < 1` | `0` |

### Expected console output

```text
TEST 03 - LOGIC SHIFT AND COMPARE
OR AND XOR: PASS
SLL SRL SRA: PASS
SLLI SRLI SRAI: PASS
SLT AND SLTU: PASS
TEST 03 RESULT: PASS
```

### Actual result

The actual output matched the expected output. The final result was PASS.

![Test 03 simulator output](screenshots/assembly-tests/03_logic_shift_compare.png)

## Final result

| Test | Simulator result |
|---|---|
| 01 Arithmetic and registers | PASS |
| 02 Immediate operations | PASS |
| 03 Logic, shifts and comparisons | PASS |

The supplied assembler produced all three 64 KB images successfully. The
programs used the `print_string` extension service only to make their result easy
to read in the simulator console.

The separate reference simulator `zx16sim.py` is not present in this repository,
so a reference-simulator screenshot has not been claimed here. That comparison
can be added when the reference simulator is provided.
