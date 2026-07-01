#include "raylib.h"
#include "memory.h"
#include "register_file.h"
#include "cpu.h"
#include "program_loader.h"

#include <assert.h>
#include <stdio.h>

void testMemory() {
    Memory memory;

    memory.write8(0x0000, 0x12);
    memory.write8(0x0020, 0xAB);
    memory.write8(0xF000, 0x05);

    assert(memory.read8(0x0000) == 0x12);
    assert(memory.read8(0x0020) == 0xAB);
    assert(memory.read8(0xF000) == 0x05);

    memory.write16(0x1000, 0xABCD);

    assert(memory.read16(0x1000) == 0xABCD);
    assert(memory.read8(0x1000) == 0xCD);
    assert(memory.read8(0x1001) == 0xAB);

    memory.write8(0x2000, 0x34);
    memory.write8(0x2001, 0x12);

    assert(memory.read16(0x2000) == 0x1234);

    memory.write16(0xEFFC, 0xBEEF);

    assert(memory.read16(0xEFFC) == 0xBEEF);
    assert(memory.read8(0xEFFC) == 0xEF);
    assert(memory.read8(0xEFFD) == 0xBE);

    printf("[PASS] Memory read/write tests passed\n");
}

void testRegisterFile() {
    RegisterFile registers;

    assert(registers.getRegister(0) == 0x0000);
    assert(registers.getRegister(1) == 0x0000);
    assert(registers.getRegister(2) == 0xEFFE);
    assert(registers.getRegister(3) == 0x0000);
    assert(registers.getRegister(4) == 0x0000);
    assert(registers.getRegister(5) == 0x0000);
    assert(registers.getRegister(6) == 0x0000);
    assert(registers.getRegister(7) == 0x0000);

    registers.setRegister(0, 0x1111);
    registers.setRegister(1, 0x2222);
    registers.setRegister(2, 0x3333);
    registers.setRegister(3, 0x4444);
    registers.setRegister(4, 0x5555);
    registers.setRegister(5, 0x6666);
    registers.setRegister(6, 0x7777);
    registers.setRegister(7, 0x8888);

    assert(registers.getRegister(0) == 0x1111);
    assert(registers.getRegister(1) == 0x2222);
    assert(registers.getRegister(2) == 0x3333);
    assert(registers.getRegister(3) == 0x4444);
    assert(registers.getRegister(4) == 0x5555);
    assert(registers.getRegister(5) == 0x6666);
    assert(registers.getRegister(6) == 0x7777);
    assert(registers.getRegister(7) == 0x8888);

    // x0 is normal in ZX16, not hardwired to zero
    registers.setRegister(0, 0xABCD);
    assert(registers.getRegister(0) == 0xABCD);

    registers.reset();

    assert(registers.getRegister(0) == 0x0000);
    assert(registers.getRegister(1) == 0x0000);
    assert(registers.getRegister(2) == 0xEFFE);
    assert(registers.getRegister(3) == 0x0000);
    assert(registers.getRegister(4) == 0x0000);
    assert(registers.getRegister(5) == 0x0000);
    assert(registers.getRegister(6) == 0x0000);
    assert(registers.getRegister(7) == 0x0000);

    printf("[PASS] Register file tests passed\n");
}

void testCPUReset() {
    CPU cpu;

    assert(cpu.getPC() == 0x0020);
    assert(cpu.getSP() == 0xEFFE);

    cpu.setPC(0x1234);
    cpu.setSP(0xABCD);

    assert(cpu.getPC() == 0x1234);
    assert(cpu.getSP() == 0xABCD);

    cpu.reset();

    assert(cpu.getPC() == 0x0020);
    assert(cpu.getSP() == 0xEFFE);

    printf("[PASS] CPU reset test passed\n");
}

void createLoaderTestFile(const char filename[]) {
    FILE* file = fopen(filename, "wb");

    assert(file != 0);

    // Fake .bin bytes for testing
    unsigned char data[6] = {
        0x12, 0x34, 0xAB, 0xCD, 0x00, 0xFF
    };

    for (int i = 0; i < 6; i++) {
        fputc(data[i], file);
    }

    fclose(file);
}

void testProgramLoader() {
    Memory memory;

    const char filename[] = "loader_test.bin";

    unsigned char expected[6] = {
        0x12, 0x34, 0xAB, 0xCD, 0x00, 0xFF
    };

    createLoaderTestFile(filename);

    int bytesLoaded = ProgramLoader::loadBin(memory, filename);

    assert(bytesLoaded == 6);

    // Program must be loaded starting at 0x0020
    for (int i = 0; i < 6; i++) {
        assert(memory.read8(0x0020 + i) == expected[i]);
    }

    // Make sure bytes before and after were not changed
    assert(memory.read8(0x001F) == 0x00);
    assert(memory.read8(0x0026) == 0x00);

    remove(filename);

    printf("[PASS] Program loader test passed\n");
}

int main() {
    testMemory();
    testRegisterFile();
    testCPUReset();
    testProgramLoader();

    InitWindow(320, 240, "ZX16 Simulator");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);

        DrawText("ZX16 Simulator", 90, 50, 20, RAYWHITE);
        DrawText("Memory read/write test: PASSED", 45, 90, 16, GREEN);
        DrawText("Register file test: PASSED", 55, 115, 16, GREEN);
        DrawText("CPU reset test: PASSED", 70, 140, 16, GREEN);
        DrawText("Program loader test: PASSED", 55, 165, 16, GREEN);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}