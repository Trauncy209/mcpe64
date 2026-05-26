#include "JoinByIPScreen.h"

#include <cstdlib>

#include "../../SavedServerList.h"
#include "client/gui/Screen.h"
#include "client/gui/components/TextBox.h"

JoinByIPScreen::JoinByIPScreen() :
    tName(0, "Server Name"),
    tIP(1, "Address or IP"),
    tPort(2, "Port"),
    bHeader(3, "Add Server"),
	bJoin(4, "Add Server"),
	bBack(5, "")
{
	bJoin.active = false;
}

JoinByIPScreen::~JoinByIPScreen()
{
}

void JoinByIPScreen::buttonClicked(Button* button)
{
	if (button->id == bJoin.id)
	{
        SavedServerEntry entry;
        entry.name = tName.text;
        entry.address = tIP.text;
        entry.port = std::atoi(tPort.text.c_str());
        if (SavedServerList::addOrUpdate(entry)) {
            minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
        }
	}
	if (button->id == bBack.id)
	{
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
	}
}

bool JoinByIPScreen::handleBackEvent(bool isDown)
{
	if (!isDown)
	{
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
	}
	return true;
}

void JoinByIPScreen::tick()
{
	Screen::tick();
	bJoin.active = !tIP.text.empty() && !tPort.text.empty();
}

void JoinByIPScreen::init()
{
    ImageDef def;
	def.name = "gui/touchgui.png";
	def.width = 34;
	def.height = 26;
	def.setSrc(IntRectangle(150, 0, (int)def.width, (int)def.height));
	bBack.setImageDef(def, true);

	buttons.push_back(&bJoin);
	buttons.push_back(&bBack);
	buttons.push_back(&bHeader);
    textBoxes.push_back(&tName);
    textBoxes.push_back(&tIP);
    textBoxes.push_back(&tPort);
#ifdef ANDROID
	tabButtons.push_back(&bJoin);
	tabButtons.push_back(&bBack);
    tabButtons.push_back(&bHeader);
#endif
    tPort.text = "19132";
}

void JoinByIPScreen::setupPositions() {
    int boxWidth = width - 40;
    if (boxWidth > 240) boxWidth = 240;
    const int portWidth = 80;
    const int boxHeight = 18;
    const int centerX = (width - boxWidth) / 2;
    const int baseY = height / 2 - 38;

	bBack.x = width - bBack.width;
	bBack.y = 0;
	bHeader.x = 0;
	bHeader.y = 0;
	bHeader.width = width - bBack.width;

    tName.x = centerX;
    tName.y = baseY;
    tName.width = boxWidth;
    tName.height = boxHeight;

    tIP.x = centerX;
    tIP.y = baseY + boxHeight + 10;
    tIP.width = boxWidth - portWidth - 6;
    tIP.height = boxHeight;

    tPort.x = tIP.x + tIP.width + 6;
    tPort.y = tIP.y;
    tPort.width = portWidth;
    tPort.height = boxHeight;

    bJoin.x = (width - bJoin.width) / 2;
	bJoin.y = tIP.y + boxHeight + 12;
}

void JoinByIPScreen::render( int xm, int ym, float a )
{
	renderBackground();
	Screen::render(xm, ym, a);
}

void JoinByIPScreen::keyPressed(int eventKey)
{
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
        return;
    }
    Screen::keyPressed(eventKey);
}
