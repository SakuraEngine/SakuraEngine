#pragma once
#include "SkrRT/containers_new/span.hpp"
#include "SkrRT/rttr/type_desc.hpp"

namespace skr::rttr
{
struct Type;

struct GenericTypeLoader {
    virtual ~GenericTypeLoader() = default;

    virtual Type* load(Span<TypeDesc> desc) = 0;
    virtual void  destroy(Type* type)       = 0;
};
} // namespace skr::rttr