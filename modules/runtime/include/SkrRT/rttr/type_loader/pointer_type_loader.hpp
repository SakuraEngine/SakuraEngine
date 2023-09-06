#pragma once
#include "SkrRT/rttr/type_loader/generic_type_loader.hpp"
#include "SkrRT/rttr/type/pointer_type.hpp"
#include "SkrRT/rttr/type_registry.hpp"

namespace skr::rttr
{
struct PointerTypeLoader final : public GenericTypeLoader {
    Type* load(Span<TypeDesc> desc) override
    {
        SKR_ASSERT(desc[0].type() == SKR_TYPE_DESC_TYPE_GUID);
        SKR_ASSERT(desc[0].value_guid() == kPointerGenericGUID);
        return SkrNew<PointerType>(get_type_from_type_desc(desc.subspan(1)));
    }
    void destroy(Type* type) override
    {
        SkrDelete(type);
    }
};
} // namespace skr::rttr