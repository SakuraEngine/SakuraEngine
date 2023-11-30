#include "SkrRT/misc/log.h"
#include "SkrRT/rttr/type/vector_type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"
#include "SkrRT/serde/json/writer.h"
#include "SkrRT/serde/json/reader.h"

namespace skr::rttr
{
inline static size_t _element_count_of(span<size_t> dimensions)
{
    size_t size = 1;
    for (auto d : dimensions)
    {
        size *= d;
    }
    return size;
}

VectorType::VectorType(Type* target_type, span<size_t> dimensions, string name)
    : GenericType(kArrayGenericGUID, std::move(name), GUID::Create(), target_type->size() * _element_count_of(dimensions), target_type->alignment())
    , _size(_element_count_of(dimensions))
    , _dimensions(dimensions.data(), dimensions.size())
{
}

bool VectorType::query_feature(ETypeFeature feature) const
{
    switch (feature)
    {
        case ETypeFeature::Hash:
            return false;
        default:
            return _target_type->query_feature(feature);
    }
}

void VectorType::call_ctor(void* ptr) const
{
    if (_target_type->query_feature(ETypeFeature::Constructor))
    {
        for (size_t i = 0; i < _size; ++i)
        {
            _target_type->call_ctor(static_cast<char*>(ptr) + i * _target_type->size());
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no ctor method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
void VectorType::call_dtor(void* ptr) const
{
    if (_target_type->query_feature(ETypeFeature::Destructor))
    {
        for (size_t i = 0; i < _size; ++i)
        {
            _target_type->call_dtor(static_cast<char*>(ptr) + i * _target_type->size());
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no dtor method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
void VectorType::call_copy(void* dst, const void* src) const
{
    if (_target_type->query_feature(ETypeFeature::Copy))
    {
        for (size_t i = 0; i < _size; ++i)
        {
            _target_type->call_copy(static_cast<char*>(dst) + i * _target_type->size(), static_cast<char const*>(src) + i * _target_type->size());
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no copy method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
void VectorType::call_move(void* dst, void* src) const
{
    if (_target_type->query_feature(ETypeFeature::Move))
    {
        for (size_t i = 0; i < _size; ++i)
        {
            _target_type->call_move(static_cast<char*>(dst) + i * _target_type->size(), static_cast<char*>(src) + i * _target_type->size());
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no move method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
void VectorType::call_assign(void* dst, const void* src) const
{
    if (_target_type->query_feature(ETypeFeature::Assign))
    {
        for (size_t i = 0; i < _size; ++i)
        {
            _target_type->call_assign(static_cast<char*>(dst) + i * _target_type->size(), static_cast<char const*>(src) + i * _target_type->size());
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no assign method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
void VectorType::call_move_assign(void* dst, void* src) const
{
    if (_target_type->query_feature(ETypeFeature::MoveAssign))
    {
        for (size_t i = 0; i < _size; ++i)
        {
            _target_type->call_move_assign(static_cast<char*>(dst) + i * _target_type->size(), static_cast<char*>(src) + i * _target_type->size());
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no move_assign method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
size_t VectorType::call_hash(const void* ptr) const
{
    SKR_LOG_ERROR(u8"[RTTR] cpp array has no hash method, before call this function, please check the type feature by query_feature().");
    SKR_UNREACHABLE_CODE();
    return 0;
}

int VectorType::write_binary(const void* dst, skr_binary_writer_t* writer) const
{
    if (_target_type->query_feature(ETypeFeature::WriteBinary))
    {
        auto p_data = static_cast<const uint8_t*>(dst);
        auto stride = _target_type->size();
        for (int i = 0; i < _size; ++i)
        {
            if (auto result = _target_type->write_binary(p_data, writer))
            {
                return result;
            }
            p_data += stride;
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no write_binary method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
    return 0;
}
int VectorType::read_binary(void* dst, skr_binary_reader_t* reader) const
{
    if (_target_type->query_feature(ETypeFeature::ReadBinary))
    {
        auto p_data = static_cast<uint8_t*>(dst);
        auto stride = _target_type->size();
        for (int i = 0; i < _size; ++i)
        {
            if (auto result = _target_type->read_binary(p_data, reader))
            {
                return result;
            }
            p_data += stride;
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no read_binary method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
    return 0;
}
void VectorType::write_json(const void* dst, skr_json_writer_t* writer) const
{
    if (_target_type->query_feature(ETypeFeature::WriteJson))
    {
        auto p_data = static_cast<const uint8_t*>(dst);
        auto stride = _target_type->size();
        for (int i = 0; i < _size; ++i)
        {
            _target_type->write_json(p_data, writer);
            p_data += stride;
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no write_json method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
skr::json::error_code VectorType::read_json(void* dst, skr::json::value_t&& reader) const
{
    if (_target_type->query_feature(ETypeFeature::ReadJson))
    {
        auto p_data = static_cast<uint8_t*>(dst);
        auto stride = _target_type->size();
        for (int i = 0; i < _size; ++i)
        {
            if (auto result = _target_type->read_json(p_data, std::move(reader)))
            {
                return result;
            }
            p_data += stride;
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no read_json method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
    return skr::json::error_code::SUCCESS;
}
} // namespace skr::rttr
