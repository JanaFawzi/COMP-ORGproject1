# ZX16 Simulator

## Project layout

```text
assembler/                    ZX16 Python assembler
asm/                          Assembly source programs
asm/tests/                    Assembly test suites (to be added)
asm/bin/                      Generated binary images
docs/assembly_test_results.md Assembly test report (to be added)
docs/screenshots/             Test evidence screenshots
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
