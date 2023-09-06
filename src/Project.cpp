#include "Common.hpp"
#include "Map.h"
#include "Tileset.h"
#include "Editor.h"
#include "Project.h"

CProject::CProject(void)
{
    name = "untitled-project";
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
    char *tmp;
    char savepath[MAX_OSPATH*2+1];

    if (!modified)
        return true; // sometimes it recurses

    N_strncpyz(savepath, BuildOSPath(Editor::GetPWD(), "Data", path.c_str()), sizeof(savepath));
    if (!N_stristr(savepath, ".proj")) {
        N_strcat(savepath, sizeof(savepath), ".proj");
    }

    Printf("Saving current project...");

    data["name"] = name;
    
    if (parm_useInternalMaps && parm_saveJsonMaps) {
        cMap->Save(data["imap"]);
    }
    else if (parm_useInternalMaps && !parm_saveJsonMaps) {
        Printf("-imap needs -jmap as well, saving binary map");
        char str[MAX_GDR_PATH];
        N_strncpyz(str, path.c_str(), sizeof(str));
        if (!N_stristr(str, ".bmf")) {
            if (N_stristr(str, ".jmap")) {
                COM_StripExtension(str, str, sizeof(str));
            }
            N_strcat(str, sizeof(str), ".bmf");
        }
        tmp = BuildOSPath(Editor::GetPWD(), "Data", str);
        cMap->Save(tmp);
    }
    else {
        cMap->Save(data["emap"]);
    }
    if (parm_useInternalTilesets && parm_saveJsonTilesets) {
        cTileset->Save(data["itileset"]);
    }
    else if (parm_useInternalMaps && !parm_saveJsonTilesets) {
        Printf("-itileset needs -jtileset as well, saving binary map");
        char str[MAX_GDR_PATH];
        N_strncpyz(str, path.c_str(), sizeof(str));
        if (!N_stristr(str, ".t2d")) {
            if (N_stristr(str, ".jtile")) {
                COM_StripExtension(str, str, sizeof(str));
            }
            N_strcat(str, sizeof(str), ".t2d");
        }
        tmp = BuildOSPath(Editor::GetPWD(), "Data", str);
        cTileset->Save(tmp);
    }
    else {
        cTileset->Save(data["etileset"]);
    }
    
    if (!Editor::SaveJSON(data, savepath)) {
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
    char *tmp;
    char savepath[MAX_OSPATH*2+1];

    if (!modified)
        return true; // sometimes it recurses
    
    N_strncpyz(savepath, BuildOSPath(Editor::GetPWD(), "Data", name.c_str()), sizeof(savepath));
    if (!N_stristr(savepath, ".proj")) {
        N_strcat(savepath, sizeof(savepath), ".proj");
    }
    
    Printf("Saving current project...");

    data["name"] = name;

    cMap->SetModified(true);
    if (parm_useInternalMaps && parm_saveJsonMaps) {
        cMap->Save(data["imap"]);
    }
    else if (parm_useInternalMaps && !parm_saveJsonMaps) {
        Printf("-imap needs -jmap as well, saving binary map");
        char str[MAX_GDR_PATH];
        N_strncpyz(str, cMap->GetName().c_str(), sizeof(str));
        if (!N_stristr(str, ".bmf")) {
            if (N_stristr(str, ".jmap")) {
                COM_StripExtension(str, str, sizeof(str));
            }
            N_strcat(str, sizeof(str), ".bmf");
        }
        tmp = BuildOSPath(Editor::GetPWD(), "Data", str);
        cMap->Save(tmp);
    }
    if (parm_useInternalTilesets && parm_saveJsonTilesets) {
        cTileset->Save(data["itileset"]);
    }
    else if (parm_useInternalMaps && !parm_saveJsonTilesets) {
        Printf("-itileset needs -jtileset as well, saving binary map");
        char str[MAX_GDR_PATH];
        N_strncpyz(str, cTileset->GetName().c_str(), sizeof(str));
        if (!N_stristr(str, ".t2d")) {
            if (N_stristr(str, ".jtile")) {
                COM_StripExtension(str, str, sizeof(str));
            }
            N_strcat(str, sizeof(str), ".t2d");
        }
        tmp = BuildOSPath(Editor::GetPWD(), "Data", str);
        cTileset->Save(tmp);
    }
    else {
        cTileset->Save(data["etileset"]);
    }

    if (!Editor::SaveJSON(data, savepath)) {
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
    cMap->Clear();
    cTileset->Clear();
    modified = true;
}

bool CProject::Load(const string_t& path)
{
    json data;
    const char *ext;
    char savepath[MAX_OSPATH*2+1];

    ext = COM_GetExtension(path.c_str());
    N_strncpyz(savepath, BuildOSPath(Editor::GetPWD(), "Data", path.c_str()), sizeof(savepath));
    if (!ext || N_stricmp(ext, ".proj") != 0) {
        return false;
    }
    
    if (!Editor::LoadJSON(data, savepath)) {
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
