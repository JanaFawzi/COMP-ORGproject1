#include "raylib.h"
#include "memory.h"
#include "register_file.h"
#include "cpu.h"
#include "program_loader.h"
#include "instruction_decoder.h"
#include "gui.h"
#include "graphics_memory.h"

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

void testGraphicsMemoryManager() {
    Memory memory;
    GraphicsMemory graphics(memory);

    assert(GraphicsMemory::SCREEN_WIDTH == 320);
    assert(GraphicsMemory::SCREEN_HEIGHT == 240);

    assert(GraphicsMemory::TILE_SIZE == 16);
    assert(GraphicsMemory::TILE_COLUMNS == 20);
    assert(GraphicsMemory::TILE_ROWS == 15);
    assert(GraphicsMemory::TILE_COUNT == 16);
    assert(GraphicsMemory::TILE_BYTES == 128);

    assert(GraphicsMemory::TILE_MAP_USED_SIZE == 300);
    assert(GraphicsMemory::TILE_MAP_USED_SIZE == GraphicsMemory::TILE_COLUMNS * GraphicsMemory::TILE_ROWS);
    assert(GraphicsMemory::TILE_DEFINITION_SIZE == 2048);
    assert(GraphicsMemory::TILE_DEFINITION_SIZE == GraphicsMemory::TILE_COUNT * GraphicsMemory::TILE_BYTES);

    assert(GraphicsMemory::GRAPHICS_BASE == 0xF000);
    assert(GraphicsMemory::GRAPHICS_END == 0xFA0F);

    assert(GraphicsMemory::TILE_MAP_BASE == 0xF000);
    assert(GraphicsMemory::TILE_MAP_USED_END == 0xF12B);
    assert(GraphicsMemory::TILE_MAP_END == 0xF1FF);
    assert(GraphicsMemory::TILE_MAP_REGION_SIZE == 512);

    assert(GraphicsMemory::TILE_DEFINITION_BASE == 0xF200);
    assert(GraphicsMemory::TILE_DEFINITION_END == 0xF9FF);

    assert(GraphicsMemory::PALETTE_BASE == 0xFA00);
    assert(GraphicsMemory::PALETTE_SIZE == 16);
    assert(GraphicsMemory::PALETTE_END == 0xFA0F);

    assert(GraphicsMemory::RESERVED_MMIO_BASE == 0xFA10);
    assert(GraphicsMemory::RESERVED_MMIO_END == 0xFFFF);

    assert(GraphicsMemory::PROJECT_STACK_RESET == 0xEFFE);
    assert(GraphicsMemory::PROJECT_STACK_RESET < GraphicsMemory::GRAPHICS_BASE);

    assert(GraphicsMemory::TILE_MAP_END + 1 == GraphicsMemory::TILE_DEFINITION_BASE);
    assert(GraphicsMemory::TILE_DEFINITION_END + 1 == GraphicsMemory::PALETTE_BASE);
    assert(GraphicsMemory::PALETTE_END + 1 == GraphicsMemory::RESERVED_MMIO_BASE);

    assert(GraphicsMemory::getTileMapAddress(0, 0) == 0xF000);
    assert(GraphicsMemory::getTileMapAddress(19, 0) == 0xF013);
    assert(GraphicsMemory::getTileMapAddress(0, 1) == 0xF014);
    assert(GraphicsMemory::getTileMapAddress(19, 14) == 0xF12B);

    assert(GraphicsMemory::getTileMapAddress(-1, 0) == GraphicsMemory::INVALID_ADDRESS);
    assert(GraphicsMemory::getTileMapAddress(20, 0) == GraphicsMemory::INVALID_ADDRESS);
    assert(GraphicsMemory::getTileMapAddress(0, -1) == GraphicsMemory::INVALID_ADDRESS);
    assert(GraphicsMemory::getTileMapAddress(0, 15) == GraphicsMemory::INVALID_ADDRESS);

    assert(GraphicsMemory::isTileMapUsedAddress(0xEFFF) == false);
    assert(GraphicsMemory::isTileMapUsedAddress(0xF000) == true);
    assert(GraphicsMemory::isTileMapUsedAddress(0xF12B) == true);
    assert(GraphicsMemory::isTileMapUsedAddress(0xF12C) == false);

    assert(GraphicsMemory::isTileMapRegionAddress(0xF000) == true);
    assert(GraphicsMemory::isTileMapRegionAddress(0xF12C) == true);
    assert(GraphicsMemory::isTileMapRegionAddress(0xF1FF) == true);
    assert(GraphicsMemory::isTileMapRegionAddress(0xF200) == false);

    assert(GraphicsMemory::getTileDefinitionBase(0) == 0xF200);
    assert(GraphicsMemory::getTileDefinitionBase(1) == 0xF280);
    assert(GraphicsMemory::getTileDefinitionBase(15) == 0xF980);
    assert(GraphicsMemory::getTileDefinitionBase(16) == GraphicsMemory::INVALID_ADDRESS);
    assert(GraphicsMemory::getTileDefinitionBase(-1) == GraphicsMemory::INVALID_ADDRESS);

    assert(GraphicsMemory::getTilePixelByteAddress(0, 0, 0) == 0xF200);
    assert(GraphicsMemory::getTilePixelByteAddress(0, 1, 0) == 0xF200);
    assert(GraphicsMemory::getTilePixelByteAddress(0, 2, 0) == 0xF201);
    assert(GraphicsMemory::getTilePixelByteAddress(0, 15, 0) == 0xF207);
    assert(GraphicsMemory::getTilePixelByteAddress(0, 0, 1) == 0xF208);
    assert(GraphicsMemory::getTilePixelByteAddress(15, 15, 15) == 0xF9FF);

    assert(GraphicsMemory::getTilePixelByteAddress(16, 0, 0) == GraphicsMemory::INVALID_ADDRESS);
    assert(GraphicsMemory::getTilePixelByteAddress(0, -1, 0) == GraphicsMemory::INVALID_ADDRESS);
    assert(GraphicsMemory::getTilePixelByteAddress(0, 16, 0) == GraphicsMemory::INVALID_ADDRESS);
    assert(GraphicsMemory::getTilePixelByteAddress(0, 0, -1) == GraphicsMemory::INVALID_ADDRESS);
    assert(GraphicsMemory::getTilePixelByteAddress(0, 0, 16) == GraphicsMemory::INVALID_ADDRESS);

    assert(GraphicsMemory::isTileDefinitionAddress(0xF1FF) == false);
    assert(GraphicsMemory::isTileDefinitionAddress(0xF200) == true);
    assert(GraphicsMemory::isTileDefinitionAddress(0xF9FF) == true);
    assert(GraphicsMemory::isTileDefinitionAddress(0xFA00) == false);

    assert(GraphicsMemory::getPaletteAddress(0) == 0xFA00);
    assert(GraphicsMemory::getPaletteAddress(15) == 0xFA0F);
    assert(GraphicsMemory::getPaletteAddress(16) == GraphicsMemory::INVALID_ADDRESS);
    assert(GraphicsMemory::getPaletteAddress(-1) == GraphicsMemory::INVALID_ADDRESS);

    assert(GraphicsMemory::isPaletteAddress(0xF9FF) == false);
    assert(GraphicsMemory::isPaletteAddress(0xFA00) == true);
    assert(GraphicsMemory::isPaletteAddress(0xFA0F) == true);
    assert(GraphicsMemory::isPaletteAddress(0xFA10) == false);

    assert(GraphicsMemory::isGraphicsAddress(0xEFFF) == false);
    assert(GraphicsMemory::isGraphicsAddress(0xF000) == true);
    assert(GraphicsMemory::isGraphicsAddress(0xF1FF) == true);
    assert(GraphicsMemory::isGraphicsAddress(0xF200) == true);
    assert(GraphicsMemory::isGraphicsAddress(0xF9FF) == true);
    assert(GraphicsMemory::isGraphicsAddress(0xFA00) == true);
    assert(GraphicsMemory::isGraphicsAddress(0xFA0F) == true);
    assert(GraphicsMemory::isGraphicsAddress(0xFA10) == false);

    assert(GraphicsMemory::isReservedMmioAddress(0xFA0F) == false);
    assert(GraphicsMemory::isReservedMmioAddress(0xFA10) == true);
    assert(GraphicsMemory::isReservedMmioAddress(0xFFFF) == true);

    assert(graphics.writeTileIndex(19, 14, 5) == true);
    assert(memory.read8(0xF12B) == 5);
    assert(graphics.readTileIndex(19, 14) == 5);

    assert(graphics.writeTileIndex(0, 0, 16) == false);
    assert(memory.read8(0xF000) == 0);

    assert(graphics.writeTileIndex(20, 0, 3) == false);
    assert(graphics.writeTileIndex(0, 15, 3) == false);
    assert(graphics.readTileIndex(20, 0) == 0);
    assert(graphics.readTileIndex(0, 15) == 0);

    assert(graphics.writeTileDefinitionByte(15, 127, 0xAB) == true);
    assert(memory.read8(0xF9FF) == 0xAB);
    assert(graphics.readTileDefinitionByte(15, 127) == 0xAB);

    assert(graphics.writeTileDefinitionByte(15, 128, 0xCD) == false);
    assert(graphics.writeTileDefinitionByte(16, 0, 0xCD) == false);
    assert(graphics.readTileDefinitionByte(16, 0) == 0);

    assert(graphics.writePaletteColor(15, 0xE3) == true);
    assert(memory.read8(0xFA0F) == 0xE3);
    assert(graphics.readPaletteColor(15) == 0xE3);

    assert(graphics.writePaletteColor(16, 0xFF) == false);
    assert(graphics.readPaletteColor(16) == 0);

    printf("[PASS] Graphics memory manager test passed\n");
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

void loadGuiDemoProgram(CPU& cpu) {
    cpu.reset();
    cpu.clearOutput();

    cpu.getMemory().write16(0x0020, makeI(5, 6, 0));
    cpu.getMemory().write16(0x0022, makeSys(0x000));

    cpu.getMemory().write16(0x0024, makeI(0x0A, 6, 7));
    cpu.getMemory().write16(0x0026, makeSys(0x001));

    cpu.getMemory().write16(0x0028, makeI(6, 6, 7));
    cpu.getMemory().write16(0x002A, makeSys(0x000));

    cpu.getMemory().write16(0x002C, makeSys(0x3FF));
}

void handleGuiAction(CPU& cpu, GuiAction action, bool& running, int& runDelay) {
    if (action == GUI_ACTION_RUN_PAUSE) {
        if (!cpu.isHalted()) {
            running = !running;
        }
    }

    if (action == GUI_ACTION_STEP) {
        if (!running && !cpu.isHalted()) {
            cpu.step();
        }
    }

    if (action == GUI_ACTION_RESET) {
        loadGuiDemoProgram(cpu);
        running = false;
        runDelay = 0;
    }

    if (cpu.isHalted()) {
        running = false;
    }
}

void testGuiStepExecutesOneInstruction() {
    CPU cpu;

    bool running = false;
    int runDelay = 0;

    cpu.reset();

    cpu.getMemory().write16(0x0020, makeI(5, 6, 7));
    cpu.getMemory().write16(0x0022, makeI(6, 7, 7));
    cpu.getMemory().write16(0x0024, makeSys(0x3FF));

    assert(cpu.getPC() == 0x0020);
    assert(cpu.getRegisters().getRegister(6) == 0x0000);
    assert(cpu.getRegisters().getRegister(7) == 0x0000);

    handleGuiAction(cpu, GUI_ACTION_STEP, running, runDelay);

    assert(cpu.getPC() == 0x0022);
    assert(cpu.getRegisters().getRegister(6) == 5);
    assert(cpu.getRegisters().getRegister(7) == 0x0000);

    handleGuiAction(cpu, GUI_ACTION_STEP, running, runDelay);

    assert(cpu.getPC() == 0x0024);
    assert(cpu.getRegisters().getRegister(6) == 5);
    assert(cpu.getRegisters().getRegister(7) == 6);

    printf("[PASS] GUI step executes one instruction test passed\n");
}

void testMemoryViewerMatchesRam() {
    CPU cpu;
    Gui gui;

    char line[80];

    cpu.getMemory().write8(0x4000, 0x12);
    cpu.getMemory().write8(0x4001, 0x34);
    cpu.getMemory().write8(0x4002, 0xAB);
    cpu.getMemory().write8(0x4003, 0xCD);
    cpu.getMemory().write8(0x4004, 0x00);
    cpu.getMemory().write8(0x4005, 0xFF);
    cpu.getMemory().write8(0x4006, 0x55);
    cpu.getMemory().write8(0x4007, 0xAA);

    gui.formatMemoryLine(cpu, 0x4000, line);

    assert(strcmp(line, "0x4000: 12 34 AB CD 00 FF 55 AA") == 0);

    cpu.getMemory().write8(0x4003, 0x99);

    gui.formatMemoryLine(cpu, 0x4000, line);

    assert(strcmp(line, "0x4000: 12 34 AB 99 00 FF 55 AA") == 0);

    printf("[PASS] Memory viewer test passed\n");
}

void writeWordToBin(FILE* file, unsigned short word) {
    fputc(word & 0x00FF, file);
    fputc((word >> 8) & 0x00FF, file);
}

void createBinProgram(const char filename[], unsigned short program[], int wordCount) {
    FILE* file = fopen(filename, "wb");

    assert(file != 0);

    for (int i = 0; i < wordCount; i++) {
        writeWordToBin(file, program[i]);
    }

    fclose(file);
}

void runCpuUntilHalt(CPU& cpu, int maxSteps) {
    int steps = 0;

    while (!cpu.isHalted() && steps < maxSteps) {
        cpu.step();
        steps++;
    }

    assert(cpu.isHalted() == true);
}

void testFinalBinPrograms() {
    CPU cpu;

    const char arithmeticBin[] = "final_arithmetic.bin";

    unsigned short arithmeticProgram[] = {
        makeI(7, 6, 7),
        makeI(5, 6, 0),
        makeSys(0x000),
        makeI(0x0A, 6, 7),
        makeSys(0x001),
        makeI(0x7F, 6, 7),
        makeSys(0x000),
        makeSys(0x3FF)
    };

    createBinProgram(arithmeticBin, arithmeticProgram, 8);

    cpu.reset();
    cpu.clearOutput();

    assert(ProgramLoader::loadBin(cpu.getMemory(), arithmeticBin) == 16);

    runCpuUntilHalt(cpu, 50);

    assert(strcmp(cpu.getOutput(), "12\n-1") == 0);
    assert(cpu.getPC() == 0x0030);

    remove(arithmeticBin);

    const char memoryBin[] = "final_memory.bin";

    unsigned short memoryProgram[] = {
        makeI(42, 3, 7),
        makeS(0xE, 3, 2, 1),
        makeL(0xE, 2, 6, 1),
        makeSys(0x000),
        makeI(0x0A, 6, 7),
        makeSys(0x001),
        makeI(0x7F, 4, 7),
        makeS(0xF, 4, 2, 0),
        makeL(0xF, 2, 6, 0),
        makeSys(0x000),
        makeSys(0x3FF)
    };

    createBinProgram(memoryBin, memoryProgram, 11);

    cpu.reset();
    cpu.clearOutput();

    assert(ProgramLoader::loadBin(cpu.getMemory(), memoryBin) == 22);

    runCpuUntilHalt(cpu, 50);

    assert(strcmp(cpu.getOutput(), "42\n-1") == 0);
    assert(cpu.getMemory().read8(0xEFFC) == 0x2A);
    assert(cpu.getMemory().read8(0xEFFD) == 0xFF);

    remove(memoryBin);

    const char branchBin[] = "final_branch.bin";

    unsigned short branchProgram[] = {
        makeI(0, 6, 7),
        makeB(1, 0, 6, 2),
        makeI(9, 6, 7),
        makeI(1, 6, 7),
        makeSys(0x000),
        makeSys(0x3FF)
    };

    createBinProgram(branchBin, branchProgram, 6);

    cpu.reset();
    cpu.clearOutput();

    assert(ProgramLoader::loadBin(cpu.getMemory(), branchBin) == 12);

    runCpuUntilHalt(cpu, 50);

    assert(strcmp(cpu.getOutput(), "1") == 0);
    assert(cpu.getPC() == 0x002C);

    remove(branchBin);

    const char jumpBin[] = "final_jump.bin";

    unsigned short jumpProgram[] = {
        makeJ(1, 3, 0, 1),
        makeSys(0x000),
        makeSys(0x3FF),
        makeR(0x0, 0, 0, 0),
        makeI(8, 6, 7),
        makeR(0xB, 0, 1, 0)
    };

    createBinProgram(jumpBin, jumpProgram, 6);

    cpu.reset();
    cpu.clearOutput();

    assert(ProgramLoader::loadBin(cpu.getMemory(), jumpBin) == 12);

    runCpuUntilHalt(cpu, 50);

    assert(strcmp(cpu.getOutput(), "8") == 0);
    assert(cpu.getPC() == 0x0026);

    remove(jumpBin);

    printf("\n[PASS] Final integration .bin programs test passed\n");
}

int main() {
    testMemory();
    testGraphicsMemoryManager();
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
    testGuiStepExecutesOneInstruction();
    testMemoryViewerMatchesRam();
    testFinalBinPrograms();

    CPU guiCpu;
    loadGuiDemoProgram(guiCpu);

    Gui gui;
    gui.open();

    bool running = false;
    int frameNumber = 0;
    int runDelay = 0;

    while (!gui.shouldClose()) {
        frameNumber++;

        GuiAction action = gui.draw(
            "All tests passed",
            guiCpu.getOutput(),
            frameNumber,
            running,
            guiCpu
        );

        handleGuiAction(guiCpu, action, running, runDelay);

        if (running && !guiCpu.isHalted()) {
            runDelay++;

            if (runDelay >= 30) {
                guiCpu.step();
                runDelay = 0;
            }
        }

        if (guiCpu.isHalted()) {
            running = false;
        }
    }

    gui.close();

    return 0;
}