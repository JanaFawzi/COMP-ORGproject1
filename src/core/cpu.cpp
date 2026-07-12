#include "cpu.h"
#include <stdio.h>

CPU::CPU() {
    clearInput();
    reset();
}

void CPU::reset() {
    memory.reset();
    registers.reset();

    pc = 0x0020;
    registers.setRegister(2, 0xEFFE);

    lastInstruction = 0x0000;
    lastHandler = -1;

    clearOutput();
    clearInput();
    
    resetRng();
    clearKeyboardKey();

    toneFrequency = 0;
    toneDurationMs = 0;
    tonePending = false;
    toneRequestId = 0;

    stopAudioPending = false;
    stopAudioRequestId = 0;

    resetVolume();

    breakpointManager.reset();
    clearBreakpointHit();

    halted = false;
}

unsigned short CPU::getPC() {
    return pc;
}

void CPU::setPC(unsigned short value) {
    pc = value;
}

unsigned short CPU::getSP() {
    return registers.getRegister(2);
}

void CPU::setSP(unsigned short value) {
    registers.setRegister(2, value);
}

bool CPU::isWordAlignedAddress(unsigned short address) {
    return (address & 0x0001) == 0;
}

unsigned short CPU::fetch() {
    unsigned short instruction = memory.read16(pc);

    pc = pc + 2;

    return instruction;
}

unsigned short CPU::step() {
    if (halted) {
        return lastInstruction;
    }

    lastInstruction = fetch();

    DecodedInstruction decoded = decoder.decode(lastInstruction);

    dispatch(decoded);

    return lastInstruction;
}

bool CPU::isValidBreakpointAddress(unsigned short address) {
    return BreakpointManager::isValidBreakpointAddress(address);
}

bool CPU::setBreakpoint(unsigned short address) {
    return breakpointManager.setBreakpoint(address);
}

bool CPU::clearBreakpoint(unsigned short address) {
    return breakpointManager.clearBreakpoint(address);
}

bool CPU::toggleBreakpoint(unsigned short address) {
    return breakpointManager.toggleBreakpoint(address);
}

bool CPU::hasBreakpoint(unsigned short address) {
    return breakpointManager.hasBreakpoint(address);
}

void CPU::clearBreakpoints() {
    breakpointManager.reset();
    clearBreakpointHit();
}

int CPU::getBreakpointCount() {
    return breakpointManager.getBreakpointCount();
}

bool CPU::hasBreakpointAtPC() {
    return breakpointManager.hasBreakpoint(pc);
}

bool CPU::hasBreakpointHit() {
    return breakpointHit;
}

unsigned short CPU::getBreakpointHitAddress() {
    return breakpointHitAddress;
}

void CPU::clearBreakpointHit() {
    breakpointHit = false;
    breakpointHitAddress = 0;
}

bool CPU::stepWithBreakpoints() {
    if (halted) {
        return false;
    }

    if (breakpointHit && breakpointHitAddress == pc) {
        clearBreakpointHit();
        step();
        return true;
    }

    if (hasBreakpointAtPC()) {
        breakpointHit = true;
        breakpointHitAddress = pc;
        return false;
    }

    clearBreakpointHit();

    step();

    return true;
}

bool CPU::isCallInstructionWord(unsigned short word) {
    InstructionDecoder localDecoder;
    DecodedInstruction instruction = localDecoder.decode(word);

    if (instruction.opcode == 5 && instruction.linkFlag == 1) {
        return true;
    }

    if (instruction.opcode == 0 && instruction.funct4 == 0xC) {
        return true;
    }

    return false;
}

bool CPU::stepOver() {
    if (halted) {
        return false;
    }

    unsigned short word = memory.read16(pc);

    if (!isCallInstructionWord(word)) {
        clearBreakpointHit();
        step();
        return true;
    }

    unsigned short returnAddress = pc + 2;

    clearBreakpointHit();

    step();

    int executedCount = 0;

    while (!halted && pc != returnAddress && executedCount < STEP_OVER_MAX_INSTRUCTIONS) {
        if (hasBreakpointAtPC()) {
            breakpointHit = true;
            breakpointHitAddress = pc;
            return false;
        }

        step();

        executedCount++;
    }

    if (pc == returnAddress) {
        return true;
    }

    if (halted) {
        return true;
    }

    return false;
}

bool CPU::isValidRunToCursorAddress(unsigned short address) {
    if ((address & 1) != 0) {
        return false;
    }

    return true;
}

bool CPU::runToCursor(unsigned short cursorAddress) {
    if (halted) {
        return false;
    }

    if (!isValidRunToCursorAddress(cursorAddress)) {
        return false;
    }

    clearBreakpointHit();

    if (pc == cursorAddress) {
        return true;
    }

    int executedCount = 0;

    while (!halted && pc != cursorAddress && executedCount < RUN_TO_CURSOR_MAX_INSTRUCTIONS) {
        if (hasBreakpointAtPC()) {
            breakpointHit = true;
            breakpointHitAddress = pc;
            return false;
        }

        step();

        executedCount++;
    }

    if (pc == cursorAddress) {
        return true;
    }

    return false;
}

unsigned short CPU::getLastInstruction() {
    return lastInstruction;
}

int CPU::getLastHandler() {
    return lastHandler;
}

Memory& CPU::getMemory() {
    return memory;
}

RegisterFile& CPU::getRegisters() {
    return registers;
}

const char* CPU::getOutput() {
    return output;
}

void CPU::clearOutput() {
    outputLength = 0;
    output[0] = '\0';
}

void CPU::dumpRegisters() {
    char text[40];

    appendText("REGS\n");

    sprintf(text, "PC=0x%04X\n", pc);
    appendText(text);

    sprintf(text, "SP=0x%04X\n", getSP());
    appendText(text);

    for (int i = 0; i < 8; i++) {
        sprintf(text, "x%d=0x%04X\n", i, registers.getRegister(i));
        appendText(text);
    }
}

bool CPU::isValidMemoryDumpLength(unsigned short length) {
    if (length > MAX_MEMORY_DUMP_BYTES) {
        return false;
    }

    return true;
}

bool CPU::dumpMemory(unsigned short startAddress, unsigned short length) {
    if (!isValidMemoryDumpLength(length)) {
        return false;
    }

    char text[40];

    appendText("MEM\n");

    sprintf(text, "START=0x%04X\n", startAddress);
    appendText(text);

    sprintf(text, "LEN=0x%04X\n", length);
    appendText(text);

    unsigned short offset = 0;

    while (offset < length) {
        unsigned short rowAddress = startAddress + offset;

        sprintf(text, "0x%04X:", rowAddress);
        appendText(text);

        for (int col = 0; col < 8 && offset < length; col++) {
            unsigned short address = startAddress + offset;
            unsigned char value = memory.read8(address);

            sprintf(text, " %02X", value);
            appendText(text);

            offset++;
        }

        appendText("\n");
    }

    return true;
}

const char* CPU::getInput() {
    return input;
}

void CPU::clearInput() {
    inputLength = 0;
    inputPosition = 0;
    input[0] = '\0';
}

void CPU::setInput(const char text[]) {
    clearInput();
    appendInput(text);
}

void CPU::appendInput(const char text[]) {
    if (text == 0) {
        return;
    }

    int i = 0;

    while (text[i] != '\0' && inputLength < INPUT_SIZE - 1) {
        input[inputLength] = text[i];
        inputLength++;
        input[inputLength] = '\0';
        i++;
    }
}

unsigned short CPU::getRngState() {
    return rngState;
}

void CPU::resetRng() {
    rngState = DEFAULT_RNG_SEED;
}

void CPU::seedRng(unsigned short seed) {
    if (seed == 0) {
        seed = DEFAULT_RNG_SEED;
    }

    rngState = seed;
}

unsigned short CPU::nextRandom() {
    unsigned int x = rngState;

    x = x ^ ((x << 7) & 0xFFFF);
    x = x & 0xFFFF;

    x = x ^ (x >> 9);
    x = x & 0xFFFF;

    x = x ^ ((x << 8) & 0xFFFF);
    x = x & 0xFFFF;

    rngState = (unsigned short)x;

    return rngState;
}

bool CPU::isValidKeyboardCode(unsigned short keyCode) {
    if (keyCode <= ZX16_KEY_ESCAPE) {
        return true;
    }

    return false;
}

unsigned short CPU::getKeyboardKey() {
    return keyboardKey;
}

bool CPU::setKeyboardKey(unsigned short keyCode) {
    if (!isValidKeyboardCode(keyCode)) {
        return false;
    }

    keyboardKey = keyCode;

    return true;
}

void CPU::clearKeyboardKey() {
    keyboardKey = ZX16_KEY_NONE;
}

bool CPU::isValidTone(unsigned short frequency, unsigned short durationMs) {
    // Frequency zero represents a timed rest in audio programs.
    if (frequency != 0 && frequency < MIN_TONE_FREQUENCY) {
        return false;
    }

    if (frequency > MAX_TONE_FREQUENCY) {
        return false;
    }

    if (durationMs == 0) {
        return false;
    }

    if (durationMs > MAX_TONE_DURATION_MS) {
        return false;
    }

    return true;
}

bool CPU::requestTone(unsigned short frequency, unsigned short durationMs) {
    if (!isValidTone(frequency, durationMs)) {
        return false;
    }

    toneFrequency = frequency;
    toneDurationMs = durationMs;
    tonePending = true;
    toneRequestId++;

    return true;
}

bool CPU::hasPendingTone() {
    return tonePending;
}

unsigned short CPU::getToneFrequency() {
    return toneFrequency;
}

unsigned short CPU::getToneDurationMs() {
    return toneDurationMs;
}

unsigned int CPU::getToneRequestId() {
    return toneRequestId;
}

void CPU::clearToneRequest() {
    toneFrequency = 0;
    toneDurationMs = 0;
    tonePending = false;
}

bool CPU::requestStopAudio() {
    clearToneRequest();

    stopAudioPending = true;
    stopAudioRequestId++;

    return true;
}

bool CPU::hasPendingStopAudio() {
    return stopAudioPending;
}

unsigned int CPU::getStopAudioRequestId() {
    return stopAudioRequestId;
}

void CPU::clearStopAudioRequest() {
    stopAudioPending = false;
}

bool CPU::isValidVolumePercent(unsigned short volumePercentValue) {
    if (volumePercentValue > MAX_VOLUME_PERCENT) {
        return false;
    }

    return true;
}

unsigned short CPU::getVolumePercent() {
    return volumePercent;
}

bool CPU::setVolumePercent(unsigned short volumePercentValue) {
    if (!isValidVolumePercent(volumePercentValue)) {
        return false;
    }

    volumePercent = volumePercentValue;

    return true;
}

void CPU::resetVolume() {
    volumePercent = DEFAULT_VOLUME_PERCENT;
}

bool CPU::isHalted() {
    return halted;
}

void CPU::dispatch(DecodedInstruction instruction) {
    switch (instruction.opcode) {
        case 0:
            handleRType(instruction);
            break;

        case 1:
            handleIType(instruction);
            break;

        case 2:
            handleBType(instruction);
            break;

        case 3:
            handleSType(instruction);
            break;

        case 4:
            handleLType(instruction);
            break;

        case 5:
            handleJType(instruction);
            break;

        case 6:
            handleUType(instruction);
            break;

        case 7:
            handleSysType(instruction);
            break;

        default:
            lastHandler = -1;
            break;
    }
}

void CPU::handleRType(DecodedInstruction instruction) {
    lastHandler = ZX16::R_TYPE;

    unsigned short rdValue = registers.getRegister(instruction.rd);
    unsigned short rs2Value = registers.getRegister(instruction.rs2);
    unsigned short result = 0;

    if (instruction.funct4 == 0x0) {
        result = rdValue + rs2Value;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.funct4 == 0x1) {
        result = rdValue - rs2Value;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.funct4 == 0x2) {
        short rdSigned = (short)rdValue;
        short rs2Signed = (short)rs2Value;
        result = rdSigned < rs2Signed ? 1 : 0;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.funct4 == 0x3) {
        result = rdValue < rs2Value ? 1 : 0;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.funct4 == 0x4) {
        int amount = rs2Value & 15;
        result = rdValue << amount;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.funct4 == 0x5) {
        int amount = rs2Value & 15;
        result = rdValue >> amount;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.funct4 == 0x6) {
        int amount = rs2Value & 15;

        if (amount == 0) {
            result = rdValue;
        }
        else {
            unsigned int temp = rdValue >> amount;

            if ((rdValue & 0x8000) != 0) {
                unsigned int mask = 0xFFFF << (16 - amount);
                temp = temp | mask;
            }

            result = temp;
        }

        registers.setRegister(instruction.rd, result);
    }

    if (instruction.funct4 == 0x7) {
        result = rdValue | rs2Value;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.funct4 == 0x8) {
        result = rdValue & rs2Value;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.funct4 == 0x9) {
        result = rdValue ^ rs2Value;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.funct4 == 0xA) {
        registers.setRegister(instruction.rd, rs2Value);
    }

    if (instruction.funct4 == 0xB) {
        pc = rdValue;
    }

    if (instruction.funct4 == 0xC) {
        registers.setRegister(instruction.rd, pc);
        pc = rs2Value;
    }
}

void CPU::handleIType(DecodedInstruction instruction) {
    lastHandler = ZX16::I_TYPE;

    unsigned short rdValue = registers.getRegister(instruction.rd);
    unsigned short result = 0;
    unsigned short immMask = (instruction.word >> 9) & 0x007F;

    if (instruction.func3 == 0x0) {
        result = rdValue + instruction.immediate;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.func3 == 0x1) {
        result = (short)rdValue < (short)instruction.immediate ? 1 : 0;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.func3 == 0x2) {
        result = rdValue < (unsigned short)instruction.immediate ? 1 : 0;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.func3 == 0x3) {
        int shiftType = (immMask >> 4) & 0x7;
        int amount = immMask & 0xF;

        if (shiftType == 0x1) {
            result = (unsigned short)(rdValue << amount);
            registers.setRegister(instruction.rd, result);
        }

        if (shiftType == 0x2) {
            result = (unsigned short)(rdValue >> amount);
            registers.setRegister(instruction.rd, result);
        }

        if (shiftType == 0x4) {
            if (amount == 0) {
                result = rdValue;
            }
            else {
                unsigned int shifted = rdValue >> amount;
                if ((rdValue & 0x8000) != 0) {
                    shifted |= 0xFFFFu << (16 - amount);
                }
                result = (unsigned short)shifted;
            }
            registers.setRegister(instruction.rd, result);
        }
    }

    if (instruction.func3 == 0x4) {
        result = rdValue | immMask;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.func3 == 0x5) {
        result = rdValue & (unsigned short)instruction.immediate;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.func3 == 0x6) {
        result = rdValue ^ (unsigned short)instruction.immediate;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.func3 == 0x7) {
        registers.setRegister(instruction.rd, (unsigned short)instruction.immediate);
    }
}

void CPU::handleBType(DecodedInstruction instruction) {
    lastHandler = ZX16::B_TYPE;

    unsigned short rs1Value = registers.getRegister(instruction.rs1);
    unsigned short rs2Value = registers.getRegister(instruction.rs2);

    int rs1Signed = rs1Value;
    int rs2Signed = rs2Value;

    if ((rs1Value & 0x8000) != 0) {
        rs1Signed = rs1Value - 0x10000;
    }

    if ((rs2Value & 0x8000) != 0) {
        rs2Signed = rs2Value - 0x10000;
    }

    if (instruction.func3 == 0x0) {
        if (rs1Value == rs2Value) {
            pc = pc + instruction.immediate;
        }
    }

    if (instruction.func3 == 0x1) {
        if (rs1Value != rs2Value) {
            pc = pc + instruction.immediate;
        }
    }

    if (instruction.func3 == 0x2) {
        if (rs1Value == 0) {
            pc = pc + instruction.immediate;
        }
    }

    if (instruction.func3 == 0x3) {
        if (rs1Value != 0) {
            pc = pc + instruction.immediate;
        }
    }

    if (instruction.func3 == 0x4) {
        if (rs1Signed < rs2Signed) {
            pc = pc + instruction.immediate;
        }
    }

    if (instruction.func3 == 0x5) {
        if (rs1Signed >= rs2Signed) {
            pc = pc + instruction.immediate;
        }
    }

    if (instruction.func3 == 0x6) {
        if (rs1Value < rs2Value) {
            pc = pc + instruction.immediate;
        }
    }

    if (instruction.func3 == 0x7) {
        if (rs1Value >= rs2Value) {
            pc = pc + instruction.immediate;
        }
    }
}

void CPU::handleSType(DecodedInstruction instruction) {
    lastHandler = ZX16::S_TYPE;

    unsigned short baseAddress = registers.getRegister(instruction.rs1);
    unsigned short address = baseAddress + instruction.immediate;
    unsigned short rs2Value = registers.getRegister(instruction.rs2);

    if (instruction.func3 == 0x0) {
        memory.write8(address, rs2Value & 0x00FF);
    }

    if (instruction.func3 == 0x1) {
        memory.write16(address, rs2Value);
    }
}

void CPU::handleLType(DecodedInstruction instruction) {
    lastHandler = ZX16::L_TYPE;

    unsigned short baseAddress = registers.getRegister(instruction.rs1);
    unsigned short address = baseAddress + instruction.immediate;
    unsigned short result = 0;

    if (instruction.func3 == 0x0) {
        unsigned char byteValue = memory.read8(address);

        if ((byteValue & 0x80) != 0) {
            result = 0xFF00 | byteValue;
        }
        else {
            result = byteValue;
        }

        registers.setRegister(instruction.rd, result);
    }

    if (instruction.func3 == 0x1) {
        result = memory.read16(address);
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.func3 == 0x4) {
        result = memory.read8(address);
        registers.setRegister(instruction.rd, result);
    }
}

void CPU::handleJType(DecodedInstruction instruction) {
    lastHandler = ZX16::J_TYPE;

    if (instruction.linkFlag == 1) {
        registers.setRegister(instruction.rd, pc);
    }

    pc = pc + instruction.immediate;
}

void CPU::handleUType(DecodedInstruction instruction) {
    lastHandler = ZX16::U_TYPE;

    unsigned short result = (unsigned short)instruction.immediate;

    if (instruction.upperFlag == 1) {
        unsigned short instructionAddress = (unsigned short)(pc - 2);
        result = (unsigned short)(instructionAddress + result);
    }

    registers.setRegister(instruction.rd, result);
}
