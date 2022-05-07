#pragma once

#ifndef TOOL_EXPORT
    #if defined(_MSC_VER)
        #define TOOL_EXPORT __declspec(dllexport) TOOL_EXTERN_C
    #else
        #define TOOL_EXPORT
    #endif
#endif

#ifdef TOOL_IMPL
    #ifndef TOOL_API
        #define TOOL_API TOOL_EXPORT
    #endif
#endif

#ifndef TOOL_API // If the build file hasn't already defined this to be dllexport...
    #ifdef TOOL_SHARED
        #if defined(_MSC_VER)
            #define TOOL_API __declspec(dllimport)
            #define TOOL_LOCAL
        #elif defined(__CYGWIN__)
            #define TOOL_API __attribute__((dllimport))
            #define TOOL_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define TOOL_API __attribute__((visibility("default")))
            #define TOOL_LOCAL __attribute__((visibility("hidden")))
        #else
            #define TOOL_API
            #define TOOL_LOCAL
        #endif

        #define EA_DLL
    #else
        #define TOOL_API
        #define TOOL_LOCAL
    #endif
#endif