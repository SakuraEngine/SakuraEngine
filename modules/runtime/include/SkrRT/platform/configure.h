#pragma once
#include "misc/macros.h"
#ifndef __cplusplus
    #include <stdbool.h>
#endif
#if __has_include("stdint.h")
    #include <stdint.h>
#endif

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

#if defined(_MSC_VER)
    #define SKR_ALIGNAS(x) __declspec(align(x))
#else
    #define SKR_ALIGNAS(x) __attribute__((aligned(x)))
#endif

#ifndef SKR_ASSUME
	#if defined(__clang__)
		#define SKR_ASSUME(x) __builtin_assume(x)
	#elif defined(_MSC_VER)
		#define SKR_ASSUME(x) __assume(x)
	#else
		#define SKR_ASSUME(x)
	#endif
#endif

#define sstatic_ctor_name_impl(index, expr) "\"StaticCtor" # index "\" : " #expr
#define sstatic_ctor_name(index, expr) sstatic_ctor_name_impl(index, #expr)
#ifdef __meta__
    #define sreflect __attribute__((annotate("__reflect__")))
    #define sfull_reflect __attribute__((annotate("__full_reflect__")))
    #define snoreflect __attribute__((annotate("__noreflect__")))
    #define sattr(...) __attribute__((annotate(SKR_MAKE_STRING(__VA_ARGS__))))
    #define spush_attr(...) __attribute__((annotate("__push__" SKR_MAKE_STRING(__VA_ARGS__))))
    #define spop_attr() __attribute__((annotate("__pop__")))

    #define sstatic_ctor(expr) __attribute__((annotate(sstatic_ctor_name(__COUNTER__, expr))))
#else
    #define sreflect
    #define sfull_reflect
    #define snoreflect
    #define sattr(...)
    #define spush_attr(...)
    #define spop_attr()

	#if __skr_clangd__
        #define sstatic_ctor(expr) //__attribute__((assume(((void)expr, true))))
    #else
        #define sstatic_ctor(expr)
    #endif
#endif
#define sreflect_struct(...) struct sreflect sattr(__VA_ARGS__)
#define sreflect_enum(...) enum sreflect sattr(__VA_ARGS__)
#define sreflect_enum_class(...) enum class sreflect sattr(__VA_ARGS__)
#define simport_struct_impl_impl(idx, name) struct sreflect import_##idx { using type = ::name; } sattr("inject" : #name)
#define simport_struct_impl(idx, name) simport_struct_impl_impl(idx, name)
#define simport_struct(name) simport_struct_impl(__COUNTER__, name)

typedef struct $T { uint32_t _; } $T;
typedef struct $Super { uint32_t _; } $Super;
typedef struct $Owner { uint32_t _; } $Owner;
typedef struct $Module { uint32_t _; } $Module;
extern const char* $name;

#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    #define SKR_OS_UNIX
#endif

#if defined(_MSC_VER)
    #define SKR_DISABLE_OPTIMIZATION __pragma(optimize("", off))
    #define SKR_ENABLE_OPTIMIZATION __pragma(optimize("", on))
#elif defined(__clang__)
    #define SKR_DISABLE_OPTIMIZATION #pragma clang optimize off
    #define SKR_ENABLE_OPTIMIZATION #pragma clang optimize on
#endif

#if defined(__APPLE__) && defined(__MACH__)
/* Apple OSX and iOS (Darwin). */
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR == 1
    /* iOS in Xcode simulator */
    #elif TARGET_OS_IPHONE == 1
    /* iOS */
    #elif TARGET_OS_MAC == 1
        /* macOS */
        #define SKR_OS_MACOSX
    #endif
#endif

#if defined(_WIN32) || defined(_WIN64)
    #define SKR_OS_WINDOWS
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#ifndef FORCEINLINE
    #define FORCEINLINE __forceinline
#endif
    #define DEFINE_ALIGNED(def, a) __declspec(align(a)) def
#else
#ifndef FORCEINLINE
    #define FORCEINLINE inline __attribute__((always_inline))
#endif
    #define DEFINE_ALIGNED(def, a) __attribute__((aligned(a))) def
#endif

#if defined(__cplusplus)
    #define SKR_CONSTEXPR constexpr
#else
    #define SKR_CONSTEXPR const
#endif

#ifdef __cplusplus
    #define RUNTIME_EXTERN_C extern "C"
#else
    #define RUNTIME_EXTERN_C extern
#endif

#if defined(__cplusplus)
    #define SKRENUM(inttype) : inttype
#else
    #define SKRENUM(inttype)
#endif

#ifndef RUNTIME_IMPORT
    #if defined(_MSC_VER)
        #define RUNTIME_IMPORT __declspec(dllimport)
    #else
        #define RUNTIME_IMPORT __attribute__((visibility("default")))
    #endif
#endif

#ifndef RUNTIME_EXPORT
    #if defined(_MSC_VER)
        // MSVC linker trims symbols, the 'dllexport' attribute prevents this.
        // But we are not archiving DLL files with SHIPPING_ONE_ARCHIVE mode.
        // TODO: do something with this workaround
        #define RUNTIME_EXPORT __declspec(dllexport)
    #else
        #define RUNTIME_EXPORT __attribute__((visibility("default")))
    #endif
#endif

#ifndef SKR_MANUAL_CONFIG_CPU_ARCHITECTURE
    #if defined(__x86_64__) || defined(_M_X64) || defined(_AMD64_) || defined(_M_AMD64)
        #define SKR_PLATFORM_X86_64
    #elif defined(__i386) || defined(_M_IX86) || defined(_X86_)
        #define SKR_PLATFORM_X86
    #elif defined(__aarch64__) || defined(__AARCH64) || defined(_M_ARM64)
        #define SKR_PLATFORM_ARM64
    #elif defined(__arm__) || defined(_M_ARM)
        #define SKR_PLATFORM_ARM32
    #elif defined(__POWERPC64__) || defined(__powerpc64__)
        #define SKR_PLATFORM_POWERPC64
    #elif defined(__POWERPC__) || defined(__powerpc__)
        #define SKR_PLATFORM_POWERPC32
    #elif defined(__wasm64__)
        #define SKR_PLATFORM_WA
        #define SKR_PLATFORM_WA64
    #elif defined(__wasm__) || defined(__EMSCRIPTEN__) || defined(__wasi__)
        #define SKR_PLATFORM_WA
        #define SKR_PLATFORM_WA32
    #else
        #error Unrecognized CPU was used.
    #endif
#endif

#ifndef RUNTIME_STATIC_API
    #define RUNTIME_STATIC_API
#endif

#ifdef RUNTIME_IMPL
    #ifndef RUNTIME_API
        #define RUNTIME_API RUNTIME_EXPORT
    #endif
#endif

#ifndef RUNTIME_API // If the build file hasn't already defined this to be dllexport...
    #ifdef RUNTIME_SHARED
        #if defined(_MSC_VER)
            #define RUNTIME_API __declspec(dllimport)
            #define RUNTIME_LOCAL
        #elif defined(__CYGWIN__)
            #define RUNTIME_API __attribute__((dllimport))
            #define RUNTIME_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define RUNTIME_API __attribute__((visibility("default")))
            #define RUNTIME_LOCAL __attribute__((visibility("hidden")))
        #else
            #define RUNTIME_API
            #define RUNTIME_LOCAL
        #endif
    #else
        #define RUNTIME_API
        #define RUNTIME_LOCAL
    #endif
#endif

#ifndef SKR_IMPORT_API
    #define SKR_IMPORT_API RUNTIME_EXTERN_C RUNTIME_IMPORT
#endif

#if defined(__cplusplus)
    #define DECLARE_ZERO(type, var) static_assert(std::is_trivially_constructible<type>::value, "not trival, 0 init is invalid!"); type var = {};
#else
    #define DECLARE_ZERO(type, var) type var = { 0 };
#endif

// VLA
#ifndef __cplusplus
    #if defined(_MSC_VER) && !defined(__clang__)
        #define DECLARE_ZERO_VLA(type, var, num)              \
            type* var = (type*)_alloca(sizeof(type) * (num)); \
            memset((void*)(var), 0, sizeof(type) * (num));
    #else
        #define DECLARE_ZERO_VLA(type, var, num) \
            type var[(num)];                     \
            memset((void*)(var), 0, sizeof(type) * (num));
    #endif
#endif

// PTR SIZE
#if INTPTR_MAX == 0x7FFFFFFFFFFFFFFFLL
    #define PTR_SIZE 8
#elif INTPTR_MAX == 0x7FFFFFFF
    #define PTR_SIZE 4
#else
    #error unsupported platform
#endif

#if defined(_MSC_VER)
    #include <crtdbg.h>
    #define COMPILE_ASSERT(exp) _STATIC_ASSERT(exp)

    #include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#if defined(__clang__)
    #define SKR_UNREF_PARAM(x) (void)x
#else
    #define SKR_UNREF_PARAM(x) (x)
#endif
    #define SKR_ALIGNAS(x) __declspec(align(x))
    #define SKR_DEFINE_ALIGNED(def, a) __declspec(align(a)) def
    #define SKR_CALLCONV __cdecl
    #if !defined(__clang__)
        #if !defined(_DEBUG) && !defined(NDEBUG)
            #define NDEBUG
        #endif

        #if defined(_M_X64)
            #define ARCH_X64
            #define ARCH_X86_FAMILY
        #elif defined(_M_IX86)
            #define ARCH_X86
            #define ARCH_X86_FAMILY
        #else
            #error "Unsupported architecture for msvc compiler"
        #endif
    #endif
#elif defined(SKR_PLATFORM_WA32)
    #define size_t uint32_t;
typedef int64_t host_ptr_t;
#elif defined(SKR_PLATFORM_WA64)
    #define size_t uint64_t;
typedef int64_t host_ptr_t;
#elif defined(__GNUC__) || defined(__clang__)
    #include <sys/types.h>
    #include <assert.h>

    #ifdef __OPTIMIZE__
        // Some platforms define NDEBUG for Release builds
        #ifndef NDEBUG
            #define NDEBUG
        #endif
    #elif !defined(_MSC_VER)
        #define _DEBUG 1
    #endif

    #ifdef __APPLE__
        #define NOREFS __unsafe_unretained
    #endif

    #define SKR_UNREF_PARAM(x) ((void)(x))
    #define SKR_ALIGNAS(x) __attribute__((aligned(x)))
    #define SKR_DEFINE_ALIGNED(def, a) __attribute__((aligned(a))) def
    #define SKR_CALLCONV

    #ifdef __clang__
        #define COMPILE_ASSERT(exp) _Static_assert(exp, #exp)
    #else
        #define COMPILE_ASSERT(exp) static_assert(exp, #exp)
    #endif

    #if defined(__i386__)
        #define ARCH_X86
        #define ARCH_X86_FAMILY
    #elif defined(__x86_64__)
        #define ARCH_X64
        #define ARCH_X86_FAMILY
    #elif defined(__arm__)
        #define ARCH_ARM
        #define ARCH_ARM_FAMILY
    #elif defined(__aarch64__)
        #define ARCH_ARM64
        #define ARCH_ARM_FAMILY
    #elif defined(__EMSCRIPTEN__) || defined(__wasi__)
        #define ARCH_WA
        #define ARCH_WEB_FAMILY
    #else
        #error "Unsupported architecture for gcc compiler"
    #endif

#else
    #error Unknown language dialect
#endif

#ifndef RUNTIME_MANUAL_CONFIG_CPU_TRAITS
    #if defined(__AVX__)
        #define SKR_PLATFORM_AVX
    #endif
    #if defined(__AVX2__)
        #define SKR_PLATFORM_AVX2
    #endif

    #if defined(SKR_PLATFORM_X86)
        #define SKR_PLATFORM_32BIT
        #define SKR_PLATFORM_LITTLE_ENDIAN
        #define SKR_PLATFORM_SSE
        #define SKR_PLATFORM_SSE2
    #endif

    #if defined(SKR_PLATFORM_X86_64)
        #define SKR_PLATFORM_64BIT
        #define SKR_PLATFORM_LITTLE_ENDIAN
        #define SKR_PLATFORM_SSE
        #define SKR_PLATFORM_SSE2
    #endif

    #if defined(SKR_PLATFORM_ARM32)
        #define SKR_PLATFORM_32BIT
        #define SKR_PLATFORM_LITTLE_ENDIAN
    #endif

    #if defined(SKR_PLATFORM_ARM64)
        #define SKR_PLATFORM_64BIT
        #define SKR_PLATFORM_LITTLE_ENDIAN
        #define SKR_PLATFORM_SSE
        #define SKR_PLATFORM_SSE2
    #endif

    #if defined(SKR_PLATFORM_POWERPC32)
        #define SKR_PLATFORM_32BIT
        #define SKR_PLATFORM_BIG_ENDIAN
    #endif

    #if defined(SKR_PLATFORM_POWERPC64)
        #define SKR_PLATFORM_64BIT
        #define SKR_PLATFORM_BIG_ENDIAN
    #endif
#endif

#ifndef RUNTIME_MANUAL_CONFIG_COMPILER
    #if defined(_MSC_VER)
        #define RUNTIME_COMPILER_MSVC
    #endif

    #if defined(__clang__)
        #define RUNTIME_COMPILER_CLANG
    #elif defined(__GNUC__)
        #define RUNTIME_COMPILER_GCC
    #elif defined(_MSC_VER)
    #else
        #error Unrecognized compiler was used.
    #endif
#endif

#if defined(RUNTIME_COMPILER_MSVC) && !defined(RUNTIME_COMPILER_CLANG)
#define SKR_TEMPLATE
#else
#define SKR_TEMPLATE template
#endif

#ifndef RUNTIME_MANUAL_CONFIG_COMPILER_TRAITS
    #if defined(RUNTIME_COMPILER_MSVC)
        #define RUNTIME_COMPILER_VERSION _MSC_VER
    #elif defined(RUNTIME_COMPILER_CLANG)
        #define RUNTIME_COMPILER_VERSION (__clang_major__ * 100 + __clang_minor__)
    #elif defined(RUNTIME_COMPILER_GCC)
        #define RUNTIME_COMPILER_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
    #endif
#endif

#ifndef RUNTIME_MANUAL_CONFIG_CPP_STANDARD
    #if (defined(RUNTIME_COMPILER_CLANG) || defined(RUNTIME_COMPILER_GCC))
        #if __cplusplus >= 201703L
            #define RUNTIME_COMPILER_CPP17
        #endif
        #if __cplusplus >= 201402L
            #define RUNTIME_COMPILER_CPP14
        #endif
    #elif defined(RUNTIME_COMPILER_MSVC)
        #if (RUNTIME_COMPILER_VERSION >= 1920) // VS 2019
            #define RUNTIME_COMPILER_CPP17
        #endif
        #if (RUNTIME_COMPILER_VERSION >= 1910) // VS 2017
            #define RUNTIME_COMPILER_CPP14
        #endif
    #else
        #error "Failed to delect C++ standard version."
    #endif
#endif // RUNTIME_MANUAL_CONFIG_CPP_STANDARD_VERSION

// no vtable
#ifdef _MSC_VER
    #define RUNTIME_NOVTABLE __declspec(novtable)
#else
    #define RUNTIME_NOVTABLE
#endif

// inline defs
#ifndef RUNTIME_FORCEINLINE
    #ifdef RUNTIME_COMPILER_MSVC
        #define RUNTIME_FORCEINLINE __forceinline
    #else
        #define RUNTIME_FORCEINLINE inline
    #endif
#endif
#define RUNTIME_INLINE inline
#ifdef __cplusplus
    // By Default we use cpp-standard above 2011XXL
    #define SKR_NOEXCEPT noexcept
#else
    #define SKR_NOEXCEPT
#endif

// Platform Specific Configure
#define SKR_HEADER_SCOPE_DEFINING_PLATFORM_CONFIGURE
#ifdef __APPLE__
    #include "apple/configure.h"
#endif
#ifdef _WIN32
    #include "win/configure.h"
#endif
#ifndef OS_DPI
    #define OS_DPI 72
#endif
#undef SKR_HEADER_SCOPE_DEFINING_PLATFORM_CONFIGURE

// Numbers
#ifndef KINDA_SMALL_NUMBER
    #define KINDA_SMALL_NUMBER (1.e-4)
#endif

#ifndef SMALL_NUMBER
    #define SMALL_NUMBER (1.e-8)
#endif

#ifndef THRESH_VECTOR_NORMALIZED
    #define THRESH_VECTOR_NORMALIZED 0.01
#endif

// TODO: 
#define SKR_RESOURCE_DEV_MODE