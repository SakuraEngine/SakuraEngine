#include "SkrInput/input.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
    skr::input::Input::Initialize();
    skr::input::Input::Finalize();
    
    printf("Press any key to exit...");
    ::getchar();
    return 0;
}