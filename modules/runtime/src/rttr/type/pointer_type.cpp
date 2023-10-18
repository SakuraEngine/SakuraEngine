#include "SkrRT/rttr/type/pointer_type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
PointerType::PointerType(Type* target_type, string name)
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