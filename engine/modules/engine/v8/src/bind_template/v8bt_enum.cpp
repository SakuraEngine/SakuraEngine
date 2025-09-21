#include <SkrV8/bind_template/v8bt_enum.hpp>
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
V8BTEnum* V8BTEnum::TryCreate(V8Isolate* isolate, const RTTRType* type)
{
    SKR_ASSERT(type->is_enum());

    auto* underlying = isolate->solve_bind_tp_as<V8BTPrimitive>(
        type->enum_underlying_type_id()
    );
    if (!underlying)
    {

        SKR_LOG_FMT_ERROR(u8"enum '{}' underlying type not supported", type->name());
        return nullptr;
    }

    V8BTEnum* result = SkrNew<V8BTEnum>();
    result->set_isolate(isolate);
    result->_rttr_type = type;
    result->_underlying = underlying;

    // export items
    type->each_enum_items([&](const RTTREnumItemData* item) {
        // filter by flag
        if (!flag_all(item->flag, ERTTREnumItemFlag::ScriptVisible)) { return; }

        result->_items.add(item->name, item);
        result->_is_signed = item->value.is_signed();
    });

    // make template
    result->_make_template();

    return result;
}

// kind
EV8BTKind V8BTEnum::kind() const
{
    return EV8BTKind::Enum;
}
String V8BTEnum::type_name() const
{
    return _rttr_type->name();
}
String V8BTEnum::cpp_namespace() const
{
    return _rttr_type->name_space_str();
}

// error process
bool V8BTEnum::any_error() const
{
    return false;
}
void V8BTEnum::dump_error(V8ErrorBuilderTreeStyle& builder) const
{
    SKR_UNREACHABLE_CODE();
}

v8::Local<v8::Value> V8BTEnum::to_v8(
    void* native_data
) const
{
    return _underlying->to_v8(native_data);
}
bool V8BTEnum::to_native(
    void* native_data,
    v8::Local<v8::Value> v8_value,
    bool is_init
) const
{
    return _underlying->to_native(native_data, v8_value, is_init);
}

// invoke native api
bool V8BTEnum::match_param(
    v8::Local<v8::Value> v8_param
) const
{
    return _underlying->match_param(v8_param);
}
void V8BTEnum::push_param_native(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp,
    v8::Local<v8::Value> v8_value
) const
{
    _underlying->push_param_native(
        stack,
        param_bind_tp,
        v8_value
    );
}
void V8BTEnum::push_param_native_pure_out(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp
) const
{
    _underlying->push_param_native_pure_out(
        stack,
        param_bind_tp
    );
}
v8::Local<v8::Value> V8BTEnum::read_return_native(
    DynamicStack& stack,
    const V8BTDataReturn& return_bind_tp
) const
{
    return _underlying->read_return_native(
        stack,
        return_bind_tp
    );
}
v8::Local<v8::Value> V8BTEnum::read_return_from_out_param(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp
) const
{
    return _underlying->read_return_from_out_param(
        stack,
        param_bind_tp
    );
}

// invoke v8 api
v8::Local<v8::Value> V8BTEnum::make_param_v8(
    void* native_data,
    const V8BTDataParam& param_bind_tp
) const
{
    return _underlying->make_param_v8(
        native_data,
        param_bind_tp
    );
}

// field api
v8::Local<v8::Value> V8BTEnum::get_field(
    void* obj,
    const RTTRType* obj_type,
    const V8BTDataField& field_bind_tp
) const
{
    return _underlying->get_field(
        obj,
        obj_type,
        field_bind_tp
    );
}
void V8BTEnum::set_field(
    v8::Local<v8::Value> v8_value,
    void* obj,
    const RTTRType* obj_type,
    const V8BTDataField& field_bind_tp
) const
{
    _underlying->set_field(
        v8_value,
        obj,
        obj_type,
        field_bind_tp
    );
}
v8::Local<v8::Value> V8BTEnum::get_static_field(
    const V8BTDataStaticField& field_bind_tp
) const
{
    return _underlying->get_static_field(field_bind_tp);
}
void V8BTEnum::set_static_field(
    v8::Local<v8::Value> v8_value,
    const V8BTDataStaticField& field_bind_tp
) const
{
    _underlying->set_static_field(
        v8_value,
        field_bind_tp
    );
}

// check api
void V8BTEnum::solve_invoke_behaviour(
    const V8BTDataParam& param_bind_tp,
    bool& appare_in_return,
    bool& appare_in_param
) const
{
    _underlying->solve_invoke_behaviour(
        param_bind_tp,
        appare_in_return,
        appare_in_param
    );
}
bool V8BTEnum::check_param(
    const V8BTDataParam& param_bind_tp,
    V8ErrorCache& errors
) const
{
    if (!_basic_type_check(param_bind_tp.modifiers, errors))
    {
        return false;
    }
    return _underlying->check_param(param_bind_tp, errors);
}
bool V8BTEnum::check_return(
    const V8BTDataReturn& return_bind_tp,
    V8ErrorCache& errors
) const
{
    if (!_basic_type_check(return_bind_tp.modifiers, errors))
    {
        return false;
    }
    return _underlying->check_return(return_bind_tp, errors);
}
bool V8BTEnum::check_field(
    const V8BTDataField& field_bind_tp,
    V8ErrorCache& errors
) const
{
    if (field_bind_tp.modifiers.is_decayed_pointer())
    {
        errors.error(
            u8"enum cannot be exported as decayed pointer type in field"
        );
        return false;
    }
    return _underlying->check_field(field_bind_tp, errors);
}
bool V8BTEnum::check_static_field(
    const V8BTDataStaticField& field_bind_tp,
    V8ErrorCache& errors
) const
{
    if (field_bind_tp.modifiers.is_decayed_pointer())
    {
        errors.error(
            u8"enum cannot be exported as decayed pointer type in field"
        );
        return false;
    }
    return _underlying->check_static_field(field_bind_tp, errors);
}

// v8 export
bool V8BTEnum::has_v8_export_obj() const
{
    return true;
}
v8::Local<v8::Value> V8BTEnum::get_v8_export_obj() const
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    return _v8_template.Get(isolate)->NewInstance(context).ToLocalChecked();
}
void V8BTEnum::dump_ts_def(
    TSDefBuilder& builder
) const
{
    // print cpp symbol
    builder.$line(
        u8"// cpp symbol: {}::{}",
        _rttr_type->name_space_str(),
        _rttr_type->name()
    );

    // enum body
    builder.$line(u8"export enum {} {{", _rttr_type->name());
    builder.$indent([&] {
        // enum values
        for (auto& [item_name, item_value] : _items)
        {
            if (item_value->value.is_signed())
            {
                builder.$line(u8"{} = {},", item_name, item_value->value.value_signed());
            }
            else
            {
                builder.$line(u8"{} = {},", item_name, item_value->value.value_unsigned());
            }
        }
    });
    builder.$line(u8"}}");

    // to_string & from_string
    builder.$line(u8"export namespace {} {{", _rttr_type->name());
    builder.$indent([&] {
        // to_string
        builder.$line(u8"export function to_string(value: {}): string", _rttr_type->name());

        // from_string
        builder.$line(u8"export function from_string(value: string): {}", _rttr_type->name());
    });
    builder.$line(u8"}}");
}
String V8BTEnum::get_ts_type_name() const
{
    return _rttr_type->name();
}
bool V8BTEnum::ts_is_nullable() const
{
    return false;
}

void V8BTEnum::_enum_to_string(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // get user data
    auto* bind_tp = reinterpret_cast<V8BTEnum*>(info.Data().As<External>()->Value());

    // check value
    if (info.Length() != 1)
    {
        Isolate->ThrowError("enum to_string need 1 argument");
        return;
    }

    // get value
    int64_t enum_singed;
    uint64_t enum_unsigned;
    if (bind_tp->_is_signed)
    {
        if (!V8Bind::to_native(info[0], enum_singed))
        {
            Isolate->ThrowError("invalid enum value");
            return;
        }
    }
    else
    {
        if (!V8Bind::to_native(info[0], enum_unsigned))
        {
            Isolate->ThrowError("invalid enum value");
            return;
        }
    }

    // find value
    for (const auto& [enum_item_name, enum_item] : bind_tp->_items)
    {
        if (enum_item->value.is_signed())
        {
            int64_t v = enum_item->value.value_signed();
            if (v == enum_singed)
            {
                info.GetReturnValue().Set(V8Bind::to_v8(enum_item_name, true));
                return;
            }
        }
        else
        {
            uint64_t v = enum_item->value.value_unsigned();
            if (v == enum_unsigned)
            {
                info.GetReturnValue().Set(V8Bind::to_v8(enum_item_name, true));
                return;
            }
        }
    }

    Isolate->ThrowError("no matched enum item");
}
void V8BTEnum::_enum_from_string(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // get user data
    auto* bind_tp = reinterpret_cast<V8BTEnum*>(info.Data().As<External>()->Value());

    // check value
    if (info.Length() != 1)
    {
        Isolate->ThrowError("enum to_string need 1 argument");
        return;
    }

    // get item name
    String enum_item_name;
    if (!V8Bind::to_native(info[0], enum_item_name))
    {
        Isolate->ThrowError("invalid enum item name");
        return;
    }

    // find value
    if (auto result = bind_tp->_items.find(enum_item_name))
    {
        info.GetReturnValue().Set(V8Bind::to_v8(result.value()->value));
    }
    else
    {
        Isolate->ThrowError("no matched enum item");
    }
}
bool V8BTEnum::_basic_type_check(const V8BTDataModifier& modifiers, V8ErrorCache& errors) const
{
    if (modifiers.is_pointer)
    {
        errors.error(
            u8"export enum {} as pointer type",
            _rttr_type->name()
        );
        return false;
    }
    return true;
}
void V8BTEnum::_make_template()
{
    using namespace ::v8;
    auto* isolate = Isolate::GetCurrent();

    HandleScope handle_scope(isolate);

    auto tp = ObjectTemplate::New(isolate);

    // add enum items
    for (const auto& [enum_item_name, enum_item] : _items)
    {
        // get value
        Local<Value> enum_value = V8Bind::to_v8(enum_item->value);

        // set value
        tp->Set(
            V8Bind::to_v8(enum_item->name, true),
            enum_value
        );
    }

    // add convert functions
    tp->Set(
        V8Bind::to_v8(u8"to_string", true),
        FunctionTemplate::New(
            isolate,
            _enum_to_string,
            External::New(isolate, this)
        )
    );
    tp->Set(
        V8Bind::to_v8(u8"from_string", true),
        FunctionTemplate::New(
            isolate,
            _enum_from_string,
            External::New(isolate, this)
        )
    );

    _v8_template.Reset(isolate, tp);
}
} // namespace skr