#include "Common.hpp"
#include <glad/glad.h>
#include "GUI.h"
#include "Editor.h"

int main(int argc, char **argv)
{
    Editor::Init(argc, argv);
    Editor::Get()->run();

    // never reached
    return 0;
}