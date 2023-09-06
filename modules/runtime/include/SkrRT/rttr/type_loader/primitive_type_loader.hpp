#pragma once
#include "SkrRT/rttr/type/primitive_type.hpp"
#include "SkrRT/rttr/type_loader/type_loader.hpp"

namespace skr::rttr
{
template <typename T>
struct PrimitiveTypeLoader final : public TypeLoader {
    Type* load() override
    {
        Type* type = SkrNew<PrimitiveType<T>>();
        return type;
    }
    void destroy(Type* type) override
    {
        SkrDelete(type);
    }
};
} // namespace skr::rttr