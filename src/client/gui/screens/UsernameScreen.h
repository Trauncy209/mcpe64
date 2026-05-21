#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__UsernameScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__UsernameScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include <string>

class UsernameScreen : public Screen
{
    typedef Screen super;
public:
    UsernameScreen();
    virtual ~UsernameScreen();

    void init();
    virtual void setupPositions() override;
    void render(int xm, int ym, float a);
    void tick();

    virtual bool isPauseScreen() { return false; }

    virtual void keyPressed(int eventKey);
    virtual void keyboardNewChar(char inputChar);
    virtual bool handleBackEvent(bool isDown) { return true; } // block back/escape
    virtual void removed();
    virtual void mouseClicked(int x, int y, int button);

protected:
    virtual void buttonClicked(Button* button);

private:
    Button _btnDone;
    std::string _input;
    int _cursorBlink;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__UsernameScreen_H__*/
