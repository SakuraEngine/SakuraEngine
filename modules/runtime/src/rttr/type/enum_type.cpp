#include "SkrRT/rttr/type/enum_type.hpp"

namespace skr::rttr
{
EnumType::EnumType(Type* underlying_type, GUID type_id)
    : Type(ETypeCategory::SKR_TYPE_CATEGORY_ENUM, type_id, underlying_type->size(), underlying_type->alignment())
    , _underlying_type(underlying_type)
{
}

bool EnumType::call_ctor(void* ptr) const
{
    return true;
}
bool EnumType::call_dtor(void* ptr) const
{
    return true;
}
bool EnumType::call_copy(void* dst, const void* src) const
{
    return _underlying_type->call_copy(dst, src);
}
bool EnumType::call_move(void* dst, void* src) const
{
    return _underlying_type->call_move(dst, src);
}
bool EnumType::call_assign(void* dst, const void* src) const
{
    return _underlying_type->call_assign(dst, src);
}
bool EnumType::call_move_assign(void* dst, void* src) const
{
    return _underlying_type->call_move_assign(dst, src);
}
bool EnumType::call_hash(const void* ptr, size_t& result) const
{
    return _underlying_type->call_hash(ptr, result);
}
} // namespace skr::rttr