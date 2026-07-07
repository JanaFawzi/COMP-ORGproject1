#include "program_loader.h"

#include <stdio.h>

int ProgramLoader::loadBin(Memory& memory, const char filename[]) {
    FILE* file = fopen(filename, "rb");

    if (file == 0) {
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return -1;
    }

    long fileSize = ftell(file);

    if (fileSize < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return -1;
    }

    int loadAddress = LOAD_ADDRESS;

    if (fileSize == FULL_MEMORY_IMAGE_SIZE) {
        loadAddress = 0;
    }

    int bytesLoaded = 0;
    int byteValue = fgetc(file);

    while (byteValue != EOF) {
        int address = loadAddress + bytesLoaded;

        if (address >= Memory::RAM_SIZE) {
            break;
        }

        memory.write8((unsigned short)address, (unsigned char)byteValue);

        bytesLoaded++;
        byteValue = fgetc(file);
    }

    fclose(file);

    return bytesLoaded;
}
