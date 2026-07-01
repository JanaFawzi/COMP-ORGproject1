#include "raylib.h"
#include "memory.h"
#include "register_file.h"
#include "cpu.h"
#include "program_loader.h"
#include "instruction_decoder.h"

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

void testFetch() {
    CPU cpu;

    // CPU starts fetching from PC = 0x0020
    assert(cpu.getPC() == 0x0020);

    // Put fake 16-bit instruction in RAM at PC
    cpu.getMemory().write16(0x0020, 0xABCD);

    // Fetch should read the instruction from RAM
    unsigned short instruction = cpu.fetch();

    assert(instruction == 0xABCD);

    // Fetch should move PC to the next instruction
    assert(cpu.getPC() == 0x0022);

    // Test another instruction using raw bytes
    cpu.getMemory().write8(0x0022, 0x34);
    cpu.getMemory().write8(0x0023, 0x12);

    instruction = cpu.fetch();

    assert(instruction == 0x1234);
    assert(cpu.getPC() == 0x0024);

    printf("[PASS] Fetch test passed\n");
}

void testSequentialPC() {
    CPU cpu;

    // Three fake 16-bit instructions in consecutive memory locations
    cpu.getMemory().write16(0x0020, 0x1111);
    cpu.getMemory().write16(0x0022, 0x2222);
    cpu.getMemory().write16(0x0024, 0x3333);

    // First step
    assert(cpu.getPC() == 0x0020);
    assert(cpu.step() == 0x1111);
    assert(cpu.getLastInstruction() == 0x1111);
    assert(cpu.getPC() == 0x0022);

    // Second step
    assert(cpu.step() == 0x2222);
    assert(cpu.getLastInstruction() == 0x2222);
    assert(cpu.getPC() == 0x0024);

    // Third step
    assert(cpu.step() == 0x3333);
    assert(cpu.getLastInstruction() == 0x3333);
    assert(cpu.getPC() == 0x0026);

    printf("[PASS] Sequential PC test passed\n");
}

void testInstructionFields() {
    InstructionDecoder decoder;
    DecodedInstruction decoded;

    unsigned short word;

    // R-Type: funct4=10, rs2=5, rd/rs1=3, func3=2, opcode=0
    word = (10 << 12) | (5 << 9) | (3 << 6) | (2 << 3) | 0;
    decoded = decoder.decode(word);

    assert(decoded.word == word);
    assert(decoded.opcode == 0);
    assert(decoded.format == ZX16::R_TYPE);
    assert(decoded.funct4 == 10);
    assert(decoded.func3 == 2);
    assert(decoded.rd == 3);
    assert(decoded.rs1 == 3);
    assert(decoded.rs2 == 5);

    // I-Type: rd=4, immediate=-1
    word = (0x7F << 9) | (4 << 6) | (0 << 3) | 1;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 1);
    assert(decoded.format == ZX16::I_TYPE);
    assert(decoded.rd == 4);
    assert(decoded.immediate == -1);

    // B-Type: rs1=1, rs2=2, immediate=+14
    word = (7 << 12) | (2 << 9) | (1 << 6) | (0 << 3) | 2;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 2);
    assert(decoded.format == ZX16::B_TYPE);
    assert(decoded.rs1 == 1);
    assert(decoded.rs2 == 2);
    assert(decoded.immediate == 14);

    // B-Type negative immediate: immediate=-16
    word = (8 << 12) | (2 << 9) | (1 << 6) | (0 << 3) | 2;
    decoded = decoder.decode(word);

    assert(decoded.format == ZX16::B_TYPE);
    assert(decoded.immediate == -16);

    // S-Type: rs1=4, rs2=3, immediate=-8
    word = (8 << 12) | (3 << 9) | (4 << 6) | (0 << 3) | 3;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 3);
    assert(decoded.format == ZX16::S_TYPE);
    assert(decoded.rs1 == 4);
    assert(decoded.rs2 == 3);
    assert(decoded.immediate == -8);

    // L-Type: rd=6, base rs1=5, immediate=+7
    word = (7 << 12) | (5 << 9) | (6 << 6) | (1 << 3) | 4;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 4);
    assert(decoded.format == ZX16::L_TYPE);
    assert(decoded.rd == 6);
    assert(decoded.rs1 == 5);
    assert(decoded.immediate == 7);

    // J-Type: link=1, rd=1, immediate=-512
    word = (1 << 15) | (32 << 9) | (1 << 6) | (0 << 3) | 5;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 5);
    assert(decoded.format == ZX16::J_TYPE);
    assert(decoded.linkFlag == 1);
    assert(decoded.rd == 1);
    assert(decoded.immediate == -512);

    // U-Type: flag=1, rd=2, imm9=0x12A, immediate=0x12A << 7
    word = (1 << 15) | (37 << 9) | (2 << 6) | (2 << 3) | 6;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 6);
    assert(decoded.format == ZX16::U_TYPE);
    assert(decoded.upperFlag == 1);
    assert(decoded.rd == 2);
    assert(decoded.immediate == 0x9500);

    // SYS-Type: service=0x3FF
    word = (0x3FF << 6) | 7;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 7);
    assert(decoded.format == ZX16::SYS_TYPE);
    assert(decoded.service == 0x3FF);
    assert(decoded.immediate == 0x3FF);

    printf("[PASS] Instruction fields test passed\n");
}

void testExecutionDispatcher() {
    CPU cpu;

    // One fake instruction for each opcode/format
    cpu.getMemory().write16(0x0020, 0x0000); // opcode 000 -> R-Type
    cpu.getMemory().write16(0x0022, 0x0001); // opcode 001 -> I-Type
    cpu.getMemory().write16(0x0024, 0x0002); // opcode 010 -> B-Type
    cpu.getMemory().write16(0x0026, 0x0003); // opcode 011 -> S-Type
    cpu.getMemory().write16(0x0028, 0x0004); // opcode 100 -> L-Type
    cpu.getMemory().write16(0x002A, 0x0005); // opcode 101 -> J-Type
    cpu.getMemory().write16(0x002C, 0x0006); // opcode 110 -> U-Type
    cpu.getMemory().write16(0x002E, 0x0007); // opcode 111 -> SYS-Type

    cpu.step();
    assert(cpu.getLastInstruction() == 0x0000);
    assert(cpu.getLastHandler() == ZX16::R_TYPE);

    cpu.step();
    assert(cpu.getLastInstruction() == 0x0001);
    assert(cpu.getLastHandler() == ZX16::I_TYPE);

    cpu.step();
    assert(cpu.getLastInstruction() == 0x0002);
    assert(cpu.getLastHandler() == ZX16::B_TYPE);

    cpu.step();
    assert(cpu.getLastInstruction() == 0x0003);
    assert(cpu.getLastHandler() == ZX16::S_TYPE);

    cpu.step();
    assert(cpu.getLastInstruction() == 0x0004);
    assert(cpu.getLastHandler() == ZX16::L_TYPE);

    cpu.step();
    assert(cpu.getLastInstruction() == 0x0005);
    assert(cpu.getLastHandler() == ZX16::J_TYPE);

    cpu.step();
    assert(cpu.getLastInstruction() == 0x0006);
    assert(cpu.getLastHandler() == ZX16::U_TYPE);

    cpu.step();
    assert(cpu.getLastInstruction() == 0x0007);
    assert(cpu.getLastHandler() == ZX16::SYS_TYPE);

    assert(cpu.getPC() == 0x0030);

    printf("[PASS] Execution dispatcher test passed\n");
}

void testAddSubExecution() {
    CPU cpu;

    unsigned short addWord;
    unsigned short subWord;

    // ADD x3, x4
    // x3 = x3 + x4
    cpu.getRegisters().setRegister(3, 5);
    cpu.getRegisters().setRegister(4, 7);

    addWord = (0x0 << 12) | (4 << 9) | (3 << 6) | 0;
    cpu.getMemory().write16(0x0020, addWord);

    cpu.step();

    assert(cpu.getRegisters().getRegister(3) == 12);
    assert(cpu.getRegisters().getRegister(4) == 7);
    assert(cpu.getLastHandler() == ZX16::R_TYPE);
    assert(cpu.getPC() == 0x0022);

    // SUB x5, x6
    // x5 = x5 - x6
    cpu.getRegisters().setRegister(5, 20);
    cpu.getRegisters().setRegister(6, 8);

    subWord = (0x1 << 12) | (6 << 9) | (5 << 6) | 0;
    cpu.getMemory().write16(0x0022, subWord);

    cpu.step();

    assert(cpu.getRegisters().getRegister(5) == 12);
    assert(cpu.getRegisters().getRegister(6) == 8);
    assert(cpu.getLastHandler() == ZX16::R_TYPE);
    assert(cpu.getPC() == 0x0024);

    // SUB wraparound test: 3 - 5 = 0xFFFE in 16-bit arithmetic
    cpu.getRegisters().setRegister(7, 3);
    cpu.getRegisters().setRegister(6, 5);

    subWord = (0x1 << 12) | (6 << 9) | (7 << 6) | 0;
    cpu.getMemory().write16(0x0024, subWord);

    cpu.step();

    assert(cpu.getRegisters().getRegister(7) == 0xFFFE);
    assert(cpu.getPC() == 0x0026);

    // ADD wraparound and x0 writable test: 0xFFFF + 1 = 0x0000
    cpu.getRegisters().setRegister(0, 0xFFFF);
    cpu.getRegisters().setRegister(1, 1);

    addWord = (0x0 << 12) | (1 << 9) | (0 << 6) | 0;
    cpu.getMemory().write16(0x0026, addWord);

    cpu.step();

    assert(cpu.getRegisters().getRegister(0) == 0x0000);
    assert(cpu.getPC() == 0x0028);

    printf("[PASS] ADD/SUB execution test passed\n");
}

int main() {
    testMemory();
    testRegisterFile();
    testCPUReset();
    testProgramLoader();
    testFetch();
    testSequentialPC();
    testInstructionFields();
    testExecutionDispatcher();
    testAddSubExecution();

    InitWindow(320, 240, "ZX16 Simulator");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);

        DrawText("ZX16 Simulator", 90, 5, 20, RAYWHITE);
        DrawText("Memory read/write test: PASSED", 45, 35, 16, GREEN);
        DrawText("Register file test: PASSED", 55, 58, 16, GREEN);
        DrawText("CPU reset test: PASSED", 70, 81, 16, GREEN);
        DrawText("Program loader test: PASSED", 55, 104, 16, GREEN);
        DrawText("Fetch test: PASSED", 90, 127, 16, GREEN);
        DrawText("Sequential PC test: PASSED", 65, 150, 16, GREEN);
        DrawText("Instruction fields test: PASSED", 45, 173, 16, GREEN);
        DrawText("Execution dispatcher test: PASSED", 35, 196, 16, GREEN);
        DrawText("ADD/SUB execution test: PASSED", 40, 219, 16, GREEN);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}