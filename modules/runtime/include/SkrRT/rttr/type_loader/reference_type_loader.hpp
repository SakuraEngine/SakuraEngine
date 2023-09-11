#pragma once
#include "SkrRT/rttr/type_loader/generic_type_loader.hpp"
#include "SkrRT/rttr/type/reference_type.hpp"

namespace skr::rttr
{
struct SKR_RUNTIME_API ReferenceTypeLoader final : public GenericTypeLoader {
    Type* load(Span<TypeDesc> desc) override;
    void  destroy(Type* type) override;
};
} // namespace skr::rttr