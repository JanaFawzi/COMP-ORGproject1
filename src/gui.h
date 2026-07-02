#ifndef GUI_H
#define GUI_H

class Gui {
public:
    Gui();

    // Open simulator window
    void open();

    // Check if user closed window
    bool shouldClose();

    // Draw full GUI every frame
    void draw(const char testStatus[], const char consoleText[], int frameNumber);

    // Close simulator window
    void close();

private:
    int width;
    int height;

    void drawStatusPanel(const char testStatus[], int frameNumber);
    void drawConsolePanel(const char consoleText[]);
};

#endif