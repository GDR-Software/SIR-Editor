#include "gln.h"

CEditor *editor;
static CPopup *curPopup;

static bool Draw_Directory(CFileEntry *entry);

static inline bool ItemWithTooltip(const char *item_name, const char *fmt, ...)
{
    const bool pressed = ImGui::MenuItem(item_name);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        va_list argptr;

        va_start(argptr, fmt);
        ImGui::SetTooltipV(fmt, argptr);
        va_end(argptr);
    }
    return pressed;
}

static inline bool ButtonWithTooltip(const char *item_name, const char *fmt, ...)
{
    const bool pressed = ImGui::Button(item_name);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        va_list argptr;

        va_start(argptr, fmt);
        ImGui::SetTooltipV(fmt, argptr);
        va_end(argptr);
    }
    return pressed;
}

static inline bool MenuWithTooltip(const char *item_name, const char *fmt, ...)
{
    const bool pressed = ImGui::BeginMenu(item_name);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        va_list argptr;

        va_start(argptr, fmt);
        ImGui::SetTooltipV(fmt, argptr);
        va_end(argptr);
    }
    return pressed;
}

CEditor::CEditor(void)
    : mConsoleActive{ false }
{
    mConfig = new CGameConfig;
}

/*
RecursiveDirectoryIterator: iterates through the given and all the subdirectories
*/
static void RecursiveDirectoryIterator(const std::filesystem::path& path, std::vector<CFileEntry>& fileList)
{
    if (!std::filesystem::directory_entry{ path }.is_directory()) {
        return;
    }
    for (const auto& it : std::filesystem::directory_iterator{ path }) {
        fileList.emplace_back(it.path(), it.is_directory());
        if (it.is_directory()) {
            RecursiveDirectoryIterator(it.path(), fileList);
        }
    }
}

/*
DirectoryIterator: iterates through the given path, does not recurse into subdirectories
*/
static void DirectoryIterator(const std::filesystem::path& path, std::vector<CFileEntry>& fileList)
{
    if (!std::filesystem::directory_entry{ path }.is_directory()) {
        return;
    }
    for (const auto& it : std::filesystem::directory_iterator{ path }) {
        fileList.emplace_back(it.path(), it.is_directory());
    }
}

void CEditor::ReloadFileCache(void)
{
    Printf("[CEditor::ReloadFileCache] reloading file cache...");
    RecursiveDirectoryIterator(mConfig->mEditorPath.c_str(), mFileCache);
}

static void Draw_Widgets(void)
{
    std::vector<CWidget>& widgets = editor->mWidgets;

    for (auto& it : widgets) {
        if (it.mActive) {
            it.mActive = ImGui::Begin(it.mName.c_str(), &it.mActive, it.mFlags);
            it.mDrawFunc(std::addressof(it));
            ImGui::End();
        }
    }
}

static void Draw_Popups(void)
{
    std::list<CPopup>& popups = editor->mPopups;

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

CFileEntry *Draw_FileList(std::vector<CFileEntry>& fileList)
{
    for (auto& it : fileList) {
        if (it.mIsDir && ImGui::BeginMenu(it.mPath.c_str())) {
            Draw_FileList(it.mDirList);
            ImGui::EndMenu();
        }
        else if (ImGui::MenuItem(it.mPath.c_str())) {
            return std::addressof(it);
        }
    }
    return NULL;
}

static void File_Menu(void)
{
    if (ImGui::BeginMenu("Project")) {
        if (ItemWithTooltip("Open Project", "Open an already made project")) {

        }
        if (ItemWithTooltip("New Project", "Create a new project")) {

        }
        if (ItemWithTooltip("Save Project", "Save your current project")) {
            
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Open Recent")) {
        ImGui::EndMenu();
    }
}

static void Edit_Preferences(void);
static void Edit_Project(void);
static void Edit_Menu(void)
{
    if (ImGui::CollapsingHeader("Preferences")) {
        Edit_Preferences();
    }
}

static void Build_Menu(void)
{
    if (ItemWithTooltip("Build Map", "Save the current map in text-based format into a .map file")) {

    }
    if (ItemWithTooltip("Compile Map", "Compile a .map file into a .bmf file,\nNOTE: .bmf files cannot be used in the map editor")) {

    }
}

void CEditor::AddPopup(const CPopup& popup)
{ editor->mPopups.emplace_back(popup); }
CWidget *CEditor::PushWidget(const CWidget& widget)
{ return std::addressof(editor->mWidgets.emplace_back(widget)); }

static CFileEntry enginePath{std::string(pwdString.string() + "Data/glnomad" EXE_EXT).c_str(), false};
static CFileEntry exePath{std::string(pwdString.string() + "Data/").c_str(), true};
static bool exePathChanged = false;
static bool enginePathChanged = false;

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
            Build_Menu();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    Draw_Popups();
    Draw_Widgets();

    if (ImGuiFileDialog::Instance()->IsOpened("SelectEnginePathDlg")) {
        if (ImGuiFileDialog::Instance()->Display("SelectEnginePathDlg")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                enginePath.mPath = ImGuiFileDialog::Instance()->GetFilePathName();
                enginePathChanged = true;
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
    if (ImGuiFileDialog::Instance()->IsOpened("SelectExePathDlg")) {
        if (ImGuiFileDialog::Instance()->Display("SelectExePathDlg")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                exePath.mPath = ImGuiFileDialog::Instance()->GetFilePathName();
                exePathChanged = true;
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
}

static bool techy = false, changed = false;
static bool textureDetailsChanged = false;
static bool textureFiltersChanged = false;
static int textureDetails, textureFilters;

static void Edit_Preferences(void)
{
    ImGui::SeparatorText("Configuration");
    if (ButtonWithTooltip(va("Engine Path: %s", editor->mConfig->mEnginePath.c_str()), "Set the editor's path to the game engine")) {
        ImGuiFileDialog::Instance()->OpenDialog("SelectEnginePathDlg", "Select File", ".*", editor->mConfig->mEditorPath);
    }
    if (ButtonWithTooltip(va("Executable Path: %s", editor->mConfig->mExecutablePath.c_str()), "Set the editor's path to executables")) {
        ImGuiFileDialog::Instance()->OpenDialog("SelectExePathDlg", "Select Folder", "/", editor->mConfig->mEditorPath);
    }

    ImGui::SeparatorText("Graphics");
    if (ImGui::BeginMenu("Texture Detail")) {
        for (uint64_t i = 0; i < arraylen(texture_details); i++) {
            if (ImGui::MenuItem(texture_details[i].s)) {
                textureDetailsChanged = true;
                changed = true;
                textureDetails = texture_details[i].i;
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Texture Filtering")) {
        if (!techy && ImGui::Button("I'm A Techie")) {
            techy = true;
        }
        else if (techy && ImGui::Button("I'm Not A Techie")) {
            techy = false;
        }
        if (techy) {
            ImGui::Text("Mag Filter | Min Filter | Filter Name");
            ImGui::Text("-----------|------------|------------");
        }
        for (uint64_t i = 0; i < arraylen(texture_filters); i++) {
            if (ImGui::MenuItem(techy ? texture_filters_alt[i].s : texture_filters[i].s)) {
                textureFiltersChanged = true;
                changed = true;
                textureFilters = texture_filters[i].i;
            }
        }
        ImGui::EndMenu();
    }
    
    if (changed) {
        if (ImGui::Button("Save Preferences")) {
            if (enginePathChanged) {
                editor->mConfig->mEnginePath = enginePath.mPath;
                enginePathChanged = false;
            }
            if (exePathChanged) {
                editor->mConfig->mExecutablePath = exePath.mPath;
                exePathChanged = false;
            }
            if (textureDetailsChanged) {
                editor->mConfig->mTextureDetail = texture_details[textureDetails].i;
                textureDetailsChanged = false;
            }
            if (textureFiltersChanged) {
                editor->mConfig->mTextureFiltering = texture_filters[textureFilters].i;
                textureFiltersChanged = false;
            }
            changed = false;
            editor->mConfig->mPrefs.SavePrefs();
        }
    }
}
