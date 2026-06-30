#include "system_memory.h"
#include <cassert>
#include <stdexcept>

uint8_t Memory::read8(uint16_t addr) const {
    return data[addr];
}

void Memory::write8(uint16_t addr, uint8_t val) {
    data[addr] = val;
}

uint16_t Memory::read16(uint16_t addr) const {
    assert(addr % 2 == 0 && "LW must be even-aligned");
    
    return static_cast<uint16_t>(data[addr]) |   //low byte at addr OR high byte at addr+1
           (static_cast<uint16_t>(data[addr + 1]) << 8);
}

void Memory::write16(uint16_t addr, uint16_t val) {
    assert(addr % 2 == 0 && "SW must be even-aligned");
    data[addr] = val & 0xFF;  //low byte
    data[addr + 1] = (val >> 8) & 0xFF;  //high byte
}

void Memory::load_program(const std::vector<uint8_t>& bytes) {
    if (bytes.size() > (0xF000 - 0x0020))   //program must fit in code region
        throw std::runtime_error("program too large for code region");

    for (size_t i = 0; i < bytes.size(); i++) {
        data[0x0020 + i] = bytes[i];        //copy bytes into memory
    }
}