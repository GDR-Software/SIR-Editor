#include "Common.hpp"
#include "Tileset.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "imstb_rectpack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "imstb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include <stb/stb_sprintf.h>
#include "GUI.h"
#include "Editor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>

#define NUM_VERTICES (MAX_MAP_HEIGHT*MAX_MAP_WIDTH*4)

GUI::GUI(void)
{
    cameraPos = glm::vec3(0.0f);
    zoomLevel = 15.0f;
    rotation = 0.0f;
    selectedTileX = 0;
    selectedTileY = 0;
}

GUI::~GUI()
{
    Free(vertices);

    ImGui_ImplSDL2_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
}

void GUI::ConvertCoords(Vertex *vertices, const glm::vec2& pos)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
    glm::mat4 mvp = camera.getVPM() * model;

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

Camera::Camera()
{
    pos = glm::vec3(0.0f); // default position
    zoom = 5.0f; // default zoom level

    proj = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f);

    MakeViewMatrix();

    zoomSpeed = 1.0f;
    rotationSpeed = 1.0f;
    moveSpeed = 1.5f;
}

void Camera::ZoomIn(void)
{
    zoom -= zoomSpeed;
    if (zoom < 3.0f)
        zoom = 3.0f;
}

void Camera::ZoomOut(void)
{
    zoom += zoomSpeed;
}

void Camera::RotateLeft(void)
{
    rotation -= rotationSpeed;
}

void Camera::RotateRight(void)
{
    rotation += rotationSpeed;
}

void Camera::MoveUp(void)
{
    pos.x += -sin(glm::radians(rotation)) * moveSpeed;
    pos.y += cos(glm::radians(rotation)) * moveSpeed;
}

void Camera::MoveDown(void)
{
    pos.x -= -sin(glm::radians(rotation)) * moveSpeed;
    pos.y -= cos(glm::radians(rotation)) * moveSpeed;
}

void Camera::MoveLeft(void)
{
    pos.x -= cos(glm::radians(rotation)) * moveSpeed;
    pos.y -= sin(glm::radians(rotation)) * moveSpeed;
}

void Camera::MoveRight(void)
{
    pos.x += cos(glm::radians(rotation)) * moveSpeed;
    pos.y += sin(glm::radians(rotation)) * moveSpeed;
}

void Camera::MakeViewMatrix(void)
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos)
                        * glm::scale(glm::mat4(1.0f), glm::vec3(zoom))
                        * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 0, 1));
        
    viewMatrix = glm::inverse(transform);
    vpm = proj * viewMatrix;
}

void GUI::PollEvents(Editor *editor)
{
    // update the event queue
    EventLoop();

    // camera movement
    if ((mouse.wheelDown || Key_IsDown(KEY_M)) && !(editor->getModeBits() & EDITOR_WIDGET))
        camera.ZoomOut();
    if ((mouse.wheelUp || Key_IsDown(KEY_N)) && !(editor->getModeBits() & EDITOR_WIDGET))
        camera.ZoomIn();
    if (Key_IsDown(KEY_W))
        camera.MoveUp();
    if (Key_IsDown(KEY_S))
        camera.MoveDown();
    if (Key_IsDown(KEY_D))
        camera.MoveRight();
    if (Key_IsDown(KEY_A))
        camera.MoveLeft();
    if (Key_IsDown(KEY_UP)) {
        selectedTileY++;
        if (selectedTileY >= MAX_MAP_HEIGHT) {
            selectedTileY = 0;
        }
    }
    if (Key_IsDown(KEY_DOWN)) {
        selectedTileY--;
        if (!selectedTileY) {
            selectedTileY = MAX_MAP_HEIGHT - 1;
        }
    }
    if (Key_IsDown(KEY_RIGHT)) {
        selectedTileX++;
        if (selectedTileX >= MAX_MAP_WIDTH) {
            selectedTileX = 0;
        }
    }
    if (Key_IsDown(KEY_LEFT)) {
        selectedTileX--;
        if (!selectedTileX) {
            selectedTileX = MAX_MAP_WIDTH - 1;
        }
    }
#if 0 // works, but no very good
    if (Key_IsDown(KEY_MOUSE_LEFT) && mouse.moving) {
        camera.pos.x += cos(mouse.angle) * (camera.moveSpeed / 2);
        camera.pos.y += sin(mouse.angle) * (camera.moveSpeed / 2);

        mouse.moving = false;
    }
#endif

    // ctrl
    if (Key_IsDown(KEY_LCTRL) || Key_IsDown(KEY_RCTRL))
        editor->setModeBits(EDITOR_CTRL);
    else
        editor->clearModeBits(EDITOR_CTRL);
    
    if (Key_IsDown(KEY_N) && editor->getModeBits() & EDITOR_CTRL)
        Editor::GetProjManager()->GetCurrent()->New();
    if (Key_IsDown(KEY_S) && editor->getModeBits() & EDITOR_CTRL)
        Editor::GetProjManager()->GetCurrent()->Save();
}

void GUI::CheckProgram(void)
{
    int success;
    char str[1024];

    glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        memset(str, 0, sizeof(str));
        glGetProgramInfoLog(shaderId, sizeof(str), NULL, str);

        Error("R_CheckProgramStatus: failed to compile and/or link shader program.\n"
                    "glslang error message: %s", str);
    }
}

void GUI::CheckShader(GLuint id, GLenum type)
{
    int success;
    char str[1024];

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        memset(str, 0, sizeof(str));
        glGetShaderInfoLog(id, sizeof(str), NULL, str);
    
        Error("GUI::CheckShader: failed to compile shader of type %s.\nglslang error message: %s",
            (type == GL_VERTEX_SHADER ? "vertex" : type == GL_FRAGMENT_SHADER ? "fragment" : "unknown shader type"), str);
    }
}

GLuint GUI::GenShader(const char *source, GLenum type)
{
    GLuint id;

    id = glCreateShader(type);
    
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    CheckShader(id, type);

    return id;
}

#define FRAME_QUADS 0x2000
#define FRAME_VERTICES (FRAME_QUADS*4)
#define FRAME_INDICES (FRAME_QUADS*6)

void GUI::InitGLObjects(void)
{
    GLuint vertid, fragid;

    uint32_t offset;
    indices = (uint32_t *)alloca(FRAME_INDICES+1024);

#if 0
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
    "layout(location = 2) in float a_Empty;\n"
    "\n"
    "uniform mat4 u_ViewProjection;\n"
    "out vec2 v_TexCoords;\n"
    "out float v_Empty;\n"
    "\n"
    "void main() {\n"
    "   v_Empty = a_Empty;\n"
    "   v_TexCoords = a_TexCoords;\n"
    "   gl_Position = u_ViewProjection * vec4(a_Position, 1.0);\n"
    "}\n";
    const char *fragShader =
    "#version 330 core\n"
    "out vec4 a_Color;\n"
    "\n"
    "in vec2 v_TexCoords;\n"
    "in float v_Empty;\n"
    "uniform bool u_Selected;\n"
    "uniform sampler2D u_Texture;\n"
    "\n"
    "void main() {\n"
    "   a_Color = texture2D(u_Texture, v_TexCoords);\n"
    "   if (v_Empty != 0.0) {\n"
    "       a_Color = vec4(0.0);\n"
    "   }\n"
    "   if (u_Selected) {\n"
    "       a_Color.a += 10.0;\n"
    "   }\n"
    "}\n";

    Printf("Allocating OpenGL buffer objects...");

    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboId);
    glGenBuffers(1, &iboId);

    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_VERTICES, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, texcoords));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, empty));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Printf("Compiling shaders...");

    vertid = GenShader(vertShader, GL_VERTEX_SHADER);
    fragid = GenShader(fragShader, GL_FRAGMENT_SHADER);

    shaderId = glCreateProgram();
    glUseProgram(shaderId);

    glAttachShader(shaderId, vertid);
    glAttachShader(shaderId, fragid);
    glLinkProgram(shaderId);
    glValidateProgram(shaderId);

    CheckProgram();

    Printf("Cleaning up shaders...");

    glDeleteShader(vertid);
    glDeleteShader(fragid);
    glUseProgram(0);

    viewProjection = glGetUniformLocation(shaderId, "u_ViewProjection");
    if (viewProjection == -1) {
        Error("Failed to find uniform u_ViewProjection");
    }
    selectedTile = glGetUniformLocation(shaderId, "u_Selected");
    if (selectedTile == -1) {
        Error("Failed to find uniform u_Selected");
    }

    Printf("Finished");
}

void GUI::BeginFrame(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glViewport(0, 0, 1980, 1080);
    camera.MakeViewMatrix();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = 1920;
    io.DisplaySize.y = 1080;

    ImGui::NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
}


void GUI::EndFrame(void)
{
    if (consoleActive) {
        const ImVec2 windowSize = ImGui::GetWindowSize();
        const ImVec2 windowPos = ImGui::GetWindowPos();

        ImGui::SetWindowSize(ImVec2(720, 1980));
        ImGui::SetWindowPos(ImVec2(0, 0));

        ImGui::Begin("Command Console");
        conbuffer.emplace_back('\0');
        ImGui::Text("%s", conbuffer.data());
        conbuffer.pop_back();
        ImGui::Text("> ");
        ImGui::SameLine();
        if (!ImGui::InputText(" ", inputBuf, sizeof(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
            *inputBuf = 0;
        }
        ImGui::End();

        ImGui::SetWindowSize(windowSize);
        ImGui::SetWindowPos(windowPos);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
}

void GUI::DrawMap(const CMap *cMap)
{
    bool empty;
    uint32_t numVertices, index;
    Vertex verts[4];
    constexpr glm::vec2 emptyCoords[4] = {
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        {0.0f, 0.0f}
    };

    numVertices = 0;

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_2D);

    glUseProgram(shaderId);
    glUniformMatrix4fv(viewProjection, 1, GL_FALSE, glm::value_ptr(camera.getVPM()));

    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    for (uint32_t y = 0; y < MAX_MAP_HEIGHT; y++) {
        for (uint32_t x = 0; x < MAX_MAP_WIDTH; x++) {
            ConvertCoords(verts, { x - (MAX_MAP_WIDTH * 0.5f), MAX_MAP_HEIGHT - y });
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

static void *ImGui_MemAlloc(size_t size, void *)
{
    return Malloc(size);
}

static void ImGui_MemFree(void *ptr, void *)
{
    Free(ptr);
}

void GUI::Init(const char *windowName, int width, int height)
{
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0) {
        Error("SDL_Init failed, reason: %s", SDL_GetError());
    }

    Printf("Setting up GUI");

    window = SDL_CreateWindow(windowName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_MOUSE_CAPTURE);
    if (!window) {
        Error("Failed to create SDL2 window, reason: %s", SDL_GetError());
    }
    context = SDL_GL_CreateContext(window);
    if (!context) {
        Error("Failed to create SDL_GLContext, reason: %s", SDL_GetError());
    }
    SDL_GL_MakeCurrent(window, context);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetSwapInterval(-1);

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        Error("Failed to init GLAD");
    }
    
    vertices = (Vertex *)Malloc(sizeof(*vertices) * NUM_VERTICES);
    InitGLObjects();
    ResetMouse();

    IMGUI_CHECKVERSION();
    
    ImGui::SetAllocatorFunctions(ImGui_MemAlloc, ImGui_MemFree);

    ImGui::CreateContext();

    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui_ImplOpenGL3_CreateDeviceObjects();
    ImGui_ImplOpenGL3_CreateFontsTexture();

    Printf("ImGui initialized");
    Printf("OpenGL initialization done");

    consoleActive = false;
}
