#ifndef _EDITOR_H_
#define _EDITOR_H_

#pragma once

#include "gui.h"

class CFileEntry
{
public:
	eastl::string mPath;
	eastl::vector<CFileEntry> mDirList;
	const bool mIsDir;

	CFileEntry(const std::filesystem::path& path, bool isDir)
		: mPath{ path.c_str() }, mDirList{}, mIsDir{ isDir } { }
	~CFileEntry() { }
};

class CEditor
{
public:
	CEditor(void);
	~CEditor() { }

	static void Init(void);
	void Draw(void);
	void ReloadFileCache(void);

	eastl::vector<CFileEntry> mFileCache;
	bool mConsoleActive;
};

extern CEditor *editor;

#endif