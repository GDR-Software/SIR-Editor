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
    COM_DefaultExtension(path, pathlen, ".bmf");
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

    bool overwriting;
    bool entered = false;
} wizard_t;
static wizard_t wizard;

static void CheckExtension(void)
{
    if (N_stristr(wizard.name, TEXTURE_FILE_EXT) || COM_GetExtension(wizard.name) != NULL) {
        COM_StripExtension(wizard.name, wizard.name, sizeof(wizard.name));
    }
}

static void DrawCreateMap(void)
{
    if (wizard.mode != MODE_CREATE_MAP) {
        return;
    }

    CMap *curTool = Editor::GetMapManager()->GetCurrent();
    string_hash_t<object_ptr_t<CMap>>& toolList = Editor::GetMapManager()->GetList();

    if (ImGui::BeginMenu("Create Map")) {
        ImGui::InputText("Map Name", wizard.name, sizeof(wizard.name));
        ImGui::InputInt("Width", &wizard.width);
        ImGui::InputInt("Height", &wizard.height);
        ImGui::InputInt("# of Checkpoints", &wizard.numCheckpoints);
        ImGui::InputInt("# of Spawns", &wizard.numSpawns);

        if (ImGui::Button("Create Map")) {
            wizard.entered = false;

            if (toolList.find(wizard.name) != toolList.end()) {
                Editor::PushPopup({"Overwrite Map?", "You Are About To Overwrite An Existing Map, Are You Sure You Want To Continue?", "Confirm Overwrite", &wizard.overwriting});

                if (wizard.overwriting) {
                    Printf("Overwriting map %s", wizard.name);
                    curTool = toolList.at(wizard.name);
                }
            }
            else {
                CheckExtension();
                toolList.try_emplace(wizard.name, Allocate<CMap>());
            }

            curTool = toolList.at(wizard.name);
            curTool->Resize(wizard.dims);
            curTool->SetName(wizard.name);
            curTool->GetCheckpoints().resize(wizard.numCheckpoints);
            curTool->GetSpawns().resize(wizard.numSpawns);
            curTool->Save(string_t(BuildOSPath(Editor::GetPWD(), "Data", va("%s" MAP_FILE_EXT, wizard.name))));

            memset(&wizard, 0, sizeof(wizard));
            wizard.mode = MODE_INVALID;
        }
        
        ImGui::EndMenu();
    }
    else {
        memset(&wizard, 0, sizeof(wizard));
    }
}



void CMapManager::DrawWizard(const string_t& menuTitle)
{
    if (!wizard.entered) {
        memset(&wizard, 0, sizeof(wizard));
        wizard.entered = true;
    }
    
    if (menuTitle == "Create Map" || wizard.mode == MODE_CREATE_MAP) {
        wizard.mode = MODE_CREATE_MAP;
        DrawCreateMap();
        return;
    }

    wizard.width = curTool->GetWidth();
    wizard.height = curTool->GetHeight();
    wizard.numCheckpoints = curTool->GetCheckpoints().size();
    wizard.numSpawns = curTool->GetSpawns().size();
    N_strncpyz(wizard.name, curTool->GetName().c_str(), sizeof(wizard.name));

    if (ImGui::BeginMenu(menuTitle.c_str())) {
        ImGui::InputInt("Width", &wizard.width);
        ImGui::InputInt("Height", &wizard.height);
        
        if (ImGui::Button("Update Map")) {
            bool modified = false;
            if (wizard.width != curTool->GetWidth() || wizard.height != curTool->GetHeight()) {
                curTool->Resize(wizard.dims);
                modified = true;
            }
            if (curTool->GetName() != wizard.name) {
                curTool->SetName(wizard.name);
                modified = true;
            }
            if (wizard.numCheckpoints != curTool->GetCheckpoints().size()) {
                curTool->GetCheckpoints().resize(wizard.numCheckpoints);
                modified = true;
            }
            if (wizard.numSpawns != curTool->GetSpawns().size()) {
                curTool->GetSpawns().resize(wizard.numSpawns);
                modified = true;
            }
            curTool->SetModified(modified);
        }
        ImGui::EndMenu();
    }
}
