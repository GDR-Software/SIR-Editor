#ifndef _GUI_H_
#define _GUI_H_

#pragma once

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 texcoords;
    glm::vec4 color;
    float empty;
};

class CInput
{
public:
    Str mName;
    Str mBuf;
    bool mModified;

    CInput(void) { }
    CInput(const char *name, uint64_t bufLen)
        : mName{ name }, mModified{ false }
    {
        mBuf.GetBufferSetLength(bufLen);
    }
    
};

class CMenuItem
{
public:
    Str mName;
    Str mShortcut;
    bool mActive;

    CMenuItem(void) { }
    CMenuItem(const char *name)
        : mName{ name }, mShortcut{ (const char *)NULL }, mActive{ false } { }
    ~CMenuItem() { }
};

class CMenu
{
public:
    Str mName;
    bool mActive;
    std::vector<CMenu> mChildList;
    std::vector<CMenuItem> mItemList;

    CMenu(void) { }
    CMenu(const char *name)
        : mName{ name }, mActive{ false }, mChildList{}, mItemList{} { }
    ~CMenu() { }
};

class Window
{
public:
    Window(void);
    ~Window();

    static void Print(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

    void EndFrame(void);
    void BeginFrame(void);

    SDL_Window *mWindow;
    SDL_GLContext mContext;

    Vertex *mVertices;
    uint32_t *mIndices;

    char mInputBuf[4096];
    std::vector<char> mConbuffer;

    CMenu mMainMenu;
    eastl::unordered_map<const char *, CMenu> mMenuList;

    float mCameraZoom;
    float mCameraRotation;
    glm::vec3 mCameraPos;

    glm::mat4 mViewProjection;
    glm::mat4 mProjection;
    glm::mat4 mViewMatrix;
};

extern Window *gui;

CMenu* GUI_PushMainMenu_Child(const Str& name);
CMenuItem* GUI_PushMainMenu_Item(const Str& name);

CMenu* GUI_PushMenu(const Str& name);
CMenu* GUI_PushMenu_Child(CMenu *parent, const Str& childName);
CMenuItem* GUI_PushMenu_Item(CMenu *menu, const Str& itemName);

#endif