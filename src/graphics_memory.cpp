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

bool GraphicsMemory::isVramAddress(unsigned short address) {
    return isGraphicsAddress(address);
}

bool GraphicsMemory::isReservedMmioAddress(unsigned short address) {
    if (address < RESERVED_MMIO_BASE || address > RESERVED_MMIO_END) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isSafeVramRange(unsigned short address, int byteCount) {
    if (byteCount <= 0) {
        return false;
    }

    int start = address;
    int end = start + byteCount - 1;

    if (start < GRAPHICS_BASE) {
        return false;
    }

    if (end > GRAPHICS_END) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isEvenAddress(unsigned short address) {
    if ((address & 1) != 0) {
        return false;
    }

    return true;
}

int GraphicsMemory::getGraphicsRegion(unsigned short address) {
    if (isTileMapUsedAddress(address)) {
        return REGION_TILE_MAP_USED;
    }

    if (isTileMapRegionAddress(address)) {
        return REGION_TILE_MAP_PADDING;
    }

    if (isTileDefinitionAddress(address)) {
        return REGION_TILE_DEFINITION;
    }

    if (isPaletteAddress(address)) {
        return REGION_PALETTE;
    }

    if (isReservedMmioAddress(address)) {
        return REGION_RESERVED_MMIO;
    }

    return REGION_NONE;
}

bool GraphicsMemory::writeVram8(unsigned short address, unsigned char value) {
    if (!isVramAddress(address)) {
        return false;
    }

    memory.write8(address, value);
    return true;
}

bool GraphicsMemory::readVram8(unsigned short address, unsigned char& value) {
    if (!isVramAddress(address)) {
        return false;
    }

    value = memory.read8(address);
    return true;
}

bool GraphicsMemory::writeVram16(unsigned short address, unsigned short value) {
    if (!isEvenAddress(address)) {
        return false;
    }

    if (!isSafeVramRange(address, 2)) {
        return false;
    }

    memory.write16(address, value);
    return true;
}

bool GraphicsMemory::readVram16(unsigned short address, unsigned short& value) {
    if (!isEvenAddress(address)) {
        return false;
    }

    if (!isSafeVramRange(address, 2)) {
        return false;
    }

    value = memory.read16(address);
    return true;
}

bool GraphicsMemory::writeTileIndex(int col, int row, unsigned char tileIndex) {
    if (!isValidTileCell(col, row)) {
        return false;
    }

    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    return writeVram8(getTileMapAddress(col, row), tileIndex);
}

unsigned char GraphicsMemory::readTileIndex(int col, int row) {
    unsigned char value = 0;

    if (!isValidTileCell(col, row)) {
        return 0;
    }

    readVram8(getTileMapAddress(col, row), value);

    return value;
}

bool GraphicsMemory::writeTileDefinitionByte(int tileIndex, int byteOffset, unsigned char value) {
    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    if (!isValidTileByteOffset(byteOffset)) {
        return false;
    }

    unsigned short address = (unsigned short)(getTileDefinitionBase(tileIndex) + byteOffset);

    return writeVram8(address, value);
}

unsigned char GraphicsMemory::readTileDefinitionByte(int tileIndex, int byteOffset) {
    unsigned char value = 0;

    if (!isValidTileIndex(tileIndex)) {
        return 0;
    }

    if (!isValidTileByteOffset(byteOffset)) {
        return 0;
    }

    unsigned short address = (unsigned short)(getTileDefinitionBase(tileIndex) + byteOffset);

    readVram8(address, value);

    return value;
}

bool GraphicsMemory::writePaletteColor(int paletteIndex, unsigned char rgb332) {
    if (!isValidPaletteIndex(paletteIndex)) {
        return false;
    }

    return writeVram8(getPaletteAddress(paletteIndex), rgb332);
}

unsigned char GraphicsMemory::readPaletteColor(int paletteIndex) {
    unsigned char value = 0;

    if (!isValidPaletteIndex(paletteIndex)) {
        return 0;
    }

    readVram8(getPaletteAddress(paletteIndex), value);

    return value;
}