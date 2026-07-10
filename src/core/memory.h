#ifndef MEMORY_H
#define MEMORY_H

class Memory {
public:
    static const int RAM_SIZE = 65536; //64KB

    Memory();

    void reset();

    unsigned char read8(unsigned short address);
    void write8(unsigned short address, unsigned char value);

    unsigned short read16(unsigned short address);
    void write16(unsigned short address, unsigned short value);

private:
    unsigned char ram[RAM_SIZE];
};

#endif