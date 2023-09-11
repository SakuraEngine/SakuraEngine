#include "SkrRT/rttr/type_loader/reference_type_loader.hpp"
#include "SkrRT/rttr/type/reference_type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
Type* ReferenceTypeLoader::load(Span<TypeDesc> desc)
{
    SKR_ASSERT(desc[0].type() == SKR_TYPE_DESC_TYPE_GUID);
    SKR_ASSERT(desc[0].value_guid() == kReferenceGenericGUID);
    return SkrNew<ReferenceType>(get_type_from_type_desc(desc.subspan(1)));
}
void ReferenceTypeLoader::destroy(Type* type)
{
    SkrDelete(type);
}
} // namespace skr::rttr