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

    float mCameraZoom;
    float mCameraRotation;
    glm::vec3 mCameraPos;

    glm::mat4 mViewProjection;
    glm::mat4 mProjection;
    glm::mat4 mViewMatrix;
};

extern Window *gui;

#endif