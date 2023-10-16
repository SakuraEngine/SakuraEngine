#include "SkrRT/rttr/type/record_type.hpp"

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

} // namespace skr::rttr
