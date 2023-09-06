#ifndef __TILESET_MANAGER__
#define __TILESET_MANAGER__

#pragma once

class CTilesetManager : public CEditorManager
{
public:
    CTilesetManager(void);
    virtual ~CTilesetManager();

    virtual inline bool HasWizard(void) const override
    { return true; }
    virtual inline bool HasOpenRecent(void) const override
    { return true; }
    virtual inline const char *GetWizardName(void) const override
    { return "Tileset"; }
    virtual inline const char *GetManagerName(void) const override
    { return "Tileset Manager"; }

    virtual void DrawWizard(const string_t& menuTitle = "Tileset Wizard") override;
    virtual void DrawWizard(void) override
    { DrawWizard("Tileset Wizard"); }

    IMPL_EDITOR_MANAGER(CTilesetManager, CTileset)
    IMPL_SINGLETON(CTilesetManager, CTileset)
};

#endif