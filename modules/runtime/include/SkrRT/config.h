//DO NOT MODIFY THIS FILE
#pragma once
#include "SkrBase/config.h"
#include "SkrBase/meta.h"

#ifdef __cplusplus
    #define SKR_RUNTIME_EXTERN_C extern "C"
#else
    #define SKR_RUNTIME_EXTERN_C
#endif

#ifndef SKR_RUNTIME_EXPORT
    #if defined(_MSC_VER)
        #define SKR_RUNTIME_EXPORT __declspec(dllexport)
    #else
        #define SKR_RUNTIME_EXPORT __attribute__((visibility("default")))
    #endif
#endif

#ifdef SKR_RUNTIME_IMPL
    #ifndef SKR_RUNTIME_API
        #define SKR_RUNTIME_API SKR_RUNTIME_EXPORT
    #endif
#endif

#ifndef SKR_RUNTIME_API // If the build file hasn't already defined this to be dllexport...
    #ifdef SKR_RUNTIME_SHARED
        #if defined(_MSC_VER)
            #define SKR_RUNTIME_API __declspec(dllimport)
            #define SKR_RUNTIME_LOCAL
        #elif defined(__CYGWIN__)
            #define SKR_RUNTIME_API __attribute__((dllimport))
            #define SKR_RUNTIME_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define SKR_RUNTIME_API __attribute__((visibility("default")))
            #define SKR_RUNTIME_LOCAL __attribute__((visibility("hidden")))
        #else
            #define SKR_RUNTIME_API
            #define SKR_RUNTIME_LOCAL
        #endif
    #else
        #define SKR_RUNTIME_API
        #define SKR_RUNTIME_LOCAL
    #endif
#endif
