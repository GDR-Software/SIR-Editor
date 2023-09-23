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
    const std::vector<CPrefData>& prefList = editor->mConfig->mPrefs.mPrefList;
    for (std::vector<CPrefData>::const_iterator it = prefList.cbegin(); it != prefList.cend(); ++it) {
        Printf(
            "[%s] =>\n"
            "\tValue: %s\n"
            "\tGroup: %s\n"
        , it->mName.c_str(), it->mValue.c_str(), it->mGroup.c_str());
    }
}

void CPrefs::LoadPrefs(const std::string& path)
{
    json data;
    if (!LoadJSON(data, path)) {
        Printf("[CPrefs::LoadPrefs] Warning: failed to load preferences file, setting to default values");
        SetDefault();
        return;
    }

    mFilePath = path;

    mPrefList.reserve(data["config"].size());
    mPrefList.emplace_back("enginePath", data["config"]["enginePath"].get<std::string>().c_str(), "config");
    mPrefList.emplace_back("exePath", data["config"]["exePath"].get<std::string>().c_str(), "config");
    mPrefList.emplace_back("editorPath", data["config"]["editorPath"].get<std::string>().c_str(), "config");
    mPrefList.emplace_back("autoSaveTime", data["config"]["autoSaveTime"].get<std::string>().c_str(), "config");
    mPrefList.emplace_back("autoSave", data["config"]["autoSave"].get<std::string>().c_str(), "config");

    mPrefList.reserve(data["graphics"].size());
    mPrefList.emplace_back("textureDetail", data["graphics"]["textureDetail"].get<std::string>().c_str(), "graphics");
    mPrefList.emplace_back("textureFiltering", data["graphics"]["textureFiltering"].get<std::string>().c_str(), "graphics");

    mPrefList.reserve(data["camera"].size());
    mPrefList.emplace_back("moveSpeed", data["camera"]["moveSpeed"].get<std::string>().c_str(), "camera");
    mPrefList.emplace_back("rotationSpeed", data["camera"]["rotationSpeed"].get<std::string>().c_str(), "camera");
    mPrefList.emplace_back("zoomSpeed", data["camera"]["zoomSpeed"].get<std::string>().c_str(), "camera");
}

void CPrefs::SetDefault(void)
{
    mPrefList.emplace_back("editorPath", "Data/", "config");
    mPrefList.emplace_back("enginePath", "", "config");
    mPrefList.emplace_back("exePath", "", "config");
    mPrefList.emplace_back("autoSaveTime", "5", "config");
    mPrefList.emplace_back("autoSave", "true", "config");

    mPrefList.emplace_back("textureDetail", "2", "graphics");
    mPrefList.emplace_back("textureFiltering", "Bilinear", "graphics");
    
    mPrefList.emplace_back("moveSpeed", "1.5f", "camera");
    mPrefList.emplace_back("rotationSpeed", "1.0f", "camera");
    mPrefList.emplace_back("zoomSpeed", "1.5f", "camera");
}

void CPrefs::SavePrefs(void) const
{
    json data;
    std::ofstream file("preferences.json", std::ios::out);

    Printf("[CPrefs::SavePrefs] saving preferences...");

    // general configuration
    {
        data["config"]["editorPath"] = FindPref("editorPath");
        data["config"]["enginePath"] = FindPref("enginePath");
        data["config"]["exePath"] = FindPref("exePath");
        data["config"]["autoSaveTime"] = FindPref("autoSaveTime");
        data["config"]["autoSave"] = FindPref("autoSave");
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

void CGameConfig::LoadMobList(void)
{
    json data;
    std::filesystem::path path;

    path = editor->mConfig->mEditorPath + "moblist.json";

    Printf("[CGameConfig::LoadMobList] loading mob list...");

    if (!LoadJSON(data, path.c_str())) {
        Error("[CGameConfig::LoadMobList] failed to load mob list file");
    }

    mMobList.reserve(data["list"].size());
    for (const auto& it : data["list"]) {
        mMobList.emplace_back(it["name"].get<std::string>().c_str(), static_cast<uint32_t>(it["id"].get<uint64_t>()));
    }
}

CGameConfig::CGameConfig(void)
{
    Cmd_AddCommand("preflist", PrintPrefs_f);

    Printf("[CGameConfig::Init] intializing current configuration");

    mPrefs.LoadPrefs("preferences.json");

    mEnginePath = mPrefs["enginePath"];
    mExecutablePath = mPrefs["exePath"];
    mEditorPath = mPrefs["editorPath"];

    if (mEditorPath.back() != PATH_SEP) {
        mEditorPath.push_back(PATH_SEP);
    }
    if (mEnginePath.back() != PATH_SEP) {
        mEnginePath.push_back(PATH_SEP);
    }

    mAutoSaveTime = atoi(mPrefs["autoSaveTime"].c_str());
    mAutoSave = mPrefs["autoSave"] == "true" ? true : false;

    mTextureDetail = StringToInt(mPrefs["textureDetail"], texture_details, arraylen(texture_details));
    mTextureFiltering = StringToInt(mPrefs["textureFiltering"], texture_filters, arraylen(texture_filters));

    mCameraMoveSpeed = atof(mPrefs["moveSpeed"].c_str());
    mCameraRotationSpeed = atof(mPrefs["rotationSpeed"].c_str());
    mCameraZoomSpeed = atof(mPrefs["zoomSpeed"].c_str());
    
    if (N_stristr(mEnginePath.c_str(), GLNOMAD_EXE)) {
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
