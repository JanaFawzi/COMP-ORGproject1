# ZX16 Instruction-Set Simulator

A from-scratch simulator for the ZX16, a 16-bit RISC instruction set built
for CSCE 2303 (Computer Organization & Assembly). It executes the full
ZX16 ISA, drives a tiled 320×240 graphics display, handles audio and
keyboard input through system calls, and ships with an interactive GUI
and a real debugger. **Snake**, written entirely in ZX16 assembly, runs on
top of it as the capstone game.

**Try it live:** https://janafawzi.github.io/COMP-ORGproject1/

---

## Features

- **Full ISA** — R/I/B/S/L/J/U/SYS-type instructions, all 8 general-purpose
  registers (x0 is a real register, not hardwired to zero)
- **System calls** — console I/O, string/int input, audio (`play_tone`,
  `set_volume`, `stop_audio`), keyboard polling, a seeded PRNG, and
  register/memory dumps
- **64 KB memory** with a tiled graphics region: a 20×15 tile map, 16
  tile definitions, and a 16-colour palette, rendered through a full
  pixel pipeline
- **GUI + debugger** — live display, Run/Pause/Step/Reset, register and
  memory views, disassembly with PC highlighting, breakpoints, step-over,
  run-to-cursor, and live register/memory editing
- **Snake**, playable via keyboard, with food placement driven by the
  same seeded PRNG so runs are reproducible
- **Bonus:** a browser build (WASM) with an in-page code editor —
  assemble and run ZX16 assembly without leaving the page

---

## Repository Structure

```
COMP-ORGproject1/
├── asm/              ZX16 assembly test programs + snake.s
├── assembler/        ZX16 assembler (source → .bin)
├── reference/         reference simulator + comparison/test tooling
├── docs/              test results and documentation
├── src/
│   ├── core/          CPU, memory, decoder, registers, ECALL, graphics, debugger
│   ├── app/           desktop entry point
│   └── desktop/       GUI + build tooling
├── web/               bonus browser build (WASM + in-browser IDE)
└── README.md
```

---

## Getting Started

### Requirements
- C++17 compiler + CMake (desktop build)
- Python 3 (assembler + reference simulator)
- Emscripten SDK (only needed to rebuild the web version locally — the
  hosted link above already runs the compiled build)

> On Windows, if `python` opens a Microsoft Store prompt instead of
> running, use `py` instead, or disable the alias under *Settings → Apps →
> Advanced app settings → App execution aliases*.

### Desktop GUI

```bash
mkdir build && cd build
cmake ..
cmake --build .
./zx16sim
```

### Web build (bonus)

The hosted build above needs nothing locally. To rebuild from source:

```powershell
src/desktop/tools/install_emscripten.ps1     # one-time SDK setup
src/desktop/tools/build_web_page.ps1          # compiles the backend + frontend
```

WASM won't load over `file://` — serve `web/` locally before opening it:

```bash
cd web && npx serve .
```

### Assembling & Running a Program

```bash
py assembler/zx16asm.py program.asm -o program.bin
py reference/zx16sim.py program.bin
```

---

## Testing

10+ documented ZX16 assembly test programs cover arithmetic/logic, shifts,
branches, loads/stores, the calling convention, and every system service,
each with expected-vs-actual results recorded in `docs/`. Corner cases
specifically targeted include x0's read/write behavior, sign handling at
immediate boundaries, `LB` vs `LBU` extension, control-flow range limits,
memory alignment, the exact PRNG stream for a given seed, and the tile
nibble/palette bit-replication order in the graphics pipeline.

## Known Limitations

- Interrupts and interrupt-related instructions (`EBREAK`, `RETI`, `EI`,
  `DI`, `MFEPC`, `MTEPC`, `STEP`) are out of scope for this project.
- `regs_dump`/`mem_dump` output formatting is our own design choice —
  only `print_int`/`print_char`/`halt` are required to match the
  reference simulator byte-for-byte.

## AI Assistance

We used AI help for parts of the bonus web frontend — build scaffolding
(CMake, Emscripten scripts) and general UI wiring that isn't part of the
graded rubric but was needed to ship the browser build. The core simulator
itself — CPU, memory, system calls, graphics pipeline, debugger, and the
Snake game — is our own work.

---

## Contributors

| Developer | Topics |
|---|---|
| Mariam AbdelRahim | CPU core & instruction set (registers, decoder, fetch–decode–execute, reset/load) |
| Jana Fawzi | Memory & system calls, GUI and debugger |
| Yahia ElHodeiby | Graphics pipeline, Snake game, assembler |
