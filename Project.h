#ifndef _PROJECT_H_
#define _PROJECT_H_

#pragma once

#include "Map.h"
#include "Tileset.h"
#include "EditorTool.h"

class CProject : public CEditorTool
{
public:
    CProject(void);
    virtual ~CProject();

    virtual bool Save(const string_t& path) const;
    virtual bool Save(void) const;
    virtual bool Load(const string_t& path);

    void New(void);

    inline void SetMap(const object_ptr_t<CMap>& nMap)
    { cMap = nMap; }
    inline void SetTileset(const object_ptr_t<CTileset>& nTileset)
    { cTileset = nTileset; }

    inline const string_t& GetName(void) const
    { return name; }
    inline void SetProjectDirectory(const std::filesystem::path& dir)
    { projectDirectory = dir; }
    inline void SetAssetDirectory(const std::filesystem::path& dir)
    { assetDirectory = dir; }
    inline const std::filesystem::path& GetProjectDirectory(void) const
    { return projectDirectory; }
    inline const std::filesystem::path& GetAssetDirectory(void) const
    { return assetDirectory; }
    inline const object_ptr_t<CMap>& GetMap(void) const
    { return cMap; }
    inline const object_ptr_t<CTileset>& GetTileset(void) const
    { return cTileset; }
private:
    string_t name;
    std::filesystem::path projectDirectory;
    std::filesystem::path assetDirectory;

    object_ptr_t<CMap> cMap;
    object_ptr_t<CTileset> cTileset;
};

#endif