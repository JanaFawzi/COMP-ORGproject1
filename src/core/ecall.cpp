#include "cpu.h"

#include <stdio.h>

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

    if (instruction.service == 0x021) {
        unsigned short value = nextRandom();

        registers.setRegister(6, value);
    }

    if (instruction.service == 0x030) {
        registers.setRegister(6, keyboardKey);
    }

    if (instruction.service == 0x040) {
        unsigned short frequency = registers.getRegister(6);
        unsigned short durationMs = registers.getRegister(7);

        requestTone(frequency, durationMs);
    }

    if (instruction.service == 0x041) {
        unsigned short newVolume = registers.getRegister(6);

        setVolumePercent(newVolume);
    }

    if (instruction.service == 0x042) {
        requestStopAudio();
    }

    if (instruction.service == 0x050) {
        dumpRegisters();
    }

    if (instruction.service == 0x051) {
        unsigned short startAddress = registers.getRegister(6);
        unsigned short length = registers.getRegister(7);

        dumpMemory(startAddress, length);
    }

    if (instruction.service == 0x3FF) {
        halted = true;
    }
}
