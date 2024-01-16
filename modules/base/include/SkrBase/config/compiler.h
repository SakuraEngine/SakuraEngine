#pragma once
//-------------------------------------------------------------------------------
// -> compiler
//      SKR_COMPILER_GCC
//      SKR_COMPILER_MSVC
//      SKR_COMPILER_CLANG
//      SKR_COMPILER_CLANG_CL
//
// -> cxx version
//      SKR_CXX_11
//      SKR_CXX_14
//      SKR_CXX_17
//      SKR_CXX_20
//      SKR_CXX_VERSION
//
// -> other
//      SKR_COMPILER_VERSION
//-------------------------------------------------------------------------------

#include "platform.h"

// compiler def
#if defined(__clang__) && !defined(_MSC_VER)
    #define SKR_COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define SKR_COMPILER_GCC 1
#elif defined(_MSC_VER)
    #if defined(__clang__) && !defined(SKR_COMPILER_CLANG_CL)
        #define SKR_COMPILER_CLANG_CL 1
    #elif !defined(SKR_COMPILER_MSVC)
        #define SKR_COMPILER_MSVC 1
    #endif
#endif

// cxx 11
#if !defined(SKR_CXX_11) && defined(__cplusplus)
    #if (__cplusplus >= 201103L)
        #define SKR_CXX_11 1
    #elif defined(__GNUC__) && defined(__GXX_EXPERIMENTAL_CXX0X__)
        #define SKR_CXX_11 1
    #elif defined(_MSC_VER) && _MSC_VER >= 1600
        #define SKR_CXX_11 1
    #endif
#endif

// cxx 14
#if !defined(SKR_CXX_14) && defined(__cplusplus)
    #if (__cplusplus >= 201402L)
        #define SKR_CXX_14 1
    #elif defined(_MSC_VER) && (_MSC_VER >= 1900)
        #define SKR_CXX_14 1
    #endif
#endif

// cxx 17
#if !defined(SKR_CXX_17) && defined(__cplusplus)
    #if (__cplusplus >= 201703L)
        #define SKR_CXX_17 1
    #elif defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L)
        #define SKR_CXX_17 1
    #endif
#endif

// cxx 20
#if !defined(SKR_CXX_20) && defined(__cplusplus)
    #if (__cplusplus >= 202002L)
        #define SKR_CXX_20 1
    #elif defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L)
        #define SKR_CXX_20 1
    #endif
#endif

// cxx version
#if defined(SKR_CXX_20)
    #define SKR_CXX_VERSION 20
#elif defined(SKR_CXX_17)
    #define SKR_CXX_VERSION 17
#elif defined(SKR_CXX_14)
    #define SKR_CXX_VERSION 14
#elif defined(SKR_CXX_11)
    #define SKR_CXX_VERSION 11
#endif

// fall back
#include "compiler_fallback.inc"