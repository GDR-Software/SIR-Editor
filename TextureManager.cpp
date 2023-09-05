#include "Common.hpp"
#include "Editor.h"
#include "TextureManager.h"

CTextureManager::CTextureManager(void)
{
    listSize = 0;
    bits = 0;
}

CTextureManager::~CTextureManager()
{
}

bool CTextureManager::LoadList(void)
{
    vector_t<path_t> fileList;

    Editor::ListFiles(fileList, "Data/", {".jpg", ".png", ".bmp", ".jpeg", ".tga"});

    toolList.reserve(fileList.size());
    for (const auto& it : fileList) {
        toolList.try_emplace(it.c_str(), Allocate<CTexture>());
    }
    return true;
}

void CTextureManager::ClearList(void)
{
    for (auto& it : toolList) {
        Deallocate(it.second);
    }
    toolList.clear();
    listSize = 0;
}

void CTextureManager::DrawRecent(void)
{
}

typedef struct
{
    const char *texturePtr;
    int minfilter;
    int magfilter;

    bool entered = false;
} wizard_t;
static wizard_t wizard;

static inline const char *FilterToString(int filter)
{
    switch (filter) {
    case GL_LINEAR: return "GL_LINEAR";
    case GL_NEAREST: return "GL_NEAREST";
    case 0: return "None";
    };
    Error("Bad Filter");
    return "";
}

void CTextureManager::DrawWizard(const string_t& menuTitle)
{
    if (!wizard.entered) {
        memset(&wizard, 0, sizeof(wizard));
        wizard.entered = true;
    }

    if (ImGui::BeginMenu(menuTitle.c_str())) {
        if (wizard.texturePtr) {
            ImGui::MenuItem(wizard.texturePtr);
            if (ImGui::Button("Clear Texture")) {
                wizard.texturePtr = NULL;
            }
        }
        else if (ImGui::BeginMenu("Select Texture")) {
            for (const auto& it : toolList) {
                if (ImGui::MenuItem(it.first.c_str())) {
                    wizard.texturePtr = it.first.c_str();
                    Printf("Selected texture file '%s'", it.first.c_str());
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("GL_TEXTURE_MAG_FILTER")) {
            bool pressed = false;
            if (ImGui::Button("GL_LINEAR")) {
                wizard.magfilter = GL_LINEAR;
                pressed = true;
            }
            if (ImGui::Button("GL_NEAREST")) {
                wizard.magfilter = GL_NEAREST;
                pressed = true;
            }
            if (pressed) {
                Printf("Selected GL_TEXTURE_MAG_FILTER = %s", FilterToString(wizard.magfilter));
            }
            ImGui::EndMenu();
        }
        ImGui::Text("Selected: %s", FilterToString(wizard.magfilter));
        if (wizard.magfilter) {
            ImGui::SameLine();
            if (ImGui::Button("Clear Filter")) {
                wizard.magfilter = 0;
            }
        }

        if (ImGui::BeginMenu("GL_TEXTURE_MIN_FITLER")) {
            bool pressed = false;
            if (ImGui::Button("GL_LINEAR")) {
                wizard.minfilter = GL_LINEAR;
                pressed = true;
            }
            if (ImGui::Button("GL_NEAREST")) {
                wizard.minfilter = GL_NEAREST;
                pressed = true;
            }
            if (pressed) {
                Printf("Selected GL_TEXTURE_MIN_FILTER = %s", FilterToString(wizard.minfilter));
            }
            ImGui::EndMenu();
        }
        ImGui::Text("Selected: %s", FilterToString(wizard.minfilter));
        if (wizard.minfilter) {
            ImGui::SameLine();
            if (ImGui::Button("Clear Filter")) {
                wizard.minfilter = 0;
            }
        }

        if (ImGui::Button("Done") && wizard.texturePtr) {
            memset(&wizard, 0, sizeof(wizard));
            toolList.try_emplace(wizard.texturePtr, Allocate<CTexture>());
        }
        ImGui::EndMenu();
    }
}
