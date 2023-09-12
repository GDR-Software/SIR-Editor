#include "gln.h"
#include "preferences.h"

CPrefs::CPrefs(void)
{
}

CPrefs::~CPrefs()
{
    SavePrefs();
}

void CPrefs::LoadPrefs(const std::string& path)
{
    json data;

    if (!FileExists(path.c_str())) {
        Printf("[CPrefs::LoadPrefs] failed to load preferences file because it could not be found");
        return;
    }

    if (!LoadJSON(data, path)) {
        
    }
}