#pragma once

#ifdef __cplusplus
    #define SKR_IMGUI_EXTERN_C extern "C"
#else
    #define SKR_IMGUI_EXTERN_C
#endif

#ifndef SKR_IMGUI_EXPORT
    #if defined(_MSC_VER)
        #define SKR_IMGUI_EXPORT __declspec(dllexport)
    #else
        #define SKR_IMGUI_EXPORT
    #endif
#endif

#ifdef SKR_IMGUI_IMPL
    #ifndef SKR_IMGUI_API
        #define SKR_IMGUI_API SKR_IMGUI_EXPORT
    #endif
#endif

#ifndef SKR_IMGUI_API // If the build file hasn't already defined this to be dllexport...
    #ifdef SKR_IMGUI_SHARED
        #if defined(_MSC_VER)
            #define SKR_IMGUI_API __declspec(dllimport)
            #define SKR_IMGUI_LOCAL
        #elif defined(__CYGWIN__)
            #define SKR_IMGUI_API __attribute__((dllimport))
            #define SKR_IMGUI_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define SKR_IMGUI_API __attribute__((visibility("default")))
            #define SKR_IMGUI_LOCAL __attribute__((visibility("hidden")))
        #else
            #define SKR_IMGUI_API
            #define SKR_IMGUI_LOCAL
        #endif
    #else
        #define SKR_IMGUI_API
        #define SKR_IMGUI_LOCAL
    #endif
#endif