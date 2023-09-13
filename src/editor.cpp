#include "gln.h"

CEditor *editor;

CEditor::CEditor(void)
    : mConsoleActive{ false }
{
    mMenu_File = GUI_PushMainMenu_Child("File");
    mMenu_Edit = GUI_PushMainMenu_Child("Edit");

    {
        GUI_PushMenu_Child(mMenu_File, "Project");
        GUI_PushMenu_Child(mMenu_File, "Open Recent");
        GUI_PushMenu_Child(mMenu_File, "Save");
    }

    ReloadFileCache();

    mConfig = new CGameConfig;
}

static void RecursiveDirectoryIterator(const std::filesystem::path& path, std::list<CFileEntry>& fileList)
{
    for (const auto& it : std::filesystem::directory_iterator{ editor->mConfig->mEditorPath.GetBuffer() }) {
        fileList.emplace_back(it.path(), it.is_directory());
        if (it.is_directory()) {
            RecursiveDirectoryIterator(it.path(), fileList.back().mDirList);
        }
    }
}

void CEditor::ReloadFileCache(void)
{
    Printf("[CEditor::ReloadFileCache] reloading file cache...");
    RecursiveDirectoryIterator(mConfig->mEditorPath.GetBuffer(), mFileCache);
}

void CEditor::Draw(void)
{
}
