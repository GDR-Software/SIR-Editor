#ifndef __TILESET__
#define __TILESET__

#pragma once

class CTileset
{
public:
    std::shared_ptr<CTexture> texData; // diffuseMap, technically
    std::shared_ptr<CTexture> normalData;
    std::vector<maptile_t> tiles;
    uint32_t tileCountX;
    uint32_t tileCountY;
    uint32_t tileWidth;
    uint32_t tileHeight;

    CTileset(void)
        : tileCountX{ 0 }, tileCountY{ 0 }, tileWidth{ 0 }, tileHeight{ 0 }
    { }
    ~CTileset() { }

    void GenerateTiles(void);
};

#endif