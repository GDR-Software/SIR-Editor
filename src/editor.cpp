#include "gln.h"

std::unique_ptr<CEditor> editor;
static CPopup *curPopup;

static bool Draw_Directory(CFileEntry *entry);

static void ClearMap_f(void)
{
    mapData->Clear();
}

static void NewMap_f(void)
{
    Map_New();
}

static void Save_f(void)
{
    Map_Save(mapData->mName.c_str());
}

static void SaveAll_f(void)
{
    Map_Save(mapData->mName.c_str());
}

static void MapInfo_f(void)
{
    uint64_t i;

    Printf("---------- Map Info ----------");
    Printf("Name: %s", mapData->mName.c_str());
    Printf("Width: %u", mapData->mWidth);
    Printf("Height: %u", mapData->mHeight);
    Printf("Number of Checkpoints: %lu", mapData->mCheckpoints.size());
    Printf("Number of Spawns: %lu", mapData->mSpawns.size());

    for (i = 0; i < mapData->mCheckpoints.size(); ++i) {
        Printf("\n[Checkpoint %lu]", i);
        Printf("Position: [%u, %u, %u]",
            mapData->mCheckpoints[i].xyz[0],
            mapData->mCheckpoints[i].xyz[1],
            mapData->mCheckpoints[i].xyz[2]);
    }

    for (i = 0; i < mapData->mSpawns.size(); ++i) {
        Printf("\n[Spawn %lu]", i);
        Printf("Position: [%u, %u, %u]",
            mapData->mSpawns[i].xyz[0],
            mapData->mSpawns[i].xyz[1],
            mapData->mSpawns[i].xyz[2]);
        Printf("Entity Type: %u", mapData->mSpawns[i].entitytype);
        Printf("Entity Id: %u", mapData->mSpawns[i].entityid);
    }
}

CEditor::CEditor(void)
    : mConsoleActive{ false }
{
    mode = MODE_MAP;

    Cmd_AddCommand("clearMap", ClearMap_f);
    Cmd_AddCommand("newMap", NewMap_f);
    Cmd_AddCommand("save", Save_f);
    Cmd_AddCommand("saveAll", SaveAll_f);
    Cmd_AddCommand("mapinfo", MapInfo_f);
}

bool CEditor::ValidateEntityId(uint32_t id) const
{
    for (const auto& it : gameConfig->mMobList) {
        if (it.mId == id) {
            return true;
        }
    }
    return false;
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
    RecursiveDirectoryIterator(gameConfig->mEditorPath.c_str(), mFileCache);
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

void CEditor::AddPopup(const CPopup& popup)
{ editor->mPopups.emplace_back(popup); }

void CEditor::Draw(void)
{
    if (Key_IsDown(KEY_LCTRL)) {
        // change editor mode
        if (Key_IsDown(KEY_M)) {
            mode = MODE_MAP;
        }
        else if (Key_IsDown(KEY_E)) {
            mode = MODE_EDIT;
        }
        else if (Key_IsDown(KEY_T)) {
            mode = MODE_TILE;
            gui->InitTileMode();
        }

        // quick-commands
        if (Key_IsDown(KEY_SLASH)) {
            Cmd_ExecuteText("cameraCenter");
        }
        if (ImGui::IsKeyPressed(ImGuiKey_LeftShift, false) && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
            Cmd_ExecuteText("saveAll");
        }
        if (ImGui::IsKeyPressed(ImGuiKey_S, false)) {
            Cmd_ExecuteText("save");
        }
    }

    Draw_Popups();
    Widgets_Draw();
}
