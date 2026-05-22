#ifndef NET_MINECRAFT_WORLD_LEVEL_STORAGE__RegionFile_H__
#define NET_MINECRAFT_WORLD_LEVEL_STORAGE__RegionFile_H__

#include <map>
#include <string>
#include "../../../raknet/BitStream.h"

typedef std::map<int, bool> FreeSectorMap;

class RegionFile
{
public:
RegionFile(const std::string& basePath);
virtual ~RegionFile();

bool open();
bool readChunk(int x, int z, RakNet::BitStream** destChunkData);
bool writeChunk(int x, int z, RakNet::BitStream& chunkData);
private:
struct RegionState {
RegionState() : file(NULL), offsets(NULL) {}

FILE* file;
std::string filename;
int* offsets;
FreeSectorMap sectorFree;
};

RegionState* getRegion(int x, int z, bool createIfMissing);
bool openRegion(RegionState* region, bool createIfMissing);
bool write(RegionState* region, int sector, RakNet::BitStream& chunkData);
void close();

std::string basePath;
std::map<long long, RegionState*> regions;
int* emptyChunk;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_STORAGE__RegionFile_H__*/
