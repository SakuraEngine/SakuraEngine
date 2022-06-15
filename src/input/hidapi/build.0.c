#if defined(_WIN32)
    #include "windows\hid.c"
#elif defined(__APPLE__)
    #include "mac\hid.c"
#elif defined(__EMSCRIPTEN__) || defined(__wasi__)
    #include "linux\hid.c"
#endif