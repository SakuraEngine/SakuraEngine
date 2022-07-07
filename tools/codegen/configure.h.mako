#pragma once

#ifdef __cplusplus
    #define ${api}_EXTERN_C extern "C"
#else
    #define ${api}_EXTERN_C
#endif

#ifndef ${api}_EXPORT
    #if defined(_MSC_VER)
        #define ${api}_EXPORT __declspec(dllexport)
    #else
        #define ${api}_EXPORT
    #endif
#endif

#ifdef ${api}_IMPL
    #ifndef ${api}_API
        #define ${api}_API ${api}_EXPORT
    #endif
#endif

#ifndef ${api}_API // If the build file hasn't already defined this to be dllexport...
    #ifdef ${api}_SHARED
        #if defined(_MSC_VER)
            #define ${api}_API __declspec(dllimport)
            #define ${api}_LOCAL
        #elif defined(__CYGWIN__)
            #define ${api}_API __attribute__((dllimport))
            #define ${api}_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define ${api}_API __attribute__((visibility("default")))
            #define ${api}_LOCAL __attribute__((visibility("hidden")))
        #else
            #define ${api}_API
            #define ${api}_LOCAL
        #endif
    #else
        #define ${api}_API
        #define ${api}_LOCAL
    #endif
#endif