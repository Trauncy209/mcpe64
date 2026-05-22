#include "CanyonFeature.h"

#include "../Level.h"
#include "../tile/Tile.h"
#include "../../../util/Random.h"
#include "../../../util/Mth.h"

void CanyonFeature::addTunnel(int xOffs, int zOffs, unsigned char* blocks, float xCave, float yCave, float zCave, float thickness, float yRot, float xRot, int step, int dist, float yScale)
{
float xMid = float(xOffs * 16 + 8);
float zMid = float(zOffs * 16 + 8);

float yRota = 0.0f;
float xRota = 0.0f;
Random random(this->random.nextLong());

if (dist <= 0) {
int max = radius * 16 - 16;
dist = max - random.nextInt(max / 4);
}

bool singleStep = false;
if (step == -1) {
step = dist / 2;
singleStep = true;
}

int splitPoint = random.nextInt(dist / 2) + dist / 4;
bool steep = random.nextInt(6) == 0;

for (; step < dist; ++step) {
float rad = 1.5f + (sin(step * Mth::PI / dist) * thickness);
float yRad = rad * yScale;

float xc = cos(xRot);
float xs = sin(xRot);
xCave += cos(yRot) * xc;
yCave += xs;
zCave += sin(yRot) * xc;

xRot *= steep ? 0.92f : 0.7f;
xRot += xRota * 0.1f;
yRot += yRota * 0.1f;

xRota *= 0.50f;
yRota *= 0.50f;
xRota += (random.nextFloat() - random.nextFloat()) * random.nextFloat() * 2.0f;
yRota += (random.nextFloat() - random.nextFloat()) * random.nextFloat() * 4.0f;

if (!singleStep && step == splitPoint && thickness > 1.0f) {
addTunnel(xOffs, zOffs, blocks, xCave, yCave, zCave, random.nextFloat() * 0.5f + 0.5f, yRot - Mth::PI / 2, xRot / 3, step, dist, 1.0f);
addTunnel(xOffs, zOffs, blocks, xCave, yCave, zCave, random.nextFloat() * 0.5f + 0.5f, yRot + Mth::PI / 2, xRot / 3, step, dist, 1.0f);
return;
}
if (!singleStep && random.nextInt(4) == 0)
continue;

float xdMid = xCave - xMid;
float zdMid = zCave - zMid;
float remaining = float(dist - step);
float rr = (thickness + 2.0f) + 16.0f;
if (xdMid * xdMid + zdMid * zdMid - (remaining * remaining) > rr * rr)
return;

if (xCave < xMid - 16 - rad * 2 || zCave < zMid - 16 - rad * 2 || xCave > xMid + 16 + rad * 2 || zCave > zMid + 16 + rad * 2)
continue;

int x0 = int(floor(xCave - rad)) - xOffs * 16 - 1;
int x1 = int(floor(xCave + rad)) - xOffs * 16 + 1;
int y0 = int(floor(yCave - yRad)) - 1;
int y1 = int(floor(yCave + yRad)) + 1;
int z0 = int(floor(zCave - rad)) - zOffs * 16 - 1;
int z1 = int(floor(zCave + rad)) - zOffs * 16 + 1;

if (x0 < 0) x0 = 0;
if (x1 > 16) x1 = 16;
if (y0 < 1) y0 = 1;
if (y1 > 120) y1 = 120;
if (z0 < 0) z0 = 0;
if (z1 > 16) z1 = 16;

bool detectedWater = false;
for (int xx = x0; !detectedWater && xx < x1; ++xx) {
for (int zz = z0; !detectedWater && zz < z1; ++zz) {
for (int yy = y1 + 1; !detectedWater && yy >= y0 - 1; --yy) {
int p = (xx * 16 + zz) * 128 + yy;
if (yy < 0 || yy >= Level::DEPTH)
continue;
if (blocks[p] == Tile::water->id || blocks[p] == Tile::calmWater->id)
detectedWater = true;
if (yy != y0 - 1 && xx != x0 && xx != x1 - 1 && zz != z0 && zz != z1 - 1)
yy = y0;
}
}
}
if (detectedWater)
continue;

for (int xx = x0; xx < x1; ++xx) {
float xd = ((xx + xOffs * 16 + 0.5f) - xCave) / rad;
for (int zz = z0; zz < z1; ++zz) {
float zd = ((zz + zOffs * 16 + 0.5f) - zCave) / rad;
int p = (xx * 16 + zz) * 128 + y1;
bool hasGrass = false;
for (int yy = y1 - 1; yy >= y0; --yy) {
float yd = (yy + 0.5f - yCave) / yRad;
if (yd > -0.7f && xd * xd + yd * yd + zd * zd < 1.0f) {
int block = blocks[p];
if (block == Tile::grass->id)
hasGrass = true;
if (block == Tile::rock->id || block == Tile::dirt->id || block == Tile::grass->id) {
if (yy < 10) {
blocks[p] = (unsigned char)Tile::lava->id;
} else {
blocks[p] = 0;
if (hasGrass && blocks[p - 1] == Tile::dirt->id)
blocks[p - 1] = (unsigned char)Tile::grass->id;
}
}
}
--p;
}
}
}
if (singleStep)
break;
}
}

void CanyonFeature::addFeature(Level* level, int x, int z, int xOffs, int zOffs, unsigned char* blocks, int blocksSize)
{
(void)level;
(void)blocksSize;
if (random.nextInt(15) != 0)
return;

float xCave = float(x * 16 + random.nextInt(16));
float yCave = float(random.nextInt(random.nextInt(120) + 8));
float zCave = float(z * 16 + random.nextInt(16));
float yRot = random.nextFloat() * Mth::PI * 2.0f;
float xRot = ((random.nextFloat() - 0.5f) * 2.0f) / 8.0f;
float thickness = (random.nextFloat() * 2.0f + random.nextFloat()) + 1.0f;

addTunnel(xOffs, zOffs, blocks, xCave, yCave, zCave, thickness, yRot, xRot, 0, 0, 5.0f);
}
