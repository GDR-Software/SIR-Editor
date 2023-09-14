#ifndef _EDITOR_H_
#define _EDITOR_H_

#pragma once

#include "gui.h"

class CFileEntry
{
public:
	string_t mPath;
	list_t<CFileEntry> mDirList;
	const bool mIsDir;

	CFileEntry(const std::filesystem::path& path, bool isDir)
		: mPath{ path.c_str() }, mDirList{}, mIsDir{ isDir } { }
	~CFileEntry() { }
};

class CPopup
{
public:
	string_t mName;
	string_t mMsg;
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

	list_t<CFileEntry> mFileCache;
	list_t<CPopup> mPopups;
	bool mConsoleActive;
	CGameConfig *mConfig;
};

extern CEditor *editor;

#endif