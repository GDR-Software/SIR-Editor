#include "Common.hpp"
#include "Editor.h"
#include "Map.h"

bool LoadMap(CMap *cMap, const char *filename);
bool SaveMap(const CMap *cMap, const char *filename);

CMap::CMap(void)
{
    name = "untitled-map.map";
    width = 0;
    height = 0;
}

CMap::~CMap()
{
}

/*
CMap::AddCheckpoint: returns false if any of the parms are invalid
*/
bool CMap::AddCheckpoint(const checkpoint_t& c)
{
    if (c[0] >= width || c[0] < 0 || c[1] >= height || c[1] < 0) {
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

    spawn_t s;
    s[0] = pos[0];
    s[1] = pos[1];
    s[2] = entityType;
    s[3] = entityId;

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
    path = BuildOSPath(Editor::GetPWD(), "Data/", "untitled-map.jmap");
    name = "untitled-map";
    cTileset = NULL;
}

bool CMap::Load(const string_t& path)
{
    json data;
    char *rpath;
    const char *ext;

    rpath = strdupa(path.c_str());
    ext = COM_GetExtension(path.c_str());
    if (IsAbsolutePath(path)) {
        rpath = BuildOSPath(Editor::GetPWD(), "Data/", path.c_str());
    }
    if (!ext || !N_stricmp(ext, ".jmap")) {
        return false;
    }

    if (!FileExists(rpath)) {
        Printf("Map::Load: file '%s' does not exist", rpath);
        return false;
    }

    Printf("Loading map file '%s'", rpath);

    if (!LoadMap(this, rpath)) {
        Printf("WARNING: failed to load map file '%s'", rpath);
        return false;
    }
    name = path;
    this->path = rpath;
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
        memcpy(checkpoints[count].data(), tmp.data(), sizeof(checkpoint_t));
        ++count;
    }
    
    count = 0;
    for (const auto& it : map.at("spawns")) {
        sprintf(s, "#%i", count);
        const eastl::array<uint32_t, 4>& tmp = it.at(s).get<eastl::array<uint32_t, 4>>();
        memcpy(spawns[count].data(), tmp.data(), sizeof(spawn_t));
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
        save[s] = { it.c[0], it.c[1] };
        ++count;
    }
    map["checkpoints"] = save;
    save.clear();

    count = 0;
    for (const auto& it : spawns) {
        sprintf(s, "#%i", count);
        save[s] = { it.s[0], it.s[1], it.s[2], it.s[3] };
        ++count;
    }
    map["spawns"] = save;

    return true;
}

bool CMap::SaveBIN(const string_t& path) const
{
    if (!modified) {
        return true; // no need to save if its already been saved
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
    char *rpath;
    path_t tmp;
    json data;

    if (!modified) {
        return true; // no need to save if its already been saved
    }

    rpath = strdupa(path.c_str());
    if (IsAbsolutePath(path)) {
        rpath = BuildOSPath(Editor::GetPWD(), "Data/", path.c_str());
    }
    tmp = rpath;
    Editor::CheckExtension(tmp, ".jmap");

    Printf("Saving map file '%s'", tmp.c_str());

    Save(data);

    if (!Editor::SaveJSON(data, tmp)) {
        Printf("WARNING: failed to save map file '%s'", tmp.c_str());
        return false;
    }

    modified = false;
    return true;
}
