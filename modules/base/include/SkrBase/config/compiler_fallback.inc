#ifdef __SKR_COMPILER_FALLBACK_INC__
    #error "compiler_fallback.inc included twice"
#endif
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
//      SKR_CXX_VER
//-------------------------------------------------------------------------------

// compiler
#ifndef SKR_COMPILER_GCC
    #define SKR_COMPILER_GCC 0
#endif
#ifndef SKR_COMPILER_MSVC
    #define SKR_COMPILER_MSVC 0
#endif
#ifndef SKR_COMPILER_CLANG
    #define SKR_COMPILER_CLANG 0
#endif
#ifndef SKR_COMPILER_CLANG_CL
    #define SKR_COMPILER_CLANG_CL 0
#endif

// cxx version
#ifndef SKR_CXX_11
    #define SKR_CXX_11 0
#endif
#ifndef SKR_CXX_14
    #define SKR_CXX_14 0
#endif
#ifndef SKR_CXX_17
    #define SKR_CXX_17 0
#endif
#ifndef SKR_CXX_20
    #define SKR_CXX_20 0
#endif

#define __SKR_COMPILER_FALLBACK_INC__