#include "Common.hpp"
#include "Editor.h"
#include "Map.h"
#include "Tileset.h"

#define MAX_TILES_X 256
#define MAX_TILES_Y 256

CTileset::CTileset()
{
    name = "untitled-tileset";
    width = 0;
    height = 0;
    tileWidth = 0;
    tileHeight = 0;
    cTexture = Allocate<CTexture>();
}

CTileset::~CTileset()
{
    Clear();
}

void CTileset::Clear(void)
{
    tiles.clear();
    Deallocate(cTexture);
    name = "untitled-tileset.tile";
}

void CTileset::GenTiles(void)
{
    const uint32_t numTilesX = cTexture->GetWidth() / tileWidth;
    const uint32_t numTilesY = cTexture->GetHeight() / tileHeight;
    maptile_t *tile;

    Printf("Generating tileset...");

    auto genCoords = [&](const glm::vec2& sheetDims, const glm::vec2& spriteDims, const glm::vec2& pos, float texcoords[4][2]) {
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
    modified = true;

    for (uint32_t y = 0; y < numTilesY; y++) {
        for(uint32_t x = 0; x < numTilesX; x++) {
            tile = &tiles[y * numTilesX + x];
            tile->index = y * numTilesX + x;
            tile->pos = { x, y, 0.0f };

            genCoords({ cTexture->GetWidth(), cTexture->GetHeight() }, { tileWidth, tileHeight }, { x, y }, tile->texcoords);
        }
    }
}

void CTileset::SetTileDims(const int dims[2])
{
    tileWidth = dims[0];
    tileHeight = dims[1];

    modified = true;
}

void CTileset::SetSheetDims(const int dims[2])
{
    width = dims[0];
    height = dims[1];

    modified = true;
}

bool CTileset::Save(const string_t& path) const
{
    const char *ext;

    ext = COM_GetExtension(path.c_str());
    if (!ext || N_stricmp(ext, ".jtile") != 0 && N_stricmp(ext, ".t2d") != 0) {
        return false;
    }

    if (!N_stricmp(ext, ".jtile")) {
        return SaveJSON(path);
    }
    else if (!N_stricmp(ext, ".t2d")) {
        return SaveBIN(path);
    }

    // never reached
    return true;
}

/*
CTileset::Save(json& data): only ever called when a project is saved
*/
bool CTileset::Save(json& data) const
{
    json& tileset = data;

    tileset["name"] = name;
    tileset["tilewidth"] = tileWidth;
    tileset["tileheight"] = tileHeight;
    tileset["numTiles"] = numTiles;

    return cTexture->Save(data);
}

bool CTileset::Load(const json& data)
{
    const json& tileset = data;

    name = tileset.at("name");
    tileWidth = tileset.at("tilewidth");
    tileHeight = tileset.at("tileHeight");
    numTiles = tileset.at("numTileset");

    return cTexture->Load(data);
}

bool CTileset::Load(const string_t& path)
{
    // its not a binary
    if (!N_stricmp(COM_GetExtension(path.c_str()), ".jtile")) {
        return LoadJSON(path);
    }
    return LoadBIN(path);
}

bool CTileset::LoadBIN(const string_t& path)
{
    return false;
}

bool CTileset::SaveBIN(const string_t& path) const
{
    return false;
}

bool CTileset::LoadJSON(const string_t& path)
{
    json data;
    const char *ext;

    ext = COM_GetExtension(path.c_str());
    if (!ext || N_stricmp(ext, ".jtile") != 0) {
        return false;
    }

    Printf("Loading tileset in json format: %s", path.c_str());

    if (!Editor::LoadJSON(data, path.c_str())) {
        Printf("Failed to load tileset %s", path.c_str());
        return false;
    }

    name = data.at("name");
    tileWidth = data.at("tilewidth");
    tileHeight = data.at("tileheight");
    if (!cTexture->Load(data)) {
        Printf("Failed to load tileset texture data");
        return false;
    }

    name = path;
    COM_StripExtension(name.c_str(), (char *)name.data(), name.size());
    modified = true;

    return true;
}

bool CTileset::SaveJSON(const string_t& path) const
{
    json data;
    const char *ext;

    ext = COM_GetExtension(path.c_str());
    if (!ext || N_stricmp(ext, ".jtile") != 0) {
        return false;
    }

    Printf("Saving tileset in json format: %s", path.c_str());

    data["name"] = name;
    data["tilewidth"] = tileWidth;
    data["tileheight"] = tileHeight;
    data["numTiles"] = numTiles;

    if (!cTexture->Save(data)) {
        Printf("Failed to save tileset texture data");
        return false;
    }

    if (!Editor::SaveJSON(data, path.c_str())) {
        Printf("Failed to save tileset %s", path.c_str());
        return false;
    }

    modified = false;

    return true;
}
