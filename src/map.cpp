#include "gln.h"

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t numSpawns;
    uint32_t numCheckpoints;
    uint32_t numLights;
    uint32_t numEntities;
} mapinfo_t;

char mapname[1024];
CMapData mapData;
static char curmapfile[1024];

void Map_New(void)
{
    mapData.Clear();
    mapData.mPath = pwdString.string() + "/Data/untitled-map.map";
    mapData.mName = "untitled-map";
}

typedef enum {
    CHUNK_CHECKPOINT,
    CHUNK_SPAWN,
    CHUNK_LIGHT,
    
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
            tmpData->mSpawns.back().entitytype = static_cast<uint32_t>(atoi(tok));
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
            tmpData->mSpawns.back().entityid = static_cast<uint32_t>(atoi(tok));
            if (!editor->ValidateEntityId(tmpData->mSpawns.back().entityid)) {
                COM_ParseError("invalid entity id found in map spawn: %u", tmpData->mSpawns.back().entityid);
                return false;
            }
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
                xyz = tmpData->mCheckpoints.back().xyz;
            }

            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for pos.x");
                return false;
            }
            xyz[0] = static_cast<uint32_t>(atoi(tok));

            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for pos.y");
                return false;
            }
            xyz[1] = static_cast<uint32_t>(atoi(tok));
            
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for pos.elevation");
                return false;
            }
            xyz[2] = static_cast<uint32_t>(atoi(tok));
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

            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for color.r");
                return false;
            }
            tmpData->mLights.back().color[0] = static_cast<byte>(clamp(atoi(tok), 0, 255));

            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for color.g");
                return false;
            }
            tmpData->mLights.back().color[1] = static_cast<byte>(clamp(atoi(tok), 0, 255));

            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for color.b");
                return false;
            }
            tmpData->mLights.back().color[2] = static_cast<byte>(clamp(atoi(tok), 0, 255));

            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for color.a");
                return false;
            }
            tmpData->mLights.back().color[3] = static_cast<byte>(clamp(atoi(tok), 0, 255));
        }
        //
        // origin <x y elevation>
        //
        else if (!N_stricmp(tok, "origin")) {
            if (type != CHUNK_LIGHT) {
                COM_ParseError("found parameter \"origin\" in chunk that isn't a light");
                return false;
            }

            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for origin.x");
                return false;
            }
            tmpData->mLights.back().origin[0] = static_cast<float>(atof(tok));

            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for origin.y");
                return false;
            }
            tmpData->mLights.back().origin[1] = static_cast<float>(atof(tok));
            
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for origin.elevation");
                return false;
            }
            tmpData->mLights.back().origin[2] = static_cast<float>(atof(tok));
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
    mapinfo_t tmp;
    memset(&tmp, 0, sizeof(tmp));

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
            tmpData->mWidth = static_cast<uint32_t>(atoi(tok));
        }
        else if (!N_stricmp(tok, "height")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map height");
                return false;
            }
            tmpData->mHeight = static_cast<uint32_t>(atoi(tok));
        }
        else if (!N_stricmp(tok, "numCheckpoints")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map numCheckpoints");
                return false;
            }
            tmpData->mCheckpoints.reserve(static_cast<size_t>(atoi(tok)));
        }
        else if (!N_stricmp(tok, "numSpawns")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map numSpawns");
                return false;
            }
            tmpData->mSpawns.reserve(static_cast<size_t>(atoi(tok)));
        }
        else if (!N_stricmp(tok, "numLights")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map numLights");
                return false;
            }
            tmpData->mLights.reserve(static_cast<size_t>(atoi(tok)));
        }
        else if (!N_stricmp(tok, "numEntities")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                COM_ParseError("missing parameter for map numEntities");
                return false;
            }
            tmpData->mEntities.reserve(static_cast<size_t>(atoi(tok)));
        }
    }
    return true;
}

void Map_LoadFile(IDataStream *file, const char *ext, const char *rpath)
{
    uint64_t fileLen;
    char *buf, *ptr;
    const char **text;
    CMapData tmpData;

    fileLen = file->GetLength();

    mapData.Clear();
    buf = (char *)GetMemory(fileLen);
    file->Read(buf, fileLen);
    file->Close();

    ptr = buf;
    text = (const char **)&ptr;

    if (!ParseMap(text, rpath, &tmpData)) {
        Printf("Error: failed to load map");
    }
    else {
        mapData = tmpData;
        mapData.mPath = rpath;
    }
    FreeMemory(buf);
}

void Map_Load(const char *filename)
{
    FileStream file;

    if (file.Open(filename, "r")) {
        Map_LoadFile(&file, GetExtension(filename), filename);
    }
    else {
        Printf("Failed to load map file '%s'", filename);
    }
}

static void SaveSpawns(IDataStream *file)
{
    char buf[1024];

    for (const auto& it : mapData.mSpawns) {
        snprintf(buf, sizeof(buf),
            "{\n"
            "classname map_spawn\n"
            "pos %u %u %u\n"
            "entity %u\n"
            "id %u\n"
            "}\n"
        , it.xyz[0], it.xyz[1], it.xyz[2], it.entitytype, it.entityid);
        file->Write(buf, strlen(buf));
    }
}

static void SaveCheckpoints(IDataStream *file)
{
    char buf[1024];

    for (const auto& it : mapData.mCheckpoints) {
        snprintf(buf, sizeof(buf),
            "{\n"
            "classname map_checkpoint\n"
            "pos %u %u %u\n"
            "}\n"
        , it.xyz[0], it.xyz[1], it.xyz[2]);
        file->Write(buf, strlen(buf));
    }
}

static void SaveLights(IDataStream *file)
{
    char buf[1024];

    for (const auto& it : mapData.mLights) {
        snprintf(buf, sizeof(buf),
            "{\n"
            "classname map_light\n"
            "brightness %f\n"
            "origin %f %f %f\n"
            "color %hu %hu %hu %hu\n"
            "}\n"
        , it.brightness, it.origin[0], it.origin[1], it.origin[2], it.color[0], it.color[1], it.color[2], it.color[3]);
        file->Write(buf, strlen(buf));
    }
}

static void SaveTiles(IDataStream *file)
{
}

void Map_Save(const char *filename)
{
    char rpath[MAX_OSPATH*2+1];
    FileStream file;

    snprintf(rpath, sizeof(rpath), "%s%s", editor->mConfig->mEditorPath.c_str(), filename);
    if (!GetExtension(filename) || N_stricmp(GetExtension(filename), "map")) {
        snprintf(rpath, sizeof(rpath), "%s%s.map", editor->mConfig->mEditorPath.c_str(), filename);
    }

    if (!file.Open(rpath, "w")) {
        Error("Map_Save: failed to open file '%s' in write mode", rpath);
    }

    {
        char buf[1024];
        snprintf(buf, sizeof(buf),
            "{\n"
            "name \"%s\"\n"
            "width %i\n"
            "height %i\n"
            "numSpawns %lu\n"
            "numCheckpoints %lu\n"
            "numLights %lu\n"
            "numEntities %lu\n"
            "}\n"
        , mapData.mName.c_str(), mapData.mWidth, mapData.mHeight, mapData.mSpawns.size(), mapData.mCheckpoints.size(),
        mapData.mLights.size(), mapData.mEntities.size());
        file.Write(buf, strlen(buf));
    }

    SaveSpawns(&file);
    SaveCheckpoints(&file);
}

CMapData::CMapData(void)
{
    mWidth = 16;
    mHeight = 16;

    mTiles.resize(MAX_MAP_TILES);
    mCheckpoints.reserve(MAX_MAP_CHECKPOINTS);
    mSpawns.reserve(MAX_MAP_SPAWNS);
    mLights.reserve(MAX_MAP_LIGHTS);

    memset(mTiles.data(), 0, sizeof(*mTiles.data()) * MAX_MAP_TILES);
}

CMapData::~CMapData()
{
}

void CMapData::Clear(void)
{
    mWidth = 16;
    mHeight = 16;
    mCheckpoints.clear();
    mSpawns.clear();
    mVertices.clear();
    mIndices.clear();
    mLights.clear();
    mEntities.clear();
    mTiles.clear();
    mPath.clear();
    mName.clear();
}
