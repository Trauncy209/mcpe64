#include "TouchIngameBlockSelectionScreen.h"
#include "../crafting/WorkbenchScreen.h"
#include "../../Screen.h"
#include "../../components/ImageButton.h"
#include "../../components/InventoryPane.h"
#include "../../../gamemode/GameMode.h"
#include "../../../renderer/TileRenderer.h"
#include "../../../player/LocalPlayer.h"
#include "../../../renderer/gles.h"
#include "../../../renderer/entity/ItemRenderer.h"
#include "../../../renderer/Tesselator.h"
#include "../../../renderer/Textures.h"
#include "../../../Minecraft.h"
#include "../../../sound/SoundEngine.h"
#include "../../../../world/entity/player/Inventory.h"
#include "../../../../platform/input/Mouse.h"
#include "../../../../util/Mth.h"
#include "../../../../world/item/ItemInstance.h"
#include "../../../../world/item/Item.h"
#include "../../../../world/item/ShearsItem.h"
#include "../../../../world/level/tile/QuartzBlockTile.h"
#include "../../../../world/level/tile/SandStoneTile.h"
#include "../../../../world/level/tile/StoneSlabTile.h"
#include "../../../../world/level/tile/TreeTile.h"
#include "../../../../world/entity/player/Player.h"
#include "../../../../world/item/crafting/Recipe.h"
#include "../../../player/input/touchscreen/TouchAreaModel.h"
#include "../ArmorScreen.h"

namespace Touch {

#if defined(__APPLE__)
    static const std::string demoVersionString("Not available in the Lite version");
#else
    static const std::string demoVersionString("Not available in the demo version");
#endif

#ifdef __APPLE__
    static const float BorderPixels = 4;
    #ifdef DEMO_MODE
        static const float BlockPixels = 22;
    #else
        static const float BlockPixels = 22;
    #endif
#else
    static const float BorderPixels = 4;
    static const float BlockPixels = 24;
#endif

static const int ItemSize = (int)(BlockPixels + 2*BorderPixels);

static const int Bx = 10; // Border Frame width
static const int By = 6; // Border Frame height

//
// Block selection screen
//
IngameBlockSelectionScreen::IngameBlockSelectionScreen()
:	selectedItem(0),
	_blockList(NULL),
	_pendingClose(false),
	bArmor  (4, "Armor"),
	bDone   (3, ""),
	//bDone   (3, "Done"),
	bMenu   (2, "Menu"),
	bCraft  (1, "Craft"),
	bHeader (0, "Select blocks"),
	bCatBlocks(10, "Blocks"),
	bCatItems (11, "Items"),
	bCatFood  (12, "Food"),
	bCatArmor (13, "Armor"),
	creativeCategory(CATEGORY_BLOCKS),
	targetHotbarSlot(0)
{
}

IngameBlockSelectionScreen::~IngameBlockSelectionScreen()
{
	delete _blockList;
}

void IngameBlockSelectionScreen::init()
{
	Inventory* inventory = minecraft->player->inventory;

	//const int itemWidth = 2 * BorderPixels +

	int maxWidth = width - Bx - Bx;
	InventoryColumns = maxWidth / ItemSize;
	const int realWidth = InventoryColumns * ItemSize;
	const int realBx = (width - realWidth) / 2;

	const int categoryBarHeight = minecraft->isCreativeMode() ? 22 : 0;
	IntRectangle rect(realBx,
#ifdef __APPLE__
		24 + By + categoryBarHeight - ((width==240)?1:0), realWidth, ((width==240)?1:0) + height-By-By-20-24-categoryBarHeight);
#else
		24 + By + categoryBarHeight, realWidth, height-By-By-20-24-categoryBarHeight);
#endif

	_blockList = new InventoryPane(this, minecraft, rect, width, BorderPixels, inventory->getContainerSize() - Inventory::MAX_SELECTION_SIZE, ItemSize, (int)BorderPixels);
	_blockList->fillMarginX = realBx;

	//for (int i = 0; i < inventory->getContainerSize(); ++i)
		//LOGI("> %d - %s\n", i, inventory->getItem(i)? inventory->getItem(i)->getDescriptionId().c_str() : "<-->\n");

	// Match the stock 0.6.1 touch inventory: the grid represents backing
	// inventory slots only. The first MAX_SELECTION_SIZE entries are hotbar
	// links and must not be counted/rendered/clicked as picker entries.
	InventorySize = inventory->getContainerSize() - Inventory::MAX_SELECTION_SIZE;
	InventoryRows = 1 + (InventorySize-1) / InventoryColumns;

    //
    // Buttons
    //
	ImageDef def;
	def.name = "gui/spritesheet.png";
	def.x = 0;
	def.y = 1;
	def.width = def.height = 18;
	def.setSrc(IntRectangle(60, 0, 18, 18));
	bDone.setImageDef(def, true);
    bDone.width = bDone.height = 19;

	bDone.scaleWhenPressed = false;

	buttons.push_back(&bHeader);
	buttons.push_back(&bDone);
	if (!minecraft->isCreativeMode()) {
		buttons.push_back(&bCraft);
		buttons.push_back(&bArmor);
	} else {
		targetHotbarSlot = inventory ? inventory->selected : 0;
		int barSlots = minecraft->gui.getItemBarSelectionSize();
		if (barSlots <= 0) barSlots = Inventory::MAX_SELECTION_SIZE;
		if (targetHotbarSlot < 0 || targetHotbarSlot >= barSlots) targetHotbarSlot = 0;
		rebuildCreativeItems();
		buttons.push_back(&bCatBlocks);
		buttons.push_back(&bCatItems);
		buttons.push_back(&bCatFood);
		buttons.push_back(&bCatArmor);
	}
}

void IngameBlockSelectionScreen::setupPositions() {
	bHeader.y = bDone.y = bCraft.y = 0;
	bDone.x   = width -  bDone.width;
	bCraft.x  = 0;//width - bDone.w - bCraft.w;
	bCraft.width = bArmor.width = 48;
	bArmor.x = bCraft.width;

	if (minecraft->isCreativeMode()) {
		bHeader.x = 0;
		bHeader.width = width;// -  bDone.w;
		bHeader.xText = width/2; // Center of the screen
		const int catW = width / 4;
		bCatBlocks.x = 0;
		bCatItems.x = catW;
		bCatFood.x = catW * 2;
		bCatArmor.x = catW * 3;
		bCatBlocks.y = bCatItems.y = bCatFood.y = bCatArmor.y = 24;
		bCatBlocks.width = bCatItems.width = bCatFood.width = catW;
		bCatArmor.width = width - catW * 3;
		bCatBlocks.selected = creativeCategory == CATEGORY_BLOCKS;
		bCatItems.selected = creativeCategory == CATEGORY_ITEMS;
		bCatFood.selected = creativeCategory == CATEGORY_FOOD;
		bCatArmor.selected = creativeCategory == CATEGORY_ARMOR;
	} else {
		bHeader.x = bCraft.width + bArmor.width;
		bHeader.width = width - bCraft.width - bArmor.width;// -  bDone.w;
		bHeader.xText = bHeader.x + (bHeader.width - bDone.width) /2;
	}

	clippingArea.x = 0;
	clippingArea.w = minecraft->width;
	clippingArea.y = 0;
	clippingArea.h = (int)(Gui::GuiScale * 24);
}

void IngameBlockSelectionScreen::removed()
{
	minecraft->gui.inventoryUpdated();
}

int IngameBlockSelectionScreen::getSlotPosX(int slotX) {
    // @todo: Number of columns
	return width / 2 - InventoryColumns * 10 + slotX * 20 + 2;
}

int IngameBlockSelectionScreen::getSlotPosY(int slotY) {
	return height - 16 - 3 - 22 * 2 - 22 * slotY;
}

void IngameBlockSelectionScreen::mouseClicked(int x, int y, int buttonNum) {
	_pendingClose = _blockList->_clickArea->isInside((float)x, (float)y);
	if (!_pendingClose)
		super::mouseClicked(x, y, buttonNum);
}

void IngameBlockSelectionScreen::mouseReleased(int x, int y, int buttonNum) {
	if (_pendingClose && _blockList->_clickArea->isInside((float)x, (float)y))
		minecraft->setScreen(NULL);
	else
		super::mouseReleased(x, y, buttonNum);
}

bool IngameBlockSelectionScreen::addItem(const InventoryPane* pane, int itemId)
{
	Inventory* inventory = minecraft->player->inventory;
	if (minecraft->isCreativeMode()) {
		if (itemId < 0 || itemId >= (int)creativeItems.size())
			return false;

		// Restore stock 0.6.1 hotbar semantics. Picking from the touch drawer
		// moves the clicked backing item into selection slot 0 with propagation,
		// shifting prior hotbar links through the bar. Category entries are
		// synthetic, so mirror the clicked category item into its matching backing
		// inventory slot first, then use the original move/select/flash path.
		int backingSlot = Inventory::MAX_SELECTION_SIZE + itemId;
		if (backingSlot < Inventory::MAX_SELECTION_SIZE || backingSlot >= inventory->getContainerSize())
			return false;

		ItemInstance picked(creativeItems[itemId]);
		picked.count = 64;
		inventory->replaceSlot(backingSlot, &picked);
		inventory->moveToSelectionSlot(0, backingSlot, true);
		inventory->selectSlot(0);
		minecraft->gui.flashSlot(inventory->selected);
#ifdef __APPLE__
		minecraft->soundEngine->playUI("random.pop", 0.3f, 0.3f);//1.0f + 0.2f*(Mth::random()-Mth::random()));
#else
		minecraft->soundEngine->playUI("random.pop2", 1.0f, 0.3f);//1.0f + 0.2f*(Mth::random()-Mth::random()));
#endif
		return true;
	} else {
		itemId += Inventory::MAX_SELECTION_SIZE;
	}

	if (!inventory->getItem(itemId))
		return false;

	inventory->moveToSelectionSlot(0, itemId, true);

	inventory->selectSlot(0);
#ifdef __APPLE__
	minecraft->soundEngine->playUI("random.pop", 0.3f, 0.3f);//1.0f + 0.2f*(Mth::random()-Mth::random()));
#else
    minecraft->soundEngine->playUI("random.pop2", 1.0f, 0.3f);//1.0f + 0.2f*(Mth::random()-Mth::random()));
#endif

	// Flash the selected gui item
	minecraft->gui.flashSlot(inventory->selected);
    return true;
}

void IngameBlockSelectionScreen::tick()
{
	_blockList->tick();
	super::tick();
}

void IngameBlockSelectionScreen::render( int xm, int ym, float a )
{
	glDisable2(GL_DEPTH_TEST);
	glEnable2(GL_BLEND);

	Screen::render(xm, ym, a);
	_blockList->render(xm, ym, a);

	// render frame
	IntRectangle& bbox = _blockList->rect;
	Tesselator::instance.colorABGR(0xffffffff);
	minecraft->textures->loadAndBindTexture("gui/itemframe.png");
	glEnable2(GL_BLEND);
	glColor4f2(1, 1, 1, 1);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	blit(0, bbox.y-By, 0, 0, width, bbox.h+By+By, 215, 256); // why bbox.h + 1*B?
	glDisable2(GL_BLEND);

	glEnable2(GL_DEPTH_TEST);
}

void IngameBlockSelectionScreen::renderDemoOverlay() {
#ifdef DEMO_MODE
	fill(	getSlotPosX(0) - 3, getSlotPosY(1) - 3,
			getSlotPosX(9) - 3, getSlotPosY(-1) - 3, 0xa0 << 24);

	const int centerX = (getSlotPosX(4) + getSlotPosX(5)) / 2;
	const int centerY = (getSlotPosY(0) + getSlotPosY(1)) / 2 + 5;
	drawCenteredString(minecraft->font, demoVersionString, centerX, centerY, 0xffffffff);
#endif /*DEMO_MODE*/
}

void IngameBlockSelectionScreen::buttonClicked(Button* button) {
	if (button->id == bDone.id)
		minecraft->setScreen(NULL);

    if (button->id == bMenu.id)
        minecraft->screenChooser.setScreen(SCREEN_PAUSE);

	if (button->id == bCraft.id)
		minecraft->setScreen(new WorkbenchScreen(Recipe::SIZE_2X2));

	if (button == &bArmor)
		minecraft->setScreen(new ArmorScreen());

	if (button == &bCatBlocks) setCreativeCategory(CATEGORY_BLOCKS);
	if (button == &bCatItems) setCreativeCategory(CATEGORY_ITEMS);
	if (button == &bCatFood) setCreativeCategory(CATEGORY_FOOD);
	if (button == &bCatArmor) setCreativeCategory(CATEGORY_ARMOR);
}

void IngameBlockSelectionScreen::setCreativeCategory(int category)
{
	if (category < 0 || category >= CATEGORY_COUNT)
		return;
	creativeCategory = category;
	bCatBlocks.selected = creativeCategory == CATEGORY_BLOCKS;
	bCatItems.selected = creativeCategory == CATEGORY_ITEMS;
	bCatFood.selected = creativeCategory == CATEGORY_FOOD;
	bCatArmor.selected = creativeCategory == CATEGORY_ARMOR;
	rebuildCreativeItems();
}

void IngameBlockSelectionScreen::addCreativeItem(const ItemInstance& item)
{
	if (item.isNull()) return;
	ItemInstance out(item);
	out.count = 64;
	creativeItems.push_back(out);
}

void IngameBlockSelectionScreen::rebuildCreativeItems()
{
	creativeItems.clear();

	if (creativeCategory == CATEGORY_BLOCKS) {
		addCreativeItem(ItemInstance(Tile::rock));
		addCreativeItem(ItemInstance(Tile::grass));
		addCreativeItem(ItemInstance(Tile::dirt));
		addCreativeItem(ItemInstance(Tile::sand));
		addCreativeItem(ItemInstance(Tile::gravel));
		addCreativeItem(ItemInstance(Tile::wood));
		addCreativeItem(ItemInstance(Tile::treeTrunk, 1, TreeTile::NORMAL_TRUNK));
		addCreativeItem(ItemInstance(Tile::treeTrunk, 1, TreeTile::DARK_TRUNK));
		addCreativeItem(ItemInstance(Tile::treeTrunk, 1, TreeTile::BIRCH_TRUNK));
		addCreativeItem(ItemInstance((Tile*)Tile::leaves, 1, 0));
		addCreativeItem(ItemInstance((Tile*)Tile::leaves, 1, 1));
		addCreativeItem(ItemInstance((Tile*)Tile::leaves, 1, 2));
		addCreativeItem(ItemInstance(Tile::sapling, 1, 0));
		addCreativeItem(ItemInstance(Tile::sapling, 1, 1));
		addCreativeItem(ItemInstance(Tile::sapling, 1, 2));
		addCreativeItem(ItemInstance(Tile::stoneBrick));
		addCreativeItem(ItemInstance(Tile::stoneBrickSmooth, 1, 0));
		addCreativeItem(ItemInstance(Tile::stoneBrickSmooth, 1, 1));
		addCreativeItem(ItemInstance(Tile::stoneBrickSmooth, 1, 2));
		addCreativeItem(ItemInstance(Tile::mossStone));
		addCreativeItem(ItemInstance(Tile::redBrick));
		addCreativeItem(ItemInstance(Tile::sandStone, 1, 0));
		addCreativeItem(ItemInstance(Tile::sandStone, 1, 1));
		addCreativeItem(ItemInstance(Tile::sandStone, 1, 2));
		addCreativeItem(ItemInstance(Tile::clay));
		addCreativeItem(ItemInstance(Tile::coalOre));
		addCreativeItem(ItemInstance(Tile::ironOre));
		addCreativeItem(ItemInstance(Tile::goldOre));
		addCreativeItem(ItemInstance(Tile::emeraldOre));
		addCreativeItem(ItemInstance(Tile::lapisOre));
		addCreativeItem(ItemInstance(Tile::redStoneOre));
		addCreativeItem(ItemInstance(Tile::goldBlock));
		addCreativeItem(ItemInstance(Tile::ironBlock));
		addCreativeItem(ItemInstance(Tile::emeraldBlock));
		addCreativeItem(ItemInstance(Tile::lapisBlock));
		addCreativeItem(ItemInstance(Tile::obsidian));
		addCreativeItem(ItemInstance(Tile::glass));
		addCreativeItem(ItemInstance(Tile::thinGlass));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 0));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 1));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 2));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 3));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 4));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 5));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 6));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 7));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 8));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 9));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 10));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 11));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 12));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 13));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 14));
		addCreativeItem(ItemInstance(Tile::cloth, 1, 15));
		addCreativeItem(ItemInstance(Tile::stoneSlabHalf, 1, StoneSlabTile::STONE_SLAB));
		addCreativeItem(ItemInstance(Tile::stoneSlabHalf, 1, StoneSlabTile::COBBLESTONE_SLAB));
		addCreativeItem(ItemInstance(Tile::stoneSlabHalf, 1, StoneSlabTile::WOOD_SLAB));
		addCreativeItem(ItemInstance(Tile::stoneSlabHalf, 1, StoneSlabTile::BRICK_SLAB));
		addCreativeItem(ItemInstance(Tile::stoneSlabHalf, 1, StoneSlabTile::SAND_SLAB));
		addCreativeItem(ItemInstance(Tile::stoneSlabHalf, 1, StoneSlabTile::SMOOTHBRICK_SLAB));
		addCreativeItem(ItemInstance(Tile::stairs_stone));
		addCreativeItem(ItemInstance(Tile::stairs_wood));
		addCreativeItem(ItemInstance(Tile::stairs_brick));
		addCreativeItem(ItemInstance(Tile::stairs_sandStone));
		addCreativeItem(ItemInstance(Tile::stairs_stoneBrickSmooth));
		addCreativeItem(ItemInstance(Tile::bookshelf));
		addCreativeItem(ItemInstance(Tile::workBench));
		addCreativeItem(ItemInstance(Tile::stonecutterBench));
		addCreativeItem(ItemInstance(Tile::chest));
		addCreativeItem(ItemInstance(Tile::furnace));
		addCreativeItem(ItemInstance(Tile::tnt));
		addCreativeItem(ItemInstance(Tile::torch));
		addCreativeItem(ItemInstance(Tile::ladder));
		addCreativeItem(ItemInstance(Tile::fence));
		addCreativeItem(ItemInstance(Tile::fenceGate));
		addCreativeItem(ItemInstance(Tile::trapdoor));
		addCreativeItem(ItemInstance((Tile*)Tile::flower));
		addCreativeItem(ItemInstance((Tile*)Tile::rose));
		addCreativeItem(ItemInstance((Tile*)Tile::mushroom1));
		addCreativeItem(ItemInstance((Tile*)Tile::mushroom2));
		addCreativeItem(ItemInstance(Tile::cactus));
		addCreativeItem(ItemInstance(Tile::melon));
		addCreativeItem(ItemInstance(Tile::netherReactor));
		addCreativeItem(ItemInstance(Tile::netherrack));
		addCreativeItem(ItemInstance(Tile::netherBrick));
		addCreativeItem(ItemInstance(Tile::lightGem));
		addCreativeItem(ItemInstance(Tile::quartzBlock, 1, QuartzBlockTile::TYPE_DEFAULT));
		addCreativeItem(ItemInstance(Tile::quartzBlock, 1, QuartzBlockTile::TYPE_LINES));
		addCreativeItem(ItemInstance(Tile::quartzBlock, 1, QuartzBlockTile::TYPE_CHISELED));
	} else if (creativeCategory == CATEGORY_ITEMS) {
		addCreativeItem(ItemInstance(Item::flintAndSteel));
		addCreativeItem(ItemInstance(Item::bow));
		addCreativeItem(ItemInstance(Item::arrow));
		addCreativeItem(ItemInstance(Item::shears));
		addCreativeItem(ItemInstance(Item::door_wood));
		addCreativeItem(ItemInstance(Item::door_iron));
		addCreativeItem(ItemInstance(Item::bed));
		addCreativeItem(ItemInstance(Item::sign));
		addCreativeItem(ItemInstance(Item::painting));
		addCreativeItem(ItemInstance(Item::reeds));
		addCreativeItem(ItemInstance(Item::seeds_wheat));
		addCreativeItem(ItemInstance(Item::seeds_melon));
		addCreativeItem(ItemInstance(Item::coal, 1, 0));
		addCreativeItem(ItemInstance(Item::coal, 1, 1));
		addCreativeItem(ItemInstance(Item::ironIngot));
		addCreativeItem(ItemInstance(Item::goldIngot));
		addCreativeItem(ItemInstance(Item::emerald));
		addCreativeItem(ItemInstance(Item::flint));
		addCreativeItem(ItemInstance(Item::leather));
		addCreativeItem(ItemInstance(Item::brick));
		addCreativeItem(ItemInstance(Item::clay));
		addCreativeItem(ItemInstance(Item::paper));
		addCreativeItem(ItemInstance(Item::book));
		addCreativeItem(ItemInstance(Item::egg));
		addCreativeItem(ItemInstance(Item::compass));
		addCreativeItem(ItemInstance(Item::clock));
		addCreativeItem(ItemInstance(Item::bone));
		addCreativeItem(ItemInstance(Item::sugar));
		addCreativeItem(ItemInstance(Item::netherbrick));
		addCreativeItem(ItemInstance(Item::netherQuartz));
		addCreativeItem(ItemInstance(Item::sword_wood));
		addCreativeItem(ItemInstance(Item::sword_stone));
		addCreativeItem(ItemInstance(Item::sword_iron));
		addCreativeItem(ItemInstance(Item::sword_emerald));
		addCreativeItem(ItemInstance(Item::sword_gold));
		addCreativeItem(ItemInstance(Item::shovel_wood));
		addCreativeItem(ItemInstance(Item::shovel_stone));
		addCreativeItem(ItemInstance(Item::shovel_iron));
		addCreativeItem(ItemInstance(Item::shovel_emerald));
		addCreativeItem(ItemInstance(Item::shovel_gold));
		addCreativeItem(ItemInstance(Item::pickAxe_wood));
		addCreativeItem(ItemInstance(Item::pickAxe_stone));
		addCreativeItem(ItemInstance(Item::pickAxe_iron));
		addCreativeItem(ItemInstance(Item::pickAxe_emerald));
		addCreativeItem(ItemInstance(Item::pickAxe_gold));
		addCreativeItem(ItemInstance(Item::hatchet_wood));
		addCreativeItem(ItemInstance(Item::hatchet_stone));
		addCreativeItem(ItemInstance(Item::hatchet_iron));
		addCreativeItem(ItemInstance(Item::hatchet_emerald));
		addCreativeItem(ItemInstance(Item::hatchet_gold));
		addCreativeItem(ItemInstance(Item::hoe_wood));
		addCreativeItem(ItemInstance(Item::hoe_stone));
		addCreativeItem(ItemInstance(Item::hoe_iron));
		addCreativeItem(ItemInstance(Item::hoe_emerald));
		addCreativeItem(ItemInstance(Item::hoe_gold));
	} else if (creativeCategory == CATEGORY_FOOD) {
		addCreativeItem(ItemInstance(Item::apple));
		addCreativeItem(ItemInstance(Item::mushroomStew));
		addCreativeItem(ItemInstance(Item::bread));
		addCreativeItem(ItemInstance(Item::porkChop_raw));
		addCreativeItem(ItemInstance(Item::porkChop_cooked));
		addCreativeItem(ItemInstance(Item::beef_raw));
		addCreativeItem(ItemInstance(Item::beef_cooked));
		addCreativeItem(ItemInstance(Item::chicken_raw));
		addCreativeItem(ItemInstance(Item::chicken_cooked));
		addCreativeItem(ItemInstance(Item::melon));
		addCreativeItem(ItemInstance(Item::wheat));
	} else if (creativeCategory == CATEGORY_ARMOR) {
		addCreativeItem(ItemInstance(Item::helmet_cloth));
		addCreativeItem(ItemInstance(Item::chestplate_cloth));
		addCreativeItem(ItemInstance(Item::leggings_cloth));
		addCreativeItem(ItemInstance(Item::boots_cloth));
		addCreativeItem(ItemInstance(Item::helmet_chain));
		addCreativeItem(ItemInstance(Item::chestplate_chain));
		addCreativeItem(ItemInstance(Item::leggings_chain));
		addCreativeItem(ItemInstance(Item::boots_chain));
		addCreativeItem(ItemInstance(Item::helmet_iron));
		addCreativeItem(ItemInstance(Item::chestplate_iron));
		addCreativeItem(ItemInstance(Item::leggings_iron));
		addCreativeItem(ItemInstance(Item::boots_iron));
		addCreativeItem(ItemInstance(Item::helmet_diamond));
		addCreativeItem(ItemInstance(Item::chestplate_diamond));
		addCreativeItem(ItemInstance(Item::leggings_diamond));
		addCreativeItem(ItemInstance(Item::boots_diamond));
		addCreativeItem(ItemInstance(Item::helmet_gold));
		addCreativeItem(ItemInstance(Item::chestplate_gold));
		addCreativeItem(ItemInstance(Item::leggings_gold));
		addCreativeItem(ItemInstance(Item::boots_gold));
	}
}

bool IngameBlockSelectionScreen::isAllowed( int slot )
{
	if (minecraft->isCreativeMode())
		return slot >= 0 && slot < (int)creativeItems.size();

	if (slot < 0 || slot >= minecraft->player->inventory->getContainerSize())
		return false;

#ifdef DEMO_MODE
	if (slot >= (minecraft->isCreativeMode()? 28 : 27)) return false;
#endif
	return true;
}

bool IngameBlockSelectionScreen::hasClippingArea( IntRectangle& out )
{
	out = clippingArea;
	return true;
}

std::vector<const ItemInstance*> IngameBlockSelectionScreen::getItems( const InventoryPane* forPane )
{
	std::vector<const ItemInstance*> out;
	if (minecraft->isCreativeMode()) {
		for (unsigned int i = 0; i < creativeItems.size(); ++i)
			out.push_back(&creativeItems[i]);
	} else {
		for (int i = Inventory::MAX_SELECTION_SIZE; i < minecraft->player->inventory->getContainerSize(); ++i)
			out.push_back(minecraft->player->inventory->getItem(i));
	}
	return out;
}

}
