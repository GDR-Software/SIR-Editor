#include "Common.hpp"
#include "Editor.h"
#include "EditorTool.h"

CEditorTool::CEditorTool(void)
{
}

CEditorTool::~CEditorTool()
{
}

bool CEditorTool::Save(const string_t& path) const
{
    Error("CEdtiorTool::Save: called directly");
    return false;
}

bool CEditorTool::Save(void) const
{
    Error("CEdtiorTool::Save: called directly");
    return false;
}

bool CEditorTool::Save(json& data) const
{
    Error("CEdtiorTool::Save: called directly");
    return false;
}

bool CEditorTool::Load(const string_t& path)
{
    Error("CEditorTool::Load: called directly");
    return false;
}

bool CEditorTool::Load(const json& data)
{
    Error("CEditorTool::Load: called directly");
    return false;
}

void CEditorTool::Clear(void)
{
    Error("CEditorTool::Clear: called directly");
}

void CEditorTool::SetPath(const path_t& _path)
{
    path = _path;
    name = GetFilename(path.c_str());
}

void CEditorTool::SetName(const string_t& _name)
{
    name = _name;
    path = BuildOSPath(Editor::GetPWD(), "Data/", name.c_str());
}
