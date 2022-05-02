#include <stdio.h>
#if defined(_WIN32)
    #include "windows.h"
#endif

#if defined(_WIN32)
void skr_debug_output(const char* msg)
{
    OutputDebugStringA(msg);
    OutputDebugStringA("\r\n");
}
#else
void skr_debug_output(const char* msg)
{
    printf("%s", msg);
}
#endif