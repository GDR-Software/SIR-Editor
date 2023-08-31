#include "Common.hpp"
#include "Editor.h"
#include "Map.h"

bool LoadMap(Map *cMap, const char *filename);
bool SaveMap(const Map *cMap, const char *filename);

CMap::CMap(void)
{
    name = "untitled-map.map";
}

CMap::~CMap()
{
    delete[] tiles;
}

/*
CMap::AddCheckpoint: returns false if any of the parms are invalid
*/
bool CMap::AddCheckpoint(const glm::vec2& pos, const glm::vec2& size)
{
    if (pos.x >= width || pos.x < 0 || pos.y >= height || pos.y < 0) {
        return false;
    }

    checkpoint_t c;
    c[0] = pos[0];
    c[1] = pos[1];
    c[2] = size[0];
    c[3] = size[1];

    checkpoints.emplace_back(c);

    return true;
}

/*
CMap::AddSpawn: returns false if any of the parms are invalid
*/
bool CMap::AddSpawn(const glm::vec2& pos, uint32_t entity)
{
    if (pos.x >= width || pos.x < 0 || pos.y >= height || pos.y < 0) {
        return false;
    }

    spawn_t s;
    s[0] = pos[0];
    s[1] = pos[1];
    s[2] = entity;

    spawns.emplace_back(s);

    return true;
}

/*
CMap::Clear: clears all map data, done every time we're loading a new map
*/
void CMap::Clear(void)
{
    delete cTileset;
    spawns.clear();
    checkpoints.clear();
    delete[] tiles;
}

bool CMap::Load(const eastl::string& path)
{
    json data;

    if (!FileExists(path.c_str())) {
        Printf("Map::Load: file '%s' does not exist", path.c_str());
        return false;
    }

    Printf("Loading map file '%s'", path.c_str());

    if (!LoadMap(this, path.c_str())) {
        Printf("WARNING: failed to load map file '%s'", path.c_str());
        return false;
    }
    return true;
}

bool CMap::Load(const json& data)
{
}

bool CMap::Save(json& data) const
{
}

bool CMap::Save(const eastl::string& path) const
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
