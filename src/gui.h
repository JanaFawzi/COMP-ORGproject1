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

    void open();

    bool shouldClose();

    GuiAction draw(
        const char testStatus[],
        const char consoleText[],
        int frameNumber,
        bool running,
        CPU& cpu
    );

    void close();

private:
    int width;
    int height;

    void drawStatusPanel(
        const char testStatus[],
        int frameNumber,
        bool running,
        CPU& cpu
    );

    void drawConsolePanel(const char consoleText[]);

    GuiAction drawControlPanel(bool running, bool halted);

    void drawRegisterPanel(CPU& cpu);

    bool drawButton(float x, float y, float w, float h, const char text[]);
};

#endif