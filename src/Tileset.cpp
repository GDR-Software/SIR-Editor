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
    char *rpath;
    const char *ext;

    ext = COM_GetExtension(path.c_str());
    rpath = strdupa(path.c_str());
    if (IsAbsolutePath(path)) {
        rpath = BuildOSPath(Editor::GetPWD(), "Data/", GetFilename(path.c_str()));
    }
    if (!ext || N_stricmp(ext, ".tile") && N_stricmp(ext, ".btf")) {
        return false;
    }

    if (!N_stricmp(ext, ".tile")) {
        return SaveJSON(rpath);
    }
    else if (!N_stricmp(ext, ".btf")) {
        return SaveBIN(rpath);
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
    if (!N_stricmp(COM_GetExtension(path.c_str()), ".tile")) {
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
    char *rpath;
    const char *ext;

    ext = COM_GetExtension(path.c_str());
    rpath = strdupa(path.c_str());
    if (IsAbsolutePath(path)) {
        rpath = BuildOSPath(Editor::GetPWD(), "Data/", GetFilename(path.c_str()));
    }
    if (!ext || N_stricmp(ext, ".tile")) {
        return false;
    }

    Printf("Loading tileset in json format: %s", rpath);

    if (!Editor::LoadJSON(data, rpath)) {
        Printf("Failed to load tileset %s", rpath);
        return false;
    }

    name = data.at("name");
    tileWidth = data.at("tilewidth");
    tileHeight = data.at("tileheight");
    if (!cTexture->Load(data)) {
        Printf("Failed to load tileset texture data");
        return false;
    }

    modified = true;

    return true;
}

bool CTileset::SaveJSON(const string_t& path) const
{
    json data;
    char *rpath;
    const char *ext;

    ext = COM_GetExtension(path.c_str());
    rpath = strdupa(path.c_str());
    if (IsAbsolutePath(path)) {
        rpath = BuildOSPath(Editor::GetPWD(), "Data/", GetFilename(path.c_str()));
    }
    if (!ext || N_stricmp(ext, ".tile")) {
        return false;
    }

    Printf("Saving tileset in json format: %s", rpath);

    data["name"] = name;
    data["tilewidth"] = tileWidth;
    data["tileheight"] = tileHeight;
    data["numTiles"] = numTiles;

    if (!cTexture->Save(data)) {
        Printf("Failed to save tileset texture data");
        return false;
    }

    if (!Editor::SaveJSON(data, rpath)) {
        Printf("Failed to save tileset %s", rpath);
        return false;
    }

    modified = false;

    return true;
}
