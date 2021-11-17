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
#define BACK_BUFFER_WIDTH 1280
#define BACK_BUFFER_HEIGHT 720

#include <EASTL/unique_ptr.h>

inline static bool SDLEventHandler(const SDL_Event& event, SDL_Window* window)
{
    switch (event.type)
    {
        case SDL_WINDOWEVENT:
			if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
                const auto ResizeWidth = event.window.data1;
                const auto ResizeHeight = event.window.data2;
                (void)ResizeWidth;
                (void)ResizeHeight;
            }
			else if(event.window.event == SDL_WINDOWEVENT_CLOSE)
            	return false;
        default: 
            return true;
    }
    return true;
}

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
    PlatformWindow pw = {};
    pw.sdl.window = sdl_window;
#ifdef _WINDOWS
    pw.win.hWnd = wmInfo.info.win.window;
    pw.win.hInstance = wmInfo.info.win.hinstance;
#endif

    while(sdl_window)
    {
        SDL_Event event;
		while (SDL_PollEvent(&event)) 
		{
			//olog::Info(u"event type: {}  windowID: {}"_o, (int)event.type, (int)event.window.windowID);
			if(SDL_GetWindowID(sdl_window) == event.window.windowID)
			{
				if(!SDLEventHandler(event, sdl_window))
				{
                    sdl_window = nullptr;
				}
			}
		}
    }
	SDL_Quit();
    return 0;
}