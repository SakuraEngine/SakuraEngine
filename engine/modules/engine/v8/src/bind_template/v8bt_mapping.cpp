#include <SkrV8/bind_template/v8bt_mapping.hpp>
#include <SkrV8/v8_bind.hpp>
#include <SkrV8/bind_template/v8bt_primitive.hpp>
#include <SkrV8/v8_isolate.hpp>
#include <SkrV8/ts_def_builder.hpp>

// v8 includes
#include <libplatform/libplatform.h>
#include <v8-initialization.h>
#include <v8-template.h>
#include <v8-external.h>
#include <v8-function.h>
#include <v8-container.h>

namespace skr
{
V8BTMapping* V8BTMapping::TryCreate(V8Isolate* isolate, const RTTRType* type)
{
    SKR_ASSERT(type->is_record());

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

    auto* result = SkrNew<V8BTMapping>();
    result->set_isolate(isolate);
    result->_rttr_type = type;
    result->_default_ctor = type->find_default_ctor();

    // each field
    type->each_field([&](const RTTRFieldData* field, const RTTRType* owner_type) {
        if (!flag_all(field->flag, ERTTRFieldFlag::ScriptVisible)) { return; }
        auto& field_data = result->_fields.try_add_default(field->name).value();
        field_data.setup(isolate, field, owner_type);
    });

    result->_make_template();
    return result;
}

// kinds
EV8BTKind V8BTMapping::kind() const
{
    return EV8BTKind::Mapping;
}
String V8BTMapping::type_name() const
{
    return _rttr_type->name();
}
String V8BTMapping::cpp_namespace() const
{
    return _rttr_type->name_space_str();
}

// error process
bool V8BTMapping::any_error() const
{
    for (const auto& [field_name, field_data] : _fields)
    {
        if (field_data.any_error()) { return true; }
    }
    return false;
}
void V8BTMapping::dump_error(V8ErrorBuilderTreeStyle& builder) const
{
    builder.write_line(u8"{} <Mapping>", _rttr_type->name());
    builder.indent([&]() {
        for (const auto& [field_name, field_data] : _fields)
        {
            if (field_data.any_error())
            {
                builder.write_line(u8"<Field> {}:", field_name);
                builder.indent([&]() {
                    field_data.dump_error(builder);
                });
            }
        }
    });
}

v8::Local<v8::Value> V8BTMapping::to_v8(
    void* native_data
) const
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    auto result = v8::Object::New(isolate);
    for (const auto& [field_name, field_data] : _fields)
    {
        // conv field to v8
        void* field_addr = field_data.solve_address(native_data, _rttr_type);
        auto v8_field_value = field_data.bind_tp->to_v8(field_addr);

        // set object field
        // clang-format off
        result->Set(
            context,
            V8Bind::to_v8(field_name, true),
            v8_field_value
        ).Check();
        // clang-format on
    }
    return result;
}
bool V8BTMapping::to_native(
    void* native_data,
    v8::Local<v8::Value> v8_value,
    bool is_init
) const
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    auto v8_object = v8_value->ToObject(context).ToLocalChecked();

    // do init
    if (!is_init)
    {
        _init_native(native_data);
    }

    // build fields
    for (const auto& [field_name, field_data] : _fields)
    {
        // clang-format off
        // find object field
        auto v8_field = v8_object->Get(
            context,
            V8Bind::to_v8(field_name, true)
        ).ToLocalChecked();
        // clang-format on

        // set field
        void* field_addr = field_data.solve_address(native_data, _rttr_type);
        field_data.bind_tp->to_native(
            field_addr,
            v8_field,
            true
        );
    }
    return true;
}

// invoke native api
bool V8BTMapping::match_param(
    v8::Local<v8::Value> v8_param
) const
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    // conv to object
    if (!v8_param->IsObject()) { return false; }
    auto v8_obj = v8_param->ToObject(context).ToLocalChecked();

    // match fields
    for (const auto& [field_name, field_data] : _fields)
    {
        // find object field
        auto v8_field_value_maybe = v8_obj->Get(
            context,
            V8Bind::to_v8(field_name, true)
        );
        // clang-format on

        // get field value
        if (v8_field_value_maybe.IsEmpty()) { return {}; }
        auto v8_field_value = v8_field_value_maybe.ToLocalChecked();

        // match field
        auto result = field_data.bind_tp->match_param(v8_field_value);
        if (!result)
        {
            return false;
        }
    }

    return true;
}
void V8BTMapping::push_param_native(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp,
    v8::Local<v8::Value> v8_value
) const
{
    DtorInvoker dtor = _rttr_type->dtor_invoker();
    void* native_data = stack.alloc_param_raw(
        _rttr_type->size(),
        _rttr_type->alignment(),
        param_bind_tp.modifiers.is_decayed_pointer() ? EDynamicStackParamKind::XValue : EDynamicStackParamKind::Direct,
        dtor
    );
    to_native(native_data, v8_value, false);
}
void V8BTMapping::push_param_native_pure_out(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp
) const
{
    DtorInvoker dtor = _rttr_type->dtor_invoker();
    void* native_data = stack.alloc_param_raw(
        _rttr_type->size(),
        _rttr_type->alignment(),
        EDynamicStackParamKind::XValue,
        dtor
    );
    _init_native(native_data);
}
v8::Local<v8::Value> V8BTMapping::read_return_native(
    DynamicStack& stack,
    const V8BTDataReturn& return_bind_tp
) const
{
    if (!stack.is_return_stored()) { return {}; }

    void* native_data = stack.get_return_raw();
    if (return_bind_tp.pass_by_ref)
    {
        native_data = *reinterpret_cast<void**>(native_data);
    }
    return to_v8(native_data);
}
v8::Local<v8::Value> V8BTMapping::read_return_from_out_param(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp
) const
{
    // get out param data (when pass by ref, we always push xvalue into dynamic stack)
    void* native_data = stack.get_param_raw(param_bind_tp.index);
    return to_v8(native_data);
}

// invoke v8 api
v8::Local<v8::Value> V8BTMapping::make_param_v8(
    void* native_data,
    const V8BTDataParam& param_bind_tp
) const
{
    return to_v8(native_data);
}

// field api
v8::Local<v8::Value> V8BTMapping::get_field(
    void* obj,
    const RTTRType* obj_type,
    const V8BTDataField& field_bind_tp
) const
{
    void* field_addr = field_bind_tp.solve_address(obj, obj_type);
    return to_v8(field_addr);
}
void V8BTMapping::set_field(
    v8::Local<v8::Value> v8_value,
    void* obj,
    const RTTRType* obj_type,
    const V8BTDataField& field_bind_tp
) const
{
    void* field_addr = field_bind_tp.solve_address(obj, obj_type);
    to_native(field_addr, v8_value, false);
}
v8::Local<v8::Value> V8BTMapping::get_static_field(
    const V8BTDataStaticField& field_bind_tp
) const
{
    void* field_addr = field_bind_tp.solve_address();
    return to_v8(field_addr);
}
void V8BTMapping::set_static_field(
    v8::Local<v8::Value> v8_value,
    const V8BTDataStaticField& field_bind_tp
) const
{
    void* field_addr = field_bind_tp.solve_address();
    to_native(field_addr, v8_value, false);
}
// check api
void V8BTMapping::solve_invoke_behaviour(
    const V8BTDataParam& param_bind_tp,
    bool& appare_in_return,
    bool& appare_in_param
) const
{
    switch (param_bind_tp.inout_flag)
    {
    case ERTTRParamFlag::Out:
        appare_in_param = false;
        appare_in_return = true;
        break;
    case ERTTRParamFlag::InOut:
        appare_in_param = true;
        appare_in_return = true;
        break;
    case ERTTRParamFlag::In:
    default:
        appare_in_param = true;
        appare_in_return = false;
        break;
    }
}
bool V8BTMapping::check_param(
    const V8BTDataParam& param_bind_tp,
    V8ErrorCache& errors
) const
{

    return _basic_type_check(param_bind_tp.modifiers, errors);
}
bool V8BTMapping::check_return(
    const V8BTDataReturn& return_bind_tp,
    V8ErrorCache& errors
) const
{
    return _basic_type_check(return_bind_tp.modifiers, errors);
}
bool V8BTMapping::check_field(
    const V8BTDataField& field_bind_tp,
    V8ErrorCache& errors
) const
{
    if (field_bind_tp.modifiers.is_decayed_pointer())
    {
        errors.error(
            u8"mapping cannot be exported as decayed pointer type in field"
        );
        return false;
    }
    return _basic_type_check(field_bind_tp.modifiers, errors);
}
bool V8BTMapping::check_static_field(
    const V8BTDataStaticField& field_bind_tp,
    V8ErrorCache& errors
) const
{
    if (field_bind_tp.modifiers.is_decayed_pointer())
    {
        errors.error(
            u8"mapping cannot be exported as decayed pointer type in field"
        );
        return false;
    }
    return _basic_type_check(field_bind_tp.modifiers, errors);
}

// v8 export
bool V8BTMapping::has_v8_export_obj() const
{
    return true;
}
v8::Local<v8::Value> V8BTMapping::get_v8_export_obj() const
{
    using namespace ::v8;

    auto isolate = Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();
    return _v8_template.Get(isolate)->GetFunction(context).ToLocalChecked();
}
void V8BTMapping::dump_ts_def(
    TSDefBuilder& builder
) const
{
    builder.$line(u8"// export as mapping");
    // print cpp symbol
    builder.$line(
        u8"// cpp symbol: {}::{}",
        _rttr_type->name_space_str(),
        _rttr_type->name()
    );

    // dump as interface
    builder.$line(u8"export interface {} {{", _rttr_type->name());
    builder.$indent([&] {
        // fields
        for (auto& [field_name, field_value] : _fields)
        {
            builder.$line(
                u8"{}: {};",
                field_name,
                builder.type_name_with_ns(field_value.bind_tp)
            );
        }
    });
    builder.$line(u8"}}");
}
String V8BTMapping::get_ts_type_name() const
{
    return _rttr_type->name();
}
bool V8BTMapping::ts_is_nullable() const
{
    return false;
}

void V8BTMapping::_init_native(
    void* native_data
) const
{
    _default_ctor.invoke(native_data);
}
bool V8BTMapping::_basic_type_check(
    const V8BTDataModifier& modifiers,
    V8ErrorCache& errors
) const
{
    if (modifiers.is_pointer)
    {
        errors.error(
            u8"export mapping {} as pointer type",
            _rttr_type->name()
        );
        return false;
    }
    return true;
}
void V8BTMapping::_make_template()
{
    using namespace ::v8;
    auto* isolate = Isolate::GetCurrent();

    HandleScope handle_scope(isolate);

    auto tp = FunctionTemplate::New(
        isolate,
        +[](const ::v8::FunctionCallbackInfo<::v8::Value>& info) {
            info.GetIsolate()->ThrowError("mappint object cannot be constructed with new");
        },
        External::New(isolate, this)
    );

    _v8_template.Reset(isolate, tp);
}
} // namespace skr