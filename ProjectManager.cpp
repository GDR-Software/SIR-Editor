#include "Common.hpp"
#include "Editor.h"
#include "ProjectManager.h"

bool CProjectManager::LoadList(void)
{
    eastl::vector<eastl::string> fileList;

    Editor::ListFiles(fileList, Editor::GetPWD(), {".proj"});

    for (const auto& it : fileList) {
        toolList.try_emplace(it);
        if (!toolList[it].Load(it)) {
            return false;
        }
    }
}

void CProjectManager::ClearList(void)
{
}

void CProjectManager::Draw(void)
{
    ImVec2 windowSize, windowPos;

    Editor::GetWindowParms(windowPos, windowSize);
    Editor::SetWindowParms();
    if (ImGui::BeginMenu("Projects")) {
        for (auto it = toolList.begin(); it != toolList.end(); ++it) {
            if (ImGui::MenuItem(it->first.c_str())) {
                curProject = &it->second;
            }
        }
        ImGui::EndMenu();
    }
    Editor::ResetWindowParms(windowPos, windowSize);
}

void CProjectManager::DrawWizard(void)
{
    ImGui::Begin("Project Wizard", NULL, ImGuiWindowFlags_NoResize);
    ImGui::End();
}