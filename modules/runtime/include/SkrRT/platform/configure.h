#pragma once
#include "SkrBase/config.h"
#include "SkrRT/misc/macros.h"

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
#define sreflect_managed_component(...) struct sreflect sattr("component" : {"custom" : "::dual::managed_component"}) sattr(__VA_ARGS__)
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