#define GLAD_GL_IMPLEMENTATION
#include "gln.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "imstb_rectpack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "imstb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include <stb/stb_sprintf.h>
#include "gui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define WINDOW_TITLE "GLNomad Level Editor"
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

Window *gui;
static std::vector<char> conBuffer;

static void GL_CheckError(void)
{
    GLenum error;

    while ((error = glGetError()) != GL_NO_ERROR) {
        Printf("[OpenGL Error] %u", error);
    }
}

static void *ImGui_MemAlloc(size_t n, void *)
{
    return GetMemory(n);
}

static void ImGui_MemFree(void *ptr, void *)
{
    FreeMemory(ptr);
}

static GLuint vaoId, vboId, iboId, shaderId;
static GLint vpmId, checkpointId;

static void MakeViewMatrix(Window *context = gui)
{
    glm::mat4 transpose = glm::translate(glm::mat4(1.0f), context->mCameraPos)
                        * glm::scale(glm::mat4(1.0f), glm::vec3(context->mCameraZoom))
                        * glm::rotate(glm::mat4(1.0f), glm::radians(context->mCameraRotation), glm::vec3(0, 0, 1));
    context->mViewMatrix = glm::inverse(transpose);
    context->mViewProjection = context->mProjection * context->mViewMatrix;
}

static void CheckProgram(void)
{
    int success;
    char str[1024];

    glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        memset(str, 0, sizeof(str));
        glGetProgramInfoLog(shaderId, sizeof(str), NULL, str);

        Error("[Window::CheckProgram] failed to compile and/or link shader program.\n"
                    "glslang error message: %s", str);
    }
}

static void CheckShader(GLuint id, GLenum type)
{
    int success;
    char str[1024];

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        memset(str, 0, sizeof(str));
        glGetShaderInfoLog(id, sizeof(str), NULL, str);
    
        Error("[Window::CheckShader] failed to compile shader of type %s.\nglslang error message: %s",
            (type == GL_VERTEX_SHADER ? "vertex" : type == GL_FRAGMENT_SHADER ? "fragment" : "unknown shader type"), str);
    }
}

static GLuint GenShader(const char *source, GLenum type)
{
    GLuint id;

    id = glCreateShader(type);
    
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    CheckShader(id, type);

    return id;
}

static GLint GetUniform(const char *name)
{
    GLint location = glGetUniformLocation(shaderId, name);
    if (location == -1) {
        Error("Failed to find uniform %s in shader!", name);
    }
    return location;
}

static void GL_ErrorCallback(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam)
{
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        Error("[OpenGL error] %u: %s", id, message);
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        Printf("[OpenGL perfomance] %u: %s", id, message);
        break;
    };
}

#define FRAME_QUADS 0x8000
#define FRAME_VERTICES (FRAME_QUADS*4)
#define FRAME_INDICES (FRAME_QUADS*6)

static void InitGLObjects(void)
{
    GLuint vertid, fragid;

    Printf("[Window::InitGLObjects] Allocating OpenGL buffer objects...");

    const char *vertShader =
    "#version 330 core\n"
    "\n"
    "layout(location = 0) in vec3 a_Position;\n"
    "layout(location = 1) in vec2 a_TexCoords;\n"
    "layout(location = 2) in vec3 a_Color;\n"
    "layout(location = 3) in float a_Alpha;\n"
    "\n"
    "uniform mat4 u_ViewProjection;\n"
    "\n"
    "out vec3 v_Position;\n"
    "out vec2 v_TexCoords;\n"
    "out vec3 v_Color;\n"
    "out float v_Alpha;\n"
    "\n"
    "void main() {\n"
    "   v_Position = a_Position;\n"
    "   v_TexCoords = a_TexCoords;\n"
    "   v_Color = a_Color;\n"
    "   v_Alpha = a_Alpha;\n"
    "   gl_Position = u_ViewProjection * vec4(a_Position, 1.0);\n"
    "}\n";
    const char *fragShader =
    "#version 330 core\n"
    "\n"
    "out vec4 a_Color;\n"
    "\n"
    "in vec3 v_Position;\n"
    "in vec2 v_TexCoords;\n"
    "in vec3 v_Color;\n"
    "in float v_Alpha;\n"
    "\n"
    "void main() {\n"
    "   a_Color.rgb = v_Color.rgb;\n"
    "   a_Color.a = v_Alpha;\n"
    "}\n";

    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboId);
    glGenBuffers(1, &iboId);

    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * FRAME_VERTICES, NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * FRAME_INDICES, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArrayARB(0);
    glVertexAttribPointerARB(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, xyz));

    glEnableVertexAttribArrayARB(1);
    glVertexAttribPointerARB(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, uv));

    glEnableVertexAttribArrayARB(2);
    glVertexAttribPointerARB(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color));

    glEnableVertexAttribArrayARB(3);
    glVertexAttribPointerARB(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(offsetof(Vertex, color) + (sizeof(float) * 3)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Printf("[Window::InitGLObjects] Compiling shaders...");

    vertid = GenShader(vertShader, GL_VERTEX_SHADER);
    fragid = GenShader(fragShader, GL_FRAGMENT_SHADER);

    shaderId = glCreateProgram();

    glAttachShader(shaderId, vertid);
    glAttachShader(shaderId, fragid);
    glLinkProgram(shaderId);
    glValidateProgram(shaderId);

    glUseProgram(shaderId);

    CheckProgram();

    Printf("[Window::InitGLObjects] Cleaning up shaders...");

    glDeleteShader(vertid);
    glDeleteShader(fragid);
    glUseProgram(0);

    vpmId = GetUniform("u_ViewProjection");

    Printf("[Window::InitGLObjects] Finished");
    
    GL_CheckError();
}

static void Clear_f(void)
{
    conBuffer.clear();
}

static void CameraCenter_f(void)
{
    gui->mCameraPos = glm::vec3(0.0f);
}

Window::Window(void)
{
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0) {
        Error("[Window::Init] SDL_Init failed, reason: %s", SDL_GetError());
    }

    Printf("[Window::Init] Setting up GUI");

    mWindowWidth = WINDOW_WIDTH;
    mWindowHeight = WINDOW_HEIGHT;
    mWindow = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_MOUSE_CAPTURE);
    if (!mWindow) {
        Error("[Window::Init] Failed to create SDL2 window, reason: %s", SDL_GetError());
    }
    mContext = SDL_GL_CreateContext(mWindow);
    if (!mContext) {
        Error("[Window::Init] Failed to create SDL_GLContext, reason: %s", SDL_GetError());
    }
    SDL_GL_MakeCurrent(mWindow, mContext);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetSwapInterval(-1);

    Printf("[Window::Init] loading gl procs");

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        Error("Failed to init GLAD2");
    }

    IMGUI_CHECKVERSION();
    
    ImGui::SetAllocatorFunctions(ImGui_MemAlloc, ImGui_MemFree);
    ImGui::CreateContext();

    ImGui_ImplSDL2_InitForOpenGL(mWindow, mContext);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui_ImplOpenGL3_CreateDeviceObjects();
    ImGui_ImplOpenGL3_CreateFontsTexture();

    Printf("[Window::Init] ImGui initialized");
    Printf("[Window::Init] OpenGL initialization done");

    mCameraPos = glm::vec3(0.0f);
    mCameraRotation = 0.0f;
    mCameraZoom = 1.5f;
//    mCameraOffset = glm::vec3( -mWindowWidth / 2 * mCameraZoom, -mWindowHeight / 2 * mCameraZoom, 0.0f );
    mProjection = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, -1.0f, 1.0f);
    MakeViewMatrix(this);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GL_ErrorCallback, NULL);
    uint32_t unusedIds = 0;
    glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, (GLuint *)&unusedIds, GL_TRUE);

    mVertices = (Vertex *)GetClearedMemory(sizeof(*mVertices) * FRAME_VERTICES);
    InitGLObjects();

    Cmd_AddCommand("clear", Clear_f);
    Cmd_AddCommand("cameraCenter", CameraCenter_f);
}

Window::~Window()
{
    FreeMemory(mVertices);
    FreeMemory(mIndices);

    glDeleteVertexArrays(1, &vaoId);
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &iboId);

    glDeleteProgram(shaderId);

    ImGui_ImplSDL2_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();

    SDL_GL_DeleteContext(mContext);
    SDL_DestroyWindow(mWindow);
}

static void ConvertCoords(Vertex *vertices, const glm::vec2& pos)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(16.0f));
    glm::mat4 mvp = gui->mViewProjection * model;

    constexpr glm::vec4 positions[4] = {
        { 0.5f,  0.5f, 0.0f, 1.0f},
        { 0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f,  0.5f, 0.0f, 1.0f},
    };

    for (uint32_t i = 0; i < arraylen(positions); i++) {
        vertices[i].xyz = mvp * positions[i];
    }
}

void Camera_ZoomIn(void)
{
    gui->mCameraZoom -= editor->mConfig->mCameraZoomSpeed;
    if (gui->mCameraZoom < 0.5f)
        gui->mCameraZoom = 0.5f;
}

void Camera_ZoomOut(void)
{
    gui->mCameraZoom += editor->mConfig->mCameraZoomSpeed;
}

void Camera_RotateLeft(void)
{
    gui->mCameraRotation -= editor->mConfig->mCameraRotationSpeed;
}

void Camera_RotateRight(void)
{
    gui->mCameraRotation += editor->mConfig->mCameraRotationSpeed;
}

void Camera_MoveUp(void)
{
    gui->mCameraPos.x += -sin(glm::radians(gui->mCameraRotation)) * editor->mConfig->mCameraMoveSpeed;
    gui->mCameraPos.y += cos(glm::radians(gui->mCameraRotation)) * editor->mConfig->mCameraMoveSpeed;
}

void Camera_MoveDown(void)
{
    gui->mCameraPos.x -= -sin(glm::radians(gui->mCameraRotation)) * editor->mConfig->mCameraMoveSpeed;
    gui->mCameraPos.y -= cos(glm::radians(gui->mCameraRotation)) * editor->mConfig->mCameraMoveSpeed;
}

static void Camera_MoveLeft(void)
{
    gui->mCameraPos.x -= cos(glm::radians(gui->mCameraRotation)) * editor->mConfig->mCameraMoveSpeed;
    gui->mCameraPos.y -= sin(glm::radians(gui->mCameraRotation)) * editor->mConfig->mCameraMoveSpeed;
}

static void Camera_MoveRight(void)
{
    gui->mCameraPos.x += cos(glm::radians(gui->mCameraRotation)) * editor->mConfig->mCameraMoveSpeed;
    gui->mCameraPos.y += sin(glm::radians(gui->mCameraRotation)) * editor->mConfig->mCameraMoveSpeed;
}

static void PollEvents(void)
{
    // update the event queue
    events.EventLoop();

    // camera movement
    if (Key_IsDown(KEY_UP))
        Camera_MoveUp();
    if (Key_IsDown(KEY_DOWN))
        Camera_MoveDown();
    if (Key_IsDown(KEY_RIGHT))
        Camera_MoveRight();
    if (Key_IsDown(KEY_LEFT))
        Camera_MoveLeft();
}

static void DrawMap(void)
{
    uint32_t numVertices, numIndices;
    Vertex *v;
    mapspawn_t *s;
    mapcheckpoint_t *c;

    v = gui->mVertices;
    numVertices = 0;
    numIndices = 0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

    glUseProgram(shaderId);
    glUniformMatrix4fv(vpmId, 1, GL_FALSE, glm::value_ptr(gui->mViewProjection));

    for (uint32_t y = 0; y < mapData.mHeight; y++) {
        for (uint32_t x = 0; x < mapData.mWidth; x++) {
            ConvertCoords(v, { x - (mapData.mWidth * 0.5f), mapData.mHeight - y });

            if (numVertices + 4 >= FRAME_VERTICES) {
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(*v) * numVertices, gui->mVertices);
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32_t) * numIndices, gui->mIndices);
                glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL);
                v = gui->mVertices;
                numVertices = 0;
                numIndices = 0;
            }
   
            for (uint32_t i = 0; i < 4; i++) {
                if (mapData.mTiles[y * mapData.mWidth + x].flags & TILE_CHECKPOINT) {
                    v[i].color[0] = 0.0f;
                    v[i].color[1] = 1.0f;
                    v[i].color[2] = 0.0f;
                    v[i].color[3] = 1.0f;
                }
                else if (mapData.mTiles[y * mapData.mWidth + x].flags & TILE_SPAWN) {
                    v[i].color[0] = 1.0f;
                    v[i].color[1] = 0.0f;
                    v[i].color[2] = 0.0f;
                    v[i].color[3] = 1.0f;
                }
                else {
                    v[i].color[0] = 1.0f;
                    v[i].color[1] = 1.0f;
                    v[i].color[2] = 1.0f;
                    v[i].color[3] = 1.0f;
                }
            }
            v += 4;
            numVertices += 4;
            numIndices += 6;
        }
    }
    if (numVertices || numIndices) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(*v) * numVertices, gui->mVertices);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32_t) * numIndices, gui->mIndices);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL);
    }
    glUseProgram(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    GL_CheckError();
}

void Window::BeginFrame(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    MakeViewMatrix(this);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = WINDOW_WIDTH;
    io.DisplaySize.y = WINDOW_HEIGHT;

    ImGui::NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();

    DrawMap();
}

void Window::Print(const char *fmt, ...)
{
    va_list argptr;
    char msg[4096];
    int length;

    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    conBuffer.insert(conBuffer.end(), msg, msg + length);
    conBuffer.emplace_back('\n');
}

static void PollCommands(const char *input)
{
    if (input[0] == '/' || input[0] == '\\') {
        input++;
    }
    else {
        return; // not a command
    }

    Cmd_ExecuteText(input);
}

void Window::EndFrame(void)
{
    if (editor->mConsoleActive) {
        const ImVec2 windowSize = ImGui::GetWindowSize();
        const ImVec2 windowPos = ImGui::GetWindowPos();

        if (ImGui::Begin("Command Console", &editor->mConsoleActive)) {
            conBuffer.emplace_back('\0');
            ImGui::Text("%s", conBuffer.data());
            conBuffer.pop_back();
            ImGui::Text("> ");
            ImGui::SameLine();

            memset(mInputBuf, 0, sizeof(mInputBuf));
            if (ImGui::InputText(" ", mInputBuf, sizeof(mInputBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                PollCommands(mInputBuf);
            }
        }
        ImGui::End();

        ImGui::SetWindowSize(windowSize);
        ImGui::SetWindowPos(windowPos);
    }
    PollEvents();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(mWindow);
}
