#include "SkrRTTR/type_loader/reference_type_loader.hpp"
#include "SkrRTTR/type/reference_type.hpp"
#include "SkrRTTR/rttr_traits.hpp"

namespace skr::rttr
{
Type* ReferenceTypeLoader::load(span<TypeDescValue> desc)
{
    SKR_ASSERT(desc[0].type() == SKR_TYPE_DESC_TYPE_TYPE_ID);

    Type* target_type = nullptr;
    return SkrNew<ReferenceType>(target_type, format(u8"{}&", target_type->name()));
}
void ReferenceTypeLoader::destroy(Type* type)
{
    SkrDelete(type);
}
} // namespace skr::rttr