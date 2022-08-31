#include "platform/configure.h"

#if !defined(__EMSCRIPTEN__) && defined(SKR_RUNTIME_USE_MIMALLOC)
    #include "stats.c"
    #include "random.c"
    #include "os.c"
    #include "bitmap.c"
    #include "arena.c"
    #include "segment-cache.c"
    #include "segment.c"
    #include "page.c"
    #include "alloc.c"
    #include "alloc-aligned.c"
    #include "alloc-posix.c"
    #include "heap.c"
    #include "options.c"
    #include "init.c"
#endif