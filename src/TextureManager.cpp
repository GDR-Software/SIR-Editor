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
    int target;
    bool techy = false;

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

static void DrawWizardChoices_Techy(void)
{
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

    if (ImGui::BeginMenu("GL Texture Target")) {
        if (ImGui::Button("GL_TEXTURE_2D")) {
            wizard.target = GL_TEXTURE_2D;
        }
        if (ImGui::Button("GL_TEXTURE_2D_MULTISAMPLE")) {
            wizard.target = GL_TEXTURE_2D_MULTISAMPLE;
        }
        ImGui::EndMenu();
    }
}

static void DrawWizardChoices(void)
{
    if (wizard.techy) {
        DrawWizardChoices_Techy();
        return;
    }

    if (ImGui::BeginMenu("Magnification Filter")) {
        bool pressed = false;
        if (ImGui::Button("Linear")) {
            wizard.magfilter = GL_LINEAR;
            pressed = true;
        }
        if (ImGui::Button("Nearest")) {
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

    if (ImGui::BeginMenu("Minification Filter")) {
        bool pressed = false;
        if (ImGui::Button("Linear")) {
            wizard.minfilter = GL_LINEAR;
            pressed = true;
        }
        if (ImGui::Button("Nearest")) {
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

    if (ImGui::BeginMenu("Use Multisampling")) {
        if (ImGui::Button("Yes")) {
            wizard.target = GL_TEXTURE_2D_MULTISAMPLE;
        }
        if (ImGui::Button("No")) {
            wizard.target = GL_TEXTURE_2D;
        }
    }
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
            if (!toolList.size()) {
                ImGui::MenuItem("No Textures in Data/");
            }
            else {
                for (const auto& it : toolList) {
                    if (ImGui::MenuItem(it.first.c_str())) {
                        wizard.texturePtr = it.first.c_str();
                        Printf("Selected texture file '%s'", it.first.c_str());
                    }
                }
            }
            ImGui::EndMenu();
        }

        DrawWizardChoices();

        wizard.techy = ImGui::Button("I'm A Techie");
        if (ImGui::Button("Create Texture") && wizard.texturePtr) {
            memset(&wizard, 0, sizeof(wizard));
            toolList.try_emplace(wizard.texturePtr, Allocate<CTexture>());
        }
        ImGui::EndMenu();
    }
}
