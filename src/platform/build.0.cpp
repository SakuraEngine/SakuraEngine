#include "platform/configure.h"
#include "debug.cpp"
#include "vfs.cpp"
#include "llfio/llfio_vfs.cpp"
#ifdef SAKURA_RUNTIME_OS_UNIX
    #include "unix/unix_vfs.cpp"
#elif defined(SAKURA_RUNTIME_OS_WINDOWS)
    #include "windows/windows_vfs.cpp"
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