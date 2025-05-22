#include "SkrBase/config.h"
#include "platform/vfs.cpp"

#include "platform/standard/stdio_vfs.cpp"
#if SKR_PLAT_UNIX
    #include "platform/unix/unix_vfs.cpp"
#elif SKR_PLAT_WINDOWS
    #include "platform/windows/windows_vfs.cpp"
#endif
