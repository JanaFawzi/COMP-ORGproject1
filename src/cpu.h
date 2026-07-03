#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include "register_file.h"
#include "instruction_decoder.h"

class CPU {
public:
    static constexpr unsigned short DEFAULT_RNG_SEED = 0xACE1;

    static constexpr unsigned short ZX16_KEY_NONE = 0;
    static constexpr unsigned short ZX16_KEY_UP = 1;
    static constexpr unsigned short ZX16_KEY_DOWN = 2;
    static constexpr unsigned short ZX16_KEY_LEFT = 3;
    static constexpr unsigned short ZX16_KEY_RIGHT = 4;
    static constexpr unsigned short ZX16_KEY_SPACE = 5;
    static constexpr unsigned short ZX16_KEY_ENTER = 6;
    static constexpr unsigned short ZX16_KEY_ESCAPE = 7;

    static constexpr unsigned short MIN_TONE_FREQUENCY = 20;
    static constexpr unsigned short MAX_TONE_FREQUENCY = 20000;
    static constexpr unsigned short MAX_TONE_DURATION_MS = 5000;

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
    unsigned short nextRandom();

    static bool isValidKeyboardCode(unsigned short keyCode);
    unsigned short getKeyboardKey();
    bool setKeyboardKey(unsigned short keyCode);
    void clearKeyboardKey();

    static bool isValidTone(unsigned short frequency, unsigned short durationMs);

    bool requestTone(unsigned short frequency, unsigned short durationMs);
    bool hasPendingTone();
    unsigned short getToneFrequency();
    unsigned short getToneDurationMs();
    unsigned int getToneRequestId();
    void clearToneRequest();

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
    unsigned short keyboardKey;

    unsigned short toneFrequency;
    unsigned short toneDurationMs;
    bool tonePending;
    unsigned int toneRequestId;

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