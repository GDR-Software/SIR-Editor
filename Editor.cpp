#include "Common.hpp"
#include "Editor.h"
#include <glad/glad.h>

#define DEFAULT_GAMEPATH "Data"

eastl::unique_ptr<Editor> Editor::editor;

Editor::Editor()
{
    int parm;

    parm = GetParm("-loglevel");

    InitFiles();

    if (parm != -1) {
        spdlog::set_level(spdlog::level::trace);
    }
    else {
        spdlog::set_level(spdlog::level::info);
    }
    
    parm = GetParm("-gamepath");
    if (parm != -1) {
        gamepath = myargv[parm];
    }
    else {
        gamepath = DEFAULT_GAMEPATH;
    }

    cProject = eastl::make_unique<Project>();
    cMap = cProject->mapData;
    cMap->tileset = cProject->tileset;
    cGUI = eastl::make_unique<GUI>();
}

Editor::~Editor()
{
	Command *cmd, *next;
	for (cmd = cmdList; cmd; cmd = next) {
		next = cmd->getNext();
		delete cmd;
	}
}

void Editor::LoadPreferences(void)
{
    FILE *fp;
    json data;

    Printf("Loading prefences file...");

    fp = SafeOpenRead("Data/preferences.json");
    try {
        data = json::parse(fp);
    } catch (const json::exception& e) {
        Printf("Failed to load json, nlohmann::json::exception =>\n\tid: %i\n\twhat(): %s", e.id, e.what());
        fclose(fp);
        return;
    }
    fclose(fp);

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

/*
Editor::SaveFileCache: saves the editor's file list to a json cache file
*/
void Editor::SaveFileCache(void)
{
    json data;

    Printf("Saving json file cache...");

    std::ofstream file("Data/filecache.json", std::ios::out);
    if (file.fail()) {
        Error("Failed to open file cache in write mode");
    }

    file << data;
}

static void InitFilesList(eastl::vector<eastl::string>& list, const std::filesystem::path& path, const eastl::vector<eastl::string>& exts)
{
    auto isExt = [&](const char *str) {
        for (const auto& it : exts) {
            if (strstr(str, it.c_str()) != NULL) {
                return true;
            }
        }
        return false;
    };

    for (const auto& i : std::filesystem::directory_iterator{path}) {
        if (i.is_directory()) {
            InitFilesList(list, i.path(), exts);
        }
        else if (isExt(i.path().c_str()) && !i.is_directory()) {
            list.emplace_back(i.path().c_str());
        }
    }
}

void Editor::InitFiles(void)
{
    curPath = std::filesystem::current_path();

    textureFiles.reserve(64);
    recentFiles.reserve(64);

    InitFilesList(textureFiles, curPath, {".png", ".jpg", ".jpeg", ".bmp"});
    InitFilesList(recentFiles, curPath, {".nlp"});

//    SaveFileCache();
}

void Editor::Init(int argc, char **argv)
{
    myargc = argc;
    myargv = argv;

    editor = eastl::make_unique<Editor>();
    editor->cGUI->Init("GLNomad Level Editor", 1920, 1080);
}

Command *Editor::findCommand(const char *name)
{
	Command *cmd;
	for (cmd = cmdList; cmd; cmd = cmd->getNext()) {
		if (!N_stricmp(cmd->getName().c_str(), name)) {
			return cmd;
		}
	}
	return NULL;
}

void Editor::registerCommand(const char *name, void (*func)(void))
{
	Command *cmd;
	
	if (findCommand(name)) {
		Printf("Command '%s' already registered", name);
	}
	
	cmd = new Command(name, func);
	cmd->setNext(cmdList);
	cmdList = cmd;
}

void Editor::PollCommands(void)
{
	const char *inputBuf;
	Command *cmd;
	
	inputBuf = cGUI->getImGuiInput();
	
	if (inputBuf[0] != '/' && inputBuf[0] != '\\') {
		return;
	}
	else {
		inputBuf++;
		cmd = findCommand(inputBuf);
		if (!cmd) {
			Printf("No such command '%s'", inputBuf);
		}
		cmd->Run();
	}
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

void Editor::NewProject(void)
{
    // set the default name
    cMap->name = "untitled-map.map";

    // clear out the old map
    cMap->Clear();

    cProject->name = "untitled-project";
    cProject->setPath(cProject->name);

    cProject->modified = true;
}

static void DirectoryIterate(const std::filesystem::path& curPath)
{
    for (const auto& entry : std::filesystem::directory_iterator(curPath)) {
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

void Editor::OpenProject(void)
{
    cProject->modified = true;
    DirectoryIterate(curPath);
}

static bool exiting = false;
void Editor::DrawFileMenu(void)
{
    if (ImGui::MenuItem("New Project", "Ctrl+N")) {
        NewProject();
    }
    if (ImGui::BeginMenu("Open Project")) {
        setModeBits(EDITOR_WIDGET);
        OpenProject();
        ImGui::EndMenu();
        clearModeBits(EDITOR_WIDGET);
    }
    if (ImGui::BeginMenu("Open Recent")) {
        setModeBits(EDITOR_WIDGET);
        if (!recentFiles.size()) {
            ImGui::MenuItem("No Recent Files");
        }
        else {
            for (const auto& i : recentFiles) {
                if (ImGui::MenuItem(i.c_str())) {
                    cProject->name = i;
                    cProject->Load(i);
                }
            }
        }
        ImGui::EndMenu();
        clearModeBits(EDITOR_WIDGET);
    }
    if (ImGui::MenuItem("Save Project", "Ctrl+S")) {
        cProject->Save();
        eastl::string saveString = eastl::string("Saved Project ") + cProject->name;
        if (ImGui::BeginPopup(saveString.c_str())) {
            ImGui::EndPopup();
        }
    }
    if (ImGui::MenuItem("Exit", "Alt+F4")) {
        setModeBits(EDITOR_WIDGET);
        exiting = cProject->modified;
        if (exiting) {
            ImGui::Begin("Save Project?");
            if (ImGui::Button("Yes")) {
                cProject->Save();
                Exit();
            }
            else {
                exiting = false;
            }
            ImGui::End();
        }
        else {
            Exit();
        }
        setModeBits(EDITOR_WIDGET);
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
    char name[1024];
    memset(name, 0, sizeof(name));

    if (ImGui::BeginMenu("Map")) {
        if (ImGui::InputInt("Set Width", &cMap->width)) {
            cProject->modified = true;
        }
        if (ImGui::InputInt("Set Height", &cMap->height)) {
            cProject->modified = true;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Project")) {
        setModeBits(EDITOR_WIDGET);
        if (ImGui::BeginMenu("Tileset")) {
            if (ImGui::BeginMenu("Texture File")) {
                for (const auto& it : textureFiles) {
                    if (ImGui::MenuItem(it.c_str())) {
                        cProject->tileset->LoadTexture(it);
                        cProject->modified = true;
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::InputInt("Tile Width", &cProject->tileset->tileWidth)) {
                cProject->modified = true;
            }
            if (ImGui::InputInt("Tile Height", &cProject->tileset->tileHeight)) {
                cProject->modified = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::InputText("Project Name", name, sizeof(name))) {
            cProject->name = name;
            cProject->setPath(name);
            cProject->modified = true;
            Printf("New project name: %s", cProject->name.c_str());
            Printf("New project path: %s", cProject->path.c_str());
        }
        ImGui::EndMenu();
        clearModeBits(EDITOR_WIDGET);
    }
    if (ImGui::BeginMenu("Settings")) {
        if (ImGui::SliderFloat("Zoom Speed", &cGUI->getCamera().zoomSpeed, 1.0f, 15.0f)) {
            SavePreferences();
        }
        if (ImGui::SliderFloat("Rotation Speed", &cGUI->getCamera().rotationSpeed, 1.0f, 15.0f)) {
            SavePreferences();
        }
        if (ImGui::SliderFloat("Move Speed", &cGUI->getCamera().moveSpeed, 1.5f, 20.0f)) {
            SavePreferences();
        }
        ImGui::EndMenu();
    }
}

void Editor::DrawProjectMenu(void)
{
    char name[1024];
    memset(name, 0, sizeof(name));

    if (ImGui::BeginMenu("Change")) {
        setModeBits(EDITOR_WIDGET);
        if (ImGui::BeginMenu("Texture File")) {
            for (const auto& it : textureFiles) {
                if (ImGui::MenuItem(it.c_str())) {
                    cProject->tileset->LoadTexture(it);
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::InputText("Project Name", name, sizeof(name))) {
            cProject->name = name;
            cProject->setPath(name);
            cProject->modified = true;
        }
        ImGui::EndMenu();
        clearModeBits(EDITOR_WIDGET);
    }
    if (ImGui::BeginMenu("Info")) {
        ImGui::Text("Name: %s", cProject->name.c_str());
        ImGui::Text("Path: %s", cProject->path.c_str());
        ImGui::Text("Map File: %s", cProject->mapData->name.c_str());
        ImGui::Text("Texture File: %s", cProject->tileset->texture->path.c_str());
        ImGui::EndMenu();
    }
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
        if (ImGui::BeginMenu("Project")) {
            DrawProjectMenu();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Editor::run(void)
{
	while (1) {
		cGUI->BeginFrame();
		cGUI->PollEvents(this);
		
		PollCommands();
        DrawWidgets();
		
		cGUI->DrawMap(cMap);
		cGUI->EndFrame();
	}
}