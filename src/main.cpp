#include "raylib.h"
#include "memory.h"

#include <assert.h>
#include <stdio.h>

void testMemory() {
    Memory memory;

    // RAM starts empty
    assert(memory.read8(0x0000) == 0);
    assert(memory.read8(0xFFFF) == 0);

    // Write different bytes
    memory.write8(0x0000, 0x12);
    memory.write8(0x0020, 0xAB);
    memory.write8(0xF000, 0x05);
    memory.write8(0xFFFF, 0xFF);

    // Read them back
    assert(memory.read8(0x0000) == 0x12);
    assert(memory.read8(0x0020) == 0xAB);
    assert(memory.read8(0xF000) == 0x05);
    assert(memory.read8(0xFFFF) == 0xFF);

    // Test several bytes
    for (int i = 0; i < 16; i++) {
        memory.write8(0x0100 + i, i + 1);
    }

    for (int i = 0; i < 16; i++) {
        assert(memory.read8(0x0100 + i) == i + 1);
    }

    printf("[PASS] Memory read8/write8 test passed\n");
}

int main() {
    testMemory();

    InitWindow(320, 240, "ZX16 Simulator");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);
        DrawText("ZX16 Simulator", 90, 90, 20, RAYWHITE);
        DrawText("Student 2: Memory class", 65, 120, 16, GRAY);
        DrawText("read8/write8 passed", 75, 145, 16, GREEN);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}