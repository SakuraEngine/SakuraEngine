#include "platform/configure.h"

#if !defined(__EMSCRIPTEN__) && defined(SKR_RUNTIME_USE_MIMALLOC)
    #include "static.c"
#endif