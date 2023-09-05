#include "Common.hpp"
#include "Editor.h"
#include "TilesetManager.h"

CTilesetManager::CTilesetManager(void)
{
    listSize = 0;
    bits = 0;
    curTool = NULL;
}

CTilesetManager::~CTilesetManager()
{
}

bool CTilesetManager::LoadList(void)
{
    vector_t<path_t> fileList;

    Editor::ListFiles(fileList, "Data/", {".tile"});

    curTool = Editor::GetTileset();

    toolList.reserve(fileList.size());
    for (const auto& it : fileList) {
        toolList.try_emplace(it.c_str(), Allocate<CTileset>());
    }
    return true;
}

void CTilesetManager::ClearList(void)
{
    for (auto& it : toolList) {
        Deallocate(it.second);
    }
    toolList.clear();
    listSize = 0;
}

void CTilesetManager::DrawRecent(void)
{
    if (ImGui::BeginMenu("Tilesets")) {
        if (!toolList.size()) {
            ImGui::MenuItem("No Recent Tileset");
        }
        else {
            for (auto& it : toolList) {
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
    IMPL_WIZARD_DIMS(sheetDims, sheetWidth, sheetHeight)
    IMPL_WIZARD_DIMS(spriteDims, spriteWidth, spriteHeight)
    const char *texturePtr;

    bool entered = false;
} wizard_t;
static wizard_t wizard;

void DrawTextureWizard(void)
{
    if (wizard.texturePtr) {
        ImGui::MenuItem(wizard.texturePtr);
        if (ImGui::Button("Clear Texture")) {
            wizard.texturePtr = NULL;
        }
    }
    else if (Editor::GetTextureManager()->GetListSize()) {
        for (const auto& it : Editor::GetTextureManager()->GetList()) {
            if (ImGui::MenuItem(it.first.c_str())) {
                wizard.texturePtr = it.first.c_str();
            }
        }
    }
}

void CTilesetManager::DrawWizard(const string_t& menuTitle)
{
    if (!wizard.entered) {
        memset(&wizard, 0, sizeof(wizard));
        wizard.entered = true;
    }

    if (ImGui::BeginMenu(menuTitle.c_str())) {
        ImGui::InputText("Tileset Name", wizard.name, sizeof(wizard.name));
        ImGui::InputInt2("Sheet Dimensions", wizard.sheetDims);
        ImGui::InputInt2("Tile Dimensions", wizard.spriteDims);

        DrawTextureWizard();
        
        if (ImGui::Button("Done")) {
            wizard.entered = false;
            curTool->SetTileDims(wizard.spriteDims);
            curTool->SetSheetDims(wizard.sheetDims);
            curTool->SetName(wizard.name);
            curTool->GenTiles();

            toolList.try_emplace(wizard.name, Allocate<CTileset>());
            memset(&wizard, 0, sizeof(wizard));
        }
        ImGui::EndMenu();
    }
}
