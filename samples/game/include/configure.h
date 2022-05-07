#pragma once

#ifdef __cplusplus
    #define GAMERT_EXTERN_C extern "C"
#else
    #define GAMERT_EXTERN_C
#endif

#ifndef GAMERT_EXPORT
    #if defined(_MSC_VER)
        #define GAMERT_EXPORT __declspec(dllexport)
    #else
        #define GAMERT_EXPORT
    #endif
#endif

#ifdef GAMERT_IMPL
    #ifndef GAMERT_API
        #define GAMERT_API GAMERT_EXPORT
    #endif
#endif

#ifndef GAMERT_API // If the build file hasn't already defined this to be dllexport...
    #ifdef GAMERT_SHARED
        #if defined(_MSC_VER)
            #define GAMERT_API __declspec(dllimport)
            #define GAMERT_LOCAL
        #elif defined(__CYGWIN__)
            #define GAMERT_API __attribute__((dllimport))
            #define GAMERT_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define GAMERT_API __attribute__((visibility("default")))
            #define GAMERT_LOCAL __attribute__((visibility("hidden")))
        #else
            #define GAMERT_API
            #define GAMERT_LOCAL
        #endif

        #define EA_DLL
    #else
        #define GAMERT_API
        #define GAMERT_LOCAL
    #endif
#endif