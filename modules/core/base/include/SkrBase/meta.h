#pragma once
#include "SkrBase/config.h"

// basic meta
#ifdef __meta__
    #define sreflect __attribute__((annotate("__reflect__")))
    #define sfull_reflect __attribute__((annotate("__full_reflect__")))
    #define snoreflect __attribute__((annotate("__noreflect__")))
    #define sattr(...) __attribute__((annotate(SKR_MAKE_STRING(__VA_ARGS__))))
    #define spush_attr(...) __attribute__((annotate("__push__" SKR_MAKE_STRING(__VA_ARGS__))))
    #define spop_attr() __attribute__((annotate("__pop__")))
#else
    #define sreflect
    #define sfull_reflect
    #define snoreflect
    #define sattr(...)
    #define spush_attr(...)
    #define spop_attr()
#endif

// common use meta
#define sreflect_struct(...) struct sreflect sattr(__VA_ARGS__)
#define sreflect_enum(...) enum sreflect sattr(__VA_ARGS__)
#define sreflect_enum_class(...) enum class sreflect sattr(__VA_ARGS__)

// meta tools
#ifdef __meta__
    #define unimplemented_no_meta(__TYPE, __MSG)
#else
    #define unimplemented_no_meta(__TYPE, __MSG) static_assert(std::is_same_v<__TYPE, __TYPE*>, __MSG)
#endif

// generate body
// TODO. 统一化的 GENERATE_BODY 会导致 meta 阶段无法过编译（典型的例子是继承了接口但实现在 GENERATE_BODY 内，或某些符号在 GENERATE_BODY 内，可是 Header用到了），
//       对过不了编的子功能，需要手动加要给 XXXXXX_META_BODY 进行抑制
// TODO. GENERATE_BODY 检查，在有内容时候需要检测是否漏写 GENERATE_BODY 宏
#define SKR_GENERATE_BODY()
