//DO NOT MODIFY THIS FILE
#pragma once
#ifdef __cplusplus
    #define USDCORE_EXTERN_C extern "C"
#else
    #define USDCORE_EXTERN_C
#endif

#ifndef USDCORE_EXPORT
    #if defined(_MSC_VER)
        #define USDCORE_EXPORT __declspec(dllexport)
    #else
        #define USDCORE_EXPORT __attribute__((visibility("default")))
    #endif
#endif

#ifdef USDCORE_IMPL
    #ifndef USDCORE_API
        #define USDCORE_API USDCORE_EXPORT
    #endif
#endif

#ifndef USDCORE_API // If the build file hasn't already defined this to be dllexport...
    #ifdef USDCORE_SHARED
        #if defined(_MSC_VER)
            #define USDCORE_API __declspec(dllimport)
            #define USDCORE_LOCAL
        #elif defined(__CYGWIN__)
            #define USDCORE_API __attribute__((dllimport))
            #define USDCORE_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define USDCORE_API __attribute__((visibility("default")))
            #define USDCORE_LOCAL __attribute__((visibility("hidden")))
        #else
            #define USDCORE_API
            #define USDCORE_LOCAL
        #endif
    #else
        #define USDCORE_API
        #define USDCORE_LOCAL
    #endif
#endif
