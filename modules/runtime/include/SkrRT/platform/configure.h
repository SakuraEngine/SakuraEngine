#pragma once
#include "SkrBase/config.h"
#include "SkrRT/misc/macros.h"
#ifndef __cplusplus
    #include <stdbool.h>
#endif
#if __has_include("stdint.h")
    #include <stdint.h>
#endif

#define sstatic_ctor_name_impl(index, expr) "\"StaticCtor" #index "\" : " #expr
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
#define simport_struct_impl_impl(idx, name) \
    struct sreflect import_##idx {          \
        using type = ::name;                \
    } sattr("inject" : #name)
#define simport_struct_impl(idx, name) simport_struct_impl_impl(idx, name)
#define simport_struct(name) simport_struct_impl(__COUNTER__, name)

typedef struct $T {
    uint32_t _;
} $T;
typedef struct $Super {
    uint32_t _;
} $Super;
typedef struct $Owner {
    uint32_t _;
} $Owner;
typedef struct $Module {
    uint32_t _;
} $Module;
extern const char* $name;

#if defined(__cplusplus)
    #define SKR_ENUM(inttype) : inttype
#else
    #define SKR_ENUM(inttype)
#endif

#ifndef SKR_STATIC_API
    #define SKR_STATIC_API
#endif

#ifndef SKR_RUNTIME_API // If the build file hasn't already defined this to be dllexport...
    #ifdef RUNTIME_SHARED
        #if defined(_MSC_VER)
            #define SKR_RUNTIME_API __declspec(dllimport)
            #define SKR_RUNTIME_LOCAL
        #elif defined(__CYGWIN__)
            #define SKR_RUNTIME_API __attribute__((dllimport))
            #define SKR_RUNTIME_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define SKR_RUNTIME_API __attribute__((visibility("default")))
            #define SKR_RUNTIME_LOCAL __attribute__((visibility("hidden")))
        #else
            #define SKR_RUNTIME_API
            #define SKR_RUNTIME_LOCAL
        #endif
    #else
        #define SKR_RUNTIME_API
        #define SKR_RUNTIME_LOCAL
    #endif
#endif

#ifndef SKR_IMPORT_API
    #define SKR_IMPORT_API SKR_EXTERN_C SKR_IMPORT
#endif

#if defined(__cplusplus)
    #define DECLARE_ZERO(type, var)                                                                    \
        static_assert(std::is_trivially_constructible<type>::value, "not trival, 0 init is invalid!"); \
        type var = {};
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
#elif SKR_ARCH_WA32
    #define size_t uint32_t;
typedef int64_t host_ptr_t;
#elif SKR_ARCH_WA64
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
#ifndef SKR_KINDA_SMALL_NUMBER
    #define SKR_KINDA_SMALL_NUMBER (1.e-4)
#endif

#ifndef SKR_SMALL_NUMBER
    #define SKR_SMALL_NUMBER (1.e-8)
#endif

#ifndef SKR_THRESH_VECTOR_NORMALIZED
    #define SKR_THRESH_VECTOR_NORMALIZED 0.01
#endif

#define SKR_RESOURCE_DEV_MODE