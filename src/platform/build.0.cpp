#include "platform/configure.h"
#ifdef _WIN32
    #include "win/window.cpp"
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