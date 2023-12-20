#include <stdio.h>
#include "SkrBase/misc/debug.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "Windows.h"

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

#ifdef __APPLE__
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void apple_assert_handler(int sig)
{
    void *array[10];
    size_t size;
    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
}
#endif