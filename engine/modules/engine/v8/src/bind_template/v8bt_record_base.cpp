#include <SkrV8/bind_template/v8bt_record_base.hpp>
#include <SkrV8/v8_bind.hpp>
#include <SkrV8/v8_bind_proxy.hpp>
#include <SkrV8/v8_isolate.hpp>
#include <SkrV8/ts_def_builder.hpp>
#include <SkrV8/bind_template/v8bt_primitive.hpp>

// v8 includes
#include <libplatform/libplatform.h>
#include <v8-initialization.h>
#include <v8-template.h>
#include <v8-external.h>
#include <v8-function.h>
#include <v8-container.h>

namespace skr
{
void V8BTRecordBase::_setup(V8Isolate* isolate, const RTTRType* type)
{
    // get basic data
    set_isolate(isolate);
    _rttr_type = type;
    _default_ctor = type->find_default_ctor();
    if (!_default_ctor)
    {
        _errors.error(u8"record type '{}' has no default ctor", type->name());
    }
    _dtor = type->dtor_invoker();

    // export ctor
    _is_script_newable = flag_all(type->record_flag(), ERTTRRecordFlag::ScriptNewable);
    type->each_ctor([&](const RTTRCtorData* ctor) {
        if (!flag_all(ctor->flag, ERTTRCtorFlag::ScriptVisible)) { return; }
        if (_ctor.is_valid())
        {
            _errors.error(u8"overload is not supported yet for ctor '{}'", type->name());
            return;
        }
        _ctor.setup(isolate, ctor);
    });

    // export mixin
    Set<const RTTRMethodData*> mixin_consumed_methods;
    type->each_method([&](const RTTRMethodData* method, const RTTRType* owner_type) {
        if (!flag_all(owner_type->record_flag(), ERTTRRecordFlag::ScriptVisible)) { return; }
        if (!flag_all(method->flag, ERTTRMethodFlag::ScriptVisible)) { return; }
        if (!flag_all(method->flag, ERTTRMethodFlag::ScriptMixin)) { return; }

        mixin_consumed_methods.add(method);

        // find mixin impl
        String impl_method_name = String::Build(method->name, u8"_impl");
        auto found_impl_method = owner_type->find_method({
            .name = impl_method_name,
            .include_bases = false,
        });
        if (!found_impl_method)
        {
            _errors.error(u8"miss impl for mixin method '{}', witch should be named '{}'", method->name, impl_method_name);
            return;
        }
        mixin_consumed_methods.add(found_impl_method);

        // check overload
        if (auto found_mixin = _methods.find(method->name))
        {
            _errors.error(u8"overload is not supported yet for mixin method '{}'", method->name);
            return;
        }

        // add method
        auto& mixin_method_data = _methods.try_add_default(method->name).value();
        mixin_method_data.rttr_data_mixin_impl = found_impl_method;
        mixin_method_data.setup(
            isolate,
            method,
            owner_type
        );

        // check signature
        if (!method->signature_equal(
                *found_impl_method,
                ETypeSignatureCompareFlag::Strict
            ))
        {
            mixin_method_data.errors.error(
                u8"mixin method '{}' and '{}'_impl signature miss match",
                method->name,
                impl_method_name
            );
        }
        // clang-format off
    }, {.include_bases = true});
    // clang-format on

    // export fields
    type->each_field([&](const RTTRFieldData* field, const RTTRType* owner_type) {
        if (!flag_all(owner_type->record_flag(), ERTTRRecordFlag::ScriptVisible)) { return; }
        if (!flag_all(field->flag, ERTTRFieldFlag::ScriptVisible)) { return; }

        auto& field_data = _fields.try_add_default(field->name).value();
        field_data.setup(
            isolate,
            field,
            owner_type
        );
        // clang-format off
    }, { .include_bases = true });
    // clang-format on

    // export static field
    type->each_static_field([&](const RTTRStaticFieldData* static_field, const RTTRType* owner_type) {
        if (!flag_all(owner_type->record_flag(), ERTTRRecordFlag::ScriptVisible)) { return; }
        if (!flag_all(static_field->flag, ERTTRStaticFieldFlag::ScriptVisible)) { return; }

        auto& static_field_data = _static_fields.try_add_default(static_field->name).value();
        static_field_data.setup(
            isolate,
            static_field,
            owner_type
        );
        // clang-format off
    }, { .include_bases = true });
    // clang-format on

    // export methods & properties
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
            _errors.error(u8"getter and setter applied on same method {}", method->name);
            return;
        }

        if (find_getter_result)
        { // getter case
            String prop_name = find_getter_result.ref().cast<skr::attr::ScriptGetter>()->prop_name;
            auto& prop_data = _properties.try_add_default(prop_name).value();
            if (prop_data.getter.is_valid())
            {
                _errors.error(u8"overload is not supported yet for property '{}'", prop_name);
            }
            else
            {
                prop_data.getter.setup(
                    isolate,
                    method,
                    owner_type
                );
            }
        }
        else if (find_setter_result)
        { // setter case
            String prop_name = find_setter_result.ref().cast<skr::attr::ScriptSetter>()->prop_name;
            auto& prop_data = _properties.try_add_default(prop_name).value();
            if (prop_data.setter.is_valid())
            {
                _errors.error(u8"overload is not supported yet for property '{}'", prop_name);
            }
            else
            {
                prop_data.setter.setup(
                    isolate,
                    method,
                    owner_type
                );
            }
        }
        else
        { // normal method
            if (auto found_method = _methods.find(method->name))
            {
                _errors.error(u8"overload is not supported yet for method '{}'", method->name);
            }
            else
            {
                // add method
                auto& method_data = _methods.try_add_default(method->name, found_method).value();
                method_data.setup(
                    isolate,
                    method,
                    owner_type
                );
            }
        }
        // clang-format off
    }, { .include_bases = true });
    // clang-format on

    // export static methods & properties
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
            _errors.error(u8"getter and setter applied on same static method{}", method->name);
            return;
        }

        if (find_getter_result)
        { // getter case
            String prop_name = find_getter_result.ref().cast<skr::attr::ScriptGetter>()->prop_name;
            auto& prop_data = _static_properties.try_add_default(prop_name).value();
            if (prop_data.getter.is_valid())
            {
                _errors.error(u8"overload is not supported yet for static property '{}'", prop_name);
            }
            else
            {
                prop_data.getter.setup(
                    isolate,
                    method,
                    owner_type
                );
            }
        }
        else if (find_setter_result)
        { // setter case
            String prop_name = find_setter_result.ref().cast<skr::attr::ScriptSetter>()->prop_name;
            auto& prop_data = _static_properties.try_add_default(prop_name).value();
            if (prop_data.setter.is_valid())
            {
                _errors.error(u8"overload is not supported yet for static property '{}'", prop_name);
            }
            else
            {
                prop_data.setter.setup(
                    isolate,
                    method,
                    owner_type
                );
            }
        }
        else
        { // normal method
            if (auto found_method = _static_methods.find(method->name))
            {
                _errors.error(u8"overload is not supported yet for static method '{}'", method->name);
            }
            else
            {
                // add method
                auto& method_data = _static_methods.try_add_default(method->name, found_method).value();
                method_data.setup(
                    isolate,
                    method,
                    owner_type
                );
            }
        }
        // clang-format off
    }, { .include_bases = true });
    // clang-format on

    // check properties conflict
    for (auto& [name, prop] : _properties)
    {
        prop.check_conflict();
    }
    for (auto& [name, static_prop] : _static_properties)
    {
        static_prop.check_conflict();
    }
}
void V8BTRecordBase::_fill_template(
    v8::Local<v8::FunctionTemplate> ctor_template
)
{
    using namespace ::v8;

    auto* isolate = Isolate::GetCurrent();

    // setup internal field count
    ctor_template->InstanceTemplate()->SetInternalFieldCount(1);

    // bind method
    for (auto& [method_name, method_binder] : _methods)
    {
        auto v8_template = FunctionTemplate::New(
            isolate,
            _call_method,
            External::New(isolate, &method_binder)
        );
        method_binder.v8_tp.Reset(isolate, v8_template);
        ctor_template->PrototypeTemplate()->Set(
            V8Bind::to_v8(method_name, true),
            v8_template
        );
    }

    // bind static method
    for (auto& [static_method_name, static_method_binder] : _static_methods)
    {
        auto v8_template = FunctionTemplate::New(
            isolate,
            _call_static_method,
            External::New(isolate, &static_method_binder)
        );
        static_method_binder.v8_tp.Reset(isolate, v8_template);
        ctor_template->Set(
            V8Bind::to_v8(static_method_name, true),
            v8_template
        );
    }

    // bind field
    for (auto& [field_name, field_binder] : _fields)
    {
        auto getter_template = FunctionTemplate::New(
            isolate,
            _get_field,
            External::New(isolate, &field_binder)
        );
        auto setter_template = FunctionTemplate::New(
            isolate,
            _set_field,
            External::New(isolate, &field_binder)
        );
        field_binder.v8_tp_getter.Reset(isolate, getter_template);
        field_binder.v8_tp_setter.Reset(isolate, setter_template);
        ctor_template->PrototypeTemplate()->SetAccessorProperty(
            V8Bind::to_v8(field_name, true),
            getter_template,
            setter_template
        );
    }

    // bind static field
    for (auto& [static_field_name, static_field_binder] : _static_fields)
    {
        auto getter_template = FunctionTemplate::New(
            isolate,
            _get_static_field,
            External::New(isolate, &static_field_binder)
        );
        auto setter_template = FunctionTemplate::New(
            isolate,
            _set_static_field,
            External::New(isolate, &static_field_binder)
        );
        static_field_binder.v8_tp_getter.Reset(isolate, getter_template);
        static_field_binder.v8_tp_setter.Reset(isolate, setter_template);
        ctor_template->SetAccessorProperty(
            V8Bind::to_v8(static_field_name, true),
            getter_template,
            setter_template
        );
    }

    // bind properties
    for (auto& [property_name, property_binder] : _properties)
    {
        Local<FunctionTemplate> getter_template = {};
        Local<FunctionTemplate> setter_template = {};

        if (property_binder.getter.is_valid())
        {
            getter_template = FunctionTemplate::New(
                isolate,
                _get_prop,
                External::New(isolate, &property_binder)
            );
            property_binder.getter.v8_tp.Reset(isolate, getter_template);
        }
        if (property_binder.setter.is_valid())
        {
            setter_template = FunctionTemplate::New(
                isolate,
                _set_prop,
                External::New(isolate, &property_binder)
            );
            property_binder.setter.v8_tp.Reset(isolate, setter_template);
        }
        ctor_template->PrototypeTemplate()->SetAccessorProperty(
            V8Bind::to_v8(property_name, true),
            getter_template,
            setter_template
        );
    }

    // bind static properties
    for (auto& [static_property_name, static_property_binder] : _static_properties)
    {
        Local<FunctionTemplate> getter_template = {};
        Local<FunctionTemplate> setter_template = {};

        if (static_property_binder.getter.is_valid())
        {
            getter_template = FunctionTemplate::New(
                isolate,
                _get_static_prop,
                External::New(isolate, &static_property_binder)
            );
            static_property_binder.getter.v8_tp.Reset(isolate, getter_template);
        }
        if (static_property_binder.setter.is_valid())
        {
            setter_template = FunctionTemplate::New(
                isolate,
                _set_static_prop,
                External::New(isolate, &static_property_binder)
            );
            static_property_binder.setter.v8_tp.Reset(isolate, setter_template);
        }
        ctor_template->SetAccessorProperty(
            V8Bind::to_v8(static_property_name, true),
            getter_template,
            setter_template
        );
    }
}
bool V8BTRecordBase::_any_error() const
{
    // test self
    if (_errors.has_error()) return true;

    // test ctor
    if (_ctor.any_error()) return true;

    // test fields
    for (auto& [field_name, field_binder] : _fields)
    {
        if (field_binder.any_error()) return true;
    }

    // test static fields
    for (auto& [static_field_name, static_field_binder] : _static_fields)
    {
        if (static_field_binder.any_error()) return true;
    }

    // test methods
    for (auto& [method_name, method_binder] : _methods)
    {
        if (method_binder.any_error()) return true;
    }

    // test static methods
    for (auto& [static_method_name, static_method_binder] : _static_methods)
    {
        if (static_method_binder.any_error()) return true;
    }

    // test properties
    for (auto& [property_name, property_binder] : _properties)
    {
        if (property_binder.any_error()) return true;
    }

    // test static properties
    for (auto& [static_property_name, static_property_binder] : _static_properties)
    {
        if (static_property_binder.any_error()) return true;
    }

    return false;
}
void V8BTRecordBase::_dump_error(V8ErrorBuilderTreeStyle& builder) const
{
    // dump self error
    if (_errors.has_error())
    {
        builder.write_line(u8"<SelfError>:");
        builder.indent([&]() {
            builder.dump_errors(_errors);
        });
    }

    // dump ctor error
    if (_ctor.any_error())
    {
        builder.write_line(u8"<Ctor>:");
        builder.indent([&]() {
            _ctor.dump_error(builder);
        });
    }

    // dump fields error
    for (const auto& [field_name, field_binder] : _fields)
    {
        if (field_binder.any_error())
        {
            builder.write_line(u8"<Field> {}:", field_name);
            builder.indent([&]() {
                field_binder.dump_error(builder);
            });
        }
    }

    // dump static fields error
    for (const auto& [static_field_name, static_field_binder] : _static_fields)
    {
        if (static_field_binder.any_error())
        {
            builder.write_line(u8"<StaticField> {}:", static_field_name);
            builder.indent([&]() {
                static_field_binder.dump_error(builder);
            });
        }
    }

    // dump methods error
    for (const auto& [method_name, method_binder] : _methods)
    {
        if (method_binder.any_error())
        {
            builder.write_line(u8"<Method> {}:", method_name);
            builder.indent([&]() {
                method_binder.dump_error(builder);
            });
        }
    }

    // dump static methods error
    for (const auto& [static_method_name, static_method_binder] : _static_methods)
    {
        if (static_method_binder.any_error())
        {
            builder.write_line(u8"<StaticMethod> {}:", static_method_name);
            builder.indent([&]() {
                static_method_binder.dump_error(builder);
            });
        }
    }

    // dump properties error
    for (const auto& [property_name, property_binder] : _properties)
    {
        if (property_binder.any_error())
        {
            builder.write_line(u8"<Property> {}:", property_name);
            builder.indent([&]() {
                property_binder.dump_error(builder);
            });
        }
    }

    // dump static properties error
    for (const auto& [static_property_name, static_property_binder] : _static_properties)
    {
        if (static_property_binder.any_error())
        {
            builder.write_line(u8"<StaticProperty> {}:", static_property_name);
            builder.indent([&]() {
                static_property_binder.dump_error(builder);
            });
        }
    }
}
void V8BTRecordBase::_dump_ts_def(TSDefBuilder& builder) const
{
    String record_full_name = String::Build(
        _rttr_type->name_space_str(),
        u8"::",
        _rttr_type->name()
    );

    // fields
    for (auto& [field_name, field_value] : _fields)
    {
        builder.$line(
            u8"// cpp symbol: {}::{}",
            record_full_name,
            field_value.rttr_data->name
        );
        builder.$line(
            u8"{}: {};",
            field_name,
            builder.type_name_with_ns(field_value.bind_tp)
        );
    }

    // methods
    for (auto& [method_name, method_value] : _methods)
    {
        if (method_value.rttr_data_mixin_impl)
        {
            builder.$line(u8"// [MIXIN]");
        }

        builder.$line(
            u8"// cpp symbol: {}::{}",
            record_full_name,
            method_value.rttr_data->name
        );
        builder.$line(
            u8"{}({}): {};",
            method_name,
            builder.params_signature(method_value.params_data),
            builder.return_signature(method_value)
        );
    }

    // static fields
    for (auto& [static_field_name, static_field_value] : _static_fields)
    {
        builder.$line(
            u8"// cpp symbol: {}::{}",
            record_full_name,
            static_field_value.rttr_data->name
        );
        builder.$line(
            u8"static {}: {};",
            static_field_name,
            builder.type_name_with_ns(static_field_value.bind_tp)
        );
    }

    // static methods
    for (auto& [static_method_name, static_method_value] : _static_methods)
    {
        builder.$line(
            u8"// cpp symbol: {}::{}",
            record_full_name,
            static_method_value.rttr_data->name
        );
        builder.$line(
            u8"static {}({}): {};",
            static_method_name,
            builder.params_signature(static_method_value.params_data),
            builder.return_signature(static_method_value)
        );
    }

    // properties
    for (auto& [property_name, property_value] : _properties)
    {
        if (property_value.setter.is_valid())
        {
            builder.$line(
                u8"// cpp symbol: {}::{}",
                record_full_name,
                property_value.setter.rttr_data->name
            );
            builder.$line(
                u8"set {}(value: {});",
                property_name,
                builder.type_name_with_ns(property_value.proxy_bind_tp())
            );
        }
        if (property_value.getter.is_valid())
        {
            builder.$line(
                u8"// cpp symbol: {}::{}",
                record_full_name,
                property_value.getter.rttr_data->name
            );
            builder.$line(
                u8"get {}(): {};",
                property_name,
                builder.type_name_with_ns(property_value.proxy_bind_tp())
            );
        }
    }

    // static properties
    for (auto& [static_property_name, static_property_value] : _static_properties)
    {
        if (static_property_value.setter.is_valid())
        {
            builder.$line(
                u8"// cpp symbol: {}::{}",
                record_full_name,
                static_property_value.setter.rttr_data->name
            );
            builder.$line(
                u8"static set {}(value: {});",
                static_property_name,
                builder.type_name_with_ns(static_property_value.proxy_bind_tp())
            );
        }
        if (static_property_value.getter.is_valid())
        {
            builder.$line(
                u8"// cpp symbol: {}::{}",
                record_full_name,
                static_property_value.getter.rttr_data->name
            );
            builder.$line(
                u8"static get {}(): {};",
                static_property_name,
                builder.type_name_with_ns(static_property_value.proxy_bind_tp())
            );
        }
    }
}

void V8BTRecordBase::_call_method(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // block ctor call
    if (info.IsConstructCall())
    {
        Isolate->ThrowError("method can not be called with new");
        return;
    }

    // get external data
    auto* bind_proxy = get_bind_proxy<V8BPRecord>(info);
    auto* bind_data = get_bind_data<V8BTDataMethod>(info);

    // check bind proxy
    if (!bind_proxy->is_valid())
    {
        Isolate->ThrowError("calling on a released object");
        return;
    }

    // match params
    if (!bind_data->match_param(info))
    {
        Isolate->ThrowError("params mismatch");
        return;
    }

    // invoke method
    bind_data->call(
        info,
        bind_proxy->address,
        bind_proxy->rttr_type
    );
}
void V8BTRecordBase::_call_static_method(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // block ctor call
    if (info.IsConstructCall())
    {
        Isolate->ThrowError("method can not be called with new");
        return;
    }

    // get external data
    auto* bind_data = get_bind_data<V8BTDataStaticMethod>(info);

    // match params
    if (!bind_data->match(info))
    {
        Isolate->ThrowError("params mismatch");
        return;
    }

    // invoke method
    bind_data->call(
        info
    );
}
void V8BTRecordBase::_get_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // get external data
    auto* bind_proxy = get_bind_proxy<V8BPRecord>(info);
    auto* bind_data = get_bind_data<V8BTDataField>(info);

    // check bind proxy
    if (!bind_proxy->is_valid())
    {
        Isolate->ThrowError("calling on a released object");
        return;
    }

    // get field
    auto v8_field = bind_data->bind_tp->get_field(
        bind_proxy->address,
        bind_proxy->rttr_type,
        *bind_data
    );
    info.GetReturnValue().Set(v8_field);
}
void V8BTRecordBase::_set_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // get external data
    auto* bind_proxy = get_bind_proxy<V8BPRecord>(info);
    auto* bind_data = get_bind_data<V8BTDataField>(info);

    // check bind proxy
    if (!bind_proxy->is_valid())
    {
        Isolate->ThrowError("calling on a released object");
        return;
    }

    // set field
    bind_data->bind_tp->set_field(
        info[0],
        bind_proxy->address,
        bind_proxy->rttr_type,
        *bind_data
    );
}
void V8BTRecordBase::_get_static_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // get external data
    auto* bind_data = get_bind_data<V8BTDataStaticField>(info);

    // get field
    auto v8_field = bind_data->bind_tp->get_static_field(
        *bind_data
    );
    info.GetReturnValue().Set(v8_field);
}
void V8BTRecordBase::_set_static_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // get external data
    auto* bind_data = get_bind_data<V8BTDataStaticField>(info);

    // set field
    bind_data->bind_tp->set_static_field(
        info[0],
        *bind_data
    );
}
void V8BTRecordBase::_get_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // get external data
    auto* bind_proxy = get_bind_proxy<V8BPRecord>(info);
    auto* bind_data = get_bind_data<V8BTDataProperty>(info);

    // check bind proxy
    if (!bind_proxy->is_valid())
    {
        Isolate->ThrowError("calling on a released object");
        return;
    }

    // match property
    if (!bind_data->getter.match_param(info))
    {
        Isolate->ThrowError("property getter params mismatch");
        return;
    }

    // invoke getter
    bind_data->getter.call(
        info,
        bind_proxy->address,
        bind_proxy->rttr_type
    );
}
void V8BTRecordBase::_set_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // get external data
    auto* bind_proxy = get_bind_proxy<V8BPRecord>(info);
    auto* bind_data = get_bind_data<V8BTDataProperty>(info);

    // check bind proxy
    if (!bind_proxy->is_valid())
    {
        Isolate->ThrowError("calling on a released object");
        return;
    }

    // match
    if (!bind_data->setter.match_param(info))
    {
        Isolate->ThrowError("property setter params mismatch");
        return;
    }

    // invoke setter
    bind_data->setter.call(
        info,
        bind_proxy->address,
        bind_proxy->rttr_type
    );
}
void V8BTRecordBase::_get_static_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // get external data
    auto* bind_data = get_bind_data<V8BTDataStaticProperty>(info);

    // match
    if (!bind_data->getter.match(info))
    {
        Isolate->ThrowError("property getter params mismatch");
        return;
    }

    // invoke getter
    bind_data->getter.call(
        info
    );
}
void V8BTRecordBase::_set_static_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // get external data
    auto* bind_data = get_bind_data<V8BTDataStaticProperty>(info);

    // match
    if (!bind_data->setter.match(info))
    {
        Isolate->ThrowError("property setter params mismatch");
        return;
    }

    // invoke setter
    bind_data->setter.call(
        info
    );
}
} // namespace skr