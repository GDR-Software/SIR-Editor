#include "gln.h"

CEditor *editor;
static CPopup *curPopup;

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

static void Draw_Widgets(void)
{
    vector_t<CWidget>& widgets = editor->mWidgets;

    for (auto& it : widgets) {
        if (!it.mActive) {
            continue;
        }
        ImGui::Begin(it.mName.c_str(), NULL, it.mFlags);
        it.mDrawFunc(eastl::addressof(it));
        ImGui::End();
    }
}

static void Draw_Popups(void)
{
    list_t<CPopup>& popups = editor->mPopups;

    if (!popups.size()) {
        return;
    }

    if (!curPopup) {
        curPopup = &popups.back();
    }

    ImGui::OpenPopup(curPopup->mName.c_str());

    if (ImGui::BeginPopupModal(curPopup->mName.c_str())) {
        ImGui::Text("%s", curPopup->mMsg.c_str());
        if (ImGui::Button("DONE")) {
            ImGui::CloseCurrentPopup();
            curPopup = NULL;
            popups.pop_back();
        }
        ImGui::EndPopup();
    }
}

CFileEntry *Draw_FileList(list_t<CFileEntry>& fileList)
{
    for (auto& it : fileList) {
        if (it.mIsDir && ImGui::BeginMenu(it.mPath.c_str())) {
            Draw_FileList(it.mDirList);
            ImGui::EndMenu();
        }
        else if (ImGui::MenuItem(it.mPath.c_str())) {
            return eastl::addressof(it);
        }
    }
    return NULL;
}

static void File_Menu(void)
{
    if (ImGui::BeginMenu("Project")) {
        if (ImGui::MenuItem("Open Project")) {
            OpenProject->mActive = true;
        }
        if (ImGui::MenuItem("New Project")) {

        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Open Recent")) {
        ImGui::EndMenu();
    }
}

static void Edit_Menu(void)
{
    if (ImGui::MenuItem("Preferences")) {
        Preferences->mActive = true;
    }
}

void CEditor::AddPopup(const CPopup& popup)
{ editor->mPopups.emplace_back(popup); }
CWidget *CEditor::PushWidget(const CWidget& widget)
{ return eastl::addressof(editor->mWidgets.emplace_back(widget)); }

void CEditor::Draw(void)
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            File_Menu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            Edit_Menu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Build")) {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    Draw_Popups();
    Draw_Widgets();
}