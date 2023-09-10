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

    Editor::ListFiles(fileList, "Data/", {".jtile", ".t2d"});

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
        const CTexture *tex = Editor::GetTextureManager()->GetTool(wizard.texturePtr);
        wizard.sheetHeight = tex->GetHeight();
        wizard.sheetWidth = tex->GetWidth();
        if (ImGui::Button("Clear Texture")) {
            wizard.texturePtr = NULL;
        }
    }
    else if (ImGui::BeginMenu("Select Texture")) {
        for (const auto& it : Editor::GetTextureManager()->GetList()) {
            if (ImGui::MenuItem(it.first.c_str())) {
                wizard.texturePtr = it.first.c_str();
                wizard.sheetHeight = it.second->GetHeight();
                wizard.sheetWidth = it.second->GetWidth();

                const eastl::string& name = it.first;
                if (name.find(".png") != eastl::string::npos || name.find(".jpg") != eastl::string::npos
                || name.find(".jpeg") != eastl::string::npos || name.find(".bmp") != eastl::string::npos) {
                    it.second->LoadImage(name.c_str());
                }
                else if (name.find(TEXTURE_FILE_EXT)) {
                    it.second->Load(string_t(name.c_str()));
                }
            }
        }
        ImGui::EndMenu();
    }
}

static void CheckExtension(void)
{
    if (N_stristr(wizard.name, TILESET_FILE_EXT) || COM_GetExtension(wizard.name) != NULL) {
        COM_StripExtension(wizard.name, wizard.name, sizeof(wizard.name));
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
        ImGui::InputInt2("Tile Dimensions", wizard.spriteDims);
        ImGui::Text("Image Width: %4i\tImage Height: %4i", wizard.sheetWidth, wizard.sheetHeight);

        DrawTextureWizard();
        
        if (ImGui::Button("Create Tileset")) {
            Printf("Creating tileset '%s'", wizard.name);
            wizard.entered = false;
            toolList.try_emplace(wizard.name, Allocate<CTileset>());

            CheckExtension();
            curTool = toolList.at(wizard.name);
            curTool->SetTexture(Editor::GetTextureManager()->GetTool(wizard.texturePtr));
            curTool->SetTileDims(wizard.spriteDims);
            curTool->SetSheetDims(wizard.sheetDims);
            curTool->SetName(wizard.name);
            curTool->GenTiles();
            curTool->Save(string_t(BuildOSPath(Editor::GetPWD(), "Data", va("%s" TILESET_FILE_EXT, wizard.name))));

            memset(&wizard, 0, sizeof(wizard));
        }
        ImGui::EndMenu();
    }
}
