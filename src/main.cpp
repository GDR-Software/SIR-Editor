#include "gln.h"

int main(int argc, char **argv)
{
    Mem_Init();

    gui = new Window;
    editor = new CEditor;
    atexit(Mem_Shutdown);

    while (1) {
        gui->BeginFrame();

        gui->EndFrame();
    }

    // never reached
    return 0;
}