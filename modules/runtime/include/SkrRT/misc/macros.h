#pragma once

#pragma region stringizing

#ifndef SKR_STRINGIZING
#define SKR_STRINGIZING(...) #__VA_ARGS__
#endif

#ifndef SKR_MAKE_STRING
#define SKR_MAKE_STRING(...) SKR_STRINGIZING(__VA_ARGS__)
#endif

#ifndef SKR_FILE_LINE
#define SKR_FILE_LINE __FILE__ ":" SKR_MAKE_STRING(__LINE__)
#endif

#pragma endregion


#pragma region utf-8

#if __cplusplus >= 201100L
#define SKR_UTF8(str) u8##str
#else
#define SKR_UTF8(str) str
#endif

#if __cpp_char8_t
#define CHAR8_T_DEFINED
#endif

#ifndef CHAR8_T_DEFINED // If the user hasn't already defined these...
    #if defined(EA_PLATFORM_APPLE)
        #define char8_t char // The Apple debugger is too stupid to realize char8_t is typedef'd to char, so we #define it.
    #else
        typedef char char8_t;
    #endif
    #define CHAR8_T_DEFINED
#endif

#pragma endregion


#pragma region typedecl

#ifdef __cplusplus
#define SKR_DECLARE_TYPE_ID_FWD(ns, type, ctype) namespace ns { struct type; }  using ctype##_t = ns::type; using ctype##_id = ns::type*;
#else
#define SKR_DECLARE_TYPE_ID_FWD(ns, type, ctype) typedef struct ctype##_t ctype##_t; typedef struct ctype* ctype##_id;
#endif

#ifdef __cplusplus
#define SKR_DECLARE_TYPE_ID(type, ctype) typedef struct type ctype##_t; typedef type* ctype##_id;
#else
#define SKR_DECLARE_TYPE_ID(type, ctype) typedef struct ctype##_t ctype##_t; typedef ctype##_t* ctype##_id;
#endif

#pragma endregion


#pragma region tracy

#if !defined(TRACY_ENABLE) && !defined(TRACY_OVERRIDE_DISABLE) && !defined(TRACY_OVERRIDE_ENABLE)
    #ifdef _DEBUG
        #define TRACY_ENABLE
    #else
    #endif
#endif

#ifdef TRACY_OVERRIDE_ENABLE
    #define TRACY_ENABLE
#endif

#if defined(TRACY_ENABLE) || defined(TRACY_OVERRIDE_ENABLE)
    #ifndef TRACY_IMPORTS
    #define TRACY_IMPORTS
    #endif
    
    #ifndef TRACY_ON_DEMAND
    #define TRACY_ON_DEMAND
    #endif

    #ifndef TRACY_FIBERS
    #define TRACY_FIBERS
    #endif
#endif

#ifdef TRACY_ENABLE
#define TRACY_TRACE_ALLOCATION
#endif

#pragma endregion


#define SKR_IS_BIG_ENDIAN 0
#define SKR_IS_LITTLE_ENDIAN 1


#ifdef __cplusplus
namespace skr
{
    template <typename T> struct hash;
}

typedef struct Dummy {
    int dummy;
    int dummy2();
} Dummy;
extern const int Dummy::* $field_ptr;
extern int(Dummy::* $method_ptr)();
#endif