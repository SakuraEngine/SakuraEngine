#pragma once

#ifdef __cplusplus
    #define SKR_RENDERER_EXTERN_C extern "C"
#else
    #define SKR_RENDERER_EXTERN_C
#endif

#ifndef SKR_RENDERER_EXPORT
    #if defined(_MSC_VER)
        #define SKR_RENDERER_EXPORT __declspec(dllexport)
    #else
        #define SKR_RENDERER_EXPORT
    #endif
#endif

#ifdef SKR_RENDERER_IMPL
    #ifndef SKR_RENDERER_API
        #define SKR_RENDERER_API SKR_RENDERER_EXPORT
    #endif
#endif

#ifndef SKR_RENDERER_API // If the build file hasn't already defined this to be dllexport...
    #ifdef SKR_RENDERER_SHARED
        #if defined(_MSC_VER)
            #define SKR_RENDERER_API __declspec(dllimport)
            #define SKR_RENDERER_LOCAL
        #elif defined(__CYGWIN__)
            #define SKR_RENDERER_API __attribute__((dllimport))
            #define SKR_RENDERER_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define SKR_RENDERER_API __attribute__((visibility("default")))
            #define SKR_RENDERER_LOCAL __attribute__((visibility("hidden")))
        #else
            #define SKR_RENDERER_API
            #define SKR_RENDERER_LOCAL
        #endif
    #else
        #define SKR_RENDERER_API
        #define SKR_RENDERER_LOCAL
    #endif
#endif