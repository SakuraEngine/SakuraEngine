#include "SkrRTTR/type/pointer_type.hpp"
#include "SkrRTTR/rttr_traits.hpp"

namespace skr::rttr
{
PointerType::PointerType(Type* target_type, skr::String name)
    : GenericType(kPointerGenericGUID, std::move(name), GUID::Create(), sizeof(void*), alignof(void*))
{
}

bool PointerType::query_feature(ETypeFeature feature) const
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
            return _target_type->query_feature(feature);
    }
}

void PointerType::call_ctor(void* ptr) const
{
}
void PointerType::call_dtor(void* ptr) const
{
}
void PointerType::call_copy(void* dst, const void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void* const*>(src);
}
void PointerType::call_move(void* dst, void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void**>(src);
}
void PointerType::call_assign(void* dst, const void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void* const*>(src);
}
void PointerType::call_move_assign(void* dst, void* src) const
{
    *reinterpret_cast<void**>(dst) = *reinterpret_cast<void**>(src);
}
size_t PointerType::call_hash(const void* ptr) const
{
    return reinterpret_cast<size_t>(*reinterpret_cast<void* const*>(ptr));
}

// TODO. 考虑多态序列化，等多态序列化的方式敲定之后再实现
int PointerType::write_binary(const void* dst, skr_binary_writer_t* writer) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return 0;
}
int PointerType::read_binary(void* dst, skr_binary_reader_t* reader) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return 0;
}
void PointerType::write_json(const void* dst, skr_json_writer_t* writer) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
skr::json::error_code PointerType::read_json(void* dst, skr::json::value_t&& reader) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return skr::json::error_code::SUCCESS;
}
} // namespace skr::rttr