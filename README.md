# ZX16 Simulator

## Project layout

```text
assembler/                    ZX16 Python assembler
asm/                          Assembly source programs
asm/tests/                    Assembly test suites
asm/bin/                      Generated binary images
docs/assembly_test_results.md Assembly test report and actual results
docs/screenshots/             Test evidence screenshots
reference/                    Provided Python reference simulator
src/                          C++ simulator implementation
```

## Assemble a program

```powershell
python assembler/zx16asm.py asm/tests/program.s -o asm/bin/program.bin -f bin
```

The supplied assembler emits a full 64 KB memory image. `ProgramLoader` preserves
the absolute addresses in such images, including code at `0x0020`, data at
`0x8000`, and BSS at `0x9000`. Smaller legacy `.bin` files are still loaded at
`0x0020`.

## Run an assembly test

Pass the generated image to the simulator:

```powershell
cmake-build-debug/zx16sim.exe asm/bin/01_arithmetic_registers.bin
```

The first six documented tests, expected results, actual results, and screenshots
are in [`docs/assembly_test_results.md`](docs/assembly_test_results.md).

## Run the reference comparison

The provided reference simulator is saved as:

```text
reference/zx16sim.py
```

After assembling the tests, run:

```powershell
python reference/compare_assembly_tests.py
```
