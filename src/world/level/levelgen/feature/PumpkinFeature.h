#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_FEATURE__PumpkinFeature_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_FEATURE__PumpkinFeature_H__

#include "../../../../util/Random.h"
#include "../../Level.h"
#include "../../tile/Tile.h"
#include "Feature.h"

class PumpkinFeature: public Feature
{
public:
    bool place(Level* level, Random* random, int x, int y, int z) {
        for (int i = 0; i < 64; i++) {
            int x2 = x + random->nextInt(8) - random->nextInt(8);
            int y2 = y + random->nextInt(4) - random->nextInt(4);
            int z2 = z + random->nextInt(8) - random->nextInt(8);
            if (level->isEmptyTile(x2, y2, z2)
                && level->getTile(x2, y2 - 1, z2) == Tile::grass->id
                && Tile::melon->canSurvive(level, x2, y2, z2)) {
                // This fork has melon but no pumpkin block registered. Use melon as
                // the closest visible Beta pumpkin-patch stand-in rather than adding
                // a risky new block ID late in the port.
                level->setTileNoUpdate(x2, y2, z2, Tile::melon->id);
            }
        }
        return true;
    }
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_FEATURE__PumpkinFeature_H__*/
