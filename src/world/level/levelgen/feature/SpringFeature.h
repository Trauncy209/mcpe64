#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_FEATURE__SpringFeature_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_FEATURE__SpringFeature_H__

//package net.minecraft.world.level.levelgen.feature;

#include "Feature.h"
#include "../../Level.h"
#include "../../tile/Tile.h"
#include "../../material/Material.h"
#include "../../../../util/Random.h"

class SpringFeature: public Feature
{
    int tile;

public:
    SpringFeature(int tile) {
        this->tile = tile;
    }

    bool place(Level* level, Random* random, int x, int y, int z) {
        if (level == NULL) return false;
        if (y <= 0 || y >= Level::DEPTH - 1) return false;

        // Springs are placed during chunk post-processing, often right on chunk
        // borders. Never force-load/generate neighboring chunks from this feature:
        // on-device this can recurse through ChunkCache -> postProcess ->
        // SpringFeature and crash in Level::setTile/getChunk.
        if (!level->hasChunksAtNow(x - 1, y - 1, z - 1, x + 1, y + 1, z + 1)) return false;

        if (level->getTile(x, y + 1, z) != Tile::rock->id) return false;
        if (level->getTile(x, y - 1, z) != Tile::rock->id) return false;

        int centerTile = level->getTile(x, y, z);
        if (centerTile != 0 && centerTile != Tile::rock->id) return false;

        int rockCount = 0;
        if (level->getTile(x - 1, y, z) == Tile::rock->id) rockCount++;
        if (level->getTile(x + 1, y, z) == Tile::rock->id) rockCount++;
        if (level->getTile(x, y, z - 1) == Tile::rock->id) rockCount++;
        if (level->getTile(x, y, z + 1) == Tile::rock->id) rockCount++;

        int holeCount = 0;
        if (level->isEmptyTile(x - 1, y, z)) holeCount++;
        if (level->isEmptyTile(x + 1, y, z)) holeCount++;
        if (level->isEmptyTile(x, y, z - 1)) holeCount++;
        if (level->isEmptyTile(x, y, z + 1)) holeCount++;

        if (rockCount == 3 && holeCount == 1) {
            if (!level->setTileNoUpdate(x, y, z, tile)) return false;
            level->setTileDirty(x, y, z);
            if (Tile::tiles[tile] != NULL) {
                level->instaTick = true;
                Tile::tiles[tile]->tick(level, x, y, z, random);
                level->instaTick = false;
            }
        }

        return true;
    }
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_FEATURE__SpringFeature_H__*/
