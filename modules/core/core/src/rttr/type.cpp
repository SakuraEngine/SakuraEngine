#include "SkrRTTR/type.hpp"
#include "SkrCore/log.hpp"
#include "SkrRTTR/export/extern_methods.hpp"

namespace skr
{
// ctor & dtor
RTTRType::RTTRType()
{
}
RTTRType::~RTTRType()
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Invalid:
        break;
    case ERTTRTypeCategory::Primitive:
        _primitive_data.~RTTRPrimitiveData();
        break;
    case ERTTRTypeCategory::Record:
        _record_data.~RTTRRecordData();
        break;
    case ERTTRTypeCategory::Enum:
        _enum_data.~RTTREnumData();
        break;
    default:
        SKR_UNREACHABLE_CODE()
        break;
    }
}

// builders
void RTTRType::build_primitive(FunctionRef<void(RTTRPrimitiveData* data)> func)
{
    // init
    if (_type_category == ERTTRTypeCategory::Invalid)
    {
        new (&_primitive_data) RTTRPrimitiveData();
        _type_category = ERTTRTypeCategory::Primitive;
    }

    // validate
    SKR_ASSERT(_type_category == ERTTRTypeCategory::Primitive && "Type category mismatch when build primitive data");

    // build
    func(&_primitive_data);
}
void RTTRType::build_record(FunctionRef<void(RTTRRecordData* data)> func)
{
    // init
    if (_type_category == ERTTRTypeCategory::Invalid)
    {
        new (&_record_data) RTTRRecordData();
        _type_category = ERTTRTypeCategory::Record;
    }

    // validate
    SKR_ASSERT(_type_category == ERTTRTypeCategory::Record && "Type category mismatch when build record data");

    // build
    func(&_record_data);
}
void RTTRType::build_enum(FunctionRef<void(RTTREnumData* data)> func)
{
    // init
    if (_type_category == ERTTRTypeCategory::Invalid)
    {
        new (&_enum_data) RTTREnumData();
        _type_category = ERTTRTypeCategory::Enum;
    }

    // validate
    SKR_ASSERT(_type_category == ERTTRTypeCategory::Enum && "Type category mismatch when build enum data");

    // build
    func(&_enum_data);
}

// caster
bool RTTRType::based_on(GUID type_id, uint32_t* out_cast_count) const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Primitive: {
        return (type_id == _primitive_data.type_id);
    }
    case ERTTRTypeCategory::Record: {
        if (type_id == _record_data.type_id)
        {
            return true;
        }
        else
        {
            // add count if walk to base
            if (out_cast_count)
            {
                ++(*out_cast_count);
            }
            // find base and cast
            for (const auto& base : _record_data.bases_data)
            {
                if (type_id == base->type_id)
                {
                    return true;
                }
            }

            // get base type and continue cast
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    if (type->based_on(type_id, out_cast_count))
                    {
                        return true;
                    }
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing cast from \"{}\"", type_id, _record_data.type_id);
                }
            }

            return false;
        }
    }
    case ERTTRTypeCategory::Enum: {
        return (type_id == _enum_data.type_id || type_id == _enum_data.underlying_type_id);
    }
    default:
        SKR_UNREACHABLE_CODE()
        return false;
    }
}
void* RTTRType::cast_to_base(GUID type_id, void* p) const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Primitive: {
        return (type_id == _primitive_data.type_id) ? p : nullptr;
    }
    case ERTTRTypeCategory::Record: {
        if (type_id == _record_data.type_id)
        {
            return p;
        }
        else
        {
            // find base and cast
            for (const auto& base : _record_data.bases_data)
            {
                if (type_id == base->type_id)
                {
                    return base->cast_to_base(p);
                }
            }

            // get base type and continue cast
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    auto casted = type->cast_to_base(type_id, p);
                    if (casted)
                    {
                        return casted;
                    }
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing cast from \"{}\"", type_id, _record_data.type_id);
                }
            }
            return nullptr;
        }
    }
    case ERTTRTypeCategory::Enum: {
        return (type_id == _enum_data.type_id || type_id == _enum_data.underlying_type_id) ? p : nullptr;
    }
    default:
        SKR_UNREACHABLE_CODE()
        return nullptr;
    }
}
RTTRTypeCaster RTTRType::caster_to_base(GUID type_id) const
{
    RTTRTypeCaster result;
    _build_caster(result, type_id);
    return result;
}

// enum getter
GUID RTTRType::enum_underlying_type_id() const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Enum:
        return _enum_data.underlying_type_id;
    default:
        SKR_UNREACHABLE_CODE()
        return {};
    }
}
void RTTRType::each_enum_items(FunctionRef<void(const RTTREnumItemData*)> each_func) const
{
    if (_type_category == ERTTRTypeCategory::Enum)
    {
        for (const auto& item : _enum_data.items)
        {
            each_func(item);
        }
    }
}

// get dtor
Optional<RTTRDtorData> RTTRType::dtor_data() const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Record:
        return _record_data.dtor_data;
    default:
        return {};
    }
}
DtorInvoker RTTRType::dtor_invoker() const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Record:
        return _record_data.dtor_data.native_invoke;
    default:
        return nullptr;
    }
}
void RTTRType::invoke_dtor(void* p) const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Record:
        _record_data.dtor_data.native_invoke(p);
        break;
    default:
        break;
    }
}

// each method & field
void RTTRType::each_bases(FunctionRef<void(const RTTRBaseData* base_data, const RTTRType* owner)> each_func, RTTRTypeEachConfig config) const
{
    if (_type_category == ERTTRTypeCategory::Record)
    {
        // each self
        for (const auto& base : _record_data.bases_data)
        {
            each_func(base, this);
        }

        // each base
        if (config.include_bases)
        {
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    type->each_bases(each_func, config);
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing each base from \"{}\"", base->type_id, _record_data.type_id);
                }
            }
        }
    }
}
void RTTRType::each_ctor(FunctionRef<void(const RTTRCtorData* ctor)> each_func, RTTRTypeEachConfig config) const
{
    if (_type_category == ERTTRTypeCategory::Record)
    {
        // each self
        for (const auto& ctor : _record_data.ctor_data)
        {
            each_func(ctor);
        }
    }
}
void RTTRType::each_method(FunctionRef<void(const RTTRMethodData* method, const RTTRType* owner)> each_func, RTTRTypeEachConfig config) const
{
    if (_type_category == ERTTRTypeCategory::Record)
    {
        // each self
        for (const auto& method : _record_data.methods)
        {
            each_func(method, this);
        }

        // each base
        if (config.include_bases)
        {
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    type->each_method(each_func, config);
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing each method from \"{}\"", base->type_id, _record_data.type_id);
                }
            }
        }
    }
}
void RTTRType::each_field(FunctionRef<void(const RTTRFieldData* field, const RTTRType* owner)> each_func, RTTRTypeEachConfig config) const
{
    if (_type_category == ERTTRTypeCategory::Record)
    {
        // each self
        for (const auto& field : _record_data.fields)
        {
            each_func(field, this);
        }

        // each base
        if (config.include_bases)
        {
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    type->each_field(each_func, config);
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing each method from \"{}\"", base->type_id, _record_data.type_id);
                }
            }
        }
    }
}
void RTTRType::each_static_method(FunctionRef<void(const RTTRStaticMethodData* method, const RTTRType* owner)> each_func, RTTRTypeEachConfig config) const
{
    if (_type_category == ERTTRTypeCategory::Record)
    {
        // each self
        for (const auto& method : _record_data.static_methods)
        {
            each_func(method, this);
        }

        // each base
        if (config.include_bases)
        {
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    type->each_static_method(each_func, config);
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing each method from \"{}\"", base->type_id, _record_data.type_id);
                }
            }
        }
    }
}
void RTTRType::each_static_field(FunctionRef<void(const RTTRStaticFieldData* field, const RTTRType* owner)> each_func, RTTRTypeEachConfig config) const
{
    if (_type_category == ERTTRTypeCategory::Record)
    {
        // each self
        for (const auto& field : _record_data.static_fields)
        {
            each_func(field, this);
        }

        // each base
        if (config.include_bases)
        {
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    type->each_static_field(each_func, config);
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing each method from \"{}\"", base->type_id, _record_data.type_id);
                }
            }
        }
    }
}
void RTTRType::each_extern_method(FunctionRef<void(const RTTRExternMethodData* method, const RTTRType* owner)> each_func, RTTRTypeEachConfig config) const
{
    if (_type_category == ERTTRTypeCategory::Record)
    {
        // each self
        for (const auto& method : _record_data.extern_methods)
        {
            each_func(method, this);
        }

        // each base
        if (config.include_bases)
        {
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    type->each_extern_method(each_func, config);
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing each method from \"{}\"", base->type_id, _record_data.type_id);
                }
            }
        }
    }
    else if (_type_category == ERTTRTypeCategory::Primitive)
    {
        // each self
        for (const auto& method : _primitive_data.extern_methods)
        {
            each_func(method, this);
        }
    }
}

// find method & field
const RTTRCtorData* RTTRType::find_ctor(RTTRTypeFindConfig config) const
{
    if (_type_category == ERTTRTypeCategory::Record)
    {
        auto find_func = [&](const RTTRCtorData* ctor) {
            // filter by signature
            if (config.signature && !ctor->signature_equal(config.signature.value(), config.signature_compare_flag))
            {
                return false;
            }

            return true;
        };

        // find self
        auto result = _record_data.ctor_data.find_if(find_func);
        if (result)
        {
            return result.ref();
        }
    }
    return nullptr;
}
const RTTRMethodData* RTTRType::find_method(RTTRTypeFindConfig config) const
{
    if (_type_category == ERTTRTypeCategory::Record)
    {
        auto find_func = [&](const RTTRMethodData* method) {
            // filter by name
            if (config.name && method->name != config.name.value())
            {
                return false;
            }

            // filter by signature
            if (config.signature && !method->signature_equal(config.signature.value(), config.signature_compare_flag))
            {
                return false;
            }

            return true;
        };

        // find self
        auto result = _record_data.methods.find_if(find_func);
        if (result)
        {
            return result.ref();
        }

        // find base
        if (config.include_bases)
        {
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    auto method = type->find_method(config);
                    if (method)
                    {
                        return method;
                    }
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing find method from \"{}\"", base->type_id, _record_data.type_id);
                }
            }
        }
    }
    return nullptr;
}
const RTTRFieldData* RTTRType::find_field(RTTRTypeFindConfig config) const
{
    if (_type_category == ERTTRTypeCategory::Record)
    {
        auto find_func = [&](const RTTRFieldData* field) {
            // filter by name
            if (config.name && field->name != config.name.value())
            {
                return false;
            }

            // filter by signature
            if (config.signature && !field->type.view().equal(config.signature.value(), config.signature_compare_flag))
            {
                return false;
            }

            return true;
        };

        // find self
        auto result = _record_data.fields.find_if(find_func);
        if (result)
        {
            return result.ref();
        }

        // find base
        if (config.include_bases)
        {
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    auto field = type->find_field(config);
                    if (field)
                    {
                        return field;
                    }
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing find field from \"{}\"", base->type_id, _record_data.type_id);
                }
            }
        }
    }
    return nullptr;
}
const RTTRStaticMethodData* RTTRType::find_static_method(RTTRTypeFindConfig config) const
{
    if (_type_category == ERTTRTypeCategory::Record)
    {
        auto find_func = [&](const RTTRStaticMethodData* method) {
            // filter by name
            if (config.name && method->name != config.name.value())
            {
                return false;
            }

            // filter by signature
            if (config.signature && !method->signature_equal(config.signature.value(), config.signature_compare_flag))
            {
                return false;
            }

            return true;
        };

        // find self
        auto result = _record_data.static_methods.find_if(find_func);
        if (result)
        {
            return result.ref();
        }

        // find base
        if (config.include_bases)
        {
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    auto method = type->find_static_method(config);
                    if (method)
                    {
                        return method;
                    }
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing find static method from \"{}\"", base->type_id, _record_data.type_id);
                }
            }
        }
    }
    return nullptr;
}
const RTTRStaticFieldData* RTTRType::find_static_field(RTTRTypeFindConfig config) const
{
    if (_type_category == ERTTRTypeCategory::Record)
    {
        auto find_func = [&](const RTTRStaticFieldData* field) {
            // filter by name
            if (config.name && field->name != config.name.value())
            {
                return false;
            }

            // filter by signature
            if (config.signature && !field->type.view().equal(config.signature.value(), config.signature_compare_flag))
            {
                return false;
            }

            return true;
        };

        // find self
        auto result = _record_data.static_fields.find_if(find_func);
        if (result)
        {
            return result.ref();
        }

        // find base
        if (config.include_bases)
        {
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    auto field = type->find_static_field(config);
                    if (field)
                    {
                        return field;
                    }
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing find static field from \"{}\"", base->type_id, _record_data.type_id);
                }
            }
        }
    }
    return nullptr;
}
const RTTRExternMethodData* RTTRType::find_extern_method(RTTRTypeFindConfig config) const
{
    auto find_func = [&](const RTTRExternMethodData* method) {
        // filter by name
        if (config.name && method->name != config.name.value())
        {
            return false;
        }

        // filter by signature
        if (config.signature && !method->signature_equal(config.signature.value(), config.signature_compare_flag))
        {
            return false;
        }

        return true;
    };

    if (_type_category == ERTTRTypeCategory::Record)
    {
        // find self
        auto result = _record_data.extern_methods.find_if(find_func);
        if (result)
        {
            return result.ref();
        }

        // find base
        if (config.include_bases)
        {
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    auto method = type->find_extern_method(config);
                    if (method)
                    {
                        return method;
                    }
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing find extern method from \"{}\"", base->type_id, _record_data.type_id);
                }
            }
        }
    }
    else if (_type_category == ERTTRTypeCategory::Primitive)
    {
        // find self
        auto result = _primitive_data.extern_methods.find_if(find_func);
        if (result)
        {
            return result.ref();
        }
    }
    else if (_type_category == ERTTRTypeCategory::Record)
    {
        // find self
        auto result = _enum_data.extern_methods.find_if(find_func);
        if (result)
        {
            return result.ref();
        }
    }
    return nullptr;
}

// find basic functions
RTTRInvokerDefaultCtor RTTRType::find_default_ctor() const
{
    return find_ctor_t<void()>();
}
RTTRInvokerCopyCtor RTTRType::find_copy_ctor() const
{
    TypeSignatureBuilder tb;
    tb.write_function_signature(1);
    tb.write_type_id(type_id_of<void>()); // return
    tb.write_const_ref();
    tb.write_type_id(type_id()); // param 1: const T&
    return find_ctor({ .signature = tb.type_signature_view() });
}
RTTRInvokerMoveCtor RTTRType::find_move_ctor() const
{
    TypeSignatureBuilder tb;
    tb.write_function_signature(1);
    tb.write_type_id(type_id_of<void>()); // return
    tb.write_ref();
    tb.write_type_id(type_id()); // param 1: T&
    return find_ctor({ .signature = tb.type_signature_view() });
}
RTTRInvokerAssign RTTRType::find_assign() const
{
    TypeSignatureBuilder tb;
    tb.write_function_signature(2);
    tb.write_type_id(type_id_of<void>()); // return
    tb.write_ref();
    tb.write_type_id(type_id()); // param 1: T&
    tb.write_const_ref();
    tb.write_type_id(type_id()); // param 2: const T&
    return find_extern_method({ .name = { CPPExternMethods::Assign }, .signature = tb.type_signature_view() });
}
RTTRInvokerMoveAssign RTTRType::find_move_assign() const
{
    TypeSignatureBuilder tb;
    tb.write_function_signature(2);
    tb.write_type_id(type_id_of<void>()); // return
    tb.write_ref();
    tb.write_type_id(type_id()); // param 1: T&
    tb.write_ref();
    tb.write_type_id(type_id()); // param 2: T&
    return find_extern_method({ .name = { CPPExternMethods::Assign }, .signature = tb.type_signature_view() });
}
RTTRInvokerEqual RTTRType::find_equal() const
{
    TypeSignatureBuilder tb;
    tb.write_function_signature(2);
    tb.write_type_id(type_id_of<bool>()); // return
    tb.write_const_ref();
    tb.write_type_id(type_id()); // param 1: const T&
    tb.write_const_ref();
    tb.write_type_id(type_id()); // param 2: const T&
    return find_extern_method({ .name = { CPPExternMethods::Eq }, .signature = tb.type_signature_view() });
}
RTTRInvokerHash RTTRType::find_hash() const
{
    TypeSignatureBuilder tb;
    tb.write_function_signature(1);
    tb.write_type_id(type_id_of<size_t>()); // return
    tb.write_const_ref();
    tb.write_type_id(type_id()); // param 1: const T&
    return find_extern_method({ .name = { SkrCoreExternMethods::Hash }, .signature = tb.type_signature_view() });
}
RTTRInvokerSwap RTTRType::find_swap() const
{
    TypeSignatureBuilder tb;
    tb.write_function_signature(2);
    tb.write_type_id(type_id_of<void>()); // return
    tb.write_ref();
    tb.write_type_id(type_id()); // param 1: T&
    tb.write_ref();
    tb.write_type_id(type_id()); // param 2: T&
    return find_extern_method({ .name = { SkrCoreExternMethods::Swap }, .signature = tb.type_signature_view() });
}

// flag & attribute
ERTTRRecordFlag RTTRType::record_flag() const
{
    SKR_ASSERT(_type_category == ERTTRTypeCategory::Record && "Type category mismatch when get record flag");
    return _record_data.flag;
}
ERTTREnumFlag RTTRType::enum_flag() const
{
    SKR_ASSERT(_type_category == ERTTRTypeCategory::Enum && "Type category mismatch when get enum flag");
    return _enum_data.flag;
}
const Any* RTTRType::find_attribute(TypeSignatureView signature) const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Record: {
        auto result = _record_data.attrs.find_if([&](const Any& v) { return v.type_is(signature); });
        if (result)
        {
            return result.ptr();
        }
        break;
    }
    case ERTTRTypeCategory::Enum: {
        auto result = _enum_data.attrs.find_if([&](const Any& v) { return v.type_is(signature); });
        if (result)
        {
            return result.ptr();
        }
        break;
    }
    default:
        break;
    }
    return nullptr;
}
void RTTRType::each_attribute(FunctionRef<void(const Any&)> each_func) const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Record: {
        for (const auto& attr : _record_data.attrs)
        {
            each_func(attr);
        }
        break;
    }
    case ERTTRTypeCategory::Enum: {
        for (const auto& attr : _enum_data.attrs)
        {
            each_func(attr);
        }
        break;
    }
    default:
        break;
    }
}
void RTTRType::each_attribute(FunctionRef<void(const Any&)> each_func, TypeSignatureView signature) const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Record: {
        for (const auto& attr : _record_data.attrs)
        {
            if (attr.type_is(signature))
            {
                each_func(attr);
            }
        }
        break;
    }
    case ERTTRTypeCategory::Enum: {
        for (const auto& attr : _enum_data.attrs)
        {
            if (attr.type_is(signature))
            {
                each_func(attr);
            }
        }
        break;
    }
    default:
        break;
    }
}

// helpers
bool RTTRType::_build_caster(RTTRTypeCaster& caster, const GUID& type_id) const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Primitive: {
        return type_id == _primitive_data.type_id;
    }
    case ERTTRTypeCategory::Record: {
        if (type_id == _record_data.type_id)
        {
            return true;
        }
        else
        {
            // find base and cast
            for (const auto& base : _record_data.bases_data)
            {
                if (type_id == base->type_id)
                {
                    caster.cast_funcs.add(base->cast_to_base);
                    return true;
                }
            }

            // get base type and continue cast
            for (const auto& base : _record_data.bases_data)
            {
                auto type = get_type_from_guid(base->type_id);
                if (type)
                {
                    if (_build_caster(caster, type_id))
                    {
                        caster.cast_funcs.add(base->cast_to_base);
                        return true;
                    }
                }
                else
                {
                    SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing cast from \"{}\"", type_id, _record_data.type_id);
                }
            }
        }
    }
    case ERTTRTypeCategory::Enum: {
        return (type_id == _enum_data.type_id || type_id == _enum_data.underlying_type_id);
    }
    default:
        SKR_UNREACHABLE_CODE()
        break;
    }
    return false;
}
} // namespace skr