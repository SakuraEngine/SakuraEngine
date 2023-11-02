#include "memory.c"
#include "timer.c"
#include "cpuinfo/cpuinfo.c"
#if defined(_WIN32)
    #include "windows/time.c"
    #include "windows/thread.c"
    #include "windows/misc.c"
#elif defined(__APPLE__)
    #include "apple/debug.c"
    #include "apple/time.c"
    #include "apple/thread.c"
#elif defined(__EMSCRIPTEN__) || defined(__wasi__)
    #include "linux/time.c"
#endif

#if SKR_PLAT_UNIX
    #include "unix/unix_thread.c"
#endif

#include "sdl2/sdl2_input.c"
#include "sdl2/sdl2_window.c"