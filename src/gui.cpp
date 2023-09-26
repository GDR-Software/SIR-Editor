#define GLAD_GL_IMPLEMENTATION
#include "gln.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "imstb_rectpack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "imstb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include <stb/stb_sprintf.h>
#include "gui.h"

struct VertexAttrib
{
    uint32_t index;
    uint32_t count;
    uint32_t type;
    uint32_t offset;

    VertexAttrib(uint32_t _index, uint32_t _count, uint32_t _type, uint32_t _offset)
        : index{ _index }, count{ _count }, type{ _type }, offset{ _offset } { }
};

class VertexCache
{
public:
    GLuint mVaoId, mVboId, mIboId;
    std::vector<VertexAttrib> mAttribs;
    uint32_t mDataStride;

    VertexCache(uint64_t numVertices, uint64_t numIndices, uint32_t dataSize) { InitBase(numVertices, numIndices, dataSize); }
    ~VertexCache()
    {
        glDeleteVertexArrays(1, &mVaoId);
        glDeleteBuffers(1, &mVboId);
        glDeleteBuffers(1, &mIboId);
    }

    void ClearAttribs(void);
    void SetAttribs(uint32_t stride, const std::initializer_list<VertexAttrib>& attribList);
    void InitBase(uint64_t numVertices, uint64_t numIndices, uint32_t dataSize);
    void Draw(const uint32_t *indices, const void *vertices, uint64_t numIndices, uint64_t numVertices) const;
};


struct BindCache
{
    INLINE BindCache(const VertexCache *cache)
    { Bind(cache); }
    INLINE ~BindCache()
    { Unbind(); }

    INLINE void Bind(const VertexCache *cache) const
    {
        glBindVertexArray(cache->mVaoId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cache->mIboId);
        glBindBuffer(GL_ARRAY_BUFFER, cache->mVboId);
    }
    INLINE void Unbind(void) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};


#define WINDOW_TITLE "GLNomad Level Editor"
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

std::unique_ptr<Window> gui;
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

static VertexCache *mapCache;
static GLuint uboId, shaderId;
static GLint vpmId, lightsId;

static void MakeViewMatrix(void)
{
    glm::mat4 transpose = glm::translate(glm::mat4(1.0f), gui->mCameraPos)
                        * glm::scale(glm::mat4(1.0f), glm::vec3(gui->mCameraZoom))
                        * glm::rotate(glm::mat4(1.0f), glm::radians(gui->mCameraRotation), glm::vec3(0, 0, 1));
    gui->mViewMatrix = glm::inverse(transpose);
    gui->mViewProjection = gui->mProjection * gui->mViewMatrix;
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

struct Light {
    float birightness;
    vec3_t origin;
    vec3_t color;
};

#define FRAME_QUADS 0x8000
#define FRAME_VERTICES (FRAME_QUADS*4)
#define FRAME_INDICES (FRAME_QUADS*6)

GLuint blockIndex;
static Light lights[MAX_MAP_LIGHTS];

static void InitUBO(void)
{
    blockIndex = glGetUniformBlockIndex(shaderId, "u_Lights");
    glUniformBlockBinding(shaderId, blockIndex, 0);

    glGenBuffers(1, &uboId);
    glBindBuffer(GL_UNIFORM_BUFFER, uboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * MAX_MAP_LIGHTS, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboId);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboId, 0, sizeof(Light) * MAX_MAP_LIGHTS);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

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
    "#define MAX_MAP_LIGHTS 1024"
    "\n"
    "out vec4 a_Color;\n"
    "\n"
    "struct Light {\n"
    "   float brightness;\n"
    "   vec3 origin;\n"
    "   vec3 color;\n"
    "};\n"
    "\n"
    "uniform int u_numLights;\n"
    "uniform sampler2D u_SpriteSheet;\n"
    "uniform float u_AmbientLight;\n"
    "layout(std140) uniform u_Lights {\n"
    "   Light lights[MAX_MAP_LIGHTS];\n"
    "};\n"
    "\n"
    "in vec3 v_Position;\n"
    "in vec2 v_TexCoords;\n"
    "in vec3 v_Color;\n"
    "in float v_Alpha;\n"
    "\n"
    "vec4 applyLights() {\n"
    "   vec4 color;\n"
    "   for (int i = 0; i < u_numLights; i++) {\n"
    "       color.rgb = color.rgb * 1.0 / distance(lights[i].origin, v_Position);\n"
    "       color.rgb += lights[i].brightness;"
    "   }\n"
    "   color.a += u_AmbientLight;\n"
    "   return color;\n"
    "}\n"
    "vec4 toGreyscale(in vec4 color) {\n"
    "   float avg = (color.r + color.g + color.b) / 3.0;\n"
    "   return vec4(avg, avg, avg, 1.0);\n"
    "}\n"
    "vec4 colorize(in vec4 greyscale, in vec4 color) {\n"
    "   return (greyscale * color);\n"
    "}\n"
    "\n"
    "void main() {\n"
    "   a_Color.rgb = v_Color.rgb;\n"
    "   a_Color.a = v_Alpha;\n"
    "   if (a_Color.rgb == vec3(1.0) && v_Alpha == 1.0) {\n"
    "   }\n"
    "   if (v_Alpha != 1.0) {\n"
    "      vec4 greyscale = toGreyscale(a_Color);\n"
    "      a_Color = colorize(greyscale, vec4(0.0, 0.0, 0.5, 1.0));\n"
    "   }\n"
    "}\n";

    mapCache = new VertexCache(MAX_MAP_VERTICES, MAX_MAP_INDICES, sizeof(mapvert_t));
    mapCache->SetAttribs(sizeof(mapvert_t), {
        // index, count, type, offset
        VertexAttrib( 0, 3, GL_FLOAT, (uint32_t)offsetof(mapvert_t, xyz) ),
        VertexAttrib( 1, 2, GL_FLOAT, (uint32_t)offsetof(mapvert_t, uv) ),
        VertexAttrib( 2, 3, GL_FLOAT, (uint32_t)offsetof(mapvert_t, color) ),
        VertexAttrib( 3, 1, GL_FLOAT, (uint32_t)(offsetof(mapvert_t, color) + (sizeof(vec_t) * 3)) )
    });
    
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

    InitUBO();

    Printf("[Window::InitGLObjects] Cleaning up shaders...");

    glDeleteShader(vertid);
    glDeleteShader(fragid);
    glUseProgram(0);

    vpmId = GetUniform("u_ViewProjection");
    lightsId = glGetUniformBlockIndex(shaderId, "u_Lights");
    glUniformBlockBinding(shaderId, lightsId, 0);


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

typedef struct {
    int curX, curY;
    bool active;
} tileMode_t;
tileMode_t tileMode;

static void TileModeInfo_f(void)
{
    Printf("Tile Current X: %i", tileMode.curX);
    Printf("Tile Current Y: %i", tileMode.curY);
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

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GL_ErrorCallback, NULL);
    uint32_t unusedIds = 0;
    glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, (GLuint *)&unusedIds, GL_TRUE);

    InitGLObjects();

    Cmd_AddCommand("clear", Clear_f);
    Cmd_AddCommand("cameraCenter", CameraCenter_f);
    Cmd_AddCommand("tilemodeInfo", TileModeInfo_f);
}

Window::~Window()
{
    FreeMemory(mVertices);
    FreeMemory(mIndices);

    delete mapCache;

    glDeleteProgram(shaderId);

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

    for (uint32_t i = 0; i < arraylen(positions); i++) {
        vertices[i].xyz = mvp * positions[i];
    }
}

void Camera_ZoomIn(void)
{
    gui->mCameraZoom -= gameConfig->mCameraZoomSpeed;
    if (gui->mCameraZoom < 0.5f)
        gui->mCameraZoom = 0.5f;
}

void Camera_ZoomOut(void)
{
    gui->mCameraZoom += gameConfig->mCameraZoomSpeed;
}

void Camera_RotateLeft(void)
{
    gui->mCameraRotation -= gameConfig->mCameraRotationSpeed;
}

void Camera_RotateRight(void)
{
    gui->mCameraRotation += gameConfig->mCameraRotationSpeed;
}

void Camera_MoveUp(void)
{
    gui->mCameraPos.x += -sin(glm::radians(gui->mCameraRotation)) * gameConfig->mCameraMoveSpeed;
    gui->mCameraPos.y += cos(glm::radians(gui->mCameraRotation)) * gameConfig->mCameraMoveSpeed;
}

void Camera_MoveDown(void)
{
    gui->mCameraPos.x -= -sin(glm::radians(gui->mCameraRotation)) * gameConfig->mCameraMoveSpeed;
    gui->mCameraPos.y -= cos(glm::radians(gui->mCameraRotation)) * gameConfig->mCameraMoveSpeed;
}

static void Camera_MoveLeft(void)
{
    gui->mCameraPos.x -= cos(glm::radians(gui->mCameraRotation)) * gameConfig->mCameraMoveSpeed;
    gui->mCameraPos.y -= sin(glm::radians(gui->mCameraRotation)) * gameConfig->mCameraMoveSpeed;
}

static void Camera_MoveRight(void)
{
    gui->mCameraPos.x += cos(glm::radians(gui->mCameraRotation)) * gameConfig->mCameraMoveSpeed;
    gui->mCameraPos.y += sin(glm::radians(gui->mCameraRotation)) * gameConfig->mCameraMoveSpeed;
}

void Window::InitTileMode(void)
{
    memset(&tileMode, 0, sizeof(tileMode));
    tileMode.active = true;
}

static void PollEvents(void)
{
    // update the event queue
    events.EventLoop();

    // camera movement
    if (editor->mode != MODE_TILE) {
        if (Key_IsDown(KEY_UP))
            Camera_MoveUp();
        if (Key_IsDown(KEY_DOWN))
            Camera_MoveDown();
        if (Key_IsDown(KEY_RIGHT))
            Camera_MoveRight();
        if (Key_IsDown(KEY_LEFT))
            Camera_MoveLeft();
    }
    else {
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, false))
            tileMode.curY--;
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, false))
            tileMode.curY++;
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, false))
            tileMode.curX++;
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false))
            tileMode.curX--;
        
        if (tileMode.curX < 0)
            tileMode.curX = mapData->mWidth - 1;
        else if (tileMode.curX >= mapData->mWidth)
            tileMode.curX = 0;
        if (tileMode.curY < 0)
            tileMode.curY = mapData->mHeight - 1;
        else if (tileMode.curY >= mapData->mHeight)
            tileMode.curY = 0;
    }
}

static void DrawMap(void)
{
    uint32_t numVertices, numIndices;
    mapvert_t *v;
    mapspawn_t *s;
    mapcheckpoint_t *c;
    BindCache bind{mapCache};

    numVertices = 0;
    numIndices = 0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(shaderId);
    glUniformMatrix4fv(vpmId, 1, GL_FALSE, glm::value_ptr(gui->mViewProjection));

    glBindBuffer(GL_UNIFORM_BUFFER, uboId);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * MAX_MAP_LIGHTS, lights);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    mapData->RedoDrawData();
    mapCache->Draw(mapData->mIndices.data(), mapData->mVertices.data(), mapData->mIndices.size(), mapData->mVertices.size());

#if 0
    for (uint32_t y = 0; y < mapData->mHeight; y++) {
        for (uint32_t x = 0; x < mapData->mWidth; x++) {
            ConvertCoords(v, { x - (mapData->mWidth * 0.5f), mapData->mHeight - y });

            if (numVertices + 4 >= FRAME_VERTICES) {
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(*v) * numVertices, gui->mVertices);
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32_t) * numIndices, gui->mIndices);
                glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL);
                v = gui->mVertices;
                numVertices = 0;
                numIndices = 0;
            }
   
            for (uint32_t i = 0; i < 4; i++) {
                if (mapData->mTiles[y * mapData->mWidth + x].flags & TILE_CHECKPOINT) {
                    v[i].color[0] = 0.0f;
                    v[i].color[1] = 1.0f;
                    v[i].color[2] = 0.0f;
                    v[i].color[3] = 1.0f;
                }
                else if (mapData->mTiles[y * mapData->mWidth + x].flags & TILE_SPAWN) {
                    v[i].color[0] = 1.0f;
                    v[i].color[1] = 0.0f;
                    v[i].color[2] = 0.0f;
                    v[i].color[3] = 0.5f;
                }
                else {
                    v[i].color[0] = 1.0f;
                    v[i].color[1] = 1.0f;
                    v[i].color[2] = 1.0f;
                    v[i].color[3] = 1.0f;
                }

                if (editor->mode == MODE_TILE && tileMode.curX == x && tileMode.curY == y) {
                    v[i].color[3] = 0.0f;
                }
            }
            v += 4;
            numVertices += 4;
            numIndices += 6;
        }
    }
#endif
    glUseProgram(0);
    GL_CheckError();
}

void Window::BeginFrame(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    MakeViewMatrix();

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

void VertexCache::ClearAttribs(void)
{
    for (uint64_t i = 0; i < mAttribs.size(); i++) {
        glDisableVertexAttribArray(i);
    }
    mAttribs.clear();
}

void VertexCache::SetAttribs(uint32_t stride, const std::initializer_list<VertexAttrib>& attribList)
{
    mAttribs = attribList;
    mDataStride = stride;

    for (uint64_t i = 0; i < mAttribs.size(); i++) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, mAttribs[i].count, mAttribs[i].type, GL_FALSE, mDataStride, (const void *)(uintptr_t)mAttribs[i].offset);
    }
}

void VertexCache::InitBase(uint64_t numVertices, uint64_t numIndices, uint32_t dataSize)
{
    mDataStride = dataSize;

    glGenVertexArrays(1, &mVaoId);
    glGenBuffers(1, &mVboId);
    glGenBuffers(1, &mIboId);

    glBindVertexArray(mVaoId);
    glBindBuffer(GL_ARRAY_BUFFER, mVboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIboId);

    glBufferData(GL_ARRAY_BUFFER, dataSize * numVertices, NULL, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * numIndices, NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void VertexCache::Draw(const uint32_t *indices, const void *vertices, uint64_t numIndices, uint64_t numVertices) const
{
    glBufferSubData(GL_ARRAY_BUFFER, 0, mDataStride * numVertices, vertices);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32_t) * numIndices, indices);

    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL);
}
