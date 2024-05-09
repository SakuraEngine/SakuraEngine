#include "SkrRTTR/type.hpp"
#include "SkrCore/log.hpp"

namespace skr::rttr
{
Type::Type(ETypeCategory type_category, skr::String name, GUID type_id, size_t size, size_t alignment)
    : _type_category(type_category)
    , _name(std::move(name))
    , _type_id(type_id)
    , _size(size)
    , _alignment(alignment)
{
}
} // namespace skr::rttr

namespace skr::rttr
{
RecordType::RecordType(skr::String name, GUID type_id, size_t size, size_t alignment)
    : Type(ETypeCategory::SKR_TYPE_CATEGORY_RECORD, std::move(name), type_id, size, alignment)
{
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

namespace skr::rttr
{
EnumType::EnumType(Type* underlying_type, GUID type_id, skr::String name)
    : Type(ETypeCategory::SKR_TYPE_CATEGORY_ENUM, std::move(name), type_id, underlying_type->size(), underlying_type->alignment())
    , _underlying_type(underlying_type)
{
}

} // namespace skr::rttr