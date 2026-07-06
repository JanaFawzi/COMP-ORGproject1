#include "snake_runtime.h"

#include "cpu.h"
#include "graphics_memory.h"

void SnakeRuntime::initialize(CPU& cpu) {
    cpu.reset();

    GraphicsMemory graphics(cpu.getMemory());

    initializePalette(graphics);
    initializeTileDefinitions(graphics);
    initializeTileMap(graphics);
}

void SnakeRuntime::initializePalette(GraphicsMemory& graphics) {
    graphics.clearPalette();

    graphics.writePaletteColor(0, 0x00); // Background: black
    graphics.writePaletteColor(1, 0x1C); // Snake: green
    graphics.writePaletteColor(2, 0xE0); // Food: red
    graphics.writePaletteColor(3, 0xFF); // Wall: white
}

void SnakeRuntime::initializeTileDefinitions(GraphicsMemory& graphics) {
    for (int tileIndex = 0; tileIndex < GraphicsMemory::TILE_COUNT; tileIndex++) {
        graphics.clearTileDefinition(tileIndex, 0);
    }

    graphics.clearTileDefinition(EMPTY_TILE, 0);
    graphics.clearTileDefinition(SNAKE_TILE, 1);
    graphics.clearTileDefinition(FOOD_TILE, 2);
    graphics.clearTileDefinition(WALL_TILE, 3);
}

void SnakeRuntime::initializeTileMap(GraphicsMemory& graphics) {
    graphics.fillTileMap(EMPTY_TILE);

    for (int col = 0; col < GraphicsMemory::TILE_COLUMNS; col++) {
        graphics.writeTileIndex(col, 0, WALL_TILE);
        graphics.writeTileIndex(col, GraphicsMemory::TILE_ROWS - 1, WALL_TILE);
    }

    for (int row = 0; row < GraphicsMemory::TILE_ROWS; row++) {
        graphics.writeTileIndex(0, row, WALL_TILE);
        graphics.writeTileIndex(GraphicsMemory::TILE_COLUMNS - 1, row, WALL_TILE);
    }
}