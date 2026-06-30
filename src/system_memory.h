#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <string>

class Memory {
public:
    std::array<uint8_t, 65536> data = {};  //64K memory, initialized to 0

    uint8_t  read8(uint16_t addr) const;
    void     write8(uint16_t addr, uint8_t val);
    uint16_t read16(uint16_t addr) const;
    void     write16(uint16_t addr, uint16_t val);

    void load_program(const std::vector<uint8_t>& bytes);   // loads at 0x0020
};