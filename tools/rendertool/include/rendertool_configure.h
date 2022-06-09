#pragma once

#ifdef __cplusplus
    #define RENDERTOOL_EXTERN_C extern "C"
#else
    #define RENDERTOOL_EXTERN_C
#endif

#ifndef RENDERTOOL_EXPORT
    #if defined(_MSC_VER)
        #define RENDERTOOL_EXPORT __declspec(dllexport)
    #else
        #define RENDERTOOL_EXPORT
    #endif
#endif

#ifdef RENDERTOOL_IMPL
    #ifndef RENDERTOOL_API
        #define RENDERTOOL_API RENDERTOOL_EXPORT
    #endif
#endif

#ifndef RENDERTOOL_API // If the build file hasn't already defined this to be dllexport...
    #ifdef RENDERTOOL_SHARED
        #if defined(_MSC_VER)
            #define RENDERTOOL_API __declspec(dllimport)
            #define RENDERTOOL_LOCAL
        #elif defined(__CYGWIN__)
            #define RENDERTOOL_API __attribute__((dllimport))
            #define RENDERTOOL_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define RENDERTOOL_API __attribute__((visibility("default")))
            #define RENDERTOOL_LOCAL __attribute__((visibility("hidden")))
        #else
            #define RENDERTOOL_API
            #define RENDERTOOL_LOCAL
        #endif
    #else
        #define RENDERTOOL_API
        #define RENDERTOOL_LOCAL
    #endif
#endif