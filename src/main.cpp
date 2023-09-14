#include "gln.h"

int main(int argc, char **argv)
{
    Mem_Init();
    atexit(Mem_Shutdown);

    gui = Allocate<Window>();
    editor = Allocate<CEditor>();

    editor->ReloadFileCache();

    while (1) {
        gui->BeginFrame();
        editor->Draw();

        gui->EndFrame();
    }

    // never reached
    return 0;
}