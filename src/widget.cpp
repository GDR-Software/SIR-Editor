#include "gln.h"

static constexpr ImVec2 FileDlgWindowSize = ImVec2( 1012, 641 );

static INLINE void ImGui_Quad_TexCoords(const float texcoords[4][2], ImVec2& min, ImVec2& max)
{
    min = { texcoords[3][0], texcoords[3][1] };
    max = { texcoords[1][0], texcoords[1][1] };
}

/*
FIXME: for some reason, only the globals method works
*/


/*
absolutely no fucking clue how to do this with macros
*/
static INLINE void CHECK_VAR(char* dst, int size, const std::string& reset, bool cond)
{
    if (cond) {
        N_strncpyz(dst, reset.c_str(), size - 1);
    }
}
template<typename T>
static INLINE void CHECK_VAR(T& dst, const T& reset, bool cond)
{
    if (cond) {
        dst = reset;
    }
}
template<typename T, typename U>
static INLINE void CHECK_VAR(T& dst, const U& reset, bool cond)
{
    if (cond) {
        dst = reset;
    }
}

template<typename T>
static INLINE void UPDATE_VAR(T& dst, const T& src, bool& cond)
{
    if (cond) {
        dst = src;
        cond = false;
    }
}
template<typename T, typename U>
static INLINE void UPDATE_VAR(T& dst, const U& src, bool& cond)
{
    if (cond) {
        dst = src;
        cond = false;
    }
}
static INLINE void UPDATE_VAR(std::string& dst, char *src, int size, bool& cond)
{
    if (cond) {
        dst = src;
        memset(src, 0, size);
        cond = false;
    }
}
template<typename T>
static INLINE void UPDATE_VAR(std::vector<T>& dst, int& amount, bool& cond)
{
    if (cond) {
        uint32_t i;
        if (dst.size() < amount) {
            for (i = 0; i < amount; i++) {
                memset(eastl::addressof(dst.emplace_back()), 0, sizeof(T));
            }
        }
        else if (dst.size() > amount) {
            i = dst.size();
            while (i != amount) {
                dst.pop_back();
                i--;
            }
        }
        cond = false;
        amount = 0;
    }
}

static INLINE void GET_VAR(bool& change, const char *title, bool& value)
{ if ((value = ImGui::Button(title))) { change = true; } }
static INLINE void GET_VAR(bool& change, const char *title, int& value)
{ if (ImGui::InputInt(title, eastl::addressof(value))) { change = true; } }
static INLINE void GET_VAR(bool& change, const char *title, float& value)
{ if (ImGui::InputFloat(title, eastl::addressof(value))) { change = true; } }
static INLINE void GET_VAR(bool& change, const char *title, char *value, int size)
{ if (ImGui::InputText(title, value, size)) { change = true; } }

static INLINE void GET_VAR(bool& change, bool& changed, const char *title, bool& value)
{ if ((value = ImGui::Button(title))) { change = true; changed = true; } }
static INLINE void GET_VAR(bool& change, bool& changed, const char *title, int& value)
{ if (ImGui::InputInt(title, eastl::addressof(value))) { change = true; changed = true; } }
static INLINE void GET_VAR(bool& change, bool& changed, const char *title, float& value)
{ if (ImGui::InputFloat(title, eastl::addressof(value))) { change = true; changed = true; } }
static INLINE void GET_VAR(bool& change, bool& changed, const char *title, char *value, int size)
{ if (ImGui::InputText(title, value, size)) { change = true; changed = true; } }

template<typename T>
static INLINE void GET_VAR_MENU(bool& change, bool& changed, T& value, const T& set, bool enabled, const char *title)
{
    if (ImGui::MenuItem(title, (const char *)NULL, (bool *)NULL, enabled)) {
        value = set;
        change = true;
        changed = true;
    }
}
template<typename T, typename U>
static INLINE void GET_VAR_MENU(bool& change, T& value, const U& set, bool enabled, const char *title)
{
    if (ImGui::MenuItem(title, (const char *)NULL, (bool *)NULL, enabled)) {
        value = set;
        change = true;
    }
}
template<typename T>
static INLINE void GET_VAR_MENU(bool& change, bool& changed, T& value, const T& set, const char *title)
{
    if (ImGui::MenuItem(title)) {
        value = set;
        change = true;
        changed = true;
    }
}
template<typename T, typename U>
static INLINE void GET_VAR_MENU(bool& change, bool& changed, T& value, const U& set, const char *title)
{
    if (ImGui::MenuItem(title)) {
        value = set;
        change = true;
        changed = true;
    }
}
template<typename T>
static INLINE void GET_VAR_MENU(bool& change, T& value, const T& set, const char *title)
{
    if (ImGui::MenuItem(title)) {
        value = set;
        change = true;
    }
}
template<typename T, typename U>
static INLINE void GET_VAR_MENU(bool& change, T& value, const U& set, const char *title)
{
    if (ImGui::MenuItem(title)) {
        value = set;
        change = true;
    }
}

typedef struct {
    int x;
    int y;
    int elevation;
    float brightness;
    vec3_t color;
    bool xChanged;
    bool yChanged;
    bool brightnessChanged;
    bool colorChanged;
} lightGlobals_t;

typedef struct {
    int color[4];
    int x;
    int y;
    int tileIndex;
    unsigned flags;
    bool flagsChanged;
    bool isCheckpoint;
    bool isSpawn;
    bool isCheckpointChanged;
    bool isSpawnChanged;
    bool open;
} tileGlobals_t;

typedef struct {
    int textureDetails;
    int textureFilters;
    bool techy;
    bool changed;
    bool textureDetailsChanged;
    bool textureFiltersChanged;
} graphicsGlobals_t;

typedef struct {
    char enginePath[MAX_OSPATH*2+1];
    char exePath[MAX_OSPATH*2+1];
    char editorPath[MAX_OSPATH*2+1];
    float cameraRotationSpeed;
    float cameraMoveSpeed;
    float cameraZoomSpeed;
    bool enginePathChanged;
    bool exePathChanged;
    bool editorPathChanged;
    bool cameraRotationSpeedChanged;
    bool cameraMoveSpeedChanged;
    bool cameraZoomSpeedChanged;
    bool changed;
} configGlobals_t;

typedef struct {
    int x;
    int y;
    int entitytype;
    int entityid;
    bool xChanged;
    bool yChanged;
    bool typeChanged;
    bool idChanged;
} spawnGlobals_t;

typedef struct {
    int x;
    int y;
    bool xChanged;
    bool yChanged;
} checkpointGlobals_t;

typedef struct {
    char name[MAX_GDR_PATH];
    char path[MAX_OSPATH*2+1];
    int width;
    int height;
    int numCheckpoints;
    int numSpawns;
    int numLights;
    int editingSpawnIndex;
    int editingCheckpointIndex;
    bool nameChanged;
    bool pathChanged;
    bool widthChanged;
    bool heightChanged;
    bool checkpointsChanged;
    bool spawnsChanged;
    bool lightsChanged;
    bool changed;
    bool editingSpawn;
    bool editingCheckpoint;
    bool open;
} mapGlobals_t;

typedef struct {
    bool tilesetOpen;
} viewGlobals_t;

typedef struct {
    char texturePath[MAX_OSPATH*2+1];
    int tileWidth;
    int tileHeight;
    int numTiles;
    bool texturePathChanged;
    bool tileWidthChanged;
    bool tileHeightChanged;
    bool numTilesChanged;
    bool changed;
    bool open;
} tilesetGlobals_t;

typedef struct widgetsGlobals_s {
    graphicsGlobals_t graphics;
    configGlobals_t config;
    mapGlobals_t map;
    spawnGlobals_t spawn;
    checkpointGlobals_t checkpoint;
    tilesetGlobals_t tileset;
    lightGlobals_t light;
    tileGlobals_t tile;
    viewGlobals_t view;
    bool preferencesChanged;
    
    widgetsGlobals_s(void)
    { memset(this, 0, sizeof(*this)); }
} widgetsGlobals_t;

static std::unique_ptr<widgetsGlobals_t> globals = std::make_unique<widgetsGlobals_t>();

static void File_Menu(void);
static void Edit_Menu(void);
static void Edit_Preferences(void);

static INLINE void Update_Config(void)
{
    configGlobals_t *g = &globals->config;

    UPDATE_VAR(gameConfig->mCameraMoveSpeed, g->cameraMoveSpeed, g->cameraMoveSpeedChanged);
    UPDATE_VAR(gameConfig->mCameraRotationSpeed, g->cameraRotationSpeed, g->cameraRotationSpeedChanged);
    UPDATE_VAR(gameConfig->mCameraZoomSpeed, g->cameraZoomSpeed, g->cameraZoomSpeedChanged);
    UPDATE_VAR(gameConfig->mEditorPath, g->editorPath, g->editorPathChanged);
    UPDATE_VAR(gameConfig->mEnginePath, g->enginePath, g->enginePathChanged);
    UPDATE_VAR(gameConfig->mExecutablePath, g->exePath, g->exePathChanged);
}
static INLINE void Update_Graphics(void)
{
    graphicsGlobals_t *g = &globals->graphics;

    UPDATE_VAR(gameConfig->mTextureDetail, g->textureDetails, g->textureDetailsChanged);
    UPDATE_VAR(gameConfig->mTextureFiltering, g->textureFilters, g->textureFiltersChanged);
}
static INLINE void Update_Map(void)
{
    mapGlobals_t *g = &globals->map;

    if (g->width != mapData->mWidth || g->height != mapData->mHeight) {
        mapData->mTiles.resize(g->width * g->height);
        mapData->CalcDrawData();
    }

    UPDATE_VAR(mapData->mWidth, g->width, g->widthChanged);
    UPDATE_VAR(mapData->mHeight, g->height, g->heightChanged);
    UPDATE_VAR(mapData->mCheckpoints, g->numCheckpoints, g->checkpointsChanged);
    UPDATE_VAR(mapData->mSpawns, g->numSpawns, g->spawnsChanged);
}

static INLINE void Update_Tileset(void)
{
    tilesetGlobals_t *g = &globals->tileset;
    std::shared_ptr<CTileset>& tileset = project->tileset;

    UPDATE_VAR(tileset->tileHeight, g->tileHeight, g->tileHeightChanged);
    UPDATE_VAR(tileset->tileWidth, g->tileWidth, g->tileWidthChanged);

    if (tileset->tileWidth == 0 || tileset->tileHeight == 0)
        return;

    tileset->GenerateTiles();
}


static void File_Menu(void)
{
    if (ImGui::BeginMenu("Project")) {
        if (ItemWithTooltip("Open Project", "Open an already made project")) {
            ImGuiFileDialog::Instance()->OpenDialog("SelectProjectDlg", "Select File", ".proj, .json", gameConfig->mEditorPath);
        }
        if (ItemWithTooltip("New Project", "Create a new project")) {
            Project_New();
        }
        if (ItemWithTooltip("Save Project", "Save your current project")) {
            Project_Save(project->mName.c_str());
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Open Recent")) {
        if (ImGui::MenuItem("Open Map")) {
            ImGuiFileDialog::Instance()->OpenDialog("SelectMapDlg", "Select File", ".map, .bmf, .*", gameConfig->mEditorPath);
        }
        ImGui::EndMenu();
    }
}

static void Edit_Menu(void)
{
    if (ImGui::BeginMenu("Preferences")) {
        Edit_Preferences();
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Map")) {
        globals->map.open = true;
    }
    if (ImGui::MenuItem("Tileset")) {
        globals->tileset.open = true;
    }
}

static void Build_Menu(void)
{
    if (ItemWithTooltip("Build Map", "Save the current map in text-based format into a .map file")) {
        Map_Save(mapData->mName.c_str());
    }
    if (ItemWithTooltip("Compile Map", "Compile a .map file into a .bmf file,\nNOTE: .bmf files cannot be used in the map editor")) {

    }
}

static void Edit_Graphics(void)
{
    graphicsGlobals_t *g = &globals->graphics;

    ImGui::SeparatorText("Graphics");
    if (ImGui::BeginMenu("Texture Detail")) {
        for (uint64_t i = 0; i < arraylen(texture_details); i++) {
            GET_VAR_MENU(g->textureDetailsChanged, g->changed, g->textureDetails, texture_details[i].i, texture_details[i].s);
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Texture Filtering")) {
        if (!g->techy && ImGui::Button("I'm A Techie")) {
            g->techy = true;
        }
        else if (g->techy && ImGui::Button("I'm Not A Techie")) {
            g->techy = false;
        }
        if (g->techy) {
            ImGui::Text("Mag Filter | Min Filter | Filter Name");
            ImGui::Text("-----------|------------|------------");
        }
        for (uint64_t i = 0; i < arraylen(texture_filters); i++) {
            if (ImGui::MenuItem(g->techy ? texture_filters_alt[i].s : texture_filters[i].s)) {
                g->textureFiltersChanged = true;
                g->changed = true;
                g->textureFilters = texture_filters[i].i;
            }
        }
        ImGui::EndMenu();
    }
}

static void Edit_Preferences(void)
{
    configGlobals_t *config = &globals->config;
    graphicsGlobals_t *graphics = &globals->graphics;

    ImGui::SeparatorText("Configuration");
    if (ButtonWithTooltip(va("Engine Path: %s", gameConfig->mEnginePath.c_str()), "Set the editor's path to the game engine")) {
        ImGuiFileDialog::Instance()->OpenDialog("SelectEnginePathDlg", "Select Executable File", ".exe, .application, .sh, .AppImage, .*", gameConfig->mEditorPath);
    }
    if (ButtonWithTooltip(va("Executable Path: %s", gameConfig->mExecutablePath.c_str()), "Set the editor's path to executables")) {
        ImGuiFileDialog::Instance()->OpenDialog("SelectExePathDlg", "Select Folder", "/", gameConfig->mEditorPath);
    }

    Edit_Graphics();
    
    if (globals->preferencesChanged) {
        if (ImGui::Button("Save Preferences")) {
            Update_Config();
            Update_Graphics();
            globals->preferencesChanged = false;
            gameConfig->mPrefs.SavePrefs();
        }
    }
}


typedef struct {
    int curX, curY;
    bool active;
} tileMode_t;
extern tileMode_t tileMode;

static void TileMode(void)
{
    tileGlobals_t *g = &globals->tile;

    if (editor->mode != MODE_TILE) {
        g->flags = 0;
        return;
    }
    if (Key_IsDown(KEY_ENTER)) {
        g->open = true;
    }

    CHECK_VAR(g->tileIndex, mapData->mTiles[tileMode.curY * mapData->mWidth + tileMode.curX].index, true);
    CHECK_VAR(g->x, tileMode.curX, true);
    CHECK_VAR(g->y, tileMode.curY, true);
    CHECK_VAR(g->isCheckpoint, mapData->mTiles[tileMode.curY * mapData->mWidth + tileMode.curX].flags & TILE_CHECKPOINT, !g->isCheckpointChanged);
    CHECK_VAR(g->isSpawn, mapData->mTiles[tileMode.curY * mapData->mWidth + tileMode.curX].flags & TILE_SPAWN, !g->isSpawnChanged);
    
    if (ImGui::Begin("Tile Info", &g->open, 0)) {
        ImGui::Text("Tile X: %i", g->x);
        ImGui::Text("Tile Y: %i", g->y);

        //
        // Tile Flags
        //
        ImGui::SeparatorText(va("Flags [0x%x]", g->flags));
        if (g->isSpawn) {
            ImGui::PushStyleColor(ImGuiCol_Button, { 0.5, 0.5, 0.5, 1 });
        }
        if (ImGui::Button(va("Checkpoint (0x%x)", (uint32_t)TILE_CHECKPOINT))) {
            if (!g->isSpawn) {
                g->isCheckpointChanged = true;
                g->isCheckpoint = true;
                g->flags |= TILE_CHECKPOINT;
                g->flagsChanged = true;
            }
        }
        if (g->isSpawn) {
            ImGui::PopStyleColor();
        }

        if (g->isCheckpoint) {
            ImGui::PushStyleColor(ImGuiCol_Button, { 0.5, 0.5, 0.5, 1 });
        }
        if (ImGui::Button(va("Spawn (0x%x)", (uint32_t)TILE_SPAWN))) {
            if (!g->isCheckpoint) {
                g->isSpawnChanged = true;
                g->isSpawn = true;
                g->flags |= TILE_SPAWN;
                g->flagsChanged = true;
            }
        }
        if (g->isCheckpoint) {
            ImGui::PopStyleColor();
        }
        if (!g->flagsChanged) {
            ImGui::PushStyleColor(ImGuiCol_Button, { 0.5, 0.5, 0.5, 1 });
        }
        if (ImGui::Button("Clear Flags")) {
            if (g->flagsChanged) {
                g->isCheckpoint = false;
                g->isSpawn = false;
                g->flags = 0;
            }
        }
        if (!g->flagsChanged) {
            ImGui::PopStyleColor();
        }

        //
        // Texture
        //
        ImGui::SeparatorText("Texture");
        ImGui::Text("Tile Index: %i", g->tileIndex);
        if (!project->tileset->tiles.size()) {
            ImGui::Text("No Tileset Generated");
        }
        else {
            if (g->tileIndex != -1) {
                ImVec2 min, max;
                const maptile_t *tile = &project->tileset->tiles[g->tileIndex];

                ImGui_Quad_TexCoords(tile->texcoords, min, max);
                ImGui::Image((ImTextureID)(uint64_t)project->tileset->texData->mId, ImVec2( 48, 48 ), min, max);
            }
            if (g->tileIndex != -1 && ImGui::Button("Clear Texture")) {
                g->tileIndex = -1;
            }
        }

        //
        // Tile Color
        //
        ImGui::SeparatorText("Color");
        ImGui::SliderInt("R", &g->color[0], 0, 255);
        ImGui::SliderInt("G", &g->color[1], 0, 255);
        ImGui::SliderInt("B", &g->color[2], 0, 255);
        g->color[3] = 1;

        if (ImGui::Button("Done")) {
            UPDATE_VAR(mapData->mTiles[tileMode.curY * mapData->mWidth + tileMode.curX].flags,
                mapData->mTiles[tileMode.curY * mapData->mWidth + tileMode.curX].flags | TILE_CHECKPOINT, g->isCheckpointChanged);
            UPDATE_VAR(mapData->mTiles[tileMode.curY * mapData->mWidth + tileMode.curX].flags,
                mapData->mTiles[tileMode.curY * mapData->mWidth + tileMode.curX].flags | TILE_SPAWN, g->isSpawnChanged);
            
            const glm::vec4 color = NormalToRGBA({ g->color[0], g->color[1], g->color[2], 1 });
            memcpy(mapData->mVertices[tileMode.curY * mapData->mWidth + tileMode.curX].color, &color[0], sizeof(vec4_t));

            g->open = false;
        }
    }
    ImGui::End();
}

static const char *EntityTypeString(int type)
{
    switch (type) {
    case ET_PLAYR:
        return "Player";
    case ET_MOB:
        return "Mob";
    case ET_BOT:
        return "Bot";
    case ET_ITEM:
        return "Item";
    case ET_WEAPON:
        return "Weapon";
    case ET_AIR:
        return "Air (Nothing, An Invalid Spawn)";
    };
    Error("Invalid entity type: %i", type);
    return "";
}

static void Edit_Spawn(void)
{
    spawnGlobals_t *g = &globals->spawn;
    mapspawn_t *s;
    bool open;
    const int index = globals->map.editingSpawnIndex;

    if (!globals->map.editingSpawn)
        return;
    
    s = &mapData->mSpawns[index];

    CHECK_VAR(g->x, s->xyz[0], !g->xChanged);
    CHECK_VAR(g->y, s->xyz[1], !g->yChanged);

    open = true;
    if (ImGui::Begin("Edit Spawn", &open, ImGuiWindowFlags_NoResize)) {
        ImGui::SetWindowSize({ 381, 168 });

        GET_VAR(g->xChanged, "x", g->x);
        GET_VAR(g->yChanged, "y", g->y);

        g->x = clamp(g->x, 0, mapData->mWidth);
        g->y = clamp(g->y, 0, mapData->mHeight);

        if (ImGui::BeginMenu("Select Entity Type")) {
            GET_VAR_MENU(g->typeChanged, g->entitytype, ET_PLAYR, "Player");
            GET_VAR_MENU(g->typeChanged, g->entitytype, ET_MOB, "Mob");
            GET_VAR_MENU(g->typeChanged, g->entitytype, ET_BOT, "Bot");
            GET_VAR_MENU(g->typeChanged, g->entitytype, ET_ITEM, "Item");
            GET_VAR_MENU(g->typeChanged, g->entitytype, ET_WEAPON, "Weapon");
            ImGui::EndMenu();
        }
        ImGui::Text("Current Entity Type: %s", EntityTypeString(g->entitytype));
        if (ImGui::BeginMenu("Select Entity Id")) {
            if (g->entitytype != ET_MOB) {
                ImGui::MenuItem("N/A");
            }
            else if (g->entitytype == ET_MOB) {
                for (const auto& it : gameConfig->mMobList) {
                    GET_VAR_MENU(g->idChanged, g->entityid, it.mId, va("Name: %-64s Id: %-16u", it.mName.c_str(), it.mId));
                }
            }
            ImGui::EndMenu();
        }
        ImGui::Text("Current Entity Id: %s", g->entityid != 0 ? gameConfig->mMobList[g->entityid].mName.c_str() : "N/A");

        if (ImGui::Button("Save Spawn")) {
            int old_x = s->xyz[0];
            int old_y = s->xyz[1];
            int old_z = s->xyz[2];

            if (mapData->mTiles[old_y * mapData->mWidth + old_x].flags & TILE_SPAWN) {
                mapData->mTiles[old_y * mapData->mWidth + old_x].flags &= ~TILE_SPAWN;
            }
            UPDATE_VAR(g->x, s->xyz[0], g->xChanged);
            UPDATE_VAR(g->y, s->xyz[1], g->yChanged);

            mapData->mTiles[s->xyz[1] * mapData->mWidth + s->xyz[0]].flags |= TILE_SPAWN;
            open = false;
        }
    }
    ImGui::End();
    globals->map.editingSpawn = open;
}

static void Edit_Checkpoint(void)
{
    checkpointGlobals_t *g = &globals->checkpoint;
    mapcheckpoint_t *c;
    bool open;
    const int index = globals->map.editingCheckpointIndex;

    if (!globals->map.editingCheckpoint)
        return;
    
    c = &mapData->mCheckpoints[index];

    CHECK_VAR(g->x, c->xyz[0], !g->xChanged);
    CHECK_VAR(g->y, c->xyz[1], !g->yChanged);

    open = true;
    if (ImGui::Begin("Edit Checkpoint", &open, ImGuiWindowFlags_NoResize)) {
        ImGui::SetWindowSize({ 223, 103 });

        GET_VAR(g->xChanged, "x", g->x);
        GET_VAR(g->yChanged, "y", g->y);

        g->x = clamp(g->x, 0, mapData->mWidth);
        g->y = clamp(g->y, 0, mapData->mHeight);

        if (ImGui::Button("Save Checkpoint")) {
            int old_x = c->xyz[0];
            int old_y = c->xyz[1];
            int old_z = c->xyz[2];

            if (mapData->mTiles[old_y * mapData->mWidth + old_x].flags & TILE_CHECKPOINT) {
                mapData->mTiles[old_y * mapData->mWidth + old_x].flags &= ~TILE_CHECKPOINT;
            }

            UPDATE_VAR(c->xyz[0], g->x, g->xChanged);
            UPDATE_VAR(c->xyz[1], g->y, g->yChanged);

            open = false;
            mapData->mTiles[c->xyz[1] * mapData->mWidth + c->xyz[0]].flags |= TILE_CHECKPOINT;
        }
    }
    ImGui::End();
    globals->map.editingCheckpoint = open;
}

static void Edit_Tileset(void)
{
    tilesetGlobals_t *g = &globals->tileset;
    const std::shared_ptr<CTileset>& tileset = project->tileset;

    if (!g->open) {
        return;
    }

    if (ImGui::Begin("Tileset", &g->open, ImGuiWindowFlags_NoResize)) {
        ImGui::SeparatorText("Parameters");

        CHECK_VAR(g->tileHeight, tileset->tileHeight, !g->tileHeightChanged && !g->changed);
        CHECK_VAR(g->tileWidth, tileset->tileWidth, !g->tileWidthChanged && !g->changed);

        if (ButtonWithTooltip(va("Texture Path: %s", tileset->texData->mName.size() ? tileset->texData->mName.c_str() : "None"),
        "Change the texture path of the current tileset")) {
            ImGuiFileDialog::Instance()->OpenDialog("SelectTexturePathDlg", "Select File", ".png, .bmp, .jpeg, .tga, .pcx, .*", gameConfig->mEditorPath);
            g->changed = true;
        }

        ImGui::Text("Texture Width: %i", tileset->texData->mWidth);
        ImGui::Text("Texture Height: %i", tileset->texData->mHeight);
        ImGui::Text("Tile Count: %lu", tileset->tiles.size());

        GET_VAR(g->tileWidthChanged, g->changed, "Tile Width", g->tileWidth);
        GET_VAR(g->tileHeightChanged, g->changed, "Tile Height", g->tileHeight);

        if (g->changed) {
            if (ImGui::Button("Confirm Tileset")) {
                Update_Tileset();

                g->changed = false;
                g->open = false;
            }
        }
    }
    ImGui::End();
}

static void Edit_Map(void)
{
    mapGlobals_t *g = &globals->map;

    if (!g->open) {
        return;
    }

    if (ImGui::Begin("Map", &g->open, ImGuiWindowFlags_NoResize)) {
        CHECK_VAR(g->name, sizeof(g->name), mapData->mName, !g->nameChanged && !g->changed);
        CHECK_VAR(g->numCheckpoints, mapData->mCheckpoints.size(), !g->checkpointsChanged && !g->changed);
        CHECK_VAR(g->numSpawns, mapData->mSpawns.size(), !g->spawnsChanged && !g->changed);
        CHECK_VAR(g->width, mapData->mWidth, !g->widthChanged && !g->changed);
        CHECK_VAR(g->height, mapData->mHeight, !g->heightChanged && !g->changed);

        GET_VAR(g->nameChanged, g->changed, "Name", g->name, sizeof(g->name) - 1);
        GET_VAR(g->widthChanged, g->changed, "Width", g->width);
        GET_VAR(g->heightChanged, g->changed, "Height", g->height);
        GET_VAR(g->checkpointsChanged, g->changed, "Checkpoint Count", g->numCheckpoints);
        GET_VAR(g->spawnsChanged, g->changed, "Spawn Count", g->numSpawns);

        g->width = clamp(g->width, 16, MAX_MAP_WIDTH);
        g->height = clamp(g->height, 16, MAX_MAP_HEIGHT);
        g->numSpawns = clamp(g->numSpawns, 1, MAX_MAP_SPAWNS);
        g->numCheckpoints = clamp(g->numCheckpoints, 0, MAX_MAP_CHECKPOINTS);

        if (ImGui::BeginMenu("Edit Checkpoint")) {
            if (!mapData->mCheckpoints.size()) {
                ImGui::MenuItem("No Checkpoints");
            }
            else {
                char buf[1024];
                for (uint64_t i = 0; i < mapData->mCheckpoints.size(); i++) {
                    snprintf(buf, sizeof(buf),
                        "-------- checkpoint %lu --------\n"
                        "coordinates: [%u, %u, %u]\n"
                    , i, mapData->mCheckpoints[i].xyz[0], mapData->mCheckpoints[i].xyz[1], mapData->mCheckpoints[i].xyz[2]);
                    if (ItemWithTooltip(va("Checkpoint #%lu", i), "%s", buf)) {
                        g->editingCheckpointIndex = i;
                        g->editingCheckpoint = true;
                    }
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit Spawn")) {
            if (!mapData->mSpawns.size()) {
                ImGui::MenuItem("No Spawns");
            }
            else {
                char buf[1024];
                for (uint64_t i = 0; i < mapData->mSpawns.size(); i++) {
                    snprintf(buf, sizeof(buf),
                        "-------- spawn %lu --------\n"
                        "coordinates: [%u, %u, %u]\n"
                        "entity type: %u\n"
                        "entity id: %s\n"
                    , i, mapData->mSpawns[i].xyz[0], mapData->mSpawns[i].xyz[1], mapData->mSpawns[i].xyz[2], mapData->mSpawns[i].entitytype,
                    mapData->mSpawns[i].entitytype == ET_MOB ? gameConfig->mMobList[mapData->mSpawns[i].entityid].mName.c_str() : "N/A");
                    if (ItemWithTooltip(va("Spawn #%lu", i), "%s", buf)) {
                        g->editingSpawnIndex = i;
                        g->editingSpawn = true;
                    }
                }
            }
            ImGui::EndMenu();
        }

        if (g->changed) {
            if (ImGui::Button("Confirm Map")) {
                Update_Map();
                g->changed = false;
                mapData->mModified = true;
                g->open = false;
            }
        }
    }
    ImGui::End();
}

static void View_Tileset(void)
{
    viewGlobals_t *g = &globals->view;
    const std::shared_ptr<CTileset>& tileset = project->tileset;
    const std::shared_ptr<CTexture>& texture = tileset->texData;
    ImVec2 textureSize = { tileset->tileWidth, tileset->tileHeight };
    ImVec2 min, max;
    const maptile_t *tile;

    if (!g->tilesetOpen) {
        return;
    }

    textureSize = clamp(textureSize, ImVec2( 64, 64 ), ImVec2( 128, 128 ));

    if (ImGui::Begin("Tiles", &g->tilesetOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
        for (uint32_t y = 0; y < tileset->tileCountY; y++) {
            for (uint32_t x = 0; x < tileset->tileCountX; x++) {
                tile = &tileset->tiles[y * tileset->tileCountX + x];
                ImGui_Quad_TexCoords(tile->texcoords, min, max);

                ImGui::PushID(1);
                if (ImGui::ImageButton((ImTextureID)(uint64_t)texture->mId, textureSize, min, max) || ImGui::IsItemClicked()) {
                    maptile_t *t = &mapData->mTiles[tileMode.curY * mapData->mWidth + tileMode.curX];
                    memcpy(t->texcoords, tile->texcoords, sizeof(t->texcoords));
                    t->index = y * tileset->tileCountX + x;
                }
                ImGui::PopID();
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
    }
    ImGui::End();
}

static void View_Menu(void)
{
    if (ImGui::MenuItem("Tileset")) {
        globals->view.tilesetOpen = true;
    }
}

void Widgets_Draw(void)
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
            View_Menu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Build")) {
            Build_Menu();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    Edit_Tileset();
    Edit_Map();
    Edit_Checkpoint();
    Edit_Spawn();
    View_Tileset();
    TileMode();

    configGlobals_t *g = &globals->config;
    if (ImGuiFileDialog::Instance()->IsOpened("SelectTexturePathDlg")) {
        if (ImGuiFileDialog::Instance()->Display("SelectTexturePathDlg", ImGuiWindowFlags_NoResize, FileDlgWindowSize, FileDlgWindowSize)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                project->tileset->texData->Load(ImGuiFileDialog::Instance()->GetFilePathName());
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
    if (ImGuiFileDialog::Instance()->IsOpened("SelectProjectDlg")) {
        if (ImGuiFileDialog::Instance()->Display("SelectProjectDlg", ImGuiWindowFlags_NoResize, FileDlgWindowSize, FileDlgWindowSize)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                Project_Load(ImGuiFileDialog::Instance()->GetFilePathName().c_str());
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
    if (ImGuiFileDialog::Instance()->IsOpened("SelectEnginePathDlg")) {
        if (ImGuiFileDialog::Instance()->Display("SelectEnginePathDlg", ImGuiWindowFlags_NoResize, FileDlgWindowSize, FileDlgWindowSize)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                N_strncpyz(g->enginePath, ImGuiFileDialog::Instance()->GetFilePathName().c_str(), sizeof(g->enginePath) - 1);
                g->enginePathChanged = true;
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
    if (ImGuiFileDialog::Instance()->IsOpened("SelectExePathDlg")) {
        if (ImGuiFileDialog::Instance()->Display("SelectExePathDlg", ImGuiWindowFlags_NoResize, FileDlgWindowSize, FileDlgWindowSize)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                N_strncpyz(g->exePath, ImGuiFileDialog::Instance()->GetFilePathName().c_str(), sizeof(g->exePath) - 1);
                g->exePathChanged = true;
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
    if (ImGuiFileDialog::Instance()->IsOpened("SelectMapDlg")) {
        if (ImGuiFileDialog::Instance()->Display("SelectMapDlg", ImGuiWindowFlags_NoResize, FileDlgWindowSize, FileDlgWindowSize)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                Map_Load(ImGuiFileDialog::Instance()->GetFilePathName().c_str());
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
}
