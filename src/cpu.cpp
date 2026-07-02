#include "cpu.h"
#include <stdio.h>

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
    clearOutput();                  // Clear console output
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

const char* CPU::getOutput() {
    return output;
}

void CPU::clearOutput() {
    outputLength = 0;
    output[0] = '\0';
}

void CPU::appendChar(char c) {
    if (outputLength < OUTPUT_SIZE - 1) {
        output[outputLength] = c;
        outputLength++;
        output[outputLength] = '\0';
    }
}

void CPU::appendText(const char text[]) {
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

    // Also show it in CLion terminal for now
    printf("%s", text);
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
    unsigned short result = 0;

    // ADD rd, rs2: rd = rd + rs2
    if (instruction.funct4 == 0x0) {
        result = rdValue + rs2Value;
        registers.setRegister(instruction.rd, result);
    }

    // SUB rd, rs2: rd = rd - rs2
    if (instruction.funct4 == 0x1) {
        result = rdValue - rs2Value;
        registers.setRegister(instruction.rd, result);
    }

    // SLL rd, rs2: rd = rd << (rs2 & 15)
    if (instruction.funct4 == 0x4) {
        int amount = rs2Value & 15;
        result = rdValue << amount;
        registers.setRegister(instruction.rd, result);
    }

    // SRL rd, rs2: rd = rd >> (rs2 & 15), logical
    if (instruction.funct4 == 0x5) {
        int amount = rs2Value & 15;
        result = rdValue >> amount;
        registers.setRegister(instruction.rd, result);
    }

    // SRA rd, rs2: rd = rd >> (rs2 & 15), arithmetic
    if (instruction.funct4 == 0x6) {
        int amount = rs2Value & 15;

        if (amount == 0) {
            result = rdValue;
        } else {
            unsigned int temp = rdValue >> amount;

            // If sign bit is 1, fill upper bits with 1s
            if ((rdValue & 0x8000) != 0) {
                unsigned int mask = 0xFFFF << (16 - amount);
                temp = temp | mask;
            }

            result = temp;
        }

        registers.setRegister(instruction.rd, result);
    }

    // OR rd, rs2: rd = rd | rs2
    if (instruction.funct4 == 0x7) {
        result = rdValue | rs2Value;
        registers.setRegister(instruction.rd, result);
    }

    // AND rd, rs2: rd = rd & rs2
    if (instruction.funct4 == 0x8) {
        result = rdValue & rs2Value;
        registers.setRegister(instruction.rd, result);
    }

    // XOR rd, rs2: rd = rd ^ rs2
    if (instruction.funct4 == 0x9) {
        result = rdValue ^ rs2Value;
        registers.setRegister(instruction.rd, result);
    }

     // JR rd: PC = rd
    if (instruction.funct4 == 0xB) {
        pc = rdValue;
    }

    // JALR rd, rs2: rd = PC + 2, PC = rs2
    // fetch() already moved PC to the return address
    if (instruction.funct4 == 0xC) {
        registers.setRegister(instruction.rd, pc);
        pc = rs2Value;
    }
}

void CPU::handleIType(DecodedInstruction instruction) {
    lastHandler = ZX16::I_TYPE;      // I-Type handler called

    unsigned short rdValue = registers.getRegister(instruction.rd);
    unsigned short result = 0;

    // ADDI rd, imm: rd = rd + signed imm7
    if (instruction.func3 == 0x0) {
        result = rdValue + instruction.immediate;
        registers.setRegister(instruction.rd, result);
    }

    // ORI rd, imm: rd = rd | zero-extended 7-bit mask
    if (instruction.func3 == 0x4) {
        unsigned short immMask = (instruction.word >> 9) & 0x007F;
        result = rdValue | immMask;
        registers.setRegister(instruction.rd, result);
    }

    // ANDI rd, imm: rd = rd & signed imm7
    if (instruction.func3 == 0x5) {
        result = rdValue & (unsigned short)instruction.immediate;
        registers.setRegister(instruction.rd, result);
    }

    // XORI rd, imm: rd = rd ^ signed imm7
    if (instruction.func3 == 0x6) {
        result = rdValue ^ (unsigned short)instruction.immediate;
        registers.setRegister(instruction.rd, result);
    }

    // LI rd, imm: rd = signed imm7
    if (instruction.func3 == 0x7) {
        registers.setRegister(instruction.rd, (unsigned short)instruction.immediate);
    }
}

void CPU::handleBType(DecodedInstruction instruction) {
    lastHandler = ZX16::B_TYPE;      // B-Type handler called

    unsigned short rs1Value = registers.getRegister(instruction.rs1);
    unsigned short rs2Value = registers.getRegister(instruction.rs2);

    int rs1Signed = rs1Value;
    int rs2Signed = rs2Value;

    // Convert 16-bit unsigned register values to signed values
    if ((rs1Value & 0x8000) != 0) {
        rs1Signed = rs1Value - 0x10000;
    }

    if ((rs2Value & 0x8000) != 0) {
        rs2Signed = rs2Value - 0x10000;
    }

    // BEQ rs1, rs2, label
    if (instruction.func3 == 0x0) {
        if (rs1Value == rs2Value) {
            pc = pc + instruction.immediate;
        }
    }

    // BNE rs1, rs2, label
    if (instruction.func3 == 0x1) {
        if (rs1Value != rs2Value) {
            pc = pc + instruction.immediate;
        }
    }

    // BZ rs1, label
    if (instruction.func3 == 0x2) {
        if (rs1Value == 0) {
            pc = pc + instruction.immediate;
        }
    }

    // BNZ rs1, label
    if (instruction.func3 == 0x3) {
        if (rs1Value != 0) {
            pc = pc + instruction.immediate;
        }
    }

    // BLT rs1, rs2, label, signed comparison
    if (instruction.func3 == 0x4) {
        if (rs1Signed < rs2Signed) {
            pc = pc + instruction.immediate;
        }
    }

    // BGE rs1, rs2, label, signed comparison
    if (instruction.func3 == 0x5) {
        if (rs1Signed >= rs2Signed) {
            pc = pc + instruction.immediate;
        }
    }

    // BLTU rs1, rs2, label, unsigned comparison
    if (instruction.func3 == 0x6) {
        if (rs1Value < rs2Value) {
            pc = pc + instruction.immediate;
        }
    }

    // BGEU rs1, rs2, label, unsigned comparison
    if (instruction.func3 == 0x7) {
        if (rs1Value >= rs2Value) {
            pc = pc + instruction.immediate;
        }
    }
}

void CPU::handleSType(DecodedInstruction instruction) {
    lastHandler = ZX16::S_TYPE;      // S-Type handler called

    unsigned short baseAddress = registers.getRegister(instruction.rs1);
    unsigned short address = baseAddress + instruction.immediate;
    unsigned short rs2Value = registers.getRegister(instruction.rs2);

    // SB rs2, offset(rs1): store low byte of rs2
    if (instruction.func3 == 0x0) {
        memory.write8(address, rs2Value & 0x00FF);
    }

    // SW rs2, offset(rs1): store full 16-bit word of rs2
    if (instruction.func3 == 0x1) {
        memory.write16(address, rs2Value);
    }
}

void CPU::handleLType(DecodedInstruction instruction) {
    lastHandler = ZX16::L_TYPE;      // L-Type handler called

    unsigned short baseAddress = registers.getRegister(instruction.rs1);
    unsigned short address = baseAddress + instruction.immediate;
    unsigned short result = 0;

    // LB rd, offset(rs): load byte and sign-extend
    if (instruction.func3 == 0x0) {
        unsigned char byteValue = memory.read8(address);

        if ((byteValue & 0x80) != 0) {
            result = 0xFF00 | byteValue;
        } else {
            result = byteValue;
        }

        registers.setRegister(instruction.rd, result);
    }

    // LW rd, offset(rs): load 16-bit word
    if (instruction.func3 == 0x1) {
        result = memory.read16(address);
        registers.setRegister(instruction.rd, result);
    }

    // LBU rd, offset(rs): load byte and zero-extend
    if (instruction.func3 == 0x4) {
        result = memory.read8(address);
        registers.setRegister(instruction.rd, result);
    }
}

void CPU::handleJType(DecodedInstruction instruction) {
    lastHandler = ZX16::J_TYPE;      // J-Type handler called

    // J imm: PC = PC + imm
    // JAL rd, imm: rd = PC + 2, PC = PC + imm
    // fetch() already moved PC to PC + 2
    if (instruction.linkFlag == 1) {
        registers.setRegister(instruction.rd, pc);
    }

    pc = pc + instruction.immediate;
}

void CPU::handleUType(DecodedInstruction instruction) {
    lastHandler = ZX16::U_TYPE;      // U-Type handler called
}

void CPU::handleSysType(DecodedInstruction instruction) {
    lastHandler = ZX16::SYS_TYPE;    // SYS-Type handler called
     // ECALL 0x000: print_int a0
    if (instruction.service == 0x000) {
        unsigned short a0 = registers.getRegister(6); // a0 = x6
        printSignedDecimal(a0);
    }
}