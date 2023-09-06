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

void CMapManager::DrawRecent(void)
{
    if (ImGui::BeginMenu("Maps")) {
        if (!toolList.size()) {
            ImGui::MenuItem("No Recent Maps");
        }
        else {
            for (const auto& it : toolList) {
                if (ImGui::MenuItem(it.first.c_str())) {
                    curTool = it.second;
                    curTool->Load(it.first);
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

    bool entered = false;
} wizard_t;
static wizard_t wizard;

static void DrawCreateMap(void)
{
    if (wizard.mode != MODE_CREATE_MAP) {
        return;
    }

    if (ImGui::Begin("Create Map")) {
        ImGui::InputText("Map Name", wizard.name, sizeof(wizard.name));
        ImGui::InputInt("Width", &wizard.width);
        ImGui::InputInt("Height", &wizard.height);
        ImGui::InputInt("# of Checkpoints", &wizard.numCheckpoints);
        ImGui::InputInt("# of Spawns", &wizard.numSpawns);

        if (ImGui::Button("Done")) {
            wizard.entered = false;
            curTool->Clear();
            curTool->Resize(wizard.dims);
            curTool->SetName(wizard.name);
            curTool->GetCheckpoints().resize(numCheckpoints);
            curTool->GetSpawns().resize(numSpawns);

            toolList.try_emplace(wizard.name, Allocate<CMap>());
            memset(&wizard, 0, sizeof(wizard));

            wizard.mode = MODE_INVALID;
        }
        
        ImGui::End();
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
    
    if (menuTitle == "Create Map") {
        wizard.mode = MODE_CREATE_MAP;
        DrawCreateMap();
        return;
    }

    wizard.width = curTool->GetWidth();
    wizard.height = curTool->GetHeight();
    wizard.numCheckpoints = curTool->GetCheckpoints().size();
    wizard.numSpawns = curTool->GetSpawns().size();

    if (ImGui::BeginMenu(menuTitle.c_str())) {
        ImGui::InputInt("Width", &wizard.width);
        ImGui::InputInt("Height", &wizard.height);
        
        if (ImGui::Button("Done")) {
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
