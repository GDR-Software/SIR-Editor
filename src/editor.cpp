#include "gln.h"

CEditor *editor;

CEditor::CEditor(void)
    : mConsoleActive{ false }
{
    mConfig = Allocate<CGameConfig>();
}

static void RecursiveDirectoryIterator(const std::filesystem::path& path, list_t<CFileEntry>& fileList)
{
    for (const auto& it : std::filesystem::directory_iterator{ editor->mConfig->mEditorPath.c_str() }) {
        fileList.emplace_back(it.path(), it.is_directory());
        if (it.is_directory()) {
            RecursiveDirectoryIterator(it.path(), fileList.back().mDirList);
        }
    }
}

void CEditor::ReloadFileCache(void)
{
    Printf("[CEditor::ReloadFileCache] reloading file cache...");
    RecursiveDirectoryIterator(mConfig->mEditorPath.c_str(), mFileCache);
}

static void File_OpenRecent(void)
{
    if (ImGui::BeginMenu("Open Recent")) {
        if (ImGui::MenuItem("Maps")) {

        }
        ImGui::EndMenu();
    }
}

void CEditor::Draw(void)
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            File_OpenRecent();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
