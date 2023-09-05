#include "Common.hpp"
#include "GUI.h"
#include "Map.h"
#include "Editor.h"

#define MAP_IDENT (('B'<<24)+('F'<<16)+('F'<<8)+'M')
#define MAP_VERSION 1

typedef struct {
    uint64_t fileofs;
    uint64_t length;
} maplump_t;

#define LUMP_TILES 0
#define LUMP_CHECKPOINTS 1
#define LUMP_SPAWNS 2
#define LUMP_LIGHTS 3
#define LUMP_VERTICES 4
#define LUMP_INDICES 5
#define NUMLUMPS 6

typedef struct
{
	uint32_t ident;
	uint32_t version;
    maplump_t lumps[NUMLUMPS];
} mapheader_t;

static void SwapBlock(uint32_t *block, uint64_t size)
{
    size >>= 2;
    for (uint64_t i = 0; i < size; i++) {
        block[i] = LittleInt(block[i]);
    }
}

void SwapMapFile(CMap *cMap)
{
    uint32_t i;
    maptile_t *tiles;
    
    tiles = cMap->GetTiles().data();

    // tiles
    for (i = 0; i < cMap->GetTiles().size(); i++) {
        tiles[i].index = LittleInt(tiles[i].index);

        tiles[i].texcoords[0][0] = LittleFloat(tiles[i].texcoords[0][0]);
        tiles[i].texcoords[0][1] = LittleFloat(tiles[i].texcoords[0][1]);
        tiles[i].texcoords[1][0] = LittleFloat(tiles[i].texcoords[1][0]);
        tiles[i].texcoords[1][1] = LittleFloat(tiles[i].texcoords[1][1]);
        tiles[i].texcoords[2][0] = LittleFloat(tiles[i].texcoords[2][0]);
        tiles[i].texcoords[2][1] = LittleFloat(tiles[i].texcoords[2][1]);
        tiles[i].texcoords[3][0] = LittleFloat(tiles[i].texcoords[3][0]);
        tiles[i].texcoords[3][1] = LittleFloat(tiles[i].texcoords[3][1]);
    }
    
    // checkpoints
    SwapBlock((uint32_t *)cMap->GetCheckpoints().data(), cMap->GetCheckpoints().size() * sizeof(mapcheckpoint_t));

    // spawns
    SwapBlock((uint32_t *)cMap->GetSpawns().data(), cMap->GetSpawns().size() * sizeof(mapspawn_t));
}

template<typename T>
static uint64_t CopyLump(vector_t<T>& dest, uint64_t size, int lumpnum, mapheader_t *header)
{
    uint64_t length, ofs;
    
    length = header->lumps[lumpnum].length;
    ofs = header->lumps[lumpnum].fileofs;

    if (length % size) {
        Error("CopyLump: bad lump size");
    }
    dest.resize(length / size);
    memcpy(dest.data(), (byte *)header + ofs, length);

    return length / size;
}

bool LoadMap(CMap *cMap, const char *filename)
{
	mapheader_t *header;
	
	if (!filename[0]) {
		Printf("LoadMap: invalid filename (empty)");
		return false;
	}

    LoadFile(filename, (void **)&header);
    SwapBlock((uint32_t *)header, sizeof(*header));

    if (header->ident != MAP_IDENT) {
        Error("Bad BFFM file, identifier is wrong");
    }

    CopyLump(cMap->GetTiles(), sizeof(maptile_t), LUMP_TILES, header);
    CopyLump(cMap->GetCheckpoints(), sizeof(mapcheckpoint_t), LUMP_CHECKPOINTS, header);
    CopyLump(cMap->GetSpawns(), sizeof(mapspawn_t), LUMP_SPAWNS, header);

    SwapMapFile(cMap);
//    CopyLump(maplights, sizeof(maplight_t), LUMP_LIGHTS, header);
//    numVertices = CopyLump((void **)&mapVertices, sizeof(mapvert_t), LUMP_VERTICES, header);
//    numIndices = CopyLump((void **)&mapIndices, sizeof(uint32_t), LUMP_INDICES, header);
	
	return true;
}

static void AddLump(const void *data, uint64_t size, int lumpnum, mapheader_t *header, FILE *fp)
{
    maplump_t *lump;

    if (!size)
        return;

    lump = &header->lumps[lumpnum];

    lump->fileofs = LittleLong(ftell(fp));
    lump->length = LittleLong(size);

    SafeWrite(data, PAD(size, sizeof(uint32_t)), fp);
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

    // overwritten later
    SafeWrite(&header, sizeof(header), fp);

    AddLump(cMap->GetTiles().data(), cMap->GetTiles().size() * sizeof(maptile_t), LUMP_TILES, &header, fp);
    AddLump(cMap->GetCheckpoints().data(), cMap->GetCheckpoints().size() * sizeof(mapcheckpoint_t), LUMP_CHECKPOINTS, &header, fp);
    AddLump(cMap->GetSpawns().data(), cMap->GetSpawns().size() * sizeof(mapspawn_t), LUMP_SPAWNS, &header, fp);

    fseek(fp, 0L, SEEK_SET);
    SafeWrite(&header, sizeof(header), fp);

    fclose(fp);

    return true;
}