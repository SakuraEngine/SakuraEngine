#pragma once
#include "SkrRTTR/type.hpp"

namespace skr::v8
{
struct V8RecordBindData {
    void*            data;
    skr::rttr::Type* type;
};

struct V8MethodBindData {
};

struct V8StaticMethodBindData {
};

struct V8FieldBindData {
};

struct V8StaticFieldBindData {
};

struct V8ExternBindData {
};

struct V8EnumBindData {
};
} // namespace skr::v8