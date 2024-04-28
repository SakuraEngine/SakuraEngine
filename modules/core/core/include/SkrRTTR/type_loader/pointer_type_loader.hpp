#pragma once
#include "SkrRTTR/type_loader/generic_type_loader.hpp"
#include "SkrRTTR/type/pointer_type.hpp"
#include "SkrRTTR/type_registry.hpp"

namespace skr::rttr
{
struct SKR_CORE_API PointerTypeLoader final : public GenericTypeLoader {
    Type* load(span<TypeDesc> desc) override;
    void  destroy(Type* type) override;
};
} // namespace skr::rttr