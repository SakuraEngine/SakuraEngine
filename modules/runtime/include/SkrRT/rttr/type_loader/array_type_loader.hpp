#pragma once
#include "SkrRT/rttr/type_loader/generic_type_loader.hpp"
#include "SkrRT/rttr/type/array_type.hpp"

namespace skr::rttr
{
struct RUNTIME_API ArrayTypeLoader final : public GenericTypeLoader {
    Type* load(Span<TypeDesc> desc) override;
    void  destroy(Type* type) override;
};
} // namespace skr::rttr