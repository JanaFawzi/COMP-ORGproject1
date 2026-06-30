#pragma once
#include <cstdint>
#include <functional>
#include <string>

struct CPUState {
    uint16_t regs[8]= {0, 0, 0xEFFE, 0, 0, 0, 0, 0}; //x0..x7, ALL writable, x0 is NOT zero, x2(sp) = 0xEFFE
    uint16_t pc= 0x0020;   //reset entry point
    bool halted  = false;     //excution state, set it to true to stop looping
};

//Person 2 implements these. Person 1 calls them. Person 3 stubs them in tests.
struct SimEvents {
    std::function<void(const std::string&)> on_print  = [](auto){}; //handles the output for print calls
    std::function<void()>                   on_halt   = []{};  //handles halt events
};