#ifndef INSTRUCTION_DECODER_H
#define INSTRUCTION_DECODER_H

namespace ZX16 {
    enum InstructionFormat {
        R_TYPE = 0,
        I_TYPE = 1,
        B_TYPE = 2,
        S_TYPE = 3,
        L_TYPE = 4,
        J_TYPE = 5,
        U_TYPE = 6,
        SYS_TYPE = 7
    };
}

struct DecodedInstruction {
    unsigned short word;        // Full 16-bit instruction

    unsigned char opcode;       // Bits [2:0]
    unsigned char func3;        // Bits [5:3]
    unsigned char funct4;       // Bits [15:12], mainly for R-Type

    unsigned char rd;           // Destination register
    unsigned char rs1;          // Source register 1 / base register
    unsigned char rs2;          // Source register 2

    int immediate;              // Decoded immediate value
    unsigned short service;     // SYS service number
    unsigned char linkFlag;     // JAL flag for J-Type
    unsigned char upperFlag;    // AUIPC flag for U-Type

    ZX16::InstructionFormat format;
};

class InstructionDecoder {
public:
    DecodedInstruction decode(unsigned short word);

private:
    unsigned char extractOpcode(unsigned short word);
    unsigned char extractFunc3(unsigned short word);
    unsigned char extractFunct4(unsigned short word);

    int signExtend(int value, int bits);

    ZX16::InstructionFormat getFormat(unsigned char opcode);
};

#endif