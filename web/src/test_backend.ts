declare const require: any;
declare const __dirname: string;
declare const process: any;

const fs = require("fs");
const path = require("path");

type LocateFile = (fileName: string) => string;

type Zx16Module = {
    cwrap(name: string, returnType: string | null, argTypes: string[]): (...args: number[]) => any;
};

type CreateZx16Backend = (options: { locateFile: LocateFile }) => Promise<Zx16Module>;

const createZx16Backend: CreateZx16Backend = require("../../web-build/zx16_backend.js");

function hex16(value: number): string {
    return "0x" + (value & 0xFFFF).toString(16).toUpperCase().padStart(4, "0");
}

function makeSys(service: number): number {
    return ((service & 0x3FF) << 6) | 7;
}

function loadWord(loadByte, address: number, value: number): void {
    loadByte(address, value & 0xFF);
    loadByte(address + 1, (value >> 8) & 0xFF);
}

function loadBin(loadByte, filePath: string): number {
    const bytes = fs.readFileSync(filePath);
    const loadAddress = bytes.length === 65536 ? 0 : 0x0020;

    for (let i = 0; i < bytes.length; i++) {
        if (!loadByte(loadAddress + i, bytes[i])) {
            throw new Error("Could not load byte " + i + " from " + path.basename(filePath));
        }
    }

    return bytes.length;
}

function findListAddress(filePath: string, label: string): number {
    const text = fs.readFileSync(filePath, "utf8");
    const pattern = new RegExp("^" + label + "\\s+=\\s+0x([0-9A-Fa-f]+)", "m");
    const match = text.match(pattern);

    if (!match) {
        throw new Error("Could not find " + label + " in " + path.basename(filePath));
    }

    return parseInt(match[1], 16);
}

async function main(): Promise<void> {
    const module = await createZx16Backend({
        locateFile: (fileName: string): string => path.join(__dirname, "..", "..", "web-build", fileName)
    });

    const reset = module.cwrap("zx16_reset", null, []);
    const loadByte = module.cwrap("zx16_load_byte", "number", ["number", "number"]);
    const read8 = module.cwrap("zx16_read8", "number", ["number"]);
    const run = module.cwrap("zx16_run", "number", ["number"]);
    const step = module.cwrap("zx16_step", "number", []);
    const isHalted = module.cwrap("zx16_is_halted", "number", []);
    const getPc = module.cwrap("zx16_get_pc", "number", []);
    const setPc = module.cwrap("zx16_set_pc", null, ["number"]);
    const getReg = module.cwrap("zx16_get_register", "number", ["number"]);
    const setReg = module.cwrap("zx16_set_register", null, ["number", "number"]);
    const getOutput = module.cwrap("zx16_get_output", "string", []);
    const setKeyboardKey = module.cwrap("zx16_set_keyboard_key", null, ["number"]);
    const hasPendingTone = module.cwrap("zx16_has_pending_tone", "number", []);
    const getToneFrequency = module.cwrap("zx16_get_tone_frequency", "number", []);
    const getToneDurationMs = module.cwrap("zx16_get_tone_duration_ms", "number", []);
    const clearToneRequest = module.cwrap("zx16_clear_tone_request", null, []);
    const hasPendingStopAudio = module.cwrap("zx16_has_pending_stop_audio", "number", []);
    const clearStopAudioRequest = module.cwrap("zx16_clear_stop_audio_request", null, []);
    const getVolumePercent = module.cwrap("zx16_get_volume_percent", "number", []);
    const stepWithBreakpoints = module.cwrap("zx16_step_with_breakpoints", "number", []);
    const toggleBreakpoint = module.cwrap("zx16_toggle_breakpoint", "number", ["number"]);
    const hasBreakpoint = module.cwrap("zx16_has_breakpoint", "number", ["number"]);
    const getBreakpointCount = module.cwrap("zx16_get_breakpoint_count", "number", []);
    const clearBreakpoints = module.cwrap("zx16_clear_breakpoints", null, []);

    const asmBinDir = path.join(__dirname, "..", "..", "asm", "bin");
    const assemblyTests = [
        ["01_arithmetic_registers.bin", "TEST 01 RESULT: PASS"],
        ["02_immediate_operations.bin", "TEST 02 RESULT: PASS"],
        ["03_logic_shift_compare.bin", "TEST 03 RESULT: PASS"],
        ["04_memory_load_store.bin", "TEST 04 RESULT: PASS"],
        ["05_branches_jumps.bin", "TEST 05 RESULT: PASS"],
        ["06_stack_subroutines.bin", "TEST 06 RESULT: PASS"],
        ["07_console_string_services.bin", "TEST 07 RESULT: PASS"],
        ["08_rng_keyboard_audio.bin", "TEST 08 RESULT: PASS"],
        ["09_graphics_vram.bin", "TEST 09 RESULT: PASS"],
        ["10_full_integration_corner_cases.bin", "TEST 10 RESULT: PASS"]
    ];

    for (const test of assemblyTests) {
        const fileName = test[0];
        const expectedOutput = test[1];
        const binPath = path.join(asmBinDir, fileName);

        reset();
        const byteCount = loadBin(loadByte, binPath);
        const steps = run(50000);
        const output = getOutput();

        if (!isHalted()) {
            throw new Error(fileName + " did not halt after " + steps + " steps");
        }

        if (output.indexOf(expectedOutput) < 0 || output.indexOf("FAIL") >= 0) {
            throw new Error(fileName + " did not print the expected PASS output");
        }

        console.log(fileName + " passed in WebAssembly (" + byteCount + " bytes, " + steps + " steps)");
    }

    reset();
    const snakePath = path.join(asmBinDir, "snake.bin");
    const snakeListPath = path.join(asmBinDir, "snake.lst");
    const foodNextAddress = findListAddress(snakeListPath, "food_next");
    const eatFoodAddress = findListAddress(snakeListPath, "eat_food");
    const lengthAddress = findListAddress(snakeListPath, "length");
    const pendingHeadAddress = findListAddress(snakeListPath, "pending_head");
    const food0Address = findListAddress(snakeListPath, "food0");
    const scoreAddress = findListAddress(snakeListPath, "score");
    const segmentsAddress = findListAddress(snakeListPath, "segments");
    loadBin(loadByte, snakePath);
    run(10000);

    if (read8(0xF000 + 21) !== 2 || read8(0xF000 + 52) !== 2 || read8(0xF000 + 165) !== 2) {
        throw new Error("Snake initial apples were not placed from the RNG sequence");
    }

    if (read8(0xF000 + 170) === 2 || read8(0xF000 + 207) === 2 || read8(0xF000 + 228) === 2) {
        throw new Error("Snake still used the old hard-coded initial apple positions");
    }

    console.log("Snake initial apples are generated by RNG");

    setKeyboardKey(4);
    const snakeSteps = run(5000);
    const snakeDrewVram = read8(0xF000) !== 0 || read8(0xF001) !== 0 || read8(0xF002) !== 0;

    if (isHalted()) {
        throw new Error("Snake halted during the startup smoke test");
    }

    if (!snakeDrewVram) {
        throw new Error("Snake did not write visible data into VRAM");
    }

    console.log("snake.bin started in WebAssembly (" + snakeSteps + " steps, PC=" + hex16(getPc()) + ")");

    reset();
    loadBin(loadByte, snakePath);

    loadWord(loadByte, 0x8004, 170);
    loadWord(loadByte, 0x800A, 170);
    setPc(foodNextAddress);
    run(2000);

    if (read8(0xF000 + 15) !== 2) {
        throw new Error("Snake food placement did not use the RNG candidate");
    }

    if (read8(0xF000 + 171) === 2) {
        throw new Error("Snake food placement put the new apple right next to the eaten apple");
    }

    console.log("Snake food placement uses RNG and does not push apples right");

    reset();
    loadBin(loadByte, snakePath);
    loadWord(loadByte, 0x8004, 170);
    loadWord(loadByte, 0x800A, 170);

    for (let offset = 0; offset < 300; offset++) {
        loadByte(0xF000 + offset, 1);
    }

    loadByte(0xF000 + 73, 0);
    setPc(foodNextAddress);
    run(2500);

    if (read8(0xF000 + 73) !== 2) {
        throw new Error("Snake food placement got stuck instead of scanning to the open tile");
    }

    console.log("Snake food placement scans safely when many cells are blocked");

    reset();
    loadBin(loadByte, snakePath);
    loadWord(loadByte, lengthAddress, 80);
    loadWord(loadByte, pendingHeadAddress, 170);
    loadWord(loadByte, food0Address, 170);

    for (let i = 0; i < 80; i++) {
        loadWord(loadByte, segmentsAddress + i * 2, 40 + i);
    }

    setPc(eatFoodAddress);
    run(1500);

    if (read8(0xF000 + 15) !== 2) {
        throw new Error("Snake stopped replacing apples at max length");
    }

    if (read8(scoreAddress) !== 5 || read8(scoreAddress + 1) !== 0) {
        throw new Error("Snake did not add 5 points after eating an apple");
    }

    console.log("Snake still replaces apples at max length");

    reset();
    setKeyboardKey(4);

    loadWord(loadByte, 0x0020, makeSys(0x030));
    step();

    if (getReg(6) !== 4) {
        throw new Error("Keyboard ECALL did not copy the browser key into x6");
    }

    console.log("Keyboard ECALL received key code " + getReg(6));

    reset();
    setReg(6, 440);
    setReg(7, 120);
    loadWord(loadByte, 0x0020, makeSys(0x020));
    step();

    if (!hasPendingTone()) {
        throw new Error("play_tone ECALL did not create a pending tone request");
    }

    if (getToneFrequency() !== 440 || getToneDurationMs() !== 120) {
        throw new Error("play_tone ECALL stored the wrong tone values");
    }

    clearToneRequest();

    if (hasPendingTone()) {
        throw new Error("Tone request was not cleared");
    }

    reset();
    setReg(6, 35);
    loadWord(loadByte, 0x0020, makeSys(0x021));
    step();

    if (getVolumePercent() !== 35) {
        throw new Error("set_volume ECALL did not update the backend volume");
    }

    reset();
    setReg(6, 330);
    setReg(7, 200);
    loadWord(loadByte, 0x0020, makeSys(0x020));
    loadWord(loadByte, 0x0022, makeSys(0x022));
    step();
    step();

    if (!hasPendingStopAudio()) {
        throw new Error("stop_audio ECALL did not create a stop request");
    }

    if (hasPendingTone()) {
        throw new Error("stop_audio ECALL did not clear the pending tone");
    }

    clearStopAudioRequest();

    console.log("Audio ECALLs produced tone, volume, and stop requests");

    reset();
    toggleBreakpoint(0x0020);
    toggleBreakpoint(0x0024);

    if (getBreakpointCount() !== 2) {
        throw new Error("Backend did not keep multiple breakpoints");
    }

    if (!hasBreakpoint(0x0020) || !hasBreakpoint(0x0024)) {
        throw new Error("Backend breakpoint lookup failed");
    }

    if (stepWithBreakpoints() !== 0) {
        throw new Error("Breakpoint-aware step did not stop at a breakpoint");
    }

    clearBreakpoints();

    if (getBreakpointCount() !== 0) {
        throw new Error("Backend breakpoints were not cleared");
    }

    console.log("Debugger breakpoints support multiple addresses");
    console.log("ZX16 backend WebAssembly TypeScript test passed");
}

main().catch((error: Error): void => {
    console.error(error.message);
    process.exit(1);
});
