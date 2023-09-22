#include "SkrRT/rttr/type/pointer_type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
PointerType::PointerType(Type* target_type, string name)
    : GenericType(kPointerGenericGUID, std::move(name), GUID::Create(), sizeof(void*), alignof(void*))
{
}

bool PointerType::call_ctor(void* ptr) const { return true; }
bool PointerType::call_dtor(void* ptr) const { return true; }
bool PointerType::call_copy(void* dst, const void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void* const*>(src);
    return true;
}
bool PointerType::call_move(void* dst, void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void**>(src);
    return true;
}
bool PointerType::call_assign(void* dst, const void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void* const*>(src);
    return true;
}
bool PointerType::call_move_assign(void* dst, void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void**>(src);
    return true;
}
bool PointerType::call_hash(const void* ptr, size_t& result) const
{
    result = reinterpret_cast<size_t>(*reinterpret_cast<void* const*>(ptr));
    return true;
}
} // namespace skr::rttr