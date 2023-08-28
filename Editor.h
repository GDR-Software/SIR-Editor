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
	Editor();
	~Editor();
	
    static void Init(int argc, char **argv);

	void DrawSettingsMenu(void);

    void DrawFileMenu(void);
    void DrawEditMenu(void);
	void DrawPluginsMenu(void);
	void DrawHelpMenu(void);
	void DrawProjectMenu(void);

	void LoadPreferences(void);
	void SavePreferences(void);

    void Redo(void);
    void Undo(void);
    
    void NewProject(void);
    void OpenProject(void);

	void InitFiles(void);
	void SaveFileCache(void);

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

	inline const std::filesystem::path& getPath(void) const
	{ return curPath; }
	inline const eastl::string& getGamePath(void) const
	{ return gamepath; }

	inline eastl::shared_ptr<Map>& getMap(void)
	{ return cMap; }
	inline const eastl::shared_ptr<Map>& getMap(void) const
	{ return cMap; }
	
	inline eastl::unique_ptr<GUI>& getGUI(void)
	{ return cGUI; }
	inline const eastl::unique_ptr<GUI>& getGUI(void) const
	{ return cGUI; }

	inline eastl::unique_ptr<Project>& getProject(void)
	{ return cProject; }
private:
	eastl::shared_ptr<Map> cMap;
	eastl::unique_ptr<GUI> cGUI;
    eastl::vector<eastl::string> recentFiles;
	eastl::vector<eastl::string> textureFiles;
	eastl::unique_ptr<Project> cProject;

	std::filesystem::path curPath;
	eastl::string gamepath;

	bool settings;
	
	Command *cmdList;
	int editorMode;
	
	static eastl::unique_ptr<Editor> editor;
};

#endif