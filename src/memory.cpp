#include "memory.h"

Memory::Memory() {
    reset();
}

void Memory::reset() {
    for (int i = 0; i < RAM_SIZE; i++) {
        ram[i] = 0;
    }
}

unsigned char Memory::read8(unsigned short address) {
    return ram[address];
}

void Memory::write8(unsigned short address, unsigned char value) {
    ram[address] = value;
}