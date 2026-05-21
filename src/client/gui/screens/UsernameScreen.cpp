#include "UsernameScreen.h"
#include "StartMenuScreen.h"
#include "../../Minecraft.h"
#include "../../User.h"
#include "../Font.h"
#include "../components/Button.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../AppPlatform.h"

UsernameScreen::UsernameScreen()
:   _btnDone(0, "Done"),
    _input(""),
    _cursorBlink(0)
{
}

UsernameScreen::~UsernameScreen()
{
}

void UsernameScreen::init()
{
    _input = "";
    _btnDone.active = false; // disabled until name typed
    buttons.push_back(&_btnDone);
    setupPositions();
}

void UsernameScreen::setupPositions()
{
    _btnDone.width  = 120;
    _btnDone.height = 20;
    _btnDone.x = (width - _btnDone.width) / 2;
    _btnDone.y = height / 2 + 52;
}

void UsernameScreen::tick()
{
    _cursorBlink++;
}

void UsernameScreen::keyPressed(int eventKey)
{
    if (eventKey == Keyboard::KEY_BACKSPACE) {
        if (!_input.empty())
            _input.erase(_input.size() - 1, 1);
    } else if (eventKey == Keyboard::KEY_RETURN) {
        if (!_input.empty())
            buttonClicked(&_btnDone);
    }
    // deliberately do NOT call super::keyPressed — that would close the screen on Escape
    _btnDone.active = !_input.empty();
}

void UsernameScreen::keyboardNewChar(char inputChar)
{
    if (_input.size() < 16 && inputChar >= 32 && inputChar < 127)
        _input += inputChar;
    _btnDone.active = !_input.empty();
}

void UsernameScreen::mouseClicked(int x, int y, int button)
{
    int cx = width / 2;
    int cy = height / 2;
    int boxW = 160;
    int boxH = 18;
    int boxX = cx - boxW / 2;
    int boxY = cy - 5;
    if (x >= boxX && x <= boxX + boxW && y >= boxY && y <= boxY + boxH) {
        minecraft->platform()->showKeyboard();
    } else {
        super::mouseClicked(x, y, button);
    }
}

void UsernameScreen::removed()
{
    minecraft->platform()->hideKeyboard();
}

void UsernameScreen::buttonClicked(Button* button)
{
    if (button == &_btnDone && !_input.empty()) {
        minecraft->options.username = _input;
        minecraft->options.save();
        minecraft->user->name = _input;
        minecraft->setScreen(NULL); // goes to StartMenuScreen
    }
}

void UsernameScreen::render(int xm, int ym, float a)
{
    // Dark dirt background
    renderBackground();

    int cx = width / 2;
    int cy = height / 2;

    // Title
    drawCenteredString(font, "Enter your username", cx, cy - 70, 0xffffffff);

    // Subtitle
    drawCenteredString(font, "Please choose a username so others can easily", cx, cy - 52, 0xffaaaaaa);
    drawCenteredString(font, "identify you in chat. Don't worry, you can", cx, cy - 40, 0xffaaaaaa);
    drawCenteredString(font, "change it anytime.", cx, cy - 28, 0xffaaaaaa);

    // Input box background
    int boxW = 160;
    int boxH = 18;
    int boxX = cx - boxW / 2;
    int boxY = cy - 5;
    fill(boxX - 1, boxY - 1, boxX + boxW + 1, boxY + boxH + 1, 0xff000000);
    fill(boxX,     boxY,     boxX + boxW,     boxY + boxH,     0xff202020);

    // Build display string with cursor
    std::string display = _input;
    if ((_cursorBlink / 10) % 2 == 0)
        display += '|';

    font->draw(display, (float)(boxX + 4), (float)(boxY + (boxH - 8) / 2 + 1), 0xffffffff, false);

    // Hint below box
    drawCenteredString(font, "Max 16 characters", cx, cy + 20, 0xff808080);

    // Buttons (Done)
    super::render(xm, ym, a);
}
