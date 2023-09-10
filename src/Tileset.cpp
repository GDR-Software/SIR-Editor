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
    cTexture = NULL;
}

CTileset::~CTileset()
{
    Clear();
}

void CTileset::Clear(void)
{
    tiles.clear();
    Deallocate(cTexture);
    name = "untitled-tileset";
}

void CTileset::GenTiles(void)
{
    const uint32_t numTilesX = cTexture->GetWidth() / tileWidth;
    const uint32_t numTilesY = cTexture->GetHeight() / tileHeight;
    maptile_t *tile;

    Printf("Generating tileset...");

    auto genCoords = [&](const glm::vec2& sheetDims, const glm::vec2& spriteDims, const glm::vec2& pos, float** texcoords) {
        const glm::vec2 min = { (pos.x * spriteDims.x) / sheetDims.x, (pos.y * spriteDims.y) / sheetDims.x };
        const glm::vec2 max = { ((pos.x + 1) * spriteDims.x) / sheetDims.y, ((pos.y + 1) * spriteDims.y) / sheetDims.y };

        texcoords[0][0] = min.x;
        texcoords[0][1] = min.y;
        texcoords[1][0] = max.x;
        texcoords[1][1] = min.y;
        texcoords[2][0] = max.x;
        texcoords[2][1] = max.y;
        texcoords[3][0] = min.x;
        texcoords[3][1] = max.y;
    };

    tiles.clear();
    tiles.resize(numTilesX * numTilesY);
    numTiles = numTilesX * numTilesY;
    modified = true;

    for (uint32_t y = 0; y < numTilesY; y++) {
        for(uint32_t x = 0; x < numTilesX; x++) {
            tile = &tiles[y * numTilesX + x];
            tile->index = y * numTilesX + x;

            tile->pos[0] = x;
            tile->pos[1] = y;

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
    if (!ext || N_stricmp(ext, ".jtile") != 0 && N_stricmp(ext, TILESET_FILE_EXT_RAW) != 0) {
        return false;
    }

    if (!N_stricmp(ext, ".jtile")) {
        return SaveJSON(path);
    }
    else if (!N_stricmp(ext, TILESET_FILE_EXT)) {
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

    if (cTexture)
        return cTexture->Save(data);
    
    return true;
}

bool CTileset::Load(const json& data)
{
    const json& tileset = data;

    name = tileset.at("name");
    tileWidth = tileset.at("tilewidth");
    tileHeight = tileset.at("tileHeight");
    numTiles = tileset.at("numTileset");

    if (cTexture)
        return cTexture->Load(data);
    
    return true;
}

bool CTileset::Load(const string_t& path)
{
    // its not a binary
    if (!N_stricmp(COM_GetExtension(path.c_str()), ".jtile")) {
        return LoadJSON(path);
    }
    return LoadBIN(path);
}

bool CTileset::Read(FILE *fp)
{
    tile2d_header_t header;
    tile2d_info_t info;

    memset(&header, 0, sizeof(header));

    SafeRead(&header, sizeof(header), fp);
    
    if (header.magic != TILESET_MAGIC) {
        return false;
    }
    if (header.version != TILESET_VERSION) {
        return false;
    }

    if (!cTexture)
        cTexture = Allocate<CTexture>();

    CopyLump((void *)&info, sizeof(info), header.lumps, TILESET_LUMP_INFO, fp);
    CopyLump<tile2d_sprite_t>(sprites, sizeof(tile2d_sprite_t), header.lumps, TILESET_LUMP_SPRITES, fp);
    CopyLump<byte>(cTexture->GetBuffer(), sizeof(byte), header.lumps, TILESET_LUMP_TEXTURE, fp);

    return true;
}

bool CTileset::Write(FILE *fp) const
{
    tile2d_header_t header;
    tile2d_info_t info;
    uint64_t pos;
    const uint64_t texSize = cTexture->GetWidth() * cTexture->GetHeight() * cTexture->GetChannels();

    memset(&header, 0, sizeof(header));

    pos = ftell(fp);

    // overwritten later
    SafeWrite(&header, sizeof(header), fp);

    header.magic = TILESET_MAGIC;
    header.version = TILESET_VERSION;

    info.numTiles = numTiles;
    info.tileHeight = tileHeight;
    info.tileWidth = tileWidth;
    info.width = width;
    info.height = height;
    info.magfilter = cTexture->GetMagFilter();
    info.minfilter = cTexture->GetMinFilter();
    info.wrapS = cTexture->GetWrapS();
    info.wrapT = cTexture->GetWrapT();

    AddLump(&info, sizeof(info), header.lumps, TILESET_LUMP_INFO, fp);
    AddLump(sprites.data(), sizeof(tile2d_sprite_t) * sprites.size(), header.lumps, TILESET_LUMP_SPRITES, fp);
    if (!cTexture) {
        byte buf[1];
        AddLump(buf, sizeof(byte), header.lumps, TILESET_LUMP_TEXTURE, fp);
    }
    else {
        AddLump(cTexture->GetBuffer().data(), sizeof(byte) * texSize, header.lumps, TILESET_LUMP_TEXTURE, fp);
    }

    fseek(fp, pos, SEEK_SET);
    SafeWrite(&header, sizeof(header), fp);

    return true;
}

bool CTileset::LoadBIN(const string_t& path)
{
    FILE *fp;

    fp = SafeOpenRead(path.c_str());
    Read(fp);
    fclose(fp);
    return false;
}

bool CTileset::SaveBIN(const string_t& path) const
{
    FILE *fp;
    
    fp = SafeOpenWrite(path.c_str());
    Write(fp);
    fclose(fp);

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
    if (cTexture) {
        if (!cTexture->Load(data)) {
            Printf("Failed to load tileset texture data");
            return false;
        }
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

    if (cTexture) {
        if (!cTexture->Save(data)) {
            Printf("Failed to save tileset texture data");
            return false;
        }
    }

    if (!Editor::SaveJSON(data, path.c_str())) {
        Printf("Failed to save tileset %s", path.c_str());
        return false;
    }

    modified = false;

    return true;
}
