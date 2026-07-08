#!/usr/bin/env python3
"""Run the six assembly test binaries on the provided reference simulator."""

from pathlib import Path
import importlib.util


ROOT = Path(__file__).resolve().parents[1]
BIN_DIR = ROOT / "asm" / "bin"
REF_PATH = ROOT / "reference" / "zx16sim.py"


def load_reference():
    spec = importlib.util.spec_from_file_location("zx16ref", REF_PATH)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def read_string(sim, address, limit=160):
    data = []
    for offset in range(limit):
        byte = sim.mem[(address + offset) & 0xFFFF]
        if byte == 0:
            break
        data.append(byte)
    return bytes(data).decode("ascii", errors="replace").replace("\n", "\\n")


def run_one(module, bin_path):
    sim = module.ZX16()
    sim.load(bin_path.read_bytes(), 0x0000)

    try:
        sim.run()
        error = ""
    except Exception as exc:
        error = str(exc)

    message_address = sim.reg[6] & 0xFFFF
    message = read_string(sim, message_address)
    passed = "RESULT: PASS" in message

    return {
        "name": bin_path.name,
        "passed": passed,
        "message": message,
        "pc": sim.pc & 0xFFFF,
        "sp": sim.reg[2] & 0xFFFF,
        "cycles": sim.cycles,
        "error": error,
    }


def main():
    module = load_reference()
    tests = sorted(BIN_DIR.glob("0[1-6]_*.bin"))

    for bin_path in tests:
        result = run_one(module, bin_path)
        status = "PASS" if result["passed"] else "FAIL"
        print(
            f"{result['name']}: {status} | "
            f"{result['message']} | "
            f"PC=0x{result['pc']:04X} SP=0x{result['sp']:04X} "
            f"cycles={result['cycles']}"
        )
        if result["error"]:
            print(f"  error: {result['error']}")


if __name__ == "__main__":
    main()
