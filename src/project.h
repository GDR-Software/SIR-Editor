#ifndef __PROJECT__
#define __PROJECT__

#pragma once

class CProject
{
public:
    std::string mName;
    std::string mPath;

    bool mModified;

    std::shared_ptr<CTileset> tileset;
    std::shared_ptr<CTexture> texData;

    CProject(void);
    ~CProject();
};

extern std::unique_ptr<CProject> project;

void Project_New(void);
void Project_Save(const char *filename);
void Project_Load(const char *filename);

#endif