#include "Common.hpp"
#include "GUI.h"
#include "Map.h"
#include "Editor.h"

#define MAP_IDENT (('B'<<24)+('F'<<16)+('F'<<8)+'M')
#define MAP_VERSION 1

typedef struct
{
	uint32_t ident;
	uint32_t version;
} mapheader_t;

static void LoadCheckpoints(Map *cMap, FILE *fp)
{
	uint32_t numCheckpoints;
	checkpoint_t check;
	
    SafeRead(&numCheckpoints, sizeof(uint32_t), fp);
	
	for (uint32_t i = 0; i < numCheckpoints; i++) {
		SafeRead(check.data(), sizeof(checkpoint_t), fp);
		
		cMap->AddCheckpoint({check[0], check[1]}, {check[2], check[3]});
	}
}

static void LoadSpawns(Map *cMap, FILE *fp)
{
    uint32_t numSpawns;
    spawn_t spawn;

    SafeRead(&numSpawns, sizeof(uint32_t), fp);
}

bool LoadMap(Map *cMap, const char *filename)
{
	FILE *fp;
	mapheader_t header;
	
	if (!filename[0]) {
		Printf("LoadMap: invalid filename (empty)");
		return false;
	}
	
	fp = fopen(filename, "rb");
	if (!fp) {
		Printf("LoadMap: couldn't open file '%s'", filename);
		return false;
	}

    cMap->Clear();
	
	SafeRead(&header, sizeof(header), fp);
	if (header.ident != MAP_IDENT) {
        Printf("LoadMap: bad map file, identifier is incorrect");
		return false;
	}
	if (header.version != MAP_VERSION) {
		Printf("LoadMap: bad map file, version is incorrect");
		return false;
	}
	
	LoadCheckpoints(cMap, fp);
	LoadSpawns(cMap, fp);
	fclose(fp);
	
	return true;
}

static void SaveCheckpoints(const Map *cMap, FILE *fp)
{
    uint32_t numCheckpoints;

    numCheckpoints = cMap->getCheckpoints().size();
    SafeWrite(&numCheckpoints, sizeof(uint32_t), fp);

    for (uint32_t i = 0; i < numCheckpoints; i++) {
        SafeWrite(cMap->getCheckpoints()[i].data(), sizeof(checkpoint_t), fp);
    }
}

static void SaveTiles(const Map *cMap, FILE *fp)
{
    uint32_t width, height;
    const Tile *tile;

    SafeWrite(cMap->getTiles().data(), sizeof(Tile) * MAX_MAP_WIDTH * MAX_MAP_HEIGHT, fp);
}

static void SaveSpawns(const Map *cMap, FILE *fp)
{
    uint32_t numSpawns;

    numSpawns = cMap->getSpawns().size();
    SafeWrite(&numSpawns, sizeof(uint32_t), fp);

    for (uint32_t i = 0; i < numSpawns; i++) {
        SafeWrite(cMap->getSpawns()[i].data(), sizeof(spawn_t), fp);
    }
}

bool SaveMap(const Map *cMap, const char *filename)
{
    FILE *fp;
    mapheader_t header;

    if (!filename[0]) {
        Printf("SaveMap: invalid filename (empty)");
        return false;
    }

    fp = SafeOpenWrite(filename);

    header.ident = MAP_IDENT;
    header.version = MAP_VERSION;

    SafeWrite(&header, sizeof(header), fp);

    SaveCheckpoints(cMap, fp);
    SaveSpawns(cMap, fp);
    SaveTiles(cMap, fp);

    fclose(fp);

    return true;
}