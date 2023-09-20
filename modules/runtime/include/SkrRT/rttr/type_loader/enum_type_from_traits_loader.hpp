#pragma once
#include "SkrRT/rttr/type_loader/type_loader.hpp"
#include "SkrRT/rttr/type/enum_type_from_traits.hpp"

namespace skr::rttr
{
template <typename T>
struct EnumTypeFromTraitsLoader final : public TypeLoader {
    Type* load() override
    {
        return SkrNew<EnumTypeFromTraits<T>>();
    }
    void destroy(Type* type) override
    {
        SkrDelete(type);
    }
};
} // namespace skr::rttr