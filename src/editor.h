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

class CWidget
{
public:
	std::string mName;
	void (*mDrawFunc)(CWidget *self);
	uint32_t mFlags;
	bool mActive;

	CWidget(const char *name, void (*draw)(CWidget *self), uint32_t flags)
		: mName{ name }, mDrawFunc{ draw }, mFlags{ flags }, mActive{ false } { }
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

class CFileBrowser
{
public:
	std::filesystem::path mCurrentDir;
	std::filesystem::path mParentDir;
	std::filesystem::path mCurrentPath;
	bool mActive;
	int mCurrent;
	CFileEntry *mCurDir = NULL;

	CFileBrowser(const std::filesystem::path& dir)
		: mCurrentDir{ dir }, mParentDir{}, mCurrentPath{ mCurrentDir }, mActive{ false }, mCurrent{ 0 } { }
	CFileBrowser(void)
		: mCurrentDir{ CurrentDirName() }, mParentDir{}, mCurrentPath{ mCurrentDir }, mActive{ false }, mCurrent{ 0 } { }
	~CFileBrowser() { }

	void LoadFiles(void);
	CFileEntry Draw(const char *action);
};

class CEditor
{
public:
	CEditor(void);
	~CEditor() { }

	static void Init(void);
	void Draw(void);
	void ReloadFileCache(void);
	static void AddPopup(const CPopup& popup);
	static CWidget *PushWidget(const CWidget& widget);

	std::vector<CFileEntry> mFileCache;
	std::vector<CWidget> mWidgets;
	std::list<CPopup> mPopups;
	bool mConsoleActive;
	CGameConfig *mConfig;
};

inline const std::filesystem::path pwdString = std::filesystem::current_path();

CFileEntry *Draw_FileList(std::list<CFileEntry>& fileList);
extern CEditor *editor;

#endif