#include "SkrRT/rttr/type/reference_type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
ReferenceType::ReferenceType(Type* target_type, string name)
    : GenericType(kReferenceGenericGUID, std::move(name), GUID::Create(), sizeof(size_t&), alignof(size_t&))
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

int ReferenceType::write_binary(const void* dst, skr_binary_writer_t* writer) const
{
    SKR_UNREACHABLE_CODE();
    return 0;
}
int ReferenceType::read_binary(void* dst, skr_binary_reader_t* reader) const
{
    SKR_UNREACHABLE_CODE();
    return 0;
}
void ReferenceType::write_json(const void* dst, skr_json_writer_t* writer) const
{
    SKR_UNREACHABLE_CODE();
}
skr::json::error_code ReferenceType::read_json(void* dst, skr::json::value_t&& reader) const
{
    SKR_UNREACHABLE_CODE();
    return skr::json::error_code::SUCCESS;
}
} // namespace skr::rttr