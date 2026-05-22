#include "RegionFile.h"
#include "FolderMethods.h"
#include "../../../platform/log.h"

#include <cassert>
#include <cstdio>
#include <cstring>

const int SECTOR_BYTES = 4096;
const int SECTOR_INTS = SECTOR_BYTES / 4;
const int SECTOR_COLS = 32;

static const char* const REGION_DIR_NAME = "region";
static const char* const REGION_FILE_EXT = ".mcr";

static void logAssert(int actual, int expected) {
if (actual != expected) {
LOGI("ERROR: I/O operation failed (%d vs %d)\n", actual, expected);
}
}

static int floorDiv32(int value)
{
if (value >= 0)
return value / SECTOR_COLS;
return -(((-value) + SECTOR_COLS - 1) / SECTOR_COLS);
}

static int mod32(int value)
{
int result = value % SECTOR_COLS;
if (result < 0)
result += SECTOR_COLS;
return result;
}

static long long getRegionKey(int regionX, int regionZ)
{
return (((long long)regionX) << 32) ^ (unsigned int)regionZ;
}

RegionFile::RegionFile(const std::string& basePath)
:basePath(basePath)
{
emptyChunk = new int[SECTOR_INTS];
memset(emptyChunk, 0, SECTOR_INTS * sizeof(int));
}

RegionFile::~RegionFile()
{
close();
delete [] emptyChunk;
}

bool RegionFile::open()
{
if (!createFolderIfNotExists(basePath.c_str()))
return false;

std::string regionDir = basePath + "/" + REGION_DIR_NAME;
return createFolderIfNotExists(regionDir.c_str());
}

void RegionFile::close()
{
for (std::map<long long, RegionState*>::iterator it = regions.begin(); it != regions.end(); ++it)
{
RegionState* region = it->second;
if (!region)
continue;

if (region->file)
fclose(region->file);
delete [] region->offsets;
delete region;
}
regions.clear();
}

bool RegionFile::openRegion(RegionState* region, bool createIfMissing)
{
if (!region)
return false;
if (region->file)
return true;

region->offsets = new int[SECTOR_INTS];
memset(region->offsets, 0, SECTOR_INTS * sizeof(int));

region->file = fopen(region->filename.c_str(), "r+b");
if (!region->file)
{
if (!createIfMissing)
return false;

region->file = fopen(region->filename.c_str(), "w+b");
if (!region->file)
{
LOGI("Failed to create region file %s\n", region->filename.c_str());
return false;
}

logAssert(fwrite(region->offsets, sizeof(int), SECTOR_INTS, region->file), SECTOR_INTS);
region->sectorFree[0] = false;
return true;
}

logAssert(fread(region->offsets, sizeof(int), SECTOR_INTS, region->file), SECTOR_INTS);

fseek(region->file, 0, SEEK_END);
const long fileSize = ftell(region->file);
const int totalSectors = int((fileSize + SECTOR_BYTES - 1) / SECTOR_BYTES);
for (int sector = 0; sector < totalSectors; ++sector)
region->sectorFree[sector] = true;
region->sectorFree[0] = false;

for (int sector = 0; sector < SECTOR_INTS; sector++)
{
int offset = region->offsets[sector];
if (!offset)
continue;

int base = offset >> 8;
int count = offset & 0xff;
for (int i = 0; i < count; i++)
region->sectorFree[base + i] = false;
}

return true;
}

RegionFile::RegionState* RegionFile::getRegion(int x, int z, bool createIfMissing)
{
const int regionX = floorDiv32(x);
const int regionZ = floorDiv32(z);
const long long key = getRegionKey(regionX, regionZ);

std::map<long long, RegionState*>::iterator it = regions.find(key);
if (it != regions.end())
{
if (!it->second->file && !openRegion(it->second, createIfMissing))
return NULL;
return it->second;
}

RegionState* region = new RegionState();
region->filename = basePath + "/" + REGION_DIR_NAME + "/r." + std::to_string(regionX) + "." + std::to_string(regionZ) + REGION_FILE_EXT;
regions[key] = region;

if (!openRegion(region, createIfMissing))
{
if (!createIfMissing)
{
delete [] region->offsets;
delete region;
regions.erase(key);
}
return NULL;
}

return region;
}

bool RegionFile::readChunk(int x, int z, RakNet::BitStream** destChunkData)
{
RegionState* region = getRegion(x, z, false);
if (!region || !region->file)
return false;

const int localX = mod32(x);
const int localZ = mod32(z);
const int offset = region->offsets[localX + localZ * SECTOR_COLS];
if (offset == 0)
return false;

const int sectorNum = offset >> 8;
fseek(region->file, sectorNum * SECTOR_BYTES, SEEK_SET);

int length = 0;
fread(&length, sizeof(int), 1, region->file);
assert(length < ((offset & 0xff) * SECTOR_BYTES));
length -= sizeof(int);
if (length <= 0)
return false;

unsigned char* data = new unsigned char[length];
logAssert(fread(data, 1, length, region->file), length);
*destChunkData = new RakNet::BitStream(data, length, false);
return true;
}

bool RegionFile::writeChunk(int x, int z, RakNet::BitStream& chunkData)
{
RegionState* region = getRegion(x, z, true);
if (!region || !region->file)
return false;

const int localX = mod32(x);
const int localZ = mod32(z);
const int tableIndex = localX + localZ * SECTOR_COLS;
const int size = chunkData.GetNumberOfBytesUsed() + sizeof(int);

int offset = region->offsets[tableIndex];
int sectorNum = offset >> 8;
int sectorCount = offset & 0xff;
int sectorsNeeded = (size / SECTOR_BYTES) + 1;

if (sectorsNeeded > 256)
{
LOGI("ERROR: Chunk is too big to be saved to file\n");
return false;
}

if (sectorNum != 0 && sectorCount == sectorsNeeded)
{
write(region, sectorNum, chunkData);
return true;
}

for (int i = 0; i < sectorCount; i++)
region->sectorFree[sectorNum + i] = true;

int slot = 0;
int runLength = 0;
bool extendFile = false;
while (runLength < sectorsNeeded)
{
if (region->sectorFree.find(slot + runLength) == region->sectorFree.end())
{
extendFile = true;
break;
}
if (region->sectorFree[slot + runLength])
{
runLength++;
}
else
{
slot = slot + runLength + 1;
runLength = 0;
}
}

if (extendFile)
{
fseek(region->file, 0, SEEK_END);
for (int i = 0; i < (sectorsNeeded - runLength); i++)
{
fwrite(emptyChunk, sizeof(int), SECTOR_INTS, region->file);
region->sectorFree[slot + runLength + i] = true;
}
}

region->offsets[tableIndex] = (slot << 8) | sectorsNeeded;
for (int i = 0; i < sectorsNeeded; i++)
region->sectorFree[slot + i] = false;

write(region, slot, chunkData);
fseek(region->file, tableIndex * sizeof(int), SEEK_SET);
fwrite(&region->offsets[tableIndex], sizeof(int), 1, region->file);
return true;
}

bool RegionFile::write(RegionState* region, int sector, RakNet::BitStream& chunkData)
{
fseek(region->file, sector * SECTOR_BYTES, SEEK_SET);
int size = chunkData.GetNumberOfBytesUsed() + sizeof(int);
logAssert(fwrite(&size, sizeof(int), 1, region->file), 1);
logAssert(fwrite(chunkData.GetData(), 1, chunkData.GetNumberOfBytesUsed(), region->file), chunkData.GetNumberOfBytesUsed());
return true;
}
