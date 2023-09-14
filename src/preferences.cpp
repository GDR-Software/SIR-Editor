#include "gln.h"
#include "preferences.h"

CPrefs::CPrefs(void)
{
}

CPrefs::~CPrefs()
{
    SavePrefs();
}

inline const char* GetString(const json& data)
{ return data.get<string_t>().c_str(); }

void CPrefs::LoadPrefs(const string_t& path)
{
    json data;
    if (!LoadJSON(data, path)) {
        Printf("[CPrefs::LoadPrefs] Warning: failed to load preferences file, setting to default values");
        SetDefault();
        return;
    }

    mFilePath = path;

    mPrefList.reserve(data["config"].size());
    mPrefList.emplace_back("enginePath", GetString(data["config"]["enginePath"]), "config");
    mPrefList.emplace_back("engineName", GetString(data["config"]["engineName"]), "config");
    mPrefList.emplace_back("exePath", GetString(data["config"]["exePath"]), "config");

    mPrefList.reserve(data["graphics"].size());
    mPrefList.emplace_back("textureDetail", GetString(data["graphics"]["textureDetail"]), "config");
    mPrefList.emplace_back("textureFiltering", GetString(data["graphics"]["textureFiltering"]), "config");

    mPrefList.reserve(data["camera"].size());
    mPrefList.emplace_back("moveSpeed", GetString(data["camera"]["moveSpeed"]), "camera");
    mPrefList.emplace_back("rotationSpeed", GetString(data["camera"]["rotationSpeed"]), "camera");
    mPrefList.emplace_back("zoomSpeed", GetString(data["camera"]["zoomSpeed"]), "camera");
}

void CPrefs::SetDefault(void)
{
    mPrefList.reserve(8);

    mPrefList.emplace_back("enginePath", "", "config");
    mPrefList.emplace_back("exePath", "", "config");
    mPrefList.emplace_back("textureDetail", "2", "config");
    mPrefList.emplace_back("textureFiltering", "Bilinear", "config");
    mPrefList.emplace_back("moveSpeed", "1.5f", "camera");
    mPrefList.emplace_back("rotationSpeed", "1.0f", "camera");
    mPrefList.emplace_back("zoomSpeed", "1.5f", "camera");
}

static void WriteJSON(const json& data)
{

}

void CPrefs::SavePrefs(void) const
{
    json data;
    std::ofstream file("Data/preferences.json");

    // general configuration
    {
        data["config"]["enginePath"] = FindPref("enginePath");
        data["config"]["engineName"] = FindPref("engineName");
        data["config"]["exePath"] = FindPref("exePath");
    }
    // graphics configuration
    {
        data["graphics"]["textureDetail"] = FindPref("textureDetail");
        data["graphics"]["textureFiltering"] = FindPref("textureFiltering");
    }
    // camera configration
    {
        data["camera"]["moveSpeed"] = FindPref("moveSpeed");
        data["camera"]["rotationSpeed"] = FindPref("rotationSpeed");
        data["camera"]["zoomSpeed"] = FindPref("zoomSpeed");
    }
    if (!file.is_open()) {
        Error("[CPrefs::SavePrefs] failed to open preferences file in save mode");
    }

    WriteJSON(data);

    file << data;
    file.close();
}

CGameConfig::CGameConfig(void)
{
    Printf("[CGameConfig::Init] initializing current configuration");
    mEditorPath = "Data/";

    mPrefs.LoadPrefs("Data/preferences.json");

    mEnginePath = mPrefs["enginePath"];
    mEngineName = mPrefs["engineName"];
    mExecutablePath = mPrefs["exePath"];

    mTextureDetail = atoi(mPrefs["textureDetail"].c_str());
    mTextureFiltering = atoi(mPrefs["textureFiltering"].c_str());

    mCameraMoveSpeed = atof(mPrefs["moveSpeed"].c_str());
    mCameraRotationSpeed = atof(mPrefs["rotationSpeed"].c_str());
    mCameraZoomSpeed = atof(mPrefs["zoomSpeed"].c_str());
}

CGameConfig::~CGameConfig()
{
    mPrefs.SavePrefs();
}
