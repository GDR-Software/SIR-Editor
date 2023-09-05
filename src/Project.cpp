#include "Common.hpp"
#include "Map.h"
#include "Tileset.h"
#include "Editor.h"
#include "Project.h"

CProject::CProject(void)
{
    name = "untitled-project";
    path = BuildOSPath(Editor::GetPWD(), "Data/", "untitled-project.proj");
    modified = true;
    cMap = Editor::GetMap();
    cTileset = Editor::GetTileset();
}

CProject::~CProject()
{
}

bool CProject::Save(const string_t& path) const
{
    json data;

    if (!modified)
        return true; // sometimes it recurses

    Printf("Saving current project...");
    Printf("Project path: %s", path.c_str());

    data["name"] = name;
    
    cMap->Save(cMap->GetPath().c_str());
    cTileset->Save(data);

//    cMap->Save(data);
//    cTileset->Save(data);
    
    if (!Editor::SaveJSON(data, this->path)) {
        Error("Failed to save project!");
        return false;
    }

    Printf("Save successful");

    modified = false;
    
    return true;
}

bool CProject::Save(void) const
{
    json data;

    if (!modified)
        return true; // sometimes it recurses
    
    Printf("Saving current project...");

    data["name"] = name;

    cMap->SetModified(true);
    if (Editor::UseInternalMaps())
        cMap->Save(data["imap"]);
    else
        cMap->Save(cMap->GetPath().c_str());
    
    if (Editor::UseInternalTileset())
        cTileset->Save(data["itileset"]);
    else
        cTileset->Save(cTileset->GetPath().c_str());

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
    path = BuildOSPath(Editor::GetPWD(), "Data/", "untitled-project.proj");
    cMap->Clear();
    cTileset->Clear();
    modified = true;
}

bool CProject::Load(const string_t& path)
{
    json data;
    char *rpath;
    const char *ext;

    ext = COM_GetExtension(path.c_str());
    rpath = strdupa(path.c_str());
    if (IsAbsolutePath(path)) {
        rpath = BuildOSPath(Editor::GetPWD(), "Data/", GetFilename(path.c_str()));
    }
    if (!ext || N_stricmp(ext, ".proj")) {
        return false;
    }
    
    if (!Editor::LoadJSON(data, rpath)) {
        Error("Failed to load project!");
        return false;
    }

    name = data.at("name");

    if (data.contains("imap")) // internal map
        cMap->Load(data.at("imap"));
    else if (data.contains("emap")) // external map
        cMap->Load(data.at("emap").get<string_t>());
    else // no map
        cMap->Clear();
    
    if (data.contains("itileset")) // internal tileset
        cTileset->Load(data.at("itileset"));
    else if (data.contains("etileset")) // extern tileset
        cTileset->Load(data.at("etileset").get<string_t>());
    else
        cTileset->Clear();

    modified = true;

    return true;
}
