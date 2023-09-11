#include "SkrRT/rttr/type/record_type.hpp"

namespace skr::rttr
{
RecordType::RecordType(GUID type_id, size_t size, size_t alignment, Span<Type*> base_types, Span<FieldInfo> fields, Span<MethodInfo> methods)
    : Type(ETypeCategory::SKR_TYPE_CATEGORY_RECORD, type_id, size, alignment)
{
    _base_types_map.reserve(base_types.size());
    _fields.reserve(fields.size());
    _methods.reserve(methods.size());

    for (Type* base_type : base_types)
    {
        _base_types_map.add(base_type->type_id(), base_type);
    }

    for (const FieldInfo& field : fields)
    {
        _fields.add(field.name, { field.name, field.type, field.offset });
    }

    for (const MethodInfo& method : methods)
    {
        auto ref = _methods.add_default(method.name);

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

} // namespace skr::rttr
