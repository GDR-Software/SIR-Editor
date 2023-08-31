#ifndef _EDITOR_H_
#define _EDITOR_H_

#pragma once

#include <functional>
#include "Map.h"
#include "GUI.h"
#include "Project.h"

class GUI;

class Command
{
public:
	inline Command(const char *_name, void (*_func)(void))
		: name{_name}, func{_func}
	{
	}
	~Command() = default;
	
    inline void setNext(Command *p)
    { next = p; }
	inline Command *getNext(void)
	{ return next; }
	inline const Command* getNext(void) const
	{ return next; }
	
	inline const eastl::string& getName(void) const
	{ return name; }
	inline void Run(void)
	{ func(); }
private:
	eastl::string name;
	std::function<void(void)> func;
	Command *next;
};

#define EDITOR_SELECT 0x0000
#define EDITOR_WIDGET 0x2000
#define EDITOR_CTRL   0x4000
#define EDITOR_MOVE   0x8000

#define ENTITY_MOB    0x0000
#define ENTITY_PLAYR  0x2000
#define ENTITY_ITEM   0x4000
#define ENTITY_WEAPON 0x8000

class Editor
{
public:
	Editor(void);
	~Editor();
	
    static void Init(int argc, char **argv);
	static void ListFiles(eastl::vector<eastl::string>& fileList, const std::filesystem::path& path,
		const eastl::vector<eastl::string>& exts);
	static bool SaveJSON(const json& data, const eastl::string& path);
	static bool LoadJSON(json& data, const eastl::string& path);
	static inline void SetWindowParms(const ImVec2& windowPos, const ImVec2& windowSize)
	{
		ImGui::SetWindowPos(windowPos);
		ImGui::SetWindowSize(windowSize);
	}
	static inline void GetWindowParms(ImVec2& windowPos, ImVec2& windowSize)
	{
		ImGui::GetWindowPos(windowPos);
		ImGui::GetWindowSize(windowSize);
	}
	static inline void ResetWindowParms(const ImVec2& windowPos, const ImVec2& windowSize)
	{
		ImGui::SetWindowPos(windowPos);
		ImGui::SetWindowSize(windowSize);
	}

    void DrawFileMenu(void);
    void DrawEditMenu(void);
	void DrawToolsMenu(void);
	void DrawPluginsMenu(void);
	void DrawHelpMenu(void);

	void LoadPreferences(void);
	void SavePreferences(void);

    void Redo(void);
    void Undo(void);

    void DrawWidgets(void);
    void PollCommands(void);
	void run(void);
	void registerCommand(const char *name, void (*func)(void));
	Command *findCommand(const char *name);

	inline void setModeBits(int mode)
	{ editorMode |= mode; }
	inline int getModeBits(void) const
	{ return editorMode; }
	inline void clearModeBits(int mode)
	{ editorMode &= ~mode; }
	
	inline static eastl::unique_ptr<Editor>& Get(void)
	{ return editor; }
	inline static const std::filesystem::path& GetPWD(void)
	{ return curPath; }
private:
	eastl::unique_ptr<GUI> cGUI;
	
	Command *cmdList;
	int editorMode;
	
	static std::filesystem::path curPath;
	static eastl::unique_ptr<Editor> editor;
};

#endif