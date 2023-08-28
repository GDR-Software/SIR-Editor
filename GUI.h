#ifndef _GUI_H_
#define _GUI_H_

#pragma once

#include <SDL2/SDL.h>
#include "Map.h"
#include "Editor.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>

#define ERROR 0

class Editor;

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 texcoords;
    glm::vec4 color;
    float empty;
};

class Camera
{
public:
    Camera();
    ~Camera() = default;

    void MoveUp(void);
    void MoveDown(void);
    void MoveLeft(void);
    void MoveRight(void);
    void RotateLeft(void);
    void RotateRight(void);

    void ZoomIn(void);
    void ZoomOut(void);

    void MakeViewMatrix(void);

    inline const glm::mat4& getVPM(void) const
    { return vpm; }
    inline const glm::mat4& getProjection(void) const
    { return proj; }
    inline const glm::mat4& getViewMatrix(void) const
    { return viewMatrix; }
public:
    glm::mat4 vpm;
    glm::mat4 proj, viewMatrix;
    glm::vec3 pos;
    float rotation;
    float zoom;
    float aspectRatio;

    float zoomSpeed;
    float rotationSpeed;
    float moveSpeed;
};

class GUI
{
public:
    GUI();
    ~GUI();

    inline void ResetMouse(void)
    {
        SDL_WarpMouseInWindow(window, (1920 / 2), (1080 / 2));
    }

    void Init(const char *windowName, int width, int height);
    void MessageBox(int level, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
    void BeginFrame(void);
    void PollEvents(Editor *editor);
    void Popup(const char *title, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
    void EndFrame(void);
    void ConvertCoords(Vertex *vertices, const glm::vec2& pos);
    void Print(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
    void DrawMap(const eastl::shared_ptr<Map>& cMap);

    void CheckShader(GLuint id, GLenum type);
    void CheckProgram(void);
    GLuint GenShader(const char *source, GLenum type);
    void InitGLObjects(void);

    inline const char *getImGuiInput(void) const
    { return inputBuf; }
    inline void setConsoleActive(bool active)
    { consoleActive = active; }
    inline bool isConsoleActive(void) const
    { return consoleActive; }

    inline void swapBuffers(void)
    { SDL_GL_SwapWindow(window); }
    inline SDL_Window *getWindow(void)
    { return window; }

    inline const Camera& getCamera(void) const
    { return camera; }
    inline Camera& getCamera(void)
    { return camera; }
    inline SDL_Event *getEvent(void)
    { return &event; }
public:
    int selectedTileY;
    int selectedTileX;

    char inputBuf[4096];
    SDL_Event event;
    SDL_Window *window;
    SDL_GLContext context;
    int windowWidth;
    int windowHeight;
    Camera camera;
    eastl::vector<char> conbuffer;

    glm::mat4 proj, vpm, viewMatrix;
    glm::vec3 cameraPos;
    float rotation;
    float zoomLevel;

    GLuint shaderId;
    GLuint vaoId, vboId, iboId;
    GLint viewProjection;
    GLint selectedTile;

    Vertex *vertices;
    uint32_t *indices;

    eastl::shared_ptr<Map> cMap;
    bool consoleActive;
};

#endif