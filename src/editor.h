#ifndef _EDITOR_H_
#define _EDITOR_H_

#pragma once

#include "gui.h"

class CFileEntry
{
public:
	std::string mPath;
	std::vector<CFileEntry> mDirList;
	bool mIsDir;
	CFileEntry *mParentDir;

	CFileEntry(const std::filesystem::path& path, bool isDir)
		: mPath{ path.c_str() }, mDirList{}, mIsDir{ isDir } { }
	~CFileEntry() { }
};

class CPopup
{
public:
	std::string mName;
	std::string mMsg;
	bool mOpen;

	CPopup(const char *name, const char *msg)
		: mName{ name }, mMsg{ msg }, mOpen{ false } { }
	CPopup(void) { }
	~CPopup() { }
};

typedef enum {
	MODE_MAP,
	MODE_EDIT,
	MODE_TILE,
} editorMode_t;

class CEditor
{
public:
	CEditor(void);
	~CEditor() { }

	static void Init(void);
	void Draw(void);
	void ReloadFileCache(void);
	bool ValidateEntityId(uint32_t id) const;
	static void AddPopup(const CPopup& popup);

	editorMode_t mode;

	std::vector<CFileEntry> mFileCache;
	std::list<CPopup> mPopups;
	bool mConsoleActive;
};

inline const std::filesystem::path pwdString = std::filesystem::current_path();

CFileEntry *Draw_FileList(std::list<CFileEntry>& fileList);
extern std::unique_ptr<CEditor> editor;

inline bool __attribute__((format(printf, 2, 3))) ItemWithTooltip(const char *item_name, const char *fmt, ...)
{
    const bool pressed = ImGui::MenuItem(item_name);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        va_list argptr;

        va_start(argptr, fmt);
        ImGui::SetTooltipV(fmt, argptr);
        va_end(argptr);
    }
    return pressed;
}

inline bool __attribute__((format(printf, 2, 3))) ButtonWithTooltip(const char *item_name, const char *fmt, ...)
{
    const bool pressed = ImGui::Button(item_name);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        va_list argptr;

        va_start(argptr, fmt);
        ImGui::SetTooltipV(fmt, argptr);
        va_end(argptr);
    }
    return pressed;
}

inline bool __attribute__((format(printf, 2, 3))) MenuWithTooltip(const char *item_name, const char *fmt, ...)
{
    const bool pressed = ImGui::BeginMenu(item_name);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        va_list argptr;

        va_start(argptr, fmt);
        ImGui::SetTooltipV(fmt, argptr);
        va_end(argptr);
    }
    return pressed;
}

void Widget_FileMenu(void);
void Widget_EditMenu(void);
void Widget_ViewMenu(void);
void Widget_BuildMenu(void);
void Widgets_Draw(void);

#endif