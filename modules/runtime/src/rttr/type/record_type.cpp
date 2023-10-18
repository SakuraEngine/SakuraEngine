#include "SkrRT/rttr/type/record_type.hpp"
#include "SkrRT/misc/log.hpp"

namespace skr::rttr
{
RecordType::RecordType(string name, GUID type_id, size_t size, size_t alignment, RecordBasicMethodTable basic_methods)
    : Type(ETypeCategory::SKR_TYPE_CATEGORY_RECORD, std::move(name), type_id, size, alignment)
    , _basic_methods(basic_methods)
{
}

bool RecordType::call_ctor(void* ptr) const
{
    if (_basic_methods.ctor)
    {
        _basic_methods.ctor(ptr);
        return true;
    }
    return false;
}
bool RecordType::call_dtor(void* ptr) const
{
    if (_basic_methods.dtor)
    {
        _basic_methods.dtor(ptr);
        return true;
    }
    return false;
}
bool RecordType::call_copy(void* dst, const void* src) const
{
    if (_basic_methods.copy)
    {
        _basic_methods.copy(dst, src);
        return true;
    }
    return false;
}
bool RecordType::call_move(void* dst, void* src) const
{
    if (_basic_methods.move)
    {
        _basic_methods.move(dst, src);
        return true;
    }
    return false;
}
bool RecordType::call_assign(void* dst, const void* src) const
{
    if (_basic_methods.assign)
    {
        _basic_methods.assign(dst, src);
        return true;
    }
    return false;
}
bool RecordType::call_move_assign(void* dst, void* src) const
{
    if (_basic_methods.move_assign)
    {
        _basic_methods.move_assign(dst, src);
        return true;
    }
    return false;
}
bool RecordType::call_hash(const void* ptr, size_t& result) const
{
    if (_basic_methods.hash)
    {
        _basic_methods.hash(ptr, result);
        return true;
    }
    return false;
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
        return find_result->value.cast_func(p_self);
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
        SKR_LOG_ERROR(u8"RecordType::write_binary: type {} has no write_binary method", name().c_str());
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
        SKR_LOG_ERROR(u8"RecordType::read_binary: type {} has no read_binary method", name().c_str());
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
        SKR_LOG_ERROR(u8"RecordType::write_json: type {} has no write_json method", name().c_str());
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
        SKR_LOG_ERROR(u8"RecordType::read_json: type {} has no read_json method", name().c_str());
    }
    return skr::json::error_code::SUCCESS;
}

} // namespace skr::rttr
