#include "Common.hpp"
#include "Editor.h"
#include "Map.h"

bool LoadMap(Map *cMap, const char *filename);
bool SaveMap(const Map *cMap, const char *filename);

Map::Map()
{
    memset(tiles.data(), 0, sizeof(Tile) * tiles.size());
    for (uint32_t y = 0; y < MAX_MAP_HEIGHT; y++) {
        for (uint32_t x = 0; x < MAX_MAP_WIDTH; x++) {
            tiles[y * MAX_MAP_WIDTH + x].empty = true;
            tiles[y * MAX_MAP_WIDTH + x].index = 0;
        }
    }

    width = MAX_MAP_WIDTH;
    height = MAX_MAP_HEIGHT;

    name = "untitled-map.map";
}

Map::~Map()
{
}

/*
Map::AddCheckpoint: returns false if any of the parms are invalid
*/
bool Map::AddCheckpoint(const glm::vec2& pos, const glm::vec2& size)
{
    if (pos.x >= MAX_MAP_WIDTH || pos.x < 0 || pos.y >= MAX_MAP_HEIGHT) {
        return false;
    }

    Editor::Get()->getProject()->modified = true;

    checkpoint_t c;
    c[0] = pos[0];
    c[1] = pos[1];
    c[2] = size[0];
    c[3] = size[1];

    checkpoints.emplace_back(c);

    return true;
}

/*
Map::AddSpawn: returns false if any of the parms are invalid
*/
bool Map::AddSpawn(const glm::vec2& pos, uint32_t entity)
{
    if (pos.x >= MAX_MAP_WIDTH || pos.x < 0 || pos.y >= MAX_MAP_HEIGHT) {
        return false;
    }

    Editor::Get()->getProject()->modified = true;

    spawn_t s;
    s[0] = pos[0];
    s[1] = pos[1];
    s[2] = entity;

    spawns.emplace_back(s);

    return true;
}

/*
Map::Clear: clears all map data, done every time we're loading a new map
*/
void Map::Clear(void)
{
    tileset.reset();

    spawns.clear();
    checkpoints.clear();
    memset(tiles.data(), 0, sizeof(Tile) * tiles.size());
    tileset->Clear();
}

void Map::Load(const eastl::string& filename)
{
    json data;

    if (!FileExists(filename.c_str())) {
        Printf("Map::Load: file '%s' does not exist", filename.c_str());
        return;
    }
    
    Editor::Get()->getProject()->modified = true;

    Printf("Loading map file '%s'", filename.c_str());

    if (!LoadMap(this, filename.c_str())) {
        Printf("WARNING: failed to load map file '%s'", filename.c_str());
        return;
    }
}

void Map::Save(const eastl::string& filename)
{
    FILE *fp;

    if (!Editor::Get()->getProject()->modified) {
        return; // no need to save if its already been saved
    }

    Printf("Saving map file '%s'", filename.c_str());

    if (!SaveMap(this, filename.c_str())) {
        Printf("WARNING: failed to save map file '%s'", filename.c_str());
        return;
    }
}
