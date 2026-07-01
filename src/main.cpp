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

    assert(cpu.getPC() == 0x0020);

    cpu.getMemory().write16(0x0020, 0xABCD);

    unsigned short instruction = cpu.fetch();

    assert(instruction == 0xABCD);
    assert(cpu.getPC() == 0x0022);

    cpu.getMemory().write8(0x0022, 0x34);
    cpu.getMemory().write8(0x0023, 0x12);

    instruction = cpu.fetch();

    assert(instruction == 0x1234);
    assert(cpu.getPC() == 0x0024);

    printf("[PASS] Fetch test passed\n");
}

void testSequentialPC() {
    CPU cpu;

    cpu.getMemory().write16(0x0020, 0x1111);
    cpu.getMemory().write16(0x0022, 0x2222);
    cpu.getMemory().write16(0x0024, 0x3333);

    assert(cpu.getPC() == 0x0020);
    assert(cpu.step() == 0x1111);
    assert(cpu.getLastInstruction() == 0x1111);
    assert(cpu.getPC() == 0x0022);

    assert(cpu.step() == 0x2222);
    assert(cpu.getLastInstruction() == 0x2222);
    assert(cpu.getPC() == 0x0024);

    assert(cpu.step() == 0x3333);
    assert(cpu.getLastInstruction() == 0x3333);
    assert(cpu.getPC() == 0x0026);

    printf("[PASS] Sequential PC test passed\n");
}

void testInstructionFields() {
    InstructionDecoder decoder;
    DecodedInstruction decoded;

    unsigned short word;

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

    word = (0x7F << 9) | (4 << 6) | (0 << 3) | 1;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 1);
    assert(decoded.format == ZX16::I_TYPE);
    assert(decoded.rd == 4);
    assert(decoded.immediate == -1);

    word = (7 << 12) | (2 << 9) | (1 << 6) | (0 << 3) | 2;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 2);
    assert(decoded.format == ZX16::B_TYPE);
    assert(decoded.rs1 == 1);
    assert(decoded.rs2 == 2);
    assert(decoded.immediate == 14);

    word = (8 << 12) | (2 << 9) | (1 << 6) | (0 << 3) | 2;
    decoded = decoder.decode(word);

    assert(decoded.format == ZX16::B_TYPE);
    assert(decoded.immediate == -16);

    word = (8 << 12) | (3 << 9) | (4 << 6) | (0 << 3) | 3;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 3);
    assert(decoded.format == ZX16::S_TYPE);
    assert(decoded.rs1 == 4);
    assert(decoded.rs2 == 3);
    assert(decoded.immediate == -8);

    word = (7 << 12) | (5 << 9) | (6 << 6) | (1 << 3) | 4;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 4);
    assert(decoded.format == ZX16::L_TYPE);
    assert(decoded.rd == 6);
    assert(decoded.rs1 == 5);
    assert(decoded.immediate == 7);

    word = (1 << 15) | (32 << 9) | (1 << 6) | (0 << 3) | 5;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 5);
    assert(decoded.format == ZX16::J_TYPE);
    assert(decoded.linkFlag == 1);
    assert(decoded.rd == 1);
    assert(decoded.immediate == -512);

    word = (1 << 15) | (37 << 9) | (2 << 6) | (2 << 3) | 6;
    decoded = decoder.decode(word);

    assert(decoded.opcode == 6);
    assert(decoded.format == ZX16::U_TYPE);
    assert(decoded.upperFlag == 1);
    assert(decoded.rd == 2);
    assert(decoded.immediate == 0x9500);

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

    cpu.getMemory().write16(0x0020, 0x0000);
    cpu.getMemory().write16(0x0022, 0x0001);
    cpu.getMemory().write16(0x0024, 0x0002);
    cpu.getMemory().write16(0x0026, 0x0003);
    cpu.getMemory().write16(0x0028, 0x0004);
    cpu.getMemory().write16(0x002A, 0x0005);
    cpu.getMemory().write16(0x002C, 0x0006);
    cpu.getMemory().write16(0x002E, 0x0007);

    cpu.step();
    assert(cpu.getLastHandler() == ZX16::R_TYPE);

    cpu.step();
    assert(cpu.getLastHandler() == ZX16::I_TYPE);

    cpu.step();
    assert(cpu.getLastHandler() == ZX16::B_TYPE);

    cpu.step();
    assert(cpu.getLastHandler() == ZX16::S_TYPE);

    cpu.step();
    assert(cpu.getLastHandler() == ZX16::L_TYPE);

    cpu.step();
    assert(cpu.getLastHandler() == ZX16::J_TYPE);

    cpu.step();
    assert(cpu.getLastHandler() == ZX16::U_TYPE);

    cpu.step();
    assert(cpu.getLastHandler() == ZX16::SYS_TYPE);

    assert(cpu.getPC() == 0x0030);

    printf("[PASS] Execution dispatcher test passed\n");
}

void testAddSubExecution() {
    CPU cpu;

    unsigned short word;

    cpu.getRegisters().setRegister(3, 5);
    cpu.getRegisters().setRegister(4, 7);

    word = (0x0 << 12) | (4 << 9) | (3 << 6) | 0;
    cpu.getMemory().write16(0x0020, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(3) == 12);
    assert(cpu.getPC() == 0x0022);

    cpu.getRegisters().setRegister(5, 20);
    cpu.getRegisters().setRegister(6, 8);

    word = (0x1 << 12) | (6 << 9) | (5 << 6) | 0;
    cpu.getMemory().write16(0x0022, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(5) == 12);
    assert(cpu.getPC() == 0x0024);

    cpu.getRegisters().setRegister(7, 3);
    cpu.getRegisters().setRegister(6, 5);

    word = (0x1 << 12) | (6 << 9) | (7 << 6) | 0;
    cpu.getMemory().write16(0x0024, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(7) == 0xFFFE);
    assert(cpu.getPC() == 0x0026);

    cpu.getRegisters().setRegister(0, 0xFFFF);
    cpu.getRegisters().setRegister(1, 1);

    word = (0x0 << 12) | (1 << 9) | (0 << 6) | 0;
    cpu.getMemory().write16(0x0026, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(0) == 0x0000);
    assert(cpu.getPC() == 0x0028);

    printf("[PASS] ADD/SUB execution test passed\n");
}

void testLogicalExecution() {
    CPU cpu;

    unsigned short word;

    cpu.getRegisters().setRegister(3, 0x00F0);
    cpu.getRegisters().setRegister(4, 0x0F00);

    word = (0x7 << 12) | (4 << 9) | (3 << 6) | 0;
    cpu.getMemory().write16(0x0020, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(3) == 0x0FF0);
    assert(cpu.getPC() == 0x0022);

    cpu.getRegisters().setRegister(5, 0x0FF0);
    cpu.getRegisters().setRegister(6, 0x00FF);

    word = (0x8 << 12) | (6 << 9) | (5 << 6) | 0;
    cpu.getMemory().write16(0x0022, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(5) == 0x00F0);
    assert(cpu.getPC() == 0x0024);

    cpu.getRegisters().setRegister(7, 0xAAAA);
    cpu.getRegisters().setRegister(1, 0x0F0F);

    word = (0x9 << 12) | (1 << 9) | (7 << 6) | 0;
    cpu.getMemory().write16(0x0024, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(7) == 0xA5A5);
    assert(cpu.getPC() == 0x0026);

    printf("[PASS] Logical operations test passed\n");
}

void testShiftExecution() {
    CPU cpu;

    unsigned short word;

    cpu.getRegisters().setRegister(3, 0x0001);
    cpu.getRegisters().setRegister(4, 20);

    word = (0x4 << 12) | (4 << 9) | (3 << 6) | 0;
    cpu.getMemory().write16(0x0020, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(3) == 0x0010);
    assert(cpu.getPC() == 0x0022);

    cpu.getRegisters().setRegister(5, 0x8000);
    cpu.getRegisters().setRegister(6, 4);

    word = (0x5 << 12) | (6 << 9) | (5 << 6) | 0;
    cpu.getMemory().write16(0x0022, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(5) == 0x0800);
    assert(cpu.getPC() == 0x0024);

    cpu.getRegisters().setRegister(7, 0x8000);
    cpu.getRegisters().setRegister(1, 4);

    word = (0x6 << 12) | (1 << 9) | (7 << 6) | 0;
    cpu.getMemory().write16(0x0024, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(7) == 0xF800);
    assert(cpu.getPC() == 0x0026);

    printf("[PASS] Shift operations test passed\n");
}

void testAddiExecution() {
    CPU cpu;

    unsigned short word;

    cpu.getRegisters().setRegister(3, 10);

    word = (5 << 9) | (3 << 6) | (0 << 3) | 1;
    cpu.getMemory().write16(0x0020, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(3) == 15);
    assert(cpu.getPC() == 0x0022);

    cpu.getRegisters().setRegister(4, 10);

    word = (0x7F << 9) | (4 << 6) | (0 << 3) | 1;
    cpu.getMemory().write16(0x0022, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(4) == 9);
    assert(cpu.getPC() == 0x0024);

    cpu.getRegisters().setRegister(5, 100);

    word = (0x40 << 9) | (5 << 6) | (0 << 3) | 1;
    cpu.getMemory().write16(0x0024, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(5) == 36);
    assert(cpu.getPC() == 0x0026);

    cpu.getRegisters().setRegister(6, 0xFFFF);

    word = (1 << 9) | (6 << 6) | (0 << 3) | 1;
    cpu.getMemory().write16(0x0026, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 0x0000);
    assert(cpu.getPC() == 0x0028);

    printf("[PASS] ADDI execution test passed\n");
}

void testImmediateLogicExecution() {
    CPU cpu;

    unsigned short word;

    cpu.getRegisters().setRegister(3, 0x00F0);

    word = (0x0F << 9) | (3 << 6) | (4 << 3) | 1;
    cpu.getMemory().write16(0x0020, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(3) == 0x00FF);
    assert(cpu.getPC() == 0x0022);

    cpu.getRegisters().setRegister(4, 0x0F00);

    word = (0x7F << 9) | (4 << 6) | (4 << 3) | 1;
    cpu.getMemory().write16(0x0022, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(4) == 0x0F7F);
    assert(cpu.getPC() == 0x0024);

    cpu.getRegisters().setRegister(5, 0x00FF);

    word = (0x0F << 9) | (5 << 6) | (5 << 3) | 1;
    cpu.getMemory().write16(0x0024, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(5) == 0x000F);
    assert(cpu.getPC() == 0x0026);

    cpu.getRegisters().setRegister(6, 0x1234);

    word = (0x7F << 9) | (6 << 6) | (5 << 3) | 1;
    cpu.getMemory().write16(0x0026, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 0x1234);
    assert(cpu.getPC() == 0x0028);

    cpu.getRegisters().setRegister(7, 0xAAAA);

    word = (0x7F << 9) | (7 << 6) | (6 << 3) | 1;
    cpu.getMemory().write16(0x0028, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(7) == 0x5555);
    assert(cpu.getPC() == 0x002A);

    printf("[PASS] Immediate logic test passed\n");
}

void testLiExecution() {
    CPU cpu;

    unsigned short word;

    word = (42 << 9) | (3 << 6) | (7 << 3) | 1;
    cpu.getMemory().write16(0x0020, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(3) == 42);
    assert(cpu.getPC() == 0x0022);

    word = (0x7F << 9) | (4 << 6) | (7 << 3) | 1;
    cpu.getMemory().write16(0x0022, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(4) == 0xFFFF);
    assert(cpu.getPC() == 0x0024);

    word = (0x40 << 9) | (5 << 6) | (7 << 3) | 1;
    cpu.getMemory().write16(0x0024, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(5) == 0xFFC0);
    assert(cpu.getPC() == 0x0026);

    word = (7 << 9) | (0 << 6) | (7 << 3) | 1;
    cpu.getMemory().write16(0x0026, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(0) == 7);
    assert(cpu.getPC() == 0x0028);

    printf("[PASS] LI execution test passed\n");
}

void testByteLoadExecution() {
    CPU cpu;

    unsigned short word;

    cpu.getRegisters().setRegister(3, 0x1000);

    cpu.getMemory().write8(0x1000, 0x7F);
    cpu.getMemory().write8(0x1001, 0x80);
    cpu.getMemory().write8(0x1002, 0xAB);

    word = (0 << 12) | (3 << 9) | (4 << 6) | (0 << 3) | 4;
    cpu.getMemory().write16(0x0020, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(4) == 0x007F);
    assert(cpu.getPC() == 0x0022);

    word = (1 << 12) | (3 << 9) | (5 << 6) | (0 << 3) | 4;
    cpu.getMemory().write16(0x0022, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(5) == 0xFF80);
    assert(cpu.getPC() == 0x0024);

    word = (1 << 12) | (3 << 9) | (6 << 6) | (4 << 3) | 4;
    cpu.getMemory().write16(0x0024, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 0x0080);
    assert(cpu.getPC() == 0x0026);

    word = (2 << 12) | (3 << 9) | (7 << 6) | (4 << 3) | 4;
    cpu.getMemory().write16(0x0026, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(7) == 0x00AB);
    assert(cpu.getPC() == 0x0028);

    printf("[PASS] Byte load test passed\n");
}

void testWordLoadExecution() {
    CPU cpu;

    unsigned short word;

    cpu.getRegisters().setRegister(3, 0x2000);

    cpu.getMemory().write16(0x2000, 0xABCD);
    cpu.getMemory().write8(0x2004, 0x34);
    cpu.getMemory().write8(0x2005, 0x12);

    word = (0 << 12) | (3 << 9) | (4 << 6) | (1 << 3) | 4;
    cpu.getMemory().write16(0x0020, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(4) == 0xABCD);
    assert(cpu.getPC() == 0x0022);

    word = (4 << 12) | (3 << 9) | (5 << 6) | (1 << 3) | 4;
    cpu.getMemory().write16(0x0022, word);

    cpu.step();

    assert(cpu.getRegisters().getRegister(5) == 0x1234);
    assert(cpu.getPC() == 0x0024);

    printf("[PASS] Word load test passed\n");
}

void testStoreExecution() {
    CPU cpu;

    unsigned short word;

    cpu.getRegisters().setRegister(3, 0x3000);

    cpu.getRegisters().setRegister(4, 0xABCD);

    word = (0 << 12) | (4 << 9) | (3 << 6) | (0 << 3) | 3;
    cpu.getMemory().write16(0x0020, word);

    cpu.step();

    assert(cpu.getMemory().read8(0x3000) == 0xCD);
    assert(cpu.getPC() == 0x0022);

    cpu.getRegisters().setRegister(5, 0x1234);

    word = (2 << 12) | (5 << 9) | (3 << 6) | (1 << 3) | 3;
    cpu.getMemory().write16(0x0022, word);

    cpu.step();

    assert(cpu.getMemory().read16(0x3002) == 0x1234);
    assert(cpu.getMemory().read8(0x3002) == 0x34);
    assert(cpu.getMemory().read8(0x3003) == 0x12);
    assert(cpu.getPC() == 0x0024);

    cpu.getRegisters().setRegister(6, 0x3010);
    cpu.getRegisters().setRegister(7, 0x00EE);

    word = (0xF << 12) | (7 << 9) | (6 << 6) | (0 << 3) | 3;
    cpu.getMemory().write16(0x0024, word);

    cpu.step();

    assert(cpu.getMemory().read8(0x300F) == 0xEE);
    assert(cpu.getPC() == 0x0026);

    cpu.getRegisters().setRegister(1, 0xBEEF);

    word = (0xE << 12) | (1 << 9) | (6 << 6) | (1 << 3) | 3;
    cpu.getMemory().write16(0x0026, word);

    cpu.step();

    assert(cpu.getMemory().read16(0x300E) == 0xBEEF);
    assert(cpu.getMemory().read8(0x300E) == 0xEF);
    assert(cpu.getMemory().read8(0x300F) == 0xBE);
    assert(cpu.getPC() == 0x0028);

    printf("[PASS] Store test passed\n");
}

void testBeqBneExecution() {
    CPU cpu;

    unsigned short word;

    // BEQ taken: x3 == x4, branch +4
    cpu.getRegisters().setRegister(3, 5);
    cpu.getRegisters().setRegister(4, 5);

    word = (2 << 12) | (4 << 9) | (3 << 6) | (0 << 3) | 2;
    cpu.getMemory().write16(0x0020, word);

    cpu.step();

    assert(cpu.getPC() == 0x0026);

    // BEQ not taken: x3 != x4
    cpu.setPC(0x0100);
    cpu.getRegisters().setRegister(3, 5);
    cpu.getRegisters().setRegister(4, 7);

    word = (2 << 12) | (4 << 9) | (3 << 6) | (0 << 3) | 2;
    cpu.getMemory().write16(0x0100, word);

    cpu.step();

    assert(cpu.getPC() == 0x0102);

    // BNE taken: x1 != x2, branch +6
    cpu.setPC(0x0200);
    cpu.getRegisters().setRegister(1, 10);
    cpu.getRegisters().setRegister(2, 20);

    word = (3 << 12) | (2 << 9) | (1 << 6) | (1 << 3) | 2;
    cpu.getMemory().write16(0x0200, word);

    cpu.step();

    assert(cpu.getPC() == 0x0208);

    // BNE not taken: x1 == x2
    cpu.setPC(0x0300);
    cpu.getRegisters().setRegister(1, 9);
    cpu.getRegisters().setRegister(2, 9);

    word = (3 << 12) | (2 << 9) | (1 << 6) | (1 << 3) | 2;
    cpu.getMemory().write16(0x0300, word);

    cpu.step();

    assert(cpu.getPC() == 0x0302);

    printf("[PASS] BEQ/BNE branch test passed\n");
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
    testLogicalExecution();
    testShiftExecution();
    testAddiExecution();
    testImmediateLogicExecution();
    testLiExecution();
    testByteLoadExecution();
    testWordLoadExecution();
    testStoreExecution();
    testBeqBneExecution();

    InitWindow(320, 240, "ZX16 Simulator");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);

        DrawText("ZX16 Simulator", 90, 2, 18, RAYWHITE);

        DrawText("Memory: PASSED", 10, 30, 9, GREEN);
        DrawText("Register file: PASSED", 10, 44, 9, GREEN);
        DrawText("CPU reset: PASSED", 10, 58, 9, GREEN);
        DrawText("Program loader: PASSED", 10, 72, 9, GREEN);
        DrawText("Fetch: PASSED", 10, 86, 9, GREEN);
        DrawText("Sequential PC: PASSED", 10, 100, 9, GREEN);
        DrawText("Instruction fields: PASSED", 10, 114, 9, GREEN);
        DrawText("Dispatcher: PASSED", 10, 128, 9, GREEN);
        DrawText("BEQ/BNE: PASSED", 10, 142, 9, GREEN);

        DrawText("ADD/SUB: PASSED", 165, 30, 9, GREEN);
        DrawText("Logical ops: PASSED", 165, 44, 9, GREEN);
        DrawText("Shift ops: PASSED", 165, 58, 9, GREEN);
        DrawText("ADDI: PASSED", 165, 72, 9, GREEN);
        DrawText("Immediate logic: PASSED", 165, 86, 9, GREEN);
        DrawText("LI: PASSED", 165, 100, 9, GREEN);
        DrawText("Byte load: PASSED", 165, 114, 9, GREEN);
        DrawText("Word load: PASSED", 165, 128, 9, GREEN);
        DrawText("Store: PASSED", 165, 142, 9, GREEN);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}