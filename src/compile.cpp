// compile.cpp: compiles multiple files into one single .bmf file

#include "Common.hpp"

static void *GetMemory(size_t size)
{
    void *buf;

    buf = malloc(size);
    if (!buf) {
        Error("GetMemory: not enough memory (RAM) to satisfy request of %lu bytes", size);
    }

    return buf;
}

static void FreeMemory(void *ptr)
{
    if (!ptr) {
        Error("double free (NULL pointer)");
    }

    free(ptr);
}

static void AddLump(const void *data, uint64_t size, mapheader_t *header, int lumpnum, FILE *fp)
{
    lump_t *lump;

    lump = &header->lumps[lumpnum];
    lump->length = size;
    lump->fileofs = ftell(fp);

    SafeWrite(data, PAD(size, sizeof(uint32_t)), fp);
}

static uint64_t CopyLump(void **dest, uint64_t size, mapheader_t *header, int lumpnum)
{
    uint64_t length, ofs;

    length = header->lumps[lumpnum].length;
    ofs = header->lumps[lumpnum].fileofs;

    if (length % size) {
        Error("CopyLump: odd lump size");
    }
    *dest = GetMemory(length);
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
static tile2d_sprite_t *sprites;
static uint64_t numSprites;

static void LoadMap(const char *mapfile)
{
    mapheader_t *header;

    if (!FileExists(mapfile)) {
        Error("LoadMap: map file '%s' doesn't exist", mapfile);
    }

    LoadFile(mapfile, (void **)&header);

    if (header->ident != MAP_IDENT) {
        Error("LoadMap: map file is not a BFF map");
    }
    if (header->version != MAP_VERSION) {
        Error("LoadMap: differing map versions");
    }

    numTiles = CopyLump((void **)&tiles, sizeof(maptile_t), header, LUMP_TILES);
    numCheckpoints = CopyLump((void **)&checkpoints, sizeof(mapcheckpoint_t), header, LUMP_CHECKPOINTS);
    numSpawns = CopyLump((void **)&spawns, sizeof(mapspawn_t), header, LUMP_SPAWNS);

    FreeMemory(header);
}

static void LoadTileset(const char *tilesetfile, tile2d_info_t *info)
{
    tile2d_header_t *header;

    if (!FileExists(tilesetfile)) {
        Error("LoadTileset: tileset file '%s' doesn't exist", tilesetfile);
    }

    LoadFile(tilesetfile, (void **)&header);

    if (header->magic != TILE2D_MAGIC) {
        Error("LoadTileset: incorrect .tile2d magic");
    }
    if (header->version != TILE2D_VERSION) {
        Error("LoadTileset: differing tileset versions");
    }

    numSprites = header->info.numTiles;
    sprites = (tile2d_sprite_t *)GetMemory(sizeof(tile2d_sprite_t) * numSprites);

    memcpy(sprites, header->sprites, sizeof(tile2d_sprite_t) * numSprites);
    memcpy(info, &header->info, sizeof(info));

    FreeMemory(header);
}

static void LoadTexture(const char *texturefile, byte **buf, uint64_t *buflen, tex2d_t *parms)
{
    tex2d_t *file;

    if (!FileExists(texturefile)) {
        Error("LoadTexture: texture file '%s' doesn't exist", texturefile);
    }

    *buflen = LoadFile(texturefile, (void **)&file);
    memcpy(parms, file, sizeof(*parms));

    *buf = (byte *)GetMemory(file->width * file->height * file->channels);
    memcpy(buf, (byte *)(file + 1), file->width * file->height * file->channels);

    FreeMemory(file);
}

void CompileBMF(const char *output, const char *map, const char *tileset, const char *texture)
{
    bmf_t bmf;
    byte *texBuffer;
    uint64_t texSize;
    FILE *fp;

    fp = SafeOpenWrite(output);

    bmf.ident = LEVEL_IDENT;
    bmf.version = LEVEL_VERSION;

    LoadMap(map);
    LoadTileset(tileset, &bmf.tileset.info);
    LoadTexture(texture, &texBuffer, &texSize, &bmf.texture);

    //
    // write everything
    //

    // overwritten later
    SafeWrite(&bmf, sizeof(bmf), fp);

    AddLump(tiles, numTiles * sizeof(maptile_t), &bmf.map, LUMP_TILES, fp);
    AddLump(checkpoints, numCheckpoints * sizeof(mapcheckpoint_t), &bmf.map, LUMP_CHECKPOINTS, fp);
    AddLump(spawns, numSpawns * sizeof(mapspawn_t), &bmf.map, LUMP_SPAWNS, fp);

    SafeWrite(sprites, sizeof(tile2d_sprite_t) * numSprites, fp);
    SafeWrite(texBuffer, texSize, fp);

    fseek(fp, 0L, SEEK_SET);
    SafeWrite(&bmf, sizeof(bmf), fp);

    fclose(fp);
}

static void print_help(void)
{
    printf(
        "usage: %s [options...] -o <out>\n"
        "[options]\n"
        "\t--map <file>     provide a map file (ext = .map)\n"
        "\t--tileset <file> provide a tileset file (ext = .tile2d)\n"
        "\t--tex <file>     provide a texture file (any image or .tex2d)\n"
    , myargv[0]);
}

int main(int argc, char **argv)
{
    myargc = argc;
    myargv = argv;
    if (argc < 2) {
        print_help();
        return 0;
    }

    const char *output;
    const char *map, *tileset, *texture;

    for (int i = 0; i < argc; i++) {
        if (!N_stricmp(argv[i], "-o")) {
            output = argv[i + 1];
            break;
        }
        else if (!N_stricmp(argv[i], "--map")) {
            map = argv[i + 1];
        }
        else if (!N_stricmp(argv[i], "--tileset")) {
            tileset = argv[i + 1];
        }
        else if (!N_stricmp(argv[i], "--tex")) {
            texture = argv[i + 1];
        }
        else {
            printf("WARNING: unrecognized command line parm: %s\n", argv[i]);
        }
    }
    if (!output) {
        Error("output file not provided");
    }

    CompileBMF(output, map, tileset, texture);

    return 0;
}
