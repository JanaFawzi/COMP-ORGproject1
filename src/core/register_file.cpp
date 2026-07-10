#include "register_file.h"

RegisterFile::RegisterFile() {
    reset();
}

void RegisterFile::reset() {
    for (int i = 0; i < REGISTER_COUNT; i++) {
        registers[i] = 0;
    }

    // x2 is the stack pointer
    registers[2] = 0xEFFE;
}

unsigned short RegisterFile::getRegister(int index) {
    if (index < 0 || index >= REGISTER_COUNT) {
        return 0;
    }

    return registers[index];
}

void RegisterFile::setRegister(int index, unsigned short value) {
    if (index < 0 || index >= REGISTER_COUNT) {
        return;
    }

    registers[index] = value;
}
