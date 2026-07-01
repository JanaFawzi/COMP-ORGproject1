#include "cpu.h"

CPU::CPU() {
    reset();
}

void CPU::reset() {
    memory.reset();             // Clear all memory
    registers.reset();          // Clear registers and set x2/sp = 0xEFFE

    pc = 0x0020;                // Program starts at 0x0020
    registers.setRegister(2, 0xEFFE); // x2 is stack pointer

    lastInstruction = 0x0000;   // No instruction fetched yet
    lastHandler = -1;           // No handler called yet
}

unsigned short CPU::getPC() {
    return pc;
}

void CPU::setPC(unsigned short value) {
    pc = value;
}

unsigned short CPU::getSP() {
    return registers.getRegister(2);  // x2 = sp
}

void CPU::setSP(unsigned short value) {
    registers.setRegister(2, value);  // x2 = sp
}

unsigned short CPU::fetch() {
    unsigned short instruction = memory.read16(pc); // Read instruction at PC

    pc = pc + 2;                                    // 16-bit instruction = 2 bytes

    return instruction;
}

unsigned short CPU::step() {
    lastInstruction = fetch();                      // Fetch raw instruction

    DecodedInstruction decoded = decoder.decode(lastInstruction);

    dispatch(decoded);                              // Call correct handler

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

void CPU::dispatch(DecodedInstruction instruction) {
    // Opcode is bits [2:0]
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
    lastHandler = ZX16::R_TYPE;      // R-Type handler called

    unsigned short rdValue = registers.getRegister(instruction.rd);
    unsigned short rs2Value = registers.getRegister(instruction.rs2);

    // ADD rd, rs2: rd = rd + rs2
    if (instruction.funct4 == 0x0) {
        registers.setRegister(instruction.rd, rdValue + rs2Value);
    }

    // SUB rd, rs2: rd = rd - rs2
    if (instruction.funct4 == 0x1) {
        registers.setRegister(instruction.rd, rdValue - rs2Value);
    }
}

void CPU::handleIType(DecodedInstruction instruction) {
    lastHandler = ZX16::I_TYPE;      // I-Type handler called
}

void CPU::handleBType(DecodedInstruction instruction) {
    lastHandler = ZX16::B_TYPE;      // B-Type handler called
}

void CPU::handleSType(DecodedInstruction instruction) {
    lastHandler = ZX16::S_TYPE;      // S-Type handler called
}

void CPU::handleLType(DecodedInstruction instruction) {
    lastHandler = ZX16::L_TYPE;      // L-Type handler called
}

void CPU::handleJType(DecodedInstruction instruction) {
    lastHandler = ZX16::J_TYPE;      // J-Type handler called
}

void CPU::handleUType(DecodedInstruction instruction) {
    lastHandler = ZX16::U_TYPE;      // U-Type handler called
}

void CPU::handleSysType(DecodedInstruction instruction) {
    lastHandler = ZX16::SYS_TYPE;    // SYS-Type handler called
}