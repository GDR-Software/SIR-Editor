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

class CMenuItem
{
public:
    const char *mName;
    const char *mShortcut;
    bool mActive;

    CMenuItem(const char *name)
        : mName{ name }, mShortcut{ NULL }, mActive{ false } { }
    ~CMenuItem() { }
};

class CMenu
{
public:
    const char *mName;
    bool mActive;
    eastl::vector<CMenu *> mChildList;
    eastl::unordered_map<const char *, CMenuItem> mItemList;

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
    eastl::vector<char> mConbuffer;

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

CMenu* GUI_PushMainMenu_Child(const char *name);
CMenuItem* GUI_PushMainMenu_Item(const char *name);

CMenu* GUI_PushMenu(const char *name);
CMenu* GUI_PushMenuChild(CMenu *parent, const char *childName);
CMenuItem* GUI_PushMenuItem(CMenu *menu, const char *itemName);

#endif