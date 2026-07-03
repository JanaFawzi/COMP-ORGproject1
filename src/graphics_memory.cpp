#include "graphics_memory.h"

GraphicsMemory::GraphicsMemory(Memory& memoryReference)
    : memory(memoryReference) {
}

bool GraphicsMemory::isValidTileCell(int col, int row) {
    if (col < 0 || col >= TILE_COLUMNS) {
        return false;
    }

    if (row < 0 || row >= TILE_ROWS) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isValidTileIndex(int tileIndex) {
    if (tileIndex < 0 || tileIndex >= TILE_COUNT) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isValidTilePixel(int x, int y) {
    if (x < 0 || x >= TILE_SIZE) {
        return false;
    }

    if (y < 0 || y >= TILE_SIZE) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isValidPaletteIndex(int paletteIndex) {
    if (paletteIndex < 0 || paletteIndex >= PALETTE_SIZE) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isValidTileByteOffset(int byteOffset) {
    if (byteOffset < 0 || byteOffset >= TILE_BYTES) {
        return false;
    }

    return true;
}

unsigned short GraphicsMemory::getTileMapAddress(int col, int row) {
    if (!isValidTileCell(col, row)) {
        return INVALID_ADDRESS;
    }

    return (unsigned short)(TILE_MAP_BASE + row * TILE_COLUMNS + col);
}

unsigned short GraphicsMemory::getTileDefinitionBase(int tileIndex) {
    if (!isValidTileIndex(tileIndex)) {
        return INVALID_ADDRESS;
    }

    return (unsigned short)(TILE_DEFINITION_BASE + tileIndex * TILE_BYTES);
}

unsigned short GraphicsMemory::getTilePixelByteAddress(int tileIndex, int x, int y) {
    if (!isValidTileIndex(tileIndex)) {
        return INVALID_ADDRESS;
    }

    if (!isValidTilePixel(x, y)) {
        return INVALID_ADDRESS;
    }

    int pixelNumber = y * TILE_SIZE + x;
    int byteOffset = pixelNumber / 2;

    return (unsigned short)(getTileDefinitionBase(tileIndex) + byteOffset);
}

unsigned short GraphicsMemory::getPaletteAddress(int paletteIndex) {
    if (!isValidPaletteIndex(paletteIndex)) {
        return INVALID_ADDRESS;
    }

    return (unsigned short)(PALETTE_BASE + paletteIndex);
}

bool GraphicsMemory::isTileMapUsedAddress(unsigned short address) {
    if (address < TILE_MAP_BASE || address > TILE_MAP_USED_END) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isTileMapRegionAddress(unsigned short address) {
    if (address < TILE_MAP_BASE || address > TILE_MAP_END) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isTileDefinitionAddress(unsigned short address) {
    if (address < TILE_DEFINITION_BASE || address > TILE_DEFINITION_END) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isPaletteAddress(unsigned short address) {
    if (address < PALETTE_BASE || address > PALETTE_END) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isGraphicsAddress(unsigned short address) {
    if (address < GRAPHICS_BASE || address > GRAPHICS_END) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isReservedMmioAddress(unsigned short address) {
    if (address < RESERVED_MMIO_BASE || address > RESERVED_MMIO_END) {
        return false;
    }

    return true;
}

bool GraphicsMemory::writeTileIndex(int col, int row, unsigned char tileIndex) {
    if (!isValidTileCell(col, row)) {
        return false;
    }

    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    memory.write8(getTileMapAddress(col, row), tileIndex);
    return true;
}

unsigned char GraphicsMemory::readTileIndex(int col, int row) {
    if (!isValidTileCell(col, row)) {
        return 0;
    }

    return memory.read8(getTileMapAddress(col, row));
}

bool GraphicsMemory::writeTileDefinitionByte(int tileIndex, int byteOffset, unsigned char value) {
    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    if (!isValidTileByteOffset(byteOffset)) {
        return false;
    }

    unsigned short address = (unsigned short)(getTileDefinitionBase(tileIndex) + byteOffset);
    memory.write8(address, value);

    return true;
}

unsigned char GraphicsMemory::readTileDefinitionByte(int tileIndex, int byteOffset) {
    if (!isValidTileIndex(tileIndex)) {
        return 0;
    }

    if (!isValidTileByteOffset(byteOffset)) {
        return 0;
    }

    unsigned short address = (unsigned short)(getTileDefinitionBase(tileIndex) + byteOffset);

    return memory.read8(address);
}

bool GraphicsMemory::writePaletteColor(int paletteIndex, unsigned char rgb332) {
    if (!isValidPaletteIndex(paletteIndex)) {
        return false;
    }

    memory.write8(getPaletteAddress(paletteIndex), rgb332);
    return true;
}

unsigned char GraphicsMemory::readPaletteColor(int paletteIndex) {
    if (!isValidPaletteIndex(paletteIndex)) {
        return 0;
    }

    return memory.read8(getPaletteAddress(paletteIndex));
}