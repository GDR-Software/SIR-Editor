#include "Common.hpp"
#include "Editor.h"
#include "Map.h"

bool LoadMap(CMap *cMap, const char *filename);
bool SaveMap(const CMap *cMap, const char *filename);

CMap::CMap(void)
{
    name = "untitled-map";
    width = 0;
    height = 0;
    cTileset = NULL;
}

CMap::~CMap()
{
}

/*
CMap::AddCheckpoint: returns false if any of the parms are invalid
*/
bool CMap::AddCheckpoint(const mapcheckpoint_t& c)
{
    if (c.x >= width || c.x < 0 || c.y >= height || c.x < 0) {
        return false;
    }

    checkpoints.emplace_back(c);

    return true;
}

/*
CMap::AddSpawn: returns false if any of the parms are invalid
*/
bool CMap::AddSpawn(const glm::vec2& pos, uint32_t entityType, uint32_t entityId)
{
    if (pos.x >= width || pos.x < 0 || pos.y >= height || pos.y < 0) {
        return false;
    }

    mapspawn_t s;
    s.pos[0] = pos[0];
    s.pos[1] = pos[1];
    s.entitytype = entityType;
    s.entityid = entityId;

    spawns.emplace_back(s);

    return true;
}

void CMap::Resize(int dims[2])
{
    // don't resize if nothing's changed
    if (dims[0] == width && dims[1] == height) {
        return;
    }

    tiles.resize(dims[0] * dims[1]);
    width = dims[0];
    height = dims[1];
}

/*
CMap::Clear: clears all map data, done every time we're loading a new map
*/
void CMap::Clear(void)
{
    spawns.clear();
    checkpoints.clear();
    tiles.clear();
    name = "untitled-map";
    cTileset = NULL;
}

bool CMap::Load(const string_t& path)
{
    const char *ext;

    ext = COM_GetExtension(path.c_str());
    if (!ext || N_stricmp(ext, MAP_FILE_EXT_RAW) != 0 && N_stricmp(ext, ".jmap") != 0) {
        return false;
    }

    if (!FileExists(path.c_str())) {
        Printf("Map::Load: file '%s' does not exist", path.c_str());
        return false;
    }

    Printf("Loading map file '%s'", path.c_str());

    if (N_stristr(path.c_str(), ".jmap")) {
        json data;
        if (!Editor::LoadJSON(data, path.c_str())) {
            Printf("WARNING: failed to load .jmap file '%s'", path.c_str());
            return false;
        }
        return Load(data);
    }
    
    // its a binary
    if (!LoadMap(this, path.c_str())) {
        Printf("WARNING: failed to load map file '%s'", path.c_str());
        return false;
    }
    name = GetFilename(path.c_str());

    return true;
}

bool CMap::Load(const json& data)
{
    const json& map = data;
    char s[64];
    uint32_t count;

    name = map.at("name");
    width = map.at("width");
    height = map.at("height");

    checkpoints.resize(map.at("numCheckpoints"));
    spawns.resize(map.at("numSpawns"));

    count = 0;
    for (const auto& it : map.at("checkpoints")) {
        sprintf(s, "#%i", count);
        const eastl::array<uint32_t, 2>& tmp = it.at(s).get<eastl::array<uint32_t, 2>>();
        memcpy(&checkpoints[count], tmp.data(), sizeof(mapcheckpoint_t));
        ++count;
    }
    
    count = 0;
    for (const auto& it : map.at("spawns")) {
        sprintf(s, "#%i", count);
        const eastl::array<uint32_t, 4>& tmp = it.at(s).get<eastl::array<uint32_t, 4>>();
        memcpy(&spawns[count], tmp.data(), sizeof(mapspawn_t));
        ++count;
    }
    
    return true;
}

bool CMap::Save(json& data) const
{
    json map, save;
    char s[64];
    uint32_t count;

    map["name"] = name;
    map["width"] = width;
    map["height"] = height;
    map["numCheckpoints"] = checkpoints.size();
    map["numSpawns"] = spawns.size();

    count = 0;
    for (const auto& it : checkpoints) {
        sprintf(s, "#%i", count);
        save[s] = { it.x, it.y };
        ++count;
    }
    map["checkpoints"] = save;
    save.clear();

    count = 0;
    for (const auto& it : spawns) {
        sprintf(s, "#%i", count);
        save[s] = { it.pos[0], it.pos[1], it.entitytype, it.entityid };
        ++count;
    }
    map["spawns"] = save;

    return true;
}

bool CMap::SaveBIN(const string_t& path) const
{
    const char *ext;

    if (!modified) {
        return true; // no need to save if its already been saved
    }

    ext = COM_GetExtension(path.c_str());
    if (!ext) {
        ext = ".bmf";
    }
    else if (N_stricmp(ext, ".bmf") != 0) {
        Printf("Not a binary file");
        return false;
    }

    Printf("Saving map file '%s'", path.c_str());

    if (!SaveMap(this, path.c_str())) {
        Printf("WARNING: failed to save map file '%s'", path.c_str());
        return false;
    }
    return true;
}

bool CMap::Save(const string_t& path) const
{
    json data;
    const char *ext;
    char temp[MAX_OSPATH*2+1];
    N_strncpyz(temp, path.c_str(), sizeof(temp));

    if (!modified) {
        return true; // no need to save if its already been saved
    }

    const bool saveJSON = parm_saveJsonMaps;
    if (!N_stristr(path.c_str(), ".jmap") && saveJSON) {
        N_strcat(temp, sizeof(temp), ".jmap");
    }
    else if (!N_stristr(path.c_str(), ".bmf") && !saveJSON) {
        N_strcat(temp, sizeof(temp), ".bmf");
    }

    Printf("Saving map file '%s'", temp);

    if (!SaveMap(this, temp)) {
        Printf("WARNING: failed to save map file '%s'", temp);
        return false;
    }

    modified = false;
    return true;
}
