#ifndef __PREFERENCES__
#define __PREFERENCES__

#pragma once

class CPrefData
{
public:
    std::string mName;
    std::string mValue;

    CPrefData(const char *name, const char *value)
        : mName{ name }, mValue{ value } { }
    ~CPrefData() { }
};

class CPrefs
{
public:
    CPrefs(void);
    ~CPrefs();

    void LoadPrefs(const std::string& path);
    void SavePrefs(void) const;
    void Clear(void);

    std::string mFilePath;
    std::vector<CPrefData> mPrefList;
};

class CGameConfig
{
public:
    CGameConfig(void);
    CGameConfig(const std::string& path);
    ~CGameConfig();

    void Dump(void);

    std::string mEditorPath; // editor's internal save path
    std::string mEnginePath; // path to the engine
    std::string mExecutablePath; // path to exe's
    std::String mEngineName; // engine's name
};

#endif