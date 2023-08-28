#ifndef _PROJECT_H_
#define _PROJECT_H_

#pragma once

class Project
{
public:
    Project();
    ~Project() = default;

    void Save(void);
    void Load(const eastl::string& newpath);
    void New(void);

    void setPath(const eastl::string& newpath);
public:
    eastl::string path;
    eastl::string name;

    eastl::shared_ptr<Map> mapData;
    eastl::shared_ptr<Tileset> tileset;

    bool modified;
};

#endif