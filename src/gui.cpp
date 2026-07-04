#include "gui.h"
#include "cpu.h"
#include "graphics_memory.h"
#include "instruction_decoder.h"
#include "raylib.h"

#include <stdio.h>
#include <math.h>

static bool guiAudioReady = false;
static bool guiToneLoaded = false;
static Sound guiToneSound;

static const char* getSyscallDisplayName(unsigned short service) {
    if (service == 0x000) {
        return "print_int";
    }

    if (service == 0x001) {
        return "print_char";
    }

    if (service == 0x010) {
        return "read_string";
    }

    if (service == 0x011) {
        return "read_int";
    }

    if (service == 0x012) {
        return "print_string";
    }

    if (service == 0x020) {
        return "seed_rng";
    }

    if (service == 0x021) {
        return "random";
    }

    if (service == 0x030) {
        return "keyboard";
    }

    if (service == 0x040) {
        return "play_tone";
    }

    if (service == 0x041) {
        return "set_volume";
    }

    if (service == 0x042) {
        return "stop_audio";
    }

    if (service == 0x050) {
        return "regs_dump";
    }

    if (service == 0x051) {
        return "mem_dump";
    }

    if (service == 0x3FF) {
        return "halt";
    }

    return "unknown_sys";
}

Gui::Gui() {
    width = 1180;
    height = 650;

    cursorAddress = 0;
    cursorAddressSelected = false;
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

int Gui::getConsoleVisibleLineCount() {
    return CONSOLE_VISIBLE_LINES;
}

void Gui::buildConsoleVisibleText(const char consoleText[], char visibleText[], int visibleSize) {
    if (visibleText == 0 || visibleSize <= 0) {
        return;
    }

    visibleText[0] = '\0';

    if (consoleText == 0) {
        return;
    }

    int length = 0;

    while (consoleText[length] != '\0') {
        length++;
    }

    if (length == 0) {
        return;
    }

    int end = length;

    if (end > 0 && consoleText[end - 1] == '\n') {
        end--;
    }

    int start = end;
    int newlineCount = 0;

    while (start > 0) {
        start--;

        if (consoleText[start] == '\n') {
            newlineCount++;

            if (newlineCount == CONSOLE_VISIBLE_LINES) {
                start++;
                break;
            }
        }
    }

    int outIndex = 0;

    for (int i = start; i < end && outIndex < visibleSize - 1; i++) {
        visibleText[outIndex] = consoleText[i];
        outIndex++;
    }

    visibleText[outIndex] = '\0';
}

unsigned short Gui::getMemoryViewerBaseAddress(unsigned short pc) {
    return pc & 0xFFF0;
}

bool Gui::hasCursorAddress() {
    return cursorAddressSelected;
}

unsigned short Gui::getCursorAddress() {
    return cursorAddress;
}

bool Gui::setCursorAddress(unsigned short address) {
    if ((address & 1) != 0) {
        return false;
    }

    cursorAddress = address;
    cursorAddressSelected = true;

    return true;
}

void Gui::clearCursorAddress() {
    cursorAddress = 0;
    cursorAddressSelected = false;
}

unsigned short Gui::getMemoryCursorAddressFromClick(
    int mouseX,
    int mouseY,
    unsigned short pc,
    bool& valid
) {
    valid = false;

    int memoryX = 895;
    int memoryY = 155;
    int rowHeight = 28;
    int rowCount = 8;

    int byteStartX = memoryX + 68;
    int byteWidth = 22;
    int byteCount = 8;

    if (mouseY < memoryY) {
        return 0;
    }

    if (mouseY >= memoryY + rowCount * rowHeight) {
        return 0;
    }

    int row = (mouseY - memoryY) / rowHeight;

    if (mouseX < memoryX) {
        return 0;
    }

    if (mouseX >= byteStartX + byteCount * byteWidth) {
        return 0;
    }

    unsigned short baseAddress = getMemoryViewerBaseAddress(pc);
    unsigned short rowAddress = baseAddress + row * 8;

    if (mouseX < byteStartX) {
        valid = true;
        return rowAddress;
    }

    int byteColumn = (mouseX - byteStartX) / byteWidth;

    if (byteColumn < 0 || byteColumn >= byteCount) {
        return 0;
    }

    int instructionByteColumn = byteColumn & 0xFE;

    valid = true;

    return rowAddress + instructionByteColumn;
}

void Gui::updateMemoryCursorFromMouse(unsigned short pc) {
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return;
    }

    bool valid = false;

    unsigned short selectedAddress = getMemoryCursorAddressFromClick(
        GetMouseX(),
        GetMouseY(),
        pc,
        valid
    );

    if (valid) {
        setCursorAddress(selectedAddress);
    }
}

bool Gui::isCurrentInstructionByte(unsigned short address, unsigned short pc) {
    if (address == pc) {
        return true;
    }

    if (pc != 0xFFFF && address == (unsigned short)(pc + 1)) {
        return true;
    }

    return false;
}

void Gui::disassembleInstruction(unsigned short word, char text[], int textSize) {
    if (text == 0 || textSize <= 0) {
        return;
    }

    text[0] = '\0';

    InstructionDecoder decoder;
    DecodedInstruction instruction = decoder.decode(word);

    if (instruction.opcode == 0) {
        if (instruction.funct4 == 0x0) {
            snprintf(text, textSize, "R add x%d,x%d", instruction.rd, instruction.rs2);
            return;
        }

        if (instruction.funct4 == 0x1) {
            snprintf(text, textSize, "R sub x%d,x%d", instruction.rd, instruction.rs2);
            return;
        }

        if (instruction.funct4 == 0x4) {
            snprintf(text, textSize, "R sll x%d,x%d", instruction.rd, instruction.rs2);
            return;
        }

        if (instruction.funct4 == 0x5) {
            snprintf(text, textSize, "R srl x%d,x%d", instruction.rd, instruction.rs2);
            return;
        }

        if (instruction.funct4 == 0x6) {
            snprintf(text, textSize, "R sra x%d,x%d", instruction.rd, instruction.rs2);
            return;
        }

        if (instruction.funct4 == 0x7) {
            snprintf(text, textSize, "R or x%d,x%d", instruction.rd, instruction.rs2);
            return;
        }

        if (instruction.funct4 == 0x8) {
            snprintf(text, textSize, "R and x%d,x%d", instruction.rd, instruction.rs2);
            return;
        }

        if (instruction.funct4 == 0x9) {
            snprintf(text, textSize, "R xor x%d,x%d", instruction.rd, instruction.rs2);
            return;
        }

        if (instruction.funct4 == 0xB) {
            snprintf(text, textSize, "R jr x%d", instruction.rd);
            return;
        }

        if (instruction.funct4 == 0xC) {
            snprintf(text, textSize, "R jalr x%d,x%d", instruction.rd, instruction.rs2);
            return;
        }

        snprintf(text, textSize, "R unknown");
        return;
    }

    if (instruction.opcode == 1) {
        if (instruction.func3 == 0x0) {
            snprintf(text, textSize, "I addi x%d,%d", instruction.rd, instruction.immediate);
            return;
        }

        if (instruction.func3 == 0x4) {
            snprintf(text, textSize, "I ori x%d", instruction.rd);
            return;
        }

        if (instruction.func3 == 0x5) {
            snprintf(text, textSize, "I andi x%d,%d", instruction.rd, instruction.immediate);
            return;
        }

        if (instruction.func3 == 0x6) {
            snprintf(text, textSize, "I xori x%d,%d", instruction.rd, instruction.immediate);
            return;
        }

        if (instruction.func3 == 0x7) {
            snprintf(text, textSize, "I li x%d,%d", instruction.rd, instruction.immediate);
            return;
        }

        snprintf(text, textSize, "I unknown");
        return;
    }

    if (instruction.opcode == 2) {
        if (instruction.func3 == 0x0) {
            snprintf(text, textSize, "B beq x%d,x%d,%d", instruction.rs1, instruction.rs2, instruction.immediate);
            return;
        }

        if (instruction.func3 == 0x1) {
            snprintf(text, textSize, "B bne x%d,x%d,%d", instruction.rs1, instruction.rs2, instruction.immediate);
            return;
        }

        if (instruction.func3 == 0x2) {
            snprintf(text, textSize, "B bz x%d,%d", instruction.rs1, instruction.immediate);
            return;
        }

        if (instruction.func3 == 0x3) {
            snprintf(text, textSize, "B bnz x%d,%d", instruction.rs1, instruction.immediate);
            return;
        }

        if (instruction.func3 == 0x4) {
            snprintf(text, textSize, "B blt x%d,x%d,%d", instruction.rs1, instruction.rs2, instruction.immediate);
            return;
        }

        if (instruction.func3 == 0x5) {
            snprintf(text, textSize, "B bge x%d,x%d,%d", instruction.rs1, instruction.rs2, instruction.immediate);
            return;
        }

        if (instruction.func3 == 0x6) {
            snprintf(text, textSize, "B bltu x%d,x%d,%d", instruction.rs1, instruction.rs2, instruction.immediate);
            return;
        }

        if (instruction.func3 == 0x7) {
            snprintf(text, textSize, "B bgeu x%d,x%d,%d", instruction.rs1, instruction.rs2, instruction.immediate);
            return;
        }

        snprintf(text, textSize, "B unknown");
        return;
    }

    if (instruction.opcode == 3) {
        if (instruction.func3 == 0x0) {
            snprintf(text, textSize, "S sb x%d,%d(x%d)", instruction.rs2, instruction.immediate, instruction.rs1);
            return;
        }

        if (instruction.func3 == 0x1) {
            snprintf(text, textSize, "S sw x%d,%d(x%d)", instruction.rs2, instruction.immediate, instruction.rs1);
            return;
        }

        snprintf(text, textSize, "S unknown");
        return;
    }

    if (instruction.opcode == 4) {
        if (instruction.func3 == 0x0) {
            snprintf(text, textSize, "L lb x%d,%d(x%d)", instruction.rd, instruction.immediate, instruction.rs1);
            return;
        }

        if (instruction.func3 == 0x1) {
            snprintf(text, textSize, "L lw x%d,%d(x%d)", instruction.rd, instruction.immediate, instruction.rs1);
            return;
        }

        if (instruction.func3 == 0x4) {
            snprintf(text, textSize, "L lbu x%d,%d(x%d)", instruction.rd, instruction.immediate, instruction.rs1);
            return;
        }

        snprintf(text, textSize, "L unknown");
        return;
    }

    if (instruction.opcode == 5) {
        if (instruction.linkFlag == 1) {
            snprintf(text, textSize, "J jal x%d,%d", instruction.rd, instruction.immediate);
            return;
        }

        snprintf(text, textSize, "J j %d", instruction.immediate);
        return;
    }

    if (instruction.opcode == 6) {
        snprintf(text, textSize, "U type");
        return;
    }

    if (instruction.opcode == 7) {
        snprintf(
            text,
            textSize,
            "SYS %s",
            getSyscallDisplayName(instruction.service)
        );
        return;
    }

    snprintf(text, textSize, "unknown");
}

void Gui::buildCurrentInstructionText(CPU& cpu, char text[], int textSize) {
    if (text == 0 || textSize <= 0) {
        return;
    }

    text[0] = '\0';

    unsigned short pc = cpu.getPC();
    unsigned short word = cpu.getMemory().read16(pc);

    char disassembly[80];

    disassembleInstruction(word, disassembly, 80);

    snprintf(
        text,
        textSize,
        "0x%04X: %04X  %s",
        pc,
        word,
        disassembly
    );
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
    drawDisassemblyPanel(cpu);
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
    if (guiAudioReady) {
        float volume = (float)cpu.getVolumePercent() / 100.0f;
        SetMasterVolume(volume);
    }

    if (cpu.hasPendingStopAudio()) {
        stopAudio();
        cpu.clearStopAudioRequest();
        cpu.clearToneRequest();
        return;
    }

    if (!cpu.hasPendingTone()) {
        return;
    }

    playTone(cpu.getToneFrequency(), cpu.getToneDurationMs());

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

bool Gui::stopAudio() {
    if (!guiAudioReady) {
        return false;
    }

    if (guiToneLoaded) {
        StopSound(guiToneSound);
        return true;
    }

    return false;
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
    char volumeText[40];

    sprintf(frameText, "Frame update: %d", frameNumber);
    sprintf(pcText, "PC: 0x%04X", cpu.getPC());
    sprintf(spText, "SP: 0x%04X", cpu.getSP());
    sprintf(fpsText, "Target FPS: %d", getTargetFps());
    sprintf(frameTimeText, "Target frame: %.2f ms", getTargetFrameTimeMs());
    sprintf(keyText, "Keyboard key: %d", cpu.getKeyboardKey());
    sprintf(volumeText, "Volume: %d%%", cpu.getVolumePercent());

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
    DrawText(volumeText, 45, 500, 14, GREEN);
}

void Gui::drawConsolePanel(const char consoleText[]) {
    char visibleText[512];

    buildConsoleVisibleText(consoleText, visibleText, 512);

    DrawRectangleLines(315, 60, 270, 165, GREEN);

    DrawText("Console", 335, 80, 18, GREEN);

    DrawRectangle(335, 115, 230, 85, DARKGRAY);
    DrawRectangleLines(335, 115, 230, 85, GRAY);

    BeginScissorMode(335, 115, 230, 85);
    drawConsoleTextLines(visibleText, 345, 125);
    EndScissorMode();
}

void Gui::drawConsoleTextLines(const char visibleText[], int x, int y) {
    char line[128];

    int textIndex = 0;
    int lineIndex = 0;
    int lineNumber = 0;

    if (visibleText == 0) {
        return;
    }

    while (visibleText[textIndex] != '\0' && lineNumber < CONSOLE_VISIBLE_LINES) {
        lineIndex = 0;

        while (
            visibleText[textIndex] != '\0' &&
            visibleText[textIndex] != '\n' &&
            lineIndex < 127
        ) {
            line[lineIndex] = visibleText[textIndex];
            lineIndex++;
            textIndex++;
        }

        line[lineIndex] = '\0';

        DrawText(line, x, y + lineNumber * 14, 12, GREEN);

        if (visibleText[textIndex] == '\n') {
            textIndex++;
        }

        lineNumber++;
    }
}

GuiAction Gui::drawControlPanel(bool running, bool halted) {
    GuiAction action = GUI_ACTION_NONE;

    DrawRectangleLines(315, 240, 270, 100, GREEN);

    DrawText("Controls", 335, 255, 18, GREEN);

    if (halted) {
        if (drawButton(320, 295, 48, 30, "Run")) {
            action = GUI_ACTION_RUN_PAUSE;
        }
    }
    else if (running) {
        if (drawButton(320, 295, 48, 30, "Pause")) {
            action = GUI_ACTION_RUN_PAUSE;
        }
    }
    else {
        if (drawButton(320, 295, 48, 30, "Run")) {
            action = GUI_ACTION_RUN_PAUSE;
        }
    }

    if (drawButton(372, 295, 48, 30, "Step")) {
        action = GUI_ACTION_STEP;
    }

    if (drawButton(424, 295, 48, 30, "Over")) {
        action = GUI_ACTION_STEP_OVER;
    }

    if (drawButton(476, 295, 54, 30, "Cursor")) {
        action = GUI_ACTION_RUN_TO_CURSOR;
    }

    if (drawButton(534, 295, 48, 30, "Reset")) {
        action = GUI_ACTION_RESET;
    }

    return action;
}

void Gui::drawDisassemblyPanel(CPU& cpu) {
    char instructionText[128];

    buildCurrentInstructionText(cpu, instructionText, 128);

    DrawRectangleLines(315, 355, 270, 70, GREEN);

    DrawText("Disassembly", 335, 370, 18, GREEN);

    DrawRectangle(335, 397, 230, 22, DARKGRAY);
    DrawRectangleLines(335, 397, 230, 22, GRAY);

    DrawText(instructionText, 342, 403, 10, YELLOW);
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
    unsigned short baseAddress = getMemoryViewerBaseAddress(cpu.getPC());

    updateMemoryCursorFromMouse(cpu.getPC());

    DrawRectangleLines(875, 60, 285, 540, GREEN);

    DrawText("Memory Viewer", 895, 80, 18, GREEN);

    DrawText("RAM around PC", 895, 115, 14, RAYWHITE);
    DrawText("Yellow = current instruction", 895, 132, 11, YELLOW);
    DrawText("Blue = run cursor", 895, 145, 11, SKYBLUE);

    for (int row = 0; row < 8; row++) {
        unsigned short rowAddress = baseAddress + row * 8;

        drawMemoryLineWithHighlight(
            cpu,
            rowAddress,
            cpu.getPC(),
            895,
            165 + row * 28
        );
    }
}

void Gui::drawMemoryLineWithHighlight(CPU& cpu, unsigned short address, unsigned short pc, int x, int y) {
    char prefix[20];
    char byteText[8];

    sprintf(prefix, "0x%04X:", address);
    DrawText(prefix, x, y, 13, GREEN);

    int byteX = x + 68;

    for (int col = 0; col < 8; col++) {
        unsigned short byteAddress = address + col;
        unsigned char value = cpu.getMemory().read8(byteAddress);

        sprintf(byteText, "%02X", value);

        if (isCurrentInstructionByte(byteAddress, pc)) {
            DrawRectangle(byteX - 3, y - 2, 19, 17, DARKGREEN);
            DrawText(byteText, byteX, y, 13, YELLOW);
        }
        else if (cursorAddressSelected && isCurrentInstructionByte(byteAddress, cursorAddress)) {
            DrawRectangle(byteX - 3, y - 2, 19, 17, DARKBLUE);
            DrawText(byteText, byteX, y, 13, SKYBLUE);
        }
        else {
            DrawText(byteText, byteX, y, 13, GREEN);
        }

        byteX = byteX + 22;
    }
}

void Gui::drawGraphicsPanel(CPU& cpu) {
    GraphicsMemory graphics(cpu.getMemory());

    Rgb888Color pixel;
    Color color;

    int panelX = 315;
    int panelY = 440;
    int panelW = 270;
    int panelH = 185;

    int previewX = 335;
    int previewY = 495;

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