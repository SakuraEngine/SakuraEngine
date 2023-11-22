#include <stdio.h>
#include "SkrBase/misc/debug.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "Windows.h"

void skr_debug_output(const char* msg)
{
    ::OutputDebugStringA(msg);
    ::OutputDebugStringA("\r\n");
}
#else
void skr_debug_output(const char* msg)
{
    printf("%s", msg);
}
#endif