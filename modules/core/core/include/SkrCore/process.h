#pragma once
#include "SkrBase/config.h"
#include "SkrBase/config.h"

typedef struct SProcess SProcess;
typedef struct SProcess* SProcessHandle;
typedef uint32_t SProcessId;

#if _WIN32
enum {
    ///Hides the window and activates another window.
    SKR_PROCESS_SW_HIDE,
    ///Activates the window and displays it as a maximized window.
    SKR_PROCESS_SW_SHOWMAXIMIZED,
    ///Activates the window and displays it as a minimized window.
    SKR_PROCESS_SW_SHOWMINIMIZED,
    ///Displays the window as a minimized window. This value is similar to `minimized`, except the window is not activated.
    SKR_PROCESS_SW_SHOWMINNOACTIVE,
    ///Displays a window in its most recent size and position. This value is similar to show_normal`, except that the window is not activated.
    SKR_PROCESS_SW_SHOWNOACTIVATE,
    ///Activates and displays a window. If the window is minimized or maximized, the system restores it to its original size and position. An application should specify this flag when displaying the window for the first time.
    SKR_PROCESS_SW_SHOWNORMAL,
    ///Activates and displays a window. If the window is minimized or maximized, the system restores it to its original size and position. An application should specify this flag when displaying the window for the first time.
    SKR_PROCESS_SW_SHOW = SKR_PROCESS_SW_SHOWNORMAL,
};
typedef struct {
    const char8_t* command;
    const char8_t** arguments;
    uint32_t arg_count;
    const char8_t* stdout_file;
    uint32_t show_window_flag;
} SkrRunProcessArgs;
#else
typedef struct {
    const char8_t* command;
    const char8_t** arguments;
    uint32_t arg_count;
    const char8_t* stdout_file;
} SkrRunProcessArgs;
#endif  // _WIN32

SKR_EXTERN_C SKR_CORE_API
SProcessHandle skr_run_process(const char8_t* command, const char8_t** arguments, uint32_t arg_count, const char8_t* stdout_file);

SKR_EXTERN_C SKR_CORE_API
SProcessHandle skr_run_process_with(SkrRunProcessArgs *args);

SKR_EXTERN_C SKR_CORE_API
SProcessId skr_get_current_process_id();

SKR_EXTERN_C SKR_CORE_API
const char8_t* skr_get_current_process_name();

SKR_EXTERN_C SKR_CORE_API
SProcessId skr_get_process_id(SProcessHandle);

SKR_EXTERN_C SKR_CORE_API
int skr_wait_process(SProcessHandle process);