#include "gln.h"

CEditor *editor;

CEditor::CEditor(void)
    : mConsoleActive{ false }
{
}

void CEditor::Init(void)
{
    Mem_Init();
    editor = Allocate<CEditor>();
    gui = Allocate<Window>();
}

void CEditor::Draw(void)
{
    if (ImGui::BeginMainMenuBar()) {
        
    }
}
