#include "gui.h"
#include "raylib.h"

#include <stdio.h>

Gui::Gui() {
    width = 640;
    height = 360;
}

void Gui::open() {
    InitWindow(width, height, "ZX16 Simulator");
    SetTargetFPS(60);
}

bool Gui::shouldClose() {
    return WindowShouldClose();
}

void Gui::draw(const char testStatus[], const char consoleText[], int frameNumber) {
    BeginDrawing();

    ClearBackground(BLACK);

    DrawText("ZX16 Simulator", 220, 15, 22, RAYWHITE);

    drawStatusPanel(testStatus, frameNumber);
    drawConsolePanel(consoleText);

    EndDrawing();
}

void Gui::drawStatusPanel(const char testStatus[], int frameNumber) {
    char frameText[40];

    sprintf(frameText, "Frame update: %d", frameNumber);

    DrawRectangleLines(25, 60, 270, 250, GREEN);

    DrawText("Status Panel", 45, 75, 18, GREEN);
    DrawText(testStatus, 45, 110, 14, RAYWHITE);

    DrawText("Window open: PASSED", 45, 145, 14, GREEN);
    DrawText("Console panel: PASSED", 45, 170, 14, GREEN);
    DrawText(frameText, 45, 195, 14, GREEN);

    DrawText("If this number changes,", 45, 235, 13, GRAY);
    DrawText("the GUI is updating.", 45, 255, 13, GRAY);
}

void Gui::drawConsolePanel(const char consoleText[]) {
    DrawRectangleLines(325, 60, 290, 250, GREEN);

    DrawText("Console", 345, 75, 18, GREEN);

    DrawRectangle(345, 110, 250, 170, DARKGRAY);
    DrawRectangleLines(345, 110, 250, 170, GRAY);

    DrawText(consoleText, 360, 130, 18, GREEN);
}

void Gui::close() {
    CloseWindow();
}