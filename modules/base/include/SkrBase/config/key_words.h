#pragma once
//-------------------------------------------------------------------------------
// unused
// alignas
// assume
// enable/disable optimization
// inline
// forceinline
// extern c
// export/import/static API
// ptr size
// no vtable
// noexcept
//-------------------------------------------------------------------------------

// UNUSED
#if defined(__cplusplus)
    #define SKR_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
    #define SKR_UNUSED __attribute__((unused))
#elif defined(_MSC_VER)
    #define SKR_UNUSED
#endif

#ifdef __cplusplus
    #define SKR_IF_CPP(...) __VA_ARGS__
#else
    #define SKR_IF_CPP(...)
#endif

#if defined(__cplusplus)
    #define SKR_CONSTEXPR constexpr
#else
    #define SKR_CONSTEXPR const
#endif

// ALIGNAS
#if defined(_MSC_VER)
    #define SKR_ALIGNAS(x) __declspec(align(x))
#else
    #define SKR_ALIGNAS(x) __attribute__((aligned(x)))
#endif

// ASSUME
#ifndef SKR_ASSUME
    #if defined(__clang__)
        #define SKR_ASSUME(x) __builtin_assume(x)
    #elif defined(_MSC_VER)
        #define SKR_ASSUME(x) __assume(x)
    #else
        #define SKR_ASSUME(x)
    #endif
#endif

// OPTIMIZATION
#if defined(_MSC_VER)
    #define SKR_DISABLE_OPTIMIZATION __pragma(optimize("", off))
    #define SKR_ENABLE_OPTIMIZATION __pragma(optimize("", on))
#elif defined(__clang__)
    #define SKR_DISABLE_OPTIMIZATION #pragma clang optimize off
    #define SKR_ENABLE_OPTIMIZATION #pragma clang optimize on
#endif

// INLINE
#if defined(__cplusplus)
    #define SKR_INLINE inline
#else
    #define SKR_INLINE
#endif

// FORCEINLINE
#if defined(_MSC_VER) && !defined(__clang__)
    #define SKR_FORCEINLINE __forceinline
#else
    #define SKR_FORCEINLINE inline __attribute__((always_inline))
#endif

// EXTERN_C
#ifdef __cplusplus
    #define SKR_EXTERN_C extern "C"
#else
    #define SKR_EXTERN_C extern
#endif

// IMPORT
#ifndef SKR_IMPORT
    #if defined(_MSC_VER)
        #define SKR_IMPORT __declspec(dllimport)
    #else
        #define SKR_IMPORT __attribute__((visibility("default")))
    #endif
#endif

// EXPORT
#ifndef SKR_EXPORT
    #if defined(_MSC_VER)
        // MSVC linker trims symbols, the 'dllexport' attribute prevents this.
        // But we are not archiving DLL files with SHIPPING_ONE_ARCHIVE mode.
        // TODO: do something with this workaround
        #define SKR_EXPORT __declspec(dllexport)
    #else
        #define SKR_EXPORT __attribute__((visibility("default")))
    #endif
#endif

// STATIC
#define SKR_STATIC_API

// PTR_SIZE
#if INTPTR_MAX == 0x7FFFFFFFFFFFFFFFLL
    #define SKR_PTR_SIZE 8
#elif INTPTR_MAX == 0x7FFFFFFF
    #define SKR_PTR_SIZE 4
#else
    #error unsupported platform
#endif

// NO_VTABLE
#ifdef _MSC_VER
    #define SKR_NO_VTABLE __declspec(novtable)
#else
    #define SKR_NO_VTABLE
#endif

// NOEXCEPT
#ifdef __cplusplus
    // By Default we use cpp-standard above 2011XXL
    #define SKR_NOEXCEPT noexcept
#else
    #define SKR_NOEXCEPT
#endif

#if defined(_MSC_VER)
    #if defined(__clang__)
        #define SKR_UNREF_PARAM(x) (void)x
    #else
        #define SKR_UNREF_PARAM(x) (x)
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #define SKR_UNREF_PARAM(x) ((void)(x))
#endif

#if defined(_MSC_VER)
    #define SKR_CALLCONV __cdecl
#elif defined(__GNUC__) || defined(__clang__)
    #define SKR_CALLCONV
#endif

#if defined(__cplusplus)
    #define SKR_DECLARE_ZERO(type, var)                                                                    \
        static_assert(std::is_trivially_constructible<type>::value, "not trival, 0 init is invalid!"); \
        type var = {};
#else
    #define SKR_DECLARE_ZERO(type, var) type var = { 0 };
#endif

// VLA
#ifndef __cplusplus
    #if defined(_MSC_VER) && !defined(__clang__)
        #define SKR_DECLARE_ZERO_VLA(type, var, num)              \
            type* var = (type*)_alloca(sizeof(type) * (num)); \
            memset((void*)(var), 0, sizeof(type) * (num));
    #else
        #define SKR_DECLARE_ZERO_VLA(type, var, num) \
            type var[(num)];                     \
            memset((void*)(var), 0, sizeof(type) * (num));
    #endif
#endif

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
#define OSTR_USE_CXX20_CHAR8_TYPE
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