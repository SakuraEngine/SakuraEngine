#if defined(_WIN32)
    #include "windows/misc.c"
    #include "windows/thread.c"
#elif defined(__APPLE__)
    #include "apple/thread.c"
#endif
#if SKR_PLAT_UNIX
    #include "unix/unix_thread.c"
#endif