#pragma once
#include "SkrRT/rttr/type_loader/generic_type_loader.hpp"
#include "SkrRT/rttr/type/pointer_type.hpp"
#include "SkrRT/rttr/type_registry.hpp"

namespace skr::rttr
{
struct SKR_RUNTIME_API PointerTypeLoader final : public GenericTypeLoader {
    Type* load(Span<TypeDesc> desc) override;
    void  destroy(Type* type) override;
};
} // namespace skr::rttr