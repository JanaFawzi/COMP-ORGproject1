#ifndef SNAKE_RUNTIME_H
#define SNAKE_RUNTIME_H

class CPU;
class GraphicsMemory;

class SnakeRuntime {
public:
    static const int EMPTY_TILE = 0;
    static const int SNAKE_TILE = 1;
    static const int FOOD_TILE = 2;
    static const int WALL_TILE = 3;

    static void initialize(CPU& cpu);

private:
    static void initializePalette(GraphicsMemory& graphics);
    static void initializeTileDefinitions(GraphicsMemory& graphics);
    static void initializeTileMap(GraphicsMemory& graphics);
};

#endif