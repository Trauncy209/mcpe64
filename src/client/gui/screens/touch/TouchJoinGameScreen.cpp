#include "TouchJoinGameScreen.h"
#include "../StartMenuScreen.h"
#include "../ProgressScreen.h"
#include "../../Font.h"
#include "../../../Minecraft.h"
#include "../../../SavedServerList.h"
#include "../../../renderer/Textures.h"
#include "../../../../network/ClientSideNetworkHandler.h"
#include "../../../../raknet/GetTime.h"

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

namespace Touch {

void AvailableGamesList::selectStart(int item) {
	startSelected = item;
}

void AvailableGamesList::selectCancel() {
	startSelected = -1;
}

void AvailableGamesList::selectItem(int item, bool doubleClick) {
	LOGI("selected an item! %d\n", item);
	selectedItem = item;
}

void AvailableGamesList::renderItem(int i, int x, int y, int h, Tesselator& t)
{
	if (startSelected == i && Multitouch::getFirstActivePointerIdEx() >= 0) {
		fill((int)x0, y, (int)x1, y + h, 0x809E684F);
	}

	const PingedCompatibleServer& s = copiedServerList[i];
	unsigned int color = s.isSpecial ? 0x6090a0 : 0xffffb0;
	unsigned int color2 = 0xffffa0;

	int xx1 = (int)x0 + 24;
	int xx2 = xx1;

	if (s.isSpecial) {
		xx1 += 50;
		glEnable2(GL_TEXTURE_2D);
        glColor4f2(1, 1, 1, 1);
        glEnable2(GL_BLEND);
		minecraft->textures->loadAndBindTexture("gui/badge/minecon140.png");
		blit(xx2, y + 6, 0, 0, 37, 8, 140, 240);
	}

	drawString(minecraft->font, s.name.C_String(), xx1, y + 6, color);
	drawString(minecraft->font, s.address.ToString(true, ':'), xx2, y + 18, color2);
}

JoinGameScreen::JoinGameScreen()
:	bJoin(2, "Join Game"),
    bAddServer(4, "Add"),
	bBack(3, "Back"),
	bHeader(0, "Join Game"),
	gamesList(NULL),
    lastManualPingTime(0)
{
	bJoin.active = false;
}

JoinGameScreen::~JoinGameScreen()
{
	delete gamesList;
}

void JoinGameScreen::init()
{
	buttons.push_back(&bAddServer);
	buttons.push_back(&bBack);
	buttons.push_back(&bHeader);

	minecraft->raknetInstance->clearServerList();
	gamesList = new AvailableGamesList(minecraft, width, height);

#ifdef ANDROID
	tabButtons.push_back(&bAddServer);
	tabButtons.push_back(&bBack);
#endif
}

void JoinGameScreen::setupPositions() {
	bAddServer.y = 0;
	bBack.y = 0;
	bHeader.y = 0;

	bBack.x = 0;
    bAddServer.x = width - bAddServer.width;
	bHeader.x = bBack.width;
	bHeader.width = width - bBack.width - bAddServer.width;
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
	if (isIndexValid(gamesList->selectedItem)) {
		buttonClicked(&bJoin);
		return;
	}

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
			for (unsigned int i = 0; i < gamesList->copiedServerList.size(); i++)
			{
				if (gamesList->copiedServerList[i].address == selectedServer.address)
				{
					gamesList->selectItem(i, false);
					break;
				}
			}
		}
	} else {
		for (int i = (int)gamesList->copiedServerList.size()-1; i >= 0 ; --i) {
			for (int j = 0; j < (int) serverList.size(); ++j)
				if (serverList[j].address == gamesList->copiedServerList[i].address && serverList[j].name.GetLength() > 0)
					gamesList->copiedServerList[i].name = serverList[j].name;
		}
	}

	bJoin.active = isIndexValid(gamesList->selectedItem);
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

	const int baseX = bHeader.x + bHeader.width / 2;
	if (hasNetwork) {
		std::string s = "Scanning for WiFi Games...";
		drawCenteredString(minecraft->font, s, baseX, 8, 0xffffffff);

		const int textWidth = minecraft->font->width(s);
		const int spinnerX = baseX + textWidth / 2 + 6;
		static const char* spinnerTexts[] = {"-", "\\", "|", "/"};
		int n = ((int)(5.5f * getTimeS()) % 4);
		drawCenteredString(minecraft->font, spinnerTexts[n], spinnerX, 8, 0xffffffff);
	} else {
		drawCenteredString(minecraft->font, gamesList->copiedServerList.empty() ? "WiFi is disabled" : "Saved Servers", baseX, 8, 0xffffffff);
	}
}

bool JoinGameScreen::isInGameScreen() { return false; }

};
