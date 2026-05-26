#include "ConsoleScreen.h"
#include "../Gui.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../world/level/Level.h"
#include "../../../network/RakNetInstance.h"
#include "../../../network/ServerSideNetworkHandler.h"
#include "../../../network/packet/ChatPacket.h"

#include <sstream>
#include <cstdlib>
#include <cctype>
#include <algorithm>

namespace {
static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t");
    return s.substr(a, b - a + 1);
}
}

ConsoleScreen::ConsoleScreen()
:   _input(""),
    _cursorBlink(0),
    _historyOffset(0)
{
}

void ConsoleScreen::init()
{
    _historyOffset = 0;
    if (minecraft->useTouchscreen())
        minecraft->platform()->showKeyboard();
}

void ConsoleScreen::removed()
{
    if (minecraft && minecraft->useTouchscreen())
        minecraft->platform()->hideKeyboard();
}

void ConsoleScreen::tick()
{
    _cursorBlink++;
}

bool ConsoleScreen::handleBackEvent(bool /*isDown*/)
{
    minecraft->setScreen(NULL);
    return true;
}

int ConsoleScreen::getVisibleHistoryLines() const
{
    const int boxH = 12;
    const int boxY = height - boxH - 2;
    const int panelTop = 4;
    const int panelBottom = boxY - 4;
    int available = panelBottom - panelTop - 14;
    int lines = available / 9;
    if (lines < 8) lines = 8;
    if (lines > 32) lines = 32;
    return lines;
}

int ConsoleScreen::getMaxHistoryOffset() const
{
    const GuiMessageList& messages = minecraft->gui.getMessages();
    int visible = getVisibleHistoryLines();
    if ((int)messages.size() <= visible)
        return 0;
    return (int)messages.size() - visible;
}

void ConsoleScreen::scrollHistory(int delta)
{
    _historyOffset += delta;
    if (_historyOffset < 0)
        _historyOffset = 0;
    int maxOffset = getMaxHistoryOffset();
    if (_historyOffset > maxOffset)
        _historyOffset = maxOffset;
}

void ConsoleScreen::keyPressed(int eventKey)
{
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->setScreen(NULL);
    } else if (eventKey == Keyboard::KEY_RETURN) {
        execute();
    } else if (eventKey == Keyboard::KEY_BACKSPACE) {
        if (!_input.empty())
            _input.erase(_input.size() - 1, 1);
    } else if (eventKey == 38) {
        scrollHistory(1);
    } else if (eventKey == 40) {
        scrollHistory(-1);
    } else {
        super::keyPressed(eventKey);
    }
}

void ConsoleScreen::keyboardNewChar(char inputChar)
{
    if (inputChar >= 32 && inputChar < 127)
        _input += inputChar;
}

void ConsoleScreen::mouseClicked(int x, int y, int buttonNum)
{
    if (buttonNum != MouseAction::ACTION_LEFT)
        return;

    const int boxH = 12;
    const int boxY = height - boxH - 2;
    const int boxX0 = 2;
    const int boxX1 = width - 2;
    const int panelTop = 4;
    const int panelBottom = boxY - 4;
    const int buttonSize = 12;
    const int upX0 = boxX1 - buttonSize;
    const int upY0 = panelTop;
    const int downX0 = boxX1 - buttonSize;
    const int downY0 = panelBottom - buttonSize;

    const int pageAmount = getVisibleHistoryLines() - 2;
    if (x >= upX0 && x < boxX1 && y >= upY0 && y < upY0 + buttonSize) {
        scrollHistory(pageAmount);
        return;
    }
    if (x >= downX0 && x < boxX1 && y >= downY0 && y < panelBottom) {
        scrollHistory(-pageAmount);
        return;
    }
    if (x >= boxX0 && x < boxX1 && y >= boxY && y < boxY + boxH && minecraft->useTouchscreen()) {
        minecraft->platform()->showKeyboard();
        return;
    }
}

void ConsoleScreen::execute()
{
    if (_input.empty()) {
        minecraft->setScreen(NULL);
        return;
    }

    if (_input[0] == '/') {
        std::string result = processCommand(_input);
        if (!result.empty())
            minecraft->gui.addMessage(result);
    } else {
        std::string msg = std::string("<") + minecraft->player->name + "> " + _input;
        if (minecraft->netCallback && minecraft->raknetInstance->isServer()) {
            static_cast<ServerSideNetworkHandler*>(minecraft->netCallback)->displayGameMessage(msg);
        } else if (minecraft->netCallback) {
            ChatPacket chatPkt(msg);
            minecraft->raknetInstance->send(chatPkt);
        } else {
            minecraft->gui.addMessage(msg);
        }
    }

    minecraft->setScreen(NULL);
}

std::string ConsoleScreen::processCommand(const std::string& raw)
{
    std::string line = raw;
    if (!line.empty() && line[0] == '/') line = line.substr(1);
    line = trim(line);

    std::vector<std::string> args;
    {
        std::istringstream ss(line);
        std::string tok;
        while (ss >> tok) args.push_back(tok);
    }

    if (args.empty()) return "";

    Level* level = minecraft->level;
    if (!level) return "No level loaded.";

    if (args[0] == "time") {
        if (args.size() < 2)
            return "Usage: /time <add|set|query> ...";

        const std::string& sub = args[1];

        if (sub == "add") {
            if (args.size() < 3) return "Usage: /time add <value>";
            long delta = std::atol(args[2].c_str());
            long newTime = level->getTime() + delta;
            level->setTime(newTime);
            std::ostringstream out;
            out << "Set the time to " << (newTime % Level::TICKS_PER_DAY);
            return out.str();
        }

        if (sub == "set") {
            if (args.size() < 3) return "Usage: /time set <value|day|night|noon|midnight>";
            const std::string& val = args[2];

            long t = -1;
            if      (val == "day")      t = 1000;
            else if (val == "noon")     t = 6000;
            else if (val == "night")    t = 13000;
            else if (val == "midnight") t = 18000;
            else {
                bool numeric = true;
                for (char c : val)
                    if (!std::isdigit((unsigned char)c)) { numeric = false; break; }
                if (!numeric) return std::string("Unknown value: ") + val;
                t = std::atol(val.c_str());
            }

            long dayCount = level->getTime() / Level::TICKS_PER_DAY;
            long newTime  = dayCount * Level::TICKS_PER_DAY + (t % Level::TICKS_PER_DAY);
            level->setTime(newTime);
            std::ostringstream out;
            out << "Set the time to " << t;
            return out.str();
        }

        if (sub == "query") {
            if (args.size() < 3) return "Usage: /time query <daytime|gametime|day>";
            const std::string& what = args[2];

            long total   = level->getTime();
            long daytime = total % Level::TICKS_PER_DAY;
            long day     = total / Level::TICKS_PER_DAY;

            std::ostringstream out;
            if      (what == "daytime")  { out << "The time of day is " << daytime; }
            else if (what == "gametime") { out << "The game time is "   << total;   }
            else if (what == "day")      { out << "The day is "         << day;     }
            else return std::string("Unknown query: ") + what;
            return out.str();
        }

        return "Unknown sub-command. Usage: /time <add|set|query> ...";
    }

    return std::string("Unknown command: /") + args[0];
}

void ConsoleScreen::render(int /*xm*/, int /*ym*/, float /*a*/)
{
    fillGradient(0, 0, width, height, 0x10000000, 0x70000000);

    const int boxH = 12;
    const int boxY = height - boxH - 2;
    const int boxX0 = 2;
    const int boxX1 = width - 2;
    const int panelTop = 4;
    const int panelBottom = boxY - 4;
    const int titleY = panelTop + 2;
    const int buttonSize = 12;
    const int upX0 = boxX1 - buttonSize;
    const int upY0 = panelTop;
    const int downX0 = boxX1 - buttonSize;
    const int downY0 = panelBottom - buttonSize;

    fill(boxX0, panelTop, boxX1, panelBottom, 0x90000000);
    fill(boxX0, panelTop, boxX1, panelTop + 1, 0xff808080);
    fill(boxX0, panelBottom - 1, boxX1, panelBottom, 0xff808080);
    fill(boxX0, panelTop, boxX0 + 1, panelBottom, 0xff808080);
    fill(boxX1 - 1, panelTop, boxX1, panelBottom, 0xff808080);

    fill(upX0, upY0, boxX1, upY0 + buttonSize, 0x90404040);
    fill(downX0, downY0, boxX1, panelBottom, 0x90404040);
    font->drawShadow("^", (float)(upX0 + 4), (float)(upY0 + 2), 0xffffffff);
    font->drawShadow("v", (float)(downX0 + 4), (float)(downY0 + 2), 0xffffffff);
    drawString(font, "Chat history", boxX0 + 4, titleY, 0xffffffff);
    font->drawShadow("tap ^ / v to scroll", (float)(boxX0 + 62), (float)titleY, 0xffbbbbbb);

    const GuiMessageList& messages = minecraft->gui.getMessages();
    int visibleLines = getVisibleHistoryLines();
    int start = _historyOffset;
    int end = std::min((int)messages.size(), start + visibleLines);
    int lineY = panelBottom - 11;
    for (int i = start; i < end; ++i) {
        const std::string& msg = messages[i].message;
        fill(boxX0 + 2, lineY - 1, boxX1 - buttonSize - 3, lineY + 8, 0x50000000);
        Gui::drawColoredString(font, msg, (float)(boxX0 + 4), (float)lineY, 255);
        lineY -= 9;
    }

    char scrollBuf[32];
    sprintf(scrollBuf, "%d/%d", _historyOffset, getMaxHistoryOffset());
    font->drawShadow(scrollBuf, (float)(boxX1 - buttonSize - 28), (float)titleY, 0xffbbbbbb);

    fill(boxX0, boxY, boxX1, boxY + boxH, 0xc0000000);
    fill(boxX0, boxY, boxX1, boxY + 1, 0xff808080);
    fill(boxX0, boxY + boxH - 1, boxX1, boxY + boxH, 0xff808080);
    fill(boxX0, boxY, boxX0 + 1, boxY + boxH, 0xff808080);
    fill(boxX1 - 1, boxY, boxX1, boxY + boxH, 0xff808080);

    std::string displayed = _input;
    if ((_cursorBlink / 10) % 2 == 0)
        displayed += '_';

    if (_input.empty() && (_cursorBlink / 10) % 2 != 0)
        font->drawShadow("Type a message or /command", (float)(boxX0 + 2), (float)(boxY + 2), 0xff606060);
    else
        font->drawShadow(displayed, (float)(boxX0 + 2), (float)(boxY + 2), 0xffffffff);
}
