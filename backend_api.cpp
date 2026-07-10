#include "cpu.h"
#include "memory.h"

static CPU webCpu;

extern "C" {

void zx16_reset() {
    webCpu.reset();
}

void zx16_seed_rng(int seed) {
    webCpu.seedRng((unsigned short)(seed & 0xFFFF));
}

int zx16_load_byte(int address, int value) {
    if (address < 0 || address >= Memory::RAM_SIZE) {
        return 0;
    }

    webCpu.getMemory().write8((unsigned short)address, (unsigned char)(value & 0xFF));
    return 1;
}

int zx16_read8(int address) {
    if (address < 0 || address >= Memory::RAM_SIZE) {
        return 0;
    }

    return webCpu.getMemory().read8((unsigned short)address);
}

int zx16_write8(int address, int value) {
    return zx16_load_byte(address, value);
}

int zx16_read16(int address) {
    if (address < 0 || address >= Memory::RAM_SIZE) {
        return 0;
    }

    return webCpu.getMemory().read16((unsigned short)address);
}

int zx16_write16(int address, int value) {
    if (address < 0 || address >= Memory::RAM_SIZE) {
        return 0;
    }

    webCpu.getMemory().write16((unsigned short)address, (unsigned short)(value & 0xFFFF));
    return 1;
}

void zx16_set_pc(int value) {
    webCpu.setPC((unsigned short)(value & 0xFFFF));
}

int zx16_get_pc() {
    return webCpu.getPC();
}

void zx16_set_sp(int value) {
    webCpu.setSP((unsigned short)(value & 0xFFFF));
}

int zx16_get_sp() {
    return webCpu.getSP();
}

int zx16_get_register(int index) {
    return webCpu.getRegisters().getRegister(index);
}

void zx16_set_register(int index, int value) {
    webCpu.getRegisters().setRegister(index, (unsigned short)(value & 0xFFFF));
}

int zx16_step() {
    return webCpu.step();
}

int zx16_step_with_breakpoints() {
    return webCpu.stepWithBreakpoints() ? 1 : 0;
}

int zx16_step_over() {
    return webCpu.stepOver() ? 1 : 0;
}

int zx16_run_to_cursor(int address) {
    return webCpu.runToCursor((unsigned short)(address & 0xFFFF)) ? 1 : 0;
}

int zx16_run(int maxSteps) {
    int steps = 0;

    while (!webCpu.isHalted() && steps < maxSteps) {
        webCpu.step();
        steps++;
    }

    return steps;
}

int zx16_is_halted() {
    return webCpu.isHalted() ? 1 : 0;
}

int zx16_get_last_instruction() {
    return webCpu.getLastInstruction();
}

const char* zx16_get_output() {
    return webCpu.getOutput();
}

void zx16_clear_output() {
    webCpu.clearOutput();
}

int zx16_toggle_breakpoint(int address) {
    return webCpu.toggleBreakpoint((unsigned short)(address & 0xFFFE)) ? 1 : 0;
}

int zx16_has_breakpoint(int address) {
    return webCpu.hasBreakpoint((unsigned short)(address & 0xFFFE)) ? 1 : 0;
}

int zx16_get_breakpoint_count() {
    return webCpu.getBreakpointCount();
}

void zx16_clear_breakpoints() {
    webCpu.clearBreakpoints();
}

int zx16_has_breakpoint_hit() {
    return webCpu.hasBreakpointHit() ? 1 : 0;
}

int zx16_get_breakpoint_hit_address() {
    return webCpu.getBreakpointHitAddress();
}

void zx16_clear_breakpoint_hit() {
    webCpu.clearBreakpointHit();
}

void zx16_set_keyboard_key(int keyCode) {
    webCpu.setKeyboardKey((unsigned short)(keyCode & 0xFFFF));
}

int zx16_get_keyboard_key() {
    return webCpu.getKeyboardKey();
}

void zx16_clear_keyboard_key() {
    webCpu.clearKeyboardKey();
}

int zx16_has_pending_tone() {
    return webCpu.hasPendingTone() ? 1 : 0;
}

int zx16_get_tone_frequency() {
    return webCpu.getToneFrequency();
}

int zx16_get_tone_duration_ms() {
    return webCpu.getToneDurationMs();
}

void zx16_clear_tone_request() {
    webCpu.clearToneRequest();
}

int zx16_has_pending_stop_audio() {
    return webCpu.hasPendingStopAudio() ? 1 : 0;
}

void zx16_clear_stop_audio_request() {
    webCpu.clearStopAudioRequest();
}

int zx16_get_volume_percent() {
    return webCpu.getVolumePercent();
}

}
