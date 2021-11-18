#pragma once

#ifdef _WINDOWS
    #define WIN_MEAN_AND_LEAN
    #include "windows.h"
    #include <ShellScalingApi.h>
    #pragma comment(lib, "shcore.lib")
#endif

struct PlatformWindow {
#ifdef _WIN32
    struct
    {
        HWND hWnd;
        HINSTANCE hInstance;
    } win;
#endif
#if __has_include("SDL2/SDL_main.h")
    struct
    {
        struct SDL_Window* window;
    } sdl;
#endif
};