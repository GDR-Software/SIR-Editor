#ifndef _MAP_H_
#define _MAP_H_

#pragma once

#include "EditorTool.h"
#include "Tileset.h"

#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MAX_MAP_WIDTH 256
#define MAX_MAP_HEIGHT 256

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
    uint32_t s[3];

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
    CMap();
    virtual ~CMap() = default;

    virtual bool Save(json& data) const override;
    virtual bool Save(const eastl::string& path) const override;
    virtual bool Load(const json& data) override;
    virtual bool Load(const eastl::string& path) override;
    virtual void Clear(void) override;

    bool AddCheckpoint(const glm::vec2& pos, const glm::vec2& size);
    bool AddSpawn(const glm::vec2& pos, uint32_t entity);
private:
    Tile *tiles;

    CTileset *cTileset;

    eastl::vector<checkpoint_t> checkpoints;
    eastl::vector<spawn_t> spawns;

    uint32_t width;
    uint32_t height;
};

#endif