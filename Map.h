#ifndef _MAP_H_
#define _MAP_H_

#pragma once

#include "Editor.h"
#include "EditorTool.h"
#include "Tileset.h"

#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MAX_MAP_WIDTH 1024
#define MAX_MAP_HEIGHT 1024

struct Tile
{
    glm::vec2 texcoords[4];
    glm::vec3 pos;
    uint32_t index;
    bool empty;

    inline Tile(void) = default;
    inline Tile(Tile &) = default;
};

typedef struct
{
    uint32_t c[4];

    inline operator uint32_t *(void)
    { return c; }
    inline operator const uint32_t *(void) const
    { return c; }
    inline uint32_t& operator[](int index)
    { return c[index]; }
    inline uint32_t *data(void)
    { return c; }
    inline const uint32_t *data(void) const
    { return c; }
} checkpoint_t;

typedef struct
{
    uint32_t s[4];

    inline operator uint32_t *(void)
    { return s; }
    inline operator const uint32_t *(void) const
    { return s; }
    inline uint32_t& operator[](int index)
    { return s[index]; }
    inline uint32_t *data(void)
    { return s; }
    inline const uint32_t *data(void) const
    { return s; }
} spawn_t;

class CMap : public CEditorTool
{
public:
    CMap(void);
    virtual ~CMap();

    virtual bool Save(json& data) const;
    virtual bool Save(const string_t& path) const;
    virtual bool Load(const json& data);
    virtual bool Load(const string_t& path);
    virtual void Clear(void);

    void Resize(int dims[2]);

    inline uint32_t GetWidth(void) const
    { return width; }
    inline uint32_t GetHeight(void) const
    { return height; }
    inline const vector_t<checkpoint_t>& GetCheckpoints(void) const
    { return checkpoints; }
    inline const vector_t<spawn_t>& GetSpawns(void) const
    { return spawns; }
    inline const vector_t<Tile>& GetTiles(void) const
    { return tiles; }
    inline vector_t<checkpoint_t>& GetCheckpoints(void)
    { return checkpoints; }
    inline vector_t<spawn_t>& GetSpawns(void)
    { return spawns; }
    inline vector_t<Tile>& GetTiles(void)
    { return tiles; }

    bool AddCheckpoint(const checkpoint_t& c);
    inline bool AddSpawn(const spawn_t& s)
    { return AddSpawn({s[0], s[1]}, s[2], s[3]); }
    bool AddSpawn(const glm::vec2& pos, uint32_t entityType, uint32_t entityId);
private:
    bool SaveBIN(const string_t& path) const;

    vector_t<Tile> tiles;
    vector_t<checkpoint_t> checkpoints;
    vector_t<spawn_t> spawns;

    object_ptr_t<CTileset> cTileset;

    uint32_t width;
    uint32_t height;
};

#endif