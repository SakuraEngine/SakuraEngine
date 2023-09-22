#include "SkrRT/rttr/type/record_type.hpp"

namespace skr::rttr
{
RecordType::RecordType(string name, GUID type_id, size_t size, size_t alignment, RecordBasicMethodTable basic_methods, Span<Type*> base_types, Span<FieldInfo> fields, Span<MethodInfo> methods)
    : Type(ETypeCategory::SKR_TYPE_CATEGORY_RECORD, std::move(name), type_id, size, alignment)
    , _basic_methods(basic_methods)
{
    _base_types_map.reserve(base_types.size());
    _fields_map.reserve(fields.size());
    _methods_map.reserve(methods.size());

    for (Type* base_type : base_types)
    {
        _base_types_map.add(base_type->type_id(), base_type);
    }

    for (const FieldInfo& field : fields)
    {
        _fields_map.add(field.name, { field.name, field.type, field.offset });
    }

    for (const MethodInfo& method : methods)
    {
        auto ref = _methods_map.add_default(method.name);

        ref->value.name        = method.name;
        ref->value.return_type = method.return_type;
        ref->value.executable  = method.executable;
        ref->value.parameters_type.reserve(method.parameters_type.size());
        for (Type* parameter_type : method.parameters_type)
        {
            ref->value.parameters_type.add(parameter_type);
        }
    }
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
