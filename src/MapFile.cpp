#include "Common.hpp"
#include "GUI.h"
#include "Map.h"
#include "Editor.h"

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
        
        tiles[i].pos[0] = LittleInt(tiles[i].pos[0]);
        tiles[i].pos[1] = LittleInt(tiles[i].pos[1]);

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

uint64_t CopyLump(void *dest, uint64_t size, lump_t *lumps, int lumpnum, FILE *fp)
{
    uint64_t length, ofs;

    length = lumps[lumpnum].length;
    ofs = lumps[lumpnum].fileofs;

    if (length % size) {
        Error("CopyLump: bad lump size");
    }
    SafeRead(dest, length, fp);

    return length / size;
}

uint64_t CopyLump(void **dest, uint64_t size, lump_t *lumps, int lumpnum, FILE *fp)
{
    uint64_t length, ofs;

    length = lumps[lumpnum].length;
    ofs = lumps[lumpnum].fileofs;

    if (length % size) {
        Error("CopyLump: bad lump size");
    }
    *dest = Malloc(length);
    SafeRead(*dest, length, fp);

    return length / size;
}

bool LoadMap(CMap *cMap, const char *filename)
{
	mapheader_t header;
    FILE *fp;
	
	if (!filename[0]) {
		Printf("LoadMap: invalid filename (empty)");
		return false;
	}
    fp = SafeOpenRead(filename);
    SafeRead(&header, sizeof(header), fp);
    SwapBlock((uint32_t *)&header, sizeof(header));

    if (header.ident != MAP_IDENT) {
        Error("Bad BFFM file, identifier is wrong");
    }

    CopyLump(cMap->GetTiles(), sizeof(maptile_t), header.lumps, MAP_LUMP_TILES, fp);
    CopyLump(cMap->GetCheckpoints(), sizeof(mapcheckpoint_t), header.lumps, MAP_LUMP_CHECKPOINTS, fp);
    CopyLump(cMap->GetSpawns(), sizeof(mapspawn_t), header.lumps, MAP_LUMP_SPAWNS, fp);

    fclose(fp);

    SwapMapFile(cMap);
	
	return true;
}

void AddLump(const void *data, uint64_t size, lump_t *lumps, int lumpnum, FILE *fp)
{
    lump_t *lump;

    if (!size)
        return;

    lump = &lumps[lumpnum];

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

    AddLump(cMap->GetTiles().data(), cMap->GetWidth() * cMap->GetHeight() * sizeof(maptile_t), header.lumps, MAP_LUMP_TILES, fp);
    AddLump(cMap->GetCheckpoints().data(), cMap->GetCheckpoints().size() * sizeof(mapcheckpoint_t), header.lumps, MAP_LUMP_CHECKPOINTS, fp);
    AddLump(cMap->GetSpawns().data(), cMap->GetSpawns().size() * sizeof(mapspawn_t), header.lumps, MAP_LUMP_SPAWNS, fp);

    fseek(fp, 0L, SEEK_SET);
    SafeWrite(&header, sizeof(header), fp);

    fclose(fp);

    return true;
}