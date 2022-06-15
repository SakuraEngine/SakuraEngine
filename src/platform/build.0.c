#include "timer.c"
#if defined(_WIN32)
    #include "windows/time.c"
#elif defined(__APPLE__)
#elif defined(__EMSCRIPTEN__) || defined(__wasi__)
    #include "linux/time.c"
#endif

#include "sdl2/sdl2_input.c"
#include "sdl2/sdl2_window.c"