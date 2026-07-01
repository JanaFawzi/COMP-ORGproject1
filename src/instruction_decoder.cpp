#include "instruction_decoder.h"

DecodedInstruction InstructionDecoder::decode(unsigned short word) {
    DecodedInstruction instruction;

    instruction.word = word;

    instruction.opcode = extractOpcode(word);
    instruction.func3 = extractFunc3(word);
    instruction.funct4 = extractFunct4(word);

    instruction.rd = 0;
    instruction.rs1 = 0;
    instruction.rs2 = 0;

    instruction.immediate = 0;
    instruction.service = 0;
    instruction.linkFlag = 0;
    instruction.upperFlag = 0;

    instruction.format = getFormat(instruction.opcode);

    if (instruction.format == ZX16::R_TYPE) {
        instruction.rd = (word >> 6) & 0x0007;
        instruction.rs1 = instruction.rd;
        instruction.rs2 = (word >> 9) & 0x0007;
    }

    if (instruction.format == ZX16::I_TYPE) {
        instruction.rd = (word >> 6) & 0x0007;

        int imm7 = (word >> 9) & 0x007F;
        instruction.immediate = signExtend(imm7, 7);
    }

    if (instruction.format == ZX16::B_TYPE) {
        instruction.rs1 = (word >> 6) & 0x0007;
        instruction.rs2 = (word >> 9) & 0x0007;

        int imm5 = ((word >> 12) & 0x000F) << 1;
        instruction.immediate = signExtend(imm5, 5);
    }

    if (instruction.format == ZX16::S_TYPE) {
        instruction.rs1 = (word >> 6) & 0x0007;
        instruction.rs2 = (word >> 9) & 0x0007;

        int imm4 = (word >> 12) & 0x000F;
        instruction.immediate = signExtend(imm4, 4);
    }

    if (instruction.format == ZX16::L_TYPE) {
        instruction.rd = (word >> 6) & 0x0007;
        instruction.rs1 = (word >> 9) & 0x0007;

        int imm4 = (word >> 12) & 0x000F;
        instruction.immediate = signExtend(imm4, 4);
    }

    if (instruction.format == ZX16::J_TYPE) {
        instruction.rd = (word >> 6) & 0x0007;
        instruction.linkFlag = (word >> 15) & 0x0001;

        int imm10 = 0;
        imm10 = imm10 | (((word >> 3) & 0x0007) << 1);
        imm10 = imm10 | (((word >> 9) & 0x003F) << 4);

        instruction.immediate = signExtend(imm10, 10);
    }

    if (instruction.format == ZX16::U_TYPE) {
        instruction.rd = (word >> 6) & 0x0007;
        instruction.upperFlag = (word >> 15) & 0x0001;

        int imm9 = 0;
        imm9 = imm9 | ((word >> 3) & 0x0007);
        imm9 = imm9 | (((word >> 9) & 0x003F) << 3);

        instruction.immediate = imm9 << 7;
    }

    if (instruction.format == ZX16::SYS_TYPE) {
        instruction.service = (word >> 6) & 0x03FF;
        instruction.immediate = instruction.service;
    }

    return instruction;
}

unsigned char InstructionDecoder::extractOpcode(unsigned short word) {
    return word & 0x0007;       // Opcode is bits [2:0]
}

unsigned char InstructionDecoder::extractFunc3(unsigned short word) {
    return (word >> 3) & 0x0007; // func3 is bits [5:3]
}

unsigned char InstructionDecoder::extractFunct4(unsigned short word) {
    return (word >> 12) & 0x000F; // funct4 is bits [15:12]
}

int InstructionDecoder::signExtend(int value, int bits) {
    int signBit = 1 << (bits - 1);
    int mask = (1 << bits) - 1;

    value = value & mask;

    if ((value & signBit) != 0) {
        value = value | (~mask);
    }

    return value;
}

ZX16::InstructionFormat InstructionDecoder::getFormat(unsigned char opcode) {
    if (opcode == 0) {
        return ZX16::R_TYPE;
    }

    if (opcode == 1) {
        return ZX16::I_TYPE;
    }

    if (opcode == 2) {
        return ZX16::B_TYPE;
    }

    if (opcode == 3) {
        return ZX16::S_TYPE;
    }

    if (opcode == 4) {
        return ZX16::L_TYPE;
    }

    if (opcode == 5) {
        return ZX16::J_TYPE;
    }

    if (opcode == 6) {
        return ZX16::U_TYPE;
    }

    return ZX16::SYS_TYPE;
}