#include "gln.h"
#include "preferences.h"

#define GLNOMAD_ENGINE "GLNE"
#define GLNOMAD_EXE "glnomad" STR(EXE_EXT)

CPrefs::CPrefs(void)
{
}

CPrefs::~CPrefs()
{
    SavePrefs();
}

static void PrintPrefs_f(void)
{
    const vector_t<CPrefData>& prefList = editor->mConfig->mPrefs.mPrefList;
    for (vector_t<CPrefData>::const_iterator it = prefList.cbegin(); it != prefList.cend(); ++it) {
        Printf(
            "[%s] =>\n"
            "\tValue: %s\n"
            "\tGroup: %s\n"
        , it->mName.c_str(), it->mValue.c_str(), it->mGroup.c_str());
    }
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

void CPrefs::SavePrefs(void) const
{
    json data;
    std::ofstream file("Data/preferences.json", std::ios::out);

    Printf("[CPrefs::SavePrefs] saving preferences...");

    // general configuration
    {
        data["config"]["enginePath"] = FindPref("enginePath");
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
    file.width(4);
    file << data;
    file.close();
}

CGameConfig::CGameConfig(void)
{
    Cmd_AddCommand("preflist", PrintPrefs_f);

    Printf("[CGameConfig::Init] initializing current configuration");
    mEditorPath = "Data/";

    mPrefs.LoadPrefs("Data/preferences.json");

    mEnginePath = mPrefs["enginePath"];
    mExecutablePath = mPrefs["exePath"];

    mTextureDetail = atoi(mPrefs["textureDetail"].c_str());
    mTextureFiltering = atoi(mPrefs["textureFiltering"].c_str());

    mCameraMoveSpeed = atof(mPrefs["moveSpeed"].c_str());
    mCameraRotationSpeed = atof(mPrefs["rotationSpeed"].c_str());
    mCameraZoomSpeed = atof(mPrefs["zoomSpeed"].c_str());
    
    if (mEnginePath.find_last_of(GLNOMAD_ENGINE) != eastl::string::npos) {
        mEngineName = GLNOMAD_ENGINE;
    }
    else {
        mEngineName = "Unknown";
    }
}

CGameConfig::~CGameConfig()
{
    mPrefs.SavePrefs();
}
