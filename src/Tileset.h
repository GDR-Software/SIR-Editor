#ifndef _TILESET_H_
#define _TILESET_H_

#pragma once

#include <glm/glm.hpp>

class CTileset : public CEditorTool
{
public:
    CTileset(void);
    virtual ~CTileset();

    const maptile_t *getTile(uint32_t y, uint32_t x) const;
    maptile_t *getTile(uint32_t y, uint32_t x);

    void GenTiles(void);
    void SetTileDims(const int dims[2]);
    void SetSheetDims(const int dims[2]);
    
    virtual bool Load(const string_t& path);
    virtual bool Load(const json& data);
    virtual bool Save(const string_t& path) const;
    virtual bool Save(json& data) const;
    virtual void Clear(void);

    inline void SetTexture(object_ptr_t<CTexture>& tex)
    { cTexture = tex; }

    bool Write(FILE *fp) const;
    bool Read(FILE *fp);
private:
    bool LoadBIN(const string_t& path);
    bool LoadJSON(const string_t& path);
    bool SaveBIN(const string_t& path) const;
    bool SaveJSON(const string_t& path) const;

    vector_t<maptile_t> tiles;
    vector_t<tile2d_sprite_t> sprites;
    object_ptr_t<CTexture> cTexture;

    uint32_t numTiles;
    uint32_t width;
    uint32_t height;

    uint32_t tileWidth;
    uint32_t tileHeight;
};

#endif