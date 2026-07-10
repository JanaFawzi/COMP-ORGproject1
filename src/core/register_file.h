#ifndef REGISTER_FILE_H
#define REGISTER_FILE_H

class RegisterFile {
public:
    static const int REGISTER_COUNT = 8;

    RegisterFile();

    void reset();

    unsigned short getRegister(int index);
    void setRegister(int index, unsigned short value);

private:
    unsigned short registers[REGISTER_COUNT];
};

#endif