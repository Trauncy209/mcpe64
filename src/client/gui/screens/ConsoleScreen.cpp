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

struct ChatLayout {
    int panelLeft;
    int panelRight;
    int panelTop;
    int panelBottom;
    int historyLeft;
    int historyRight;
    int historyTop;
    int historyBottom;
    int inputLeft;
    int inputRight;
    int inputTop;
    int inputBottom;
    int closeLeft;
    int closeRight;
    int closeTop;
    int closeBottom;
    int sendLeft;
    int sendRight;
    int sendTop;
    int sendBottom;
    int upLeft;
    int upRight;
    int upTop;
    int upBottom;
    int downLeft;
    int downRight;
    int downTop;
    int downBottom;
    int titleY;
    int lineHeight;
};

static ChatLayout buildLayout(int width, int height, bool touch) {
    ChatLayout l;
    if (touch) {
        const int margin = 8;
        const int headerH = 22;
        const int footerH = 24;
        const int pad = 6;
        const int scrollW = 16;
        const int buttonW = 48;

        l.panelLeft = margin;
        l.panelRight = width - margin;
        l.panelTop = 10;
        l.panelBottom = height - 10;
        l.historyLeft = l.panelLeft + pad;
        l.historyRight = l.panelRight - pad - scrollW - 2;
        l.historyTop = l.panelTop + headerH + 3;
        l.historyBottom = l.panelBottom - footerH - 5;
        l.inputLeft = l.panelLeft + pad + buttonW + 4;
        l.inputRight = l.panelRight - pad - buttonW - 4;
        l.inputTop = l.panelBottom - footerH + 3;
        l.inputBottom = l.panelBottom - 5;
        l.closeLeft = l.panelLeft + pad;
        l.closeRight = l.closeLeft + buttonW;
        l.closeTop = l.inputTop;
        l.closeBottom = l.inputBottom;
        l.sendRight = l.panelRight - pad;
        l.sendLeft = l.sendRight - buttonW;
        l.sendTop = l.inputTop;
        l.sendBottom = l.inputBottom;
        l.upLeft = l.historyRight + 4;
        l.upRight = l.panelRight - pad;
        l.upTop = l.historyTop;
        l.upBottom = l.upTop + 18;
        l.downLeft = l.upLeft;
        l.downRight = l.upRight;
        l.downBottom = l.historyBottom;
        l.downTop = l.downBottom - 18;
        l.titleY = l.panelTop + 7;
        l.lineHeight = 10;
        return l;
    }

    const int boxH = 12;
    const int boxY = height - boxH - 2;
    const int boxX0 = 2;
    const int boxX1 = width - 2;
    const int panelTop = 4;
    const int panelBottom = boxY - 4;
    const int buttonSize = 12;

    l.panelLeft = boxX0;
    l.panelRight = boxX1;
    l.panelTop = panelTop;
    l.panelBottom = panelBottom;
    l.historyLeft = boxX0 + 2;
    l.historyRight = boxX1 - buttonSize - 3;
    l.historyTop = panelTop + 14;
    l.historyBottom = panelBottom - 2;
    l.inputLeft = boxX0;
    l.inputRight = boxX1;
    l.inputTop = boxY;
    l.inputBottom = boxY + boxH;
    l.closeLeft = boxX0;
    l.closeRight = boxX0;
    l.closeTop = boxY;
    l.closeBottom = boxY;
    l.sendLeft = boxX1;
    l.sendRight = boxX1;
    l.sendTop = boxY;
    l.sendBottom = boxY;
    l.upLeft = boxX1 - buttonSize;
    l.upRight = boxX1;
    l.upTop = panelTop;
    l.upBottom = panelTop + buttonSize;
    l.downLeft = boxX1 - buttonSize;
    l.downRight = boxX1;
    l.downBottom = panelBottom;
    l.downTop = panelBottom - buttonSize;
    l.titleY = panelTop + 2;
    l.lineHeight = 9;
    return l;
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
    ChatLayout l = buildLayout(width, height, minecraft->useTouchscreen());
    int available = l.historyBottom - l.historyTop - 4;
    int lines = available / l.lineHeight;
    if (minecraft->useTouchscreen()) {
        if (lines < 10) lines = 10;
        if (lines > 28) lines = 28;
    } else {
        if (lines < 8) lines = 8;
        if (lines > 32) lines = 32;
    }
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
        execute(!minecraft->useTouchscreen());
    } else if (eventKey == Keyboard::KEY_BACKSPACE) {
        if (!_input.empty())
            _input.erase(_input.size() - 1, 1);
    } else if (eventKey == 38) {
        scrollHistory(minecraft->useTouchscreen() ? 4 : 1);
    } else if (eventKey == 40) {
        scrollHistory(minecraft->useTouchscreen() ? -4 : -1);
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

    ChatLayout l = buildLayout(width, height, minecraft->useTouchscreen());

    if (minecraft->useTouchscreen()) {
        const int pageAmount = std::max(4, getVisibleHistoryLines() - 3);
        if (x >= l.upLeft && x < l.upRight && y >= l.upTop && y < l.upBottom) {
            scrollHistory(pageAmount);
            return;
        }
        if (x >= l.downLeft && x < l.downRight && y >= l.downTop && y < l.downBottom) {
            scrollHistory(-pageAmount);
            return;
        }
        if (x >= l.closeLeft && x < l.closeRight && y >= l.closeTop && y < l.closeBottom) {
            minecraft->setScreen(NULL);
            return;
        }
        if (x >= l.sendLeft && x < l.sendRight && y >= l.sendTop && y < l.sendBottom) {
            execute(false);
            return;
        }
        if (x >= l.inputLeft && x < l.inputRight && y >= l.inputTop && y < l.inputBottom) {
            minecraft->platform()->showKeyboard();
            return;
        }
        return;
    }

    const int pageAmount = getVisibleHistoryLines() - 2;
    if (x >= l.upLeft && x < l.upRight && y >= l.upTop && y < l.upBottom) {
        scrollHistory(pageAmount);
        return;
    }
    if (x >= l.downLeft && x < l.downRight && y >= l.downTop && y < l.downBottom) {
        scrollHistory(-pageAmount);
        return;
    }
}

void ConsoleScreen::execute(bool closeAfter)
{
    if (_input.empty()) {
        if (closeAfter)
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

    _input.clear();
    _cursorBlink = 0;
    _historyOffset = 0;

    if (closeAfter)
        minecraft->setScreen(NULL);
    else if (minecraft->useTouchscreen())
        minecraft->platform()->showKeyboard();
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

    const bool touch = minecraft->useTouchscreen();
    ChatLayout l = buildLayout(width, height, touch);

    fill(l.panelLeft, l.panelTop, l.panelRight, l.panelBottom, touch ? 0xe0101010 : 0x90000000);
    fill(l.panelLeft, l.panelTop, l.panelRight, l.panelTop + 1, 0xff808080);
    fill(l.panelLeft, l.panelBottom - 1, l.panelRight, l.panelBottom, 0xff808080);
    fill(l.panelLeft, l.panelTop, l.panelLeft + 1, l.panelBottom, 0xff808080);
    fill(l.panelRight - 1, l.panelTop, l.panelRight, l.panelBottom, 0xff808080);

    if (touch) {
        fill(l.panelLeft + 1, l.panelTop + 1, l.panelRight - 1, l.panelTop + 21, 0xb0303030);
        drawString(font, "Chat", l.panelLeft + 8, l.titleY, 0xffffffff);
        font->drawShadow("Messages", (float)(l.panelLeft + 46), (float)l.titleY, 0xffbbbbbb);
    } else {
        fill(l.upLeft, l.upTop, l.upRight, l.upBottom, 0x90404040);
        fill(l.downLeft, l.downTop, l.downRight, l.downBottom, 0x90404040);
        font->drawShadow("^", (float)(l.upLeft + 4), (float)(l.upTop + 2), 0xffffffff);
        font->drawShadow("v", (float)(l.downLeft + 4), (float)(l.downTop + 2), 0xffffffff);
        drawString(font, "Chat history", l.panelLeft + 4, l.titleY, 0xffffffff);
        font->drawShadow("tap ^ / v to scroll", (float)(l.panelLeft + 62), (float)l.titleY, 0xffbbbbbb);
    }

    fill(l.historyLeft, l.historyTop, l.historyRight, l.historyBottom, touch ? 0x50000000 : 0x40000000);
    if (touch) {
        fill(l.upLeft, l.upTop, l.upRight, l.upBottom, 0x90404040);
        fill(l.downLeft, l.downTop, l.downRight, l.downBottom, 0x90404040);
        drawCenteredString(font, "^", (l.upLeft + l.upRight) / 2, l.upTop + 5, 0xffffffff);
        drawCenteredString(font, "v", (l.downLeft + l.downRight) / 2, l.downTop + 5, 0xffffffff);
    }

    const GuiMessageList& messages = minecraft->gui.getMessages();
    int visibleLines = getVisibleHistoryLines();
    int start = _historyOffset;
    int end = std::min((int)messages.size(), start + visibleLines);
    int lineY = l.historyBottom - (touch ? 12 : 11);
    for (int i = start; i < end; ++i) {
        const std::string& msg = messages[i].message;
        fill(l.historyLeft + 2, lineY - 1, l.historyRight - 2, lineY + 8, touch ? 0x44000000 : 0x50000000);
        Gui::drawColoredString(font, msg, (float)(l.historyLeft + 4), (float)lineY, 255);
        lineY -= l.lineHeight;
    }

    char scrollBuf[32];
    sprintf(scrollBuf, "%d/%d", _historyOffset, getMaxHistoryOffset());
    font->drawShadow(scrollBuf, (float)(touch ? l.upLeft - 28 : l.upLeft - 28), (float)l.titleY, 0xffbbbbbb);

    if (touch) {
        fill(l.closeLeft, l.closeTop, l.closeRight, l.closeBottom, 0x90404040);
        fill(l.sendLeft, l.sendTop, l.sendRight, l.sendBottom, 0x90605030);
        drawCenteredString(font, "Close", (l.closeLeft + l.closeRight) / 2, l.closeTop + 6, 0xffffffff);
        drawCenteredString(font, "Send", (l.sendLeft + l.sendRight) / 2, l.sendTop + 6, 0xffffffff);
    }

    fill(l.inputLeft, l.inputTop, l.inputRight, l.inputBottom, 0xc0000000);
    fill(l.inputLeft, l.inputTop, l.inputRight, l.inputTop + 1, 0xff808080);
    fill(l.inputLeft, l.inputBottom - 1, l.inputRight, l.inputBottom, 0xff808080);
    fill(l.inputLeft, l.inputTop, l.inputLeft + 1, l.inputBottom, 0xff808080);
    fill(l.inputRight - 1, l.inputTop, l.inputRight, l.inputBottom, 0xff808080);

    std::string displayed = _input;
    if ((_cursorBlink / 10) % 2 == 0)
        displayed += '_';

    if (_input.empty() && (_cursorBlink / 10) % 2 != 0)
        font->drawShadow(touch ? "Tap here to type a message" : "Type a message or /command", (float)(l.inputLeft + 4), (float)(l.inputTop + 5), 0xff606060);
    else
        font->drawShadow(displayed, (float)(l.inputLeft + 4), (float)(l.inputTop + 5), 0xffffffff);
}
