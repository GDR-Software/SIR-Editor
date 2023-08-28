#ifndef _TILESET_H_
#define _TILESET_H_

#pragma once

#include <glm/glm.hpp>
#include "Texture.h"

struct Tile
{
    glm::vec2 texcoords[4];
    glm::vec3 pos;
    uint32_t index;
    bool empty;

    inline Tile(void) = default;
    inline Tile(Tile &) = default;
};

class Tileset
{
public:
    Tileset();
    ~Tileset();

    void Shutdown(void);

    const Tile *getTile(uint32_t y, uint32_t x) const;
    Tile *getTile(uint32_t y, uint32_t x);

    void LoadTexture(const eastl::string& path);
    void GenTiles(void);
    void Clear(void);

    void Load(const eastl::string& filename, int _tileWidth, int _tileHeight);
public:
    eastl::vector<Tile> tiles;
    eastl::unique_ptr<Texture> texture;

    int numTiles;
    int width;
    int height;

    int tileWidth;
    int tileHeight;

    bool inited;
};

inline const Tile* Tileset::getTile(uint32_t y, uint32_t x) const
{
    return &tiles[y * width + x];
}

inline Tile* Tileset::getTile(uint32_t y, uint32_t x)
{
    return &tiles[y * width + x];
}

#endif