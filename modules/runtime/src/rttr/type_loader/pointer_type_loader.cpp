#include "SkrRT/rttr/type_loader/pointer_type_loader.hpp"
#include "SkrRT/rttr/type/pointer_type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
Type* PointerTypeLoader::load(Span<TypeDesc> desc)
{
    SKR_ASSERT(desc[0].type() == SKR_TYPE_DESC_TYPE_GUID);
    SKR_ASSERT(desc[0].value_guid() == kPointerGenericGUID);
    return SkrNew<PointerType>(get_type_from_type_desc(desc.subspan(1)));
}
void PointerTypeLoader::destroy(Type* type)
{
    SkrDelete(type);
}
} // namespace skr::rttr