#include "gln.h"

CWidget *OpenProject;
CWidget *Preferences;

static void File_OpenProject(CWidget *self);
static void Edit_Preferences(CWidget *self);

int main(int argc, char **argv)
{
    Mem_Init();
    atexit(Mem_Shutdown);

    gui = Allocate<Window>();
    editor = Allocate<CEditor>();

    editor->ReloadFileCache();

    OpenProject = CEditor::PushWidget({ "Open Recent", File_OpenProject, ImGuiWindowFlags_MenuBar });
    Preferences = CEditor::PushWidget({ "Preferences", Edit_Preferences, ImGuiWindowFlags_MenuBar });

    while (1) {
        gui->BeginFrame();
        editor->Draw();

        gui->EndFrame();
    }

    // never reached
    return 0;
}


static void File_OpenProject(CWidget *self)
{
    CFileEntry *project;

    if (ImGui::BeginMenuBar()) {
        if (ImGui::MenuItem("Cancel")) {
            self->mActive = false;
        }
        if (ImGui::MenuItem("Open")) {
            self->mActive = false;
        }
        ImGui::EndMenuBar();
    }

    project = Draw_FileList(editor->mFileCache);
    if (!project) {
        return;
    }
}

static constexpr const char *texture_details[] = {
    "GPU vs God",
    "Expensive Shit We've Got Here",
    "Xtreme",
    "Normie",
    "Integrated GPU",
    "MS-DOS"
};

static constexpr const char *texture_filters[] = {
    "Nearest",
    "Linear",
    "Bilinear",
    "Trilinear"
};

static std::filesystem::path curDir;
static void Draw_Directory(CFileEntry *entry)
{
    for (const auto& it : std::filesystem::directory_iterator{ curDir }) {
        if (it.is_directory()) {
            if (ImGui::BeginMenu(it.path().c_str())) {
                curDir = it.path();
                ImGui::EndMenu();
                Draw_Directory(entry);
            }
        }
        else {
            if (ImGui::MenuItem(it.path().c_str())) {
                entry->mPath = it.path().c_str();
            }
        }
    }
}

static void Edit_Preferences(CWidget *self)
{
    bool changed;
    ImGui::SetWindowSize(ImVec2( 1080, 720 ));
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Configuration")) {
            if (ImGui::BeginMenu("Engine Path")) {
                CFileEntry entry{"", false};
                curDir = editor->mConfig->mEditorPath.c_str();
                Draw_Directory(&entry);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Executable Path")) {
                CFileEntry entry{"", false};
                curDir = editor->mConfig->mEditorPath.c_str();
                Draw_Directory(&entry);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Graphics")) {
            changed = ImGui::ListBox("Texture Detail", &editor->mConfig->mTextureDetail, texture_details, arraylen(texture_details));
            changed = ImGui::ListBox("Texture Filtering", &editor->mConfig->mTextureFiltering, texture_filters, arraylen(texture_filters));
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Camera")) {
            changed = ImGui::SliderFloat("Move Speed", &editor->mConfig->mCameraMoveSpeed, 0.25f, 15.0f);
            changed = ImGui::SliderFloat("Rotation Speed", &editor->mConfig->mCameraRotationSpeed, 0.25f, 15.0f);
            changed = ImGui::SliderFloat("Zoom Speed", &editor->mConfig->mCameraZoomSpeed, 0.25f, 15.0f);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    if (changed) {
        if (ImGui::Button("Save Preferences")) {
            editor->mConfig->mPrefs.SavePrefs();
        }
    }
}