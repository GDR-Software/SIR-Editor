#ifndef _EDITOR_H_
#define _EDITOR_H_

#pragma once

#include <functional>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "GUI.h"
#include "EditorManager.h"
#include "ProjectManager.h"
#include "MapManager.h"
#include "TilesetManager.h"
#include "TextureManager.h"

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
	
	inline const string_t& getName(void) const
	{ return name; }
	inline void Run(void)
	{ func(); }
private:
	string_t name;
	void (*func)(void);
	Command *next;
};

#define EDITOR_SELECT 0x0000
#define EDITOR_WIDGET 0x2000
#define EDITOR_CTRL   0x4000
#define EDITOR_MOVE   0x8000
#define EDITOR_WIZARD 0x0200

#define ENTITY_MOB    0x0000
#define ENTITY_PLAYR  0x2000
#define ENTITY_ITEM   0x4000
#define ENTITY_WEAPON 0x8000

struct Popup
{
	const char *title;
	const char *message;
	const char *confirmation;
	bool *yesno;

	inline constexpr Popup(const char *_title, const char *msg, const char *_confirmation, bool *_yesno)
		: title{_title}, message{msg}, confirmation{_confirmation}, yesno{_yesno} {}
};

class Editor
{
public:
	Editor(void);
	~Editor();
	
    static void Init(int argc, char **argv);
	static void ListFiles(vector_t<path_t>& fileList, const path_t& path, const vector_t<const char *>& exts);
	static bool SaveJSON(const json& data, const path_t& path);
	static bool LoadJSON(json& data, const path_t& path);
	static void CheckExtension(path_t& path, const char *ext);
	static inline void SetWindowParms(const ImVec2& windowPos, const ImVec2& windowSize)
	{
		ImGui::SetWindowPos(windowPos);
		ImGui::SetWindowSize(windowSize);
	}
	static inline void GetWindowParms(ImVec2& windowPos, ImVec2& windowSize)
	{
		windowPos = ImGui::GetWindowPos();
		windowSize = ImGui::GetWindowSize();
	}
	static inline void ResetWindowParms(const ImVec2& windowPos, const ImVec2& windowSize)
	{
		ImGui::SetWindowPos(windowPos);
		ImGui::SetWindowSize(windowSize);
	}

    void DrawFileMenu(void);
    void DrawEditMenu(void);
	void DrawWizardMenu(void);
	void DrawPluginsMenu(void);
	void DrawHelpMenu(void);

	void LoadPreferences(void);
	void SavePreferences(void);

    void Redo(void);
    void Undo(void);

	void DrawPopups(void);
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

	inline static object_ptr_t<CMap>& GetMap(void)
	{ return editor->cMap; }
	inline static object_ptr_t<CProject>& GetProject(void)
	{ return editor->cProject; }
	inline static object_ptr_t<CTileset>& GetTileset(void)
	{ return editor->cTileset; }
	inline static object_ptr_t<CProjectManager>& GetProjManager(void)
	{ return editor->cProjectManager; }
	inline static object_ptr_t<CMapManager>& GetMapManager(void)
	{ return editor->cMapManager; }
	inline static object_ptr_t<CTilesetManager>& GetTilesetManager(void)
	{ return editor->cTilesetManager; }
	inline static object_ptr_t<CTextureManager>& GetTextureManager(void)
	{ return editor->cTextureManager; }
	inline static object_ptr_t<Editor>& Get(void)
	{ return editor; }
	inline static object_ptr_t<GUI>& GetGUI(void)
	{ return editor->cGUI; }
	inline static const std::filesystem::path& GetPWD(void)
	{ return curPath; }
	static bool IsAllocated(void);

	static inline void PushPopup(const Popup& p)
	{ editor->popups.emplace_back(p); }

	template<typename T>
	inline object_ptr_t<T> AddManager(const eastl::string& name)
	{
		object_ptr_t<T> mem = Allocate<T>();
		managerList.try_emplace(name);
		managerList.at(name) = dynamic_cast<CEditorManager*>(mem);
		return mem;
	}
	template<typename T>
	inline object_ptr_t<T> GetManager(const eastl::string& name)
	{ return dynamic_cast<T*>(managerList.at(name)); }
private:
	struct FileEntry
	{
		vector_t<FileEntry> DirList;
	    path_t path;
	    bool isDirectory;
	};

	void RecursiveDirectoryIterator(const path_t& path, vector_t<FileEntry>& dirList, uint32_t& depth);
	void DisplayFileCache(const vector_t<FileEntry>& cache, uint32_t& depth);
	void InitFiles(void);
	void ReloadFiles(void);
	void SaveFileCache(void);
	void LoadFileCache(void);

	object_ptr_t<CProject> cProject;
	object_ptr_t<CMap> cMap;
	object_ptr_t<CTileset> cTileset;
	object_ptr_t<GUI> cGUI;

	vector_t<FileEntry> fileCache;
	string_hash_t<object_ptr_t<CEditorManager>> managerList;

	vector_t<directory_entry_t> entryCache;

	object_ptr_t<CProjectManager> cProjectManager;
	object_ptr_t<CTilesetManager> cTilesetManager;
	object_ptr_t<CMapManager> cMapManager;
	object_ptr_t<CTextureManager> cTextureManager;

	vector_t<Popup> popups;
	Popup *curPopup;
	
	int editorMode;
	
	static path_t curPath;
	static object_ptr_t<Editor> editor;
};

struct EditorPreferences
{

};

#endif