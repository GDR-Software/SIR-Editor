#ifndef __MAP_MANAGER__
#define __MAP_MANAGER__

#pragma once

class CMapManager : public CEditorManager
{
public:
    CMapManager(void);
    virtual ~CMapManager();

    virtual void DrawWizard(const string_t& menuTitle = "Map Wizard") override;
    virtual void DrawWizard(void) override
    { DrawWizard("Map Wizard"); }

    virtual inline bool HasWizard(void) const override
    { return true; }
    virtual inline bool HasOpenRecent(void) const override
    { return true; }
    virtual inline const char *GetWizardName(void) const override
    { return "Map"; }
    virtual inline const char *GetManagerName(void) const override
    { return "Map Manager"; }

    IMPL_EDITOR_MANAGER(CMapManager, CMap)
    IMPL_SINGLETON(CMapManager, CMap)
};

#endif