#pragma once
#include "SkrBase/config.h"

// basic meta
#ifdef __meta__
    #define sattr(...) __attribute__((annotate(SKR_MAKE_STRING(__VA_ARGS__))))
    #define sreflect_struct(...) struct __attribute__((annotate("__reflect__"))) sattr(__VA_ARGS__)
    #define sreflect_enum(...) enum __attribute__((annotate("__reflect__"))) sattr(__VA_ARGS__)
    #define sreflect_enum_class(...) enum class __attribute__((annotate("__reflect__"))) sattr(__VA_ARGS__)
#else
    #define sattr(...)
    #define sreflect_struct(...) struct
    #define sreflect_enum(...) enum
    #define sreflect_enum_class(...) enum class
#endif

// meta tools
// TODO. remove it
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
