#ifndef __TEXTURE_MANAGER__
#define __TEXTURE_MANAGER__

#pragma once

#include "EditorManager.h"
#include "Texture.h"

class CTextureManager : public CEditorManager
{
public:
    CTextureManager(void);
    virtual ~CTextureManager();

    virtual void DrawWizard(const string_t& menuTitle = "Texture Wizard") override;
    virtual void DrawWizard(void) override
    { DrawWizard("Texture Wizard"); }

    virtual inline bool HasWizard(void) const override
    { return true; }
    virtual inline bool HasOpenRecent(void) const override
    { return false; }

    IMPL_EDITOR_MANAGER(CTextureManager, CTexture)
};

#endif