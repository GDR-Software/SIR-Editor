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

    Editor::ListFiles(fileList, "Data/", {".jpg", ".png", ".bmp", ".jpeg", ".tga", TEXTURE_FILE_EXT});

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
    char name[1024];
    const char *texturePtr;
    int minfilter;
    int magfilter;
    int target;
    int wrapS;
    int wrapT;
    bool techy = false;

    bool entered = false;
} wizard_t;
static wizard_t wizard;

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
    ImGui::Text("Selected MagFilter: %s", FilterToString(wizard.magfilter));
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
    ImGui::Text("Selected MinFilter: %s", FilterToString(wizard.minfilter));
    if (wizard.minfilter) {
        ImGui::SameLine();
        if (ImGui::Button("Clear Filter")) {
            wizard.minfilter = 0;
        }
    }

    if (ImGui::BeginMenu("GL_TEXTURE_WRAP_S")) {
        bool pressed = false;
        if (ImGui::Button("GL_REPEAT")) {
            wizard.wrapS = GL_REPEAT;
            pressed = true;
        }
        if (ImGui::Button("GL_CLAMP_TO_EDGE")) {
            wizard.wrapS = GL_CLAMP_TO_EDGE;
            pressed = true;
        }
        if (ImGui::Button("GL_CLAMP_TO_BORDER")) {
            wizard.wrapS = GL_CLAMP_TO_BORDER;
            pressed = true;
        }
        if (ImGui::Button("GL_MIRRORED_REPEAT")) {
            wizard.wrapS = GL_MIRRORED_REPEAT;
            pressed = true;
        }
        if (pressed) {
            Printf("Selected GL_TEXTURE_WRAP_S = %s", WrapToString(wizard.wrapS));
        }
        ImGui::EndMenu();
    }
    ImGui::Text("Selected WrapS: %s", WrapToString(wizard.wrapS));
    if (wizard.wrapS) {
        ImGui::SameLine();
        if (ImGui::Button("Clear WrapS")) {
            wizard.wrapS;
        }
    }

    if (ImGui::BeginMenu("GL_TEXTURE_WRAP_T")) {
        bool pressed = false;
        if (ImGui::Button("GL_REPEAT")) {
            wizard.wrapT = GL_REPEAT;
            pressed = true;
        }
        if (ImGui::Button("GL_CLAMP_TO_EDGE")) {
            wizard.wrapT = GL_CLAMP_TO_EDGE;
            pressed = true;
        }
        if (ImGui::Button("GL_CLAMP_TO_BORDER")) {
            wizard.wrapT = GL_CLAMP_TO_BORDER;
            pressed = true;
        }
        if (ImGui::Button("GL_MIRRORED_REPEAT")) {
            wizard.wrapT = GL_MIRRORED_REPEAT;
            pressed = true;
        }
        if (pressed) {
            Printf("Selected GL_TEXTURE_WRAP_T = %s", WrapToString(wizard.wrapT));
        }
        ImGui::EndMenu();
    }
    ImGui::Text("Selected WrapT: %s", WrapToString(wizard.wrapT));
    if (wizard.wrapT) {
        ImGui::SameLine();
        if (ImGui::Button("Clear WrapT")) {
            wizard.wrapT;
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

    wizard.wrapS = GL_REPEAT;
    wizard.wrapT = GL_REPEAT;

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
    ImGui::Text("Selected MagFilter: %s", FilterToString(wizard.magfilter));
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
    ImGui::Text("Selected MinFilter: %s", FilterToString(wizard.minfilter));
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
        ImGui::EndMenu();
    }
    ImGui::Text("Selected: %s", wizard.target == GL_TEXTURE_2D ? "No" : "Yes");
}

static void CheckExtension(void)
{
    if (N_stristr(wizard.name, TEXTURE_FILE_EXT) || COM_GetExtension(wizard.name) != NULL) {
        COM_StripExtension(wizard.name, wizard.name, sizeof(wizard.name));
    }
}

void CTextureManager::DrawWizard(const string_t& menuTitle)
{
    CTexture *curTool;
    if (!wizard.entered) {
        memset(&wizard, 0, sizeof(wizard));
        wizard.entered = true;
    }

    if (ImGui::BeginMenu(menuTitle.c_str())) {
        ImGui::InputText("Name", wizard.name, sizeof(wizard.name));
        if (wizard.texturePtr) {
            ImGui::MenuItem(wizard.texturePtr);
            if (ImGui::Button("Clear File")) {
                wizard.texturePtr = NULL;
            }
        }
        else if (ImGui::BeginMenu("Select File")) {
            if (!toolList.size()) {
                ImGui::MenuItem("No Texture Files in Data/");
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

        if (ImGui::Button("Default Parameters")) {
            Printf("Assigning default parameters to texture...");
            wizard.minfilter = GL_LINEAR;
            wizard.magfilter = GL_NEAREST;
            wizard.wrapS = GL_REPEAT;
            wizard.wrapT = GL_REPEAT;
            wizard.target = GL_TEXTURE_2D;
        }

        DrawWizardChoices();

        if (ImGui::Button("I'm A Techie")) {
            wizard.techy = true;
        }
        if (ImGui::Button("Create Texture") && wizard.texturePtr) {
            Printf("Creating texture with path '%s'", wizard.texturePtr);
            wizard.entered = false;
            toolList.try_emplace(wizard.texturePtr, Allocate<CTexture>());

            CheckExtension();
            
            curTool = toolList.at(wizard.texturePtr);
            curTool->SetName(wizard.name);
            curTool->SetParms(wizard.minfilter, wizard.magfilter, wizard.wrapS, wizard.wrapT, wizard.target);
            curTool->LoadImage(string_t(wizard.texturePtr));
            curTool->Save(string_t(BuildOSPath(Editor::GetPWD(), "Data", va("%s" TEXTURE_FILE_EXT, wizard.name))));

            lastCreated = curTool;

            memset(&wizard, 0, sizeof(wizard));
        }
        ImGui::EndMenu();
    }
}
