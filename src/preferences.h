#ifndef __PREFERENCES__
#define __PREFERENCES__

#pragma once

class CPrefData
{
public:
    Str mName;
    Str mValue;
    Str mGroup;

    CPrefData(const char *name, const char *value, const char *group)
        : mName{ name }, mValue{ value }, mGroup{ group } { }
    ~CPrefData() { }
};

class CPrefs
{
public:
    CPrefs(void);
    ~CPrefs();

    void LoadPrefs(const Str& path);
    void SavePrefs(void) const;
    void Clear(void);

    inline const Str& operator[](const char *name) const
    {
        for (const auto& it : mPrefList) {
            if (it.mName == name)
                return it.mValue;
        }
        Error("[CPrefs::GetPrefData] preference '%s' doesn't exist", name);
    }

    Str mFilePath;
    std::vector<CPrefData> mPrefList;
};

class CGameConfig
{
public:
    CGameConfig(void);
    CGameConfig(const Str& path);
    ~CGameConfig();

    void Dump(void);

    Str mEditorPath; // editor's internal save path
    Str mEnginePath; // path to the engine
    Str mExecutablePath; // path to exe's
    Str mEngineName; // engine's name

    int mTextureDetail;
    int mTextureFiltering;

    CPrefs mPrefs;
};

#endif