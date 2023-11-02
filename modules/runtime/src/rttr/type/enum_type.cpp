#include "SkrRT/rttr/type/enum_type.hpp"

namespace skr::rttr
{
EnumType::EnumType(Type* underlying_type, GUID type_id, string name)
    : Type(ETypeCategory::SKR_TYPE_CATEGORY_ENUM, std::move(name), type_id, underlying_type->size(), underlying_type->alignment())
    , _underlying_type(underlying_type)
{
}

void EnumType::call_ctor(void* ptr) const
{
}
void EnumType::call_dtor(void* ptr) const
{
}
void EnumType::call_copy(void* dst, const void* src) const
{
    _underlying_type->call_copy(dst, src);
}
void EnumType::call_move(void* dst, void* src) const
{
    _underlying_type->call_move(dst, src);
}
void EnumType::call_assign(void* dst, const void* src) const
{
    _underlying_type->call_assign(dst, src);
}
void EnumType::call_move_assign(void* dst, void* src) const
{
    _underlying_type->call_move_assign(dst, src);
}
size_t EnumType::call_hash(const void* ptr) const
{
    return _underlying_type->call_hash(ptr);
}
} // namespace skr::rttr