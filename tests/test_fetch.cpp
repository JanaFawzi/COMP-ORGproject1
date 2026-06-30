// test_fetch.cpp
#include <cassert>
#include <iostream>
#include <vector>
#include "../src/system_memory.h"
#include "../src/program_loader.h"

void test_read_write_byte() {
    Memory mem;
    mem.write8(0x0020, 0xAB);
    assert(mem.read8(0x0020) == 0xAB);
    std::cout << "test_read_write_byte PASSED\n";
}

void test_little_endian() {
    Memory mem;
    mem.write16(0x0020, 0xABCD);
    assert(mem.read8(0x0020) == 0xCD);   // low byte at lower address
    assert(mem.read8(0x0021) == 0xAB);   // high byte at higher address
    assert(mem.read16(0x0020) == 0xABCD);
    std::cout << "test_little_endian PASSED\n";
}

void test_fetch_two_instructions() {
    Memory mem;
    // place two fake 16-bit instructions manually
    mem.write16(0x0020, 0x1234);   // first instruction
    mem.write16(0x0022, 0x5678);   // second instruction

    // simulate what fetch() will do: read16 then advance by 2
    uint16_t pc = 0x0020;

    uint16_t word1 = mem.read16(pc);
    pc += 2;
    assert(word1 == 0x1234);
    assert(pc == 0x0022);

    uint16_t word2 = mem.read16(pc);
    pc += 2;
    assert(word2 == 0x5678);
    assert(pc == 0x0024);

    std::cout << "test_fetch_two_instructions PASSED\n";
}

void test_load_program_starts_at_0x0020() {
    Memory mem;
    std::vector<uint8_t> fake_program = {0xCD, 0xAB, 0x78, 0x56};  // two little-endian words
    mem.load_program(fake_program);

    // 0xABCD should be at 0x0020
    assert(mem.read16(0x0020) == 0xABCD);
    // 0x5678 should be at 0x0022
    assert(mem.read16(0x0022) == 0x5678);
    // address 0x0000 must be untouched — program does NOT load at 0x0000
    assert(mem.read16(0x0000) == 0x0000);

    std::cout << "test_load_program_starts_at_0x0020 PASSED\n";
}

int main() {
    test_read_write_byte();
    test_little_endian();
    test_fetch_two_instructions();
    test_load_program_starts_at_0x0020();
    std::cout << "ALL TESTS PASSED\n";
    return 0;
}