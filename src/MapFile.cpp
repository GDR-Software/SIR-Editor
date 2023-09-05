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

static void LoadCheckpoints(CMap *cMap, FILE *fp)
{
	uint32_t numCheckpoints;
	checkpoint_t check;

    cMap->GetCheckpoints().clear();
    SafeRead(&numCheckpoints, sizeof(uint32_t), fp);
    cMap->GetCheckpoints().reserve(numCheckpoints + 64);
	
	for (uint32_t i = 0; i < numCheckpoints; i++) {
		SafeRead(check.data(), sizeof(checkpoint_t), fp);
		cMap->AddCheckpoint(check);
	}
}

static void LoadSpawns(CMap *cMap, FILE *fp)
{
    uint32_t numSpawns;
    spawn_t spawn;

    cMap->GetSpawns().clear();
    SafeRead(&numSpawns, sizeof(uint32_t), fp);
    cMap->GetSpawns().reserve(numSpawns + 64);

    for (uint32_t i = 0; i < numSpawns; i++) {
        SafeRead(spawn.data(), sizeof(spawn_t), fp);
        cMap->AddSpawn(spawn);
    }
}

static void LoadTiles(CMap *cMap, FILE *fp)
{
    int dims[2];

    SafeRead(dims, sizeof(dims), fp);

    if (cMap->GetWidth() != dims[0] || cMap->GetHeight() != dims[1]) {
        cMap->Resize(dims);
    }

    SafeRead(cMap->GetTiles().data(), sizeof(Tile) * dims[0] * dims[1], fp);
}

bool LoadMap(CMap *cMap, const char *filename)
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
    LoadTiles(cMap, fp);

	fclose(fp);

    cMap->SetModified(true);
	
	return true;
}

static void SaveCheckpoints(const CMap *cMap, FILE *fp)
{
    uint32_t numCheckpoints;

    numCheckpoints = cMap->GetCheckpoints().size();
    SafeWrite(&numCheckpoints, sizeof(uint32_t), fp);

    for (uint32_t i = 0; i < numCheckpoints; i++) {
        SafeWrite(cMap->GetCheckpoints()[i], sizeof(checkpoint_t), fp);
    }
}

static void SaveTiles(const CMap *cMap, FILE *fp)
{
    int dims[2];

    dims[0] = cMap->GetWidth();
    dims[1] = cMap->GetHeight();

    SafeWrite(dims, sizeof(dims), fp);
    SafeWrite(cMap->GetTiles().data(), sizeof(Tile) * dims[0] * dims[1], fp);
}

static void SaveSpawns(const CMap *cMap, FILE *fp)
{
    uint32_t numSpawns;

    numSpawns = cMap->GetSpawns().size();
    SafeWrite(&numSpawns, sizeof(uint32_t), fp);

    for (uint32_t i = 0; i < numSpawns; i++) {
        SafeWrite(cMap->GetSpawns()[i], sizeof(spawn_t), fp);
    }
}

bool SaveMap(const CMap *cMap, const char *filename)
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