#pragma once
#include "platform/configure.h"

typedef struct SMonitor SMonitor;
typedef struct SWindow SWindow;
typedef SWindow* SWindowHandle;
typedef SMonitor* SMonitorHandle;

typedef enum SWindowFlag {
    SKR_WINDOW_CENTERED = 0x00000001,
    SKR_WINDOW_RESIZABLE = 0x00000002,
    SKR_WINDOW_BOARDLESS = 0x00000004,
    SKR_WINDOW_HIDDEN = 0x00000008,
    SKR_WINDOW_TOPMOST = 0x00000010,
    SKR_WINDOW_FLAG_ENUM_MAX = 0x7FFFFFFF
} SWindowFlag;
typedef uint32_t SWindowFlags;

typedef struct SWindowDescroptor {
    uint32_t width;
    uint32_t height;
    uint32_t posx;
    uint32_t posy;
    SWindowFlags flags;
} SWindowDescroptor;

#ifdef __cplusplus
extern "C" {
#endif
RUNTIME_API void skr_get_all_monitors(uint32_t* count, SMonitorHandle* monitors);
RUNTIME_API void skr_monitor_get_extent(SMonitorHandle monitor, int32_t* width, int32_t* height);
RUNTIME_API void skr_monitor_get_position(SMonitorHandle monitor, int32_t* x, int32_t* y);
RUNTIME_API bool skr_monitor_get_ddpi(SMonitorHandle monitor, float* ddpi, float* hdpi, float* vdpi);

RUNTIME_API SWindowHandle skr_create_window(const char8_t* name, const SWindowDescroptor* desc);
RUNTIME_API void skr_show_window(SWindowHandle window);
RUNTIME_API void skr_window_set_title(SWindowHandle window, const char8_t* name);
RUNTIME_API void skr_window_set_extent(SWindowHandle window, int32_t width, int32_t height);
RUNTIME_API void skr_window_set_position(SWindowHandle window, int32_t x, int32_t y);
RUNTIME_API void skr_window_get_extent(SWindowHandle window, int32_t* width, int32_t* height);
RUNTIME_API void skr_window_get_position(SWindowHandle window, int32_t* x, int32_t* y);
RUNTIME_API bool skr_window_is_minimized(SWindowHandle window);
RUNTIME_API bool skr_window_is_focused(SWindowHandle window);

RUNTIME_API SWindowHandle skr_get_mouse_focused_window();
RUNTIME_API void* skr_window_get_native_handle(SWindowHandle window);
RUNTIME_API void* skr_window_get_native_view(SWindowHandle window);
RUNTIME_API void skr_free_window(SWindowHandle window);
#ifdef __cplusplus
}
#endif