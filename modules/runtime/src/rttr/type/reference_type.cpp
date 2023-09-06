#include "SkrRT/rttr/type/reference_type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
ReferenceType::ReferenceType(Type* target_type)
    : GenericType(kReferenceGenericGUID, GUID::Create(), sizeof(size_t&), alignof(size_t&))
{
}

bool ReferenceType::call_ctor(void* ptr) const { return true; }
bool ReferenceType::call_dtor(void* ptr) const { return true; }
bool ReferenceType::call_copy(void* dst, const void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void* const*>(src);
    return true;
}
bool ReferenceType::call_move(void* dst, void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void**>(src);
    return true;
}
bool ReferenceType::call_assign(void* dst, const void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void* const*>(src);
    return true;
}
bool ReferenceType::call_move_assign(void* dst, void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void**>(src);
    return true;
}
bool ReferenceType::call_hash(const void* ptr, size_t& result) const
{
    result = reinterpret_cast<size_t>(*reinterpret_cast<void* const*>(ptr));
    return true;
}
} // namespace skr::rttr