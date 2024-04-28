#pragma once
#include "SkrContainers/span.hpp"
#include "SkrRTTR/type_desc.hpp"

namespace skr::rttr
{
struct Type;

struct GenericTypeLoader {
    virtual ~GenericTypeLoader() = default;

    virtual Type* load(span<TypeDesc> desc) = 0;
    virtual void  destroy(Type* type)       = 0;
};
} // namespace skr::rttr