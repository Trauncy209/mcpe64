#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_TOUCH__TouchIngameBlockSelectionScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_TOUCH__TouchIngameBlockSelectionScreen_H__

#include "../../Screen.h"
#include "../../components/InventoryPane.h"
#include "../../components/Button.h"
#include "../../components/ScrollingPane.h"
#include "../../components/ItemPane.h"
#include "../../TweenData.h"
#include "../../../player/input/touchscreen/TouchAreaModel.h"
#include "../../../../AppPlatform.h"
#include <vector>

namespace Touch {

class IngameBlockSelectionScreen :	public Screen,
									public IInventoryPaneCallback
{
	typedef Screen super;

public:
	IngameBlockSelectionScreen();
	virtual ~IngameBlockSelectionScreen();

	enum CreativeCategory {
		CATEGORY_BLOCKS = 0,
		CATEGORY_ITEMS,
		CATEGORY_FOOD,
		CATEGORY_ARMOR,
		CATEGORY_COUNT
	};

	virtual void init();
	virtual void setupPositions();
	virtual void removed();

	void tick();
	void render(int xm, int ym, float a);

	bool hasClippingArea(IntRectangle& out);

	// IInventoryPaneCallback
	bool addItem(const InventoryPane* pane, int itemId);
	bool isAllowed(int slot);
	std::vector<const ItemInstance*> getItems(const InventoryPane* forPane);

	void buttonClicked(Button* button);
protected:
	virtual void mouseClicked(int x, int y, int buttonNum);
	virtual void mouseReleased(int x, int y, int buttonNum);
private:
	void renderDemoOverlay();
	void rebuildCreativeItems();
	void setCreativeCategory(int category);
	void addCreativeItem(const ItemInstance& item);

	//int getLinearSlotId(int x, int y);
	int getSlotPosX(int slotX);
	int getSlotPosY(int slotY);

private:
	int selectedItem;
	bool  _pendingClose;
	InventoryPane* _blockList;

	THeader bHeader;
	ImageButton bDone;
	TButton bCraft;
	TButton bArmor;
	TButton bMenu;
	TButton bCatBlocks;
	TButton bCatItems;
	TButton bCatFood;
	TButton bCatArmor;
	std::vector<ItemInstance> creativeItems;
	int creativeCategory;
	int targetHotbarSlot;

	IntRectangle clippingArea;

	int InventoryRows;
	int InventorySize;
	int InventoryColumns;
};

}

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS_TOUCH__TouchIngameBlockSelectionScreen_H__*/
