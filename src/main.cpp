#include "raylib.h"
#include "memory.h"
#include "register_file.h"
#include "cpu.h"
#include "program_loader.h"
#include "instruction_decoder.h"
#include "gui.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

unsigned short makeR(int funct4, int rs2, int rd, int func3) {
    return (funct4 << 12) | (rs2 << 9) | (rd << 6) | (func3 << 3) | 0;
}

unsigned short makeI(int imm7, int rd, int func3) {
    return ((imm7 & 0x7F) << 9) | (rd << 6) | (func3 << 3) | 1;
}

unsigned short makeB(int imm4, int rs2, int rs1, int func3) {
    return ((imm4 & 0xF) << 12) | (rs2 << 9) | (rs1 << 6) | (func3 << 3) | 2;
}

unsigned short makeS(int imm4, int rs2, int rs1, int func3) {
    return ((imm4 & 0xF) << 12) | (rs2 << 9) | (rs1 << 6) | (func3 << 3) | 3;
}

unsigned short makeL(int imm4, int rs1, int rd, int func3) {
    return ((imm4 & 0xF) << 12) | (rs1 << 9) | (rd << 6) | (func3 << 3) | 4;
}

unsigned short makeJ(int linkFlag, int immLow3, int immHigh6, int rd) {
    return (linkFlag << 15) | ((immHigh6 & 0x3F) << 9) | (rd << 6) | ((immLow3 & 0x7) << 3) | 5;
}

unsigned short makeSys(int service) {
    return ((service & 0x3FF) << 6) | 7;
}

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

    printf("[PASS] Memory test passed\n");
}

void testRegisterFile() {
    RegisterFile registers;

    assert(registers.getRegister(0) == 0x0000);
    assert(registers.getRegister(2) == 0xEFFE);

    for (int i = 0; i < 8; i++) {
        registers.setRegister(i, 0x1111 + i);
        assert(registers.getRegister(i) == 0x1111 + i);
    }

    registers.reset();

    assert(registers.getRegister(0) == 0x0000);
    assert(registers.getRegister(2) == 0xEFFE);

    printf("[PASS] Register file test passed\n");
}

void testCPUReset() {
    CPU cpu;

    assert(cpu.getPC() == 0x0020);
    assert(cpu.getSP() == 0xEFFE);

    cpu.setPC(0x1234);
    cpu.setSP(0xABCD);

    cpu.reset();

    assert(cpu.getPC() == 0x0020);
    assert(cpu.getSP() == 0xEFFE);

    printf("[PASS] CPU reset test passed\n");
}

void createLoaderTestFile(const char filename[]) {
    FILE* file = fopen(filename, "wb");

    assert(file != 0);

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

    for (int i = 0; i < 6; i++) {
        assert(memory.read8(0x0020 + i) == expected[i]);
    }

    remove(filename);

    printf("[PASS] Program loader test passed\n");
}

void testFetch() {
    CPU cpu;

    cpu.getMemory().write16(0x0020, 0xABCD);

    assert(cpu.fetch() == 0xABCD);
    assert(cpu.getPC() == 0x0022);

    printf("[PASS] Fetch test passed\n");
}

void testSequentialPC() {
    CPU cpu;

    cpu.getMemory().write16(0x0020, 0x1111);
    cpu.getMemory().write16(0x0022, 0x2222);
    cpu.getMemory().write16(0x0024, 0x3333);

    assert(cpu.step() == 0x1111);
    assert(cpu.getPC() == 0x0022);

    assert(cpu.step() == 0x2222);
    assert(cpu.getPC() == 0x0024);

    assert(cpu.step() == 0x3333);
    assert(cpu.getPC() == 0x0026);

    printf("[PASS] Sequential PC test passed\n");
}

void testInstructionFields() {
    InstructionDecoder decoder;
    DecodedInstruction decoded;

    decoded = decoder.decode(makeR(10, 5, 3, 2));

    assert(decoded.opcode == 0);
    assert(decoded.format == ZX16::R_TYPE);
    assert(decoded.funct4 == 10);
    assert(decoded.func3 == 2);
    assert(decoded.rd == 3);
    assert(decoded.rs1 == 3);
    assert(decoded.rs2 == 5);

    decoded = decoder.decode(makeI(0x7F, 4, 0));

    assert(decoded.opcode == 1);
    assert(decoded.format == ZX16::I_TYPE);
    assert(decoded.rd == 4);
    assert(decoded.immediate == -1);

    decoded = decoder.decode(makeB(7, 2, 1, 0));

    assert(decoded.opcode == 2);
    assert(decoded.format == ZX16::B_TYPE);
    assert(decoded.rs1 == 1);
    assert(decoded.rs2 == 2);
    assert(decoded.immediate == 14);

    decoded = decoder.decode(makeS(8, 3, 4, 0));

    assert(decoded.opcode == 3);
    assert(decoded.format == ZX16::S_TYPE);
    assert(decoded.rs1 == 4);
    assert(decoded.rs2 == 3);
    assert(decoded.immediate == -8);

    decoded = decoder.decode(makeL(7, 5, 6, 1));

    assert(decoded.opcode == 4);
    assert(decoded.format == ZX16::L_TYPE);
    assert(decoded.rd == 6);
    assert(decoded.rs1 == 5);
    assert(decoded.immediate == 7);

    decoded = decoder.decode(makeSys(0x3FF));

    assert(decoded.opcode == 7);
    assert(decoded.format == ZX16::SYS_TYPE);
    assert(decoded.service == 0x3FF);

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

    printf("[PASS] Execution dispatcher test passed\n");
}

void testAddSubExecution() {
    CPU cpu;

    cpu.getRegisters().setRegister(3, 5);
    cpu.getRegisters().setRegister(4, 7);

    cpu.getMemory().write16(0x0020, makeR(0x0, 4, 3, 0));
    cpu.step();

    assert(cpu.getRegisters().getRegister(3) == 12);

    cpu.getRegisters().setRegister(5, 20);
    cpu.getRegisters().setRegister(6, 8);

    cpu.getMemory().write16(0x0022, makeR(0x1, 6, 5, 0));
    cpu.step();

    assert(cpu.getRegisters().getRegister(5) == 12);

    printf("[PASS] ADD/SUB test passed\n");
}

void testLogicalExecution() {
    CPU cpu;

    cpu.getRegisters().setRegister(3, 0x00F0);
    cpu.getRegisters().setRegister(4, 0x0F00);
    cpu.getMemory().write16(0x0020, makeR(0x7, 4, 3, 0));
    cpu.step();
    assert(cpu.getRegisters().getRegister(3) == 0x0FF0);

    cpu.getRegisters().setRegister(5, 0x0FF0);
    cpu.getRegisters().setRegister(6, 0x00FF);
    cpu.getMemory().write16(0x0022, makeR(0x8, 6, 5, 0));
    cpu.step();
    assert(cpu.getRegisters().getRegister(5) == 0x00F0);

    cpu.getRegisters().setRegister(7, 0xAAAA);
    cpu.getRegisters().setRegister(1, 0x0F0F);
    cpu.getMemory().write16(0x0024, makeR(0x9, 1, 7, 0));
    cpu.step();
    assert(cpu.getRegisters().getRegister(7) == 0xA5A5);

    printf("[PASS] Logical operations test passed\n");
}

void testShiftExecution() {
    CPU cpu;

    cpu.getRegisters().setRegister(3, 0x0001);
    cpu.getRegisters().setRegister(4, 20);
    cpu.getMemory().write16(0x0020, makeR(0x4, 4, 3, 0));
    cpu.step();
    assert(cpu.getRegisters().getRegister(3) == 0x0010);

    cpu.getRegisters().setRegister(5, 0x8000);
    cpu.getRegisters().setRegister(6, 4);
    cpu.getMemory().write16(0x0022, makeR(0x5, 6, 5, 0));
    cpu.step();
    assert(cpu.getRegisters().getRegister(5) == 0x0800);

    cpu.getRegisters().setRegister(7, 0x8000);
    cpu.getRegisters().setRegister(1, 4);
    cpu.getMemory().write16(0x0024, makeR(0x6, 1, 7, 0));
    cpu.step();
    assert(cpu.getRegisters().getRegister(7) == 0xF800);

    printf("[PASS] Shift operations test passed\n");
}

void testAddiExecution() {
    CPU cpu;

    cpu.getRegisters().setRegister(3, 10);
    cpu.getMemory().write16(0x0020, makeI(5, 3, 0));
    cpu.step();
    assert(cpu.getRegisters().getRegister(3) == 15);

    cpu.getRegisters().setRegister(4, 10);
    cpu.getMemory().write16(0x0022, makeI(0x7F, 4, 0));
    cpu.step();
    assert(cpu.getRegisters().getRegister(4) == 9);

    cpu.getRegisters().setRegister(5, 100);
    cpu.getMemory().write16(0x0024, makeI(0x40, 5, 0));
    cpu.step();
    assert(cpu.getRegisters().getRegister(5) == 36);

    printf("[PASS] ADDI test passed\n");
}

void testImmediateLogicExecution() {
    CPU cpu;

    cpu.getRegisters().setRegister(3, 0x00F0);
    cpu.getMemory().write16(0x0020, makeI(0x0F, 3, 4));
    cpu.step();
    assert(cpu.getRegisters().getRegister(3) == 0x00FF);

    cpu.getRegisters().setRegister(5, 0x00FF);
    cpu.getMemory().write16(0x0022, makeI(0x0F, 5, 5));
    cpu.step();
    assert(cpu.getRegisters().getRegister(5) == 0x000F);

    cpu.getRegisters().setRegister(7, 0xAAAA);
    cpu.getMemory().write16(0x0024, makeI(0x7F, 7, 6));
    cpu.step();
    assert(cpu.getRegisters().getRegister(7) == 0x5555);

    printf("[PASS] Immediate logic test passed\n");
}

void testLiExecution() {
    CPU cpu;

    cpu.getMemory().write16(0x0020, makeI(42, 3, 7));
    cpu.step();
    assert(cpu.getRegisters().getRegister(3) == 42);

    cpu.getMemory().write16(0x0022, makeI(0x7F, 4, 7));
    cpu.step();
    assert(cpu.getRegisters().getRegister(4) == 0xFFFF);

    cpu.getMemory().write16(0x0024, makeI(0x40, 5, 7));
    cpu.step();
    assert(cpu.getRegisters().getRegister(5) == 0xFFC0);

    printf("[PASS] LI test passed\n");
}

void testByteLoadExecution() {
    CPU cpu;

    cpu.getRegisters().setRegister(3, 0x1000);

    cpu.getMemory().write8(0x1000, 0x7F);
    cpu.getMemory().write8(0x1001, 0x80);
    cpu.getMemory().write8(0x1002, 0xAB);

    cpu.getMemory().write16(0x0020, makeL(0, 3, 4, 0));
    cpu.step();
    assert(cpu.getRegisters().getRegister(4) == 0x007F);

    cpu.getMemory().write16(0x0022, makeL(1, 3, 5, 0));
    cpu.step();
    assert(cpu.getRegisters().getRegister(5) == 0xFF80);

    cpu.getMemory().write16(0x0024, makeL(1, 3, 6, 4));
    cpu.step();
    assert(cpu.getRegisters().getRegister(6) == 0x0080);

    printf("[PASS] Byte load test passed\n");
}

void testWordLoadExecution() {
    CPU cpu;

    cpu.getRegisters().setRegister(3, 0x2000);

    cpu.getMemory().write16(0x2000, 0xABCD);
    cpu.getMemory().write8(0x2004, 0x34);
    cpu.getMemory().write8(0x2005, 0x12);

    cpu.getMemory().write16(0x0020, makeL(0, 3, 4, 1));
    cpu.step();
    assert(cpu.getRegisters().getRegister(4) == 0xABCD);

    cpu.getMemory().write16(0x0022, makeL(4, 3, 5, 1));
    cpu.step();
    assert(cpu.getRegisters().getRegister(5) == 0x1234);

    printf("[PASS] Word load test passed\n");
}

void testStoreExecution() {
    CPU cpu;

    cpu.getRegisters().setRegister(3, 0x3000);

    cpu.getRegisters().setRegister(4, 0xABCD);
    cpu.getMemory().write16(0x0020, makeS(0, 4, 3, 0));
    cpu.step();

    assert(cpu.getMemory().read8(0x3000) == 0xCD);

    cpu.getRegisters().setRegister(5, 0x1234);
    cpu.getMemory().write16(0x0022, makeS(2, 5, 3, 1));
    cpu.step();

    assert(cpu.getMemory().read16(0x3002) == 0x1234);
    assert(cpu.getMemory().read8(0x3002) == 0x34);
    assert(cpu.getMemory().read8(0x3003) == 0x12);

    printf("[PASS] Store test passed\n");
}

void testBeqBneExecution() {
    CPU cpu;

    cpu.getRegisters().setRegister(3, 5);
    cpu.getRegisters().setRegister(4, 5);

    cpu.getMemory().write16(0x0020, makeB(2, 4, 3, 0));
    cpu.step();
    assert(cpu.getPC() == 0x0026);

    cpu.setPC(0x0100);
    cpu.getRegisters().setRegister(3, 5);
    cpu.getRegisters().setRegister(4, 7);

    cpu.getMemory().write16(0x0100, makeB(2, 4, 3, 0));
    cpu.step();
    assert(cpu.getPC() == 0x0102);

    cpu.setPC(0x0200);
    cpu.getRegisters().setRegister(1, 10);
    cpu.getRegisters().setRegister(2, 20);

    cpu.getMemory().write16(0x0200, makeB(3, 2, 1, 1));
    cpu.step();
    assert(cpu.getPC() == 0x0208);

    printf("[PASS] BEQ/BNE test passed\n");
}

void testRemainingBranchesExecution() {
    CPU cpu;

    cpu.setPC(0x1000);
    cpu.getRegisters().setRegister(3, 0);
    cpu.getMemory().write16(0x1000, makeB(7, 0, 3, 2));
    cpu.step();
    assert(cpu.getPC() == 0x1010);

    cpu.setPC(0x1200);
    cpu.getRegisters().setRegister(4, 9);
    cpu.getMemory().write16(0x1200, makeB(8, 0, 4, 3));
    cpu.step();
    assert(cpu.getPC() == 0x11F2);

    cpu.setPC(0x1400);
    cpu.getRegisters().setRegister(1, 0xFFFF);
    cpu.getRegisters().setRegister(2, 1);
    cpu.getMemory().write16(0x1400, makeB(2, 2, 1, 4));
    cpu.step();
    assert(cpu.getPC() == 0x1406);

    cpu.setPC(0x1800);
    cpu.getRegisters().setRegister(1, 1);
    cpu.getRegisters().setRegister(2, 0xFFFF);
    cpu.getMemory().write16(0x1800, makeB(2, 2, 1, 6));
    cpu.step();
    assert(cpu.getPC() == 0x1806);

    printf("[PASS] Remaining branches test passed\n");
}

void testJumpExecution() {
    CPU cpu;

    cpu.getMemory().write16(0x0020, makeJ(0, 2, 0, 0));
    cpu.step();
    assert(cpu.getPC() == 0x0026);

    cpu.setPC(0x0100);
    cpu.getMemory().write16(0x0100, makeJ(1, 3, 0, 1));
    cpu.step();

    assert(cpu.getRegisters().getRegister(1) == 0x0102);
    assert(cpu.getPC() == 0x0108);

    cpu.setPC(0x0200);
    cpu.getRegisters().setRegister(5, 0x2222);
    cpu.getMemory().write16(0x0200, makeR(0xB, 0, 5, 0));
    cpu.step();

    assert(cpu.getPC() == 0x2222);

    cpu.setPC(0x0240);
    cpu.getRegisters().setRegister(6, 0x3330);
    cpu.getMemory().write16(0x0240, makeR(0xC, 6, 1, 0));
    cpu.step();

    assert(cpu.getRegisters().getRegister(1) == 0x0242);
    assert(cpu.getPC() == 0x3330);

    printf("[PASS] Jump/function-call test passed\n");
}

void testEcallPrintIntExecution() {
    CPU cpu;

    unsigned short word = makeSys(0x000);

    cpu.clearOutput();
    cpu.setPC(0x1000);
    cpu.getRegisters().setRegister(6, 123);
    cpu.getMemory().write16(0x1000, word);
    cpu.step();

    assert(strcmp(cpu.getOutput(), "123") == 0);

    cpu.clearOutput();
    cpu.setPC(0x1100);
    cpu.getRegisters().setRegister(6, 0xFFFF);
    cpu.getMemory().write16(0x1100, word);
    cpu.step();

    assert(strcmp(cpu.getOutput(), "-1") == 0);

    cpu.clearOutput();
    cpu.setPC(0x1200);
    cpu.getRegisters().setRegister(6, 0x8000);
    cpu.getMemory().write16(0x1200, word);
    cpu.step();

    assert(strcmp(cpu.getOutput(), "-32768") == 0);

    printf("\n[PASS] ECALL print_int test passed\n");
}

void testEcallPrintCharExecution() {
    CPU cpu;

    unsigned short word = makeSys(0x001);

    cpu.clearOutput();

    cpu.setPC(0x2000);
    cpu.getRegisters().setRegister(6, 0x0041);
    cpu.getMemory().write16(0x2000, word);
    cpu.step();

    assert(strcmp(cpu.getOutput(), "A") == 0);

    cpu.setPC(0x2002);
    cpu.getRegisters().setRegister(6, 0x000A);
    cpu.getMemory().write16(0x2002, word);
    cpu.step();

    assert(strcmp(cpu.getOutput(), "A\n") == 0);

    cpu.setPC(0x2004);
    cpu.getRegisters().setRegister(6, 0x1242);
    cpu.getMemory().write16(0x2004, word);
    cpu.step();

    assert(strcmp(cpu.getOutput(), "A\nB") == 0);

    printf("\n[PASS] ECALL print_char test passed\n");
}

void testEcallHaltExecution() {
    CPU cpu;

    unsigned short haltWord = makeSys(0x3FF);
    unsigned short addWord = makeR(0x0, 4, 3, 0);

    cpu.setPC(0x3000);
    cpu.getRegisters().setRegister(3, 5);
    cpu.getRegisters().setRegister(4, 7);

    cpu.getMemory().write16(0x3000, haltWord);
    cpu.getMemory().write16(0x3002, addWord);

    assert(cpu.isHalted() == false);

    cpu.step();

    assert(cpu.isHalted() == true);
    assert(cpu.getPC() == 0x3002);

    cpu.step();

    assert(cpu.isHalted() == true);
    assert(cpu.getPC() == 0x3002);
    assert(cpu.getRegisters().getRegister(3) == 5);

    cpu.reset();

    assert(cpu.isHalted() == false);
    assert(cpu.getPC() == 0x0020);

    printf("[PASS] ECALL halt test passed\n");
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
    testRemainingBranchesExecution();
    testJumpExecution();
    testEcallPrintIntExecution();
    testEcallPrintCharExecution();
    testEcallHaltExecution();

    CPU consoleCpu;

    unsigned short printInt = makeSys(0x000);
    unsigned short printChar = makeSys(0x001);

    consoleCpu.clearOutput();

    consoleCpu.setPC(0x5000);
    consoleCpu.getRegisters().setRegister(6, 123);
    consoleCpu.getMemory().write16(0x5000, printInt);
    consoleCpu.step();

    consoleCpu.setPC(0x5002);
    consoleCpu.getRegisters().setRegister(6, 0x000A);
    consoleCpu.getMemory().write16(0x5002, printChar);
    consoleCpu.step();

    consoleCpu.setPC(0x5004);
    consoleCpu.getRegisters().setRegister(6, 0x0041);
    consoleCpu.getMemory().write16(0x5004, printChar);
    consoleCpu.step();

    Gui gui;
    gui.open();

    int frameNumber = 0;

    while (!gui.shouldClose()) {
        frameNumber++;

        gui.draw("All tests passed", consoleCpu.getOutput(), frameNumber);
    }

    gui.close();

    return 0;
}