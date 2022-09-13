#include "platform/window.h"
#include "SDL2/SDL_system.h"
#include "SDL2/SDL_syswm.h"

SWindowHandle skr_create_window(const char8_t* name, const SWindowDescroptor* desc)
{
    uint32_t flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN;
    flags = desc->resizable ? (flags | SDL_WINDOW_RESIZABLE) : flags;
    flags |= desc->boardless ? SDL_WINDOW_BORDERLESS : 0;
    flags |= desc->hidden ? SDL_WINDOW_HIDDEN : 0;
    SDL_Window* sdl_window = SDL_CreateWindow(name,
        desc->centered ? SDL_WINDOWPOS_CENTERED : desc->posx,
        desc->centered ? SDL_WINDOWPOS_CENTERED : desc->posy,
        desc->width, desc->height, flags);
    return (SWindowHandle)sdl_window;
}

void skr_show_window(SWindowHandle window)
{
    SDL_ShowWindow((SDL_Window*)window);
}

void skr_get_all_monitors(uint32_t* count, SMonitorHandle* monitors)
{
    int display_count = SDL_GetNumVideoDisplays();
    if (count) *count = display_count;
    if (monitors)
    {
        for (uint64_t i = 0; i < display_count; ++i)
        {
            monitors[i] = (SMonitorHandle)i;
        }
    } 
}

void skr_monitor_get_extent(SMonitorHandle monitor, int32_t* width, int32_t* height)
{
    const uint64_t n = (uint64_t)monitor;
    SDL_Rect r;
    SDL_GetDisplayBounds(n, &r);
    if (width) *width = r.w;
    if (height) *height = r.h;
}

void skr_monitor_get_position(SMonitorHandle monitor, int32_t* x, int32_t* y)
{
    const uint64_t n = (uint64_t)monitor;
    SDL_Rect r;
    SDL_GetDisplayBounds(n, &r);
    if (x) *x = r.x;
    if (y) *y = r.y;
}

void skr_window_set_title(SWindowHandle window, const char8_t* name)
{
    SDL_SetWindowTitle((SDL_Window*)window, name);
}

void skr_window_get_extent(SWindowHandle window, int32_t* width, int32_t* height)
{
    SDL_Window* sdl_window = (SDL_Window*)window;
    SDL_GetWindowSize(sdl_window, width, height);
}

void skr_window_get_position(SWindowHandle window, int32_t* x, int32_t* y)
{
    SDL_Window* sdl_window = (SDL_Window*)window;
    SDL_GetWindowPosition(sdl_window, x, y);
}

bool skr_window_is_focused(SWindowHandle window)
{
    uint32_t flags = SDL_GetWindowFlags((SDL_Window*)window);
    return (flags & SDL_WINDOW_MOUSE_FOCUS) || (flags & SDL_WINDOW_INPUT_FOCUS);
}

bool skr_window_is_minimized(SWindowHandle window)
{
    uint32_t flags = SDL_GetWindowFlags((SDL_Window*)window);
    return (flags & SDL_WINDOW_MINIMIZED);
}

SWindowHandle skr_get_mouse_focused_window()
{
    return (SWindowHandle)SDL_GetMouseFocus();
}

void* skr_window_get_native_handle(SWindowHandle window)
{
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo((SDL_Window*)window, &wmInfo);
#ifdef SKR_OS_WINDOWS
    return wmInfo.info.win.window;
#elif defined(SKR_OS_MACOSX)
    return wmInfo.info.cocoa.window;
#endif
    return NULL;
}

#ifdef SKR_OS_MACOSX
    #include "platform/apple/macos/window.h"
#endif

void* skr_window_get_native_view(SWindowHandle window)
{
#ifdef SKR_OS_MACOSX
    void* ns_view =
        nswindow_get_content_view(skr_window_get_native_handle(window));
    return ns_view;
#else
    return skr_window_get_native_handle(window);
#endif
}

void skr_free_window(SWindowHandle window)
{
    SDL_DestroyWindow((SDL_Window*)window);
}