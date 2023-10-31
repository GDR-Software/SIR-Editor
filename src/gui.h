#ifndef _GUI_H_
#define _GUI_H_

#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define FRAME_QUADS 0x8000
#define FRAME_VERTICES (FRAME_QUADS*4)
#define FRAME_INDICES (FRAME_QUADS*6)

struct Vertex
{
    glm::vec4 color;
    glm::vec3 xyz;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec2 worldPos;
};

class Window
{
public:
    Window(void);
    ~Window();

    static void Print(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

    void EndFrame(void);
    void BeginFrame(void);
    void InitTileMode(void);

    SDL_Window *mWindow;
    SDL_GLContext mContext;
    void *iconBuf;

    Vertex *mVertices;
    uint32_t *mIndices;

    char mInputBuf[4096];

    uint32_t mWindowWidth;
    uint32_t mWindowHeight;

    float mCameraZoom;
    float mCameraZoomInverse;
    float mCameraRotation;
    glm::vec3 mCameraPos;

    glm::mat4 mViewProjection;
    glm::mat4 mProjection;
    glm::mat4 mViewMatrix;
};

extern std::unique_ptr<Window> gui;

INLINE glm::ivec4 RGBAToNormal(const glm::vec4& color)
{
    return glm::ivec4((
        ((uint32_t)(color.a)<<IM_COL32_A_SHIFT) |
        ((uint32_t)(color.b)<<IM_COL32_B_SHIFT) |
        ((uint32_t)(color.g)<<IM_COL32_G_SHIFT) |
        ((uint32_t)(color.a)<<IM_COL32_R_SHIFT))
    );
}

INLINE glm::vec4 NormalToRGBA(const glm::ivec4& color)
{
    const uint32_t rgba =
        color[0] | (color[1] << 8) | (color[2] << 16) | (color[3] << 24);
    return glm::vec4(
        (float)((rgba >> IM_COL32_R_SHIFT) & 0xFF) * (1.0f / 255.0f),
        (float)((rgba >> IM_COL32_G_SHIFT) & 0xFF) * (1.0f / 255.0f),
        (float)((rgba >> IM_COL32_B_SHIFT) & 0xFF) * (1.0f / 255.0f),
        (float)((rgba >> IM_COL32_A_SHIFT) & 0xFF) * (1.0f / 255.0f)
    );
}

INLINE glm::vec3 ScreenToWorldSpace(int mousex, int mousey)
{
    double x = 2.0 * mousex / gui->mWindowWidth - 1;
    double y = 2.0 * mousey / gui->mWindowHeight - 1;

    glm::vec4 screenPos = { x, -y, -1.0f, 1.0f} ;
    glm::vec4 worldPos = gui->mViewProjection * screenPos;

    return glm::vec3(worldPos);
}

void InitGLObjects(void);

#endif