#pragma once
#include "SkrRT/rttr/type_loader/generic_type_loader.hpp"
#include "SkrRT/rttr/type/vector_type.hpp"

namespace skr::rttr
{
struct SKR_RUNTIME_API VectorTypeLoader final : public GenericTypeLoader {
    Type* load(span<TypeDesc> desc) override;
    void  destroy(Type* type) override;
};
} // namespace skr::rttr