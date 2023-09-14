#ifndef __PREFERENCES__
#define __PREFERENCES__

#pragma once

class CPrefData
{
public:
    string_t mName;
    string_t mValue;
    string_t mGroup;

    CPrefData(const char *name, const char *value, const char *group)
        : mName{ name }, mValue{ value }, mGroup{ group } { }
    ~CPrefData() { }
};

class CPrefs
{
public:
    CPrefs(void);
    ~CPrefs();

    void LoadPrefs(const string_t& path);
    void SavePrefs(void) const;
    void SetDefault(void);
    void Clear(void);

    inline const string_t& FindPref(const char *name) const
    { return operator[](name); }

    inline const string_t& operator[](const char *name) const
    {
        for (const auto& it : mPrefList) {
            if (it.mName == name)
                return it.mValue;
        }
        Error("[CPrefs::GetPrefData] preference '%s' doesn't exist", name);
    }

    string_t mFilePath;
    vector_t<CPrefData> mPrefList;
};

class CGameConfig
{
public:
    CGameConfig(void);
    CGameConfig(const string_t& path);
    ~CGameConfig();

    void Dump(void);

    string_t mEditorPath; // editor's internal save path
    string_t mEnginePath; // path to the engine
    string_t mExecutablePath; // path to exe's
    string_t mEngineName; // engine's name

    int mTextureDetail;
    int mTextureFiltering;

    float mCameraMoveSpeed;
    float mCameraRotationSpeed;
    float mCameraZoomSpeed;

    CPrefs mPrefs;
};

#endif