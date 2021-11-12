#define RUNTIME_DLL

#include "platform/adaptive_types.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#ifndef __APPLE__
#define SDL_MAIN_NEEDED
#include <SDL2/SDL_main.h>
#define SDL_MAIN_HANDLED
#endif

#if defined(_WIN32) || defined(_WIN64)
#ifndef __WIN32__
#define __WIN32__
#endif // __WIN32__
#endif

#ifdef _WINDOWS
#include "platform/win/window.h"
#endif

#define BACK_BUFFER_WIDTH 1280
#define BACK_BUFFER_HEIGHT 720

#include <thread>
#include <chrono>

int main(int , char* [])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		return -1;
	}
    SDL_SysWMinfo wmInfo;

    auto sdl_window = SDL_CreateWindow("title",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT, 
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );
    SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(sdl_window, &wmInfo);
#ifdef _WINDOWS
    BorderlessWindow window;
    auto platform_child = window.get_hwnd();
#endif
    auto sdl_child = SDL_CreateWindowFrom(platform_child);
    while(sdl_window && sdl_child)
    {
		SDL_Event event;
        while (SDL_PollEvent(&event)) 
		{
            switch (event.type)
            {
                case SDL_WINDOWEVENT:
                {
                    if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                        return 0;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    return 0;
}