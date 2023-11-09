#pragma once
#include "SkrBase/config.h"

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