#include <SkrV8/bind_template/v8bt_value.hpp>
#include <SkrV8/v8_bind.hpp>
#include <SkrV8/v8_bind_proxy.hpp>
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
V8BTValue* V8BTValue::TryCreate(V8Isolate* isolate, const RTTRType* type)
{
    V8BTValue* result = SkrNew<V8BTValue>();
    result->_setup(isolate, type);
    result->_make_template();
    result->_copy_ctor = type->find_copy_ctor();
    result->_assign = type->find_assign();
    return result;
}

// kind
EV8BTKind V8BTValue::kind() const
{
    return EV8BTKind::Value;
}
String V8BTValue::type_name() const
{
    return _rttr_type->name();
}
String V8BTValue::cpp_namespace() const
{
    return _rttr_type->name_space_str();
}

// error process
bool V8BTValue::any_error() const
{
    return _any_error();
}
void V8BTValue::dump_error(V8ErrorBuilderTreeStyle& builder) const
{
    builder.write_line(u8"{} <Value>", _rttr_type->name());
    builder.indent([&]() {
        _dump_error(builder);
    });
}

// convert helper
v8::Local<v8::Value> V8BTValue::to_v8(
    void* native_data
) const
{
    using namespace ::v8;
    Isolate* isolate = Isolate::GetCurrent();

    auto* bind_core = _create_value(native_data);
    return bind_core->v8_object.Get(isolate);
}
bool V8BTValue::to_native(
    void* native_data,
    v8::Local<v8::Value> v8_value,
    bool is_init
) const
{
    using namespace ::v8;
    auto isolate = Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    auto v8_object = v8_value->ToObject(context).ToLocalChecked();
    auto* bind_proxy = get_bind_proxy<V8BPValue>(v8_object);

    // check bind proxy
    if (!bind_proxy->is_valid())
    {
        isolate->ThrowError("calling on a released object");
        return false;
    }

    // do cast
    void* casted_ptr = bind_proxy->rttr_type->cast_to_base(_rttr_type->type_id(), bind_proxy->address);

    // assign or copy ctor
    if (is_init)
    {
        if (!_assign)
        {
            isolate->ThrowError("lost assign operator");
            return false;
        }
        _assign.invoke(native_data, casted_ptr);
    }
    else
    {
        if (!_copy_ctor)
        {
            isolate->ThrowError("lost copy ctor");
            return false;
        }
        _copy_ctor.invoke(native_data, casted_ptr);
    }
    return true;
}

// invoke native api
bool V8BTValue::match_param(
    v8::Local<v8::Value> v8_param
) const
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    // check v8 value type
    if (!v8_param->IsObject()) { return {}; }
    auto v8_object = v8_param->ToObject(context).ToLocalChecked();

    // check object
    if (v8_object->InternalFieldCount() < 1) { return {}; }
    auto* bind_proxy = get_bind_proxy<V8BPValue>(v8_object);
    if (!bind_proxy->is_valid()) { return {}; }

    // check inherit
    bool base_on = bind_proxy->rttr_type->based_on(
        _rttr_type->type_id()
    );
    return base_on;
}
void V8BTValue::push_param_native(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp,
    v8::Local<v8::Value> v8_value
) const
{
    void* native_data;
    if (param_bind_tp.modifiers.is_decayed_pointer())
    { // optimize for ref case
        auto* isolate = v8::Isolate::GetCurrent();
        auto context = isolate->GetCurrentContext();

        // get bind core
        auto v8_object = v8_value->ToObject(context).ToLocalChecked();
        auto* bind_proxy = get_bind_proxy<V8BPValue>(v8_object);

        // check bind core
        if (!bind_proxy->is_valid())
        {
            isolate->ThrowError("calling on a released object");
            return;
        }

        // push pointer
        auto* cast_ptr = bind_proxy->rttr_type->cast_to_base(
            bind_proxy->rttr_type->type_id(),
            bind_proxy->address
        );
        stack.add_param<void*>(cast_ptr);
    }
    else
    {
        DtorInvoker dtor = _rttr_type->dtor_invoker();
        native_data = stack.alloc_param_raw(
            _rttr_type->size(),
            _rttr_type->alignment(),
            EDynamicStackParamKind::Direct,
            dtor
        );
        to_native(native_data, v8_value, false);
    }
}
void V8BTValue::push_param_native_pure_out(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp
) const
{
    auto* new_value_core = _create_value(nullptr);
    stack.add_param<void*>(new_value_core->address);
}
v8::Local<v8::Value> V8BTValue::read_return_native(
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
v8::Local<v8::Value> V8BTValue::read_return_from_out_param(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp
) const
{
    // get out param param
    void* native_data = stack.get_param_raw(param_bind_tp.index);

    // get created value proxy
    native_data = *reinterpret_cast<void**>(native_data);
    auto bind_proxy = isolate()->map_bind_proxy(native_data);
    auto* bind_proxy_value = static_cast<V8BPValue*>(bind_proxy);
    return bind_proxy_value->v8_object.Get(v8::Isolate::GetCurrent());
}

// invoke v8 api
v8::Local<v8::Value> V8BTValue::make_param_v8(
    void* native_data,
    const V8BTDataParam& param_bind_tp
) const
{
    // make bind proxy
    auto* bind_proxy = _create_value(native_data);
    bind_proxy->address = native_data;
    bind_proxy->need_delete = false;

    // push param cache
    isolate()->push_param_proxy(bind_proxy);

    return bind_proxy->v8_object.Get(v8::Isolate::GetCurrent());
}

// field api
v8::Local<v8::Value> V8BTValue::get_field(
    void* obj,
    const RTTRType* obj_type,
    const V8BTDataField& field_bind_tp
) const
{
    // find cache
    void* field_address = field_bind_tp.solve_address(obj, obj_type);
    if (auto found = isolate()->map_bind_proxy(field_address))
    {
        auto* bind_proxy = static_cast<V8BPValue*>(found);
        return bind_proxy->v8_object.Get(v8::Isolate::GetCurrent());
    }

    // make bind proxy
    auto* bind_proxy = _new_bind_proxy(field_address);
    bind_proxy->need_delete = false;

    // setup owner ship
    auto* parent_bind_proxy = isolate()->map_bind_proxy(obj);
    bind_proxy->set_parent(parent_bind_proxy);

    return bind_proxy->v8_object.Get(v8::Isolate::GetCurrent());
}
void V8BTValue::set_field(
    v8::Local<v8::Value> v8_value,
    void* obj,
    const RTTRType* obj_type,
    const V8BTDataField& field_bind_tp
) const
{
    to_native(
        field_bind_tp.solve_address(obj, obj_type),
        v8_value,
        true
    );
}
v8::Local<v8::Value> V8BTValue::get_static_field(
    const V8BTDataStaticField& field_bind_tp
) const
{
    // find cache
    void* field_address = field_bind_tp.solve_address();
    if (auto found = isolate()->map_bind_proxy(field_address))
    {
        auto* bind_proxy = static_cast<V8BPValue*>(found);
        return bind_proxy->v8_object.Get(v8::Isolate::GetCurrent());
    }

    // make bind proxy
    auto* bind_proxy = _new_bind_proxy(field_address);
    bind_proxy->need_delete = false;
    return bind_proxy->v8_object.Get(v8::Isolate::GetCurrent());
}
void V8BTValue::set_static_field(
    v8::Local<v8::Value> v8_value,
    const V8BTDataStaticField& field_bind_tp
) const
{
    to_native(
        field_bind_tp.solve_address(),
        v8_value,
        true
    );
}

// check api
void V8BTValue::solve_invoke_behaviour(
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
        appare_in_return = false; // optimized for value type
        break;
    case ERTTRParamFlag::In:
    default:
        appare_in_param = true;
        appare_in_return = false;
        break;
    }
}
bool V8BTValue::check_param(
    const V8BTDataParam& param_bind_tp,
    V8ErrorCache& errors
) const
{
    return _basic_type_check(param_bind_tp.modifiers, errors);
}
bool V8BTValue::check_return(
    const V8BTDataReturn& return_bind_tp,
    V8ErrorCache& errors
) const
{
    return _basic_type_check(return_bind_tp.modifiers, errors);
}
bool V8BTValue::check_field(
    const V8BTDataField& field_bind_tp,
    V8ErrorCache& errors
) const
{
    if (field_bind_tp.modifiers.is_decayed_pointer())
    {
        errors.error(
            u8"value cannot be exported as decayed pointer type in field"
        );
        return false;
    }
    return _basic_type_check(field_bind_tp.modifiers, errors);
}
bool V8BTValue::check_static_field(
    const V8BTDataStaticField& field_bind_tp,
    V8ErrorCache& errors
) const
{
    if (field_bind_tp.modifiers.is_decayed_pointer())
    {
        errors.error(
            u8"value cannot be exported as decayed pointer type in field"
        );
        return false;
    }
    return _basic_type_check(field_bind_tp.modifiers, errors);
}

// v8 export
bool V8BTValue::has_v8_export_obj() const
{
    return true;
}
v8::Local<v8::Value> V8BTValue::get_v8_export_obj() const
{
    using namespace ::v8;

    auto isolate = Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    return _v8_template.Get(isolate)->GetFunction(context).ToLocalChecked();
}
void V8BTValue::dump_ts_def(
    TSDefBuilder& builder
) const
{
    builder.$line(u8"// export as value");

    // print cpp symbol
    builder.$line(
        u8"// cpp symbol: {}::{}",
        _rttr_type->name_space_str(),
        _rttr_type->name()
    );

    // body
    builder.$line(u8"export class {} {{", _rttr_type->name());
    builder.$indent([&] {
        // ctors
        if (_ctor.is_valid())
        {
            builder.$line(
                u8"// cpp symbol: {}::{}",
                _rttr_type->name_space_str(),
                _rttr_type->name()
            );
            builder.$line(
                u8"constructor({});",
                builder.params_signature(_ctor.params_data)
            );
        }

        _dump_ts_def(builder);
    });
    builder.$line(u8"}}");
}
String V8BTValue::get_ts_type_name() const
{
    return _rttr_type->name();
}
bool V8BTValue::ts_is_nullable() const
{
    return false;
}

// helper
V8BPValue* V8BTValue::_create_value(void* native_data) const
{
    using namespace ::v8;

    auto* isolate = Isolate::GetCurrent();
    HandleScope handle_scope(isolate);

    // construct value
    void* alloc_mem = _rttr_type->alloc();
    if (native_data)
    {
        // copy data
        _copy_ctor.invoke(alloc_mem, native_data);
    }
    else
    {
        // init data
        _default_ctor.invoke(alloc_mem);
    }

    // make bind proxy
    auto* bind_proxy = _new_bind_proxy(alloc_mem);
    bind_proxy->need_delete = true;

    return bind_proxy;
}
V8BPValue* V8BTValue::_new_bind_proxy(void* address, v8::Local<v8::Object> self) const
{
    using namespace ::v8;

    auto* isolate = Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    // make v8 object
    Local<ObjectTemplate> instance_template = _v8_template.Get(isolate)->InstanceTemplate();
    Local<v8::Object> object =
        self.IsEmpty() ?
        instance_template->NewInstance(context).ToLocalChecked() :
        self;

    // make bind core
    V8BPValue* bind_proxy = this->isolate()->create_bind_proxy<V8BPValue>();
    bind_proxy->rttr_type = _rttr_type;
    bind_proxy->isolate = this->isolate();
    bind_proxy->bind_tp = this;
    bind_proxy->address = address;
    bind_proxy->v8_object.Reset(isolate, object);

    // setup gc callback
    bind_proxy->v8_object.SetWeak(
        bind_proxy,
        _gc_callback,
        WeakCallbackType::kInternalFields
    );

    // set internal field
    object->SetInternalField(0, External::New(isolate, bind_proxy));

    // register bind proxy
    this->isolate()->register_bind_proxy(address, bind_proxy);

    return bind_proxy;
}
bool V8BTValue::_basic_type_check(const V8BTDataModifier& modifiers, V8ErrorCache& errors) const
{
    if (modifiers.is_pointer)
    {
        errors.error(
            u8"export value {} as pointer type",
            _rttr_type->name()
        );
        return false;
    }
    return true;
}
void V8BTValue::_make_template()
{
    using namespace ::v8;
    auto* isolate = Isolate::GetCurrent();

    HandleScope handle_scope(isolate);

    auto tp = FunctionTemplate::New(
        isolate,
        _call_ctor,
        External::New(isolate, this)
    );
    _fill_template(tp);
    _v8_template.Reset(isolate, tp);
}

// v8 callback
void V8BTValue::_gc_callback(const ::v8::WeakCallbackInfo<V8BPValue>& data)
{
    auto* bind_proxy = data.GetParameter();
    bind_proxy->invalidate();
    bind_proxy->isolate->destroy_bind_proxy(bind_proxy);
}
void V8BTValue::_call_ctor(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // get user data
    auto* bind_tp = get_bind_data<V8BTValue>(info);

    // block not construct call
    if (!info.IsConstructCall())
    {
        Isolate->ThrowError("ctor can only be called with new");
        return;
    }

    // check constructable
    if (!bind_tp->_is_script_newable)
    {
        Isolate->ThrowError("object is not constructable");
        return;
    }

    // check params
    if (!bind_tp->_ctor.match(info))
    {
        Isolate->ThrowError("ctor params mismatch");
        return;
    }

    // new object
    void* alloc_mem = bind_tp->_rttr_type->alloc();
    bind_tp->_ctor.call(info, alloc_mem);

    // make bind proxy
    bind_tp->_new_bind_proxy(alloc_mem, info.This());
}
} // namespace skr