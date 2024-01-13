#include "time/timer.c"
#if defined(_WIN32)
    #include "time/windows_time.c"
#elif defined(__APPLE__)
    #include "time/apple_time.c"
#elif defined(__EMSCRIPTEN__) || defined(__wasi__)
    #include "time/linux_time.c"
#endif