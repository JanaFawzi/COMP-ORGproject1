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
    assert(GraphicsMemory::SCREEN_PIXEL_COUNT == 76800);

    assert(GraphicsMemory::TILE_SIZE == 16);
    assert(GraphicsMemory::TILE_COLUMNS == 20);
    assert(GraphicsMemory::TILE_ROWS == 15);
    assert(GraphicsMemory::TILE_COUNT == 16);
    assert(GraphicsMemory::TILE_BYTES == 128);
    assert(GraphicsMemory::TILE_PIXEL_COUNT == 256);

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

void testGraphicsMemoryProtection() {
    Memory memory;
    GraphicsMemory graphics(memory);

    unsigned char byteValue = 0;
    unsigned short wordValue = 0;

    assert(GraphicsMemory::getGraphicsRegion(0xEFFF) == GraphicsMemory::REGION_NONE);
    assert(GraphicsMemory::getGraphicsRegion(0xF000) == GraphicsMemory::REGION_TILE_MAP_USED);
    assert(GraphicsMemory::getGraphicsRegion(0xF12B) == GraphicsMemory::REGION_TILE_MAP_USED);
    assert(GraphicsMemory::getGraphicsRegion(0xF12C) == GraphicsMemory::REGION_TILE_MAP_PADDING);
    assert(GraphicsMemory::getGraphicsRegion(0xF1FF) == GraphicsMemory::REGION_TILE_MAP_PADDING);
    assert(GraphicsMemory::getGraphicsRegion(0xF200) == GraphicsMemory::REGION_TILE_DEFINITION);
    assert(GraphicsMemory::getGraphicsRegion(0xF9FF) == GraphicsMemory::REGION_TILE_DEFINITION);
    assert(GraphicsMemory::getGraphicsRegion(0xFA00) == GraphicsMemory::REGION_PALETTE);
    assert(GraphicsMemory::getGraphicsRegion(0xFA0F) == GraphicsMemory::REGION_PALETTE);
    assert(GraphicsMemory::getGraphicsRegion(0xFA10) == GraphicsMemory::REGION_RESERVED_MMIO);
    assert(GraphicsMemory::getGraphicsRegion(0xFFFF) == GraphicsMemory::REGION_RESERVED_MMIO);

    assert(GraphicsMemory::isVramAddress(0xEFFF) == false);
    assert(GraphicsMemory::isVramAddress(0xF000) == true);
    assert(GraphicsMemory::isVramAddress(0xFA0F) == true);
    assert(GraphicsMemory::isVramAddress(0xFA10) == false);

    assert(GraphicsMemory::isSafeVramRange(0xF000, 1) == true);
    assert(GraphicsMemory::isSafeVramRange(0xFA0F, 1) == true);
    assert(GraphicsMemory::isSafeVramRange(0xFA0E, 2) == true);
    assert(GraphicsMemory::isSafeVramRange(0xFA0F, 2) == false);
    assert(GraphicsMemory::isSafeVramRange(0xEFFF, 1) == false);
    assert(GraphicsMemory::isSafeVramRange(0xFA10, 1) == false);
    assert(GraphicsMemory::isSafeVramRange(0xF000, 0) == false);
    assert(GraphicsMemory::isSafeVramRange(0xF000, -1) == false);

    memory.write8(0xEFFF, 0x11);
    memory.write8(0xFA10, 0x22);
    memory.write8(0xFFFF, 0x33);

    assert(graphics.writeVram8(0xF000, 0xA1) == true);
    assert(graphics.writeVram8(0xF12B, 0xA2) == true);
    assert(graphics.writeVram8(0xF12C, 0xA3) == true);
    assert(graphics.writeVram8(0xF1FF, 0xA4) == true);
    assert(graphics.writeVram8(0xF200, 0xA5) == true);
    assert(graphics.writeVram8(0xF9FF, 0xA6) == true);
    assert(graphics.writeVram8(0xFA00, 0xA7) == true);
    assert(graphics.writeVram8(0xFA0F, 0xA8) == true);

    assert(memory.read8(0xF000) == 0xA1);
    assert(memory.read8(0xF12B) == 0xA2);
    assert(memory.read8(0xF12C) == 0xA3);
    assert(memory.read8(0xF1FF) == 0xA4);
    assert(memory.read8(0xF200) == 0xA5);
    assert(memory.read8(0xF9FF) == 0xA6);
    assert(memory.read8(0xFA00) == 0xA7);
    assert(memory.read8(0xFA0F) == 0xA8);

    assert(graphics.readVram8(0xF000, byteValue) == true);
    assert(byteValue == 0xA1);

    assert(graphics.readVram8(0xF12B, byteValue) == true);
    assert(byteValue == 0xA2);

    assert(graphics.readVram8(0xF12C, byteValue) == true);
    assert(byteValue == 0xA3);

    assert(graphics.readVram8(0xF1FF, byteValue) == true);
    assert(byteValue == 0xA4);

    assert(graphics.readVram8(0xF200, byteValue) == true);
    assert(byteValue == 0xA5);

    assert(graphics.readVram8(0xF9FF, byteValue) == true);
    assert(byteValue == 0xA6);

    assert(graphics.readVram8(0xFA00, byteValue) == true);
    assert(byteValue == 0xA7);

    assert(graphics.readVram8(0xFA0F, byteValue) == true);
    assert(byteValue == 0xA8);

    assert(graphics.writeVram8(0xEFFF, 0xFF) == false);
    assert(graphics.writeVram8(0xFA10, 0xFF) == false);
    assert(graphics.writeVram8(0xFFFF, 0xFF) == false);

    assert(memory.read8(0xEFFF) == 0x11);
    assert(memory.read8(0xFA10) == 0x22);
    assert(memory.read8(0xFFFF) == 0x33);

    byteValue = 0x77;
    assert(graphics.readVram8(0xEFFF, byteValue) == false);
    assert(byteValue == 0x77);

    byteValue = 0x88;
    assert(graphics.readVram8(0xFA10, byteValue) == false);
    assert(byteValue == 0x88);

    assert(graphics.writeVram16(0xF000, 0x1234) == true);
    assert(memory.read8(0xF000) == 0x34);
    assert(memory.read8(0xF001) == 0x12);

    wordValue = 0;
    assert(graphics.readVram16(0xF000, wordValue) == true);
    assert(wordValue == 0x1234);

    assert(graphics.writeVram16(0xFA0E, 0xBEEF) == true);
    assert(memory.read8(0xFA0E) == 0xEF);
    assert(memory.read8(0xFA0F) == 0xBE);

    wordValue = 0;
    assert(graphics.readVram16(0xFA0E, wordValue) == true);
    assert(wordValue == 0xBEEF);

    memory.write8(0xFA0F, 0x55);
    memory.write8(0xFA10, 0x66);

    assert(graphics.writeVram16(0xFA0F, 0xABCD) == false);
    assert(memory.read8(0xFA0F) == 0x55);
    assert(memory.read8(0xFA10) == 0x66);

    assert(graphics.writeVram16(0xEFFE, 0xABCD) == false);
    assert(memory.read8(0xEFFF) == 0x11);

    assert(graphics.writeVram16(0xFA10, 0xABCD) == false);
    assert(memory.read8(0xFA10) == 0x66);

    wordValue = 0x9999;
    assert(graphics.readVram16(0xFA0F, wordValue) == false);
    assert(wordValue == 0x9999);

    wordValue = 0xAAAA;
    assert(graphics.readVram16(0xEFFE, wordValue) == false);
    assert(wordValue == 0xAAAA);

    assert(graphics.writeVram16(0xF001, 0x5555) == false);

    wordValue = 0xBBBB;
    assert(graphics.readVram16(0xF001, wordValue) == false);
    assert(wordValue == 0xBBBB);

    printf("[PASS] Graphics memory protection test passed\n");
}

void testTileMapAccess() {
    Memory memory;
    GraphicsMemory graphics(memory);

    unsigned char value = 0;

    assert(GraphicsMemory::getTileMapOffset(0, 0) == 0);
    assert(GraphicsMemory::getTileMapOffset(19, 0) == 19);
    assert(GraphicsMemory::getTileMapOffset(0, 1) == 20);
    assert(GraphicsMemory::getTileMapOffset(19, 14) == 299);

    assert(GraphicsMemory::getTileMapOffset(-1, 0) == -1);
    assert(GraphicsMemory::getTileMapOffset(20, 0) == -1);
    assert(GraphicsMemory::getTileMapOffset(0, -1) == -1);
    assert(GraphicsMemory::getTileMapOffset(0, 15) == -1);

    assert(GraphicsMemory::getTileMapAddressByOffset(0) == 0xF000);
    assert(GraphicsMemory::getTileMapAddressByOffset(299) == 0xF12B);
    assert(GraphicsMemory::getTileMapAddressByOffset(-1) == GraphicsMemory::INVALID_ADDRESS);
    assert(GraphicsMemory::getTileMapAddressByOffset(300) == GraphicsMemory::INVALID_ADDRESS);

    assert(graphics.writeTileIndex(0, 0, 1) == true);
    assert(graphics.writeTileIndex(19, 0, 2) == true);
    assert(graphics.writeTileIndex(0, 14, 3) == true);
    assert(graphics.writeTileIndex(19, 14, 15) == true);

    assert(memory.read8(0xF000) == 1);
    assert(memory.read8(0xF013) == 2);
    assert(memory.read8(0xF118) == 3);
    assert(memory.read8(0xF12B) == 15);

    assert(graphics.readTileIndex(0, 0) == 1);
    assert(graphics.readTileIndex(19, 0) == 2);
    assert(graphics.readTileIndex(0, 14) == 3);
    assert(graphics.readTileIndex(19, 14) == 15);

    value = 0xAA;
    assert(graphics.readTileIndexChecked(19, 14, value) == true);
    assert(value == 15);

    value = 0xAA;
    assert(graphics.readTileIndexChecked(20, 14, value) == false);
    assert(value == 0xAA);

    assert(graphics.writeTileIndexByOffset(0, 4) == true);
    assert(graphics.writeTileIndexByOffset(299, 5) == true);

    assert(memory.read8(0xF000) == 4);
    assert(memory.read8(0xF12B) == 5);

    value = 0;
    assert(graphics.readTileIndexByOffset(0, value) == true);
    assert(value == 4);

    value = 0;
    assert(graphics.readTileIndexByOffset(299, value) == true);
    assert(value == 5);

    value = 0x55;
    assert(graphics.readTileIndexByOffset(-1, value) == false);
    assert(value == 0x55);

    value = 0x66;
    assert(graphics.readTileIndexByOffset(300, value) == false);
    assert(value == 0x66);

    memory.write8(0xF000, 0x09);
    assert(graphics.writeTileIndex(0, 0, 16) == false);
    assert(memory.read8(0xF000) == 0x09);

    assert(graphics.writeTileIndex(-1, 0, 1) == false);
    assert(graphics.writeTileIndex(20, 0, 1) == false);
    assert(graphics.writeTileIndex(0, -1, 1) == false);
    assert(graphics.writeTileIndex(0, 15, 1) == false);

    memory.write8(0xF12C, 0xCC);

    assert(graphics.fillTileMap(7) == true);

    for (int row = 0; row < GraphicsMemory::TILE_ROWS; row++) {
        for (int col = 0; col < GraphicsMemory::TILE_COLUMNS; col++) {
            assert(graphics.readTileIndex(col, row) == 7);
        }
    }

    assert(memory.read8(0xF12C) == 0xCC);

    assert(graphics.fillTileMap(16) == false);
    assert(memory.read8(0xF000) == 7);

    assert(graphics.clearTileMap() == true);

    for (int row = 0; row < GraphicsMemory::TILE_ROWS; row++) {
        for (int col = 0; col < GraphicsMemory::TILE_COLUMNS; col++) {
            assert(graphics.readTileIndex(col, row) == 0);
        }
    }

    assert(memory.read8(0xF12C) == 0xCC);

    printf("[PASS] Tile map access test passed\n");
}

void testTileDefinitionAccess() {
    Memory memory;
    GraphicsMemory graphics(memory);

    unsigned char value = 0;
    unsigned char definition[GraphicsMemory::TILE_BYTES];
    unsigned char readBack[GraphicsMemory::TILE_BYTES];

    assert(GraphicsMemory::getTilePixelNumber(0, 0) == 0);
    assert(GraphicsMemory::getTilePixelNumber(1, 0) == 1);
    assert(GraphicsMemory::getTilePixelNumber(15, 0) == 15);
    assert(GraphicsMemory::getTilePixelNumber(0, 1) == 16);
    assert(GraphicsMemory::getTilePixelNumber(15, 15) == 255);

    assert(GraphicsMemory::getTilePixelByteOffset(0, 0) == 0);
    assert(GraphicsMemory::getTilePixelByteOffset(1, 0) == 0);
    assert(GraphicsMemory::getTilePixelByteOffset(2, 0) == 1);
    assert(GraphicsMemory::getTilePixelByteOffset(15, 0) == 7);
    assert(GraphicsMemory::getTilePixelByteOffset(0, 1) == 8);
    assert(GraphicsMemory::getTilePixelByteOffset(15, 15) == 127);

    assert(GraphicsMemory::getTilePixelByteOffset(-1, 0) == -1);
    assert(GraphicsMemory::getTilePixelByteOffset(16, 0) == -1);
    assert(GraphicsMemory::getTilePixelByteOffset(0, -1) == -1);
    assert(GraphicsMemory::getTilePixelByteOffset(0, 16) == -1);

    assert(graphics.writeTileDefinitionByte(0, 0, 0x12) == true);
    assert(graphics.writeTileDefinitionByte(1, 0, 0x34) == true);
    assert(graphics.writeTileDefinitionByte(15, 127, 0xAB) == true);

    assert(memory.read8(0xF200) == 0x12);
    assert(memory.read8(0xF280) == 0x34);
    assert(memory.read8(0xF9FF) == 0xAB);

    assert(graphics.readTileDefinitionByte(0, 0) == 0x12);
    assert(graphics.readTileDefinitionByte(1, 0) == 0x34);
    assert(graphics.readTileDefinitionByte(15, 127) == 0xAB);

    value = 0x44;
    assert(graphics.readTileDefinitionByteChecked(15, 127, value) == true);
    assert(value == 0xAB);

    value = 0x55;
    assert(graphics.readTileDefinitionByteChecked(16, 0, value) == false);
    assert(value == 0x55);

    value = 0x66;
    assert(graphics.readTileDefinitionByteChecked(0, 128, value) == false);
    assert(value == 0x66);

    memory.write8(0xF200, 0x99);
    assert(graphics.writeTileDefinitionByte(0, 128, 0xFF) == false);
    assert(graphics.writeTileDefinitionByte(16, 0, 0xFF) == false);
    assert(memory.read8(0xF200) == 0x99);

    assert(graphics.clearTileDefinition(3, 7) == true);

    for (int i = 0; i < GraphicsMemory::TILE_BYTES; i++) {
        assert(memory.read8(GraphicsMemory::getTileDefinitionBase(3) + i) == 0x77);
    }

    assert(graphics.clearTileDefinition(3, 16) == false);
    assert(graphics.clearTileDefinition(16, 1) == false);

    for (int i = 0; i < GraphicsMemory::TILE_BYTES; i++) {
        definition[i] = (unsigned char)(i ^ 0x5A);
        readBack[i] = 0;
    }

    assert(graphics.writeTileDefinition(4, definition, GraphicsMemory::TILE_BYTES) == true);
    assert(graphics.readTileDefinition(4, readBack, GraphicsMemory::TILE_BYTES) == true);

    for (int i = 0; i < GraphicsMemory::TILE_BYTES; i++) {
        assert(readBack[i] == definition[i]);
        assert(memory.read8(GraphicsMemory::getTileDefinitionBase(4) + i) == definition[i]);
    }

    assert(graphics.writeTileDefinition(4, definition, GraphicsMemory::TILE_BYTES - 1) == false);
    assert(graphics.writeTileDefinition(4, 0, GraphicsMemory::TILE_BYTES) == false);
    assert(graphics.writeTileDefinition(16, definition, GraphicsMemory::TILE_BYTES) == false);

    assert(graphics.readTileDefinition(4, readBack, GraphicsMemory::TILE_BYTES - 1) == false);
    assert(graphics.readTileDefinition(4, 0, GraphicsMemory::TILE_BYTES) == false);
    assert(graphics.readTileDefinition(16, readBack, GraphicsMemory::TILE_BYTES) == false);

    memory.write8(GraphicsMemory::getTilePixelByteAddress(2, 0, 0), 0x00);

    assert(graphics.writeTilePixel(2, 0, 0, 0x0A) == true);
    assert(memory.read8(GraphicsMemory::getTilePixelByteAddress(2, 0, 0)) == 0x0A);

    assert(graphics.writeTilePixel(2, 1, 0, 0x0B) == true);
    assert(memory.read8(GraphicsMemory::getTilePixelByteAddress(2, 1, 0)) == 0xBA);

    value = 0;
    assert(graphics.readTilePixel(2, 0, 0, value) == true);
    assert(value == 0x0A);

    value = 0;
    assert(graphics.readTilePixel(2, 1, 0, value) == true);
    assert(value == 0x0B);

    assert(graphics.writeTilePixel(2, 0, 0, 0x05) == true);
    assert(memory.read8(GraphicsMemory::getTilePixelByteAddress(2, 0, 0)) == 0xB5);

    assert(graphics.writeTilePixel(2, 1, 0, 0x0C) == true);
    assert(memory.read8(GraphicsMemory::getTilePixelByteAddress(2, 1, 0)) == 0xC5);

    value = 0;
    assert(graphics.readTilePixel(2, 0, 0, value) == true);
    assert(value == 0x05);

    value = 0;
    assert(graphics.readTilePixel(2, 1, 0, value) == true);
    assert(value == 0x0C);

    assert(graphics.writeTilePixel(2, 0, 0, 16) == false);
    assert(memory.read8(GraphicsMemory::getTilePixelByteAddress(2, 0, 0)) == 0xC5);

    assert(graphics.writeTilePixel(16, 0, 0, 1) == false);
    assert(graphics.writeTilePixel(2, 16, 0, 1) == false);
    assert(graphics.writeTilePixel(2, 0, 16, 1) == false);

    value = 0xEE;
    assert(graphics.readTilePixel(16, 0, 0, value) == false);
    assert(value == 0xEE);

    value = 0xDD;
    assert(graphics.readTilePixel(2, 16, 0, value) == false);
    assert(value == 0xDD);

    value = 0xCC;
    assert(graphics.readTilePixel(2, 0, 16, value) == false);
    assert(value == 0xCC);

    printf("[PASS] Tile definition access test passed\n");
}

void testPaletteMemory() {
    Memory memory;
    GraphicsMemory graphics(memory);

    unsigned char raw = 0;
    unsigned char red8 = 0;
    unsigned char green8 = 0;
    unsigned char blue8 = 0;

    assert(GraphicsMemory::isValidRgb3(0) == true);
    assert(GraphicsMemory::isValidRgb3(7) == true);
    assert(GraphicsMemory::isValidRgb3(-1) == false);
    assert(GraphicsMemory::isValidRgb3(8) == false);

    assert(GraphicsMemory::isValidRgb2(0) == true);
    assert(GraphicsMemory::isValidRgb2(3) == true);
    assert(GraphicsMemory::isValidRgb2(-1) == false);
    assert(GraphicsMemory::isValidRgb2(4) == false);

    assert(GraphicsMemory::makeRgb332(0, 0, 0) == 0x00);
    assert(GraphicsMemory::makeRgb332(7, 7, 3) == 0xFF);
    assert(GraphicsMemory::makeRgb332(5, 2, 1) == 0xA9);

    assert(GraphicsMemory::makeRgb332(8, 0, 0) == 0x00);
    assert(GraphicsMemory::makeRgb332(0, 8, 0) == 0x00);
    assert(GraphicsMemory::makeRgb332(0, 0, 4) == 0x00);

    assert(GraphicsMemory::getRgb332Red3(0xA9) == 5);
    assert(GraphicsMemory::getRgb332Green3(0xA9) == 2);
    assert(GraphicsMemory::getRgb332Blue2(0xA9) == 1);

    assert(GraphicsMemory::getRgb332Red3(0xE3) == 7);
    assert(GraphicsMemory::getRgb332Green3(0xE3) == 0);
    assert(GraphicsMemory::getRgb332Blue2(0xE3) == 3);

    assert(GraphicsMemory::expandRgb3To8(0) == 0x00);
    assert(GraphicsMemory::expandRgb3To8(1) == 0x24);
    assert(GraphicsMemory::expandRgb3To8(2) == 0x49);
    assert(GraphicsMemory::expandRgb3To8(5) == 0xB6);
    assert(GraphicsMemory::expandRgb3To8(7) == 0xFF);

    assert(GraphicsMemory::expandRgb2To8(0) == 0x00);
    assert(GraphicsMemory::expandRgb2To8(1) == 0x55);
    assert(GraphicsMemory::expandRgb2To8(2) == 0xAA);
    assert(GraphicsMemory::expandRgb2To8(3) == 0xFF);

    GraphicsMemory::expandRgb332ToRgb888(0x00, red8, green8, blue8);
    assert(red8 == 0x00);
    assert(green8 == 0x00);
    assert(blue8 == 0x00);

    GraphicsMemory::expandRgb332ToRgb888(0xFF, red8, green8, blue8);
    assert(red8 == 0xFF);
    assert(green8 == 0xFF);
    assert(blue8 == 0xFF);

    GraphicsMemory::expandRgb332ToRgb888(0xA9, red8, green8, blue8);
    assert(red8 == 0xB6);
    assert(green8 == 0x49);
    assert(blue8 == 0x55);

    assert(graphics.writePaletteColor(0, 0x00) == true);
    assert(graphics.writePaletteColor(1, 0xFF) == true);
    assert(graphics.writePaletteColor(15, 0xA9) == true);

    assert(memory.read8(0xFA00) == 0x00);
    assert(memory.read8(0xFA01) == 0xFF);
    assert(memory.read8(0xFA0F) == 0xA9);

    assert(graphics.readPaletteColor(0) == 0x00);
    assert(graphics.readPaletteColor(1) == 0xFF);
    assert(graphics.readPaletteColor(15) == 0xA9);

    raw = 0x55;
    assert(graphics.readPaletteColorChecked(15, raw) == true);
    assert(raw == 0xA9);

    raw = 0x77;
    assert(graphics.readPaletteColorChecked(16, raw) == false);
    assert(raw == 0x77);

    assert(graphics.writePaletteColor(16, 0xFF) == false);
    assert(memory.read8(0xFA0F) == 0xA9);

    assert(graphics.writePaletteRgb(2, 5, 2, 1) == true);
    assert(memory.read8(0xFA02) == 0xA9);

    red8 = 0;
    green8 = 0;
    blue8 = 0;

    assert(graphics.readPaletteRgb888(2, red8, green8, blue8) == true);
    assert(red8 == 0xB6);
    assert(green8 == 0x49);
    assert(blue8 == 0x55);

    memory.write8(0xFA03, 0x12);

    assert(graphics.writePaletteRgb(3, 8, 0, 0) == false);
    assert(memory.read8(0xFA03) == 0x12);

    assert(graphics.writePaletteRgb(3, 0, 8, 0) == false);
    assert(memory.read8(0xFA03) == 0x12);

    assert(graphics.writePaletteRgb(3, 0, 0, 4) == false);
    assert(memory.read8(0xFA03) == 0x12);

    assert(graphics.writePaletteRgb(16, 1, 1, 1) == false);
    assert(memory.read8(0xFA0F) == 0xA9);

    red8 = 0x11;
    green8 = 0x22;
    blue8 = 0x33;

    assert(graphics.readPaletteRgb888(16, red8, green8, blue8) == false);
    assert(red8 == 0x11);
    assert(green8 == 0x22);
    assert(blue8 == 0x33);

    assert(graphics.fillPalette(0xE3) == true);

    for (int i = 0; i < GraphicsMemory::PALETTE_SIZE; i++) {
        assert(memory.read8(GraphicsMemory::PALETTE_BASE + i) == 0xE3);
        assert(graphics.readPaletteColor(i) == 0xE3);
    }

    assert(graphics.clearPalette() == true);

    for (int i = 0; i < GraphicsMemory::PALETTE_SIZE; i++) {
        assert(memory.read8(GraphicsMemory::PALETTE_BASE + i) == 0x00);
        assert(graphics.readPaletteColor(i) == 0x00);
    }

    assert(memory.read8(0xFA10) == 0x00);

    printf("[PASS] Palette memory test passed\n");
}

void testRgb332Decode() {
    Rgb332Color color;

    color = GraphicsMemory::decodeRgb332(0x00);
    assert(color.red3 == 0);
    assert(color.green3 == 0);
    assert(color.blue2 == 0);

    color = GraphicsMemory::decodeRgb332(0xFF);
    assert(color.red3 == 7);
    assert(color.green3 == 7);
    assert(color.blue2 == 3);

    color = GraphicsMemory::decodeRgb332(0xE0);
    assert(color.red3 == 7);
    assert(color.green3 == 0);
    assert(color.blue2 == 0);

    color = GraphicsMemory::decodeRgb332(0x1C);
    assert(color.red3 == 0);
    assert(color.green3 == 7);
    assert(color.blue2 == 0);

    color = GraphicsMemory::decodeRgb332(0x03);
    assert(color.red3 == 0);
    assert(color.green3 == 0);
    assert(color.blue2 == 3);

    color = GraphicsMemory::decodeRgb332(0xA9);
    assert(color.red3 == 5);
    assert(color.green3 == 2);
    assert(color.blue2 == 1);

    color = GraphicsMemory::decodeRgb332(0x92);
    assert(color.red3 == 4);
    assert(color.green3 == 4);
    assert(color.blue2 == 2);

    assert(GraphicsMemory::makeRgb332(color.red3, color.green3, color.blue2) == 0x92);

    Memory memory;
    GraphicsMemory graphics(memory);

    assert(graphics.writePaletteColor(0, 0x00) == true);
    assert(graphics.writePaletteColor(1, 0xFF) == true);
    assert(graphics.writePaletteColor(2, 0xA9) == true);
    assert(graphics.writePaletteColor(15, 0xE3) == true);

    color.red3 = 9;
    color.green3 = 9;
    color.blue2 = 9;

    assert(graphics.readPaletteRgb332(0, color) == true);
    assert(color.red3 == 0);
    assert(color.green3 == 0);
    assert(color.blue2 == 0);

    assert(graphics.readPaletteRgb332(1, color) == true);
    assert(color.red3 == 7);
    assert(color.green3 == 7);
    assert(color.blue2 == 3);

    assert(graphics.readPaletteRgb332(2, color) == true);
    assert(color.red3 == 5);
    assert(color.green3 == 2);
    assert(color.blue2 == 1);

    assert(graphics.readPaletteRgb332(15, color) == true);
    assert(color.red3 == 7);
    assert(color.green3 == 0);
    assert(color.blue2 == 3);

    color.red3 = 8;
    color.green3 = 8;
    color.blue2 = 8;

    assert(graphics.readPaletteRgb332(16, color) == false);
    assert(color.red3 == 8);
    assert(color.green3 == 8);
    assert(color.blue2 == 8);

    printf("[PASS] RGB332 decode test passed\n");
}

void testRgb332ToRgb888Expansion() {
    Rgb888Color color;

    assert(GraphicsMemory::expandRgb3To8(0) == 0x00);
    assert(GraphicsMemory::expandRgb3To8(1) == 0x24);
    assert(GraphicsMemory::expandRgb3To8(2) == 0x49);
    assert(GraphicsMemory::expandRgb3To8(3) == 0x6D);
    assert(GraphicsMemory::expandRgb3To8(4) == 0x92);
    assert(GraphicsMemory::expandRgb3To8(5) == 0xB6);
    assert(GraphicsMemory::expandRgb3To8(6) == 0xDB);
    assert(GraphicsMemory::expandRgb3To8(7) == 0xFF);

    assert(GraphicsMemory::expandRgb2To8(0) == 0x00);
    assert(GraphicsMemory::expandRgb2To8(1) == 0x55);
    assert(GraphicsMemory::expandRgb2To8(2) == 0xAA);
    assert(GraphicsMemory::expandRgb2To8(3) == 0xFF);

    color = GraphicsMemory::expandRgb332ToRgb888Color(0x00);
    assert(color.red8 == 0x00);
    assert(color.green8 == 0x00);
    assert(color.blue8 == 0x00);

    color = GraphicsMemory::expandRgb332ToRgb888Color(0xFF);
    assert(color.red8 == 0xFF);
    assert(color.green8 == 0xFF);
    assert(color.blue8 == 0xFF);

    color = GraphicsMemory::expandRgb332ToRgb888Color(0xE0);
    assert(color.red8 == 0xFF);
    assert(color.green8 == 0x00);
    assert(color.blue8 == 0x00);

    color = GraphicsMemory::expandRgb332ToRgb888Color(0x1C);
    assert(color.red8 == 0x00);
    assert(color.green8 == 0xFF);
    assert(color.blue8 == 0x00);

    color = GraphicsMemory::expandRgb332ToRgb888Color(0x03);
    assert(color.red8 == 0x00);
    assert(color.green8 == 0x00);
    assert(color.blue8 == 0xFF);

    color = GraphicsMemory::expandRgb332ToRgb888Color(0xA9);
    assert(color.red8 == 0xB6);
    assert(color.green8 == 0x49);
    assert(color.blue8 == 0x55);

    color = GraphicsMemory::expandRgb332ToRgb888Color(0x92);
    assert(color.red8 == 0x92);
    assert(color.green8 == 0x92);
    assert(color.blue8 == 0xAA);

    Memory memory;
    GraphicsMemory graphics(memory);

    assert(graphics.writePaletteColor(0, 0x00) == true);
    assert(graphics.writePaletteColor(1, 0xFF) == true);
    assert(graphics.writePaletteColor(2, 0xE0) == true);
    assert(graphics.writePaletteColor(3, 0x1C) == true);
    assert(graphics.writePaletteColor(4, 0x03) == true);
    assert(graphics.writePaletteColor(5, 0xA9) == true);

    color.red8 = 1;
    color.green8 = 2;
    color.blue8 = 3;

    assert(graphics.readPaletteRgb888Color(0, color) == true);
    assert(color.red8 == 0x00);
    assert(color.green8 == 0x00);
    assert(color.blue8 == 0x00);

    assert(graphics.readPaletteRgb888Color(1, color) == true);
    assert(color.red8 == 0xFF);
    assert(color.green8 == 0xFF);
    assert(color.blue8 == 0xFF);

    assert(graphics.readPaletteRgb888Color(2, color) == true);
    assert(color.red8 == 0xFF);
    assert(color.green8 == 0x00);
    assert(color.blue8 == 0x00);

    assert(graphics.readPaletteRgb888Color(3, color) == true);
    assert(color.red8 == 0x00);
    assert(color.green8 == 0xFF);
    assert(color.blue8 == 0x00);

    assert(graphics.readPaletteRgb888Color(4, color) == true);
    assert(color.red8 == 0x00);
    assert(color.green8 == 0x00);
    assert(color.blue8 == 0xFF);

    assert(graphics.readPaletteRgb888Color(5, color) == true);
    assert(color.red8 == 0xB6);
    assert(color.green8 == 0x49);
    assert(color.blue8 == 0x55);

    color.red8 = 0x11;
    color.green8 = 0x22;
    color.blue8 = 0x33;

    assert(graphics.readPaletteRgb888Color(16, color) == false);
    assert(color.red8 == 0x11);
    assert(color.green8 == 0x22);
    assert(color.blue8 == 0x33);

    printf("[PASS] RGB332 to RGB888 expansion test passed\n");
}

void testTilePixelExtraction() {
    Memory memory;
    GraphicsMemory graphics(memory);

    unsigned char value = 0;

    assert(GraphicsMemory::extractLowNibble(0xBA) == 0x0A);
    assert(GraphicsMemory::extractHighNibble(0xBA) == 0x0B);

    assert(GraphicsMemory::extractPaletteIndexFromTileByte(0xBA, 0) == 0x0A);
    assert(GraphicsMemory::extractPaletteIndexFromTileByte(0xBA, 1) == 0x0B);
    assert(GraphicsMemory::extractPaletteIndexFromTileByte(0xBA, 14) == 0x0A);
    assert(GraphicsMemory::extractPaletteIndexFromTileByte(0xBA, 15) == 0x0B);

    assert(graphics.writeTileDefinitionByte(6, 0, 0xBA) == true);

    value = 0;
    assert(graphics.readTilePixelPaletteIndex(6, 0, 0, value) == true);
    assert(value == 0x0A);

    value = 0;
    assert(graphics.readTilePixelPaletteIndex(6, 1, 0, value) == true);
    assert(value == 0x0B);

    assert(graphics.writeTileDefinitionByte(6, 1, 0xC5) == true);

    value = 0;
    assert(graphics.readTilePixelPaletteIndex(6, 2, 0, value) == true);
    assert(value == 0x05);

    value = 0;
    assert(graphics.readTilePixelPaletteIndex(6, 3, 0, value) == true);
    assert(value == 0x0C);

    assert(graphics.writeTileDefinitionByte(6, 127, 0xD7) == true);

    value = 0;
    assert(graphics.readTilePixelPaletteIndex(6, 14, 15, value) == true);
    assert(value == 0x07);

    value = 0;
    assert(graphics.readTilePixelPaletteIndex(6, 15, 15, value) == true);
    assert(value == 0x0D);

    value = 0xEE;
    assert(graphics.readTilePixelPaletteIndex(16, 0, 0, value) == false);
    assert(value == 0xEE);

    value = 0xDD;
    assert(graphics.readTilePixelPaletteIndex(6, 16, 0, value) == false);
    assert(value == 0xDD);

    value = 0xCC;
    assert(graphics.readTilePixelPaletteIndex(6, 0, 16, value) == false);
    assert(value == 0xCC);

    printf("[PASS] Tile pixel extraction test passed\n");
}

void testTileRenderer() {
    Memory memory;
    GraphicsMemory graphics(memory);

    Rgb888Color pixels[GraphicsMemory::TILE_PIXEL_COUNT];
    Rgb888Color color;
    unsigned char palettePixels[GraphicsMemory::TILE_PIXEL_COUNT];

    for (int i = 0; i < GraphicsMemory::TILE_PIXEL_COUNT; i++) {
        pixels[i].red8 = 0x11;
        pixels[i].green8 = 0x22;
        pixels[i].blue8 = 0x33;
        palettePixels[i] = 0xEE;
    }

    assert(graphics.writePaletteColor(0, 0x00) == true);
    assert(graphics.writePaletteColor(1, 0xFF) == true);
    assert(graphics.writePaletteColor(2, 0xE0) == true);
    assert(graphics.writePaletteColor(3, 0x1C) == true);
    assert(graphics.writePaletteColor(4, 0x03) == true);
    assert(graphics.writePaletteColor(5, 0xA9) == true);

    assert(graphics.clearTileDefinition(4, 0) == true);

    assert(graphics.writeTilePixel(4, 0, 0, 1) == true);
    assert(graphics.writeTilePixel(4, 1, 0, 2) == true);
    assert(graphics.writeTilePixel(4, 2, 0, 3) == true);
    assert(graphics.writeTilePixel(4, 8, 8, 5) == true);
    assert(graphics.writeTilePixel(4, 15, 15, 4) == true);

    assert(graphics.renderTilePaletteIndices(4, palettePixels, GraphicsMemory::TILE_PIXEL_COUNT) == true);

    assert(palettePixels[0] == 1);
    assert(palettePixels[1] == 2);
    assert(palettePixels[2] == 3);
    assert(palettePixels[3] == 0);
    assert(palettePixels[8 + 8 * GraphicsMemory::TILE_SIZE] == 5);
    assert(palettePixels[255] == 4);

    assert(graphics.renderTile(4, pixels, GraphicsMemory::TILE_PIXEL_COUNT) == true);

    assert(pixels[0].red8 == 0xFF);
    assert(pixels[0].green8 == 0xFF);
    assert(pixels[0].blue8 == 0xFF);

    assert(pixels[1].red8 == 0xFF);
    assert(pixels[1].green8 == 0x00);
    assert(pixels[1].blue8 == 0x00);

    assert(pixels[2].red8 == 0x00);
    assert(pixels[2].green8 == 0xFF);
    assert(pixels[2].blue8 == 0x00);

    assert(pixels[3].red8 == 0x00);
    assert(pixels[3].green8 == 0x00);
    assert(pixels[3].blue8 == 0x00);

    int middlePixel = 8 + 8 * GraphicsMemory::TILE_SIZE;

    assert(pixels[middlePixel].red8 == 0xB6);
    assert(pixels[middlePixel].green8 == 0x49);
    assert(pixels[middlePixel].blue8 == 0x55);

    assert(pixels[255].red8 == 0x00);
    assert(pixels[255].green8 == 0x00);
    assert(pixels[255].blue8 == 0xFF);

    color.red8 = 0x11;
    color.green8 = 0x22;
    color.blue8 = 0x33;

    assert(graphics.renderTilePixel(4, 0, 0, color) == true);
    assert(color.red8 == 0xFF);
    assert(color.green8 == 0xFF);
    assert(color.blue8 == 0xFF);

    color.red8 = 0x11;
    color.green8 = 0x22;
    color.blue8 = 0x33;

    assert(graphics.renderTilePixel(16, 0, 0, color) == false);
    assert(color.red8 == 0x11);
    assert(color.green8 == 0x22);
    assert(color.blue8 == 0x33);

    assert(graphics.renderTile(16, pixels, GraphicsMemory::TILE_PIXEL_COUNT) == false);
    assert(graphics.renderTile(4, 0, GraphicsMemory::TILE_PIXEL_COUNT) == false);
    assert(graphics.renderTile(4, pixels, GraphicsMemory::TILE_PIXEL_COUNT - 1) == false);

    assert(graphics.renderTilePaletteIndices(16, palettePixels, GraphicsMemory::TILE_PIXEL_COUNT) == false);
    assert(graphics.renderTilePaletteIndices(4, 0, GraphicsMemory::TILE_PIXEL_COUNT) == false);
    assert(graphics.renderTilePaletteIndices(4, palettePixels, GraphicsMemory::TILE_PIXEL_COUNT - 1) == false);

    printf("[PASS] Tile renderer test passed\n");
}

void assertRgbColor(Rgb888Color color, unsigned char red, unsigned char green, unsigned char blue) {
    assert(color.red8 == red);
    assert(color.green8 == green);
    assert(color.blue8 == blue);
}

void setupRendererTestPalette(GraphicsMemory& graphics) {
    assert(graphics.writePaletteColor(0, 0x00) == true);
    assert(graphics.writePaletteColor(1, 0xFF) == true);
    assert(graphics.writePaletteColor(2, 0xE0) == true);
    assert(graphics.writePaletteColor(3, 0x1C) == true);
    assert(graphics.writePaletteColor(4, 0x03) == true);
    assert(graphics.writePaletteColor(5, 0xA9) == true);
}

void testCompleteOneTileRenderer() {
    Memory memory;
    GraphicsMemory graphics(memory);

    Rgb888Color pixels[GraphicsMemory::TILE_PIXEL_COUNT];
    unsigned char palettePixels[GraphicsMemory::TILE_PIXEL_COUNT];

    setupRendererTestPalette(graphics);

    for (int y = 0; y < GraphicsMemory::TILE_SIZE; y++) {
        for (int x = 0; x < GraphicsMemory::TILE_SIZE; x++) {
            unsigned char paletteIndex = (unsigned char)((x + y) % 6);
            assert(graphics.writeTilePixel(7, x, y, paletteIndex) == true);
        }
    }

    assert(graphics.renderTilePaletteIndices(7, palettePixels, GraphicsMemory::TILE_PIXEL_COUNT) == true);

    for (int y = 0; y < GraphicsMemory::TILE_SIZE; y++) {
        for (int x = 0; x < GraphicsMemory::TILE_SIZE; x++) {
            int pixelNumber = GraphicsMemory::getTilePixelNumber(x, y);
            unsigned char expected = (unsigned char)((x + y) % 6);
            assert(palettePixels[pixelNumber] == expected);
        }
    }

    assert(graphics.renderTile(7, pixels, GraphicsMemory::TILE_PIXEL_COUNT) == true);

    assertRgbColor(pixels[0], 0x00, 0x00, 0x00);
    assertRgbColor(pixels[1], 0xFF, 0xFF, 0xFF);
    assertRgbColor(pixels[2], 0xFF, 0x00, 0x00);
    assertRgbColor(pixels[3], 0x00, 0xFF, 0x00);
    assertRgbColor(pixels[4], 0x00, 0x00, 0xFF);
    assertRgbColor(pixels[5], 0xB6, 0x49, 0x55);

    printf("[PASS] Complete one tile renderer test passed\n");
}

void testFullScreenRenderer() {
    Memory memory;
    GraphicsMemory graphics(memory);

    static unsigned char palettePixels[GraphicsMemory::SCREEN_PIXEL_COUNT];
    static Rgb888Color pixels[GraphicsMemory::SCREEN_PIXEL_COUNT];

    setupRendererTestPalette(graphics);

    assert(graphics.clearTileDefinition(2, 3) == true);
    assert(graphics.fillTileMap(2) == true);

    assert(graphics.renderScreenPaletteIndices(palettePixels, GraphicsMemory::SCREEN_PIXEL_COUNT) == true);

    for (int i = 0; i < GraphicsMemory::SCREEN_PIXEL_COUNT; i++) {
        assert(palettePixels[i] == 3);
    }

    assert(graphics.renderScreen(pixels, GraphicsMemory::SCREEN_PIXEL_COUNT) == true);

    assertRgbColor(pixels[0], 0x00, 0xFF, 0x00);
    assertRgbColor(pixels[319], 0x00, 0xFF, 0x00);
    assertRgbColor(pixels[320], 0x00, 0xFF, 0x00);
    assertRgbColor(pixels[GraphicsMemory::SCREEN_PIXEL_COUNT - 1], 0x00, 0xFF, 0x00);

    printf("[PASS] Full screen renderer test passed\n");
}

void testRendererLiveUpdate() {
    Memory memory;
    GraphicsMemory graphics(memory);

    Rgb888Color color;

    setupRendererTestPalette(graphics);

    assert(graphics.clearTileDefinition(0, 0) == true);
    assert(graphics.fillTileMap(0) == true);

    color.red8 = 0x11;
    color.green8 = 0x22;
    color.blue8 = 0x33;

    assert(graphics.renderScreenPixel(0, 0, color) == true);
    assertRgbColor(color, 0x00, 0x00, 0x00);

    assert(graphics.writeTilePixel(0, 0, 0, 1) == true);
    assert(graphics.renderScreenPixel(0, 0, color) == true);
    assertRgbColor(color, 0xFF, 0xFF, 0xFF);

    assert(graphics.writeTilePixel(0, 0, 0, 2) == true);
    assert(graphics.renderScreenPixel(0, 0, color) == true);
    assertRgbColor(color, 0xFF, 0x00, 0x00);

    assert(graphics.writePaletteColor(2, 0x03) == true);
    assert(graphics.renderScreenPixel(0, 0, color) == true);
    assertRgbColor(color, 0x00, 0x00, 0xFF);

    printf("[PASS] Renderer live update test passed\n");
}

void testRendererConnectedToVram() {
    Memory memory;
    GraphicsMemory graphics(memory);

    Rgb888Color color;

    setupRendererTestPalette(graphics);

    memory.write8(GraphicsMemory::getTileMapAddress(0, 0), 5);
    memory.write8(GraphicsMemory::getTileDefinitionBase(5), 0x21);

    assert(graphics.renderScreenPixel(0, 0, color) == true);
    assertRgbColor(color, 0xFF, 0xFF, 0xFF);

    assert(graphics.renderScreenPixel(1, 0, color) == true);
    assertRgbColor(color, 0xFF, 0x00, 0x00);

    memory.write8(GraphicsMemory::getPaletteAddress(1), 0x1C);

    assert(graphics.renderScreenPixel(0, 0, color) == true);
    assertRgbColor(color, 0x00, 0xFF, 0x00);

    memory.write8(GraphicsMemory::getTileMapAddress(0, 0), 6);
    memory.write8(GraphicsMemory::getTileDefinitionBase(6), 0x04);

    assert(graphics.renderScreenPixel(0, 0, color) == true);
    assertRgbColor(color, 0x00, 0x00, 0xFF);

    printf("[PASS] Renderer connected to VRAM test passed\n");
}

void testFrameTimingStableRefresh() {
    assert(Gui::getTargetFps() == 60);

    float target = Gui::getTargetFrameTimeMs();

    assert(target > 16.0f);
    assert(target < 17.0f);

    assert(Gui::isStableFrameTimeMs(16.67f) == true);
    assert(Gui::isStableFrameTimeMs(15.00f) == true);
    assert(Gui::isStableFrameTimeMs(20.00f) == true);

    assert(Gui::isStableFrameTimeMs(5.00f) == false);
    assert(Gui::isStableFrameTimeMs(30.00f) == false);
    assert(Gui::isStableFrameTimeMs(100.00f) == false);

    printf("[PASS] Frame timing test passed\n");
}

void testEcallPrintStringExecution() {
    CPU cpu;

    unsigned short word = makeSys(0x012);
    unsigned short address = 0x8000;

    cpu.clearOutput();

    cpu.getMemory().write8(address + 0, 'H');
    cpu.getMemory().write8(address + 1, 'e');
    cpu.getMemory().write8(address + 2, 'l');
    cpu.getMemory().write8(address + 3, 'l');
    cpu.getMemory().write8(address + 4, 'o');
    cpu.getMemory().write8(address + 5, 0);

    cpu.setPC(0x1000);
    cpu.getRegisters().setRegister(6, address);
    cpu.getMemory().write16(0x1000, word);
    cpu.step();

    assert(strcmp(cpu.getOutput(), "Hello") == 0);

    cpu.clearOutput();

    cpu.getMemory().write8(address + 0, 'Z');
    cpu.getMemory().write8(address + 1, 'X');
    cpu.getMemory().write8(address + 2, '1');
    cpu.getMemory().write8(address + 3, '6');
    cpu.getMemory().write8(address + 4, '\n');
    cpu.getMemory().write8(address + 5, 'O');
    cpu.getMemory().write8(address + 6, 'K');
    cpu.getMemory().write8(address + 7, 0);

    cpu.setPC(0x1100);
    cpu.getRegisters().setRegister(6, address);
    cpu.getMemory().write16(0x1100, word);
    cpu.step();

    assert(strcmp(cpu.getOutput(), "ZX16\nOK") == 0);

    cpu.clearOutput();

    cpu.getMemory().write8(address, 0);

    cpu.setPC(0x1200);
    cpu.getRegisters().setRegister(6, address);
    cpu.getMemory().write16(0x1200, word);
    cpu.step();

    assert(strcmp(cpu.getOutput(), "") == 0);

    printf("[PASS] ECALL print_string test passed\n");
}

void testEcallReadIntExecution() {
    CPU cpu;

    unsigned short word = makeSys(0x011);

    cpu.setInput("123 -45\n32767 -32768 abc 99");

    cpu.setPC(0x2000);
    cpu.getMemory().write16(0x2000, word);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 123);

    cpu.setPC(0x2002);
    cpu.getMemory().write16(0x2002, word);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == (unsigned short)-45);

    cpu.setPC(0x2004);
    cpu.getMemory().write16(0x2004, word);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 32767);

    cpu.setPC(0x2006);
    cpu.getMemory().write16(0x2006, word);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 0x8000);

    cpu.setPC(0x2008);
    cpu.getMemory().write16(0x2008, word);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 0);

    printf("[PASS] ECALL read_int test passed\n");
}

void testEcallReadStringExecution() {
    CPU cpu;

    unsigned short word = makeSys(0x010);
    unsigned short address = 0x8100;

    cpu.setInput("hello world\nabcdef\nxy\n");

    cpu.setPC(0x3000);
    cpu.getRegisters().setRegister(6, address);
    cpu.getRegisters().setRegister(7, 20);
    cpu.getMemory().write16(0x3000, word);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 11);
    assert(cpu.getMemory().read8(address + 0) == 'h');
    assert(cpu.getMemory().read8(address + 10) == 'd');
    assert(cpu.getMemory().read8(address + 11) == 0);

    cpu.setPC(0x3002);
    cpu.getRegisters().setRegister(6, address + 32);
    cpu.getRegisters().setRegister(7, 4);
    cpu.getMemory().write16(0x3002, word);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 3);
    assert(cpu.getMemory().read8(address + 32) == 'a');
    assert(cpu.getMemory().read8(address + 33) == 'b');
    assert(cpu.getMemory().read8(address + 34) == 'c');
    assert(cpu.getMemory().read8(address + 35) == 0);

    cpu.setPC(0x3004);
    cpu.getRegisters().setRegister(6, address + 64);
    cpu.getRegisters().setRegister(7, 3);
    cpu.getMemory().write16(0x3004, word);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 2);
    assert(cpu.getMemory().read8(address + 64) == 'x');
    assert(cpu.getMemory().read8(address + 65) == 'y');
    assert(cpu.getMemory().read8(address + 66) == 0);

    cpu.setInput("no-write\n");

    cpu.getMemory().write8(address + 96, 0xAA);

    cpu.setPC(0x3006);
    cpu.getRegisters().setRegister(6, address + 96);
    cpu.getRegisters().setRegister(7, 0);
    cpu.getMemory().write16(0x3006, word);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 0);
    assert(cpu.getMemory().read8(address + 96) == 0xAA);

    cpu.setInput("z\n");

    cpu.getMemory().write8(address + 128, 0xBB);

    cpu.setPC(0x3008);
    cpu.getRegisters().setRegister(6, address + 128);
    cpu.getRegisters().setRegister(7, 1);
    cpu.getMemory().write16(0x3008, word);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 0);
    assert(cpu.getMemory().read8(address + 128) == 0);

    printf("[PASS] ECALL read_string test passed\n");
}

void testRngStateReset() {
    CPU cpu;

    assert(cpu.getRngState() == CPU::DEFAULT_RNG_SEED);

    cpu.seedRng(0x1234);
    assert(cpu.getRngState() == 0x1234);

    cpu.seedRng(0x0000);
    assert(cpu.getRngState() == 0x0000);

    cpu.seedRng(0xFFFF);
    assert(cpu.getRngState() == 0xFFFF);

    cpu.reset();
    assert(cpu.getRngState() == CPU::DEFAULT_RNG_SEED);

    cpu.seedRng(0xBEEF);
    assert(cpu.getRngState() == 0xBEEF);

    cpu.resetRng();
    assert(cpu.getRngState() == CPU::DEFAULT_RNG_SEED);

    printf("[PASS] RNG state reset test passed\n");
}

void testEcallSeedRngExecution() {
    CPU cpu;

    unsigned short word = makeSys(0x020);

    assert(cpu.getRngState() == CPU::DEFAULT_RNG_SEED);

    cpu.setPC(0x4000);
    cpu.getRegisters().setRegister(6, 0x1234);
    cpu.getMemory().write16(0x4000, word);
    cpu.step();

    assert(cpu.getRngState() == 0x1234);
    assert(cpu.getPC() == 0x4002);
    assert(cpu.getLastHandler() == ZX16::SYS_TYPE);

    cpu.setPC(0x4002);
    cpu.getRegisters().setRegister(6, 0xACE1);
    cpu.getMemory().write16(0x4002, word);
    cpu.step();

    assert(cpu.getRngState() == 0xACE1);
    assert(cpu.getPC() == 0x4004);

    cpu.setPC(0x4004);
    cpu.getRegisters().setRegister(6, 0x0000);
    cpu.getMemory().write16(0x4004, word);
    cpu.step();

    assert(cpu.getRngState() == 0x0000);
    assert(cpu.getPC() == 0x4006);

    printf("[PASS] ECALL seed_rng test passed\n");
}

void testEcallRandomXorshiftExecution() {
    CPU cpu;

    unsigned short randomWord = makeSys(0x021);

    cpu.seedRng(0xACE1);

    assert(cpu.nextRandom() == 0xD30F);
    assert(cpu.getRngState() == 0xD30F);

    assert(cpu.nextRandom() == 0xF1A5);
    assert(cpu.getRngState() == 0xF1A5);

    assert(cpu.nextRandom() == 0x1734);
    assert(cpu.getRngState() == 0x1734);

    assert(cpu.nextRandom() == 0xFF72);
    assert(cpu.getRngState() == 0xFF72);

    assert(cpu.nextRandom() == 0x1751);
    assert(cpu.getRngState() == 0x1751);

    cpu.seedRng(0x1234);

    cpu.setPC(0x5000);
    cpu.getMemory().write16(0x5000, randomWord);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 0x3830);
    assert(cpu.getRngState() == 0x3830);
    assert(cpu.getPC() == 0x5002);
    assert(cpu.getLastHandler() == ZX16::SYS_TYPE);

    cpu.setPC(0x5002);
    cpu.getMemory().write16(0x5002, randomWord);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 0x0020);
    assert(cpu.getRngState() == 0x0020);
    assert(cpu.getPC() == 0x5004);

    cpu.setPC(0x5004);
    cpu.getMemory().write16(0x5004, randomWord);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 0x3828);
    assert(cpu.getRngState() == 0x3828);
    assert(cpu.getPC() == 0x5006);

    cpu.seedRng(0x0000);

    cpu.setPC(0x5006);
    cpu.getMemory().write16(0x5006, randomWord);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == 0x0000);
    assert(cpu.getRngState() == 0x0000);

    printf("[PASS] ECALL random xorshift test passed\n");
}


void testKeyboardEcallDetectKeyPress() {
    CPU cpu;

    unsigned short keyboardWord = makeSys(0x030);

    assert(cpu.getKeyboardKey() == CPU::ZX16_KEY_NONE);

    cpu.setPC(0x6000);
    cpu.getMemory().write16(0x6000, keyboardWord);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == CPU::ZX16_KEY_NONE);
    assert(cpu.getPC() == 0x6002);
    assert(cpu.getLastHandler() == ZX16::SYS_TYPE);

    assert(cpu.setKeyboardKey(CPU::ZX16_KEY_UP) == true);
    assert(cpu.getKeyboardKey() == CPU::ZX16_KEY_UP);

    cpu.setPC(0x6002);
    cpu.getMemory().write16(0x6002, keyboardWord);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == CPU::ZX16_KEY_UP);
    assert(cpu.getPC() == 0x6004);

    assert(cpu.setKeyboardKey(CPU::ZX16_KEY_RIGHT) == true);
    assert(cpu.getKeyboardKey() == CPU::ZX16_KEY_RIGHT);

    cpu.setPC(0x6004);
    cpu.getMemory().write16(0x6004, keyboardWord);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == CPU::ZX16_KEY_RIGHT);
    assert(cpu.getPC() == 0x6006);

    cpu.clearKeyboardKey();
    assert(cpu.getKeyboardKey() == CPU::ZX16_KEY_NONE);

    cpu.setPC(0x6006);
    cpu.getMemory().write16(0x6006, keyboardWord);
    cpu.step();

    assert(cpu.getRegisters().getRegister(6) == CPU::ZX16_KEY_NONE);
    assert(cpu.getPC() == 0x6008);

    assert(cpu.setKeyboardKey(99) == false);
    assert(cpu.getKeyboardKey() == CPU::ZX16_KEY_NONE);

    printf("[PASS] Keyboard ECALL key press test passed\n");
}

void testRaylibKeyboardMappingDirections() {
    assert(Gui::mapRaylibKeyToZx16(KEY_UP) == CPU::ZX16_KEY_UP);
    assert(Gui::mapRaylibKeyToZx16(KEY_DOWN) == CPU::ZX16_KEY_DOWN);
    assert(Gui::mapRaylibKeyToZx16(KEY_LEFT) == CPU::ZX16_KEY_LEFT);
    assert(Gui::mapRaylibKeyToZx16(KEY_RIGHT) == CPU::ZX16_KEY_RIGHT);

    assert(Gui::mapRaylibKeyToZx16(KEY_W) == CPU::ZX16_KEY_UP);
    assert(Gui::mapRaylibKeyToZx16(KEY_S) == CPU::ZX16_KEY_DOWN);
    assert(Gui::mapRaylibKeyToZx16(KEY_A) == CPU::ZX16_KEY_LEFT);
    assert(Gui::mapRaylibKeyToZx16(KEY_D) == CPU::ZX16_KEY_RIGHT);

    assert(Gui::mapRaylibKeyToZx16(KEY_SPACE) == CPU::ZX16_KEY_SPACE);
    assert(Gui::mapRaylibKeyToZx16(KEY_ENTER) == CPU::ZX16_KEY_ENTER);
    assert(Gui::mapRaylibKeyToZx16(KEY_ESCAPE) == CPU::ZX16_KEY_ESCAPE);

    assert(Gui::mapRaylibKeyToZx16(0) == CPU::ZX16_KEY_NONE);
    assert(Gui::mapRaylibKeyToZx16(9999) == CPU::ZX16_KEY_NONE);

    printf("[PASS] Keyboard mapping directions test passed\n");
}

void testEcallPlayToneExecution() {
    CPU cpu;

    unsigned short toneWord = makeSys(0x040);

    assert(CPU::isValidTone(440, 100) == true);
    assert(CPU::isValidTone(CPU::MIN_TONE_FREQUENCY, 1) == true);
    assert(CPU::isValidTone(CPU::MAX_TONE_FREQUENCY, CPU::MAX_TONE_DURATION_MS) == true);

    assert(CPU::isValidTone(0, 100) == false);
    assert(CPU::isValidTone(19, 100) == false);
    assert(CPU::isValidTone(20001, 100) == false);
    assert(CPU::isValidTone(440, 0) == false);
    assert(CPU::isValidTone(440, 5001) == false);

    assert(cpu.hasPendingTone() == false);
    assert(cpu.getToneRequestId() == 0);

    cpu.setPC(0x7000);
    cpu.getRegisters().setRegister(6, 440);
    cpu.getRegisters().setRegister(7, 120);
    cpu.getMemory().write16(0x7000, toneWord);
    cpu.step();

    assert(cpu.hasPendingTone() == true);
    assert(cpu.getToneFrequency() == 440);
    assert(cpu.getToneDurationMs() == 120);
    assert(cpu.getToneRequestId() == 1);
    assert(cpu.getPC() == 0x7002);
    assert(cpu.getLastHandler() == ZX16::SYS_TYPE);

    cpu.clearToneRequest();

    assert(cpu.hasPendingTone() == false);
    assert(cpu.getToneFrequency() == 0);
    assert(cpu.getToneDurationMs() == 0);
    assert(cpu.getToneRequestId() == 1);

    cpu.setPC(0x7002);
    cpu.getRegisters().setRegister(6, 880);
    cpu.getRegisters().setRegister(7, 250);
    cpu.getMemory().write16(0x7002, toneWord);
    cpu.step();

    assert(cpu.hasPendingTone() == true);
    assert(cpu.getToneFrequency() == 880);
    assert(cpu.getToneDurationMs() == 250);
    assert(cpu.getToneRequestId() == 2);
    assert(cpu.getPC() == 0x7004);

    cpu.clearToneRequest();

    cpu.setPC(0x7004);
    cpu.getRegisters().setRegister(6, 0);
    cpu.getRegisters().setRegister(7, 100);
    cpu.getMemory().write16(0x7004, toneWord);
    cpu.step();

    assert(cpu.hasPendingTone() == false);
    assert(cpu.getToneRequestId() == 2);
    assert(cpu.getPC() == 0x7006);

    cpu.setPC(0x7006);
    cpu.getRegisters().setRegister(6, 440);
    cpu.getRegisters().setRegister(7, 0);
    cpu.getMemory().write16(0x7006, toneWord);
    cpu.step();

    assert(cpu.hasPendingTone() == false);
    assert(cpu.getToneRequestId() == 2);
    assert(cpu.getPC() == 0x7008);

    cpu.requestTone(1000, 50);
    assert(cpu.hasPendingTone() == true);
    assert(cpu.getToneRequestId() == 3);

    cpu.reset();

    assert(cpu.hasPendingTone() == false);
    assert(cpu.getToneFrequency() == 0);
    assert(cpu.getToneDurationMs() == 0);
    assert(cpu.getToneRequestId() == 0);

    printf("[PASS] ECALL play_tone test passed\n");
}

void testEcallSetVolumeExecution() {
    CPU cpu;

    unsigned short volumeWord = makeSys(0x041);

    assert(CPU::isValidVolumePercent(0) == true);
    assert(CPU::isValidVolumePercent(50) == true);
    assert(CPU::isValidVolumePercent(100) == true);
    assert(CPU::isValidVolumePercent(101) == false);
    assert(CPU::isValidVolumePercent(0xFFFF) == false);

    assert(cpu.getVolumePercent() == CPU::DEFAULT_VOLUME_PERCENT);

    assert(cpu.setVolumePercent(75) == true);
    assert(cpu.getVolumePercent() == 75);

    assert(cpu.setVolumePercent(0) == true);
    assert(cpu.getVolumePercent() == 0);

    assert(cpu.setVolumePercent(100) == true);
    assert(cpu.getVolumePercent() == 100);

    assert(cpu.setVolumePercent(101) == false);
    assert(cpu.getVolumePercent() == 100);

    cpu.setPC(0x7200);
    cpu.getRegisters().setRegister(6, 25);
    cpu.getMemory().write16(0x7200, volumeWord);
    cpu.step();

    assert(cpu.getVolumePercent() == 25);
    assert(cpu.getPC() == 0x7202);
    assert(cpu.getLastHandler() == ZX16::SYS_TYPE);

    cpu.setPC(0x7202);
    cpu.getRegisters().setRegister(6, 0);
    cpu.getMemory().write16(0x7202, volumeWord);
    cpu.step();

    assert(cpu.getVolumePercent() == 0);
    assert(cpu.getPC() == 0x7204);

    cpu.setPC(0x7204);
    cpu.getRegisters().setRegister(6, 100);
    cpu.getMemory().write16(0x7204, volumeWord);
    cpu.step();

    assert(cpu.getVolumePercent() == 100);
    assert(cpu.getPC() == 0x7206);

    cpu.setPC(0x7206);
    cpu.getRegisters().setRegister(6, 150);
    cpu.getMemory().write16(0x7206, volumeWord);
    cpu.step();

    assert(cpu.getVolumePercent() == 100);
    assert(cpu.getPC() == 0x7208);

    cpu.setVolumePercent(30);
    assert(cpu.getVolumePercent() == 30);

    cpu.resetVolume();
    assert(cpu.getVolumePercent() == CPU::DEFAULT_VOLUME_PERCENT);

    cpu.setVolumePercent(40);
    assert(cpu.getVolumePercent() == 40);

    cpu.reset();
    assert(cpu.getVolumePercent() == CPU::DEFAULT_VOLUME_PERCENT);

    printf("[PASS] ECALL set_volume test passed\n");
}

void testEcallStopAudioExecution() {
    CPU cpu;

    unsigned short stopWord = makeSys(0x042);

    assert(cpu.hasPendingStopAudio() == false);
    assert(cpu.getStopAudioRequestId() == 0);

    assert(cpu.requestTone(440, 500) == true);
    assert(cpu.hasPendingTone() == true);
    assert(cpu.getToneFrequency() == 440);
    assert(cpu.getToneDurationMs() == 500);
    assert(cpu.getToneRequestId() == 1);

    cpu.setPC(0x7400);
    cpu.getMemory().write16(0x7400, stopWord);
    cpu.step();

    assert(cpu.hasPendingStopAudio() == true);
    assert(cpu.getStopAudioRequestId() == 1);
    assert(cpu.hasPendingTone() == false);
    assert(cpu.getToneFrequency() == 0);
    assert(cpu.getToneDurationMs() == 0);
    assert(cpu.getPC() == 0x7402);
    assert(cpu.getLastHandler() == ZX16::SYS_TYPE);

    cpu.clearStopAudioRequest();

    assert(cpu.hasPendingStopAudio() == false);
    assert(cpu.getStopAudioRequestId() == 1);

    cpu.setPC(0x7402);
    cpu.getMemory().write16(0x7402, stopWord);
    cpu.step();

    assert(cpu.hasPendingStopAudio() == true);
    assert(cpu.getStopAudioRequestId() == 2);
    assert(cpu.hasPendingTone() == false);
    assert(cpu.getPC() == 0x7404);

    cpu.clearStopAudioRequest();

    assert(cpu.requestStopAudio() == true);
    assert(cpu.hasPendingStopAudio() == true);
    assert(cpu.getStopAudioRequestId() == 3);

    cpu.reset();

    assert(cpu.hasPendingStopAudio() == false);
    assert(cpu.getStopAudioRequestId() == 0);
    assert(cpu.hasPendingTone() == false);

    printf("[PASS] ECALL stop_audio test passed\n");
}

void testEcallRegsDumpExecution() {
    CPU cpu;

    unsigned short dumpWord = makeSys(0x050);

    cpu.clearOutput();

    cpu.setPC(0x7600);

    cpu.getRegisters().setRegister(0, 0x0000);
    cpu.getRegisters().setRegister(1, 0x1111);
    cpu.getRegisters().setRegister(2, 0xEFFE);
    cpu.getRegisters().setRegister(3, 0x3333);
    cpu.getRegisters().setRegister(4, 0x4444);
    cpu.getRegisters().setRegister(5, 0x5555);
    cpu.getRegisters().setRegister(6, 0x6666);
    cpu.getRegisters().setRegister(7, 0x7777);

    cpu.getMemory().write16(0x7600, dumpWord);
    cpu.step();

    const char expected[] =
        "REGS\n"
        "PC=0x7602\n"
        "SP=0xEFFE\n"
        "x0=0x0000\n"
        "x1=0x1111\n"
        "x2=0xEFFE\n"
        "x3=0x3333\n"
        "x4=0x4444\n"
        "x5=0x5555\n"
        "x6=0x6666\n"
        "x7=0x7777\n";

    assert(strcmp(cpu.getOutput(), expected) == 0);
    assert(cpu.getPC() == 0x7602);
    assert(cpu.getLastHandler() == ZX16::SYS_TYPE);

    cpu.clearOutput();

    cpu.setPC(0x7602);
    cpu.getRegisters().setRegister(1, 0xAAAA);
    cpu.getRegisters().setRegister(6, 0x1234);
    cpu.getMemory().write16(0x7602, dumpWord);
    cpu.step();

    const char expected2[] =
        "REGS\n"
        "PC=0x7604\n"
        "SP=0xEFFE\n"
        "x0=0x0000\n"
        "x1=0xAAAA\n"
        "x2=0xEFFE\n"
        "x3=0x3333\n"
        "x4=0x4444\n"
        "x5=0x5555\n"
        "x6=0x1234\n"
        "x7=0x7777\n";

    assert(strcmp(cpu.getOutput(), expected2) == 0);
    assert(cpu.getPC() == 0x7604);

    printf("[PASS] ECALL regs_dump test passed\n");
}

void testEcallMemDumpExecution() {
    CPU cpu;

    unsigned short dumpWord = makeSys(0x051);

    assert(CPU::isValidMemoryDumpLength(0) == true);
    assert(CPU::isValidMemoryDumpLength(1) == true);
    assert(CPU::isValidMemoryDumpLength(64) == true);
    assert(CPU::isValidMemoryDumpLength(65) == false);
    assert(CPU::isValidMemoryDumpLength(0xFFFF) == false);

    cpu.getMemory().write8(0x8000, 0x12);
    cpu.getMemory().write8(0x8001, 0x34);
    cpu.getMemory().write8(0x8002, 0xAB);
    cpu.getMemory().write8(0x8003, 0xCD);
    cpu.getMemory().write8(0x8004, 0x00);
    cpu.getMemory().write8(0x8005, 0xFF);
    cpu.getMemory().write8(0x8006, 0x55);
    cpu.getMemory().write8(0x8007, 0xAA);
    cpu.getMemory().write8(0x8008, 0x10);
    cpu.getMemory().write8(0x8009, 0x20);
    cpu.getMemory().write8(0x800A, 0x30);
    cpu.getMemory().write8(0x800B, 0x40);
    cpu.getMemory().write8(0x800C, 0x50);
    cpu.getMemory().write8(0x800D, 0x60);
    cpu.getMemory().write8(0x800E, 0x70);
    cpu.getMemory().write8(0x800F, 0x80);

    cpu.clearOutput();

    cpu.setPC(0x7800);
    cpu.getRegisters().setRegister(6, 0x8000);
    cpu.getRegisters().setRegister(7, 16);
    cpu.getMemory().write16(0x7800, dumpWord);
    cpu.step();

    const char expected[] =
        "MEM\n"
        "START=0x8000\n"
        "LEN=0x0010\n"
        "0x8000: 12 34 AB CD 00 FF 55 AA\n"
        "0x8008: 10 20 30 40 50 60 70 80\n";

    assert(strcmp(cpu.getOutput(), expected) == 0);
    assert(cpu.getPC() == 0x7802);
    assert(cpu.getLastHandler() == ZX16::SYS_TYPE);

    cpu.clearOutput();

    cpu.setPC(0x7802);
    cpu.getRegisters().setRegister(6, 0x8005);
    cpu.getRegisters().setRegister(7, 6);
    cpu.getMemory().write16(0x7802, dumpWord);
    cpu.step();

    const char expected2[] =
        "MEM\n"
        "START=0x8005\n"
        "LEN=0x0006\n"
        "0x8005: FF 55 AA 10 20 30\n";

    assert(strcmp(cpu.getOutput(), expected2) == 0);
    assert(cpu.getPC() == 0x7804);
    assert(cpu.getLastHandler() == ZX16::SYS_TYPE);

    cpu.clearOutput();

    cpu.setPC(0x7804);
    cpu.getRegisters().setRegister(6, 0x8000);
    cpu.getRegisters().setRegister(7, 0);
    cpu.getMemory().write16(0x7804, dumpWord);
    cpu.step();

    const char expected3[] =
        "MEM\n"
        "START=0x8000\n"
        "LEN=0x0000\n";

    assert(strcmp(cpu.getOutput(), expected3) == 0);
    assert(cpu.getPC() == 0x7806);

    cpu.clearOutput();

    cpu.setPC(0x7806);
    cpu.getRegisters().setRegister(6, 0x8000);
    cpu.getRegisters().setRegister(7, 65);
    cpu.getMemory().write16(0x7806, dumpWord);
    cpu.step();

    assert(strcmp(cpu.getOutput(), "") == 0);
    assert(cpu.getPC() == 0x7808);

    printf("[PASS] ECALL mem_dump test passed\n");
}

void testConsoleUpdatesCorrectly() {
    char visible[512];

    Gui::buildConsoleVisibleText("", visible, 512);
    assert(strcmp(visible, "") == 0);

    Gui::buildConsoleVisibleText("A", visible, 512);
    assert(strcmp(visible, "A") == 0);

    Gui::buildConsoleVisibleText("Line1\nLine2\nLine3\n", visible, 512);
    assert(strcmp(visible, "Line1\nLine2\nLine3") == 0);

    const char longText[] =
        "L1\n"
        "L2\n"
        "L3\n"
        "L4\n"
        "L5\n"
        "L6\n"
        "L7\n"
        "L8\n"
        "L9\n"
        "L10\n";

    Gui::buildConsoleVisibleText(longText, visible, 512);

   const char expectedLong[] =
    "L6\n"
    "L7\n"
    "L8\n"
    "L9\n"
    "L10";

    assert(strcmp(visible, expectedLong) == 0);

    const char memDumpText[] =
        "MEM\n"
        "START=0x8000\n"
        "LEN=0x0010\n"
        "0x8000: 12 34 AB CD 00 FF 55 AA\n"
        "0x8008: 10 20 30 40 50 60 70 80\n";

    Gui::buildConsoleVisibleText(memDumpText, visible, 512);

    const char expectedMemDump[] =
        "MEM\n"
        "START=0x8000\n"
        "LEN=0x0010\n"
        "0x8000: 12 34 AB CD 00 FF 55 AA\n"
        "0x8008: 10 20 30 40 50 60 70 80";

    assert(strcmp(visible, expectedMemDump) == 0);

    char smallVisible[8];

    Gui::buildConsoleVisibleText("ABCDEFGH", smallVisible, 8);
    assert(strcmp(smallVisible, "ABCDEFG") == 0);

    assert(Gui::getConsoleVisibleLineCount() == 5);

    printf("[PASS] Console update test passed\n");
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

void testStepOverSkipsFunctionBody() {
    CPU cpu;

    unsigned short randomWord = makeSys(0x021);

    // funct4=0xC (JALR), rs2=3 (jump target reg), rd=1 (return-addr reg), func3=0, opcode=0
    unsigned short jalrWord = (unsigned short)((0xC << 12) | (3 << 9) | (1 << 6));

    // funct4=0xB (JR), rd=1 (register holding the address to jump to), opcode=0
    unsigned short jrWord = (unsigned short)((0xB << 12) | (1 << 6));

    assert(CPU::isCallInstructionWord(jalrWord) == true);
    assert(CPU::isCallInstructionWord(jrWord) == false);
    assert(CPU::isCallInstructionWord(randomWord) == false);

    cpu.setPC(0x7000);
    cpu.seedRng(0x1234);

    cpu.getRegisters().setRegister(3, 0x7100);

    cpu.getMemory().write16(0x7000, jalrWord);
    cpu.getMemory().write16(0x7002, randomWord);

    cpu.getMemory().write16(0x7100, randomWord);
    cpu.getMemory().write16(0x7102, randomWord);
    cpu.getMemory().write16(0x7104, jrWord);

    bool executed = cpu.stepOver();

    assert(executed == true);
    assert(cpu.getPC() == 0x7002);
    assert(cpu.getRegisters().getRegister(1) == 0x7002);
    assert(cpu.getRegisters().getRegister(6) == 0x0020);
    assert(cpu.hasBreakpointHit() == false);

    cpu.step();

    assert(cpu.getPC() == 0x7004);
    assert(cpu.getRegisters().getRegister(6) == 0x3828);

    cpu.setPC(0x7200);
    cpu.getMemory().write16(0x7200, randomWord);
    cpu.seedRng(0x1234);
    cpu.getRegisters().setRegister(6, 0);

    executed = cpu.stepOver();

    assert(executed == true);
    assert(cpu.getPC() == 0x7202);
    assert(cpu.getRegisters().getRegister(6) == 0x3830);

    printf("[PASS] Step over skips function body test passed\n");
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

void loadFirstGraphicsDemo(CPU& cpu) {
    cpu.reset();

    GraphicsMemory graphics(cpu.getMemory());

    graphics.writePaletteColor(0, 0x00); // Black
    graphics.writePaletteColor(1, 0xE0); // Red
    graphics.writePaletteColor(2, 0x1C); // Green
    graphics.writePaletteColor(3, 0x03); // Blue
    graphics.writePaletteColor(4, 0xFF); // White
    graphics.writePaletteColor(5, 0xFC); // Yellow
    graphics.writePaletteColor(6, 0x1F); // Cyan
    graphics.writePaletteColor(7, 0x80); // Dark red

    graphics.clearTileDefinition(0, 3); // Sky
    graphics.clearTileDefinition(1, 2); // Grass
    graphics.clearTileDefinition(2, 3); // Sun tile background
    graphics.clearTileDefinition(3, 1); // House wall
    graphics.clearTileDefinition(4, 7); // House roof
    graphics.clearTileDefinition(5, 1); // Door tile background
    graphics.clearTileDefinition(6, 1); // Window tile background

    for (int y = 4; y <= 11; y++) {
        for (int x = 4; x <= 11; x++) {
            graphics.writeTilePixel(2, x, y, 5);
        }
    }

    for (int y = 4; y < 16; y++) {
        for (int x = 5; x <= 10; x++) {
            graphics.writeTilePixel(5, x, y, 0);
        }
    }

    for (int y = 4; y <= 11; y++) {
        for (int x = 4; x <= 11; x++) {
            graphics.writeTilePixel(6, x, y, 4);
        }
    }

    graphics.fillTileMap(0);

    for (int row = 10; row < GraphicsMemory::TILE_ROWS; row++) {
        for (int col = 0; col < GraphicsMemory::TILE_COLUMNS; col++) {
            graphics.writeTileIndex(col, row, 1);
        }
    }

    graphics.writeTileIndex(16, 1, 2);

    for (int col = 8; col <= 11; col++) {
        graphics.writeTileIndex(col, 8, 4);
    }

    for (int row = 9; row <= 12; row++) {
        for (int col = 8; col <= 11; col++) {
            graphics.writeTileIndex(col, row, 3);
        }
    }

    graphics.writeTileIndex(8, 10, 6);
    graphics.writeTileIndex(11, 10, 6);
    graphics.writeTileIndex(9, 12, 5);
}

int getKeyboardDemoBackgroundTile(int row) {
    if (row >= 10) {
        return 1;
    }

    return 0;
}

static int keyboardDemoObjectCol = 2;
static int keyboardDemoObjectRow = 12;

void drawKeyboardDemoObject(CPU& cpu, int col, int row) {
    GraphicsMemory graphics(cpu.getMemory());

    int tileIndex = 8;

    if (row >= 10) {
        tileIndex = 9;
    }

    graphics.writeTileIndex(col, row, (unsigned char)tileIndex);
}

void loadKeyboardDemo(CPU& cpu) {
    loadFirstGraphicsDemo(cpu);

    GraphicsMemory graphics(cpu.getMemory());

    graphics.clearTileDefinition(8, 3);
    graphics.clearTileDefinition(9, 2);

    for (int y = 4; y <= 11; y++) {
        for (int x = 4; x <= 11; x++) {
            graphics.writeTilePixel(8, x, y, 4);
            graphics.writeTilePixel(9, x, y, 4);
        }
    }

    keyboardDemoObjectCol = 2;
    keyboardDemoObjectRow = 12;

    drawKeyboardDemoObject(cpu, keyboardDemoObjectCol, keyboardDemoObjectRow);
}

bool updateKeyboardDemo(CPU& cpu) {
    int oldCol = keyboardDemoObjectCol;
    int oldRow = keyboardDemoObjectRow;

    int newCol = oldCol;
    int newRow = oldRow;

    unsigned short key = cpu.getKeyboardKey();

    if (key == CPU::ZX16_KEY_LEFT && newCol > 0) {
        newCol--;
    }

    if (key == CPU::ZX16_KEY_RIGHT && newCol < 6) {
        newCol++;
    }

    if (key == CPU::ZX16_KEY_UP && newRow > 0) {
        newRow--;
    }

    if (key == CPU::ZX16_KEY_DOWN && newRow < GraphicsMemory::TILE_ROWS - 1) {
        newRow++;
    }

    if (newCol == oldCol && newRow == oldRow) {
        return false;
    }

    GraphicsMemory graphics(cpu.getMemory());

    int backgroundTile = getKeyboardDemoBackgroundTile(oldRow);
    graphics.writeTileIndex(oldCol, oldRow, (unsigned char)backgroundTile);

    keyboardDemoObjectCol = newCol;
    keyboardDemoObjectRow = newRow;

    drawKeyboardDemoObject(cpu, newCol, newRow);

    return true;
}

bool updateAudioDemo(CPU& cpu) {
    if (cpu.getKeyboardKey() != CPU::ZX16_KEY_SPACE) {
        return false;
    }

    return cpu.requestTone(440, 200);
}

void loadGuiDemoProgram(CPU& cpu) {
    loadKeyboardDemo(cpu);

    cpu.getMemory().write16(0x0020, makeSys(0x050));
    cpu.getMemory().write16(0x0022, makeSys(0x3FF));
}

void testFirstGraphicsDemoDrawsStaticImage() {
    CPU cpu;

    loadFirstGraphicsDemo(cpu);

    GraphicsMemory graphics(cpu.getMemory());
    Rgb888Color color;

    assert(graphics.readTileIndex(0, 0) == 0);
    assert(graphics.readTileIndex(0, 14) == 1);
    assert(graphics.readTileIndex(16, 1) == 2);
    assert(graphics.readTileIndex(8, 8) == 4);
    assert(graphics.readTileIndex(8, 10) == 6);
    assert(graphics.readTileIndex(9, 12) == 5);

    assert(graphics.renderScreenPixel(0, 0, color) == true);
    assertRgbColor(color, 0x00, 0x00, 0xFF);

    assert(graphics.renderScreenPixel(0, 239, color) == true);
    assertRgbColor(color, 0x00, 0xFF, 0x00);

    assert(graphics.renderScreenPixel(16 * 16 + 8, 16 + 8, color) == true);
    assertRgbColor(color, 0xFF, 0xFF, 0x00);

    printf("[PASS] First graphics demo draws static image test passed\n");
}

void testKeyboardDemoMovesObject() {
    CPU cpu;

    loadKeyboardDemo(cpu);

    GraphicsMemory graphics(cpu.getMemory());

    assert(graphics.readTileIndex(2, 12) == 9);

    cpu.setKeyboardKey(CPU::ZX16_KEY_RIGHT);
    assert(updateKeyboardDemo(cpu) == true);

    assert(keyboardDemoObjectCol == 3);
    assert(keyboardDemoObjectRow == 12);
    assert(graphics.readTileIndex(2, 12) == 1);
    assert(graphics.readTileIndex(3, 12) == 9);

    cpu.setKeyboardKey(CPU::ZX16_KEY_UP);
    assert(updateKeyboardDemo(cpu) == true);

    assert(keyboardDemoObjectCol == 3);
    assert(keyboardDemoObjectRow == 11);
    assert(graphics.readTileIndex(3, 12) == 1);
    assert(graphics.readTileIndex(3, 11) == 9);

    cpu.clearKeyboardKey();
    assert(updateKeyboardDemo(cpu) == false);

    printf("[PASS] Keyboard demo moves object test passed\n");
}

void testAudioDemoPlaysSoundWithKeyPress() {
    CPU cpu;

    cpu.setKeyboardKey(CPU::ZX16_KEY_SPACE);

    assert(updateAudioDemo(cpu) == true);
    assert(cpu.hasPendingTone() == true);
    assert(cpu.getToneFrequency() == 440);
    assert(cpu.getToneDurationMs() == 200);

    cpu.clearToneRequest();
    cpu.setKeyboardKey(CPU::ZX16_KEY_LEFT);

    assert(updateAudioDemo(cpu) == false);
    assert(cpu.hasPendingTone() == false);

    printf("[PASS] Audio demo plays sound with key press test passed\n");
}

void testAllEcallServicesTogether() {
    CPU cpu;

    unsigned short address = 0x7400;

    cpu.setPC(address);

    cpu.getMemory().write16(address + 0, makeSys(0x000));
    cpu.getMemory().write16(address + 2, makeSys(0x001));
    cpu.getMemory().write16(address + 4, makeSys(0x010));
    cpu.getMemory().write16(address + 6, makeSys(0x011));
    cpu.getMemory().write16(address + 8, makeSys(0x012));
    cpu.getMemory().write16(address + 10, makeSys(0x020));
    cpu.getMemory().write16(address + 12, makeSys(0x021));
    cpu.getMemory().write16(address + 14, makeSys(0x030));
    cpu.getMemory().write16(address + 16, makeSys(0x040));
    cpu.getMemory().write16(address + 18, makeSys(0x041));
    cpu.getMemory().write16(address + 20, makeSys(0x042));
    cpu.getMemory().write16(address + 22, makeSys(0x050));
    cpu.getMemory().write16(address + 24, makeSys(0x051));
    cpu.getMemory().write16(address + 26, makeSys(0x3FF));

    cpu.getRegisters().setRegister(6, (unsigned short)-7);
    cpu.step();
    assert(strcmp(cpu.getOutput(), "-7") == 0);

    cpu.getRegisters().setRegister(6, ' ');
    cpu.step();
    assert(strcmp(cpu.getOutput(), "-7 ") == 0);

    cpu.setInput("ZX16\n");
    cpu.getRegisters().setRegister(6, 0x6000);
    cpu.getRegisters().setRegister(7, 16);
    cpu.step();
    assert(cpu.getMemory().read8(0x6000) == 'Z');
    assert(cpu.getMemory().read8(0x6001) == 'X');
    assert(cpu.getMemory().read8(0x6002) == '1');
    assert(cpu.getMemory().read8(0x6003) == '6');
    assert(cpu.getMemory().read8(0x6004) == 0);

    cpu.setInput("42");
    cpu.step();
    assert(cpu.getRegisters().getRegister(6) == 42);

    cpu.getRegisters().setRegister(6, 0x6000);
    cpu.step();
    assert(strcmp(cpu.getOutput(), "-7 ZX16") == 0);

    cpu.getRegisters().setRegister(6, 0x1234);
    cpu.step();
    assert(cpu.getRngState() == 0x1234);

    cpu.step();
    assert(cpu.getRegisters().getRegister(6) == 0x3830);

    cpu.setKeyboardKey(CPU::ZX16_KEY_RIGHT);
    cpu.step();
    assert(cpu.getRegisters().getRegister(6) == CPU::ZX16_KEY_RIGHT);

    cpu.getRegisters().setRegister(6, 440);
    cpu.getRegisters().setRegister(7, 200);
    cpu.step();
    assert(cpu.hasPendingTone() == true);
    assert(cpu.getToneFrequency() == 440);
    assert(cpu.getToneDurationMs() == 200);

    cpu.getRegisters().setRegister(6, 25);
    cpu.step();
    assert(cpu.getVolumePercent() == 25);

    cpu.step();
    assert(cpu.hasPendingTone() == false);
    assert(cpu.hasPendingStopAudio() == true);

    cpu.step();
    assert(strstr(cpu.getOutput(), "REGS\n") != 0);

    cpu.getRegisters().setRegister(6, 0x6000);
    cpu.getRegisters().setRegister(7, 5);
    cpu.step();
    assert(strstr(cpu.getOutput(), "MEM\n") != 0);
    assert(strstr(cpu.getOutput(), "5A 58 31 36 00") != 0);

    cpu.step();
    assert(cpu.isHalted() == true);
    assert(cpu.getPC() == address + 28);

    printf("[PASS] All ECALL services integration test passed\n");
}

void resetWholeSimulator(Gui& gui, CPU& cpu, bool& running, int& runDelay) {
    loadGuiDemoProgram(cpu);

    gui.resetDebugState();

    running = false;
    runDelay = 0;
}

void handleGuiAction(
    Gui& gui,
    CPU& cpu,
    GuiAction action,
    bool& running,
    bool& runToCursorActive,
    int& runDelay
) {
    if (action == GUI_ACTION_RUN_PAUSE) {
        if (!cpu.isHalted()) {
            running = !running;
            runToCursorActive = false;
        }
    }

    if (action == GUI_ACTION_STEP) {
        if (!running && !cpu.isHalted()) {
            cpu.step();
            runToCursorActive = false;
        }
    }

    if (action == GUI_ACTION_RUN_TO_CURSOR) {
        if (!cpu.isHalted() && gui.hasCursorAddress()) {
            running = cpu.getPC() != gui.getCursorAddress();
            runToCursorActive = running;
            runDelay = 0;
        }
    }

    if (action == GUI_ACTION_RESET) {
        resetWholeSimulator(gui, cpu, running, runDelay);
        runToCursorActive = false;
        return;
    }

    if (cpu.isHalted()) {
        running = false;
        runToCursorActive = false;
    }
}

void testEntireSimulatorReset() {
    Gui gui;
    CPU cpu;

    bool running = true;
    int runDelay = 17;

    loadGuiDemoProgram(cpu);

    unsigned short expectedFirstWord = cpu.getMemory().read16(0x0020);
    unsigned short expectedSecondWord = cpu.getMemory().read16(0x0022);
    unsigned char expectedPalette0 = cpu.getMemory().read8(0xFA00);
    unsigned char expectedPalette1 = cpu.getMemory().read8(0xFA01);

    cpu.setPC(0x6000);

    for (int i = 0; i < 8; i++) {
        cpu.getRegisters().setRegister(i, (unsigned short)(0x1110 + i));
    }

    cpu.setSP(0xABCD);

    cpu.getMemory().write16(0x0020, 0xAAAA);
    cpu.getMemory().write16(0x0022, 0xBBBB);
    cpu.getMemory().write8(0x9000, 0x55);
    cpu.getMemory().write8(0xFA00, 0xCC);
    cpu.getMemory().write8(0xFA01, 0xDD);

    cpu.dumpRegisters();
    assert(strcmp(cpu.getOutput(), "") != 0);

    cpu.setInput("12345\n");
    assert(strcmp(cpu.getInput(), "12345\n") == 0);

    cpu.seedRng(0x1234);
    assert(cpu.getRngState() == 0x1234);

    cpu.setKeyboardKey(CPU::ZX16_KEY_LEFT);
    assert(cpu.getKeyboardKey() == CPU::ZX16_KEY_LEFT);

    cpu.requestTone(440, 250);
    assert(cpu.hasPendingTone() == true);

    cpu.requestStopAudio();
    assert(cpu.hasPendingStopAudio() == true);

    cpu.setVolumePercent(25);
    assert(cpu.getVolumePercent() == 25);

    cpu.setBreakpoint(0x0020);
    assert(cpu.getBreakpointCount() == 1);

    gui.setCursorAddress(0x0040);
    assert(gui.hasCursorAddress() == true);

    cpu.getMemory().write16(0x6000, makeSys(0x3FF));
    cpu.setPC(0x6000);
    cpu.step();
    assert(cpu.isHalted() == true);

    resetWholeSimulator(gui, cpu, running, runDelay);

    assert(running == false);
    assert(runDelay == 0);

    assert(cpu.isHalted() == false);
    assert(cpu.getPC() == 0x0020);
    assert(cpu.getSP() == 0xEFFE);

    for (int i = 0; i < 8; i++) {
        if (i == 2) {
            assert(cpu.getRegisters().getRegister(i) == 0xEFFE);
        }
        else {
            assert(cpu.getRegisters().getRegister(i) == 0x0000);
        }
    }

    assert(cpu.getMemory().read16(0x0020) == expectedFirstWord);
    assert(cpu.getMemory().read16(0x0022) == expectedSecondWord);

    assert(cpu.getMemory().read8(0x9000) == 0x00);
    assert(cpu.getMemory().read8(0xFA00) == expectedPalette0);
    assert(cpu.getMemory().read8(0xFA01) == expectedPalette1);

    assert(strcmp(cpu.getOutput(), "") == 0);
    assert(strcmp(cpu.getInput(), "") == 0);

    assert(cpu.getRngState() == CPU::DEFAULT_RNG_SEED);
    assert(cpu.getKeyboardKey() == CPU::ZX16_KEY_NONE);

    assert(cpu.hasPendingTone() == false);
    assert(cpu.hasPendingStopAudio() == false);
    assert(cpu.getVolumePercent() == CPU::DEFAULT_VOLUME_PERCENT);

    assert(cpu.getBreakpointCount() == 0);
    assert(cpu.hasBreakpointHit() == false);

    assert(gui.hasCursorAddress() == false);

    printf("[PASS] Entire simulator reset test passed\n");
}

void testGuiStepExecutesOneInstruction() {
    CPU cpu;
    Gui gui;

    bool running = false;
    bool runToCursorActive = false;
    int runDelay = 0;

    cpu.reset();

    cpu.getMemory().write16(0x0020, makeI(5, 6, 7));
    cpu.getMemory().write16(0x0022, makeI(6, 7, 7));
    cpu.getMemory().write16(0x0024, makeSys(0x3FF));

    assert(cpu.getPC() == 0x0020);
    assert(cpu.getRegisters().getRegister(6) == 0x0000);
    assert(cpu.getRegisters().getRegister(7) == 0x0000);

    handleGuiAction(gui, cpu, GUI_ACTION_STEP, running, runToCursorActive, runDelay);

    assert(cpu.getPC() == 0x0022);
    assert(cpu.getRegisters().getRegister(6) == 5);
    assert(cpu.getRegisters().getRegister(7) == 0x0000);

    handleGuiAction(gui, cpu, GUI_ACTION_STEP, running, runToCursorActive, runDelay);

    assert(cpu.getPC() == 0x0024);
    assert(cpu.getRegisters().getRegister(6) == 5);
    assert(cpu.getRegisters().getRegister(7) == 6);

    printf("[PASS] GUI step executes one instruction test passed\n");
}

void testGuiRegisterEditingModifiesRegisters() {
    Gui gui;
    CPU cpu;

    unsigned short value = 0;

    cpu.reset();

    assert(Gui::isValidRegisterEditIndex(0) == true);
    assert(Gui::isValidRegisterEditIndex(7) == true);
    assert(Gui::isValidRegisterEditIndex(-1) == false);
    assert(Gui::isValidRegisterEditIndex(8) == false);

    assert(Gui::parseHex16("0000", value) == true);
    assert(value == 0x0000);

    assert(Gui::parseHex16("1234", value) == true);
    assert(value == 0x1234);

    assert(Gui::parseHex16("0xABCD", value) == true);
    assert(value == 0xABCD);

    assert(Gui::parseHex16("beef", value) == true);
    assert(value == 0xBEEF);

    assert(Gui::parseHex16("", value) == false);
    assert(Gui::parseHex16("10000", value) == false);
    assert(Gui::parseHex16("12G4", value) == false);

    assert(gui.hasSelectedRegister() == false);
    assert(gui.getSelectedRegisterIndex() == -1);

    assert(gui.editRegister(cpu, 0, 0x1111) == true);
    assert(cpu.getRegisters().getRegister(0) == 0x1111);

    assert(gui.editRegister(cpu, 1, 0x2222) == true);
    assert(cpu.getRegisters().getRegister(1) == 0x2222);

    assert(gui.editRegister(cpu, 2, 0xD000) == true);
    assert(cpu.getRegisters().getRegister(2) == 0xD000);
    assert(cpu.getSP() == 0xD000);

    assert(gui.editRegister(cpu, 7, 0x7777) == true);
    assert(cpu.getRegisters().getRegister(7) == 0x7777);

    assert(gui.editRegister(cpu, -1, 0x9999) == false);
    assert(gui.editRegister(cpu, 8, 0x9999) == false);

    printf("[PASS] GUI register editing modifies registers test passed\n");
}

void testGuiMemoryEditingModifiesRam() {
    Gui gui;
    CPU cpu;

    unsigned char byteValue = 0;
    unsigned short wordValue = 0;

    cpu.reset();

    assert(Gui::parseHex8("00", byteValue) == true);
    assert(byteValue == 0x00);

    assert(Gui::parseHex8("7F", byteValue) == true);
    assert(byteValue == 0x7F);

    assert(Gui::parseHex8("0xAB", byteValue) == true);
    assert(byteValue == 0xAB);

    assert(Gui::parseHex8("ff", byteValue) == true);
    assert(byteValue == 0xFF);

    assert(Gui::parseHex8("", byteValue) == false);
    assert(Gui::parseHex8("100", byteValue) == false);
    assert(Gui::parseHex8("GG", byteValue) == false);

    assert(gui.hasSelectedMemoryAddress() == false);
    assert(gui.getSelectedMemoryAddress() == 0);

    assert(gui.editMemoryByte(cpu, 0x0040, 0xAB) == true);
    assert(cpu.getMemory().read8(0x0040) == 0xAB);

    assert(gui.editMemoryByte(cpu, 0x0041, 0xCD) == true);
    assert(cpu.getMemory().read8(0x0041) == 0xCD);

    assert(gui.editMemoryWord(cpu, 0x0042, 0xBEEF) == true);
    assert(cpu.getMemory().read16(0x0042) == 0xBEEF);
    assert(cpu.getMemory().read8(0x0042) == 0xEF);
    assert(cpu.getMemory().read8(0x0043) == 0xBE);

    cpu.getMemory().write8(0x0045, 0x11);
    cpu.getMemory().write8(0x0046, 0x22);

    assert(gui.editMemoryWord(cpu, 0x0045, 0x1234) == false);
    assert(cpu.getMemory().read8(0x0045) == 0x11);
    assert(cpu.getMemory().read8(0x0046) == 0x22);

    cpu.getMemory().write8(0xFFFF, 0x55);
    cpu.getMemory().write8(0x0000, 0x66);

    assert(gui.editMemoryWord(cpu, 0xFFFF, 0x9999) == false);
    assert(cpu.getMemory().read8(0xFFFF) == 0x55);
    assert(cpu.getMemory().read8(0x0000) == 0x66);

    assert(Gui::parseHex16("BEEF", wordValue) == true);
    assert(wordValue == 0xBEEF);

    printf("[PASS] GUI memory editing modifies RAM test passed\n");
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

void testBreakpointStopsExecution() {
    CPU cpu;

    unsigned short randomWord = makeSys(0x021);

    assert(CPU::isValidBreakpointAddress(0x7000) == true);
    assert(CPU::isValidBreakpointAddress(0x7002) == true);
    assert(CPU::isValidBreakpointAddress(0x7001) == false);

    assert(cpu.getBreakpointCount() == 0);
    assert(cpu.hasBreakpointHit() == false);
    assert(cpu.getBreakpointHitAddress() == 0);

    assert(cpu.setBreakpoint(0x7002) == true);
    assert(cpu.getBreakpointCount() == 1);
    assert(cpu.hasBreakpoint(0x7002) == true);

    assert(cpu.setBreakpoint(0x7002) == true);
    assert(cpu.getBreakpointCount() == 1);

    assert(cpu.setBreakpoint(0x7003) == false);
    assert(cpu.getBreakpointCount() == 1);

    cpu.setPC(0x7000);
    cpu.seedRng(0x1234);

    cpu.getMemory().write16(0x7000, randomWord);
    cpu.getMemory().write16(0x7002, randomWord);

    bool executed = cpu.stepWithBreakpoints();

    assert(executed == true);
    assert(cpu.hasBreakpointHit() == false);
    assert(cpu.getPC() == 0x7002);
    assert(cpu.getRegisters().getRegister(6) == 0x3830);

    executed = cpu.stepWithBreakpoints();

    assert(executed == false);
    assert(cpu.hasBreakpointHit() == true);
    assert(cpu.getBreakpointHitAddress() == 0x7002);
    assert(cpu.getPC() == 0x7002);
    assert(cpu.getRegisters().getRegister(6) == 0x3830);

    assert(cpu.clearBreakpoint(0x7002) == true);
    assert(cpu.getBreakpointCount() == 0);
    assert(cpu.hasBreakpoint(0x7002) == false);

    cpu.clearBreakpointHit();

    assert(cpu.hasBreakpointHit() == false);

    executed = cpu.stepWithBreakpoints();

    assert(executed == true);
    assert(cpu.getPC() == 0x7004);
    assert(cpu.getRegisters().getRegister(6) == 0x0020);

    assert(cpu.toggleBreakpoint(0x7004) == true);
    assert(cpu.hasBreakpoint(0x7004) == true);
    assert(cpu.getBreakpointCount() == 1);

    assert(cpu.toggleBreakpoint(0x7004) == true);
    assert(cpu.hasBreakpoint(0x7004) == false);
    assert(cpu.getBreakpointCount() == 0);

    assert(cpu.setBreakpoint(0x7006) == true);
    assert(cpu.getBreakpointCount() == 1);

    cpu.clearBreakpoints();

    assert(cpu.getBreakpointCount() == 0);
    assert(cpu.hasBreakpoint(0x7006) == false);
    assert(cpu.hasBreakpointHit() == false);

    printf("[PASS] Breakpoint stops execution test passed\n");
}

void testCurrentPcHighlightMovesCorrectly() {
    CPU cpu;

    unsigned short randomWord = makeSys(0x021);

    assert(Gui::getMemoryViewerBaseAddress(0x0020) == 0x0020);
    assert(Gui::getMemoryViewerBaseAddress(0x0022) == 0x0020);
    assert(Gui::getMemoryViewerBaseAddress(0x002E) == 0x0020);
    assert(Gui::getMemoryViewerBaseAddress(0x0030) == 0x0030);

    assert(Gui::isCurrentInstructionByte(0x7000, 0x7000) == true);
    assert(Gui::isCurrentInstructionByte(0x7001, 0x7000) == true);
    assert(Gui::isCurrentInstructionByte(0x7002, 0x7000) == false);

    cpu.setPC(0x7000);
    cpu.seedRng(0x1234);

    cpu.getMemory().write16(0x7000, randomWord);
    cpu.getMemory().write16(0x7002, randomWord);

    assert(Gui::getMemoryViewerBaseAddress(cpu.getPC()) == 0x7000);
    assert(Gui::isCurrentInstructionByte(0x7000, cpu.getPC()) == true);
    assert(Gui::isCurrentInstructionByte(0x7001, cpu.getPC()) == true);
    assert(Gui::isCurrentInstructionByte(0x7002, cpu.getPC()) == false);

    cpu.step();

    assert(cpu.getPC() == 0x7002);
    assert(Gui::getMemoryViewerBaseAddress(cpu.getPC()) == 0x7000);
    assert(Gui::isCurrentInstructionByte(0x7000, cpu.getPC()) == false);
    assert(Gui::isCurrentInstructionByte(0x7001, cpu.getPC()) == false);
    assert(Gui::isCurrentInstructionByte(0x7002, cpu.getPC()) == true);
    assert(Gui::isCurrentInstructionByte(0x7003, cpu.getPC()) == true);

    cpu.step();

    assert(cpu.getPC() == 0x7004);
    assert(Gui::isCurrentInstructionByte(0x7002, cpu.getPC()) == false);
    assert(Gui::isCurrentInstructionByte(0x7003, cpu.getPC()) == false);
    assert(Gui::isCurrentInstructionByte(0x7004, cpu.getPC()) == true);
    assert(Gui::isCurrentInstructionByte(0x7005, cpu.getPC()) == true);

    printf("[PASS] Current PC highlight moves correctly test passed\n");
}

void testCurrentInstructionDisplayed() {
    CPU cpu;

    char text[128];
    char expected[128];

    unsigned short randomWord = makeSys(0x021);
    unsigned short regsDumpWord = makeSys(0x050);
    unsigned short haltWord = makeSys(0x3FF);

    char disassembled[80];

    Gui::disassembleInstruction(randomWord, disassembled, 80);
    assert(strcmp(disassembled, "SYS random") == 0);

    Gui::disassembleInstruction(regsDumpWord, disassembled, 80);
    assert(strcmp(disassembled, "SYS regs_dump") == 0);

    Gui::disassembleInstruction(haltWord, disassembled, 80);
    assert(strcmp(disassembled, "SYS halt") == 0);

    cpu.setPC(0x7900);
    cpu.getMemory().write16(0x7900, randomWord);
    cpu.getMemory().write16(0x7902, regsDumpWord);
    cpu.getMemory().write16(0x7904, haltWord);

    Gui::buildCurrentInstructionText(cpu, text, 128);
    sprintf(expected, "0x7900: %04X  SYS random", randomWord);
    assert(strcmp(text, expected) == 0);

    cpu.step();

    Gui::buildCurrentInstructionText(cpu, text, 128);
    sprintf(expected, "0x7902: %04X  SYS regs_dump", regsDumpWord);
    assert(strcmp(text, expected) == 0);

    cpu.step();

    Gui::buildCurrentInstructionText(cpu, text, 128);
    sprintf(expected, "0x7904: %04X  SYS halt", haltWord);
    assert(strcmp(text, expected) == 0);

    printf("[PASS] Current instruction displayed test passed\n");
}

int main() {
    testMemory();
    testGraphicsMemoryManager();
    testGraphicsMemoryProtection();
    testTileMapAccess();
    testTileDefinitionAccess();
    testPaletteMemory();
    testRgb332Decode();
    testRgb332ToRgb888Expansion();
    testTilePixelExtraction();
    testTileRenderer();
    testCompleteOneTileRenderer();
    testFullScreenRenderer();
    testRendererLiveUpdate();
    testRendererConnectedToVram();
    testFrameTimingStableRefresh();
    testFirstGraphicsDemoDrawsStaticImage();
    testKeyboardDemoMovesObject();
    testAudioDemoPlaysSoundWithKeyPress();

    testEcallPrintStringExecution();
    testEcallReadIntExecution();
    testEcallReadStringExecution();

    testRngStateReset();
    testEcallSeedRngExecution();
    testEcallRandomXorshiftExecution();

    testKeyboardEcallDetectKeyPress();
    testRaylibKeyboardMappingDirections();

    testEcallPlayToneExecution();
    testEcallSetVolumeExecution();
    testEcallStopAudioExecution();

    testEcallRegsDumpExecution();
    testEcallMemDumpExecution();
    testAllEcallServicesTogether();

    testConsoleUpdatesCorrectly();
    testBreakpointStopsExecution();

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
    testEntireSimulatorReset();
    testGuiRegisterEditingModifiesRegisters();
    testGuiMemoryEditingModifiesRam();
    testMemoryViewerMatchesRam();
    testFinalBinPrograms();

    testCurrentPcHighlightMovesCorrectly();
    testCurrentInstructionDisplayed();
    testStepOverSkipsFunctionBody();



    CPU guiCpu;
    loadGuiDemoProgram(guiCpu);

    Gui gui;
    gui.open();

    bool running = false;
    bool runToCursorActive = false;
    int frameNumber = 0;
    int runDelay = 0;
    int keyboardMoveDelay = 0;
    unsigned short lastAudioDemoKey = CPU::ZX16_KEY_NONE;

    while (!gui.shouldClose()) {
        frameNumber++;

        GuiAction action = gui.draw(
            "All tests passed",
            guiCpu.getOutput(),
            frameNumber,
            running,
            guiCpu
        );

        handleGuiAction(
    gui,
    guiCpu,
    action,
    running,
    runToCursorActive,
            runDelay
        );

        unsigned short currentAudioDemoKey = guiCpu.getKeyboardKey();

        if (currentAudioDemoKey == CPU::ZX16_KEY_SPACE && lastAudioDemoKey != CPU::ZX16_KEY_SPACE) {
            updateAudioDemo(guiCpu);
        }

        lastAudioDemoKey = currentAudioDemoKey;

        if (guiCpu.getKeyboardKey() == CPU::ZX16_KEY_NONE) {
            keyboardMoveDelay = 0;
        }
        else {
            keyboardMoveDelay++;

            if (keyboardMoveDelay >= 6) {
                updateKeyboardDemo(guiCpu);
                keyboardMoveDelay = 0;
            }
        }

        if (running && !guiCpu.isHalted()) {
            runDelay++;

            if (runDelay >= 30) {
                bool executed = guiCpu.stepWithBreakpoints();
                if (runToCursorActive && guiCpu.getPC() == gui.getCursorAddress()) {
                    running = false;
                    runToCursorActive = false;
                }

                if (!executed && guiCpu.hasBreakpointHit()) {
                    running = false;
                    runToCursorActive = false;
                }



                runDelay = 0;
            }
        }

        if (guiCpu.isHalted()) {
            running = false;
            runToCursorActive = false;
        }
    }

    gui.close();


    return 0;
}