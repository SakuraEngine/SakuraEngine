#pragma once
#include "SkrRTTR/type.hpp"

namespace skr::v8
{
// TODO. 将函数签名等信息单独拷贝下来进行使用，部分重复数据可以直接使用 span 存储来减少内存分配
struct V8CtorBindData {
};
struct V8MethodBindData {
};
struct V8StaticMethodBindData {
};
struct V8ExternMethodBindData {
};

struct V8RecordBindData {
    void*            data;
    skr::rttr::Type* type;

    // TODO. ctor data
    // TODO. method data
    // TODO. static method data
    // TODO. extern method data
};

struct V8EnumBindData {
};
} // namespace skr::v8