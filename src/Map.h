#ifndef _MAP_H_
#define _MAP_H_

#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MAX_MAP_WIDTH 1024
#define MAX_MAP_HEIGHT 1024

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
    inline const vector_t<mapcheckpoint_t>& GetCheckpoints(void) const
    { return checkpoints; }
    inline const vector_t<mapspawn_t>& GetSpawns(void) const
    { return spawns; }
    inline const vector_t<maptile_t>& GetTiles(void) const
    { return tiles; }
    inline vector_t<mapcheckpoint_t>& GetCheckpoints(void)
    { return checkpoints; }
    inline vector_t<mapspawn_t>& GetSpawns(void)
    { return spawns; }
    inline vector_t<maptile_t>& GetTiles(void)
    { return tiles; }
    inline const object_ptr_t<CTileset>& GetTileset(void) const
    { return cTileset; }
    inline object_ptr_t<CTileset>& GetTileset(void)
    { return cTileset; }

    bool AddCheckpoint(const mapcheckpoint_t& c);
    inline bool AddSpawn(const mapspawn_t& s)
    { return AddSpawn({s.pos[0], s.pos[1]}, s.entitytype, s.entityid); }
    bool AddSpawn(const glm::vec2& pos, uint32_t entityType, uint32_t entityId);
private:
    bool SaveBIN(const string_t& path) const;

    vector_t<maptile_t> tiles;
    vector_t<mapcheckpoint_t> checkpoints;
    vector_t<mapspawn_t> spawns;

    object_ptr_t<CTileset> cTileset;

    uint32_t width;
    uint32_t height;
};

#endif