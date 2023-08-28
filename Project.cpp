#include "Common.hpp"
#include "Map.h"
#include "Tileset.h"
#include "Editor.h"
#include "Project.h"

Project::Project()
    : name{"untitled-project"}, modified{false}
{
    setPath(name);

    mapData = eastl::make_shared<Map>();
    tileset = eastl::make_shared<Tileset>();
}

void Project::Save(void)
{
    json data;

    if (!modified)
        return; // sometimes it recurses

    Printf("Saving current project...");

    std::ofstream file(path.c_str(), std::ios::out);
    if (file.fail()) {
        Error("Failed to open file '%s' in write mode", path.c_str());
    }

    data["name"] = name;
    data["mapname"] = mapData->name;
    data["texture"] = tileset->texture->path;

    file << data;

    file.close();

    Printf("Save successful");

    modified = false;
}

void Project::New(void)
{
    tileset.reset();
    mapData.reset();
    modified = true;
}

void Project::setPath(const eastl::string& newpath)
{
    eastl::string p = newpath;
    if (newpath.find(".nlp") == eastl::string::npos) {
        p.append(".nlp");
    }
    path = eastl::string("Data/") + p;
}

void Project::Load(const eastl::string& newpath)
{
    json data;
    FILE *fp;

    if (!FileExists(newpath.c_str())) {
        Printf("Project::Load: cannot load project '%s' because it does not exist", newpath.c_str());
        return;
    }

    fp = SafeOpenRead(newpath.c_str());
    try {
        data = json::parse(fp);
    } catch (const json::exception& e) {
        Printf("Failed to load json, nlohmann::json::exception =>\n\tid: %i\n\twhat(): %s", e.id, e.what());
        fclose(fp);
        return;
    }
    fclose(fp);

    path = newpath;

    name = data.at("name");
    const eastl::string& mapName = data.at("mapname");
    const eastl::string& textureFile = data.at("texture");

    mapData->Clear();
    mapData->name = mapName;
    tileset->LoadTexture(textureFile);

    modified = true;
}
