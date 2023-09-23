#ifndef _GUI_H_
#define _GUI_H_

#pragma once

struct Vertex
{
    glm::vec3 xyz;
    glm::vec2 uv;
    glm::vec4 color;
    float flags;
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

    uint32_t mWindowWidth;
    uint32_t mWindowHeight;

    float mCameraZoom;
    float mCameraRotation;
    glm::vec3 mCameraPos;

    glm::mat4 mViewProjection;
    glm::mat4 mProjection;
    glm::mat4 mViewMatrix;
};

extern Window *gui;

INLINE glm::vec3 ScreenToWorldSpace(int mousex, int mousey)
{
    double x = 2.0 * mousex / gui->mWindowWidth - 1;
    double y = 2.0 * mousey / gui->mWindowHeight - 1;

    glm::vec4 screenPos = { x, -y, -1.0f, 1.0f} ;
    glm::vec4 worldPos = gui->mViewProjection * screenPos;

    return glm::vec3(worldPos);
}

#endif