#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include "register_file.h"
#include "instruction_decoder.h"

class CPU {
public:
    CPU();

    // Set CPU back to project reset state
    void reset();

    // Read/write program counter
    unsigned short getPC();
    void setPC(unsigned short value);

    // Read/write SP by name instead of using register index 2
    unsigned short getSP();
    void setSP(unsigned short value);

    // Read 16-bit instruction from memory at PC, then move PC by 2
    unsigned short fetch();

    // Fetch, decode, and dispatch one instruction
    unsigned short step();

    // Read last fetched instruction
    unsigned short getLastInstruction();

    // Read which handler was called last
    int getLastHandler();

    // Access CPU memory and registers without pointers
    Memory& getMemory();
    RegisterFile& getRegisters();

private:
    Memory memory;                  // 64 KB RAM
    RegisterFile registers;         // 8 registers x0..x7
    InstructionDecoder decoder;     // Decodes raw 16-bit instructions

    unsigned short pc;              // Program counter
    unsigned short lastInstruction; // Last instruction fetched by CPU
    int lastHandler;                // Last handler called by dispatcher

    // Dispatcher chooses the correct handler using opcode
    void dispatch(DecodedInstruction instruction);

    // Handler functions
    void handleRType(DecodedInstruction instruction);
    void handleIType(DecodedInstruction instruction);
    void handleBType(DecodedInstruction instruction);
    void handleSType(DecodedInstruction instruction);
    void handleLType(DecodedInstruction instruction);
    void handleJType(DecodedInstruction instruction);
    void handleUType(DecodedInstruction instruction);
    void handleSysType(DecodedInstruction instruction);
};

#endif