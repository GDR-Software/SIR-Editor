#include "gln.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "imstb_rectpack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "imstb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include <stb/stb_sprintf.h>
#include "gui.h"
#include <glm/gtc/matrix_transform.hpp>

#define WINDOW_TITLE "GLNomad Level Editor"
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

Window *gui;
static vector_t<char> conBuffer;

static void Clear_f(void)
{
    conBuffer.clear();
}

static void *ImGui_MemAlloc(size_t n, void *)
{
    return GetMemory(n);
}

static void ImGui_MemFree(void *ptr, void *)
{
    FreeMemory(ptr);
}

static GLuint vaoId, vboId, shaderId;
static GLint vpmId;

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

    nglGetProgramiv(shaderId, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        memset(str, 0, sizeof(str));
        nglGetProgramInfoLog(shaderId, sizeof(str), NULL, str);

        Error("[Window::CheckProgram] failed to compile and/or link shader program.\n"
                    "glslang error message: %s", str);
    }
}

static void CheckShader(GLuint id, GLenum type)
{
    int success;
    char str[1024];

    nglGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        memset(str, 0, sizeof(str));
        nglGetShaderInfoLog(id, sizeof(str), NULL, str);
    
        Error("[Window::CheckShader] failed to compile shader of type %s.\nglslang error message: %s",
            (type == GL_VERTEX_SHADER ? "vertex" : type == GL_FRAGMENT_SHADER ? "fragment" : "unknown shader type"), str);
    }
}

static GLuint GenShader(const char *source, GLenum type)
{
    GLuint id;

    id = nglCreateShader(type);
    
    nglShaderSource(id, 1, &source, NULL);
    nglCompileShader(id);

    CheckShader(id, type);

    return id;
}

#define NUM_VERTICES 0x80000
#define FRAME_QUADS 0x2000
#define FRAME_VERTICES (FRAME_QUADS*4)
#define FRAME_INDICES (FRAME_QUADS*6)

static void InitGLObjects(void)
{
    GLuint vertid, fragid;

#if 0
    uint32_t offset;
    indices = (uint32_t *)alloca(FRAME_INDICES+1024);

    offset = 0;
    for (uint32_t i = 0; i < FRAME_INDICES; i += 4) {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;

        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;

        offset += 6;
    }
#endif

    const char *vertShader =
    "#version 330 core\n"
    "layout(location = 0) in vec3 a_Position;\n"
    "layout(location = 1) in vec2 a_TexCoords;\n"
    "\n"
    "uniform mat4 u_ViewProjection;\n"
    "out vec2 v_TexCoords;\n"
    "\n"
    "void main() {\n"
    "   v_TexCoords = a_TexCoords;\n"
    "   gl_Position = u_ViewProjection * vec4(a_Position, 1.0);\n"
    "}\n";
    const char *fragShader =
    "#version 330 core\n"
    "out vec4 a_Color;\n"
    "\n"
    "in vec2 v_TexCoords;\n"
    "uniform sampler2D u_Texture;\n"
    "\n"
    "void main() {\n"
    "   a_Color = texture2D(u_Texture, v_TexCoords);\n"
    "}\n";

    Printf("[Window::InitGLObjects] Allocating OpenGL buffer objects...");

    nglGenVertexArrays(1, &vaoId);
    nglGenBuffersARB(1, &vboId);

    nglBindVertexArray(vaoId);
    nglBindBuffer(GL_ARRAY_BUFFER_ARB, vboId);
    nglBufferData(GL_ARRAY_BUFFER_ARB, sizeof(Vertex) * NUM_VERTICES, NULL, GL_DYNAMIC_DRAW);

    nglEnableVertexAttribArray(0);
    nglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos));

    nglEnableVertexAttribArray(1);
    nglVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, texcoords));

    nglBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
    nglBindVertexArray(0);

    Printf("[Window::InitGLObjects] Compiling shaders...");

    vertid = GenShader(vertShader, GL_VERTEX_SHADER);
    fragid = GenShader(fragShader, GL_FRAGMENT_SHADER);

    shaderId = nglCreateProgram();
    nglUseProgram(shaderId);

    nglAttachShader(shaderId, vertid);
    nglAttachShader(shaderId, fragid);
    nglLinkProgram(shaderId);
    nglValidateProgram(shaderId);

    CheckProgram();

    Printf("[Window::InitGLObjects] Cleaning up shaders...");

    nglDeleteShader(vertid);
    nglDeleteShader(fragid);
    nglUseProgram(0);

    vpmId = nglGetUniformLocation(shaderId, "u_ViewProjection");
    if (vpmId == -1) {
        Error("[Window::InitGLObjects] Failed to find uniform u_ViewProjection");
    }
    Printf("[Window::InitGLObjects] Finished");
}

Window::Window(void)
{
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0) {
        Error("[Window::Init] SDL_Init failed, reason: %s", SDL_GetError());
    }

    Printf("[Window::Init] Setting up GUI");

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

    Printf("[Window::Init] loading ngl procs");
    load_gl_procs(SDL_GL_GetProcAddress);
    
    mVertices = (Vertex *)GetMemory(sizeof(*mVertices) * NUM_VERTICES);
    InitGLObjects();

    IMGUI_CHECKVERSION();
    
    ImGui::SetAllocatorFunctions(ImGui_MemAlloc, ImGui_MemFree);
    ImGui::CreateContext();

    ImGui_ImplSDL2_InitForOpenGL(mWindow, mContext);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui_ImplOpenGL3_CreateDeviceObjects();
    ImGui_ImplOpenGL3_CreateFontsTexture();

    Printf("[Window::Init] ImGui initialized");
    Printf("[Window::Init] OpenGL initialization done");

    mProjection = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, -1.0f, 1.0f);
    MakeViewMatrix(this);
}

Window::~Window()
{
    FreeMemory(mVertices);

    nglDeleteVertexArrays(1, &vaoId);
    nglDeleteBuffersARB(1, &vboId);
    nglDeleteProgram(shaderId);

    ImGui_ImplSDL2_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();

    SDL_GL_DeleteContext(mContext);
    SDL_DestroyWindow(mWindow);
}

static void ConvertCoords(Vertex *vertices, const glm::vec2& pos)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
    glm::mat4 mvp = gui->mViewProjection * model;

    constexpr glm::vec4 positions[4] = {
        { 0.5f,  0.5f, 0.0f, 1.0f},
        { 0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f,  0.5f, 0.0f, 1.0f},
    };

    for (uint32_t i = 0; i < 4; i++) {
        vertices[i].pos = mvp * positions[i];
    }
}

void Camera_ZoomIn(void)
{
    gui->mCameraZoom -= editor->mConfig->mCameraZoomSpeed;
    if (gui->mCameraZoom < 3.0f)
        gui->mCameraZoom = 3.0f;
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
    if (Key_IsDown(KEY_N))
        Camera_ZoomIn();
    if (Key_IsDown(KEY_M))
        Camera_ZoomOut();
    if (Key_IsDown(KEY_W))
        Camera_MoveUp();
    if (Key_IsDown(KEY_S))
        Camera_MoveDown();
    if (Key_IsDown(KEY_D))
        Camera_MoveRight();
    if (Key_IsDown(KEY_A))
        Camera_MoveLeft();
#if 0 // works, but no very good
    if (Key_IsDown(KEY_MOUSE_LEFT) && mouse.moving) {
        camera.pos.x += cos(mouse.angle) * (camera.moveSpeed / 2);
        camera.pos.y += sin(mouse.angle) * (camera.moveSpeed / 2);

        mouse.moving = false;
    }
#endif

    // ctrl
//    if (Key_IsDown(KEY_LCTRL) || Key_IsDown(KEY_RCTRL))
//        editor->setModeBits(EDITOR_CTRL);
//    else
//        editor->clearModeBits(EDITOR_CTRL);
//    
//    if (Key_IsDown(KEY_N) && editor->getModeBits() & EDITOR_CTRL)
//        Editor::GetProjManager()->GetCurrent()->New();
//    if (Key_IsDown(KEY_S) && editor->getModeBits() & EDITOR_CTRL)
//        Editor::GetProjManager()->GetCurrent()->Save();
}

void Window::BeginFrame(void)
{
    nglClear(GL_COLOR_BUFFER_BIT);
    nglClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    nglViewport(0, 0, 1980, 1080);

    events.EventLoop();

    MakeViewMatrix();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = 1920;
    io.DisplaySize.y = 1080;

    ImGui::NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
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

static void DrawMap(void)
{
    uint32_t numVertices;
    Vertex *v;

    numVertices = 0;
    v = gui->mVertices;

    nglUseProgram(shaderId);
    nglBindVertexArray(vaoId);
    nglBindBuffer(GL_ARRAY_BUFFER, vboId);
    for (uint32_t y = 0; y < mapData.mHeight; y++) {
        for (uint32_t x = 0; x < mapData.mWidth; x++) {
            ConvertCoords(v, { x - (mapData.mWidth * 0.5f), mapData.mHeight - y });

            if (numVertices + 6 >= NUM_VERTICES) {
                nglBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(*v) * numVertices, gui->mVertices);
                nglDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
                v = gui->mVertices;
                numVertices = 0;
            }

            for (uint32_t i = 0; i < 4; i++, numVertices++) {
            }
        }
    }
    if (numVertices) {
        nglBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(*v) * numVertices, gui->mVertices);
        nglDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
    }
    nglBindBuffer(GL_ARRAY_BUFFER, 0);
    nglBindVertexArray(0);
    nglUseProgram(0);
}

void Window::EndFrame(void)
{
    if (editor->mConsoleActive) {
        const ImVec2 windowSize = ImGui::GetWindowSize();
        const ImVec2 windowPos = ImGui::GetWindowPos();

        ImGui::Begin("Command Console");
        conBuffer.emplace_back('\0');
        ImGui::Text("%s", conBuffer.data());
        conBuffer.pop_back();
        ImGui::Text("> ");
        ImGui::SameLine();

        memset(mInputBuf, 0, sizeof(mInputBuf));
        if (ImGui::InputText(" ", mInputBuf, sizeof(mInputBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
            PollCommands(mInputBuf);
        }
        ImGui::End();

        ImGui::SetWindowSize(windowSize);
        ImGui::SetWindowPos(windowPos);
    }

    DrawMap();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(mWindow);
}
