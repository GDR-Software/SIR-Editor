#ifndef __PREFERENCES__
#define __PREFERENCES__

#pragma once

class CPrefData
{
public:
    std::string mName;
    std::string mValue;
    std::string mGroup;

    CPrefData(const char *name, const char *value, const char *group)
        : mName{ name }, mValue{ value }, mGroup{ group } { }
    ~CPrefData() { }
};

class CPrefs
{
public:
    CPrefs(void);
    ~CPrefs();

    void LoadPrefs(const std::string& path);
    void SavePrefs(void) const;
    void SetDefault(void);
    void Clear(void);

    inline const std::string& FindPref(const char *name) const
    { return operator[](name); }

    inline const std::string& operator[](const char *name) const
    {
        for (const auto& it : mPrefList) {
            if (it.mName == name)
                return it.mValue;
        }
        Error("[CPrefs::GetPrefData] preference '%s' doesn't exist", name);
    }

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
    std::string mEngineName; // engine's name

    int mTextureDetail;
    int mTextureFiltering;

    float mCameraMoveSpeed;
    float mCameraRotationSpeed;
    float mCameraZoomSpeed;

    CPrefs mPrefs;
};

#endif