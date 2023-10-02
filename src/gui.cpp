#define GLAD_GL_IMPLEMENTATION
#include "gln.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "imstb_rectpack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "imstb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include <stb/stb_sprintf.h>
#include "gui.h"
#include "stb_image.h"
#include <SDL2/SDL_image.h>

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

static GLuint vaoId, vboId, iboId;
static GLuint shaderId;
static std::unordered_map<std::string, GLint> uniformCache;

#define FRAME_QUADS 0x8000
#define FRAME_VERTICES (FRAME_QUADS*4)
#define FRAME_INDICES (FRAME_QUADS*6)

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

static GLuint GenShader(const char **source, uint32_t numSources, GLenum type)
{
    GLuint id;

    id = glCreateShader(type);
    
    glShaderSource(id, numSources, source, NULL);
    glCompileShader(id);

    CheckShader(id, type);

    return id;
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

static INLINE glm::vec3 CalcNormal(const Vertex *quad)
{
    return glm::normalize(quad[0].xyz + quad[1].xyz + quad[2].xyz + quad[3].xyz);
}

static qboolean printableChar( char c ) {
	if ( ( c >= ' ' && c <= '~' ) || c == '\n' || c == '\r' || c == '\t' )
		return qtrue;
	else
		return qfalse;
}

static std::string LoadStringFile(const char *path)
{
    FILE *fp;
    std::string buf;

    fp = SafeOpenRead(path);
    buf.resize(FileLength(fp));
    SafeRead(buf.data(), buf.size(), fp);
    fclose(fp);

    return buf;
}

void InitGLObjects(void)
{
    GLuint vertid, fragid;
    const char *str;
    const std::string vs = LoadStringFile(va("%sbasic.glsl.vs", gameConfig->mEditorPath.c_str()));
    const std::string fs = LoadStringFile(va("%sbasic.glsl.fs", gameConfig->mEditorPath.c_str()));

    Printf("[Window::InitGLObjects] Allocating OpenGL buffer objects...");

    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboId);
    glGenBuffers(1, &iboId);

    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * FRAME_VERTICES, NULL, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * FRAME_INDICES, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, xyz));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, uv));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(offsetof(Vertex, color) + (sizeof(vec_t) * 3)));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, worldPos));

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, normal));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Printf("[Window::InitGLObjects] Compiling shaders...");

    str = vs.c_str();
    vertid = GenShader(&str, 1, GL_VERTEX_SHADER);
    str = fs.c_str();
    fragid = GenShader(&str, 1, GL_FRAGMENT_SHADER);

    shaderId = glCreateProgram();

    glAttachShader(shaderId, vertid);
    glAttachShader(shaderId, fragid);
    glLinkProgram(shaderId);
    glValidateProgram(shaderId);

    glUseProgram(shaderId);

//    InitUBO();

    CheckProgram();

    Printf("[Window::InitGLObjects] Cleaning up shaders...");

    glDeleteShader(vertid);
    glDeleteShader(fragid);
    glUseProgram(0);

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
    uint32_t offset, i;
    int width, height, channels;

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

    // load and set the icon
    SDL_RWops *rw = SDL_RWFromFile("icon.png", "rb");
    SDL_Surface *icon = IMG_LoadPNG_RW(rw);
    SDL_SetWindowIcon(mWindow, icon);
    SDL_RWclose(rw);
    SDL_FreeSurface(icon);

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

    mVertices = (Vertex *)GetMemory(sizeof(*mVertices) * FRAME_VERTICES);
    mIndices = (uint32_t *)GetMemory(sizeof(*mIndices) * FRAME_INDICES);

    offset = 0;
    for (i = 0; i < FRAME_INDICES; i += 6) {
        mIndices[i + 0] = offset + 0;
        mIndices[i + 1] = offset + 1;
        mIndices[i + 2] = offset + 2;

        mIndices[i + 3] = offset + 3;
        mIndices[i + 4] = offset + 2;
        mIndices[i + 5] = offset + 0;

        offset += 4;
    }

    Cmd_AddCommand("clear", Clear_f);
    Cmd_AddCommand("cameraCenter", CameraCenter_f);
    Cmd_AddCommand("tilemodeInfo", TileModeInfo_f);
}

Window::~Window()
{
    if (mVertices)
        FreeMemory(mVertices);
    if (mIndices)
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

static GLuint curIbo;

static void DrawElements(uint64_t numIndices)
{
    SDL_GL_MakeCurrent(gui->mWindow, gui->mContext);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL);
}

static GLint GetUniform(const std::string& name)
{
    const auto& it = uniformCache.find(name);
    if (it != uniformCache.end()) {
        if (it->second == -1) { // try again
            GLint location = glGetUniformLocation(shaderId, name.c_str());
            if (location == -1) {
                return -1;
            }
            it->second = location;
            return location;
        }
        else {
            return it->second;
        }
    }

    GLint location = glGetUniformLocation(shaderId, name.c_str());
    if (location == -1) {
        // save it so that we only get the warning once and it doesn't fill up the log with random shit
        Printf("WARNING: failed to find uniform '%s'", name.c_str());
    }
    uniformCache.try_emplace(name, location);
    return location;
}

static void DrawMap(void)
{
    uint32_t numVertices, numIndices;
    Vertex *v;
    mapspawn_t *s;
    mapcheckpoint_t *c;

    numVertices = 0;
    numIndices = 0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

    glUseProgram(shaderId);
    glUniform1i(GetUniform("u_numLights"), mapData->mLights.size());
    for (uint32_t i = 0; i < mapData->mLights.size(); ++i) {
        glUniform1f(GetUniform(va("lights[%i].brightness", i)), mapData->mLights[i].brightness);
        glUniform2f(GetUniform(va("lights[%i].origin", i)), mapData->mLights[i].origin[0], mapData->mLights[i].origin[1]);
        glUniform3f(GetUniform(va("lights[%i].color", i)), mapData->mLights[i].color[0], mapData->mLights[i].color[1],
            mapData->mLights[i].color[2]);
    }
    glUniformMatrix4fv(GetUniform("u_ViewProjection"), 1, GL_FALSE, glm::value_ptr(gui->mViewProjection));
    glUniform3f(GetUniform("u_AmbientColor"), mapData->mAmbientColor.r, mapData->mAmbientColor.g, mapData->mAmbientColor.b);
    glUniform1f(GetUniform("u_AmbientIntensity"), mapData->mAmbientIntensity);
    glUniform1i(GetUniform("u_SpriteSheet"), 0);

    glActiveTexture(GL_TEXTURE0);
    project->texData->Bind();

    v = gui->mVertices;
    for (uint32_t y = 0; y < mapData->mHeight; y++) {
        for (uint32_t x = 0; x < mapData->mWidth; x++) {
            ConvertCoords(v, { x - (mapData->mWidth * 0.5f), mapData->mHeight - y });

            if (numVertices + 4 >= FRAME_VERTICES) {
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * numVertices, gui->mVertices);
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32_t) * numIndices, gui->mIndices);
                glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL);
                v = gui->mVertices;
                mapData->CalcLighting(v, FRAME_VERTICES);
                numVertices = 0;
                numIndices = 0;
            }
   
            for (uint32_t i = 0; i < 4; i++) {
                v[i].uv[0] = mapData->mTiles[y * mapData->mWidth + x].texcoords[i][0];
                v[i].uv[1] = mapData->mTiles[y * mapData->mWidth + x].texcoords[i][1];
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
                v[i].worldPos = { x, y };
            }
            CalcVertexNormals(v);
            v += 4;
            numVertices += 4;
            numIndices += 6;
        }
    }
    if (numVertices || numIndices) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * numVertices, gui->mVertices);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32_t) * numIndices, gui->mIndices);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL);
        v = gui->mVertices;
        numVertices = 0;
        numIndices = 0;
    }

    if (project->texData->mId != 0) {
        project->texData->Unbind();
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

static void DrawGraph(void)
{
    const uint64_t width = gui->mWindowWidth * 0.5f;
    const uint64_t height = gui->mWindowHeight * 0.5f;
    Vertex *v;
    uint32_t numVertices, numIndices;

    numVertices = 0;
    numIndices = 0;
    v = gui->mVertices;

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    for (uint64_t y = 0; y < height; ++y) {
        for (uint64_t x = 0; x < width; ++x) {
            ConvertCoords(v, { x, y });

            if (numVertices + 4 >= FRAME_VERTICES) {
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * numVertices, gui->mVertices);
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32_t) * numIndices, gui->mIndices);
                glDrawElements(GL_LINES, numIndices, GL_UNSIGNED_INT, NULL);
                v = gui->mVertices;
                numVertices = 0;
                numIndices = 0;
            }
            
            numIndices += 6;
            numVertices += 4;
            v += 4;
        }
    }
    if (numVertices || numIndices) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * numVertices, gui->mVertices);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32_t) * numIndices, gui->mIndices);
        glDrawElements(GL_LINES, numIndices, GL_UNSIGNED_INT, NULL);
        v = gui->mVertices;
        numVertices = 0;
        numIndices = 0;
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
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
//    DrawGraph();
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
        glEnableVertexAttribArray(mAttribs[i].index);
        glVertexAttribPointer(mAttribs[i].index, mAttribs[i].count, mAttribs[i].type, GL_FALSE, mDataStride, (const void *)(uintptr_t)mAttribs[i].offset);
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
