#include "SkrContainersDef/set.hpp"
#include "SkrCore/log.hpp"
#include "SkrRTTR/script/scriptble_object.hpp"
#include <SkrRTTR/script/script_binder.hpp>

// helpers
namespace skr
{
template <typename OverloadType>
inline void _solve_param_return_count(OverloadType& overload)
{
    // solve return count
    overload.return_count = overload.return_binder.is_void ? 0 : 1;
    overload.params_count = 0;

    // each param and add count
    for (ScriptBinderParam& param : overload.params_binder)
    {

        if (param.binder.is_value())
        { // optimize case for value
            if (param.inout_flag == ERTTRParamFlag::Out)
            { // pure out, return
                param.appare_in_return = true;
                ++overload.return_count;
            }
            else
            {
                ++overload.params_count;
            }
        }
        else
        {
            if (param.inout_flag != ERTTRParamFlag::Out)
            { // not pure out
                ++overload.params_count;
            }
            if (flag_all(param.inout_flag, ERTTRParamFlag::Out))
            { // has out flag
                param.appare_in_return = true;
                ++overload.return_count;
            }
        }
    }
}
} // namespace skr

namespace skr
{
// ctor & dtor
ScriptBinderManager::ScriptBinderManager()
{
}
ScriptBinderManager::~ScriptBinderManager()
{
    clear();
}

// get binder
ScriptBinderRoot ScriptBinderManager::get_or_build(GUID type_id)
{
    // find cache
    if (auto result = _cached_root_binders.find(type_id))
    {
        return result.value();
    }

    // make new binder
    if (auto* primitive_binder = _make_primitive(type_id))
    {
        _cached_root_binders.add(type_id, primitive_binder);
        return primitive_binder;
    }

    // mapping or object
    auto* type = get_type_from_guid(type_id);
    if (type->is_enum())
    {
        // make enum binder
        if (auto* enum_binder = _make_enum(type))
        {
            _cached_root_binders.add(type_id, enum_binder);
            return enum_binder;
        }
    }
    else if (type->based_on(type_id_of<ScriptbleObject>()))
    {
        // make object binder
        if (auto* object_binder = _make_object(type))
        {
            _cached_root_binders.add(type_id, object_binder);
            return object_binder;
        }
    }
    else if (flag_all(type->record_flag(), ERTTRRecordFlag::ScriptMapping))
    {
        // make mapping binder
        if (auto* mapping_binder = _make_mapping(type))
        {
            _cached_root_binders.add(type_id, mapping_binder);
            return mapping_binder;
        }
    }
    else
    {
        // make value binder
        if (auto* value_binder = _make_value(type))
        {
            _cached_root_binders.add(type_id, value_binder);
            return value_binder;
        }
    }

    // failed to export
    if (type)
    {
        _logger.error(u8"failed to export type '{}'", type->name());
    }
    else
    {
        _logger.error(u8"failed to export type '{}'", type_id);
    }
    _cached_root_binders.add(type_id, {});
    return {};
}
ScriptBinderCallScript ScriptBinderManager::build_call_script_binder(span<const StackProxy> params, StackProxy ret)
{
    auto _log_stack = _logger.stack(u8"export call script binder");

    ScriptBinderCallScript out;

    // export params
    uint32_t index = 0;
    for (const auto& param : params)
    {
        RTTRParamData param_data = {};
        param_data.type          = param.signature;
        param_data.index         = index;
        format_to(param_data.name, u8"#{}", index);

        auto& param_binder = out.params_binder.add_default().ref();
        _make_param_call_script(param_binder, &param_data);
        param_binder.data = nullptr; // temporal data

        ++index;
    }

    // export return
    _make_return_call_script(out.return_binder, ret.data ? ret.signature : type_signature_of<void>());

    // solve param count and return count
    _solve_param_return_count(out);

    out.failed |= _logger.any_error();
    return out;
}

// each
void ScriptBinderManager::each_cached_root_binder(FunctionRef<void(const GUID&, const ScriptBinderRoot&)> func)
{
    for (auto& [type_id, binder] : _cached_root_binders)
    {
        func(type_id, binder);
    }
}

// clear
void ScriptBinderManager::clear()
{
    // delete root binder cache
    for (auto& [type_id, binder] : _cached_root_binders)
    {
        switch (binder.kind())
        {
        case ScriptBinderRoot::EKind::Primitive:
            SkrDelete(binder.primitive());
            break;
        case ScriptBinderRoot::EKind::Mapping:
            SkrDelete(binder.mapping());
            break;
        case ScriptBinderRoot::EKind::Object:
            SkrDelete(binder.object());
            break;
        case ScriptBinderRoot::EKind::Enum:
            SkrDelete(binder.enum_());
            break;
        case ScriptBinderRoot::EKind::Value:
            SkrDelete(binder.value());
            break;
        default:
            SKR_UNREACHABLE_CODE()
        }
    }
    _cached_root_binders.clear();
}

// make root binder
template <typename T>
ScriptBinderPrimitive* _new_primitive()
{
    ScriptBinderPrimitive* result = SkrNew<ScriptBinderPrimitive>();

    // setup primitive data
    result->size      = sizeof(T);
    result->alignment = alignof(T);
    result->type_id   = type_id_of<T>();
    if constexpr (std::is_trivially_destructible_v<T>)
    {
        result->dtor = nullptr;
    }
    else
    {
        result->dtor = +[](void* p) -> void {
            reinterpret_cast<T*>(p)->~T();
        };
    }

    return result;
}
ScriptBinderPrimitive* _new_void()
{
    ScriptBinderPrimitive* result = SkrNew<ScriptBinderPrimitive>();

    result->size      = 0;
    result->alignment = 0;
    result->type_id   = type_id_of<void>();
    result->dtor      = nullptr;

    return result;
}
ScriptBinderPrimitive* ScriptBinderManager::_make_primitive(GUID type_id)
{
    switch (type_id.get_hash())
    {
    case type_id_of<void>().get_hash():
        return _new_void();
    case type_id_of<int8_t>().get_hash():
        return _new_primitive<int8_t>();
    case type_id_of<int16_t>().get_hash():
        return _new_primitive<int16_t>();
    case type_id_of<int32_t>().get_hash():
        return _new_primitive<int32_t>();
    case type_id_of<int64_t>().get_hash():
        return _new_primitive<int64_t>();
    case type_id_of<uint8_t>().get_hash():
        return _new_primitive<uint8_t>();
    case type_id_of<uint16_t>().get_hash():
        return _new_primitive<uint16_t>();
    case type_id_of<uint32_t>().get_hash():
        return _new_primitive<uint32_t>();
    case type_id_of<uint64_t>().get_hash():
        return _new_primitive<uint64_t>();
    case type_id_of<float>().get_hash():
        return _new_primitive<float>();
    case type_id_of<double>().get_hash():
        return _new_primitive<double>();
    case type_id_of<bool>().get_hash():
        return _new_primitive<bool>();
    case type_id_of<String>().get_hash():
        return _new_primitive<String>();
    case type_id_of<StringView>().get_hash():
        return _new_primitive<StringView>();
    default:
        return nullptr;
    }
}
ScriptBinderMapping* ScriptBinderManager::_make_mapping(const RTTRType* type)
{
    auto _log_stack = _logger.stack(u8"export mapping type {}", type->name());

    // check flag
    // clang-format off
    if (!flag_all(
        type->record_flag(),
        ERTTRRecordFlag::ScriptVisible | ERTTRRecordFlag::ScriptMapping
    ))
    {
        return nullptr;
    }
    // clang-format on

    ScriptBinderMapping result{};
    result.type = type;

    // each field
    type->each_field([&](const RTTRFieldData* field, const RTTRType* owner_type) {
        if (!flag_all(field->flag, ERTTRFieldFlag::ScriptVisible)) { return; }
        auto& field_data = result.fields.try_add_default(field->name).value();
        _make_field(field_data, field, owner_type);
    });

    result.failed |= _logger.any_error();

    // return result
    return SkrNew<ScriptBinderMapping>(std::move(result));
}
ScriptBinderObject* ScriptBinderManager::_make_object(const RTTRType* type)
{
    auto _log_stack = _logger.stack(u8"export object type {}", type->name());

    // check flag
    // clang-format off
    if (!flag_all(
        type->record_flag(),
        ERTTRRecordFlag::ScriptVisible
    ))
    {
        return nullptr;
    }
    // clang-format on

    // check base
    if (!type->based_on(type_id_of<ScriptbleObject>()))
    {
        return nullptr;
    }

    // build result
    auto* result = SkrNew<ScriptBinderObject>();
    _cached_root_binders.add(type->type_id(), result);
    _fill_record_info(*result, type);

    // return result
    return result;
}
ScriptBinderValue* ScriptBinderManager::_make_value(const RTTRType* type)
{
    auto _log_stack = _logger.stack(u8"export value type {}", type->name());

    // check flag
    // clang-format off
    if (!flag_all(
        type->record_flag(),
        ERTTRRecordFlag::ScriptVisible
    ))
    {
        return nullptr;
    }
    // clang-format on

    // build result
    auto* result = SkrNew<ScriptBinderValue>();
    _cached_root_binders.add(type->type_id(), result);
    _fill_record_info(*result, type);

    // return result
    return result;
}
ScriptBinderEnum* ScriptBinderManager::_make_enum(const RTTRType* type)
{
    auto _log_stack = _logger.stack(u8"export enum type {}", type->name());

    ScriptBinderEnum result = {};

    result.type = type;

    // export underlying type
    result.underlying_binder = _make_primitive(type->enum_underlying_type_id());
    if (!result.underlying_binder)
    {
        _logger.error(u8"enum '{}' underlying type not supported", type->name());
        return nullptr;
    }

    // export items
    type->each_enum_items([&](const RTTREnumItemData* item) {
        // filter by flag
        if (!flag_all(item->flag, ERTTREnumItemFlag::ScriptVisible)) { return; }

        result.items.add(item->name, item);
        result.is_signed = item->value.is_signed();
    });

    return SkrNew<ScriptBinderEnum>(std::move(result));
}
void ScriptBinderManager::_fill_record_info(ScriptBinderRecordBase& out, const RTTRType* type)
{
    out.type = type;

    // export ctors
    out.is_script_newable = flag_all(type->record_flag(), ERTTRRecordFlag::ScriptNewable);
    if (out.is_script_newable)
    {
        type->each_ctor([&](const RTTRCtorData* ctor) {
            if (!flag_all(ctor->flag, ERTTRCtorFlag::ScriptVisible)) { return; }
            if (out.ctor.data)
            {
                _logger.error(u8"overload is not supported yet for ctor '{}'", type->name());
                return;
            }
            _make_ctor(out.ctor, ctor, type);
        });
    }

    Set<const RTTRMethodData*> mixin_consumed_methods;

    // export mixin
    type->each_method([&](const RTTRMethodData* method, const RTTRType* owner_type) {
        if (!flag_all(owner_type->record_flag(), ERTTRRecordFlag::ScriptVisible)) { return; }
        if (!flag_all(method->flag, ERTTRMethodFlag::ScriptVisible)) { return; }
        if (!flag_all(method->flag, ERTTRMethodFlag::ScriptMixin)) { return; }

        mixin_consumed_methods.add(method);

        // find mixin impl
        String impl_method_name  = String::Build(method->name, u8"_impl");
        auto   found_impl_method = owner_type->find_method({
              .name          = impl_method_name,
              .include_bases = false,
        });
        if (!found_impl_method)
        {
            _logger.error(u8"miss impl for mixin method '{}', witch should be named '{}'", method->name, impl_method_name);
            return;
        }
        mixin_consumed_methods.add(found_impl_method);

        // export
        if (auto found_mixin = out.methods.find(method->name))
        {
            _logger.error(u8"overload is not supported yet for mixin method '{}'", method->name);
        }
        else
        {
            auto& mixin_method_data    = out.methods.try_add_default(method->name, found_mixin).value();
            mixin_method_data.is_mixin = true;
            _make_mixin_method(mixin_method_data, method, found_impl_method, owner_type);
        }

        // clang-format off
    }, {.include_bases = true});
    // clang-format on

    // export fields
    type->each_field([&](const RTTRFieldData* field, const RTTRType* owner_type) {
        if (!flag_all(owner_type->record_flag(), ERTTRRecordFlag::ScriptVisible)) { return; }
        if (!flag_all(field->flag, ERTTRFieldFlag::ScriptVisible)) { return; }

        auto& field_data = out.fields.try_add_default(field->name).value();
        _make_field(field_data, field, owner_type);
        // clang-format off
    }, { .include_bases = true });
    // clang-format on

    // export static field
    type->each_static_field([&](const RTTRStaticFieldData* static_field, const RTTRType* owner_type) {
        if (!flag_all(owner_type->record_flag(), ERTTRRecordFlag::ScriptVisible)) { return; }
        if (!flag_all(static_field->flag, ERTTRStaticFieldFlag::ScriptVisible)) { return; }

        auto& static_field_data = out.static_fields.try_add_default(static_field->name).value();
        _make_static_field(static_field_data, static_field, owner_type);
        // clang-format off
    }, { .include_bases = true });
    // clang-format on

    // export methods or properties
    type->each_method([&](const RTTRMethodData* method, const RTTRType* owner_type) {
        if (!flag_all(owner_type->record_flag(), ERTTRRecordFlag::ScriptVisible)) { return; }
        if (!flag_all(method->flag, ERTTRMethodFlag::ScriptVisible)) { return; }
        if (mixin_consumed_methods.contains(method)) { return; }

        auto find_getter_result = method->attrs.find_if([&](const Any& attr) {
            return attr.type_is<skr::attr::ScriptGetter>();
        });
        auto find_setter_result = method->attrs.find_if([&](const Any& attr) {
            return attr.type_is<skr::attr::ScriptSetter>();
        });

        // check flag error
        if (find_getter_result && find_setter_result)
        {
            _logger.error(u8"getter and setter applied on same method {}", method->name);
            return;
        }

        if (find_getter_result)
        { // getter case
            String prop_name  = find_getter_result.ref().cast<skr::attr::ScriptGetter>()->prop_name;
            auto&  prop_data  = out.properties.try_add_default(prop_name).value();
            auto   _log_stack = _logger.stack(u8"export property '{}' getter", prop_name);
            if (prop_data.getter.data)
            {
                _logger.error(u8"overload is not supported yet for property '{}'", prop_name);
            }
            else
            {
                _make_prop_getter(prop_data.getter, method, owner_type);
            }
        }
        else if (find_setter_result)
        { // setter case
            String prop_name  = find_setter_result.ref().cast<skr::attr::ScriptSetter>()->prop_name;
            auto&  prop_data  = out.properties.try_add_default(prop_name).value();
            auto   _log_stack = _logger.stack(u8"export property '{}' setter", prop_name);
            if (prop_data.setter.data)
            {
                _logger.error(u8"overload is not supported yet for property '{}'", prop_name);
            }
            else
            {
                _make_prop_setter(prop_data.setter, method, owner_type);
            }
        }
        else
        { // normal method
            if (auto found_method = out.methods.find(method->name))
            {
                _logger.error(u8"overload is not supported yet for method '{}'", method->name);
            }
            else
            {
                // add method
                auto& method_data = out.methods.try_add_default(method->name, found_method).value();
                _make_method(method_data, method, owner_type);
            }
        }
        // clang-format off
    }, { .include_bases = true });
    // clang-format on

    // export static methods or properties
    type->each_static_method([&](const RTTRStaticMethodData* method, const RTTRType* owner_type) {
        if (!flag_all(owner_type->record_flag(), ERTTRRecordFlag::ScriptVisible)) { return; }
        if (!flag_all(method->flag, ERTTRStaticMethodFlag::ScriptVisible)) { return; }

        auto find_getter_result = method->attrs.find_if([&](const Any& attr) {
            return attr.type_is<skr::attr::ScriptGetter>();
        });
        auto find_setter_result = method->attrs.find_if([&](const Any& attr) {
            return attr.type_is<skr::attr::ScriptSetter>();
        });

        // check flag error
        if (find_getter_result && find_setter_result)
        {
            _logger.error(u8"getter and setter applied on same static method{}", method->name);
            return;
        }

        if (find_getter_result)
        { // getter case
            String prop_name  = find_getter_result.ref().cast<skr::attr::ScriptGetter>()->prop_name;
            auto&  prop_data  = out.static_properties.try_add_default(prop_name).value();
            auto   _log_stack = _logger.stack(u8"export static getter '{}'", prop_name);
            if (prop_data.getter.data)
            {
                _logger.error(u8"overload is not supported yet for static property '{}'", prop_name);
            }
            else
            {
                _make_static_prop_getter(prop_data.getter, method, owner_type);
            }
        }
        else if (find_setter_result)
        { // setter case
            String prop_name  = find_setter_result.ref().cast<skr::attr::ScriptSetter>()->prop_name;
            auto&  prop_data  = out.static_properties.try_add_default(prop_name).value();
            auto   _log_stack = _logger.stack(u8"export static setter '{}'", prop_name);
            if (prop_data.setter.data)
            {
                _logger.error(u8"overload is not supported yet for static property '{}'", prop_name);
            }
            else
            {
                _make_static_prop_setter(prop_data.setter, method, owner_type);
            }
        }
        else
        { // normal method
            if (auto found_method = out.static_methods.find(method->name))
            {
                _logger.error(u8"overload is not supported yet for static method '{}'", method->name);
            }
            else
            {
                // add method
                auto& method_data = out.static_methods.try_add_default(method->name, found_method).value();
                _make_static_method(method_data, method, owner_type);
            }
        }
        // clang-format off
    }, { .include_bases = true });
    // clang-format on

    // check properties conflict
    for (auto& [name, prop] : out.properties)
    {
        if (prop.failed) { continue; }

        // check property type
        {
            auto& getter = prop.getter;
            auto& setter = prop.setter;

            if (!getter.failed && !setter.failed)
            {
                auto getter_binder = getter.return_binder.binder;
                auto setter_binder = setter.params_binder[0].binder;

                bool prop_mismatch = false;
                if (getter_binder != setter_binder)
                {
                    if (getter_binder.is_primitive() && setter_binder.is_primitive())
                    {
                        bool getter_is_str = getter_binder.primitive()->type_id == type_id_of<String>() ||
                                             getter_binder.primitive()->type_id == type_id_of<StringView>();
                        bool setter_is_str = setter_binder.primitive()->type_id == type_id_of<String>() ||
                                             setter_binder.primitive()->type_id == type_id_of<StringView>();
                        if (getter_is_str && setter_is_str)
                        { // optimize for string
                        }
                        else
                        {
                            prop_mismatch = true;
                        }
                    }
                    else
                    {
                        prop_mismatch = true;
                    }
                }
                if (prop_mismatch)
                {
                    _logger.error(
                        u8"prop '{}' getter and setter type mismatch, getter '{}', setter '{}' ",
                        name,
                        getter.data->name,
                        setter.data->name
                    );
                    prop.failed = true;
                }
                prop.binder = getter.return_binder.binder;
            }
        }
    }

    // check static properties conflict
    for (auto& [name, static_prop] : out.static_properties)
    {
        if (static_prop.failed) { continue; }

        // check property type
        {
            auto& getter = static_prop.getter;
            auto& setter = static_prop.setter;

            if (!getter.failed && !setter.failed)
            {
                auto getter_binder = getter.return_binder.binder;
                auto setter_binder = setter.params_binder[0].binder;

                bool prop_mismatch = false;
                if (getter_binder != setter_binder)
                {
                    if (getter_binder.is_primitive() && setter_binder.is_primitive())
                    {
                        bool getter_is_str = getter_binder.primitive()->type_id == type_id_of<String>() ||
                                             getter_binder.primitive()->type_id == type_id_of<StringView>();
                        bool setter_is_str = setter_binder.primitive()->type_id == type_id_of<String>() ||
                                             setter_binder.primitive()->type_id == type_id_of<StringView>();
                        if (getter_is_str && setter_is_str)
                        { // optimize for string
                        }
                        else
                        {
                            prop_mismatch = true;
                        }
                    }
                    else
                    {
                        prop_mismatch = true;
                    }
                }
                if (prop_mismatch)
                {
                    _logger.error(
                        u8"prop '{}' getter and setter type mismatch, getter '{}', setter '{}'",
                        name,
                        getter.data->name,
                        setter.data->name
                    );
                    static_prop.failed = true;
                }
                static_prop.binder = getter_binder;
            }
        }
    }

    out.failed |= _logger.any_error();
}

// make nested binder
void ScriptBinderManager::_make_ctor(ScriptBinderCtor& out, const RTTRCtorData* ctor, const RTTRType* owner)
{
    auto _log_stack = _logger.stack(u8"export ctor");

    // basic data
    out.data = ctor;

    // export params
    for (const auto* param : ctor->param_data)
    {
        auto& param_binder = out.params_binder.add_default().ref();
        _make_param(param_binder, param);
    }

    // check out flag
    for (const auto& param_binder : out.params_binder)
    {
        if (flag_all(param_binder.inout_flag, ERTTRParamFlag::Out))
        {
            auto _param_log_stack = _logger.stack(
                u8"export param '{}', index {}",
                param_binder.data->name,
                param_binder.data->index
            );
            _logger.error(u8"ctor param cannot has out flag");
        }
    }

    out.failed |= _logger.any_error();
}
void ScriptBinderManager::_make_method(ScriptBinderMethod& out, const RTTRMethodData* method, const RTTRType* owner)
{
    auto _log_stack = _logger.stack(u8"export method '{}'", method->name);

    // basic data
    out.data  = method;
    out.owner = owner;

    // export params
    for (const auto* param : method->param_data)
    {
        auto& param_binder = out.params_binder.add_default().ref();
        _make_param(param_binder, param);
    }

    // export return
    _make_return(out.return_binder, method->ret_type.view());

    // solve param count and return count
    _solve_param_return_count(out);

    out.failed |= _logger.any_error();
}
void ScriptBinderManager::_make_static_method(ScriptBinderStaticMethod& out, const RTTRStaticMethodData* static_method, const RTTRType* owner)
{
    auto _log_stack = _logger.stack(u8"export static method {}", static_method->name);

    // basic data
    out.data  = static_method;
    out.owner = owner;

    // export params
    for (const auto* param : static_method->param_data)
    {
        auto& param_binder = out.params_binder.add_default().ref();
        _make_param(param_binder, param);
    }

    // export return
    _make_return(out.return_binder, static_method->ret_type.view());

    // solve param count and return count
    _solve_param_return_count(out);

    out.failed |= _logger.any_error();
}
void ScriptBinderManager::_make_mixin_method(ScriptBinderMethod& out, const RTTRMethodData* method, const RTTRMethodData* impl_method, const RTTRType* owner)
{
    auto _log_stack = _logger.stack(u8"export mixin method {}", method->name);

    // basic data
    out.data            = method;
    out.mixin_impl_data = impl_method;
    out.owner           = owner;

    // check signature
    if (!method->signature_equal(*impl_method, ETypeSignatureCompareFlag::Strict))
    {
        _logger.error(u8"mixin method '{}' and '{}'_impl signature not match", method->name);
    }

    // export params
    for (const auto* param : method->param_data)
    {
        auto& param_binder = out.params_binder.add_default().ref();
        _make_param_call_script(param_binder, param);
    }

    // export return
    _make_return_call_script(out.return_binder, method->ret_type.view());

    // solve param count and return count
    _solve_param_return_count(out);

    out.failed |= _logger.any_error();
}
void ScriptBinderManager::_make_prop_getter(ScriptBinderMethod& out, const RTTRMethodData* method, const RTTRType* owner)
{
    // check param count
    if (method->param_data.size() != 0)
    {
        _logger.error(u8"getter param count must be 0, but got {}", method->param_data.size());
        return;
    }

    // check return type
    TypeSignatureTyped<void> void_signature;
    if (method->ret_type.view().equal(void_signature))
    {
        _logger.error(u8"getter return type must not be void");
        return;
    }

    // fill data
    _make_method(out, method, owner);

    out.failed |= _logger.any_error();
}
void ScriptBinderManager::_make_prop_setter(ScriptBinderMethod& out, const RTTRMethodData* method, const RTTRType* owner)
{
    // check param count
    if (method->param_data.size() != 1)
    {
        _logger.error(u8"setter param count must be 1, but got {}", method->param_data.size());
        return;
    }

    // check return type
    TypeSignatureTyped<void> void_signature;
    if (!method->ret_type.view().equal(void_signature))
    {
        _logger.error(u8"setter return type must be void");
        return;
    }

    // fill data
    _make_method(out, method, owner);

    out.failed |= _logger.any_error();
}
void ScriptBinderManager::_make_static_prop_getter(ScriptBinderStaticMethod& out, const RTTRStaticMethodData* method, const RTTRType* owner)
{
    // check param count
    if (method->param_data.size() != 0)
    {
        _logger.error(u8"getter param count must be 0, but got {}", method->param_data.size());
        return;
    }

    // check return type
    TypeSignatureTyped<void> void_signature;
    if (method->ret_type.view().equal(void_signature))
    {
        _logger.error(u8"getter return type must not be void");
        return;
    }

    // fill data
    _make_static_method(out, method, owner);

    out.failed |= _logger.any_error();
}
void ScriptBinderManager::_make_static_prop_setter(ScriptBinderStaticMethod& out, const RTTRStaticMethodData* method, const RTTRType* owner)
{
    // check param count
    if (method->param_data.size() != 1)
    {
        _logger.error(u8"setter param count must be 1, but got {}", method->param_data.size());
        return;
    }

    // check return type
    TypeSignatureTyped<void> void_signature;
    if (!method->ret_type.view().equal(void_signature))
    {
        _logger.error(u8"setter return type must be void");
        return;
    }

    // fill data
    _make_static_method(out, method, owner);

    out.failed |= _logger.any_error();
}
void ScriptBinderManager::_make_field(ScriptBinderField& out, const RTTRFieldData* field, const RTTRType* owner)
{
    auto _log_stack = _logger.stack(u8"export field '{}'", field->name);

    // get type signature
    auto signature = field->type.view();

    // export
    ScriptBinderRoot field_binder = {};
    _try_export_field(signature, field_binder);
    // clang-format off
    out = {
        .owner = owner,
        .binder = field_binder,
        .data = field,
    };
    // clang-format on

    out.failed |= _logger.any_error();
}
void ScriptBinderManager::_make_static_field(ScriptBinderStaticField& out, const RTTRStaticFieldData* static_field, const RTTRType* owner)
{
    auto _log_stack = _logger.stack(u8"export static field '{}'", static_field->name);

    // get type signature
    auto signature = static_field->type.view();

    // export
    ScriptBinderRoot field_binder = {};
    _try_export_field(signature, field_binder);
    // clang-format off
    out = {
        .owner = owner,
        .binder = field_binder,
        .data = static_field,
    };
    // clang-format on

    out.failed |= _logger.any_error();
}
void ScriptBinderManager::_make_param(ScriptBinderParam& out, const RTTRParamData* param)
{
    auto _log_stack = _logger.stack(u8"export param '{}', index {}", param->name, param->index);

    out.data  = param;
    out.index = param->index;

    // get type signature
    auto signature = param->type.view();

    // check signature type
    if (!signature.is_type())
    {
        _logger.error(u8"unsupported type");
        return;
    }

    // check pointer level
    if (signature.decayed_pointer_level() > 1)
    {
        _logger.error(u8"pointer level greater than 1");
        return;
    }

    // solve modifier
    bool is_any_ref         = signature.is_any_ref();
    bool is_pointer         = signature.is_pointer();
    bool is_decayed_pointer = is_any_ref || is_pointer;
    bool is_const           = signature.is_const();
    bool has_param_out      = flag_all(param->flag, ERTTRParamFlag::Out);
    bool has_param_in       = flag_all(param->flag, ERTTRParamFlag::In);
    if (!has_param_in && !has_param_out)
    { // use param default inout flag
        if (is_any_ref && !is_const)
        {
            out.inout_flag |= ERTTRParamFlag::In | ERTTRParamFlag::Out;
        }
    }
    else
    {
        if (has_param_in)
        {
            out.inout_flag |= ERTTRParamFlag::In;
        }
        if (has_param_out)
        {
            out.inout_flag |= ERTTRParamFlag::Out;
            if (!is_any_ref || is_const)
            {
                _logger.error(u8"only T& can be out param");
            }
        }
    }

    // read type id
    skr::GUID param_type_id;
    signature.jump_modifier();
    signature.read_type_id(param_type_id);

    // get binder
    out.binder = get_or_build(param_type_id);

    // solve dynamic stack kind
    out.pass_by_ref = is_decayed_pointer;

    // check bad binder
    if (out.binder.is_empty())
    {
        _logger.error(u8"unsupported type");
        return;
    }

    // check binder
    switch (out.binder.kind())
    {
    case ScriptBinderRoot::EKind::Primitive: {
        if (is_pointer)
        {
            _logger.error(
                u8"export primitive {} as pointer type",
                out.binder.primitive()->type_id
            );
        }
        else if (out.binder.primitive()->type_id == type_id_of<StringView>())
        {
            if (is_decayed_pointer)
            {
                _logger.error(u8"cannot export StringView as any reference");
            }
        }
        break;
    }
    case ScriptBinderRoot::EKind::Enum: {
        if (is_pointer)
        {
            _logger.error(
                u8"export enum {} as pointer type",
                out.binder.enum_()->type->name()
            );
        }
        break;
    }
    case ScriptBinderRoot::EKind::Mapping: {
        if (is_pointer)
        {
            _logger.error(
                u8"export mapping {} as pointer type",
                out.binder.mapping()->type->name()
            );
        }
        break;
    }
    case ScriptBinderRoot::EKind::Value: {
        if (is_pointer)
        {
            _logger.error(
                u8"export value {} as pointer type",
                out.binder.value()->type->name()
            );
        }
        break;
    }
    case ScriptBinderRoot::EKind::Object: {
        if (!is_pointer)
        {
            _logger.error(
                u8"export object {} as value or reference type",
                out.binder.object()->type->name()
            );
        }
        break;
    }
    default: {
        _logger.error(u8"unsupported type");
        break;
    }
    }
}
void ScriptBinderManager::_make_return(ScriptBinderReturn& out, TypeSignatureView signature)
{
    auto _log_stack = _logger.stack(u8"export return");

    // check signature type
    if (!signature.is_type())
    {
        _logger.error(u8"unsupported type");
        return;
    }

    // check pointer level
    if (signature.decayed_pointer_level() > 1)
    {
        _logger.error(u8"pointer level greater than 1");
        return;
    }

    // solve modifier
    bool is_pointer         = signature.is_pointer();
    bool is_decayed_pointer = signature.is_decayed_pointer();
    out.pass_by_ref         = is_decayed_pointer;

    // read type id
    skr::GUID param_type_id;
    signature.jump_modifier();
    signature.read_type_id(param_type_id);

    // get binder
    out.binder = get_or_build(param_type_id);

    // check binder
    switch (out.binder.kind())
    {
    case ScriptBinderRoot::EKind::Primitive: {
        if (is_pointer)
        {
            _logger.error(
                u8"export primitive {} as pointer type",
                out.binder.primitive()->type_id
            );
        }
        if (out.binder.primitive()->type_id == type_id_of<void>())
        {
            out.is_void = true;
        }
        break;
    }
    case ScriptBinderRoot::EKind::Enum: {
        if (is_pointer)
        {
            _logger.error(
                u8"export enum {} as pointer type",
                out.binder.enum_()->type->name()
            );
        }
        break;
    }
    case ScriptBinderRoot::EKind::Mapping: {
        if (is_pointer)
        {
            _logger.error(
                u8"export primitive/mapping {} as pointer type",
                out.binder.mapping()->type->name()
            );
        }
        break;
    }
    case ScriptBinderRoot::EKind::Value: {
        if (is_pointer)
        {
            _logger.error(
                u8"export value {} as pointer type",
                out.binder.value()->type->name()
            );
        }
        break;
    }
    case ScriptBinderRoot::EKind::Object: {
        if (!is_pointer)
        {
            _logger.error(
                u8"export object {} as value or reference type",
                out.binder.object()->type->name()
            );
        }
        break;
    }
    default: {
        _logger.error(u8"unsupported type");
        break;
    }
    }
}
void ScriptBinderManager::_make_param_call_script(ScriptBinderParam& out, const RTTRParamData* param)
{
    // just same as call native param
    _make_param(out, param);
}
void ScriptBinderManager::_make_return_call_script(ScriptBinderReturn& out, TypeSignatureView signature)
{
    auto _log_stack = _logger.stack(u8"export return");

    // check signature type
    if (!signature.is_type())
    {
        _logger.error(u8"unsupported type");
        return;
    }

    // check pointer level
    if (signature.decayed_pointer_level() > 1)
    {
        _logger.error(u8"pointer level greater than 1");
        return;
    }

    // solve modifier
    bool is_pointer         = signature.is_pointer();
    bool is_decayed_pointer = signature.is_decayed_pointer();
    out.pass_by_ref         = is_decayed_pointer;

    // read type id
    skr::GUID param_type_id;
    signature.jump_modifier();
    signature.read_type_id(param_type_id);

    // get binder
    out.binder = get_or_build(param_type_id);

    // check binder
    switch (out.binder.kind())
    {
    case ScriptBinderRoot::EKind::Primitive: {
        if (is_decayed_pointer)
        {
            _logger.error(
                u8"cannot return primitive reference when using mixin",
                out.binder.primitive()->type_id
            );
        }
        if (out.binder.primitive()->type_id == type_id_of<void>())
        {
            out.is_void = true;
        }
        break;
    }
    case ScriptBinderRoot::EKind::Enum: {
        if (is_decayed_pointer)
        {
            _logger.error(
                u8"cannot return enum reference when using mixin",
                out.binder.enum_()->type->name()
            );
        }
        break;
    }
    case ScriptBinderRoot::EKind::Mapping: {
        if (is_decayed_pointer)
        {
            _logger.error(
                u8"cannot return primitive/mapping reference when using mixin",
                out.binder.mapping()->type->name()
            );
        }
        break;
    }
    case ScriptBinderRoot::EKind::Value: {
        if (is_decayed_pointer)
        {
            _logger.error(
                u8"cannot return value reference when using mixin",
                out.binder.value()->type->name()
            );
        }
        break;
    }
    case ScriptBinderRoot::EKind::Object: {
        if (!is_pointer)
        {
            _logger.error(
                u8"export object {} as value or reference type",
                out.binder.object()->type->name()
            );
        }
        break;
    }
    default: {
        _logger.error(u8"unsupported type");
        break;
    }
    }
}

// checker
void ScriptBinderManager::_try_export_field(TypeSignatureView signature, ScriptBinderRoot& out_binder)
{
    // check type
    if (!signature.is_type())
    {
        _logger.error(u8"unsupported type");
        return;
    }

    // check pointer level
    if (signature.decayed_pointer_level() > 1)
    {
        _logger.error(u8"pointer level greater than 1");
        return;
    }

    if (signature.is_decayed_pointer())
    { // only support pointer type for object binder
        if (signature.is_any_ref())
        {
            _logger.error(u8"cannot export reference field");
            return;
        }

        // read type id
        skr::GUID field_type_id;
        signature.jump_modifier();
        signature.read_type_id(field_type_id);

        // export object type
        out_binder = get_or_build(field_type_id);
        if (!out_binder.is_object())
        {
            _logger.error(u8"cannot export pointer field for non-object");
            return;
        }
    }
    else
    {
        // read type id
        skr::GUID field_type_id;
        signature.jump_modifier();
        signature.read_type_id(field_type_id);

        // export primitive type
        out_binder = get_or_build(field_type_id);
        if (out_binder.is_empty())
        {
            _logger.error(u8"unsupported type");
            return;
        }
        else if (out_binder.is_object())
        {
            _logger.error(u8"cannot export object as value type");
        }
        else if (out_binder.is_primitive() && out_binder.primitive()->type_id == type_id_of<StringView>())
        {
            _logger.error(u8"cannot export StringView as field");
        }
    }
}
} // namespace skr