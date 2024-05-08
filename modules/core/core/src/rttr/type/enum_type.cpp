#include "SkrRTTR/type/enum_type.hpp"

namespace skr::rttr
{
EnumType::EnumType(Type* underlying_type, GUID type_id, skr::String name)
    : Type(ETypeCategory::SKR_TYPE_CATEGORY_ENUM, std::move(name), type_id, underlying_type->size(), underlying_type->alignment())
    , _underlying_type(underlying_type)
{
}

} // namespace skr::rttr