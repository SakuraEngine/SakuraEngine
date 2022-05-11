#pragma once

#ifdef __cplusplus
    #define GAMETOOL_EXTERN_C extern "C"
#else
    #define GAMETOOL_EXTERN_C
#endif

#ifndef GAMETOOL_EXPORT
    #if defined(_MSC_VER)
        #define GAMETOOL_EXPORT __declspec(dllexport)
    #else
        #define GAMETOOL_EXPORT
    #endif
#endif

#ifdef GAMETOOL_IMPL
    #ifndef GAMETOOL_API
        #define GAMETOOL_API GAMETOOL_EXPORT
    #endif
#endif

#ifndef GAMETOOL_API // If the build file hasn't already defined this to be dllexport...
    #ifdef GAMETOOL_SHARED
        #if defined(_MSC_VER)
            #define GAMETOOL_API __declspec(dllimport)
            #define GAMETOOL_LOCAL
        #elif defined(__CYGWIN__)
            #define GAMETOOL_API __attribute__((dllimport))
            #define GAMETOOL_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define GAMETOOL_API __attribute__((visibility("default")))
            #define GAMETOOL_LOCAL __attribute__((visibility("hidden")))
        #else
            #define GAMETOOL_API
            #define GAMETOOL_LOCAL
        #endif
    #else
        #define GAMETOOL_API
        #define GAMETOOL_LOCAL
    #endif
#endif