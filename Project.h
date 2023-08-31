#ifndef _PROJECT_H_
#define _PROJECT_H_

#pragma once

class CProject
{
public:
    CProject(void);
    ~CProject() = default;

    bool Save(const eastl::string& path) const;
    bool Load(const eastl::string& path);

    void New(void);

    inline const eastl::string& GetName(void) const
    { return name; }
    inline const std::filesystem::path& GetProjectDirectory(void) const
    { return projectDirectory; }
    inline const std::filesystem::path& GetAssetDirectory(void) const
    { return return assetDirectory; }
    inline const CMap *GetMap(void) const
    { return cMap; }
    inline const CTileset *GetTileset(void) const
    { return cTileset; }
private:
    eastl::string name;
    std::filesystem::path projectDirectory;
    std::filesystem::path assetDirectory;

    CMap *cMap;
    CTileset *cTileset;
};

#endif