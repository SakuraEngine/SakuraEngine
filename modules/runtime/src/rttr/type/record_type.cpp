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

} // namespace skr::rttr
