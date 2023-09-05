#include "Common.hpp"
#include "Editor.h"
#include "ProjectManager.h"

CProjectManager::CProjectManager(void)
{
    listSize = 0;
    bits = 0;
    curTool = NULL;
}

CProjectManager::~CProjectManager()
{
}

bool CProjectManager::LoadList(void)
{
    vector_t<path_t> fileList;

    Editor::ListFiles(fileList, "Data/", {".proj"});

    curTool = Editor::GetProject();

    for (const auto& it : fileList) {
        toolList.try_emplace(it.c_str(), Allocate<CProject>());
    }
    return true;
}

void CProjectManager::LoadProject(const string_t& name)
{
    curTool->Load(name);
}

void CProjectManager::DrawRecent(void)
{
    if (ImGui::BeginMenu("Projects")) {
        if (!toolList.size()) {
            ImGui::MenuItem("No Recent Projects");
        }
        else {
            for (auto& it : toolList) {
                if (ImGui::MenuItem(it.first.c_str())) {
                    curTool = it.second;
                    LoadProject(it.first.c_str());
                }
            }
        }
        ImGui::EndMenu();
    }
}

void CProjectManager::ClearList(void)
{
    for (auto it : toolList) {
        Deallocate(it.second);
    }
    toolList.clear();
    listSize = 0;
}

typedef struct
{
    char name[2048];
    const char *mapPtr;
    const char *tilesetPtr;

    bool entered = false;
} wizard_t;
static wizard_t wizard;

static void DrawMap(void)
{
    if (wizard.mapPtr) {
        ImGui::MenuItem(wizard.mapPtr);
        if (ImGui::Button("Clear Map")) {
            wizard.mapPtr = NULL;
        }
    }
    else if (Editor::GetMapManager()->GetListSize()) {
        if (ImGui::BeginMenu("Select Map")) {
            for (auto& it : Editor::GetMapManager()->GetList()) {
                if (ImGui::MenuItem(it.first.c_str())) {
                    wizard.mapPtr = it.first.c_str();
                }
            }
            ImGui::EndMenu();
        }
    }
    else {
        if (Editor::GetMapManager()->HasWizard()) {
            Editor::GetMapManager()->DrawWizard("Create Map");
            wizard.mapPtr = Editor::GetMapManager()->GetCurrentName().c_str();
            if (!N_stricmp(wizard.mapPtr, "untitled-map.map")) {
                wizard.mapPtr = NULL;
            }
        }
    }
}

static void DrawTileset(void)
{
    if (wizard.tilesetPtr) {
        ImGui::MenuItem(wizard.tilesetPtr);
        if (ImGui::Button("Clear Tileset")) {
            wizard.tilesetPtr = NULL;
        }
    }
    else if (Editor::GetTilesetManager()->GetListSize()) {
        if (ImGui::BeginMenu("Select Tileset")) {
            for (const auto& it : Editor::GetTilesetManager()->GetList()) {
                if (ImGui::MenuItem(it.first.c_str())) {
                    wizard.tilesetPtr = it.first.c_str();
                }
            }
            ImGui::End();
        }
    }
    else {
        if (Editor::GetTilesetManager()->HasWizard()) {
            Editor::GetTilesetManager()->DrawWizard("Create Tileset");
            wizard.tilesetPtr = Editor::GetTilesetManager()->GetCurrentName().c_str();
            if (!N_stricmp(wizard.tilesetPtr, "untitled-tileset.tile")) {
                wizard.tilesetPtr = NULL;
            }
        }
    }
}

void CProjectManager::DrawWizard(const string_t& menuTitle)
{
    if (!wizard.entered) {
        memset(&wizard, 0, sizeof(wizard));
        wizard.entered = true;
    }

    if (ImGui::BeginMenu(menuTitle.c_str())) {
        ImGui::InputText("Project Name", wizard.name, sizeof(wizard.name));
        DrawMap();
        DrawTileset();
        if (ImGui::Button("Done")) {
            wizard.entered = false;
            curTool->SetName(wizard.name);
            curTool->SetModified(true);
            if (wizard.mapPtr)
                curTool->SetMap(Editor::GetMapManager()->GetTool(wizard.mapPtr));
            if (wizard.tilesetPtr)
                curTool->SetTileset(Editor::GetTilesetManager()->GetTool(wizard.tilesetPtr));

            toolList.try_emplace(wizard.name, Allocate<CProject>());
            memset(&wizard, 0, sizeof(wizard));
        }
        ImGui::EndMenu();
    }
}