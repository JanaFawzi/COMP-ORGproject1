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
    lastInstruction = fetch();       // Sequential execution for now

    return lastInstruction;
}

unsigned short CPU::getLastInstruction() {
    return lastInstruction;
}

Memory& CPU::getMemory() {
    return memory;
}

RegisterFile& CPU::getRegisters() {
    return registers;
}