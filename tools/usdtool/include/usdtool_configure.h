#pragma once

#ifdef __cplusplus
    #define USDTOOL_EXTERN_C extern "C"
#else
    #define USDTOOL_EXTERN_C
#endif

#ifndef USDTOOL_EXPORT
    #if defined(_MSC_VER)
        #define USDTOOL_EXPORT __declspec(dllexport)
    #else
        #define USDTOOL_EXPORT
    #endif
#endif

#ifdef USDTOOL_IMPL
    #ifndef USDTOOL_API
        #define USDTOOL_API USDTOOL_EXPORT
    #endif
#endif

#ifndef USDTOOL_API // If the build file hasn't already defined this to be dllexport...
    #ifdef USDTOOL_SHARED
        #if defined(_MSC_VER)
            #define USDTOOL_API __declspec(dllimport)
            #define USDTOOL_LOCAL
        #elif defined(__CYGWIN__)
            #define USDTOOL_API __attribute__((dllimport))
            #define USDTOOL_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define USDTOOL_API __attribute__((visibility("default")))
            #define USDTOOL_LOCAL __attribute__((visibility("hidden")))
        #else
            #define USDTOOL_API
            #define USDTOOL_LOCAL
        #endif
    #else
        #define USDTOOL_API
        #define USDTOOL_LOCAL
    #endif
#endif