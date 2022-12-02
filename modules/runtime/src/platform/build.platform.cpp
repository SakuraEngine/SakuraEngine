#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h> 
#endif
#include "platform/configure.h"
#include "debug.cpp"
#include "vfs.cpp"
#include "standard/stdio_vfs.cpp"
#ifdef SKR_OS_UNIX
    #include "unix/unix_vfs.cpp"
    #include "unix/process.cpp"
#elif defined(SKR_OS_WINDOWS)
    #include "windows/windows_vfs.cpp"
    #include "windows/process.cpp"
#endif

#ifdef RUNTIME_SHARED
extern "C" {
RUNTIME_API bool mi_allocator_init(const char** message)
{
    if (message != NULL) *message = NULL;
    return true;
}
RUNTIME_API void mi_allocator_done(void)
{
    // nothing to do
}
}
#endif