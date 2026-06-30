#pragma once
#include <fstream>
#include <vector>
#include <stdexcept>
#include "system_memory.h"
//reads .bin files from disk and hands them to load_program().
class ProgramLoader {
public:
    static void load_file(const std::string& path, Memory& mem) {
        std::ifstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("cannot open file: " + path);

        std::vector<uint8_t> bytes(
            (std::istreambuf_iterator<char>(f)),
            std::istreambuf_iterator<char>()//reads entire file content into a string
        );

        mem.load_program(bytes);   //sends bytes to memory.
    }
};