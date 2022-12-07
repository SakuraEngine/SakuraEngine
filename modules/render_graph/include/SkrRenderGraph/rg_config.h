#pragma once
#include "cgpu/api.h"

#ifdef __cplusplus
    #define SKR_RENDER_GRAPH_EXTERN_C extern "C"
#else
    #define SKR_RENDER_GRAPH_EXTERN_C
#endif

#ifndef SKR_RENDER_GRAPH_EXPORT
    #if defined(_MSC_VER)
        #define SKR_RENDER_GRAPH_EXPORT __declspec(dllexport)
    #else
        #define SKR_RENDER_GRAPH_EXPORT
    #endif
#endif

#ifdef SKR_RENDER_GRAPH_IMPL
    #ifndef SKR_RENDER_GRAPH_API
        #define SKR_RENDER_GRAPH_API SKR_RENDER_GRAPH_EXPORT
    #endif
#endif

#ifndef SKR_RENDER_GRAPH_API // If the build file hasn't already defined this to be dllexport...
    #ifdef SKR_RENDER_GRAPH_SHARED
        #if defined(_MSC_VER)
            #define SKR_RENDER_GRAPH_API __declspec(dllimport)
            #define SKR_RENDER_GRAPH_LOCAL
        #elif defined(__CYGWIN__)
            #define SKR_RENDER_GRAPH_API __attribute__((dllimport))
            #define SKR_RENDER_GRAPH_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define SKR_RENDER_GRAPH_API __attribute__((visibility("default")))
            #define SKR_RENDER_GRAPH_LOCAL __attribute__((visibility("hidden")))
        #else
            #define SKR_RENDER_GRAPH_API
            #define SKR_RENDER_GRAPH_LOCAL
        #endif
    #else
        #define SKR_RENDER_GRAPH_API
        #define SKR_RENDER_GRAPH_LOCAL
    #endif
#endif

#define RG_USE_FIXED_STRING
#define RG_USE_FIXED_VECTOR