#include "Common.hpp"
#include "ProjectManager.h"
#include "Project.h"
#include "Editor.h"
#include <glad/glad.h>
#include <execinfo.h>

#define DEFAULT_GAMEPATH "Data"

object_ptr_t<Editor> Editor::editor;
path_t Editor::curPath;

void assert_failure(const char *expr, const char *file, const char *func, unsigned line)
{
    Error(
        "Error: ImGui Assertion Failed:\n"
        "\tFile: %s\n"
        "\tFunction: %s\n"
        "\tLine: %u\n"
        "\tExpression: %s",
    file, func, line, expr);
}

static void SetCompression_f(void)
{
    const char *str;

    if (Argc() < 2) {
        Printf(
            "usage: setcompression <compression library>\n"
            "\toptions: bzip2, zlib (default)");
        return;
    }

    str = Argv(1);
    if (!N_stricmp(str, "bzip2")) {
        parm_compression = COMPRESS_BZIP2;
        Printf("Set compression to bzip2");
    }
    else if (!N_stricmp(str, "zlib")) {
        parm_compression = COMPRESS_ZLIB;
        Printf("Set compression to zlib");
    }
    else {
        Printf("Bad compression library option %s", str);
    }
}

bool Editor::IsAllocated(void)
{
    if (!editor)
        return false;
    if (!editor->cGUI)
        return false;
    
    return true;
}

Editor::Editor(void)
{
    int parm;
    const char *load;

    parm = GetParm("-debug");
    if (parm != -1)
        spdlog::set_level(spdlog::level::trace);
    else
        spdlog::set_level(spdlog::level::info);

    parm = GetParm("-load");
    if (parm != -1)
        load = myargv[parm];
    else
        load = NULL;
    
    Cmd_Init();

    parm_saveJsonMaps = GetParm("-jmap") != -1;
    parm_saveJsonTilesets = GetParm("-jtileset") != -1;
    parm_useInternalMaps = GetParm("-imap") != -1;
    parm_useInternalTilesets = GetParm("-itileset") != -1;

    parm = GetParm("-compression");
    if (parm != -1) {
        const char *s = myargv[parm + 1];

        if (!N_stricmp(s, "bzip2"))
            parm_compression = COMPRESS_BZIP2;
        else if (!N_stricmp(s, "zlib"))
            parm_compression = COMPRESS_ZLIB;
        else {
            Printf("Bad parameter for compression: %s", s);
            s = "zlib";
            parm_compression = COMPRESS_ZLIB;
        }
        
        Printf("Using compression library %s", s);
    }
    else {
        parm_compression = COMPRESS_ZLIB;
        Printf("Using compression library zlib");
    }

    Cmd_AddCommand("reloadfiles", this->ReloadFiles);
    Cmd_AddCommand("setcompression", SetCompression_f);
    
    cGUI = Allocate<GUI>();
    curPath = std::filesystem::current_path();
    ReloadFiles();
    
    cGUI->Init("GLNomad Level Editor", 1920, 1080);
    
    cProjectManager = AddManager<CProjectManager>("ProjectManager");
    cMapManager = AddManager<CMapManager>("MapManager");
    cTilesetManager = AddManager<CTilesetManager>("TilesetManager");
    cTextureManager = AddManager<CTextureManager>("TextureManager");

    cTileset = Allocate<CTileset>();
    cMap = Allocate<CMap>();
    cProject = Allocate<CProject>();

    cTileset->ClearName();
    cMap->ClearName();
    cProject->ClearName();

    cProjectManager->LoadList();
    cMapManager->LoadList();
    cTilesetManager->LoadList();
    cTextureManager->LoadList();

//    if (load)
//        cProjectManager->SetCurrent(load);

    LoadPreferences();
}

Editor::~Editor()
{
    Cmd_Shutdown();
}

void Editor::CheckExtension(path_t& path, const char *ext)
{
    const char *_ext;

    _ext = COM_GetExtension(path.c_str());
    if (!_ext || (_ext && N_stricmp(_ext, ext) != 0)) {
        path.append(ext);
    }
}

bool Editor::SaveJSON(const json& data, const path_t& path)
{
    const char *rpath;

    rpath = path.c_str();

    std::ofstream file(rpath, std::ios::out);
    if (file.fail()) {
        Printf("Editor::SaveJSON: failed to save json to '%s', std::ofstream failed", rpath);
        return false;
    }
    file << data;
    file.close();
    
    return true;
}

void Editor::InitFiles(void)
{
    ReloadFiles();
}

bool Editor::LoadJSON(json& data, const path_t& path)
{
    const char *rpath;
    FILE *fp;

    rpath = strdupa(path.c_str());
    if (IsAbsolutePath(path.c_str())) {
        rpath = BuildOSPath(curPath, "Data/", path.c_str());
    }

    if (!FileExists(rpath)) {
        Printf("Editor::LoadJSON: bad json path '%s', file does not exist", rpath);
        return false;
    }
    
    fp = SafeOpenRead(rpath);
    try {
        data = json::parse(fp);
    } catch (const json::exception& e) {
        Printf("JSON parse failure, nlohmann::json::exception =>\n\tid: %i\n\twhat: %s", e.id, e.what());
        fclose(fp);
        return false;
    }
    fclose(fp);

    return true;
}

void Editor::LoadPreferences(void)
{
    json data;

    Printf("Loading prefences file...");

    if (!LoadJSON(data, "preferences.json")) {
        Printf("WARNING: failed to load preferences.json");
        return;
    }

    cGUI->getCamera().moveSpeed = data["camera"]["movespeed"];
    cGUI->getCamera().rotationSpeed = data["camera"]["rotationspeed"];
    cGUI->getCamera().zoomSpeed = data["camera"]["zoomspeed"];
}

void Editor::SavePreferences(void)
{
    json data;

    Printf("Saving preferences file...");

    data["camera"]["movespeed"] = cGUI->getCamera().moveSpeed;
    data["camera"]["rotationspeed"] = cGUI->getCamera().rotationSpeed;
    data["camera"]["zoomspeed"] = cGUI->getCamera().zoomSpeed;

    std::ofstream file("Data/preferences.json", std::ios::out);
    if (file.fail()) {
        Error("Failed to open perferences file in write mode");
    }
    file << data;
    file.close();
}

void Editor::ListFiles(vector_t<path_t>& fileList, const path_t& path, const vector_t<const char *>& exts)
{
    auto isExt = [&](const char *str) {
        for (const auto *it : exts) {
            if (N_stristr(str, it)) {
                return true;
            }
        }
        return false;
    };

    for (const FileEntry& it : editor->fileCache) {
        if (it.isDirectory) {
            ListFiles(fileList, it.path, exts);
        }
        else if (isExt(it.path.c_str()) && !it.isDirectory) {
            fileList.emplace_back(it.path);
        }
    }
}

void Editor::Init(int argc, char **argv)
{
    myargc = argc;
    myargv = argv;

#ifdef USE_ZONE
    Z_Init();
    atexit(Z_Shutdown);

    editor = (Editor *)Z_Malloc(sizeof(*editor), TAG_STATIC, &editor, "editor");
#else
    Mem_Init();
    atexit(Mem_Shutdown);

    editor = (Editor *)Mem_Alloc(sizeof(*editor));
#endif
    ::new (editor) Editor();
}

static const char *SpawnEntityToString(uint32_t entity)
{
    switch (entity) {
    case ENTITY_ITEM: return "Item";
    case ENTITY_MOB: return "Mob";
    case ENTITY_PLAYR: return "Player";
    case ENTITY_WEAPON: return "Weapon";
    };
    Printf("ERROR: Bad entity type: %i", entity);
}

static uint32_t SpawnStringToEntity(const char *s)
{
    if (!N_stricmp(s, "Item")) return ENTITY_ITEM;
    else if (!N_stricmp(s, "Weapon")) return ENTITY_WEAPON;
    else if (!N_stricmp(s, "Player")) return ENTITY_PLAYR;
    else if (!N_stricmp(s, "Mob")) return ENTITY_MOB;
    Printf("ERROR: Bad entity string: %s", s);
}

static void DirectoryIterate(const path_t& curPath)
{
    for (const auto& entry : directory_iterator_t{curPath}) {
        if (entry.is_directory()) {
            if (ImGui::BeginMenu(entry.path().c_str())) {
                DirectoryIterate(entry.path());
                ImGui::EndMenu();
            }
        }
        else {
            if (ImGui::MenuItem(entry.path().c_str())) {

            }
        }
    }
}

void Editor::RecursiveDirectoryIterator(const path_t& path, vector_t<FileEntry>& dirList, uint32_t& depth)
{
    for (const auto& it : directory_iterator_t{path}) {
        dirList.emplace_back();
        FileEntry& entry = dirList.back();
        entry.path = it.path();

        if (it.is_directory()) {
            entry.isDirectory = true;
            RecursiveDirectoryIterator(it.path(), entry.DirList, depth);
        }
        else if (!it.is_directory()) {
            entry.isDirectory = false;
        }
    }
}

void Editor::DisplayFileCache(const vector_t<FileEntry>& cache, uint32_t& depth)
{
    for (const auto& it : cache) {
        if (it.isDirectory) {
            depth++;
            printf("|");
            for (uint32_t i= 0; i < depth; i++) {
                printf("-");
            }
            printf(" %s/\n", GetAbsolutePath(it.path));
            DisplayFileCache(cache, depth);
        }
        else if (!it.isDirectory) {
            printf("|");
            for (uint32_t i= 0; i < depth; i++) {
                printf("-");
            }
            printf(" %s\n", GetAbsolutePath(it.path));
        }
    }
}

void Editor::ReloadFiles(void)
{
    uint32_t depth;

    Printf("Reloading file cache...");

    depth = 0;
    fileCache.clear();
    string_t path = string_t(curPath.c_str()) + "/Data/";
    RecursiveDirectoryIterator(path_t(path.c_str()), fileCache, depth);

#ifndef NDEBUG
    depth = 0;
    DisplayFileCache(fileCache, depth);
#endif
}

static bool exiting = false;
void Editor::DrawFileMenu(void)
{
    if (ImGui::MenuItem("New Project", "Ctrl+N")) {
        cProjectManager->NewProject();
    }
    if (ImGui::BeginMenu("Open Project")) {
        setModeBits(EDITOR_WIDGET);
        cProjectManager->SaveCurrent();
        ImGui::EndMenu();
        clearModeBits(EDITOR_WIDGET);
    }
    if (ImGui::BeginMenu("Open Recent")) {
        setModeBits(EDITOR_WIDGET);
        for (const auto& it : managerList) {
            if (it.second->HasOpenRecent()) {
                it.second->DrawRecent();
            }
        }
        ImGui::EndMenu();
        clearModeBits(EDITOR_WIDGET);
    }
    if (ImGui::MenuItem("Save Project", "Ctrl+S")) {
        cProjectManager->SaveCurrent();

        const uint64_t size = cProjectManager->GetCurrentName().size() + strlen("Saved Project ") + 1;
        char *saveString = (char *)alloca(size);
        memset(saveString, 0, size);
        snprintf(saveString, size, "Saved Project %s", cProjectManager->GetCurrentName().c_str());
        Printf("%s", saveString);

        if (ImGui::BeginPopup(saveString)) {
            ImGui::EndPopup();
        }
    }
    if (ImGui::MenuItem("Exit", "Alt+F4")) {
        setModeBits(EDITOR_WIDGET);
        exiting = cProjectManager->GetCurrentModified();
        if (exiting) {
            ImGui::Begin("Save Project?");
            if (ImGui::Button("Yes")) {
                cProjectManager->SaveCurrent();
                Exit();
            }
            ImGui::End();
        }
        setModeBits(EDITOR_WIDGET);
    }
}

void Editor::DrawPopups(void)
{
    bool open;

    if (!popups.size())
        return;
    
    if (!curPopup) {
        curPopup = popups.end() - 1;
    }

    open = ImGui::Begin("Popup", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
    if (!open) {
        popups.pop_back();
        ImGui::End();
        return;
    }
    ImGui::BeginPopup(curPopup->title);
    ImGui::Text(curPopup->message);
    if (ImGui::Button(curPopup->confirmation)) {
        *curPopup->yesno = true;
    }
    ImGui::EndPopup();
    ImGui::End();
}

void Editor::DrawWizardMenu(void)
{
    char wizardname[MAX_GDR_PATH];
    
    for (const auto& it : managerList) {
        if (it.second->HasWizard()) {
            N_strncpyz(wizardname, va("%s Wizard", it.second->GetWizardName()), sizeof(wizardname));
            it.second->DrawWizard(wizardname);
        }
    }
}

void Editor::Redo(void)
{
}

void Editor::Undo(void)
{
}

void Editor::DrawEditMenu(void)
{
}

void Editor::PollCommands(void)
{
    const char *inputBuf;
    inputBuf = cGUI->getImGuiInput();
    Cmd_Execute(inputBuf);
}

void Editor::DrawWidgets(void)
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            DrawFileMenu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            DrawEditMenu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Wizard")) {
            DrawWizardMenu();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    DrawPopups();
}

void Editor::run(void)
{
	while (1) {
		cGUI->BeginFrame();
		cGUI->PollEvents(this);
		
        PollCommands();
        DrawWidgets();
		
		cGUI->EndFrame();
	}
}