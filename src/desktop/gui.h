#ifndef GUI_H
#define GUI_H

class CPU;

enum GuiViewMode {
    GUI_VIEW_GRAPHICS_ONLY = 0,
    GUI_VIEW_EXPANDED = 1
};

enum GuiDataPanel {
    GUI_DATA_REGISTERS = 0,
    GUI_DATA_RAM = 1
};

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
    static unsigned short getDisassemblyBaseAddress(unsigned short pc);
    static bool isCurrentInstructionByte(unsigned short address, unsigned short pc);

    static bool isValidRegisterEditIndex(int registerIndex);
    static bool isHexDigit(char c);
    static int hexDigitValue(char c);
    static char normalizeHexDigit(char c);
    static bool parseHex16(const char text[], unsigned short& value);
    static bool parseHex8(const char text[], unsigned char& value);

    static void disassembleInstruction(unsigned short word, char text[], int textSize);
    static void buildCurrentInstructionText(CPU& cpu, char text[], int textSize);
    
    bool hasCursorAddress();
    unsigned short getCursorAddress();
    bool setCursorAddress(unsigned short address);
    void clearCursorAddress();
    
    void resetDebugState();

    static unsigned short getMemoryCursorAddressFromClick(
        int mouseX,
        int mouseY,
        unsigned short pc,
        bool& valid
    );

    void open(bool startExpanded = false);

    bool isExpanded();
    void toggleViewMode();

    bool shouldClose();

    GuiAction draw(
        const char testStatus[],
        const char consoleText[],
        int frameNumber,
        bool running,
        CPU& cpu
    );

    void formatMemoryLine(CPU& cpu, unsigned short address, char text[]);

    bool editRegister(CPU& cpu, int registerIndex, unsigned short value);
    bool hasSelectedRegister();
    int getSelectedRegisterIndex();
    void clearSelectedRegister();


    bool editMemoryByte(CPU& cpu, unsigned short address, unsigned char value);
    bool editMemoryWord(CPU& cpu, unsigned short address, unsigned short value);
    bool hasSelectedMemoryAddress();
    unsigned short getSelectedMemoryAddress();
    void clearSelectedMemoryAddress();


    void close();


private:
    int width;
    int height;
    int graphicsWidth;
    int graphicsHeight;
    int expandedWidth;
    int cpuPanelX;
    float cpuScale;
    int cpuOffsetY;
    bool drawingCpuPanel;
    bool statusPanelVisible;
    GuiViewMode viewMode;
    GuiDataPanel dataPanel;

    static const int TARGET_FPS = 60;
    static const int CONSOLE_VISIBLE_LINES = 5;

    void updateKeyboardFromRaylib(CPU& cpu);
    void calculateLayout();
    bool drawViewToggleButton();
    void drawDataPanelTabs();
    bool drawStatusToggleButton();
    int getUiMouseX();
    int getUiMouseY();
    
    void updateAudioFromCpu(CPU& cpu);
    bool playTone(unsigned short frequency, unsigned short durationMs);
    bool stopAudio();

    unsigned short cursorAddress;
    bool cursorAddressSelected;
    void updateMemoryCursorFromMouse(unsigned short pc);
    void updateMemoryBreakpointFromMouse(bool running, CPU& cpu);

    static unsigned short getMemoryByteAddressFromClick(
        int mouseX,
        int mouseY,
        unsigned short pc,
        bool& valid
    );

    void startMemoryEdit(unsigned short address);
    void updateMemoryEditorFromMouse(bool running, CPU& cpu);
    void updateMemoryEditorFromKeyboard(bool running, CPU& cpu);
    bool appendMemoryEditDigit(char c);
    bool removeMemoryEditDigit();
    bool applyMemoryEdit(CPU& cpu);

    int selectedRegisterIndex;
    bool registerEditActive;
    char registerEditText[5];
    int registerEditLength;

    unsigned short selectedMemoryAddress;
    bool memoryAddressSelected;

    bool memoryEditActive;
    char memoryEditText[5];
    int memoryEditLength;

    void startRegisterEdit(int registerIndex, unsigned short currentValue);
    void updateRegisterEditorFromMouse(bool running, CPU& cpu);
    void updateRegisterEditorFromKeyboard(bool running, CPU& cpu);
    bool appendRegisterEditDigit(char c);
    bool removeRegisterEditDigit();
    bool applyRegisterEdit(CPU& cpu);
    static int getRegisterIndexFromClick(int mouseX, int mouseY);

    void drawStatusPanel(
        const char testStatus[],
        int frameNumber,
        bool running,
        CPU& cpu
    );

    void drawConsolePanel(const char consoleText[]);
    void drawConsoleTextLines(const char visibleText[], int x, int y);

    GuiAction drawControlPanel(bool running, bool halted);

    void drawRegisterPanel(CPU& cpu, bool running);

    void drawMemoryPanel(CPU& cpu, bool running);
    void drawCompactMemoryLine(
        CPU& cpu,
        unsigned short address,
        unsigned short pc,
        int x,
        int y
    );

    GuiAction drawGraphicsPanel(CPU& cpu, bool running);

    bool drawButton(int x, int y, int w, int h, const char text[]);

    void drawDisassemblyPanel(CPU& cpu, bool running);
    void drawDisassemblyLine(CPU& cpu, unsigned short address, int x, int y);
};

#endif
