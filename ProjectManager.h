#ifndef __PROJECT_MANAGER__
#define __PROJECT_MANAGER__

#pragma once

#include "EditorManager.h"
#include "Project.h"

class CProjectManager : public CEditorManager<CProject>
{
public:
    CProjectManager(void);
    ~CProjectManager();

    virtual bool LoadList(void) override;
    virtual void ClearList(void) override;
    virtual void Draw(void) override;

    virtual inline bool HasWizard(void) const override
    { return true; }

    inline const std::filesystem::path& GetProjectDirectory(void) const
    { return curProject->GetProjectDirectory(); }
    inline const std::filesystem::path& GetAssetDirectory(void) const
    { return curProject->GetAssetDirectory(); }
    inline const eastl::string& GetProjectName(void) const
    { return curProject->GetName(); }
    inline const CProject *GetProject(void) const
    { return curProject; }
private:
    void DrawWizard(void);
    CProject *curProject;
};

#endif