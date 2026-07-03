#ifndef GRAPHICS_MEMORY_H
#define GRAPHICS_MEMORY_H

#include "memory.h"

class GraphicsMemory {
public:
    static constexpr int SCREEN_WIDTH = 320;
    static constexpr int SCREEN_HEIGHT = 240;

    static constexpr int TILE_SIZE = 16;
    static constexpr int TILE_COLUMNS = 20;
    static constexpr int TILE_ROWS = 15;
    static constexpr int TILE_COUNT = 16;
    static constexpr int TILE_BYTES = 128;

    static constexpr unsigned short GRAPHICS_BASE = 0xF000;
    static constexpr unsigned short GRAPHICS_END = 0xFA0F;

    static constexpr unsigned short TILE_MAP_BASE = 0xF000;
    static constexpr int TILE_MAP_USED_SIZE = 300;
    static constexpr int TILE_MAP_REGION_SIZE = 512;
    static constexpr unsigned short TILE_MAP_USED_END = 0xF12B;
    static constexpr unsigned short TILE_MAP_END = 0xF1FF;

    static constexpr unsigned short TILE_DEFINITION_BASE = 0xF200;
    static constexpr int TILE_DEFINITION_SIZE = 2048;
    static constexpr unsigned short TILE_DEFINITION_END = 0xF9FF;

    static constexpr unsigned short PALETTE_BASE = 0xFA00;
    static constexpr int PALETTE_SIZE = 16;
    static constexpr unsigned short PALETTE_END = 0xFA0F;

    static constexpr unsigned short RESERVED_MMIO_BASE = 0xFA10;
    static constexpr unsigned short RESERVED_MMIO_END = 0xFFFF;

    static constexpr unsigned short PROJECT_STACK_RESET = 0xEFFE;
    static constexpr unsigned short INVALID_ADDRESS = 0xFFFF;

    GraphicsMemory(Memory& memoryReference);

    static bool isValidTileCell(int col, int row);
    static bool isValidTileIndex(int tileIndex);
    static bool isValidTilePixel(int x, int y);
    static bool isValidPaletteIndex(int paletteIndex);
    static bool isValidTileByteOffset(int byteOffset);

    static unsigned short getTileMapAddress(int col, int row);
    static unsigned short getTileDefinitionBase(int tileIndex);
    static unsigned short getTilePixelByteAddress(int tileIndex, int x, int y);
    static unsigned short getPaletteAddress(int paletteIndex);

    static bool isTileMapUsedAddress(unsigned short address);
    static bool isTileMapRegionAddress(unsigned short address);
    static bool isTileDefinitionAddress(unsigned short address);
    static bool isPaletteAddress(unsigned short address);
    static bool isGraphicsAddress(unsigned short address);
    static bool isReservedMmioAddress(unsigned short address);

    bool writeTileIndex(int col, int row, unsigned char tileIndex);
    unsigned char readTileIndex(int col, int row);

    bool writeTileDefinitionByte(int tileIndex, int byteOffset, unsigned char value);
    unsigned char readTileDefinitionByte(int tileIndex, int byteOffset);

    bool writePaletteColor(int paletteIndex, unsigned char rgb332);
    unsigned char readPaletteColor(int paletteIndex);

private:
    Memory& memory;
};

#endif