#ifndef __MAP__
#define __MAP__

#pragma once

// tile flags
#define TILE_CHECKPOINT 0x2000
#define TILE_SPAWN 0x4000

struct Vertex;

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

    bool mDarkAmbience;
    float mAmbientIntensity;
    glm::vec3 mAmbientColor;
    uint32_t mWidth;
    uint32_t mHeight;

    std::string mPath;
    std::string mName;

    bool mModified;

    CMapData(void);
    ~CMapData();

    void Clear(void);
    void SetMapSize(uint32_t width, uint32_t height);
    void CalcDrawData(void);
    void CalcLighting(Vertex *vertices, uint32_t numVertices);

    boost::shared_mutex resourceLock;

    const CMapData& operator=(const CMapData& other);
};

extern std::unique_ptr<CMapData> mapData;
extern char mapname[1024];

void Map_New(void);
void Map_Load(const char *filename);
void Map_Save(const char *filename);
void CheckAutoSave(void);
void CalcVertexNormals(Vertex *quad);

#endif