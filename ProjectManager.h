#ifndef __PROJECT_MANAGER__
#define __PROJECT_MANAGER__

#pragma once

#include <EASTL/unordered_map.h>
#include <EASTL/string.h>

#include "Project.h"

class CProjectManager
{
public:
    CProjectManager(void);
    ~CProjectManager();

    bool LoadList(void);
    void ClearList(void);
private:
    eastl::unordered_map<eastl::string, CProject> projectList;
    int numProjects;
};

#endif