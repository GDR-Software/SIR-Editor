#ifndef _EDITOR_H_
#define _EDITOR_H_

#pragma once

#include "gui.h"

class CFileEntry
{
public:
	Str mPath;
	std::list<CFileEntry> mDirList;
	const bool mIsDir;

	CFileEntry(const std::filesystem::path& path, bool isDir)
		: mPath{ path.c_str() }, mDirList{}, mIsDir{ isDir } { }
	~CFileEntry() { }
};

class CPopup
{
public:
	Str mName;
	Str mMsg;
	bool mOpen;

	CPopup(const char *name, const char *msg)
		: mName{ name }, mMsg{ msg }, mOpen{ false } { }
	CPopup(void) { }
	~CPopup() { }
};

class CEditor
{
public:
	CEditor(void);
	~CEditor() { }

	static void Init(void);
	void Draw(void);
	void ReloadFileCache(void);

	std::list<CFileEntry> mFileCache;
	std::list<CPopup> mPopups;
	bool mConsoleActive;
	CGameConfig *mConfig;

	CMenu *mMenu_File;
	CMenu *mMenu_Edit;
};

extern CEditor *editor;

#endif