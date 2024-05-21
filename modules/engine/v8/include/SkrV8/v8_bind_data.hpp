#pragma once
#include "SkrRTTR/type.hpp"

namespace skr::v8
{
struct V8BindData {
    void*            data;
    skr::rttr::Type* type;
};
} // namespace skr::v8