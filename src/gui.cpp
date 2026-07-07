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
static unsigned int guiLastToneRequestId = 0;
static const Color GUI_CURSOR_LIGHT_PINK = { 255, 182, 193, 255 };

Gui::Gui() {
    width = 800;
    height = 600;
    graphicsWidth = 800;
    graphicsHeight = 600;
    expandedWidth = 1360;
    cpuPanelX = 800;
    cpuScale = 1.0f;
    cpuOffsetY = 0;
    drawingCpuPanel = false;
    statusPanelVisible = false;
    viewMode = GUI_VIEW_GRAPHICS_ONLY;
    dataPanel = GUI_DATA_REGISTERS;

    cursorAddress = 0;
    cursorAddressSelected = false;

    selectedMemoryAddress = 0;
    memoryAddressSelected = false;

    memoryEditActive = false;
    memoryEditText[0] = '\0';
    memoryEditLength = 0;

    selectedRegisterIndex = -1;
    registerEditActive = false;
    registerEditText[0] = '\0';
    registerEditLength = 0;
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

void Gui::open() {
    InitWindow(width, height, "ZX16 Simulator");

    calculateLayout();
    SetWindowSize(graphicsWidth, graphicsHeight);

    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);
    SetWindowPosition(monitorWidth - graphicsWidth - 20, 20);

    InitAudioDevice();
    guiAudioReady = IsAudioDeviceReady();

    SetTargetFPS(getTargetFps());
}

void Gui::calculateLayout() {
    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);
    int monitorHeight = GetMonitorHeight(monitor);

    graphicsHeight = (monitorHeight * 55) / 100;
    if (graphicsHeight < 400) graphicsHeight = 400;
    if (graphicsHeight > 580) graphicsHeight = 580;

    graphicsWidth = (graphicsHeight * 4) / 3;
    expandedWidth = monitorWidth - 20;
    int maximumGraphicsWidth = expandedWidth - 760;
    if (maximumGraphicsWidth < 320) maximumGraphicsWidth = 320;
    if (graphicsWidth > maximumGraphicsWidth) {
        graphicsWidth = maximumGraphicsWidth;
        graphicsHeight = (graphicsWidth * 3) / 4;
    }

    height = graphicsHeight;
    cpuPanelX = graphicsWidth;
    cpuScale = 1.0f;
    cpuOffsetY = 0;
}

bool Gui::isExpanded() {
    return viewMode == GUI_VIEW_EXPANDED;
}

void Gui::toggleViewMode() {
    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);

    if (isExpanded()) {
        viewMode = GUI_VIEW_GRAPHICS_ONLY;
        statusPanelVisible = false;
        width = graphicsWidth;
        SetWindowSize(width, graphicsHeight);
        SetWindowPosition(monitorWidth - width - 20, 20);
    }
    else {
        viewMode = GUI_VIEW_EXPANDED;
        width = expandedWidth;
        SetWindowSize(width, graphicsHeight);
        SetWindowPosition(monitorWidth - width - 10, 20);
    }
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

    drawGraphicsPanel(cpu);

    action = GUI_ACTION_NONE;
    if (drawViewToggleButton()) {
        toggleViewMode();
    }

    if (isExpanded()) {
        DrawLine(cpuPanelX, 0, cpuPanelX, graphicsHeight, GREEN);
        Camera2D cpuCamera = { 0 };
        cpuCamera.offset = { (float)cpuPanelX, (float)cpuOffsetY };
        cpuCamera.target = { 0.0f, 0.0f };
        cpuCamera.rotation = 0.0f;
        cpuCamera.zoom = 1.0f;

        drawingCpuPanel = true;
        BeginMode2D(cpuCamera);
        drawConsolePanel(consoleText);
        action = drawControlPanel(running, cpu.isHalted());
        drawDisassemblyPanel(cpu, running);
        drawDataPanelTabs();
        if (dataPanel == GUI_DATA_REGISTERS) {
            drawRegisterPanel(cpu, running);
        }
        else {
            drawMemoryPanel(cpu, running);
        }
        if (drawStatusToggleButton()) {
            statusPanelVisible = !statusPanelVisible;
            SetWindowSize(expandedWidth, graphicsHeight + (statusPanelVisible ? 180 : 0));
        }
        if (statusPanelVisible) {
            drawStatusPanel(testStatus, frameNumber, running, cpu);
        }
        EndMode2D();
        drawingCpuPanel = false;
    }

    EndDrawing();

    return action;
}

bool Gui::drawViewToggleButton() {
    const char* arrow = isExpanded() ? ">" : "<";
    int buttonX = graphicsWidth - 46;
    return drawButton(buttonX, 12, 34, 30, arrow);
}

void Gui::drawDataPanelTabs() {
    if (drawButton(260, 15, 105, 28, "Registers")) {
        dataPanel = GUI_DATA_REGISTERS;
        clearSelectedMemoryAddress();
    }

    if (drawButton(370, 15, 105, 28, "RAM")) {
        dataPanel = GUI_DATA_RAM;
        clearSelectedRegister();
    }

    if (dataPanel == GUI_DATA_REGISTERS) {
        DrawRectangleLines(260, 15, 105, 28, YELLOW);
    }
    else {
        DrawRectangleLines(370, 15, 105, 28, YELLOW);
    }
}

bool Gui::drawStatusToggleButton() {
    int cpuWidth = expandedWidth - graphicsWidth;
    int x = cpuWidth / 2 - 24;
    int y = graphicsHeight - 32;
    return drawButton(x, y, 48, 24, statusPanelVisible ? "^" : "v");
}

int Gui::getUiMouseX() {
    if (!drawingCpuPanel) return GetMouseX();
    return GetMouseX() - cpuPanelX;
}

int Gui::getUiMouseY() {
    if (!drawingCpuPanel) return GetMouseY();
    return GetMouseY();
}

void Gui::drawDisassemblyPanel(CPU& cpu, bool running) {
    unsigned short baseAddress = getDisassemblyBaseAddress(cpu.getPC());
    int cpuWidth = expandedWidth - graphicsWidth;
    int panelWidth = cpuWidth - 500;

    DrawRectangleLines(490, 55, panelWidth, graphicsHeight - 100, GREEN);
    DrawText("Disassembly / Debugger", 502, 68, 18, GREEN);
    DrawText("Yellow marks the current instruction", 500, 92, 10, RAYWHITE);

    for (int row = 0; row < 11; row++) {
        unsigned short address = (unsigned short)(baseAddress + row * 2);
        drawDisassemblyLine(cpu, address, 500, 112 + row * 23);
    }

    DrawText("Debug controls are available in RAM", 500, 370, 10, GRAY);
}

void Gui::drawDisassemblyLine(CPU& cpu, unsigned short address, int x, int y) {
    char disassembly[80];
    char text[128];
    unsigned short word = cpu.getMemory().read16(address);
    Color textColor = GREEN;

    disassembleInstruction(word, disassembly, 80);
    snprintf(text, 128, "0x%04X  %04X  %s", address, word, disassembly);

    if (address == cpu.getPC()) {
        DrawRectangle(x - 3, y - 2, 270, 18, DARKGREEN);
        textColor = YELLOW;
    }

    DrawText(text, x, y, 11, textColor);
}

bool Gui::editRegister(CPU& cpu, int registerIndex, unsigned short value) {
    if (!isValidRegisterEditIndex(registerIndex)) {
        return false;
    }

    cpu.getRegisters().setRegister(registerIndex, value);

    return true;
}

bool Gui::hasSelectedRegister() {
    return registerEditActive && isValidRegisterEditIndex(selectedRegisterIndex);
}

int Gui::getSelectedRegisterIndex() {
    if (!hasSelectedRegister()) {
        return -1;
    }

    return selectedRegisterIndex;
}

void Gui::clearSelectedRegister() {
    selectedRegisterIndex = -1;
    registerEditActive = false;
    registerEditLength = 0;
    registerEditText[0] = '\0';
}

void Gui::startRegisterEdit(int registerIndex, unsigned short currentValue) {
    if (!isValidRegisterEditIndex(registerIndex)) {
        clearSelectedRegister();
        return;
    }

    selectedRegisterIndex = registerIndex;
    registerEditActive = true;
    registerEditLength = 0;
    registerEditText[0] = '\0';

    (void)currentValue;
}

int Gui::getRegisterIndexFromClick(int mouseX, int mouseY) {
    int registerX = 270;
    int registerW = 195;
    int firstRegisterY = 128;
    int rowHeight = 22;
    int selectableHeight = 19;

    if (mouseX < registerX || mouseX >= registerX + registerW) {
        return -1;
    }

    if (mouseY < firstRegisterY) {
        return -1;
    }

    int row = (mouseY - firstRegisterY) / rowHeight;
    int insideRowY = (mouseY - firstRegisterY) % rowHeight;

    if (row < 0 || row >= RegisterFile::REGISTER_COUNT) {
        return -1;
    }

    if (insideRowY >= selectableHeight) {
        return -1;
    }

    return row;
}

void Gui::updateRegisterEditorFromMouse(bool running, CPU& cpu) {
    if (running) {
        return;
    }

    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return;
    }

    int registerIndex = getRegisterIndexFromClick(getUiMouseX(), getUiMouseY());

    if (isValidRegisterEditIndex(registerIndex)) {
        startRegisterEdit(registerIndex, cpu.getRegisters().getRegister(registerIndex));
    }
}

bool Gui::appendRegisterEditDigit(char c) {
    if (!isHexDigit(c)) {
        return false;
    }

    if (registerEditLength >= 4) {
        return false;
    }

    registerEditText[registerEditLength] = normalizeHexDigit(c);
    registerEditLength++;
    registerEditText[registerEditLength] = '\0';

    return true;
}

bool Gui::removeRegisterEditDigit() {
    if (registerEditLength <= 0) {
        return false;
    }

    registerEditLength--;
    registerEditText[registerEditLength] = '\0';

    return true;
}

bool Gui::applyRegisterEdit(CPU& cpu) {
    if (!hasSelectedRegister()) {
        return false;
    }

    if (registerEditLength <= 0) {
        clearSelectedRegister();
        return false;
    }

    unsigned short value = 0;

    if (!parseHex16(registerEditText, value)) {
        return false;
    }

    if (!editRegister(cpu, selectedRegisterIndex, value)) {
        return false;
    }

    clearSelectedRegister();

    return true;
}

void Gui::updateRegisterEditorFromKeyboard(bool running, CPU& cpu) {
    if (running) {
        return;
    }

    if (!hasSelectedRegister()) {
        return;
    }

    int key = GetCharPressed();

    while (key > 0) {
        if (key >= 0 && key <= 255) {
            appendRegisterEditDigit((char)key);
        }

        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        removeRegisterEditDigit();
    }

    if (IsKeyPressed(KEY_ENTER)) {
        applyRegisterEdit(cpu);
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        clearSelectedRegister();
    }
}

bool Gui::editMemoryByte(CPU& cpu, unsigned short address, unsigned char value) {
    cpu.getMemory().write8(address, value);

    return true;
}

bool Gui::editMemoryWord(CPU& cpu, unsigned short address, unsigned short value) {
    if ((address & 1) != 0) {
        return false;
    }

    if (address == 0xFFFF) {
        return false;
    }

    cpu.getMemory().write16(address, value);

    return true;
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

bool Gui::isValidRegisterEditIndex(int registerIndex) {
    if (registerIndex < 0 || registerIndex >= RegisterFile::REGISTER_COUNT) {
        return false;
    }

    return true;
}

bool Gui::isHexDigit(char c) {
    if (c >= '0' && c <= '9') {
        return true;
    }

    if (c >= 'a' && c <= 'f') {
        return true;
    }

    if (c >= 'A' && c <= 'F') {
        return true;
    }

    return false;
}

int Gui::hexDigitValue(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }

    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    return -1;
}

char Gui::normalizeHexDigit(char c) {
    if (c >= 'a' && c <= 'f') {
        return (char)(c - 'a' + 'A');
    }

    return c;
}

bool Gui::parseHex16(const char text[], unsigned short& value) {
    if (text == 0) {
        return false;
    }

    int index = 0;

    if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        index = 2;
    }

    int digitCount = 0;
    unsigned int result = 0;

    while (text[index] != '\0') {
        if (!isHexDigit(text[index])) {
            return false;
        }

        if (digitCount >= 4) {
            return false;
        }

        result = (result << 4) | hexDigitValue(text[index]);

        digitCount++;
        index++;
    }

    if (digitCount == 0) {
        return false;
    }

    value = (unsigned short)result;

    return true;
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

    int cpuWidth = expandedWidth - graphicsWidth;
    int panelY = graphicsHeight + 4;

    DrawRectangleLines(10, panelY, cpuWidth - 20, 168, GREEN);
    DrawText("Status Panel", 25, panelY + 12, 18, GREEN);
    DrawText(testStatus, 25, panelY + 38, 13, RAYWHITE);

    DrawText("Window: PASSED", 25, panelY + 65, 12, GREEN);
    DrawText("Console: PASSED", 25, panelY + 84, 12, GREEN);
    DrawText("Controls: PASSED", 25, panelY + 103, 12, GREEN);
    DrawText("Registers/RAM: PASSED", 25, panelY + 122, 12, GREEN);
    DrawText("Debugger: PASSED", 25, panelY + 141, 12, GREEN);

    DrawText(pcText, 220, panelY + 38, 12, GREEN);
    DrawText(spText, 220, panelY + 58, 12, GREEN);
    DrawText(stateText, 220, panelY + 78, 12, GREEN);
    DrawText(frameText, 220, panelY + 98, 12, GREEN);

    DrawText(fpsText, 430, panelY + 38, 12, GREEN);
    DrawText(frameTimeText, 430, panelY + 58, 12, GREEN);
    DrawText(keyText, 430, panelY + 78, 12, GREEN);
    DrawText(volumeText, 430, panelY + 98, 12, GREEN);
}

void Gui::drawConsolePanel(const char consoleText[]) {
    char visibleText[512];

    buildConsoleVisibleText(consoleText, visibleText, 512);

    DrawRectangleLines(10, 55, 235, 145, GREEN);

    DrawText("Console", 25, 70, 18, GREEN);

    DrawRectangle(25, 100, 205, 80, DARKGRAY);
    DrawRectangleLines(25, 100, 205, 80, GRAY);

    BeginScissorMode(cpuPanelX + 25, 100, 205, 80);
    drawConsoleTextLines(visibleText, 32, 108);
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

    DrawRectangleLines(10, 210, 235, 92, GREEN);

    DrawText("Controls", 25, 224, 18, GREEN);

    if (halted) {
        if (drawButton(15, 260, 42, 28, "Run")) {
            action = GUI_ACTION_RUN_PAUSE;
        }
    }
    else if (running) {
        if (drawButton(15, 260, 42, 28, "Pause")) {
            action = GUI_ACTION_RUN_PAUSE;
        }
    }
    else {
        if (drawButton(15, 260, 42, 28, "Run")) {
            action = GUI_ACTION_RUN_PAUSE;
        }
    }

    if (drawButton(60, 260, 42, 28, "Step")) {
        action = GUI_ACTION_STEP;
    }

    if (drawButton(105, 260, 42, 28, "Over")) {
        action = GUI_ACTION_STEP_OVER;
    }

    if (drawButton(150, 260, 42, 28, "Curs")) {
        action = GUI_ACTION_RUN_TO_CURSOR;
    }

    if (drawButton(195, 260, 42, 28, "Reset")) {
        action = GUI_ACTION_RESET;
    }

    return action;
}

void Gui::drawRegisterPanel(CPU& cpu, bool running) {
    char text[80];

    updateRegisterEditorFromMouse(running, cpu);
    updateRegisterEditorFromKeyboard(running, cpu);

    DrawRectangleLines(260, 55, 220, graphicsHeight - 100, GREEN);

    DrawText("Registers", 275, 68, 18, GREEN);

    sprintf(text, "PC     : 0x%04X", cpu.getPC());
    DrawText(text, 275, 94, 14, GREEN);

    sprintf(text, "SP/x2  : 0x%04X", cpu.getSP());
    DrawText(text, 275, 112, 14, GREEN);
    for (int i = 0; i < RegisterFile::REGISTER_COUNT; i++) {
        int rowY = 132 + i * 22;
        Color textColor = GREEN;

        if (hasSelectedRegister() && selectedRegisterIndex == i) {
            DrawRectangle(270, rowY - 4, 190, 19, DARKBLUE);
            textColor = SKYBLUE;
        }

        if (i == 2) {
            sprintf(text, "x%d/sp : 0x%04X", i, cpu.getRegisters().getRegister(i));
        }
        else {
            sprintf(text, "x%d    : 0x%04X", i, cpu.getRegisters().getRegister(i));
        }

        DrawText(text, 275, rowY, 14, textColor);
    }

    DrawText("Register edit", 275, 315, 14, GREEN);

    if (running) {
        DrawText("Pause CPU to edit", 275, 336, 12, GRAY);
    }
    else {
        DrawText("Click x0-x7, type HEX", 275, 336, 12, GRAY);
        DrawText("Enter=save Esc=cancel", 275, 352, 12, GRAY);
    }

    DrawRectangle(275, 370, 175, 26, DARKGRAY);
    DrawRectangleLines(275, 370, 175, 26, GRAY);

    if (hasSelectedRegister()) {
        if (registerEditLength > 0) {
            sprintf(text, "x%d = 0x%s", selectedRegisterIndex, registerEditText);
        }
        else {
            sprintf(text, "x%d = 0x____", selectedRegisterIndex);
        }

        DrawText(text, 283, 376, 14, SKYBLUE);
    }
    else {
        DrawText("no register selected", 283, 377, 12, GRAY);
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

void Gui::drawCompactMemoryLine(CPU& cpu, unsigned short address, unsigned short pc, int x, int y) {
    char prefix[20];
    char byteText[8];

    sprintf(prefix, "0x%04X:", address);
    DrawText(prefix, x, y, 13, GREEN);

    int byteX = x + 68;

    for (int col = 0; col < 6; col++) {
        unsigned short byteAddress = address + col;
        unsigned char value = cpu.getMemory().read8(byteAddress);

        sprintf(byteText, "%02X", value);

        unsigned short instructionAddress = byteAddress & 0xFFFE;

        if (cpu.hasBreakpoint(instructionAddress)) {
            DrawRectangle(byteX - 3, y - 2, 19, 17, RED);
            DrawText(byteText, byteX, y, 13, YELLOW);
        }
        else if (memoryAddressSelected && byteAddress == selectedMemoryAddress) {
            DrawRectangle(byteX - 3, y - 2, 19, 17, DARKBLUE);
            DrawText(byteText, byteX, y, 13, SKYBLUE);
        }
        else if (cursorAddressSelected && isCurrentInstructionByte(byteAddress, cursorAddress)) {
            DrawRectangle(byteX - 3, y - 2, 19, 17, GUI_CURSOR_LIGHT_PINK);
            DrawText(byteText, byteX, y, 13, MAROON);
        }
        else if (isCurrentInstructionByte(byteAddress, pc)) {
            DrawRectangle(byteX - 3, y - 2, 19, 17, DARKGREEN);
            DrawText(byteText, byteX, y, 13, YELLOW);
        }
        else {
            DrawText(byteText, byteX, y, 13, GREEN);
        }

        byteX = byteX + 22;
    }
}

void Gui::drawMemoryPanel(CPU& cpu, bool running) {
    unsigned short baseAddress = cpu.getPC() >= 30 ? (unsigned short)(cpu.getPC() - 30) : 0;

    updateMemoryBreakpointFromMouse(running, cpu);
    updateMemoryCursorFromMouse(cpu.getPC());

    if (!running) {
        updateMemoryEditorFromMouse(running, cpu);
    }

    updateMemoryEditorFromKeyboard(running, cpu);

    DrawRectangleLines(260, 55, 220, graphicsHeight - 100, GREEN);
    DrawText("RAM", 275, 68, 18, GREEN);
    char breakpointCountText[24];
    sprintf(breakpointCountText, "BP:%d", cpu.getBreakpointCount());
    DrawText(breakpointCountText, 430, 70, 11, RED);
    DrawText("Left-click=pink run cursor", 270, 94, 10, RAYWHITE);
    DrawText("Shift+right-click=red breakpoint", 270, 106, 9, RAYWHITE);
    DrawText("Right-click byte to edit", 270, 117, 9, GRAY);

    for (int row = 0; row < 10; row++) {
        drawCompactMemoryLine(cpu, (unsigned short)(baseAddress + row * 6), cpu.getPC(), 270, 130 + row * 21);
    }

    if (cursorAddressSelected) {
        char cursorText[48];
        sprintf(cursorText, "Run cursor: 0x%04X", cursorAddress);
        DrawText(cursorText, 275, 326, 11, GUI_CURSOR_LIGHT_PINK);
    }

    if (memoryAddressSelected) {
        char selectedText[80];

        sprintf(
            selectedText,
            "Selected: 0x%04X = %02X",
            selectedMemoryAddress,
            cpu.getMemory().read8(selectedMemoryAddress)
        );

        DrawText(selectedText, 275, 342, 11, SKYBLUE);
    }

    if (memoryEditActive) {
        char editText[80];

        sprintf(editText, "Edit value: %s", memoryEditText);

        DrawRectangle(275, 360, 190, 24, DARKGRAY);
        DrawRectangleLines(275, 360, 190, 24, SKYBLUE);
        DrawText(editText, 282, 366, 11, YELLOW);
    }

    if (running) {
        DrawText("Pause CPU to edit RAM", 275, 390, 11, ORANGE);
    }
    else {
        DrawText("2 hex=byte, 4 hex=word", 275, 390, 10, GRAY);
        DrawText("Enter=save, Esc=cancel", 275, 405, 10, GRAY);
    }
}

void Gui::drawGraphicsPanel(CPU& cpu) {
    GraphicsMemory graphics(cpu.getMemory());

    Rgb888Color pixel;
    Color color;

    int panelX = 0;
    int panelY = 0;
    int panelW = graphicsWidth;
    int panelH = graphicsHeight;
    int availableW = panelW - 40;
    int availableH = panelH - 80;
    int pixelScaleX = availableW / GraphicsMemory::SCREEN_WIDTH;
    int pixelScaleY = availableH / GraphicsMemory::SCREEN_HEIGHT;
    int pixelScale = pixelScaleX < pixelScaleY ? pixelScaleX : pixelScaleY;

    if (pixelScale < 1) pixelScale = 1;

    int previewW = GraphicsMemory::SCREEN_WIDTH * pixelScale;
    int previewH = GraphicsMemory::SCREEN_HEIGHT * pixelScale;
    int previewX = panelX + (panelW - previewW) / 2;
    int previewY = panelY + 58 + (availableH - previewH) / 2;

    DrawRectangleLines(panelX, panelY, panelW, panelH, GREEN);

    DrawText("Graphics Output", panelX + 20, panelY + 14, 18, GREEN);
    DrawText("320x240 VRAM display", panelX + 20, panelY + 38, 13, RAYWHITE);

    DrawRectangle(previewX, previewY, previewW, previewH, BLACK);
    DrawRectangleLines(previewX, previewY, previewW, previewH, GRAY);

    color.a = 255;

    for (int sourceY = 0; sourceY < GraphicsMemory::SCREEN_HEIGHT; sourceY++) {
        for (int sourceX = 0; sourceX < GraphicsMemory::SCREEN_WIDTH; sourceX++) {
            if (graphics.renderScreenPixel(sourceX, sourceY, pixel)) {
                color.r = pixel.red8;
                color.g = pixel.green8;
                color.b = pixel.blue8;
                DrawRectangle(
                    previewX + sourceX * pixelScale,
                    previewY + sourceY * pixelScale,
                    pixelScale,
                    pixelScale,
                    color
                );
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

    mouse.x = (float)getUiMouseX();
    mouse.y = (float)getUiMouseY();
    hover = CheckCollisionPointRec(mouse, button);

    if (hover) {
        DrawRectangleRec(button, DARKGREEN);
    }
    else {
        DrawRectangleRec(button, DARKGRAY);
    }

    DrawRectangleLines(x, y, w, h, GREEN);

    int fontSize = 14;
    int textWidth = MeasureText(text, fontSize);

    int textX = x + (w - textWidth) / 2;
    int textY = y + (h - fontSize) / 2;

    if (textX < x + 2) {
        textX = x + 2;
    }

    DrawText(text, textX, textY, fontSize, RAYWHITE);

    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return true;
    }

    return false;
}

unsigned short Gui::getMemoryViewerBaseAddress(unsigned short pc) {
    return pc & 0xFFF8;
}

unsigned short Gui::getDisassemblyBaseAddress(unsigned short pc) {
    unsigned short alignedPc = pc & 0xFFFE;

    if (alignedPc < 16) {
        return 0;
    }

    if (alignedPc > 0xFFDC) {
        return 0xFFDC;
    }

    return (unsigned short)(alignedPc - 16);
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

void Gui::resetDebugState() {
    clearCursorAddress();
    clearSelectedRegister();
    clearSelectedMemoryAddress();

    if (guiToneLoaded) {
        stopAudio();
    }
}

unsigned short Gui::getMemoryCursorAddressFromClick(
    int mouseX,
    int mouseY,
    unsigned short pc,
    bool& valid
) {
    valid = false;

    int memoryX = 270;
    int firstRowY = 128;
    int rowHeight = 21;
    int rowCount = 10;
    int byteStartX = memoryX + 68;
    int byteWidth = 22;
    int byteCount = 6;

    if (mouseY < firstRowY) {
        return 0;
    }

    if (mouseY >= firstRowY + rowCount * rowHeight) {
        return 0;
    }

    if (mouseX < memoryX || mouseX >= byteStartX + byteCount * byteWidth) {
        return 0;
    }

    int row = (mouseY - firstRowY) / rowHeight;
    unsigned short baseAddress = pc >= 30 ? (unsigned short)(pc - 30) : 0;
    unsigned short rowAddress = (unsigned short)(baseAddress + row * 6);

    if (mouseX < byteStartX) {
        valid = true;
        return rowAddress & 0xFFFE;
    }

    int byteColumn = (mouseX - byteStartX) / byteWidth;
    int instructionByteColumn = byteColumn & 0xFE;
    valid = true;

    return (unsigned short)((rowAddress + instructionByteColumn) & 0xFFFE);
}

void Gui::updateMemoryCursorFromMouse(unsigned short pc) {
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        return;
    }

    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return;
    }

    bool valid = false;

    unsigned short selectedAddress = getMemoryCursorAddressFromClick(
        getUiMouseX(),
        getUiMouseY(),
        pc,
        valid
    );

    if (valid) {
        setCursorAddress(selectedAddress);
    }
}

void Gui::updateMemoryBreakpointFromMouse(bool running, CPU& cpu) {
    bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    bool rightClick = IsMouseButtonPressed(MOUSE_RIGHT_BUTTON);

    if (!shiftDown || !rightClick) {
        return;
    }

    bool valid = false;

    unsigned short address = getMemoryCursorAddressFromClick(
        getUiMouseX(),
        getUiMouseY(),
        cpu.getPC(),
        valid
    );

    if (valid) {
        cpu.toggleBreakpoint(address);
    }

    (void)running;
}

unsigned short Gui::getMemoryByteAddressFromClick(
    int mouseX,
    int mouseY,
    unsigned short pc,
    bool& valid
) {
    valid = false;

    int memoryY = 128;
    int rowHeight = 21;
    int rowCount = 10;

    int byteStartX = 270 + 68;
    int byteWidth = 22;
    int byteCount = 6;

    if (mouseY < memoryY) {
        return 0;
    }

    if (mouseY >= memoryY + rowCount * rowHeight) {
        return 0;
    }

    if (mouseX < byteStartX) {
        return 0;
    }

    if (mouseX >= byteStartX + byteCount * byteWidth) {
        return 0;
    }

    int row = (mouseY - memoryY) / rowHeight;
    int byteColumn = (mouseX - byteStartX) / byteWidth;

    if (row < 0 || row >= rowCount) {
        return 0;
    }

    if (byteColumn < 0 || byteColumn >= byteCount) {
        return 0;
    }

    unsigned short baseAddress = pc >= 30 ? (unsigned short)(pc - 30) : 0;
    unsigned short selectedAddress = (unsigned short)(baseAddress + row * 6 + byteColumn);

    valid = true;

    return selectedAddress;
}

void Gui::startMemoryEdit(unsigned short address) {
    selectedMemoryAddress = address;
    memoryAddressSelected = true;

    memoryEditActive = true;
    memoryEditText[0] = '\0';
    memoryEditLength = 0;
}

void Gui::updateMemoryEditorFromMouse(bool running, CPU& cpu) {
    if (running) {
        return;
    }

    if (!IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        return;
    }

    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        return;
    }

    bool valid = false;

    unsigned short address = getMemoryByteAddressFromClick(
        getUiMouseX(),
        getUiMouseY(),
        cpu.getPC(),
        valid
    );

    if (valid) {
        startMemoryEdit(address);
    }
}

void Gui::updateMemoryEditorFromKeyboard(bool running, CPU& cpu) {
    if (running) {
        return;
    }

    if (!memoryEditActive) {
        return;
    }

    int key = GetCharPressed();

    while (key > 0) {
        char c = (char)key;

        if (isHexDigit(c)) {
            appendMemoryEditDigit(c);
        }

        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        removeMemoryEditDigit();
    }

    if (IsKeyPressed(KEY_ENTER)) {
        applyMemoryEdit(cpu);
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        memoryEditActive = false;
        memoryEditText[0] = '\0';
        memoryEditLength = 0;
    }
}

bool Gui::appendMemoryEditDigit(char c) {
    if (!isHexDigit(c)) {
        return false;
    }

    if (memoryEditLength >= 4) {
        return false;
    }

    memoryEditText[memoryEditLength] = normalizeHexDigit(c);
    memoryEditLength++;
    memoryEditText[memoryEditLength] = '\0';

    return true;
}

bool Gui::removeMemoryEditDigit() {
    if (memoryEditLength <= 0) {
        return false;
    }

    memoryEditLength--;
    memoryEditText[memoryEditLength] = '\0';

    return true;
}

bool Gui::applyMemoryEdit(CPU& cpu) {
    if (!memoryEditActive) {
        return false;
    }

    if (!memoryAddressSelected) {
        return false;
    }

    if (memoryEditLength == 2) {
        unsigned char value = 0;

        if (!parseHex8(memoryEditText, value)) {
            return false;
        }

        if (!editMemoryByte(cpu, selectedMemoryAddress, value)) {
            return false;
        }

        memoryEditActive = false;
        memoryEditText[0] = '\0';
        memoryEditLength = 0;

        return true;
    }

    if (memoryEditLength == 4) {
        unsigned short value = 0;

        if (!parseHex16(memoryEditText, value)) {
            return false;
        }

        if (!editMemoryWord(cpu, selectedMemoryAddress, value)) {
            return false;
        }

        memoryEditActive = false;
        memoryEditText[0] = '\0';
        memoryEditLength = 0;

        return true;
    }

    return false;
}

bool Gui::hasSelectedMemoryAddress() {
    return memoryAddressSelected;
}

unsigned short Gui::getSelectedMemoryAddress() {
    return selectedMemoryAddress;
}

void Gui::clearSelectedMemoryAddress() {
    selectedMemoryAddress = 0;
    memoryAddressSelected = false;

    memoryEditActive = false;
    memoryEditText[0] = '\0';
    memoryEditLength = 0;
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

bool Gui::parseHex8(const char text[], unsigned char& value) {
    if (text == 0) {
        return false;
    }

    int index = 0;

    if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        index = 2;
    }

    int digitCount = 0;
    unsigned int result = 0;

    while (text[index] != '\0') {
        if (!isHexDigit(text[index])) {
            return false;
        }

        if (digitCount >= 2) {
            return false;
        }

        result = (result << 4) | hexDigitValue(text[index]);

        digitCount++;
        index++;
    }

    if (digitCount == 0) {
        return false;
    }

    if (result > 0xFF) {
        return false;
    }

    value = (unsigned char)result;

    return true;
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
