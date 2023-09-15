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

class CWidget
{
public:
	string_t mName;
	void (*mDrawFunc)(CWidget *self);
	uint32_t mFlags;
	bool mActive;

	CWidget(const char *name, void (*draw)(CWidget *self), uint32_t flags)
		: mName{ name }, mDrawFunc{ draw }, mFlags{ flags }, mActive{ true } { }
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
	static void AddPopup(const CPopup& popup);
	static CWidget *PushWidget(const CWidget& widget);

	list_t<CFileEntry> mFileCache;
	vector_t<CWidget> mWidgets;
	list_t<CPopup> mPopups;
	bool mConsoleActive;
	CGameConfig *mConfig;
};

CFileEntry *Draw_FileList(list_t<CFileEntry>& fileList);

extern CWidget *OpenProject;
extern CWidget *Preferences;
extern CEditor *editor;

#endif