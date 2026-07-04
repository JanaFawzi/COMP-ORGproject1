#ifndef GUI_H
#define GUI_H

class CPU;

enum GuiAction {
    GUI_ACTION_NONE = 0,
    GUI_ACTION_RUN_PAUSE = 1,
    GUI_ACTION_STEP = 2,
    GUI_ACTION_STEP_OVER = 3,
    GUI_ACTION_RUN_TO_CURSOR = 4,
    GUI_ACTION_RESET = 5
};

class Gui {
public:
    Gui();

    static int getTargetFps();
    static float getTargetFrameTimeMs();
    static bool isStableFrameTimeMs(float frameTimeMs);

    static unsigned short mapRaylibKeyToZx16(int raylibKey);

    static int getConsoleVisibleLineCount();
    static void buildConsoleVisibleText(const char consoleText[], char visibleText[], int visibleSize);

    static unsigned short getMemoryViewerBaseAddress(unsigned short pc);
    static bool isCurrentInstructionByte(unsigned short address, unsigned short pc);

    static void disassembleInstruction(unsigned short word, char text[], int textSize);
    static void buildCurrentInstructionText(CPU& cpu, char text[], int textSize);

    bool hasCursorAddress();
    unsigned short getCursorAddress();
    bool setCursorAddress(unsigned short address);
    void clearCursorAddress();

    static unsigned short getMemoryCursorAddressFromClick(
        int mouseX,
        int mouseY,
        unsigned short pc,
        bool& valid
    );

    void open();

    bool shouldClose();

    GuiAction draw(
        const char testStatus[],
        const char consoleText[],
        int frameNumber,
        bool running,
        CPU& cpu
    );

    void formatMemoryLine(CPU& cpu, unsigned short address, char text[]);

    void close();

private:
    int width;
    int height;

    static const int TARGET_FPS = 60;
    static const int CONSOLE_VISIBLE_LINES = 5;

    void updateKeyboardFromRaylib(CPU& cpu);

    void updateAudioFromCpu(CPU& cpu);
    bool playTone(unsigned short frequency, unsigned short durationMs);
    bool stopAudio();

    unsigned short cursorAddress;
    bool cursorAddressSelected;
    void updateMemoryCursorFromMouse(unsigned short pc);

    void drawStatusPanel(
        const char testStatus[],
        int frameNumber,
        bool running,
        CPU& cpu
    );

    void drawConsolePanel(const char consoleText[]);
    void drawConsoleTextLines(const char visibleText[], int x, int y);

    GuiAction drawControlPanel(bool running, bool halted);

    void drawDisassemblyPanel(CPU& cpu);

    void drawRegisterPanel(CPU& cpu);

    void drawMemoryPanel(CPU& cpu);
    void drawMemoryLineWithHighlight(
        CPU& cpu,
        unsigned short address,
        unsigned short pc,
        int x,
        int y
    );

    void drawGraphicsPanel(CPU& cpu);

    bool drawButton(int x, int y, int w, int h, const char text[]);
};

#endif