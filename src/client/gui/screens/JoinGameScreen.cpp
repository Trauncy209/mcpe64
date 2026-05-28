#include "JoinGameScreen.h"
#include "StartMenuScreen.h"
#include "ProgressScreen.h"
#include "../Font.h"
#include "../../SavedServerList.h"
#include "../../../network/ClientSideNetworkHandler.h"
#include "../../../network/RakNetInstance.h"
#include "../../../raknet/GetTime.h"

namespace {
RakNet::SystemAddress makeAddress(const SavedServerEntry& entry) {
    return RakNet::SystemAddress(entry.address.c_str(), entry.port);
}

std::string formatAddress(const RakNet::SystemAddress& address) {
    return address.ToString(true, ':');
}

std::vector<JoinGameListEntry> buildVisibleServerList(const std::vector<SavedServerEntry>& savedServers, const ServerList& discovered) {
    std::vector<JoinGameListEntry> serverList;

    for (size_t i = 0; i < savedServers.size(); ++i) {
        JoinGameListEntry entry;
        entry.address = makeAddress(savedServers[i]);
        entry.title = savedServers[i].name.empty() ? formatAddress(entry.address) : savedServers[i].name;
        entry.subtitle = formatAddress(entry.address);
        entry.isSpecial = false;
        entry.isSaved = true;
        serverList.push_back(entry);
    }

    for (size_t i = 0; i < discovered.size(); ++i) {
        if (discovered[i].name.GetLength() == 0) {
            continue;
        }

        bool merged = false;
        for (size_t j = 0; j < serverList.size(); ++j) {
            if (serverList[j].address == discovered[i].address) {
                serverList[j].title = discovered[i].name.C_String();
                serverList[j].isSpecial = discovered[i].isSpecial;
                merged = true;
                break;
            }
        }

        if (!merged) {
            JoinGameListEntry entry;
            entry.address = discovered[i].address;
            entry.title = discovered[i].name.C_String();
            entry.subtitle = "LAN";
            entry.isSpecial = discovered[i].isSpecial;
            entry.isSaved = false;
            serverList.push_back(entry);
        }
    }

    return serverList;
}

ClientProtocol detectProtocolForServer(const JoinGameListEntry& server) {
    if (server.address.GetPort() == 19134) {
        return CLIENT_PROTOCOL_RAKET_COMPAT;
    }

    return CLIENT_PROTOCOL_CLASSIC;
}

bool connectToServer(Minecraft* minecraft, const JoinGameListEntry& server) {
    minecraft->raknetInstance->setClientProtocol(detectProtocolForServer(server));
    if (!minecraft->netCallback) {
        minecraft->netCallback = new ClientSideNetworkHandler(minecraft, minecraft->raknetInstance);
    }
    minecraft->isLookingForMultiplayer = true;
    return minecraft->raknetInstance->connect(server.address.ToString(false), server.address.GetPort());
}
}

void AvailableGamesList::renderItem(int i, int x, int y, int h, Tesselator& t)
{
    const JoinGameListEntry& s = copiedServerList[i];
    const int left = x - 4;
    const int right = x + 216;
    const int bottom = y + h - 2;
    const int panelColor = isSelectedItem(i) ? 0x80576f8f : 0x60303030;
    const int borderColor = isSelectedItem(i) ? 0xffd8e6ff : 0xff202020;
    unsigned int titleColor = s.isSpecial ? 0xff00b0 : 0xffffffff;
    unsigned int subtitleColor = s.isSaved ? 0xffd0d0d0 : 0xffa8ff90;

    fill(left, y - 1, right, bottom, borderColor);
    fill(left + 1, y, right - 1, bottom - 1, panelColor);

    drawString(minecraft->font, s.title, x, y + 2, titleColor);
    drawString(minecraft->font, s.subtitle, x, y + 16, subtitleColor);
}

JoinGameScreen::JoinGameScreen()
:	bJoin(2, "Join Game"),
    bDelete(5, "Delete"),
	bBack(3, "Back"),
    bAddServer(4, "Add Server"),
	gamesList(NULL),
    lastManualPingTime(0)
{
	bJoin.active = false;
    bDelete.active = false;
}

JoinGameScreen::~JoinGameScreen()
{
	delete gamesList;
}

bool JoinGameScreen::canDeleteSelection() const
{
    return gamesList && gamesList->selectedItem >= 0 && gamesList->selectedItem < (int)gamesList->copiedServerList.size()
        && gamesList->copiedServerList[gamesList->selectedItem].isSaved;
}

void JoinGameScreen::buttonClicked(Button* button)
{
	if (button->id == bJoin.id)
	{
		if (isIndexValid(gamesList->selectedItem))
		{
			JoinGameListEntry selectedServer = gamesList->copiedServerList[gamesList->selectedItem];
            if (connectToServer(minecraft, selectedServer)) {
				bJoin.active = false;
                bDelete.active = false;
				bBack.active = false;
                bAddServer.active = false;
				minecraft->setScreen(new ProgressScreen());
            }
		}
	}
    if (button->id == bDelete.id)
    {
        if (canDeleteSelection()) {
            const JoinGameListEntry& selectedServer = gamesList->copiedServerList[gamesList->selectedItem];
            if (SavedServerList::remove(selectedServer.address.ToString(false), selectedServer.address.GetPort())) {
                gamesList->selectItem(-1, false);
                bJoin.active = false;
                bDelete.active = false;
            }
        }
    }
	if (button->id == bBack.id)
	{
		minecraft->cancelLocateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	}
    if (button->id == bAddServer.id)
    {
        minecraft->screenChooser.setScreen(SCREEN_JOINBYIP);
    }
}

bool JoinGameScreen::handleBackEvent(bool isDown)
{
	if (!isDown)
	{
		minecraft->cancelLocateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	}
	return true;
}

bool JoinGameScreen::isIndexValid(int index)
{
	return gamesList && index >= 0 && index < gamesList->getNumberOfItems();
}

void JoinGameScreen::tick()
{
    std::vector<SavedServerEntry> savedServers = SavedServerList::load();
    RakNet::TimeMS now = RakNet::GetTimeMS();
    if (now - lastManualPingTime > 1000) {
        for (size_t i = 0; i < savedServers.size(); ++i) {
            minecraft->raknetInstance->pingForHost(savedServers[i].address.c_str(), savedServers[i].port);
        }
        lastManualPingTime = now;
    }

    std::vector<JoinGameListEntry> serverList = buildVisibleServerList(savedServers, minecraft->raknetInstance->getServerList());

    RakNet::SystemAddress selectedAddress;
    bool hasSelection = false;
    if (isIndexValid(gamesList->selectedItem)) {
        selectedAddress = gamesList->copiedServerList[gamesList->selectedItem].address;
        hasSelection = true;
    }

    gamesList->copiedServerList = serverList;
    gamesList->selectItem(-1, false);

    if (hasSelection) {
        for (unsigned int i = 0; i < gamesList->copiedServerList.size(); ++i)
        {
            if (gamesList->copiedServerList[i].address == selectedAddress)
            {
                gamesList->selectItem(i, false);
                break;
            }
        }
    }

	bJoin.active = isIndexValid(gamesList->selectedItem);
    bDelete.active = canDeleteSelection();
}

void JoinGameScreen::init()
{
	buttons.push_back(&bJoin);
    buttons.push_back(&bDelete);
	buttons.push_back(&bBack);
    buttons.push_back(&bAddServer);

	minecraft->raknetInstance->clearServerList();
	gamesList = new AvailableGamesList(minecraft, width, height);

#ifdef ANDROID
	tabButtons.push_back(&bJoin);
    tabButtons.push_back(&bDelete);
	tabButtons.push_back(&bBack);
    tabButtons.push_back(&bAddServer);
#endif
}

void JoinGameScreen::setupPositions() {
	int yBase = height - 26;
    const int buttonWidth = 100;
    const int gap = 6;
    const int startX = (width - buttonWidth * 3 - gap * 2) / 2;

	bJoin.y = yBase;
    bDelete.y = yBase;
	bBack.y = yBase;
	bJoin.width = buttonWidth;
    bDelete.width = buttonWidth;
	bBack.width = buttonWidth;

	bJoin.x = startX;
    bDelete.x = startX + buttonWidth + gap;
	bBack.x = startX + (buttonWidth + gap) * 2;

    bAddServer.width = 120;
    bAddServer.y = 4;
    bAddServer.x = width - bAddServer.width - 8;
}

void JoinGameScreen::render(int xm, int ym, float a)
{
	bool hasNetwork = minecraft->platform()->isNetworkEnabled(true);
#ifdef WIN32
	hasNetwork = hasNetwork && !GetAsyncKeyState(VK_TAB);
#endif

	renderBackground();
    gamesList->render(xm, ym, a);
	Screen::render(xm, ym, a);

	if (hasNetwork) {
#ifdef RPI
		std::string s = "Scanning for Local Network Games...";
#else
		std::string s = "Scanning for WiFi Games...";
#endif
		drawCenteredString(minecraft->font, s, width / 2, 8, 0xffffffff);

		const int textWidth = minecraft->font->width(s);
		const int spinnerX = width / 2 + textWidth / 2 + 6;
		static const char* spinnerTexts[] = {"-", "\\", "|", "/"};
		int n = ((int)(5.5f * getTimeS()) % 4);
		drawCenteredString(minecraft->font, spinnerTexts[n], spinnerX, 8, 0xffffffff);
	} else {
        std::string s = gamesList->copiedServerList.empty() ? "WiFi is disabled" : "Saved Servers";
		const int yy = height / 2 - 8;
		drawCenteredString(minecraft->font, s, width / 2, yy, 0xffffffff);
	}
}

bool JoinGameScreen::isInGameScreen() { return false; }
