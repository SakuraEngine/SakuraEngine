#pragma once
#ifndef __cplusplus
    #include <stdbool.h>
#endif
#include <stdint.h>

#ifndef RUNTIME_EXPORT
    #if defined(_MSC_VER)
        #define RUNTIME_EXPORT __declspec(dllexport)
    #else
        #define RUNTIME_EXPORT
    #endif
#endif

#ifndef RUNTIME_API // If the build file hasn't already defined this to be dllexport...
    #ifdef RUNTIME_DLL
        #if defined(_MSC_VER)
            #define RUNTIME_API __declspec(dllimport)
            #define RUNTIME_LOCAL
        #elif defined(__CYGWIN__)
            #define RUNTIME_API __attribute__((dllimport))
            #define RUNTIME_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define RUNTIME_API __attribute__((visibility("default")))
            #define RUNTIME_LOCAL __attribute__((visibility("hidden")))
        #else
            #define RUNTIME_API
            #define RUNTIME_LOCAL
        #endif

        #define EA_DLL

    #else
        #define RUNTIME_API
        #define RUNTIME_LOCAL
    #endif
#endif

#ifdef __APPLE__
    #include "TargetConditionals.h"
    #ifdef TARGET_OS_MAC
        #define _MACOS
    #endif
#elif defined _WIN32 || defined _WIN64
#endif

#ifndef CHAR8_T_DEFINED // If the user hasn't already defined these...
    #define CHAR8_T_DEFINED
    #if defined(EA_PLATFORM_APPLE)
        #define char8_t char // The Apple debugger is too stupid to realize char8_t is typedef'd to char, so we #define it.
    #else
typedef char char8_t;
    #endif
#endif

#if defined(__cplusplus)
    #define DECLARE_ZERO(type, var) type var = {};
#else
    #define DECLARE_ZERO(type, var) type var = {0};
#endif