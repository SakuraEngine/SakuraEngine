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

#ifndef __cplusplus
    #include <stdbool.h>
#endif

#if __has_include("stdint.h")
    #include <stdint.h>
#endif

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