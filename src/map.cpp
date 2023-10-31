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

void Map_New(void)
{
    mapData->Clear();
    mapData->mPath = pwdString.string() + "/Data/untitled-map.map";
    mapData->mName = "untitled-map";
#ifndef BMFC
    SDL_SetWindowTitle(gui->mWindow, mapData->mName.c_str());
#endif
}

#ifndef BMFC
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
    std::shared_ptr<CTileset>& tileset = project->tileset;

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
            tileset->tileCountX = (uint32_t)atoi(tok);
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
            tileset->tileCountY = (uint32_t)atoi(tok);
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
            tileset->tiles.reserve(static_cast<uint32_t>(atoi(tok)));
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
            tileset->tileWidth = (uint32_t)atoi(tok);
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
            tileset->texData->mName = tok;
            tileset->texData->Load(tok);
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
            tileset->tileHeight = (uint32_t)atoi(tok);
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
            if (!editor->ValidateEntityId(tmpData->mSpawns.back().entityid)) {
                COM_ParseError("invalid entity id found in map spawn: %u", tmpData->mSpawns.back().entityid);
                return false;
            }
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
        N_strncpyz(mapname, GetFilename(rpath), sizeof(mapname));
        project->tileset->GenerateTiles();
        *mapData = tmpData;
        mapData->mPath = rpath;
        SDL_SetWindowTitle(gui->mWindow, mapData->mName.c_str());
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

    Printf("Saving %lu lights...", mapData->mLights.size());
    for (const auto& it : mapData->mLights) {
        snprintf(buf, sizeof(buf),
            "{\n"
            "classname map_light\n"
            "brightness %f\n"
            "range %f\n"
            "origin ( %u %u %u )\n"
            "color ( %f %f %f %f )\n"
            "}\n"
        , it.brightness, it.range, it.origin[0], it.origin[1], it.origin[2], it.color[0], it.color[1], it.color[2], it.color[3]);
        file->Write(buf, strlen(buf));
    }
}

static void SaveTiles(IDataStream *file)
{
    char buf[1024];

    Printf("Saving %lu tiles...", mapData->mTiles.size());
    for (uint64_t i = 0; i < mapData->mTiles.size(); i++) {
        const auto& it = mapData->mTiles[i];
        snprintf(buf, sizeof(buf),
            "{\n"
            "classname map_tile\n"
            "texIndex %i\n"
            "flags %x\n"
            "sides ( %i %i %i %i %i )\n"
            "texcoords ( ( %f %f ) ( %f %f ) ( %f %f ) ( %f %f ) )\n"
            "}\n"
        , it.index,
        it.flags,
        (int)it.sides[0], (int)it.sides[1], (int)it.sides[2], (int)it.sides[3], (int)it.sides[4],
        it.texcoords[0][0], it.texcoords[0][1],
        it.texcoords[1][0], it.texcoords[1][1],
        it.texcoords[2][0], it.texcoords[2][1],
        it.texcoords[3][0], it.texcoords[3][1]);

        file->Write(buf, strlen(buf));
    }
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
        const std::shared_ptr<CTileset>& tileset = project->tileset;
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
            "numTiles %lu\n"
            "ambientType %s\n"
            "ambientIntensity %f\n"
            "ambientColor ( %f %f %f )\n"
            "{\n"
            // save the tileset as well
            "classname map_tileset\n"
            "texture %s\n"
            "tileWidth %u\n"
            "tileHeight %u\n"
            "tileCountX %u\n"
            "tileCountY %u\n"
            "numTiles %lu\n"
            "}\n"
        , mapData->mName.c_str(), mapData->mWidth, mapData->mHeight, mapData->mSpawns.size(), mapData->mCheckpoints.size(),
        mapData->mLights.size(), mapData->mEntities.size(), mapData->mTiles.size(), mapData->mDarkAmbience ? "dark" : "light", mapData->mAmbientIntensity,
        mapData->mAmbientColor[0], mapData->mAmbientColor[1], mapData->mAmbientColor[2],
        tileset->texData->mName.c_str(), tileset->tileWidth, tileset->tileHeight, tileset->tileCountX, tileset->tileCountY, tileset->tiles.size());
        file.Write(buf, strlen(buf));
    }

    SaveSpawns(&file);
    SaveCheckpoints(&file);
    SaveLights(&file);
    SaveTiles(&file);

    file.Write("}\n", 2);

    mapData->mModified = false;
}

#endif

CMapData::CMapData(void)
{
    mWidth = 16;
    mHeight = 16;
    mModified = true;
    mAmbientColor = { 1.0f, 1.0f, 1.0f };
    mAmbientIntensity = 0.0f;

    mTiles.resize(MAX_MAP_TILES);
    mCheckpoints.reserve(MAX_MAP_CHECKPOINTS);
    mSpawns.reserve(MAX_MAP_SPAWNS);

    memset(mTiles.data(), 0, sizeof(*mTiles.data()) * MAX_MAP_TILES);
}

CMapData::~CMapData()
{
}

#ifndef BMFC
static void ConvertCoords(Vertex *vertices, const glm::vec2& pos)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
    glm::mat4 mvp = gui->mViewProjection * model;

    constexpr glm::vec4 positions[4] = {
        { 0.5f,  0.5f, 0.0f, 1.0f},
        { 0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f,  0.5f, 0.0f, 1.0f},
    };

    for (uint32_t i = 0; i < arraylen(positions); i++) {
        vertices[i].xyz = mvp * positions[i];
    }
}

static void LightingAtVertex(vec3_t origin, vec3_t normal, vec4_t color, vec3_t ambient, float ambientIntensity)
{
    vec3_t ambience, dir;
    std::vector<maplight_t>::iterator light;
    float dist;
    float add;
    float angle;
    float diffuse;

    VectorCopy(ambience, ambient);
    VectorScale(ambience, ambientIntensity, ambience);

    // apply all lights
    for (light = mapData->mLights.begin(); light != mapData->mLights.end(); light++) {
        #if 0
        if (DotProduct(light->origin, normal) - DotProduct(normal, origin) < 0) {
            continue;
        }
        #endif
        
        // calculate the amount of lighting at this vertex
 //       if (light->type == light_point) {
            VectorSubtract(light->origin, origin, dir);
            dist = VectorNormalize(dir, dir);
            
            // clamp the distance to prevent super hot spots
            if (dist < 4) {
                dist = 4;
            }
            angle = DotProduct(normal, dir);
            add = light->brightness / ( dist * dist ) * angle;
 //       }
 //       else if (light->type == light_spotlight) {
  //      }
        
        if (add <= 1.0) {
            continue;
        }
        diffuse = 0.0;
        if (dist <= light->brightness) {
            diffuse = 1.0 - fabs(dist / light->brightness);
        }

        // add the result
        color[0] += add * light->color[0] * diffuse;
        color[1] += add * light->color[1] * diffuse;
        color[2] += add * light->color[2] * diffuse;
    }
}

/*
CalcVertexNormals: calculates normals for a single quad
*/
void CalcVertexNormals(Vertex *quad)
{
    vec3_t vtx1, vtx2;
    vec3_t normal;

    VectorSubtract(quad[1].xyz, quad[0].xyz, vtx1);
    VectorSubtract(quad[2].xyz, quad[0].xyz, vtx2);
    CrossProduct(vtx1, vtx2, normal);
    VectorNormalize(normal, normal);

    VectorCopy(quad[0].normal, normal);
    VectorCopy(quad[1].normal, normal);
    VectorCopy(quad[2].normal, normal);
    VectorCopy(quad[3].normal, normal);
}

void CMapData::CalcLighting(Vertex *vertices, uint32_t numVertices)
{
    Vertex *vt;

    for (uint32_t i = 0; i < numVertices; i++) {
        vt = &vertices[i];
        LightingAtVertex(&vt->xyz[0], &vt->normal[0], &vt->color[0], &mapData->mAmbientColor[0], mapData->mAmbientIntensity);
    }
}
#endif

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
    tmpOther = std::addressof(other);
    Clear();
    
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mName = other.mName;
    mAmbientIntensity = other.mAmbientIntensity;
    mAmbientColor = other.mAmbientColor;
    mDarkAmbience = other.mDarkAmbience;

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
#ifndef BMFC
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
        uint32_t index = 0;
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
                convertCoords({ x, y }, &vertices[index]);
                index += 4;
            }
        }
    };

    calculateIndices(mIndices);
    calculateVertices(mVertices, mWidth, mHeight);
}
#endif

void CMapData::SetMapSize(uint32_t width, uint32_t height)
{
    mWidth = width;
    mHeight = height;
    mTiles.resize(mWidth * mHeight);
    mModified = true;
}

void CMapData::Clear(void)
{
    mDarkAmbience = true;
    mWidth = 16;
    mHeight = 16;
    mAmbientIntensity = 0.0f;
    mAmbientColor = { 1.0f, 1.0f, 1.0f };
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
    
    // always at least one spawn for the player
    const mapspawn_t s = {
        .xyz{ 0, 0, 0 },
        .entitytype = ET_PLAYR,
        .entityid = 0
    };
    mSpawns.emplace_back(s);
}

#ifndef BMFC
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
#endif
