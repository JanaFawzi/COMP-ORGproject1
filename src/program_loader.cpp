#include "program_loader.h"

#include <stdio.h>

int ProgramLoader::loadBin(Memory& memory, const char filename[]) {
    FILE* file = fopen(filename, "rb");     // Open binary file

    if (file == 0) {
        return -1;                          // File failed to open
    }

    int bytesLoaded = 0;
    int byteValue = fgetc(file);

    while (byteValue != EOF) {
        int address = LOAD_ADDRESS + bytesLoaded;

        if (address >= Memory::RAM_SIZE) {
            break;                          // Stop if RAM is full
        }

        memory.write8((unsigned short)address, (unsigned char)byteValue);

        bytesLoaded++;
        byteValue = fgetc(file);
    }

    fclose(file);

    return bytesLoaded;
}