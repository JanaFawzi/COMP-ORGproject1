#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include "register_file.h"
#include "instruction_decoder.h"

class CPU {
public:
    static constexpr unsigned short DEFAULT_RNG_SEED = 0xACE1;

    CPU();

    void reset();

    unsigned short getPC();
    void setPC(unsigned short value);

    unsigned short getSP();
    void setSP(unsigned short value);

    unsigned short fetch();
    unsigned short step();

    unsigned short getLastInstruction();
    int getLastHandler();

    Memory& getMemory();
    RegisterFile& getRegisters();

    const char* getOutput();
    void clearOutput();

    const char* getInput();
    void clearInput();
    void setInput(const char text[]);
    void appendInput(const char text[]);

    unsigned short getRngState();
    void resetRng();
    void seedRng(unsigned short seed);

    bool isHalted();

private:
    Memory memory;
    RegisterFile registers;
    InstructionDecoder decoder;

    unsigned short pc;
    unsigned short lastInstruction;
    int lastHandler;

    static const int OUTPUT_SIZE = 1024;
    char output[OUTPUT_SIZE];
    int outputLength;

    static const int INPUT_SIZE = 1024;
    char input[INPUT_SIZE];
    int inputLength;
    int inputPosition;

    unsigned short rngState;

    bool halted;

    void dispatch(DecodedInstruction instruction);

    void handleRType(DecodedInstruction instruction);
    void handleIType(DecodedInstruction instruction);
    void handleBType(DecodedInstruction instruction);
    void handleSType(DecodedInstruction instruction);
    void handleLType(DecodedInstruction instruction);
    void handleJType(DecodedInstruction instruction);
    void handleUType(DecodedInstruction instruction);
    void handleSysType(DecodedInstruction instruction);

    void appendChar(char c);
    void appendText(const char text[]);
    void printSignedDecimal(unsigned short value);
    int toSigned16(unsigned short value);

    void printStringFromMemory(unsigned short address);
    void readIntToA0();
    void readStringToMemory(unsigned short address, unsigned short maxLength);

    char peekInputChar();
    char consumeInputChar();
    bool isInputAtEnd();
    bool isWhitespace(char c);
};

#endif