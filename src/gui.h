#ifndef GUI_H
#define GUI_H

class CPU;

enum GuiAction {
    GUI_ACTION_NONE = 0,
    GUI_ACTION_RUN_PAUSE = 1,
    GUI_ACTION_STEP = 2,
    GUI_ACTION_RESET = 3
};

class Gui {
public:
    Gui();

    static int getTargetFps();
    static float getTargetFrameTimeMs();
    static bool isStableFrameTimeMs(float frameTimeMs);

    static unsigned short mapRaylibKeyToZx16(int raylibKey);

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

    void updateKeyboardFromRaylib(CPU& cpu);

    void updateAudioFromCpu(CPU& cpu);
    bool playTone(unsigned short frequency, unsigned short durationMs);

    void drawStatusPanel(
        const char testStatus[],
        int frameNumber,
        bool running,
        CPU& cpu
    );

    void drawConsolePanel(const char consoleText[]);

    GuiAction drawControlPanel(bool running, bool halted);

    void drawRegisterPanel(CPU& cpu);

    void drawMemoryPanel(CPU& cpu);

    void drawGraphicsPanel(CPU& cpu);

    bool drawButton(int x, int y, int w, int h, const char text[]);
};

#endif