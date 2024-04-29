#include "SkrCore/log.h"
#include "SkrRTTR/type/reference_type.hpp"
#include "SkrRTTR/rttr_traits.hpp"

namespace skr::rttr
{
ReferenceType::ReferenceType(Type* target_type, skr::String name)
    : GenericType({}, std::move(name), GUID::Create(), sizeof(size_t&), alignof(size_t&))
{
}

bool ReferenceType::query_feature(ETypeFeature feature) const
{
    switch (feature)
    {
        case ETypeFeature::Constructor:
        case ETypeFeature::Destructor:
        case ETypeFeature::Copy:
        case ETypeFeature::Move:
        case ETypeFeature::Assign:
        case ETypeFeature::MoveAssign:
        case ETypeFeature::Hash:
            return true;
        case ETypeFeature::WriteBinary:
        case ETypeFeature::ReadBinary:
        case ETypeFeature::WriteJson:
        case ETypeFeature::ReadJson:
            return false;
    }
    return false;
}

void ReferenceType::call_ctor(void* ptr) const
{
}
void ReferenceType::call_dtor(void* ptr) const
{
}
void ReferenceType::call_copy(void* dst, const void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void* const*>(src);
}
void ReferenceType::call_move(void* dst, void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void**>(src);
}
void ReferenceType::call_assign(void* dst, const void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void* const*>(src);
}
void ReferenceType::call_move_assign(void* dst, void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void**>(src);
}
size_t ReferenceType::call_hash(const void* ptr) const
{
    return reinterpret_cast<size_t>(*reinterpret_cast<void* const*>(ptr));
}

int ReferenceType::write_binary(const void* dst, skr_binary_writer_t* writer) const
{
    SKR_LOG_ERROR(u8"[RTTR] Reference type has no binary writer, before call this function, please check the type feature by query_feature().");
    SKR_UNREACHABLE_CODE();
    return 0;
}
int ReferenceType::read_binary(void* dst, skr_binary_reader_t* reader) const
{
    SKR_LOG_ERROR(u8"[RTTR] Reference type has no binary reader, before call this function, please check the type feature by query_feature().");
    SKR_UNREACHABLE_CODE();
    return 0;
}
void ReferenceType::write_json(const void* dst, skr_json_writer_t* writer) const
{
    SKR_LOG_ERROR(u8"[RTTR] Reference type has no json writer, before call this function, please check the type feature by query_feature().");
    SKR_UNREACHABLE_CODE();
}
skr::json::error_code ReferenceType::read_json(void* dst, skr::json::value_t&& reader) const
{
    SKR_LOG_ERROR(u8"[RTTR] Reference type has no json reader, before call this function, please check the type feature by query_feature().");
    SKR_UNREACHABLE_CODE();
    return skr::json::error_code::SUCCESS;
}
} // namespace skr::rttr