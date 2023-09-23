#ifndef __MAP__
#define __MAP__

#pragma once

// tile flags
#define TILE_CHECKPOINT 0x2000
#define TILE_SPAWN 0x4000

class CMapData
{
public:
    std::vector<maptile_t> mTiles;
    std::vector<maplight_t> mLights;
    std::vector<uint32_t> mIndices;
    std::vector<mapvert_t> mVertices;
    std::vector<mapspawn_t> mSpawns;
    std::vector<mapcheckpoint_t> mCheckpoints;
    std::vector<CEntity> mEntities;

    uint32_t mWidth;
    uint32_t mHeight;

    std::string mPath;
    std::string mName;

    CMapData(void);
    ~CMapData();

    void Clear(void);

    const CMapData& operator=(const CMapData& other)
    {
        Clear();
        mTiles.resize(other.mTiles.size());
        mLights.resize(other.mLights.size());
        mIndices.resize(other.mIndices.size());
        mVertices.resize(other.mVertices.size());
        mSpawns.resize(other.mSpawns.size());
        mCheckpoints.resize(other.mCheckpoints.size());
        mEntities.resize(other.mEntities.size());

        memcpy(mTiles.data(), other.mTiles.data(), sizeof(maptile_t) * other.mTiles.size());
        memcpy(mLights.data(), other.mLights.data(), sizeof(maplight_t) * other.mLights.size());
        memcpy(mIndices.data(), other.mIndices.data(), sizeof(uint32_t) * other.mIndices.size());
        memcpy(mVertices.data(), other.mVertices.data(), sizeof(mapvert_t) * other.mVertices.size());
        memcpy(mSpawns.data(), other.mSpawns.data(), sizeof(mapspawn_t) * other.mSpawns.size());
        memcpy(mCheckpoints.data(), other.mCheckpoints.data(), sizeof(mapcheckpoint_t) * other.mCheckpoints.size());
        memcpy(mEntities.data(), other.mEntities.data(), sizeof(CEntity) * other.mEntities.size());

        return *this;
    }
};

extern CMapData mapData;
extern char mapname[1024];

void Map_New(void);
void Map_Load(const char *filename);
void Map_Save(const char *filename);

#endif