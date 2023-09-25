#include "gln.h"

std::unique_ptr<CProject> project = std::make_unique<CProject>();

void Project_New(void)
{
    project->mPath = gameConfig->mEditorPath + "untitled-project.proj";
    project->mName = "untitled-project";
    project->mModified = true;
    Map_New();
}

void Project_Save(const char *filename)
{
    char rpath[MAX_OSPATH*2+1];
    json data, tileset;
    std::ofstream file;

    snprintf(rpath, sizeof(rpath), "%s%s", gameConfig->mEditorPath.c_str(), filename);
    if (!GetExtension(filename) || N_stricmp(GetExtension(filename), "proj")) {
        snprintf(rpath, sizeof(rpath), "%s%s.proj", gameConfig->mEditorPath.c_str(), filename);
    }

    Printf("Saving project '%s'...", filename);

    data["name"] = project->mName;
    data["mapName"] = mapData->mPath;

    tileset["tilewidth"] = project->tileset->tileWidth;
    tileset["tileheight"] = project->tileset->tileHeight;
    tileset["tileCountX"] = project->tileset->tileCountX;
    tileset["tileCountY"] = project->tileset->tileCountY;
    tileset["numTiles"] = project->tileset->tiles.size();
    tileset["texFile"] = project->texData->mName;

    // cache all the tiles
    for (uint64_t i = 0; i < project->tileset->tiles.size(); i++) {
        const maptile_t *tile = &project->tileset->tiles[i];
        tileset[va("tile%lu", i)] = {
            {"flags", tile->flags},
            {"pos", { tile->pos[0], tile->pos[1], tile->pos[2] }},
            {"texcoords", {
                tile->texcoords[0][0], tile->texcoords[0][1],
                tile->texcoords[1][0], tile->texcoords[1][1],
                tile->texcoords[2][0], tile->texcoords[2][1],
                tile->texcoords[3][0], tile->texcoords[3][1]
            }}
        };
    }
    data["tileset"] = tileset;

    file.open(rpath, std::ios::out);
    if (!file.is_open()) {
        Error("Project_Save: failed to open file '%s' in write mode", rpath);
    }
    file.width(4);
    file << data;
    file.close();

    project->mModified = false;
}

static bool ValidateProject(const json& data)
{
    if (!data.contains("name")) return false;
    if (!data.contains("tileset")) return false;
    if (!data.at("tileset").contains("tilewidth")) return false;
    if (!data.at("tileset").contains("tileheight")) return false;
    if (!data.at("tileset").contains("tileCountX")) return false;
    if (!data.at("tileset").contains("tileCountY")) return false;
    if (!data.at("tileset").contains("numTiles")) return false;
    if (!data.at("tileset").contains("texFile")) return false;

    return true;
}

void Project_Load(const char *filename)
{
    json data;

    if (!LoadJSON(data, filename)) {
        Printf("Failed to load project file '%s'", filename);
        return;
    }

    if (!ValidateProject(data)) {
        Printf("Error: '%s' is not a valid project file", filename);
        return;
    }

    project->mPath = filename;
    project->mName = data["name"];
    project->tileset->tileCountX = data["tileset"]["tileCountX"];
    project->tileset->tileCountY = data["tileset"]["tileCountY"];
    project->tileset->tileHeight = data["tileset"]["tileheight"];
    project->tileset->tileWidth = data["tileset"]["tilewidth"];
    project->tileset->tiles.resize(data["tileset"]["numTiles"]);

    for (uint64_t i = 0; i < project->tileset->tiles.size(); i++) {
        const json& tile = data["tileset"][va("tile%lu", i)];
        maptile_t *t = &project->tileset->tiles[i];

        t->flags = tile["flags"];
        t->index = i;

        const std::array<std::array<float, 4>, 2>& texcoords = tile["texcoords"].get<std::array<std::array<float, 4>, 2>>();

        memcpy(t->texcoords, texcoords.data(), sizeof(t->texcoords));
    }

    project->texData->Load(data["tileset"]["texFile"]);

    Map_Load(data["mapName"].get<std::string>().c_str());
}

CProject::CProject(void)
{
    tileset = std::make_shared<CTileset>();
    tileset->texData = std::make_shared<CTexture>();
    texData = tileset->texData;
    mModified = true;
}

CProject::~CProject()
{
}
