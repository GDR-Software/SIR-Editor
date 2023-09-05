#ifndef __PROJECT_MANAGER__
#define __PROJECT_MANAGER__

#pragma once

#include "EditorManager.h"
#include "Project.h"

class CProjectManager : public CEditorManager
{
public:
    CProjectManager(void);
    virtual ~CProjectManager();

    virtual inline bool HasWizard(void) const override
    { return true; }
    virtual inline bool HasOpenRecent(void) const override
    { return true; }

    inline void SetCurrentAssetDir(const std::filesystem::path& dir)
    { curTool->SetAssetDirectory(dir); }
    inline void SetCurrentProjDir(const std::filesystem::path& dir)
    { curTool->SetProjectDirectory(dir); }
    inline const std::filesystem::path& GetProjectDirectory(void) const
    { return curTool->GetProjectDirectory(); }
    inline const std::filesystem::path& GetAssetDirectory(void) const
    { return curTool->GetAssetDirectory(); }
    inline void NewProject(void)
    { curTool->New(); }
    inline void SaveCurrent(void)
    { curTool->Save(); }

    virtual void DrawWizard(const string_t& menuTitle = "Project Wizard") override;
    virtual void DrawWizard(void) override
    { DrawWizard("Project Wizard"); }

    IMPL_EDITOR_MANAGER(CProjectManager, CProject)
    IMPL_SINGLETON(CProjectManager, CProject)
private:
    void LoadProject(const string_t& name);
};

#endif