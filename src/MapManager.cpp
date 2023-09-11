#include "Common.hpp"
#include "Editor.h"
#include "MapManager.h"

CMapManager::CMapManager(void)
{
    listSize = 0;
    bits = 0;
    curTool = NULL;
}

CMapManager::~CMapManager()
{
}

bool CMapManager::LoadList(void)
{
    vector_t<path_t> fileList;

    Editor::ListFiles(fileList, "Data/", {".bmf", ".jmap"});

    curTool = Editor::GetMap();

    toolList.reserve(fileList.size());
    for (const auto& it : fileList) {
        toolList.try_emplace(it.generic_string().c_str(), Allocate<CMap>());
    }
    return true;
}

void CMapManager::ClearList(void)
{
    for (auto it : toolList) {
        Deallocate(it.second);
    }
    toolList.clear();
    listSize = 0;
}

static void MakePath(char *path, uint64_t pathlen, const eastl::string& name)
{
    N_strncpyz(path, name.c_str(), pathlen);
    if (IsAbsolutePath(name.c_str())) {
        N_strncpyz(path, BuildOSPath(Editor::GetPWD(), "Data", name.c_str()), pathlen);
    }
    COM_DefaultExtension(path, pathlen, MAP_FILE_EXT);
}

void CMapManager::DrawRecent(void)
{
    char str[MAX_OSPATH*2+1];
    if (ImGui::BeginMenu("Maps")) {
        if (!toolList.size()) {
            ImGui::MenuItem("No Recent Maps");
        }
        else {
            for (const auto& it : toolList) {
                MakePath(str, sizeof(str), it.first);
                if (ImGui::MenuItem(str)) {
                    curTool = it.second;
                    curTool->Load(string_t(str));
                    Editor::GetProjManager()->GetCurrent()->SetMap(curTool);
                }
            }
        }
        ImGui::EndMenu();
    }
}

#define MODE_INVALID -1
#define MODE_CREATE_MAP 0
#define MODE_UPDATE_MAP 1

typedef struct
{
    char name[1024];
    IMPL_WIZARD_DIMS(dims, width, height)
    int numCheckpoints;
    int numSpawns;
    int mode;
    CMap *mapPtr;

    bool overwriting;
    bool entered = false;
} wizard_t;
static wizard_t wizard;

static void CheckExtension(void)
{
    if (N_stristr(wizard.name, MAP_FILE_EXT) || COM_GetExtension(wizard.name) != NULL) {
        COM_StripExtension(wizard.name, wizard.name, sizeof(wizard.name));
    }
}

void CMapManager::DrawWizard(const string_t& menuTitle)
{
    if (!wizard.entered) {
        memset(&wizard, 0, sizeof(wizard));
        wizard.entered = true;
    }

    if (ImGui::BeginMenu("Map Wizard")) {
        ImGui::InputText("Map Name", wizard.name, sizeof(wizard.name));
        ImGui::InputInt("Width", &wizard.width);
        ImGui::InputInt("Height", &wizard.height);
        ImGui::InputInt("# of Checkpoints", &wizard.numCheckpoints);
        ImGui::InputInt("# of Spawns", &wizard.numSpawns);

        if (!wizard.mapPtr && ImGui::BeginMenu("Open Map")) {
            if (!toolList.size()) {
                ImGui::MenuItem("No Recent Maps");
            }
            else {
                for (auto& it : toolList) {
                    if (ImGui::MenuItem(it.first.c_str())) {
                        wizard.mapPtr = it.second;
                        wizard.mapPtr->Load(string_t(it.first.c_str()));

                        wizard.width = wizard.mapPtr->GetWidth();
                        wizard.height = wizard.mapPtr->GetHeight();
                        wizard.numCheckpoints = wizard.mapPtr->GetCheckpoints().size();
                        wizard.numSpawns = wizard.mapPtr->GetSpawns().size();
                        N_strncpyz(wizard.name, wizard.mapPtr->GetName().c_str(), sizeof(wizard.name));
                        COM_StripExtension(wizard.name, wizard.name, sizeof(wizard.name));
                        break;
                    }
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::Button(wizard.mapPtr ? "Update Map" : "Create Map")) {
            bool modified = false;

            CheckExtension();
            string_t path = BuildOSPath(Editor::GetPWD(), "Data", va("%s" MAP_FILE_EXT, wizard.name));
            Printf("Saving map '%s'", path.c_str());

            if (wizard.mapPtr) {
                if (wizard.width != wizard.mapPtr->GetWidth() || wizard.height != wizard.mapPtr->GetHeight()) {
                    wizard.mapPtr->Resize(wizard.dims);
                }
                if (wizard.mapPtr->GetName() != wizard.name) {
                    wizard.mapPtr->SetName(wizard.name);
                }
                if (wizard.numCheckpoints != wizard.mapPtr->GetCheckpoints().size()) {
                    wizard.mapPtr->GetCheckpoints().resize(wizard.numCheckpoints);
                }
                if (wizard.numSpawns != wizard.mapPtr->GetSpawns().size()) {
                    wizard.mapPtr->GetSpawns().resize(wizard.numSpawns);
                }
                wizard.mapPtr->Save(path);
                curTool = wizard.mapPtr;
            }
            else {
                toolList.try_emplace(path.c_str(), Allocate<CMap>());

                curTool = toolList.at(path.c_str());
                curTool->Resize(wizard.dims);
                curTool->GetCheckpoints().resize(wizard.numCheckpoints);
                curTool->GetSpawns().resize(wizard.numSpawns);
                curTool->SetName(wizard.name);
                curTool->Save(path);
            }
            memset(&wizard, 0, sizeof(wizard));
        }
        ImGui::EndMenu();
    }
}
