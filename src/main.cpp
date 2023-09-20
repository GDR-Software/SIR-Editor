#include "gln.h"

int main(int argc, char **argv)
{
    gui = new Window;
    editor = new CEditor;

    editor->ReloadFileCache();
    editor->mConfig->LoadMobList();

    // if we're given something from the command line, load it up
    if (argv[1] && N_stricmp(GetExtension(argv[1]), "map")) {
        Map_Load(argv[1]);
    }
    else {
        Map_New();
    }

    while (1) {
        gui->BeginFrame();
        editor->Draw();

        gui->EndFrame();
    }

    // never reached
    return 0;
}
