#pragma once
#include "SkrRTTR/type_loader/generic_type_loader.hpp"
#include "SkrRTTR/type/reference_type.hpp"

namespace skr::rttr
{
struct SKR_CORE_API ReferenceTypeLoader final : public GenericTypeLoader {
    Type* load(TypeDescView desc) override;
    void  destroy(Type* type) override;
};
} // namespace skr::rttr