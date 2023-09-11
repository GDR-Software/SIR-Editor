// optimize.cpp: meant to optimize a glnomad map/level file, possibly making it smaller and faster

#include "Common.hpp"

static uint64_t CopyLump(void **dest, uint64_t size, mapheader_t *header, int lumpnum)
{
    uint64_t length, ofs;

    length = header->lumps[lumpnum].length;
    ofs = header->lumps[lumpnum].fileofs;

    if (length % size)
        Error("CopyLump: odd lump size");
    
    *dest = Malloc(length);
    memcpy(*dest, (byte *)header + ofs, length);

    return length / size;
}

static maptexture_t *textures;
static uint64_t numTextures;
static mapcheckpoint_t *checkpoints;
static uint64_t numCheckpoints;
static mapspawn_t *spawns;
static uint64_t numSpawns;
static maplight_t *lights;
static uint64_t numLights;
static maptile_t *tiles;
static uint64_t numTiles;
static mapvert_t *vertices;
static uint64_t numVertices;
static uint32_t *indices;
static uint64_t numIndices;

void Optimize_Map(const char *filepath)
{
    mapheader_t *header;

    if (!FileExists(filepath)) {
        Printf("Optimize_Map: filepath '%s' doesn't exist", filepath);
        return;
    }

    LoadMap();

    LoadFile(filepath, (void **)&header);

    numTiles = CopyLump((void **)&tiles, sizeof(maptile_t), header, MAP_LUMP_TILES);
    numCheckpoints = CopyLump((void **)&checkpoints, sizeof(mapcheckpoint_t), header, MAP_LUMP_CHECKPOINTS);
    numSpawns = CopyLump((void **)&spawns, sizeof(mapspawn_t), header, MAP_LUMP_SPAWNS);

    Free(header);
}

void Shrink_Tiles(void)
{
    
}

void Shrink_VerticesAndIndices(void)
{
    // multiple of four
    if (!(numVertices % 4)) {
        
    }
}
