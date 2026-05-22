#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__CanyonFeature_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__CanyonFeature_H__

#include "LargeFeature.h"

class CanyonFeature : public LargeFeature {
protected:
void addTunnel(int xOffs, int zOffs, unsigned char* blocks, float xCave, float yCave, float zCave, float thickness, float yRot, float xRot, int step, int dist, float yScale);
virtual void addFeature(Level* level, int x, int z, int xOffs, int zOffs, unsigned char* blocks, int blocksSize);
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__CanyonFeature_H__*/
