#pragma once

#define SKR_EDITOR_CORE_SHARED

#ifdef __cplusplus
    #define SKR_EDITOR_CORE_EXTERN_C extern "C"
#else
    #define SKR_EDITOR_CORE_EXTERN_C
#endif

#ifndef SKR_EDITOR_CORE_EXPORT
    #if defined(_MSC_VER)
        #define SKR_EDITOR_CORE_EXPORT __declspec(dllexport)
    #else
        #define SKR_EDITOR_CORE_EXPORT
    #endif
#endif

#ifdef SKR_EDITOR_CORE_IMPL
    #ifndef SKR_EDITOR_CORE_API
        #define SKR_EDITOR_CORE_API SKR_EDITOR_CORE_EXPORT
    #endif
#endif

#ifndef SKR_EDITOR_CORE_API // If the build file hasn't already defined this to be dllexport...
    #ifdef SKR_EDITOR_CORE_SHARED
        #if defined(_MSC_VER)
            #define SKR_EDITOR_CORE_API __declspec(dllimport)
            #define SKR_EDITOR_CORE_LOCAL
        #elif defined(__CYGWIN__)
            #define SKR_EDITOR_CORE_API __attribute__((dllimport))
            #define SKR_EDITOR_CORE_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define SKR_EDITOR_CORE_API __attribute__((visibility("default")))
            #define SKR_EDITOR_CORE_LOCAL __attribute__((visibility("hidden")))
        #else
            #define SKR_EDITOR_CORE_API
            #define SKR_EDITOR_CORE_LOCAL
        #endif
    #else
        #define SKR_EDITOR_CORE_API
        #define SKR_EDITOR_CORE_LOCAL
    #endif
#endif