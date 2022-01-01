#pragma once
#include "platform/adaptive_types.h"
#include "stdbool.h"
#ifdef TARGET_MACOS
    #include "platform/apple/macos/window.h"
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#define BACK_BUFFER_WIDTH 360
#define BACK_BUFFER_HEIGHT 360

inline static bool SDLEventHandler(const SDL_Event* event, SDL_Window* window)
{
    switch (event->type)
    {
        case SDL_WINDOWEVENT:
            if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                const int32_t ResizeWidth = event->window.data1;
                const int32_t ResizeHeight = event->window.data2;
                (void)ResizeWidth;
                (void)ResizeHeight;
            }
            else if (event->window.event == SDL_WINDOWEVENT_CLOSE)
                return false;
        default:
            return true;
    }
    return true;
}
