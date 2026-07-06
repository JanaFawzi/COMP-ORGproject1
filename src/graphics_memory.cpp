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

bool GraphicsMemory::isValidTileMapOffset(int offset) {
    if (offset < 0 || offset >= TILE_MAP_USED_SIZE) {
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

bool GraphicsMemory::isValidScreenPixel(int x, int y) {
    if (x < 0 || x >= SCREEN_WIDTH) {
        return false;
    }

    if (y < 0 || y >= SCREEN_HEIGHT) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isValidRgb3(int value) {
    if (value < 0 || value > 7) {
        return false;
    }

    return true;
}

bool GraphicsMemory::isValidRgb2(int value) {
    if (value < 0 || value > 3) {
        return false;
    }

    return true;
}

int GraphicsMemory::getTileMapOffset(int col, int row) {
    if (!isValidTileCell(col, row)) {
        return -1;
    }

    return row * TILE_COLUMNS + col;
}

unsigned short GraphicsMemory::getTileMapAddress(int col, int row) {
    int offset = getTileMapOffset(col, row);

    if (!isValidTileMapOffset(offset)) {
        return INVALID_ADDRESS;
    }

    return (unsigned short)(TILE_MAP_BASE + offset);
}

unsigned short GraphicsMemory::getTileMapAddressByOffset(int offset) {
    if (!isValidTileMapOffset(offset)) {
        return INVALID_ADDRESS;
    }

    return (unsigned short)(TILE_MAP_BASE + offset);
}

int GraphicsMemory::getTilePixelNumber(int x, int y) {
    if (!isValidTilePixel(x, y)) {
        return -1;
    }

    return y * TILE_SIZE + x;
}

int GraphicsMemory::getTilePixelByteOffset(int x, int y) {
    int pixelNumber = getTilePixelNumber(x, y);

    if (pixelNumber < 0) {
        return -1;
    }

    return pixelNumber / 2;
}

bool GraphicsMemory::isLowNibblePixel(int x) {
    if ((x & 1) == 0) {
        return true;
    }

    return false;
}

int GraphicsMemory::getScreenPixelNumber(int x, int y) {
    if (!isValidScreenPixel(x, y)) {
        return -1;
    }

    return y * SCREEN_WIDTH + x;
}

int GraphicsMemory::getScreenTileColumn(int x) {
    if (x < 0 || x >= SCREEN_WIDTH) {
        return -1;
    }

    return x / TILE_SIZE;
}

int GraphicsMemory::getScreenTileRow(int y) {
    if (y < 0 || y >= SCREEN_HEIGHT) {
        return -1;
    }

    return y / TILE_SIZE;
}

int GraphicsMemory::getScreenTilePixelX(int x) {
    if (x < 0 || x >= SCREEN_WIDTH) {
        return -1;
    }

    return x % TILE_SIZE;
}

int GraphicsMemory::getScreenTilePixelY(int y) {
    if (y < 0 || y >= SCREEN_HEIGHT) {
        return -1;
    }

    return y % TILE_SIZE;
}

unsigned char GraphicsMemory::extractLowNibble(unsigned char value) {
    return value & 0x0F;
}

unsigned char GraphicsMemory::extractHighNibble(unsigned char value) {
    return (value >> 4) & 0x0F;
}

unsigned char GraphicsMemory::extractPaletteIndexFromTileByte(unsigned char tileByte, int x) {
    if (isLowNibblePixel(x)) {
        return extractLowNibble(tileByte);
    }

    return extractHighNibble(tileByte);
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

    int byteOffset = getTilePixelByteOffset(x, y);

    if (!isValidTileByteOffset(byteOffset)) {
        return INVALID_ADDRESS;
    }

    return (unsigned short)(getTileDefinitionBase(tileIndex) + byteOffset);
}

unsigned short GraphicsMemory::getPaletteAddress(int paletteIndex) {
    if (!isValidPaletteIndex(paletteIndex)) {
        return INVALID_ADDRESS;
    }

    return (unsigned short)(PALETTE_BASE + paletteIndex);
}

unsigned char GraphicsMemory::makeRgb332(int red3, int green3, int blue2) {
    if (!isValidRgb3(red3)) {
        return 0;
    }

    if (!isValidRgb3(green3)) {
        return 0;
    }

    if (!isValidRgb2(blue2)) {
        return 0;
    }

    return (unsigned char)(((red3 & 0x07) << 5) | ((green3 & 0x07) << 2) | (blue2 & 0x03));
}

unsigned char GraphicsMemory::getRgb332Red3(unsigned char rgb332) {
    return (rgb332 >> 5) & 0x07;
}

unsigned char GraphicsMemory::getRgb332Green3(unsigned char rgb332) {
    return (rgb332 >> 2) & 0x07;
}

unsigned char GraphicsMemory::getRgb332Blue2(unsigned char rgb332) {
    return rgb332 & 0x03;
}

Rgb332Color GraphicsMemory::decodeRgb332(unsigned char rgb332) {
    Rgb332Color color;

    color.red3 = getRgb332Red3(rgb332);
    color.green3 = getRgb332Green3(rgb332);
    color.blue2 = getRgb332Blue2(rgb332);

    return color;
}

unsigned char GraphicsMemory::expandRgb3To8(unsigned char value3) {
    value3 = value3 & 0x07;

    return (unsigned char)((value3 << 5) | (value3 << 2) | (value3 >> 1));
}

unsigned char GraphicsMemory::expandRgb2To8(unsigned char value2) {
    value2 = value2 & 0x03;

    return (unsigned char)((value2 << 6) | (value2 << 4) | (value2 << 2) | value2);
}

void GraphicsMemory::expandRgb332ToRgb888(
    unsigned char rgb332,
    unsigned char& red8,
    unsigned char& green8,
    unsigned char& blue8
) {
    red8 = expandRgb3To8(getRgb332Red3(rgb332));
    green8 = expandRgb3To8(getRgb332Green3(rgb332));
    blue8 = expandRgb2To8(getRgb332Blue2(rgb332));
}

Rgb888Color GraphicsMemory::expandRgb332ToRgb888Color(unsigned char rgb332) {
    Rgb888Color color;

    expandRgb332ToRgb888(rgb332, color.red8, color.green8, color.blue8);

    return color;
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
    int offset = getTileMapOffset(col, row);

    return writeTileIndexByOffset(offset, tileIndex);
}

unsigned char GraphicsMemory::readTileIndex(int col, int row) {
    unsigned char value = 0;

    if (!readTileIndexChecked(col, row, value)) {
        return 0;
    }

    return value;
}

bool GraphicsMemory::readTileIndexChecked(int col, int row, unsigned char& tileIndex) {
    int offset = getTileMapOffset(col, row);

    return readTileIndexByOffset(offset, tileIndex);
}

bool GraphicsMemory::writeTileIndexByOffset(int offset, unsigned char tileIndex) {
    if (!isValidTileMapOffset(offset)) {
        return false;
    }

    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    return writeVram8(getTileMapAddressByOffset(offset), tileIndex);
}

bool GraphicsMemory::readTileIndexByOffset(int offset, unsigned char& tileIndex) {
    if (!isValidTileMapOffset(offset)) {
        return false;
    }

    return readVram8(getTileMapAddressByOffset(offset), tileIndex);
}

bool GraphicsMemory::clearTileMap() {
    return fillTileMap(0);
}

bool GraphicsMemory::fillTileMap(unsigned char tileIndex) {
    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    for (int offset = 0; offset < TILE_MAP_USED_SIZE; offset++) {
        if (!writeTileIndexByOffset(offset, tileIndex)) {
            return false;
        }
    }

    return true;
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

    if (!readTileDefinitionByteChecked(tileIndex, byteOffset, value)) {
        return 0;
    }

    return value;
}

bool GraphicsMemory::readTileDefinitionByteChecked(int tileIndex, int byteOffset, unsigned char& value) {
    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    if (!isValidTileByteOffset(byteOffset)) {
        return false;
    }

    unsigned short address = (unsigned short)(getTileDefinitionBase(tileIndex) + byteOffset);

    return readVram8(address, value);
}

bool GraphicsMemory::writeTileDefinition(int tileIndex, const unsigned char data[], int byteCount) {
    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    if (data == 0) {
        return false;
    }

    if (byteCount != TILE_BYTES) {
        return false;
    }

    for (int i = 0; i < TILE_BYTES; i++) {
        if (!writeTileDefinitionByte(tileIndex, i, data[i])) {
            return false;
        }
    }

    return true;
}

bool GraphicsMemory::readTileDefinition(int tileIndex, unsigned char data[], int byteCount) {
    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    if (data == 0) {
        return false;
    }

    if (byteCount != TILE_BYTES) {
        return false;
    }

    for (int i = 0; i < TILE_BYTES; i++) {
        if (!readTileDefinitionByteChecked(tileIndex, i, data[i])) {
            return false;
        }
    }

    return true;
}

bool GraphicsMemory::clearTileDefinition(int tileIndex, unsigned char paletteIndex) {
    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    if (!isValidPaletteIndex(paletteIndex)) {
        return false;
    }

    unsigned char packed = (unsigned char)((paletteIndex & 0x0F) | ((paletteIndex & 0x0F) << 4));

    for (int i = 0; i < TILE_BYTES; i++) {
        if (!writeTileDefinitionByte(tileIndex, i, packed)) {
            return false;
        }
    }

    return true;
}

bool GraphicsMemory::writeTilePixel(int tileIndex, int x, int y, unsigned char paletteIndex) {
    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    if (!isValidTilePixel(x, y)) {
        return false;
    }

    if (!isValidPaletteIndex(paletteIndex)) {
        return false;
    }

    unsigned short address = getTilePixelByteAddress(tileIndex, x, y);
    unsigned char oldByte = 0;

    if (!readVram8(address, oldByte)) {
        return false;
    }

    unsigned char newByte = oldByte;

    if (isLowNibblePixel(x)) {
        newByte = (unsigned char)((oldByte & 0xF0) | (paletteIndex & 0x0F));
    }
    else {
        newByte = (unsigned char)((oldByte & 0x0F) | ((paletteIndex & 0x0F) << 4));
    }

    return writeVram8(address, newByte);
}

bool GraphicsMemory::readTilePixel(int tileIndex, int x, int y, unsigned char& paletteIndex) {
    return readTilePixelPaletteIndex(tileIndex, x, y, paletteIndex);
}

bool GraphicsMemory::readTilePixelPaletteIndex(int tileIndex, int x, int y, unsigned char& paletteIndex) {
    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    if (!isValidTilePixel(x, y)) {
        return false;
    }

    unsigned short address = getTilePixelByteAddress(tileIndex, x, y);
    unsigned char byteValue = 0;

    if (!readVram8(address, byteValue)) {
        return false;
    }

    paletteIndex = extractPaletteIndexFromTileByte(byteValue, x);

    return true;
}

bool GraphicsMemory::renderTilePixel(int tileIndex, int x, int y, Rgb888Color& color) {
    unsigned char paletteIndex = 0;
    unsigned char rgb332 = 0;

    if (!readTilePixelPaletteIndex(tileIndex, x, y, paletteIndex)) {
        return false;
    }

    if (!readPaletteColorChecked(paletteIndex, rgb332)) {
        return false;
    }

    color = expandRgb332ToRgb888Color(rgb332);

    return true;
}

bool GraphicsMemory::renderTile(int tileIndex, Rgb888Color pixels[], int pixelCount) {
    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    if (pixels == 0) {
        return false;
    }

    if (pixelCount != TILE_PIXEL_COUNT) {
        return false;
    }

    for (int y = 0; y < TILE_SIZE; y++) {
        for (int x = 0; x < TILE_SIZE; x++) {
            int pixelNumber = getTilePixelNumber(x, y);

            if (!renderTilePixel(tileIndex, x, y, pixels[pixelNumber])) {
                return false;
            }
        }
    }

    return true;
}

bool GraphicsMemory::renderTilePaletteIndices(int tileIndex, unsigned char pixels[], int pixelCount) {
    if (!isValidTileIndex(tileIndex)) {
        return false;
    }

    if (pixels == 0) {
        return false;
    }

    if (pixelCount != TILE_PIXEL_COUNT) {
        return false;
    }

    for (int y = 0; y < TILE_SIZE; y++) {
        for (int x = 0; x < TILE_SIZE; x++) {
            int pixelNumber = getTilePixelNumber(x, y);

            if (!readTilePixelPaletteIndex(tileIndex, x, y, pixels[pixelNumber])) {
                return false;
            }
        }
    }

    return true;
}

bool GraphicsMemory::renderScreenPixel(int x, int y, Rgb888Color& color) {
    if (!isValidScreenPixel(x, y)) {
        return false;
    }

    int tileCol = getScreenTileColumn(x);
    int tileRow = getScreenTileRow(y);
    int tileX = getScreenTilePixelX(x);
    int tileY = getScreenTilePixelY(y);

    unsigned char tileIndex = 0;

    if (!readTileIndexChecked(tileCol, tileRow, tileIndex)) {
        return false;
    }

    return renderTilePixel(tileIndex, tileX, tileY, color);
}

bool GraphicsMemory::renderScreen(Rgb888Color pixels[], int pixelCount) {
    if (pixels == 0) {
        return false;
    }

    if (pixelCount != SCREEN_PIXEL_COUNT) {
        return false;
    }

    Rgb888Color paletteColors[PALETTE_SIZE];

    for (int paletteIndex = 0; paletteIndex < PALETTE_SIZE; paletteIndex++) {
        if (!readPaletteRgb888Color(paletteIndex, paletteColors[paletteIndex])) {
            return false;
        }
    }

    for (int tileRow = 0; tileRow < TILE_ROWS; tileRow++) {
        for (int tileCol = 0; tileCol < TILE_COLUMNS; tileCol++) {
            unsigned char tileIndex = 0;

            if (!readTileIndexChecked(tileCol, tileRow, tileIndex)) {
                return false;
            }

            for (int tileY = 0; tileY < TILE_SIZE; tileY++) {
                for (int tileX = 0; tileX < TILE_SIZE; tileX++) {
                    unsigned char paletteIndex = 0;

                    if (!readTilePixelPaletteIndex(tileIndex, tileX, tileY, paletteIndex)) {
                        return false;
                    }

                    int screenX = tileCol * TILE_SIZE + tileX;
                    int screenY = tileRow * TILE_SIZE + tileY;
                    int pixelNumber = getScreenPixelNumber(screenX, screenY);

                    pixels[pixelNumber] = paletteColors[paletteIndex];
                }
            }
        }
    }

    return true;
}

bool GraphicsMemory::renderScreenPaletteIndices(unsigned char pixels[], int pixelCount) {
    if (pixels == 0) {
        return false;
    }

    if (pixelCount != SCREEN_PIXEL_COUNT) {
        return false;
    }

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int pixelNumber = getScreenPixelNumber(x, y);

            int tileCol = getScreenTileColumn(x);
            int tileRow = getScreenTileRow(y);
            int tileX = getScreenTilePixelX(x);
            int tileY = getScreenTilePixelY(y);

            unsigned char tileIndex = 0;

            if (!readTileIndexChecked(tileCol, tileRow, tileIndex)) {
                return false;
            }

            if (!readTilePixelPaletteIndex(tileIndex, tileX, tileY, pixels[pixelNumber])) {
                return false;
            }
        }
    }

    return true;
}

bool GraphicsMemory::writePaletteColor(int paletteIndex, unsigned char rgb332) {
    if (!isValidPaletteIndex(paletteIndex)) {
        return false;
    }

    return writeVram8(getPaletteAddress(paletteIndex), rgb332);
}

unsigned char GraphicsMemory::readPaletteColor(int paletteIndex) {
    unsigned char value = 0;

    if (!readPaletteColorChecked(paletteIndex, value)) {
        return 0;
    }

    return value;
}

bool GraphicsMemory::readPaletteColorChecked(int paletteIndex, unsigned char& rgb332) {
    if (!isValidPaletteIndex(paletteIndex)) {
        return false;
    }

    return readVram8(getPaletteAddress(paletteIndex), rgb332);
}

bool GraphicsMemory::writePaletteRgb(int paletteIndex, int red3, int green3, int blue2) {
    if (!isValidPaletteIndex(paletteIndex)) {
        return false;
    }

    if (!isValidRgb3(red3)) {
        return false;
    }

    if (!isValidRgb3(green3)) {
        return false;
    }

    if (!isValidRgb2(blue2)) {
        return false;
    }

    return writePaletteColor(paletteIndex, makeRgb332(red3, green3, blue2));
}

bool GraphicsMemory::readPaletteRgb332(int paletteIndex, Rgb332Color& color) {
    unsigned char rgb332 = 0;

    if (!readPaletteColorChecked(paletteIndex, rgb332)) {
        return false;
    }

    color = decodeRgb332(rgb332);

    return true;
}

bool GraphicsMemory::readPaletteRgb888Color(int paletteIndex, Rgb888Color& color) {
    unsigned char rgb332 = 0;

    if (!readPaletteColorChecked(paletteIndex, rgb332)) {
        return false;
    }

    color = expandRgb332ToRgb888Color(rgb332);

    return true;
}

bool GraphicsMemory::readPaletteRgb888(
    int paletteIndex,
    unsigned char& red8,
    unsigned char& green8,
    unsigned char& blue8
) {
    unsigned char rgb332 = 0;

    if (!readPaletteColorChecked(paletteIndex, rgb332)) {
        return false;
    }

    expandRgb332ToRgb888(rgb332, red8, green8, blue8);

    return true;
}

bool GraphicsMemory::fillPalette(unsigned char rgb332) {
    for (int i = 0; i < PALETTE_SIZE; i++) {
        if (!writePaletteColor(i, rgb332)) {
            return false;
        }
    }

    return true;
}

bool GraphicsMemory::clearPalette() {
    return fillPalette(0);
}