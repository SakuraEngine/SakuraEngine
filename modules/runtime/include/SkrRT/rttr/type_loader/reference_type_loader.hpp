#pragma once
#include "SkrRT/rttr/type_loader/generic_type_loader.hpp"
#include "SkrRT/rttr/type/reference_type.hpp"

namespace skr::rttr
{
struct ReferenceTypeLoader final : public GenericTypeLoader {
    Type* load(Span<TypeDesc> desc) override
    {
        SKR_ASSERT(desc[0].type() == SKR_TYPE_DESC_TYPE_GUID);
        SKR_ASSERT(desc[0].value_guid() == kReferenceGenericGUID);
        return SkrNew<ReferenceType>(get_type_from_type_desc(desc.subspan(1)));
    }
    void destroy(Type* type) override
    {
        SkrDelete(type);
    }
};
} // namespace skr::rttr