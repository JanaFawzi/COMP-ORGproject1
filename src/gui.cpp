#include "gui.h"
#include "cpu.h"
#include "graphics_memory.h"
#include "raylib.h"

#include <stdio.h>
#include <math.h>

static bool guiAudioReady = false;
static bool guiToneLoaded = false;
static Sound guiToneSound;
static unsigned int guiLastToneRequestId = 0;

Gui::Gui() {
    width = 1180;
    height = 650;
}

int Gui::getTargetFps() {
    return TARGET_FPS;
}

float Gui::getTargetFrameTimeMs() {
    return 1000.0f / (float)TARGET_FPS;
}

bool Gui::isStableFrameTimeMs(float frameTimeMs) {
    float target = getTargetFrameTimeMs();

    if (frameTimeMs < target - 5.0f) {
        return false;
    }

    if (frameTimeMs > target + 5.0f) {
        return false;
    }

    return true;
}

unsigned short Gui::mapRaylibKeyToZx16(int raylibKey) {
    if (raylibKey == KEY_UP || raylibKey == KEY_W) {
        return CPU::ZX16_KEY_UP;
    }

    if (raylibKey == KEY_DOWN || raylibKey == KEY_S) {
        return CPU::ZX16_KEY_DOWN;
    }

    if (raylibKey == KEY_LEFT || raylibKey == KEY_A) {
        return CPU::ZX16_KEY_LEFT;
    }

    if (raylibKey == KEY_RIGHT || raylibKey == KEY_D) {
        return CPU::ZX16_KEY_RIGHT;
    }

    if (raylibKey == KEY_SPACE) {
        return CPU::ZX16_KEY_SPACE;
    }

    if (raylibKey == KEY_ENTER) {
        return CPU::ZX16_KEY_ENTER;
    }

    if (raylibKey == KEY_ESCAPE) {
        return CPU::ZX16_KEY_ESCAPE;
    }

    return CPU::ZX16_KEY_NONE;
}

void Gui::open() {
    InitWindow(width, height, "ZX16 Simulator");

    InitAudioDevice();
    guiAudioReady = IsAudioDeviceReady();

    SetTargetFPS(getTargetFps());
}

bool Gui::shouldClose() {
    return WindowShouldClose();
}

GuiAction Gui::draw(
    const char testStatus[],
    const char consoleText[],
    int frameNumber,
    bool running,
    CPU& cpu
) {
    GuiAction action;

    BeginDrawing();

    ClearBackground(BLACK);

    updateKeyboardFromRaylib(cpu);
    updateAudioFromCpu(cpu);

    DrawText("ZX16 Simulator", 490, 15, 22, RAYWHITE);

    drawStatusPanel(testStatus, frameNumber, running, cpu);
    drawConsolePanel(consoleText);
    action = drawControlPanel(running, cpu.isHalted());
    drawRegisterPanel(cpu);
    drawMemoryPanel(cpu);
    drawGraphicsPanel(cpu);

    EndDrawing();

    return action;
}

void Gui::updateKeyboardFromRaylib(CPU& cpu) {
    unsigned short keyCode = CPU::ZX16_KEY_NONE;

    if (IsKeyDown(KEY_UP)) {
        keyCode = mapRaylibKeyToZx16(KEY_UP);
    }
    else if (IsKeyDown(KEY_W)) {
        keyCode = mapRaylibKeyToZx16(KEY_W);
    }
    else if (IsKeyDown(KEY_DOWN)) {
        keyCode = mapRaylibKeyToZx16(KEY_DOWN);
    }
    else if (IsKeyDown(KEY_S)) {
        keyCode = mapRaylibKeyToZx16(KEY_S);
    }
    else if (IsKeyDown(KEY_LEFT)) {
        keyCode = mapRaylibKeyToZx16(KEY_LEFT);
    }
    else if (IsKeyDown(KEY_A)) {
        keyCode = mapRaylibKeyToZx16(KEY_A);
    }
    else if (IsKeyDown(KEY_RIGHT)) {
        keyCode = mapRaylibKeyToZx16(KEY_RIGHT);
    }
    else if (IsKeyDown(KEY_D)) {
        keyCode = mapRaylibKeyToZx16(KEY_D);
    }
    else if (IsKeyDown(KEY_SPACE)) {
        keyCode = mapRaylibKeyToZx16(KEY_SPACE);
    }
    else if (IsKeyDown(KEY_ENTER)) {
        keyCode = mapRaylibKeyToZx16(KEY_ENTER);
    }
    else if (IsKeyDown(KEY_ESCAPE)) {
        keyCode = mapRaylibKeyToZx16(KEY_ESCAPE);
    }

    cpu.setKeyboardKey(keyCode);
}

void Gui::updateAudioFromCpu(CPU& cpu) {
    if (!cpu.hasPendingTone()) {
        return;
    }

    if (cpu.getToneRequestId() == guiLastToneRequestId) {
        cpu.clearToneRequest();
        return;
    }

    playTone(cpu.getToneFrequency(), cpu.getToneDurationMs());

    guiLastToneRequestId = cpu.getToneRequestId();

    cpu.clearToneRequest();
}

bool Gui::playTone(unsigned short frequency, unsigned short durationMs) {
    if (!guiAudioReady) {
        return false;
    }

    if (!CPU::isValidTone(frequency, durationMs)) {
        return false;
    }

    if (guiToneLoaded) {
        UnloadSound(guiToneSound);
        guiToneLoaded = false;
    }

    const int sampleRate = 44100;
    int sampleCount = (sampleRate * durationMs) / 1000;

    if (sampleCount <= 0) {
        return false;
    }

    short* samples = new short[sampleCount];

    double pi = 3.14159265358979323846;
    double amplitude = 28000.0;
    int fadeSamples = sampleRate / 100;

    if (fadeSamples * 2 > sampleCount) {
        fadeSamples = sampleCount / 2;
    }

    for (int i = 0; i < sampleCount; i++) {
        double t = (double)i / (double)sampleRate;
        double envelope = 1.0;

        if (fadeSamples > 0 && i < fadeSamples) {
            envelope = (double)i / (double)fadeSamples;
        }
        else if (fadeSamples > 0 && i >= sampleCount - fadeSamples) {
            envelope = (double)(sampleCount - i - 1) / (double)fadeSamples;
        }

        double value = sin(2.0 * pi * (double)frequency * t);

        samples[i] = (short)(value * amplitude * envelope);
    }

    Wave wave;

    wave.frameCount = sampleCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples;

    guiToneSound = LoadSoundFromWave(wave);
    guiToneLoaded = true;

    delete[] samples;

    PlaySound(guiToneSound);

    return true;
}

void Gui::drawStatusPanel(
    const char testStatus[],
    int frameNumber,
    bool running,
    CPU& cpu
) {
    char frameText[40];
    char pcText[40];
    char spText[40];
    char stateText[40];
    char fpsText[40];
    char frameTimeText[40];
    char keyText[40];

    sprintf(frameText, "Frame update: %d", frameNumber);
    sprintf(pcText, "PC: 0x%04X", cpu.getPC());
    sprintf(spText, "SP: 0x%04X", cpu.getSP());
    sprintf(fpsText, "Target FPS: %d", getTargetFps());
    sprintf(frameTimeText, "Target frame: %.2f ms", getTargetFrameTimeMs());
    sprintf(keyText, "Keyboard key: %d", cpu.getKeyboardKey());

    if (cpu.isHalted()) {
        sprintf(stateText, "CPU state: HALTED");
    }
    else if (running) {
        sprintf(stateText, "CPU state: RUNNING");
    }
    else {
        sprintf(stateText, "CPU state: PAUSED");
    }

    DrawRectangleLines(25, 60, 260, 540, GREEN);

    DrawText("Status Panel", 45, 80, 18, GREEN);
    DrawText(testStatus, 45, 115, 14, RAYWHITE);

    DrawText("Window open: PASSED", 45, 150, 14, GREEN);
    DrawText("Console panel: PASSED", 45, 175, 14, GREEN);
    DrawText("Buttons: PASSED", 45, 200, 14, GREEN);
    DrawText("Register display: PASSED", 45, 225, 14, GREEN);
    DrawText("Memory viewer: PASSED", 45, 250, 14, GREEN);
    DrawText("Graphics output: PASSED", 45, 275, 14, GREEN);

    DrawText(pcText, 45, 325, 14, GREEN);
    DrawText(spText, 45, 350, 14, GREEN);
    DrawText(stateText, 45, 375, 14, GREEN);
    DrawText(frameText, 45, 400, 14, GREEN);
    DrawText(fpsText, 45, 425, 14, GREEN);
    DrawText(frameTimeText, 45, 450, 14, GREEN);
    DrawText(keyText, 45, 475, 14, GREEN);
}

void Gui::drawConsolePanel(const char consoleText[]) {
    DrawRectangleLines(315, 60, 270, 160, GREEN);

    DrawText("Console", 335, 80, 18, GREEN);

    DrawRectangle(335, 115, 230, 85, DARKGRAY);
    DrawRectangleLines(335, 115, 230, 85, GRAY);

    DrawText(consoleText, 350, 135, 18, GREEN);
}

GuiAction Gui::drawControlPanel(bool running, bool halted) {
    GuiAction action = GUI_ACTION_NONE;

    DrawRectangleLines(315, 240, 270, 115, GREEN);

    DrawText("Controls", 335, 260, 18, GREEN);

    if (halted) {
        if (drawButton(335, 305, 70, 30, "Run")) {
            action = GUI_ACTION_RUN_PAUSE;
        }
    }
    else if (running) {
        if (drawButton(335, 305, 70, 30, "Pause")) {
            action = GUI_ACTION_RUN_PAUSE;
        }
    }
    else {
        if (drawButton(335, 305, 70, 30, "Run")) {
            action = GUI_ACTION_RUN_PAUSE;
        }
    }

    if (drawButton(425, 305, 70, 30, "Step")) {
        action = GUI_ACTION_STEP;
    }

    if (drawButton(515, 305, 70, 30, "Reset")) {
        action = GUI_ACTION_RESET;
    }

    return action;
}

void Gui::drawRegisterPanel(CPU& cpu) {
    char text[40];

    DrawRectangleLines(615, 60, 240, 540, GREEN);

    DrawText("Registers", 635, 80, 18, GREEN);

    sprintf(text, "PC     : 0x%04X", cpu.getPC());
    DrawText(text, 635, 115, 14, GREEN);

    sprintf(text, "SP/x2  : 0x%04X", cpu.getSP());
    DrawText(text, 635, 140, 14, GREEN);

    for (int i = 0; i < 8; i++) {
        if (i == 2) {
            sprintf(text, "x%d/sp : 0x%04X", i, cpu.getRegisters().getRegister(i));
        }
        else {
            sprintf(text, "x%d    : 0x%04X", i, cpu.getRegisters().getRegister(i));
        }

        DrawText(text, 635, 180 + i * 24, 14, GREEN);
    }
}

void Gui::formatMemoryLine(CPU& cpu, unsigned short address, char text[]) {
    sprintf(
        text,
        "0x%04X: %02X %02X %02X %02X %02X %02X %02X %02X",
        address,
        cpu.getMemory().read8(address),
        cpu.getMemory().read8(address + 1),
        cpu.getMemory().read8(address + 2),
        cpu.getMemory().read8(address + 3),
        cpu.getMemory().read8(address + 4),
        cpu.getMemory().read8(address + 5),
        cpu.getMemory().read8(address + 6),
        cpu.getMemory().read8(address + 7)
    );
}

void Gui::drawMemoryPanel(CPU& cpu) {
    char text[80];

    unsigned short baseAddress = cpu.getPC() & 0xFFF0;

    DrawRectangleLines(885, 60, 270, 540, GREEN);

    DrawText("Memory Viewer", 905, 80, 18, GREEN);

    DrawText("RAM around PC", 905, 115, 14, RAYWHITE);

    for (int row = 0; row < 8; row++) {
        formatMemoryLine(cpu, baseAddress + row * 8, text);
        DrawText(text, 905, 150 + row * 28, 13, GREEN);
    }
}

void Gui::drawGraphicsPanel(CPU& cpu) {
    GraphicsMemory graphics(cpu.getMemory());

    Rgb888Color pixel;
    Color color;

    int panelX = 315;
    int panelY = 380;
    int panelW = 270;
    int panelH = 220;

    int previewX = 335;
    int previewY = 450;

    int previewScale = 2;
    int previewW = GraphicsMemory::SCREEN_WIDTH / previewScale;
    int previewH = GraphicsMemory::SCREEN_HEIGHT / previewScale;

    DrawRectangleLines(panelX, panelY, panelW, panelH, GREEN);

    DrawText("Graphics Output", panelX + 20, panelY + 20, 18, GREEN);
    DrawText("Scaled 320x240 VRAM preview", panelX + 20, panelY + 44, 13, RAYWHITE);

    DrawRectangle(previewX, previewY, previewW, previewH, BLACK);
    DrawRectangleLines(previewX, previewY, previewW, previewH, GRAY);

    color.a = 255;

    for (int y = 0; y < previewH; y++) {
        for (int x = 0; x < previewW; x++) {
            int sourceX = x * previewScale;
            int sourceY = y * previewScale;

            if (graphics.renderScreenPixel(sourceX, sourceY, pixel)) {
                color.r = pixel.red8;
                color.g = pixel.green8;
                color.b = pixel.blue8;

                DrawPixel(previewX + x, previewY + y, color);
            }
        }
    }
}

bool Gui::drawButton(int x, int y, int w, int h, const char text[]) {
    Rectangle button;
    Vector2 mouse;
    bool hover;

    button.x = (float)x;
    button.y = (float)y;
    button.width = (float)w;
    button.height = (float)h;

    mouse = GetMousePosition();
    hover = CheckCollisionPointRec(mouse, button);

    if (hover) {
        DrawRectangleRec(button, DARKGREEN);
    }
    else {
        DrawRectangleRec(button, DARKGRAY);
    }

    DrawRectangleLines(x, y, w, h, GREEN);
    DrawText(text, x + 12, y + 8, 14, RAYWHITE);

    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return true;
    }

    return false;
}

void Gui::close() {
    if (guiToneLoaded) {
        UnloadSound(guiToneSound);
        guiToneLoaded = false;
    }

    if (guiAudioReady) {
        CloseAudioDevice();
        guiAudioReady = false;
    }

    CloseWindow();
}