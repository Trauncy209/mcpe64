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

std::string formatAddress(const RakNet::SystemAddress& address) {
    return address.ToString(true, ':');
}

std::vector<Touch::JoinGameListEntry> buildVisibleServerList(const std::vector<SavedServerEntry>& savedServers, const ServerList& discovered) {
    std::vector<Touch::JoinGameListEntry> serverList;

    for (size_t i = 0; i < savedServers.size(); ++i) {
        Touch::JoinGameListEntry entry;
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
            Touch::JoinGameListEntry entry;
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

ClientProtocol detectProtocolForServer(const Touch::JoinGameListEntry& server) {
    if (server.address.GetPort() == 19134) {
        return CLIENT_PROTOCOL_RAKET_COMPAT;
    }

    return CLIENT_PROTOCOL_CLASSIC;
}

bool connectToServer(Minecraft* minecraft, const Touch::JoinGameListEntry& server) {
    minecraft->raknetInstance->setClientProtocol(detectProtocolForServer(server));
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
	selectedItem = item;
}

void AvailableGamesList::renderItem(int i, int x, int y, int h, Tesselator& t)
{
	const JoinGameListEntry& s = copiedServerList[i];
    const int left = (int)x0 + 6;
    const int right = (int)x1 - 6;
    const int bottom = y + h;
    const int borderColor = isSelectedItem(i) ? 0xffd8e6ff : 0xff202020;
    const int panelColor = isSelectedItem(i) ? 0x80576f8f : 0x60303030;
    unsigned int titleColor = s.isSpecial ? 0x6090a0 : 0xffffffff;
    unsigned int subtitleColor = s.isSaved ? 0xffd0d0d0 : 0xffa8ff90;

	if (startSelected == i && Multitouch::getFirstActivePointerIdEx() >= 0) {
		fill(left, y, right, bottom, 0x809E684F);
	}

    fill(left, y, right, bottom, borderColor);
    fill(left + 1, y + 1, right - 1, bottom - 1, panelColor);

    drawString(minecraft->font, s.title, left + 8, y + 6, titleColor);
    drawString(minecraft->font, s.subtitle, left + 8, y + 20, subtitleColor);
}

JoinGameScreen::JoinGameScreen()
:	bJoin(2, "Join"),
    bDelete(5, "Delete"),
    bAddServer(4, "Add"),
	bBack(3, "Back"),
	bHeader(0, "Servers"),
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

void JoinGameScreen::init()
{
	buttons.push_back(&bHeader);
	buttons.push_back(&bBack);
    buttons.push_back(&bAddServer);
    buttons.push_back(&bJoin);
    buttons.push_back(&bDelete);

	minecraft->raknetInstance->clearServerList();
	gamesList = new AvailableGamesList(minecraft, width, height);

#ifdef ANDROID
	tabButtons.push_back(&bBack);
	tabButtons.push_back(&bAddServer);
    tabButtons.push_back(&bJoin);
    tabButtons.push_back(&bDelete);
#endif
}

void JoinGameScreen::setupPositions() {
    const int topPadding = 4;
    const int bottomY = height - 30;
    const int bottomButtonWidth = 90;
    const int gap = 8;
    const int leftX = width / 2 - bottomButtonWidth - gap / 2;
    const int rightX = width / 2 + gap / 2;

	bBack.y = topPadding;
    bAddServer.y = topPadding;
	bHeader.y = topPadding;

	bBack.x = 0;
    bAddServer.x = width - bAddServer.width;
	bHeader.x = bBack.width;
	bHeader.width = width - (bBack.width + bAddServer.width);
    bHeader.xText = width / 2;

    bDelete.width = bottomButtonWidth;
    bJoin.width = bottomButtonWidth;
    bDelete.y = bottomY;
    bJoin.y = bottomY;
    bDelete.x = leftX;
    bJoin.x = rightX;
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

	if (hasSelection)
	{
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

void JoinGameScreen::render(int xm, int ym, float a)
{
	bool hasNetwork = minecraft->platform()->isNetworkEnabled(true);
#ifdef WIN32
	hasNetwork = hasNetwork && !GetAsyncKeyState(VK_TAB);
#endif

	renderBackground();
	gamesList->render(xm, ym, a);
	Screen::render(xm, ym, a);

    const int statusY = 34;
	if (hasNetwork) {
		std::string s = "Scanning for WiFi Games...";
		drawCenteredString(minecraft->font, s, width / 2, statusY, 0xffffffff);

		const int textWidth = minecraft->font->width(s);
		const int spinnerX = width / 2 + textWidth / 2 + 6;
		static const char* spinnerTexts[] = {"-", "\\", "|", "/"};
		int n = ((int)(5.5f * getTimeS()) % 4);
		drawCenteredString(minecraft->font, spinnerTexts[n], spinnerX, statusY, 0xffffffff);
	} else {
		drawCenteredString(minecraft->font, gamesList->copiedServerList.empty() ? "WiFi is disabled" : "Saved Servers", width / 2, statusY, 0xffffffff);
	}
}

bool JoinGameScreen::isInGameScreen() { return false; }

};
