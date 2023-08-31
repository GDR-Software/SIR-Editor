#ifndef _TILESET_H_
#define _TILESET_H_

#pragma once

#include <glm/glm.hpp>
#include "EditorTool.h"
#include "Texture.h"

struct Tile;

class CTileset : public CEditorTool
{
public:
    CTileset();
    virtual ~CTileset();

    const Tile *getTile(uint32_t y, uint32_t x) const;
    Tile *getTile(uint32_t y, uint32_t x);

    void LoadTexture(const eastl::string& path);
    void GenTiles(void);

    virtual bool Load(const std::string& path) override;
    virtual bool Load(const json& data) override;
    virtual bool Save(const std::string& path) const override;
    virtual bool Save(json& data) const override;
    virtual void Clear(void) override;
private:
    Tile *tiles
    eastl::unique_ptr<Texture> texture;

    int numTiles;
    int width;
    int height;

    int tileWidth;
    int tileHeight;
};

inline const Tile* CTileset::getTile(uint32_t y, uint32_t x) const
{
    return &tiles[y * width + x];
}

inline Tile* CTileset::getTile(uint32_t y, uint32_t x)
{
    return &tiles[y * width + x];
}

#endif