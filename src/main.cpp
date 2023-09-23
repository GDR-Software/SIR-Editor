#include "gln.h"

#define FRAME_QUADS 0x8000
#define FRAME_VERTICES (FRAME_QUADS*4)
#define FRAME_INDICES (FRAME_QUADS*6)


static void GenIndexes(void)
{
    gui->mIndices = (uint32_t *)GetClearedMemory(sizeof(*gui->mIndices) * (FRAME_QUADS * 6));

    uint32_t offset;

    offset = 0;
    for (uint32_t i = 0; i < (FRAME_QUADS * 6); i += 6) {
        gui->mIndices[i + 0] = offset + 0;
        gui->mIndices[i + 1] = offset + 1;
        gui->mIndices[i + 2] = offset + 2;

        gui->mIndices[i + 3] = offset + 3;
        gui->mIndices[i + 4] = offset + 2;
        gui->mIndices[i + 5] = offset + 0;

        offset += 4;
    }
}

int main(int argc, char **argv)
{
    gui = new Window;
    editor = new CEditor;

    editor->ReloadFileCache();
    editor->mConfig->LoadMobList();

    GenIndexes();

    // if we're given something from the command line, load it up
    if (argv[1] && N_stricmp(GetExtension(argv[1]), "map")) {
        Map_Load(argv[1]);
    }
    else {
        Map_New();
    }

    while (1) {
        CheckAutoSave();
        gui->BeginFrame();
        editor->Draw();

        gui->EndFrame();
    }

    // never reached
    return 0;
}
