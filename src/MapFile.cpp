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
    }
    
    // checkpoints
    SwapBlock((uint32_t *)cMap->GetCheckpoints().data(), cMap->GetCheckpoints().size() * sizeof(mapcheckpoint_t));

    // spawns
    SwapBlock((uint32_t *)cMap->GetSpawns().data(), cMap->GetSpawns().size() * sizeof(mapspawn_t));
}

uint64_t CopyLump(void *dest, uint64_t size, mapheader_t *header, int lumpnum)
{
    uint64_t length, ofs;

    length = header->lumps[lumpnum].length;
    ofs = header->lumps[lumpnum].fileofs;

    if (length % size) {
        Error("CopyLump: bad lump size");
    }
    memcpy(dest, (byte *)header + ofs, length);
    return length / size;
}

template<typename T>
uint64_t CopyLump(vector_t<T>& dest, uint64_t size, mapheader_t *header, int lumpnum)
{
    uint64_t length;

    length = header->lumps[lumpnum].length;

    if (length % size) {
        Error("CopyLump: odd lump size");
    }
    dest.resize(length / size);
    return CopyLump(dest.data(), size, header, lumpnum);
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
        Error("LoadMap: bad map file, identifier is wrong");
    }
    if (!cMap->GetTileset()) {
        cMap->GetTileset() = Allocate<CTileset>();
    }

    CopyLump<maptile_t>(cMap->GetTiles(), sizeof(maptile_t), header, LUMP_TILES);
    CopyLump<mapcheckpoint_t>(cMap->GetCheckpoints(), sizeof(mapcheckpoint_t), header, LUMP_CHECKPOINTS);
    CopyLump<mapspawn_t>(cMap->GetSpawns(), sizeof(mapspawn_t), header, LUMP_SPAWNS);
    if (header->lumps[LUMP_TILESET].length) {
        cMap->GetTileset()->Read((const byte *)header + header->lumps[LUMP_TILESET].fileofs);
    }

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

    memset(&header, 0, sizeof(header));
    header.ident = MAP_IDENT;
    header.version = MAP_VERSION;

    // overwritten later
    SafeWrite(&header, sizeof(header), fp);

    AddLump(cMap->GetTiles().data(), cMap->GetWidth() * cMap->GetHeight() * sizeof(maptile_t), header.lumps, LUMP_TILES, fp);
    AddLump(cMap->GetCheckpoints().data(), cMap->GetCheckpoints().size() * sizeof(mapcheckpoint_t), header.lumps, LUMP_CHECKPOINTS, fp);
    AddLump(cMap->GetSpawns().data(), cMap->GetSpawns().size() * sizeof(mapspawn_t), header.lumps, LUMP_SPAWNS, fp);
    if (cMap->GetTileset())
        cMap->GetTileset()->Write(fp);

    fseek(fp, 0L, SEEK_SET);
    SafeWrite(&header, sizeof(header), fp);

    fclose(fp);

    return true;
}