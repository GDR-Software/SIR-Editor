#include "gln.h"

void CTileset::GenerateTiles(void)
{
    auto genCoords = [&](const glm::vec2& sheetDims, const glm::vec2& spriteDims, const glm::vec2& coords, float texcoords[4][2]) {
        const glm::vec2 min = { (coords.x * spriteDims.x) / sheetDims.x, (coords.y * spriteDims.y) / sheetDims.y };
        const glm::vec2 max = { ((coords.x + 1) * spriteDims.x) / sheetDims.x, ((coords.y + 1) * spriteDims.y) / sheetDims.y };

        texcoords[0][0] = min.x;
        texcoords[0][1] = min.y;

        texcoords[1][0] = max.x;
        texcoords[1][1] = min.y;

        texcoords[2][0] = max.x;
        texcoords[2][1] = max.y;
        
        texcoords[3][0] = min.x;
        texcoords[3][1] = max.y;
    };
    
    tileCountX = texData->mWidth / tileWidth;
    tileCountY = texData->mHeight / tileHeight;

    tiles.resize(tileCountX * tileCountY);
    memset(tiles.data(), 0, sizeof(maptile_t) * tiles.size());
    for (uint32_t y = 0; y < tileCountY; y++) {
        for (uint32_t x = 0; x < tileCountX; x++) {
            genCoords({ texData->mWidth, texData->mHeight }, { tileWidth, tileHeight }, { x, y }, tiles[y * tileCountX + x].texcoords);
        }
    }
}