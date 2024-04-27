#include "SkrBase/config.h"

#include "platforms/standard/crash_handler.cpp"
#if SKR_PLAT_UNIX
    #if SKR_PLAT_MACOSX
        #define UNIX_CRASH_HANDLER_IMPLEMENTED
        #include "platforms/apple/crash_handler.cpp"
    #elif SKR_PLAT_LINUX
        #define UNIX_CRASH_HANDLER_IMPLEMENTED
        #include "platforms/linux/crash_handler.cpp"
    #endif
    #include "platforms/unix/crash_handler.cpp"
#elif SKR_PLAT_WINDOWS
    #include "platforms/windows/crash_handler.cpp"
#endif