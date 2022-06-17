#if defined(_WIN32)
    #include "windows/hid.c"
#elif defined(__APPLE__)
    #include "mac/hid.c"
#else
    #include "linux/hid.c"
#endif