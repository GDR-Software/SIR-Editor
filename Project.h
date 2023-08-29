#ifndef _PROJECT_H_
#define _PROJECT_H_

#pragma once

#include "EditorTool.h"

class CProject : public CEditorTool
{
public:
    CProject(void);
    ~CProject() = default;

    virtual bool Load(const eastl::string& path) override;
    virtual bool Load(const json& data) override;

    void setPath(const eastl::string& newpath);
public:
    eastl::string path;
    eastl::string name;

    eastl::shared_ptr<Map> mapData;
    eastl::shared_ptr<Tileset> tileset;

    bool modified;
};

#endif