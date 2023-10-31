// compile.cpp: compiles multiple files into one single .bmf file

#ifndef BMFC
    #define BMFC
#endif // BMFC
#include "gln.h"
#include <filesystem>
inline const std::filesystem::path pwdString = std::filesystem::current_path();
#include "gln.cpp"
#include "parse.cpp"
#include "stream.cpp"
#include "map.cpp"

static tile2d_info_t tilesetInfo;

typedef enum {
    CHUNK_CHECKPOINT,
    CHUNK_SPAWN,
    CHUNK_LIGHT,
    CHUNK_TILE,
    CHUNK_TILESET,
    
    CHUNK_INVALID
} chunkType_t;

static bool ParseChunk(const char **text, CMapData *tmpData)
{
    const char *tok;
    chunkType_t type;

    type = CHUNK_INVALID;

    while (1) {
        tok = COM_ParseExt(text, qtrue);
        if (!tok[0]) {
            COM_ParseWarning("no matching '}' found");
            return false;
        }
        
        if (tok[0] == '}') {
            break;
        }
        //
        // classname <name>
        //
        else if (!N_stricmp(tok, "classname")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseWarning("missing parameter for classname");
                return false;
            }
            if (!N_stricmp(tok, "map_checkpoint")) {
                tmpData->mCheckpoints.emplace_back();
                type = CHUNK_CHECKPOINT;
            }
            else if (!N_stricmp(tok, "map_spawn")) {
                tmpData->mSpawns.emplace_back();
                type = CHUNK_SPAWN;
            }
            else if (!N_stricmp(tok, "map_light")) {
                tmpData->mLights.emplace_back();
                type = CHUNK_LIGHT;
            }
            else if (!N_stricmp(tok, "map_tile")) {
                tmpData->mTiles.emplace_back();
                type = CHUNK_TILE;
            }
            else if (!N_stricmp(tok, "map_tileset")) {
                type = CHUNK_TILESET;
            }
            else {
                COM_ParseWarning("unrecognized token for classname '%s'", tok);
                return false;
            }
        }
        //
        // entity <entitytype>
        //
        else if (!N_stricmp(tok, "entity")) {
            if (type != CHUNK_SPAWN) {
                COM_ParseError("found parameter \"entity\" in chunk that isn't a spawn");
                return false;
            }
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for spawn entity type");
                return false;
            }
            tmpData->mSpawns.back().entitytype = (uint32_t)atoi(tok);
        }
        //
        // tileCountX <count>
        //
        else if (!N_stricmp(tok, "tileCountX")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for tileset tileCountX");
                return false;
            }
            tilesetInfo.tileCountX = (uint32_t)atoi(tok);
        }
        //
        // tileCountY <count>
        //
        else if (!N_stricmp(tok, "tileCountY")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for tileset tileCountY");
                return false;
            }
            tilesetInfo.tileCountY = (uint32_t)atoi(tok);
        }
        //
        // numTiles <number>
        //
        else if (!N_stricmp(tok, "numTiles")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for tileset numTiles");
                return false;
            }
            tilesetInfo.numTiles = (uint32_t)atoi(tok);
        }
        //
        // tileWidth <width>
        //
        else if (!N_stricmp(tok, "tileWidth")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for tileset tileWidth");
                return false;
            }
            tilesetInfo.tileWidth = (uint32_t)atoi(tok);
        }
        //
        // texture <path>
        //
        else if (!N_stricmp(tok, "texture")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for tileset texture");
                return false;
            }
            if (strlen(GetFilename(tok)) >= MAX_GDR_PATH) {
                Error("Texture path '%s' too long", tok);
            }
            N_strncpyz(tilesetInfo.texture, tok, MAX_GDR_PATH);
        }
        //
        // tileHeight <height>
        //
        else if (!N_stricmp(tok, "tileHeight")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for tileset tileHeight");
                return false;
            }
            tilesetInfo.tileHeight = (uint32_t)atoi(tok);
        }
        //
        // texIndex <index>
        //
        else if (!N_stricmp(tok, "texIndex")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map tile texIndex");
                return false;
            }
            tmpData->mTiles.back().index = (int32_t)atoi(tok);
        }
        //
        // id <entityid>
        //
        else if (!N_stricmp(tok, "id")) {
            if (type != CHUNK_SPAWN) {
                COM_ParseError("found parameter \"id\" in chunk that isn't a spawn");
                return false;
            }
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for spawn entity id");
                return false;
            }
            tmpData->mSpawns.back().entityid = (uint32_t)atoi(tok);
        }
        //
        // flags <flags>
        //
        else if (!N_stricmp(tok, "flags")) {
            if (type != CHUNK_TILE) {
                COM_ParseError("found parameter \"flags\" in chunk that isn't a tile");
                return false;
            }
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for tile flags");
                return false;
            }
            tmpData->mTiles.back().flags = (uint32_t)ParseHex(tok);
        }
        //
        // sides <sides...>
        //
        else if (!N_stricmp(tok, "sides")) {
            int sides[5];
            if (!Parse1DMatrix(text, 5, (float *)sides)) {
                COM_ParseError("failed to parse sides for map tile");
                return false;
            }
            tmpData->mTiles.back().sides[0] = sides[0];
            tmpData->mTiles.back().sides[1] = sides[1];
            tmpData->mTiles.back().sides[2] = sides[2];
            tmpData->mTiles.back().sides[3] = sides[3];
            tmpData->mTiles.back().sides[4] = sides[4];
        }
        //
        // texcoords <texcoords...>
        //
        else if (!N_stricmp(tok, "texcoords")) {
            float coords[4 * 2];
            if (!Parse2DMatrix(text, 4, 2, coords)) {
                COM_ParseError("failed to parse texture coordinates for map tile");
                return false;
            }
            memcpy(tmpData->mTiles.back().texcoords, coords, sizeof(coords));
        }
        //
        // pos <x y elevation>
        //
        else if (!N_stricmp(tok, "pos")) {
            uint32_t *xyz;
            if (type == CHUNK_INVALID) {
                COM_ParseError("chunk type not specified before parameters");
                return false;
            } else if (type == CHUNK_CHECKPOINT) {
                xyz = tmpData->mCheckpoints.back().xyz;
            } else if (type == CHUNK_SPAWN) {
                xyz = tmpData->mSpawns.back().xyz;
            }

            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for pos.x");
                return false;
            }
            xyz[0] = static_cast<uint32_t>(clamp(atoi(tok), 0, tmpData->mWidth));

            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for pos.y");
                return false;
            }
            xyz[1] = static_cast<uint32_t>(clamp(atoi(tok), 0, tmpData->mHeight));
            
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for pos.elevation");
                return false;
            }
            xyz[2] = (uint32_t)atoi(tok);
        }
        //
        // brightness <value>
        //
        else if (!N_stricmp(tok, "brightness")) {
            if (type != CHUNK_LIGHT) {
                COM_ParseError("found parameter \"brightness\" in chunk that isn't a light");
                return false;
            }

            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for brightness");
                return false;
            }
            tmpData->mLights.back().brightness = static_cast<float>(atof(tok));
        }
        //
        // color <r g b a>
        //
        else if (!N_stricmp(tok, "color")) {
            if (type != CHUNK_LIGHT) {
                COM_ParseError("found parameter \"color\" in chunk that isn't a light");
                return false;
            }

            if (!Parse1DMatrix(text, 4, tmpData->mLights.back().color)) {
                COM_ParseError("failed to parse light color");
                return false;
            }
        }
        //
        // range <range>
        //
        else if (!N_stricmp(tok, "range")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for light range");
                return false;
            }
            tmpData->mLights.back().range = atof(tok);
        }
        //
        // origin <x y elevation>
        //
        else if (!N_stricmp(tok, "origin")) {
            if (type != CHUNK_LIGHT) {
                COM_ParseError("found parameter \"origin\" in chunk that isn't a light");
                return false;
            }

            vec3_t origin;
            if (!Parse1DMatrix(text, 3, origin)) {
                COM_ParseError("failed to parse light origin");
                return false;
            }
            VectorCopy(tmpData->mLights.back().origin, origin);
        }
        else {
            COM_ParseWarning("unrecognized token '%s'", tok);
            continue;
        }
    }
    return true;
}

static bool ParseMap(const char **text, const char *path, CMapData *tmpData)
{
    const char *tok;

    COM_BeginParseSession(path);

    tok = COM_ParseExt(text, qtrue);
    if (tok[0] != '{') {
        COM_ParseWarning("expected '{', got '%s'", tok);
        return false;
    }

    while (1) {
        tok = COM_ParseComplex(text, qtrue);
        if (!tok[0]) {
            COM_ParseWarning("no concluding '}' in map file '%s'", path);
            return false;
        }
        // end of map file
        if (tok[0] == '}') {
            break;
        }
        // chunk definition
        else if (tok[0] == '{') {
            if (!ParseChunk(text, tmpData)) {
                return false;
            }
            continue;
        }
        //
        // General Map Info
        //
        else if (!N_stricmp(tok, "name")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map name");
                return false;
            }
            tmpData->mName = tok;
        }
        else if (!N_stricmp(tok, "width")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map width");
                return false;
            }
            tmpData->mWidth = (uint32_t)atoi(tok);
        }
        else if (!N_stricmp(tok, "height")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map height");
                return false;
            }
            tmpData->mHeight = (uint32_t)atoi(tok);
        }
        else if (!N_stricmp(tok, "ambientIntensity")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map ambient light");
                return false;
            }
            tmpData->mAmbientIntensity = atof(tok);
        }
        else if (!N_stricmp(tok, "ambientColor")) {
            if (!Parse1DMatrix(text, 3, &tmpData->mAmbientColor[0])) {
                COM_ParseError("failed to parse map ambient color");
                return false;
            }
        }
        else if (!N_stricmp(tok, "ambientType")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map ambientType");
                return false;
            }
            if (!N_stricmp(tok, "dark")) {
                tmpData->mDarkAmbience = true;
            }
            else if (!N_stricmp(tok, "light")) {
                tmpData->mDarkAmbience = false;
            }
            else {
                COM_ParseError("invalid parameter for map ambientType '%s'", tok);
            }
        }
        else if (!N_stricmp(tok, "numCheckpoints")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map numCheckpoints");
                return false;
            }
            tmpData->mCheckpoints.reserve((size_t)atoi(tok));
        }
        else if (!N_stricmp(tok, "numSpawns")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map numSpawns");
                return false;
            }
            tmpData->mSpawns.reserve((size_t)atoi(tok));
        }
        else if (!N_stricmp(tok, "numLights")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map numLights");
                return false;
            }
            tmpData->mLights.reserve((size_t)atoi(tok));
        }
        else if (!N_stricmp(tok, "numTiles")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map numTiles");
                return false;
            }
            tmpData->mTiles.reserve((size_t)atoi(tok));
        }
        else if (!N_stricmp(tok, "numEntities")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map numEntities");
                return false;
            }
            tmpData->mEntities.reserve((size_t)atoi(tok));
        }
        else {
            COM_ParseWarning("unrecognized token: '%s'", tok);
        }
    }
    return true;
}

bool Map_LoadFile(IDataStream *file, const char *ext, const char *rpath)
{
    uint64_t fileLen;
    char *buf, *ptr;
    const char **text;
    CMapData tmpData;

    fileLen = file->GetLength();

    tmpData.Clear();
    buf = (char *)GetMemory(fileLen);
    file->Read(buf, fileLen);
    file->Close();

    ptr = buf;
    text = (const char **)&ptr;

    if (!ParseMap(text, rpath, &tmpData)) {
        Printf("Error: failed to load map");
        return false;
    }
    else {
        *mapData = tmpData;
        mapData->mPath = rpath;
    }
    FreeMemory(buf);

    return true;
}

void Map_Load(const char *filename)
{
    FileStream file;
    Printf("Loading map file '%s'", filename);

    if (file.Open(filename, "r")) {
        Map_LoadFile(&file, GetExtension(filename), filename);
    }
    else {
        Printf("Failed to load map file '%s'", filename);
    }
}

static uint64_t CopyLump(void **dest, uint64_t size, mapheader_t *header, int lumpnum)
{
    uint64_t length, ofs;

    length = header->lumps[lumpnum].length;
    ofs = header->lumps[lumpnum].fileofs;

    if (length % size)
        Error("CopyLump: funny lump size");
    
    *dest = GetMemory(length);
    memcpy(*dest, (byte *)header + ofs, length);
    
    return length / size;
}

static void AddLump(const void *data, uint64_t size, mapheader_t *header, int lumpnum, FILE *fp)
{
    lump_t *lump;

    lump = &header->lumps[lumpnum];
    lump->length = size;
    lump->fileofs = LittleLong(ftello64(fp));

    SafeWrite(data, PAD(size, sizeof(uint32_t)), fp);
}

static INLINE void GenCoords(const glm::vec2& sheetDims, const glm::vec2& spriteDims, const glm::vec2& coords, float texcoords[4][2])
{
    const glm::vec2 min = { ((coords.x + 1) * spriteDims.x) / sheetDims.x, ((coords.y + 1) * spriteDims.y) / sheetDims.y };
    const glm::vec2 max = { (coords.x * spriteDims.x) / sheetDims.x, (coords.y * spriteDims.y) / sheetDims.y };

    texcoords[0][0] = min.x;
    texcoords[0][1] = max.y;

    texcoords[1][0] = min.x;
    texcoords[1][1] = min.y;

    texcoords[2][0] = max.x;
    texcoords[2][1] = min.y;
        
    texcoords[3][0] = max.x;
    texcoords[3][1] = max.y;
}

tile2d_sprite_t *GenerateSprites(void)
{
    tile2d_sprite_t *sprites;
    uint32_t i;
    const glm::vec2 sheetDims = { tilesetInfo.tileCountX * tilesetInfo.tileWidth, tilesetInfo.tileCountY * tilesetInfo.tileHeight };

    sprites = (tile2d_sprite_t *)GetClearedMemory(sizeof(*sprites) * tilesetInfo.numTiles);

    for (uint32_t y = 0; y < sheetDims.y; y++) {
        for (uint32_t x = 0; x < sheetDims.x; x++) {
            vec2_t uv[4];
            GenCoords(sheetDims, { tilesetInfo.tileWidth, tilesetInfo.tileHeight }, { x, y }, uv);

            sprites[i].index = y * tilesetInfo.tileCountX + x;
            memcpy(sprites[i].uv, uv, sizeof(sprites[i].uv));
        }
    }

    return sprites;
}

// engine-specific draw data
typedef struct {
    vec3_t xyz;
    vec3_t color;
    vec3_t normal;
    vec2_t uv;
} drawVert_t;

void PrepareDrawData(void)
{
    drawVert_t *vertices;
    uint32_t *indices;

    vertices = (drawVert_t *)GetMemory(sizeof(drawVert_t) * MAX_MAP_VERTICES);
    indices = (uint32_t *)GetMemory(sizeof(uint32_t) * MAX_MAP_INDICES);

    for (uint32_t y = 0; y < mapData->mHeight; y++) {
        for (uint32_t x = 0; x < mapData->mWidth; x++) {

        }
    }
}

void WriteBMF(const char *filename, bmf_t *data)
{
    FILE *fp;

    if (strlen(GetFilename(filename)) >= MAX_GDR_PATH) {
        Error("Map name '%s' is too long", filename);
    }
    fp = SafeOpenWrite(filename);

    //
    // write everything
    //

    // overwritten later
    SafeWrite(&data->ident, sizeof(data->ident), fp);
    SafeWrite(&data->version, sizeof(data->version), fp);
    SafeWrite(&data->tileset, sizeof(data->tileset), fp);
    SafeWrite(&data->map, sizeof(data->map), fp);

    AddLump(mapData->mTiles.data(), sizeof(maptile_t) * mapData->mTiles.size(), &data->map, LUMP_TILES, fp);
    AddLump(mapData->mCheckpoints.data(), sizeof(mapcheckpoint_t) * mapData->mCheckpoints.size(), &data->map, LUMP_CHECKPOINTS, fp);
    AddLump(mapData->mSpawns.data(), sizeof(mapspawn_t) * mapData->mSpawns.size(), &data->map, LUMP_SPAWNS, fp);
    AddLump(mapData->mLights.data(), sizeof(maplight_t) * mapData->mLights.size(), &data->map, LUMP_LIGHTS, fp);
    AddLump(data->tileset.sprites, sizeof(tile2d_sprite_t) * data->tileset.info.numTiles, &data->map, LUMP_SPRITES, fp);

    fseek(fp, 0L, SEEK_SET);

    SafeWrite(&data->ident, sizeof(data->ident), fp);
    SafeWrite(&data->version, sizeof(data->version), fp);
    SafeWrite(&data->tileset, sizeof(data->tileset), fp);
    SafeWrite(&data->map, sizeof(data->map), fp);

    fclose(fp);
}

void CompileBMF(const char *output, const char *input)
{
    bmf_t bmf;
    char *compressed;
    FileStream file;
    const char *filename = input;

    Printf("Loading map file '%s'", filename);

    if (file.Open(filename, "r")) {
        if (!Map_LoadFile(&file, GetExtension(filename), filename)) {
            Printf("Failed to load map file '%s', not compiling", filename);
            return;
        }
    }
    else {
        Printf("Failed to load map file '%s', not compiling", filename);
        return;
    }
    bmf.tileset.sprites = GenerateSprites();

    bmf.ident = LEVEL_IDENT;
    bmf.version = LEVEL_VERSION;
    bmf.map.ident = MAP_IDENT;
    bmf.map.version = MAP_VERSION;
    bmf.tileset.magic = TILE2D_MAGIC;
    bmf.tileset.version = TILE2D_VERSION;

//    Printf("Compressing sprite buffer...");
    memcpy(&bmf.tileset.info, &tilesetInfo, sizeof(tilesetInfo));
//    compressed = Compress(bmf.tileset.sprites, sizeof(tile2d_sprite_t) * bmf.tileset.info.numTiles, &bmf.tileset.info.compressedSize);
//    FreeMemory(bmf.tileset.sprites);
//    bmf.tileset.sprites = (tile2d_sprite_t *)compressed;

    Printf("Writing file...");
    WriteBMF(output, &bmf);
    Printf("Finished write bmf file '%s'", output);
}

static void print_help(void)
{
    printf(
        "usage: %s [options...] -o <out>\n"
        "[options]\n"
        "\t--map <file>     provide a map file (ext = .map)\n"
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

    mapData = std::make_unique<CMapData>();
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
    }
    if (!output) {
        Error("output file not provided");
    }

    CompileBMF(output, map);

    return 0;
}
