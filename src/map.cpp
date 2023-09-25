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
std::unique_ptr<CMapData> mapData;
static char curmapfile[1024];
static bool first = false;

void Map_New(void)
{
    mapData->Clear();
    mapData->mPath = pwdString.string() + "/Data/untitled-map.map";
    mapData->mName = "untitled-map";
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

    tmpData.Clear();
    buf = (char *)GetMemory(fileLen);
    file->Read(buf, fileLen);
    file->Close();

    ptr = buf;
    text = (const char **)&ptr;

    if (!ParseMap(text, rpath, &tmpData)) {
        Printf("Error: failed to load map");
    }
    else {
        *mapData = tmpData;
        mapData->mPath = rpath;
    }
    FreeMemory(buf);
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

static void SaveSpawns(IDataStream *file)
{
    char buf[1024];

    for (const auto& it : mapData->mSpawns) {
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

    for (const auto& it : mapData->mCheckpoints) {
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

    for (const auto& it : mapData->mLights) {
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

    snprintf(rpath, sizeof(rpath), "%s%s", gameConfig->mEditorPath.c_str(), filename);
    if (!GetExtension(filename) || N_stricmp(GetExtension(filename), "map")) {
        snprintf(rpath, sizeof(rpath), "%s%s.map", gameConfig->mEditorPath.c_str(), filename);
    }

    Printf("Saving map file '%s'", filename);

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
        , mapData->mName.c_str(), mapData->mWidth, mapData->mHeight, mapData->mSpawns.size(), mapData->mCheckpoints.size(),
        mapData->mLights.size(), mapData->mEntities.size());
        file.Write(buf, strlen(buf));
    }

    SaveSpawns(&file);
    SaveCheckpoints(&file);

    file.Write("}\n", 2);

    mapData->mModified = false;
}

CMapData::CMapData(void)
{
    mWidth = 16;
    mHeight = 16;
    mModified = true;

    mTiles.resize(MAX_MAP_TILES);
    mCheckpoints.reserve(MAX_MAP_CHECKPOINTS);
    mSpawns.reserve(MAX_MAP_SPAWNS);

    memset(mTiles.data(), 0, sizeof(*mTiles.data()) * MAX_MAP_TILES);
}

CMapData::~CMapData()
{
}


static void SetTileCheckpoints(void)
{
    const uint32_t width = mapData->mWidth;
    const uint32_t height = mapData->mHeight;
    const std::vector<mapcheckpoint_t>& checkpoints = mapData->mCheckpoints;
    std::vector<maptile_t>& tiles = mapData->mTiles;

    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            for (const auto& it : checkpoints) {
                if (it.xyz[0] == x && it.xyz[1] == y) {
                    boost::lock_guard<boost::shared_mutex> lock{mapData->resourceLock};
                    tiles[y * width + x].flags |= TILE_CHECKPOINT;
                }
            }
        }
    }
}

static void SetTileSpawns(void)
{
    const uint32_t width = mapData->mWidth;
    const uint32_t height = mapData->mHeight;
    const std::vector<mapspawn_t>& spawns = mapData->mSpawns;
    std::vector<maptile_t>& tiles = mapData->mTiles;

    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            for (const auto& it : spawns) {
                if (it.xyz[0] == x && it.xyz[1] == y) {
                    boost::lock_guard<boost::shared_mutex> lock{mapData->resourceLock};
                    tiles[y * width + x].flags |= TILE_SPAWN;
                }
            }
        }
    }
}

static const CMapData *tmpOther;

template<typename T>
static void CopyVector(std::vector<T>& dst, const std::vector<T>& src)
{
    dst.resize(src.size());
    memcpy(dst.data(), src.data(), sizeof(T) * src.size());
}

static void CopyTiles(void)
{ CopyVector(mapData->mTiles, tmpOther->mTiles); }
static void CopySpawns(void)
{ CopyVector(mapData->mSpawns, tmpOther->mSpawns); }
static void CopyLights(void)
{ CopyVector(mapData->mLights, tmpOther->mLights); }
static void CopyCheckpoints(void)
{ CopyVector(mapData->mCheckpoints, tmpOther->mCheckpoints); }
static void CopyEntities(void)
{ CopyVector(mapData->mEntities, tmpOther->mEntities); }
static void CopyVertices(void)
{ CopyVector(mapData->mVertices, tmpOther->mVertices); }
static void CopyIndices(void)
{ CopyVector(mapData->mIndices, tmpOther->mIndices); }

const CMapData& CMapData::operator=(const CMapData& other)
{
    tmpOther = eastl::addressof(other);
    Clear();
    
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mName = other.mName;

    {
        boost::thread_group group;

        group.create_thread(CopyTiles);
        group.create_thread(CopySpawns);
        group.create_thread(CopyLights);
        group.create_thread(CopyCheckpoints);
        group.create_thread(CopyEntities);
        group.create_thread(CopyVertices);
        group.create_thread(CopyIndices);

        group.join_all();
    }

    {
        boost::thread_group group;

        group.create_thread(SetTileCheckpoints);
        group.create_thread(SetTileSpawns);

        group.join_all();
    }

    mModified = true;
    
    return *this;
}

/*
map vertices and indices aren't saved to the text based .map format
*/
void CMapData::CalcDrawData(void)
{
    mVertices.resize(mWidth * mHeight * 4);
    mIndices.resize(mWidth * mHeight * 6);

    auto calculateIndices = [&](std::vector<uint32_t>& indices) {
        uint32_t i, offset;

        offset = 0;
        for (i = 0; i < indices.size(); i += 6) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;

            indices[i + 3] = offset + 3;
            indices[i + 4] = offset + 2;
            indices[i + 5] = offset + 0;

            offset += 4;
        }
    };

    auto calculateVertices = [&](std::vector<mapvert_t>& vertices, uint32_t width, uint32_t height) {
        auto convertCoords = [&](const glm::vec2& pos, mapvert_t *verts) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3( pos.x, pos.y, 0.0f ));
            glm::mat4 mvp = gui->mViewProjection * model;

            constexpr glm::vec4 positions[4] = {
                { 0.5f,  0.5f, 0.0f, 1.0f},
                { 0.5f, -0.5f, 0.0f, 1.0f},
                {-0.5f, -0.5f, 0.0f, 1.0f},
                {-0.5f,  0.5f, 0.0f, 1.0f},
            };

            for (uint32_t i = 0; i < arraylen(positions); i++) {
                const glm::vec3 p = mvp * positions[i];
            }
        };
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                convertCoords({ x, y }, &vertices[y * width + x]);
            }
        }
    };

    boost::thread indices(calculateIndices, mIndices);
    boost::thread vertices(calculateVertices, mVertices, mWidth, mHeight);

    indices.join();
    vertices.join();
}

void CMapData::SetMapSize(uint32_t width, uint32_t height)
{
    mWidth = width;
    mHeight = height;
    mTiles.resize(mWidth * mHeight);
    mModified = true;
    CalcDrawData();
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
    mModified = true;

    mCheckpoints.reserve(MAX_MAP_CHECKPOINTS);
    mSpawns.reserve(MAX_MAP_SPAWNS);

    for (auto& it : mTiles) {
        it.index = -1;
    }

    CalcDrawData();
    
    // always at least one spawn for the player
    const mapspawn_t s = {
        .xyz{ 0, 0, 0 },
        .entitytype = ET_PLAYR,
        .entityid = 0
    };
    mSpawns.emplace_back(s);
}

/*
CheckAutoSave: if mAutoSaveTime (in minutes) have passed since last edit, and the map hasn't been saved yet, save it
*/
static time_t s_start = 0;
void CheckAutoSave(void)
{
    time_t now;
    time(&now);

    if (mapData->mModified || !s_start) {
        s_start = now;
        return;
    }

    if ((now - s_start) > (60 * gameConfig->mAutoSaveTime)) {
        if (gameConfig->mAutoSave) {
            const char *strMsg;

            strMsg = "Autosaving map...";
            Printf("%s", strMsg);
            Map_Save(mapData->mName.c_str());
        }
        else {
            Printf("Autosave skipped...");
        }
        s_start = now;
    }
}
