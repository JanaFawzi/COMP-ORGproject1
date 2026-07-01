#ifndef PROGRAM_LOADER_H
#define PROGRAM_LOADER_H

#include "memory.h"

class ProgramLoader {
public:
    static const int LOAD_ADDRESS = 0x0020;

    // Load .bin file into RAM starting at 0x0020
    // Returns number of bytes loaded, or -1 if file failed to open
    static int loadBin(Memory& memory, const char filename[]);
};

#endif