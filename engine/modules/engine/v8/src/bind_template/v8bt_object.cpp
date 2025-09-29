#include <SkrV8/bind_template/v8bt_object.hpp>
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
V8BTObject* V8BTObject::TryCreate(V8Isolate* isolate, const RTTRType* type)
{
    if (type->is_record() && type->based_on(type_id_of<ScriptbleObject>()))
    {
        V8BTObject* result = SkrNew<V8BTObject>();
        result->_setup(isolate, type);
        result->_make_template();
        return result;
    }
    else
    {
        return nullptr;
    }
}

// kind
EV8BTKind V8BTObject::kind() const
{
    return EV8BTKind::Object;
}
String V8BTObject::type_name() const
{
    return _rttr_type->name();
}
String V8BTObject::cpp_namespace() const
{
    return _rttr_type->name_space_str();
}

// error process
bool V8BTObject::any_error() const
{
    return _any_error();
}
void V8BTObject::dump_error(V8ErrorBuilderTreeStyle& builder) const
{
    builder.write_line(u8"{} <Object>", _rttr_type->name());
    builder.indent([&]() {
        _dump_error(builder);
    });
}

// convert helper
v8::Local<v8::Value> V8BTObject::to_v8(
    void* native_data
) const
{
    using namespace ::v8;

    // get address
    auto* address = *reinterpret_cast<void**>(native_data);

    // get proxy
    auto* bind_proxy = _get_or_make_proxy(address);

    return bind_proxy->v8_object.Get(Isolate::GetCurrent());
}
bool V8BTObject::to_native(
    void* native_data,
    v8::Local<v8::Value> v8_value,
    bool is_init
) const
{
    using namespace ::v8;

    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    // get bind proxy
    auto v8_object = v8_value->ToObject(context).ToLocalChecked();
    auto* bind_proxy = get_bind_proxy<V8BPObject>(v8_object);

    // check bind proxy
    if (!bind_proxy->is_valid())
    {
        isolate->ThrowError("calling on a released object");
        return false;
    }

    // do cast
    void* casted_ptr = bind_proxy->rttr_type->cast_to_base(
        bind_proxy->rttr_type->type_id(),
        bind_proxy->address
    );
    *reinterpret_cast<void**>(native_data) = casted_ptr;

    return true;
}

// invoke native api
bool V8BTObject::match_param(
    v8::Local<v8::Value> v8_param
) const
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    // check v8 value type
    if (!v8_param->IsObject()) { return false; }
    auto v8_object = v8_param->ToObject(context).ToLocalChecked();

    // check object internal field
    if (v8_object->InternalFieldCount() < 1) { return false; }

    // check bind proxy
    auto* bind_proxy = get_bind_proxy<V8BPObject>(v8_object);
    if (!bind_proxy->is_valid()) { return false; }

    // check inherit
    bool base_on = bind_proxy->rttr_type->based_on(
        _rttr_type->type_id()
    );
    return base_on;
}
void V8BTObject::push_param_native(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp,
    v8::Local<v8::Value> v8_value
) const
{
    void* native_data = stack.alloc_param_raw(
        sizeof(void*),
        alignof(void*),
        EDynamicStackParamKind::Direct,
        nullptr
    );
    to_native(native_data, v8_value, false);
}
void V8BTObject::push_param_native_pure_out(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp
) const
{
    SKR_UNREACHABLE_CODE();
}
v8::Local<v8::Value> V8BTObject::read_return_native(
    DynamicStack& stack,
    const V8BTDataReturn& return_bind_tp
) const
{
    if (!stack.is_return_stored()) { return {}; }
    void* native_data = stack.get_return_raw(); // always void**
    return to_v8(native_data);
}
v8::Local<v8::Value> V8BTObject::read_return_from_out_param(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp
) const
{
    void* native_data = stack.get_param_raw(param_bind_tp.index); // always void**
    return to_v8(native_data);
}

// invoke v8 api
v8::Local<v8::Value> V8BTObject::make_param_v8(
    void* native_data,
    const V8BTDataParam& param_bind_tp
) const
{
    return to_v8(native_data);
}

// field api
v8::Local<v8::Value> V8BTObject::get_field(
    void* obj,
    const RTTRType* obj_type,
    const V8BTDataField& field_bind_tp
) const
{
    void* field_address = field_bind_tp.solve_address(obj, obj_type);
    return to_v8(field_address);
}
void V8BTObject::set_field(
    v8::Local<v8::Value> v8_value,
    void* obj,
    const RTTRType* obj_type,
    const V8BTDataField& field_bind_tp
) const
{
    void* field_address = field_bind_tp.solve_address(obj, obj_type);
    to_native(field_address, v8_value, false);
}
v8::Local<v8::Value> V8BTObject::get_static_field(
    const V8BTDataStaticField& field_bind_tp
) const
{
    void* field_address = field_bind_tp.solve_address();
    return to_v8(field_address);
}
void V8BTObject::set_static_field(
    v8::Local<v8::Value> v8_value,
    const V8BTDataStaticField& field_bind_tp
) const
{
    void* field_address = field_bind_tp.solve_address();
    to_native(field_address, v8_value, false);
}

// check api
void V8BTObject::solve_invoke_behaviour(
    const V8BTDataParam& param_bind_tp,
    bool& appare_in_return,
    bool& appare_in_param
) const
{
    appare_in_param = true;
    appare_in_return = false;
}
bool V8BTObject::check_param(
    const V8BTDataParam& param_bind_tp,
    V8ErrorCache& errors
) const
{
    switch (param_bind_tp.inout_flag)
    {
    case ERTTRParamFlag::Out:
    case ERTTRParamFlag::InOut:
        errors.warning(
            u8"Out/InOut param flag will be ignored for object type"
        );
        break;
    default:
        break;
    }
    return _basic_type_check(param_bind_tp.modifiers, errors);
}
bool V8BTObject::check_return(
    const V8BTDataReturn& return_bind_tp,
    V8ErrorCache& errors
) const
{
    return _basic_type_check(return_bind_tp.modifiers, errors);
}
bool V8BTObject::check_field(
    const V8BTDataField& field_bind_tp,
    V8ErrorCache& errors
) const
{
    return _basic_type_check(field_bind_tp.modifiers, errors);
}
bool V8BTObject::check_static_field(
    const V8BTDataStaticField& field_bind_tp,
    V8ErrorCache& errors
) const
{
    return _basic_type_check(field_bind_tp.modifiers, errors);
}

// v8 export
bool V8BTObject::has_v8_export_obj() const
{
    return true;
}
v8::Local<v8::Value> V8BTObject::get_v8_export_obj() const
{
    using namespace ::v8;

    auto isolate = Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    return _v8_template.Get(isolate)->GetFunction(context).ToLocalChecked();
}
void V8BTObject::dump_ts_def(
    TSDefBuilder& builder
) const
{
    builder.$line(u8"// export as object");

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
String V8BTObject::get_ts_type_name() const
{
    return _rttr_type->name();
}
bool V8BTObject::ts_is_nullable() const
{
    return true;
}
// helper
V8BPObject* V8BTObject::_get_or_make_proxy(void* address) const
{
    using namespace ::v8;

    // find exist bind proxy
    if (auto found = isolate()->map_bind_proxy(address))
    {
        return static_cast<V8BPObject*>(found);
    }

    return _new_bind_proxy(address, v8::Local<v8::Object>());
}
V8BPObject* V8BTObject::_new_bind_proxy(void* address, v8::Local<v8::Object> self) const
{
    using namespace ::v8;

    auto* isolate = Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();
    HandleScope handle_scope(isolate);

    // get scriptble object
    ScriptbleObject* scriptble_object;
    {
        void* casted_ptr = _rttr_type->cast_to_base(
            type_id_of<ScriptbleObject>(),
            address
        );
        scriptble_object = reinterpret_cast<ScriptbleObject*>(casted_ptr);
    }

    // make object
    Local<ObjectTemplate> instance_template = _v8_template.Get(isolate)->InstanceTemplate();
    Local<v8::Object> object =
        self.IsEmpty() ?
        instance_template->NewInstance(context).ToLocalChecked() :
        self;

    // make bind proxy
    V8BPObject* bind_proxy = this->isolate()->create_bind_proxy<V8BPObject>();
    bind_proxy->rttr_type = _rttr_type;
    bind_proxy->isolate = this->isolate();
    bind_proxy->bind_tp = this;
    bind_proxy->address = address;
    bind_proxy->object = scriptble_object;
    bind_proxy->v8_object.Reset(isolate, object);

    // setup gc callback
    bind_proxy->v8_object.SetWeak(
        bind_proxy,
        _gc_callback,
        WeakCallbackType::kInternalFields
    );

    // set internal field
    object->SetInternalField(0, ::v8::External::New(isolate, bind_proxy));

    // register bind proxy
    this->isolate()->register_bind_proxy(address, bind_proxy);

    // bind mixin core
    scriptble_object->set_mixin_core(this->isolate());

    return bind_proxy;
}
bool V8BTObject::_basic_type_check(const V8BTDataModifier& modifiers, V8ErrorCache& errors) const
{
    if (!modifiers.is_pointer)
    {
        errors.error(
            u8"export object {} as value or reference type",
            _rttr_type->name()
        );
        return false;
    }
    return true;
}
void V8BTObject::_make_template()
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
void V8BTObject::_gc_callback(const ::v8::WeakCallbackInfo<V8BPObject>& data)
{
    auto* bind_proxy = data.GetParameter();
    bind_proxy->invalidate();
    bind_proxy->isolate->destroy_bind_proxy(bind_proxy);
}
void V8BTObject::_call_ctor(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    Isolate* Isolate = info.GetIsolate();
    HandleScope HandleScope(Isolate);

    // get user data
    auto* bind_tp = get_bind_data<V8BTObject>(info);

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

    // check ctor
    if (!bind_tp->_ctor.is_valid())
    {
        Isolate->ThrowError("object has no exported ctor");
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