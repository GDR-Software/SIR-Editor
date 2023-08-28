#ifndef _MAP_H_
#define _MAP_H_

#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MAX_MAP_WIDTH 256
#define MAX_MAP_HEIGHT 256

typedef struct
{
    uint32_t c[4];

    inline uint32_t& operator[](int index)
    { return c[index]; }
    inline uint32_t *data(void)
    { return c; }
    inline const uint32_t *data(void) const
    { return c; }
} checkpoint_t;

typedef struct
{
    uint32_t s[3];

    inline uint32_t& operator[](int index)
    { return s[index]; }
    inline uint32_t *data(void)
    { return s; }
    inline const uint32_t *data(void) const
    { return s; }
} spawn_t;

#include "Tileset.h"

class Map
{
public:
    Map();
    ~Map();

    void Load(const eastl::string& filename);
    void Save(const eastl::string& filename);

    void Clear(void);

    bool AddCheckpoint(const glm::vec2& pos, const glm::vec2& size);
    bool AddSpawn(const glm::vec2& pos, uint32_t entity);
public:
    eastl::string path;
    eastl::string name;

    eastl::shared_ptr<Tileset> tileset;
    eastl::array<Tile, MAX_MAP_WIDTH * MAX_MAP_HEIGHT> tiles;
    
    eastl::vector<checkpoint_t> checkpoints;
    eastl::vector<spawn_t> spawns;

    int width;
    int height;
};

#endif