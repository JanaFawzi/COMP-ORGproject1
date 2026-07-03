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
    resetRng();

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
    rngState = seed;
}

bool CPU::isHalted() {
    return halted;
}

void CPU::appendChar(char c) {
    if (outputLength < OUTPUT_SIZE - 1) {
        output[outputLength] = c;
        outputLength++;
        output[outputLength] = '\0';
    }
}

void CPU::appendText(const char text[]) {
    if (text == 0) {
        return;
    }

    int i = 0;

    while (text[i] != '\0') {
        appendChar(text[i]);
        i++;
    }
}

int CPU::toSigned16(unsigned short value) {
    if ((value & 0x8000) != 0) {
        return (int)value - 0x10000;
    }

    return value;
}

void CPU::printSignedDecimal(unsigned short value) {
    char text[16];

    int signedValue = toSigned16(value);

    sprintf(text, "%d", signedValue);

    appendText(text);

    printf("%s", text);
}

bool CPU::isInputAtEnd() {
    if (inputPosition >= inputLength) {
        return true;
    }

    return false;
}

char CPU::peekInputChar() {
    if (isInputAtEnd()) {
        return '\0';
    }

    return input[inputPosition];
}

char CPU::consumeInputChar() {
    char c = peekInputChar();

    if (!isInputAtEnd()) {
        inputPosition++;
    }

    return c;
}

bool CPU::isWhitespace(char c) {
    if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
        return true;
    }

    return false;
}

void CPU::printStringFromMemory(unsigned short address) {
    int count = 0;

    while (count < 65536) {
        unsigned char c = memory.read8((unsigned short)(address + count));

        if (c == 0) {
            return;
        }

        appendChar((char)c);
        printf("%c", (char)c);

        count++;
    }
}

void CPU::readIntToA0() {
    int sign = 1;
    int value = 0;
    bool hasDigit = false;

    while (!isInputAtEnd() && isWhitespace(peekInputChar())) {
        consumeInputChar();
    }

    if (!isInputAtEnd() && peekInputChar() == '-') {
        sign = -1;
        consumeInputChar();
    }
    else if (!isInputAtEnd() && peekInputChar() == '+') {
        consumeInputChar();
    }

    while (!isInputAtEnd()) {
        char c = peekInputChar();

        if (c < '0' || c > '9') {
            break;
        }

        hasDigit = true;
        value = value * 10 + (c - '0');

        consumeInputChar();
    }

    if (!hasDigit) {
        registers.setRegister(6, 0);
        return;
    }

    value = value * sign;

    registers.setRegister(6, (unsigned short)value);
}

void CPU::readStringToMemory(unsigned short address, unsigned short maxLength) {
    unsigned short written = 0;

    if (maxLength == 0) {
        registers.setRegister(6, 0);
        return;
    }

    while (!isInputAtEnd()) {
        char c = peekInputChar();

        if (c == '\n' || c == '\r') {
            consumeInputChar();

            if (!isInputAtEnd()) {
                char next = peekInputChar();

                if ((c == '\r' && next == '\n') || (c == '\n' && next == '\r')) {
                    consumeInputChar();
                }
            }

            break;
        }

        if (written < maxLength - 1) {
            memory.write8((unsigned short)(address + written), (unsigned char)c);
            written++;
        }

        consumeInputChar();
    }

    memory.write8((unsigned short)(address + written), 0);

    registers.setRegister(6, written);
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

    if (instruction.func3 == 0x0) {
        result = rdValue + instruction.immediate;
        registers.setRegister(instruction.rd, result);
    }

    if (instruction.func3 == 0x4) {
        unsigned short immMask = (instruction.word >> 9) & 0x007F;
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
}

void CPU::handleSysType(DecodedInstruction instruction) {
    lastHandler = ZX16::SYS_TYPE;

    if (instruction.service == 0x000) {
        unsigned short a0 = registers.getRegister(6);
        printSignedDecimal(a0);
    }

    if (instruction.service == 0x001) {
        unsigned short a0 = registers.getRegister(6);
        char c = (char)(a0 & 0x00FF);

        appendChar(c);
        printf("%c", c);
    }

    if (instruction.service == 0x010) {
        unsigned short address = registers.getRegister(6);
        unsigned short maxLength = registers.getRegister(7);

        readStringToMemory(address, maxLength);
    }

    if (instruction.service == 0x011) {
        readIntToA0();
    }

    if (instruction.service == 0x012) {
        unsigned short address = registers.getRegister(6);

        printStringFromMemory(address);
    }

    if (instruction.service == 0x020) {
        unsigned short seed = registers.getRegister(6);

        seedRng(seed);
    }

    if (instruction.service == 0x3FF) {
        halted = true;
    }
}