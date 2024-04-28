#include "SkrRTTR/type/record_type.hpp"
#include "SkrCore/log.hpp"

namespace skr::rttr
{
RecordType::RecordType(skr::String name, GUID type_id, size_t size, size_t alignment, RecordBasicMethodTable basic_methods)
    : Type(ETypeCategory::SKR_TYPE_CATEGORY_RECORD, std::move(name), type_id, size, alignment)
    , _basic_methods(basic_methods)
{
}

bool RecordType::query_feature(ETypeFeature feature) const
{
    switch (feature)
    {
        case ETypeFeature::Constructor:
            return _basic_methods.ctor != nullptr;
        case ETypeFeature::Destructor:
            return _basic_methods.dtor != nullptr;
        case ETypeFeature::Copy:
            return _basic_methods.copy != nullptr;
        case ETypeFeature::Move:
            return _basic_methods.move != nullptr;
        case ETypeFeature::Assign:
            return _basic_methods.assign != nullptr;
        case ETypeFeature::MoveAssign:
            return _basic_methods.move_assign != nullptr;
        case ETypeFeature::Hash:
            return _basic_methods.hash != nullptr;
        case ETypeFeature::WriteBinary:
            return _basic_methods.write_binary != nullptr;
        case ETypeFeature::ReadBinary:
            return _basic_methods.read_binary != nullptr;
        case ETypeFeature::WriteJson:
            return _basic_methods.write_json != nullptr;
        case ETypeFeature::ReadJson:
            return _basic_methods.read_json != nullptr;
    }
    return false;
}

void RecordType::call_ctor(void* ptr) const
{
    if (_basic_methods.ctor)
    {
        _basic_methods.ctor(ptr);
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no ctor method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
void RecordType::call_dtor(void* ptr) const
{
    if (_basic_methods.dtor)
    {
        _basic_methods.dtor(ptr);
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no dtor method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
void RecordType::call_copy(void* dst, const void* src) const
{
    if (_basic_methods.copy)
    {
        _basic_methods.copy(dst, src);
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no copy method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
void RecordType::call_move(void* dst, void* src) const
{
    if (_basic_methods.move)
    {
        _basic_methods.move(dst, src);
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no move method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
void RecordType::call_assign(void* dst, const void* src) const
{
    if (_basic_methods.assign)
    {
        _basic_methods.assign(dst, src);
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no assign method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
void RecordType::call_move_assign(void* dst, void* src) const
{
    if (_basic_methods.move_assign)
    {
        _basic_methods.move_assign(dst, src);
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no move_assign method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
size_t RecordType::call_hash(const void* ptr) const
{
    if (_basic_methods.hash)
    {
        return _basic_methods.hash(ptr);
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no hash method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
        return 0;
    }
}

// find base
void* RecordType::cast_to(const Type* target_type, void* p_self) const
{
    if (target_type == this)
    {
        return p_self;
    }
    else if (auto find_result = _base_types_map.find(target_type->type_id()))
    {
        return find_result.value().cast_func(p_self);
    }
    else
    {
        for (const auto& pair : _base_types_map)
        {
            const Type* type = pair.value.type;
            if (type->type_category() == ETypeCategory::SKR_TYPE_CATEGORY_RECORD)
            {
                if (auto cast_p = static_cast<const RecordType*>(type)->cast_to(target_type, pair.value.cast_func(p_self)))
                {
                    return cast_p;
                }
            }
            else
            {
                SKR_UNREACHABLE_CODE()
            }
        }
    }
    return nullptr;
}

int RecordType::write_binary(const void* dst, skr_binary_writer_t* writer) const
{
    if (_basic_methods.write_binary)
    {
        return _basic_methods.write_binary(dst, writer);
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no write_binary method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
    return 0;
}
int RecordType::read_binary(void* dst, skr_binary_reader_t* reader) const
{
    if (_basic_methods.read_binary)
    {
        return _basic_methods.read_binary(dst, reader);
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no read_binary method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
    return 0;
}
void RecordType::write_json(const void* dst, skr_json_writer_t* writer) const
{
    if (_basic_methods.write_json)
    {
        _basic_methods.write_json(dst, writer);
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no write_json method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
}
skr::json::error_code RecordType::read_json(void* dst, skr::json::value_t&& reader) const
{
    if (_basic_methods.read_json)
    {
        return _basic_methods.read_json(dst, std::move(reader));
    }
    else
    {
        SKR_LOG_ERROR(u8"[RTTR] type %s has no read_json method, before call this function, please check the type feature by query_feature().", name().c_str());
        SKR_UNREACHABLE_CODE();
    }
    return skr::json::error_code::SUCCESS;
}

// setup
void RecordType::set_base_types(Map<GUID, BaseInfo> base_types)
{
    // validate
    for (const auto& data : base_types)
    {
        if (data.value.type == nullptr)
        {
            skr::String guid_str = skr::format(u8"{}", data.key);
            SKR_LOG_ERROR(u8"[RTTR] type %s has a null base type.\n GUID: {%s}", name().c_str(), guid_str.c_str());
        }
    }

    _base_types_map = std::move(base_types);
}
void RecordType::set_fields(MultiMap<skr::String, Field> fields)
{
    // validate
    for (const auto& data : fields)
    {
        if (data.value.type == nullptr)
        {
            skr::String guid_str = skr::format(u8"{}", data.key);
            SKR_LOG_ERROR(u8"[RTTR] type %s has a null field type.\n GUID: {%s}", name().c_str(), guid_str.c_str());
        }
    }

    _fields_map = std::move(fields);
}
void RecordType::set_methods(MultiMap<skr::String, Method> methods)
{
    _methods_map = std::move(methods);
}

} // namespace skr::rttr
