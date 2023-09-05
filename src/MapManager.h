#ifndef __MAP_MANAGER__
#define __MAP_MANAGER__

#pragma once

#include "EditorManager.h"
#include "Map.h"

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

    IMPL_EDITOR_MANAGER(CMapManager, CMap)
    IMPL_SINGLETON(CMapManager, CMap)
};

#endif