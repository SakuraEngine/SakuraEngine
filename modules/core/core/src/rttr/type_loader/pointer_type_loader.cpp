#include "SkrRTTR/type_loader/pointer_type_loader.hpp"
#include "SkrRTTR/type/pointer_type.hpp"
#include "SkrRTTR/rttr_traits.hpp"

namespace skr::rttr
{
Type* PointerTypeLoader::load(span<TypeDescValue> desc)
{
    SKR_ASSERT(desc[0].type() == SKR_TYPE_DESC_TYPE_TYPE_ID);

    Type* target_type = nullptr;
    return SkrNew<PointerType>(target_type, format(u8"{}*", target_type->name()));
}
void PointerTypeLoader::destroy(Type* type)
{
    SkrDelete(type);
}
} // namespace skr::rttr