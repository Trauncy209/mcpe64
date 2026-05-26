#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ConsoleScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ConsoleScreen_H__

#include "../Screen.h"
#include <string>

class ConsoleScreen: public Screen
{
    typedef Screen super;
public:
    ConsoleScreen();
    virtual ~ConsoleScreen() {}

    void init();
    void render(int xm, int ym, float a);
    void tick();
    void removed();

    virtual bool renderGameBehind() { return true; }
    virtual bool isInGameScreen()   { return true; }
    virtual bool isPauseScreen()    { return false; }

    virtual void keyPressed(int eventKey);
    virtual void keyboardNewChar(char inputChar);
    virtual bool handleBackEvent(bool isDown);
    virtual void mouseClicked(int x, int y, int buttonNum);

private:
    void execute();
    std::string processCommand(const std::string& cmd);
    int getVisibleHistoryLines() const;
    int getMaxHistoryOffset() const;
    void scrollHistory(int delta);

    std::string _input;
    int         _cursorBlink;
    int         _historyOffset;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__ConsoleScreen_H__*/
