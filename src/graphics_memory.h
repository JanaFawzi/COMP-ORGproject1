#ifndef GRAPHICS_MEMORY_H
#define GRAPHICS_MEMORY_H

#include "memory.h"

struct Rgb332Color {
    unsigned char red3;
    unsigned char green3;
    unsigned char blue2;
};

struct Rgb888Color {
    unsigned char red8;
    unsigned char green8;
    unsigned char blue8;
};

class GraphicsMemory {
public:
    static constexpr int SCREEN_WIDTH = 320;
    static constexpr int SCREEN_HEIGHT = 240;

    static constexpr int TILE_SIZE = 16;
    static constexpr int TILE_COLUMNS = 20;
    static constexpr int TILE_ROWS = 15;
    static constexpr int TILE_COUNT = 16;
    static constexpr int TILE_BYTES = 128;
    static constexpr int TILE_PIXEL_COUNT = 256;

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

    static constexpr int REGION_NONE = 0;
    static constexpr int REGION_TILE_MAP_USED = 1;
    static constexpr int REGION_TILE_MAP_PADDING = 2;
    static constexpr int REGION_TILE_DEFINITION = 3;
    static constexpr int REGION_PALETTE = 4;
    static constexpr int REGION_RESERVED_MMIO = 5;

    GraphicsMemory(Memory& memoryReference);

    static bool isValidTileCell(int col, int row);
    static bool isValidTileMapOffset(int offset);
    static bool isValidTileIndex(int tileIndex);
    static bool isValidTilePixel(int x, int y);
    static bool isValidPaletteIndex(int paletteIndex);
    static bool isValidTileByteOffset(int byteOffset);

    static bool isValidRgb3(int value);
    static bool isValidRgb2(int value);

    static int getTileMapOffset(int col, int row);
    static unsigned short getTileMapAddress(int col, int row);
    static unsigned short getTileMapAddressByOffset(int offset);

    static int getTilePixelNumber(int x, int y);
    static int getTilePixelByteOffset(int x, int y);
    static bool isLowNibblePixel(int x);

    static unsigned char extractLowNibble(unsigned char value);
    static unsigned char extractHighNibble(unsigned char value);
    static unsigned char extractPaletteIndexFromTileByte(unsigned char tileByte, int x);

    static unsigned short getTileDefinitionBase(int tileIndex);
    static unsigned short getTilePixelByteAddress(int tileIndex, int x, int y);
    static unsigned short getPaletteAddress(int paletteIndex);

    static unsigned char makeRgb332(int red3, int green3, int blue2);
    static unsigned char getRgb332Red3(unsigned char rgb332);
    static unsigned char getRgb332Green3(unsigned char rgb332);
    static unsigned char getRgb332Blue2(unsigned char rgb332);

    static Rgb332Color decodeRgb332(unsigned char rgb332);

    static unsigned char expandRgb3To8(unsigned char value3);
    static unsigned char expandRgb2To8(unsigned char value2);

    static void expandRgb332ToRgb888(
        unsigned char rgb332,
        unsigned char& red8,
        unsigned char& green8,
        unsigned char& blue8
    );

    static Rgb888Color expandRgb332ToRgb888Color(unsigned char rgb332);

    static bool isTileMapUsedAddress(unsigned short address);
    static bool isTileMapRegionAddress(unsigned short address);
    static bool isTileDefinitionAddress(unsigned short address);
    static bool isPaletteAddress(unsigned short address);
    static bool isGraphicsAddress(unsigned short address);
    static bool isVramAddress(unsigned short address);
    static bool isReservedMmioAddress(unsigned short address);
    static bool isSafeVramRange(unsigned short address, int byteCount);
    static bool isEvenAddress(unsigned short address);
    static int getGraphicsRegion(unsigned short address);

    bool writeVram8(unsigned short address, unsigned char value);
    bool readVram8(unsigned short address, unsigned char& value);

    bool writeVram16(unsigned short address, unsigned short value);
    bool readVram16(unsigned short address, unsigned short& value);

    bool writeTileIndex(int col, int row, unsigned char tileIndex);
    unsigned char readTileIndex(int col, int row);
    bool readTileIndexChecked(int col, int row, unsigned char& tileIndex);

    bool writeTileIndexByOffset(int offset, unsigned char tileIndex);
    bool readTileIndexByOffset(int offset, unsigned char& tileIndex);

    bool clearTileMap();
    bool fillTileMap(unsigned char tileIndex);

    bool writeTileDefinitionByte(int tileIndex, int byteOffset, unsigned char value);
    unsigned char readTileDefinitionByte(int tileIndex, int byteOffset);
    bool readTileDefinitionByteChecked(int tileIndex, int byteOffset, unsigned char& value);

    bool writeTileDefinition(int tileIndex, const unsigned char data[], int byteCount);
    bool readTileDefinition(int tileIndex, unsigned char data[], int byteCount);
    bool clearTileDefinition(int tileIndex, unsigned char paletteIndex);

    bool writeTilePixel(int tileIndex, int x, int y, unsigned char paletteIndex);
    bool readTilePixel(int tileIndex, int x, int y, unsigned char& paletteIndex);
    bool readTilePixelPaletteIndex(int tileIndex, int x, int y, unsigned char& paletteIndex);

    bool renderTilePixel(int tileIndex, int x, int y, Rgb888Color& color);
    bool renderTile(int tileIndex, Rgb888Color pixels[], int pixelCount);
    bool renderTilePaletteIndices(int tileIndex, unsigned char pixels[], int pixelCount);

    bool writePaletteColor(int paletteIndex, unsigned char rgb332);
    unsigned char readPaletteColor(int paletteIndex);
    bool readPaletteColorChecked(int paletteIndex, unsigned char& rgb332);

    bool writePaletteRgb(int paletteIndex, int red3, int green3, int blue2);

    bool readPaletteRgb332(int paletteIndex, Rgb332Color& color);
    bool readPaletteRgb888Color(int paletteIndex, Rgb888Color& color);

    bool readPaletteRgb888(
        int paletteIndex,
        unsigned char& red8,
        unsigned char& green8,
        unsigned char& blue8
    );

    bool fillPalette(unsigned char rgb332);
    bool clearPalette();

private:
    Memory& memory;
};

#endif