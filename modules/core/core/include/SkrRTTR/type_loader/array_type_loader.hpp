#pragma once
#include "SkrRTTR/type_loader/generic_type_loader.hpp"
#include "SkrRTTR/type/vector_type.hpp"

namespace skr::rttr
{
struct SKR_CORE_API VectorTypeLoader final : public GenericTypeLoader {
    Type* load(span<TypeDesc> desc) override;
    void  destroy(Type* type) override;
};
} // namespace skr::rttr