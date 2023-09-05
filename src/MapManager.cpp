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

    Editor::ListFiles(fileList, "Data/", {".map", ".jmap"});

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

typedef struct
{
    char name[1024];
    IMPL_WIZARD_DIMS(dims, width, height)

    bool entered = false;
} wizard_t;
static wizard_t wizard;

void CMapManager::DrawWizard(const string_t& menuTitle)
{
    if (!wizard.entered) {
        memset(&wizard, 0, sizeof(wizard));
        wizard.entered = true;
    }

    if (ImGui::BeginMenu(menuTitle.c_str())) {
        ImGui::InputText("Map Name", wizard.name, sizeof(wizard.name));
        ImGui::InputInt("Width", &wizard.width);
        ImGui::InputInt("Height", &wizard.height);
        if (ImGui::Button("Done")) {
            wizard.entered = false;
            curTool->Clear();
            curTool->Resize(wizard.dims);
            curTool->SetName(wizard.name);

            toolList.try_emplace(wizard.name, Allocate<CMap>());
            memset(&wizard, 0, sizeof(wizard));
        }
        ImGui::EndMenu();
    }
}