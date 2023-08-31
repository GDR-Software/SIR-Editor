#include "Common.hpp"
#include "Map.h"
#include "Tileset.h"
#include "Editor.h"
#include "Project.h"

CProject::CProject(void)
{
    New();
}

bool CProject::Save(json& data)
{
}

bool CProject::Save(const eastl::string& path) const
{
    json data;

    if (!modified)
        return true; // sometimes it recurses

    Printf("Saving current project...");

    data["name"] = name;
    data["mapname"] = cMap->GetName();
    data["texture"] = cTileset->GetTexture()->GetName();

    if (!Editor::SaveJSON(data, path)) {
        Error("Failed to save project!");
        return false;
    }

    Printf("Save successful");

    modified = false;
    
    return true;
}

void CProject::New(void)
{
    name = "untitled-project";
    cTileset = NULL;
    cMap = NULL;
    modified = true;
}

bool CProject::Load(const eastl::string& path)
{
    json data;
    
    if (!Editor::LoadJSON(data, path)) {
        Error("Failed to load project!");
        return false;
    }

    name = data.at("name");
    const eastl::string& mapName = data.at("mapname");
    const eastl::string& texture = data.at("texture");

    cMap->SetName(mapName);
    cMap->Load(mapName);
    CTileset->LoadTexture(texture);

    modified = true;

    return true;
}
