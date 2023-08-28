#include "Common.hpp"
#include "Map.h"
#include "Tileset.h"

#define MAX_TILES_X 256
#define MAX_TILES_Y 256

Tileset::Tileset()
    : width{0}, height{0}, tileWidth{0}, tileHeight{0}
{
    inited = false;
    texture = eastl::make_unique<Texture>();
}

Tileset::~Tileset()
{
}

void Tileset::Clear(void)
{
    texture->Shutdown();
}

void Tileset::LoadTexture(const eastl::string& path)
{
    Clear();
    texture->Load(path.c_str());
}

void Tileset::GenTiles(void)
{
    const uint32_t numTilesX = texture->width / tileWidth;
    const uint32_t numTilesY = texture->height / tileHeight;
    Tile *tile;

    Printf("Generating tileset...");

    auto genCoords = [&](const glm::vec2& sheetDims, const glm::vec2& spriteDims, const glm::vec2& pos, glm::vec2 texcoords[4]) {
        const glm::vec2 min = { (pos.x * spriteDims.x) / sheetDims.x, (pos.y * spriteDims.y) / sheetDims.x };
        const glm::vec2 max = { ((pos.x + 1) * spriteDims.x) / sheetDims.y, ((pos.y + 1) * spriteDims.y) / sheetDims.y };

        texcoords[0] = { min.x, min.y };
        texcoords[1] = { max.x, min.y };
        texcoords[2] = { max.x, max.y };
        texcoords[3] = { min.x, max.y };
    };

    tiles.clear();
    tiles.resize(numTilesX * numTilesY);
    numTiles = numTilesX * numTilesY;

    for (uint32_t y = 0; y < numTilesY; y++) {
        for(uint32_t x = 0; x < numTilesX; x++) {
            tile = &tiles[y * numTilesX + x];
            tile->index = y * numTilesX + x;
            tile->pos = { x, y, 0.0f };

            genCoords({ texture->width, texture->height }, { tileWidth, tileHeight }, { x, y }, tile->texcoords);
        }
    }
    inited = true;
}

void Tileset::Load(const eastl::string& filename, int _tileWidth, int _tileHeight)
{
    tiles.clear();
    tileWidth = _tileWidth;
    tileHeight = _tileHeight;
    GenTiles();
    texture->Load(filename);
}