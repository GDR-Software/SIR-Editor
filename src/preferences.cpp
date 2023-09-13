#include "gln.h"
#include "preferences.h"

CPrefs::CPrefs(void)
{
}

CPrefs::~CPrefs()
{
    SavePrefs();
}

#define GET_STRING(x) data##x.get<eastl::string>().c_str()

void CPrefs::LoadPrefs(const Str& path)
{
    FileStream file;
    json data;

    if (!FileExists(path.GetBuffer())) {
        Printf("[CPrefs::LoadPrefs] failed to load preferences file because it could not be found");
        return;
    }

    if (!file.Open(path.GetBuffer(), "r")) {
        Error("[CPrefs::LoadPrefs] failed to open preferences file");
    }

    try {
        data = json::parse(file.GetStream());
    } catch (const json::exception &) {
        Error("[CPrefs::LoadPrefs] json parsing failed");
    }
    file.Close();

    mFilePath = path;

    mPrefList.reserve(data["config"].size());
    mPrefList.emplace_back("enginePath", GET_STRING(["config"]["enginePath"]), "config");
    mPrefList.emplace_back("enginename", GET_STRING(["config"]["engineName"]), "config");
    mPrefList.emplace_back("exePath", GET_STRING(["config"]["exePath"]), "config");

    mPrefList.reserve(data["graphics"].size());
    mPrefList.emplace_back("textureDetail", GET_STRING(["graphics"]["textureDetail"]), "config");
    mPrefList.emplace_back("textureFiltering", GET_STRING(["graphics"]["textureFiltering"]), "config");
}

CGameConfig::CGameConfig(void)
{
    mEditorPath = "Data/";

    mPrefs.LoadPrefs("Data/preferences.json");

    mEnginePath = mPrefs["enginePath"];
    mEngineName = mPrefs["engineName"];
    mExecutablePath = mPrefs["exePath"];
}
