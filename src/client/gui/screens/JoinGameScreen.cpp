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

ServerList buildVisibleServerList(const std::vector<SavedServerEntry>& savedServers, const ServerList& discovered) {
    ServerList serverList;

    for (size_t i = 0; i < savedServers.size(); ++i) {
        PingedCompatibleServer server;
        server.name = savedServers[i].name.c_str();
        server.address = makeAddress(savedServers[i]);
        server.pingTime = 0;
        server.isSpecial = false;
        serverList.push_back(server);
    }

    for (size_t i = 0; i < discovered.size(); ++i) {
        if (discovered[i].name.GetLength() == 0) {
            continue;
        }

        bool merged = false;
        for (size_t j = 0; j < serverList.size(); ++j) {
            if (serverList[j].address == discovered[i].address) {
                serverList[j].name = discovered[i].name;
                serverList[j].pingTime = discovered[i].pingTime;
                serverList[j].isSpecial = discovered[i].isSpecial;
                merged = true;
                break;
            }
        }

        if (!merged) {
            serverList.push_back(discovered[i]);
        }
    }

    return serverList;
}

bool connectToServer(Minecraft* minecraft, const PingedCompatibleServer& server) {
    if (!minecraft->netCallback) {
        minecraft->netCallback = new ClientSideNetworkHandler(minecraft, minecraft->raknetInstance);
    }
    minecraft->isLookingForMultiplayer = true;
    return minecraft->raknetInstance->connect(server.address.ToString(false), server.address.GetPort());
}
}

void AvailableGamesList::renderItem(int i, int x, int y, int h, Tesselator& t)
{
    const PingedCompatibleServer& s = copiedServerList[i];
    unsigned int color = s.isSpecial ? 0xff00b0 : 0xffffa0;
    drawString(minecraft->font, s.name.C_String(), x, y + 2, color);
    drawString(minecraft->font, s.address.ToString(true, ':'), x, y + 16, 0xffffa0);
}

JoinGameScreen::JoinGameScreen()
:	bJoin(2, "Join Game"),
	bBack(3, "Back"),
    bAddServer(4, "Add Server"),
	gamesList(NULL),
    lastManualPingTime(0)
{
	bJoin.active = false;
}

JoinGameScreen::~JoinGameScreen()
{
	delete gamesList;
}

void JoinGameScreen::buttonClicked(Button* button)
{
	if (button->id == bJoin.id)
	{
		if (isIndexValid(gamesList->selectedItem))
		{
			PingedCompatibleServer selectedServer = gamesList->copiedServerList[gamesList->selectedItem];
            if (connectToServer(minecraft, selectedServer)) {
				bJoin.active = false;
				bBack.active = false;
                bAddServer.active = false;
				minecraft->setScreen(new ProgressScreen());
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

    ServerList serverList = buildVisibleServerList(savedServers, minecraft->raknetInstance->getServerList());

	if (serverList.size() != gamesList->copiedServerList.size())
	{
		PingedCompatibleServer selectedServer;
		bool hasSelection = false;
		if (isIndexValid(gamesList->selectedItem))
		{
			selectedServer = gamesList->copiedServerList[gamesList->selectedItem];
			hasSelection = true;
		}

		gamesList->copiedServerList = serverList;
		gamesList->selectItem(-1, false);

		if (hasSelection)
		{
			for (unsigned int i = 0; i < gamesList->copiedServerList.size(); ++i)
			{
				if (gamesList->copiedServerList[i].address == selectedServer.address)
				{
					gamesList->selectItem(i, false);
					break;
				}
			}
		}
	}
    else {
        for (int i = (int)gamesList->copiedServerList.size() - 1; i >= 0; --i) {
            for (int j = 0; j < (int)serverList.size(); ++j) {
                if (serverList[j].address == gamesList->copiedServerList[i].address && serverList[j].name.GetLength() > 0) {
                    gamesList->copiedServerList[i].name = serverList[j].name;
                }
            }
        }
	}

	bJoin.active = isIndexValid(gamesList->selectedItem);
}

void JoinGameScreen::init()
{
	buttons.push_back(&bJoin);
	buttons.push_back(&bBack);
    buttons.push_back(&bAddServer);

	minecraft->raknetInstance->clearServerList();
	gamesList = new AvailableGamesList(minecraft, width, height);

#ifdef ANDROID
	tabButtons.push_back(&bJoin);
	tabButtons.push_back(&bBack);
    tabButtons.push_back(&bAddServer);
#endif
}

void JoinGameScreen::setupPositions() {
	int yBase = height - 26;

	bJoin.y = yBase;
	bBack.y = yBase;
	bBack.width = bJoin.width = 120;

	bJoin.x = width / 2 - 4 - bJoin.width;
	bBack.x = width / 2 + 4;

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
