#include "SkrBase/config.h"

#if SKR_PLAT_UNIX
    #include "process/unix_process.cpp"
#elif SKR_PLAT_WINDOWS
    #include "process/windows_process.cpp"
#endif