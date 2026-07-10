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

unsigned short Memory::read16(unsigned short address) {
    unsigned short nextAddress = address + 1;

    unsigned char low = ram[address];
    unsigned char high = ram[nextAddress];

    return (unsigned short)(low | (high << 8));
}

void Memory::write16(unsigned short address, unsigned short value) {
    unsigned short nextAddress = address + 1;

    ram[address] = value & 0xFF;
    ram[nextAddress] = (value >> 8) & 0xFF;
}