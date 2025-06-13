#include "SkrV8/v8_isolate.hpp"
#include "SkrContainers/set.hpp"
#include "SkrCore/log.hpp"
#include "SkrV8/v8_bind_data.hpp"
#include "libplatform/libplatform.h"
#include "v8-initialization.h"
#include "SkrRTTR/type.hpp"
#include "v8-template.h"
#include "v8-external.h"
#include "v8-function.h"
#include "v8-container.h"
#include "SkrV8/v8_bind.hpp"

// allocator
namespace skr
{
struct V8Allocator final : ::v8::ArrayBuffer::Allocator {
    static constexpr const char* kV8DefaultPoolName = "v8-allocate";

    void* AllocateUninitialized(size_t length) override
    {
#if defined(TRACY_TRACE_ALLOCATION)
        SkrCZoneNCS(z, "v8::allocate", SKR_ALLOC_TRACY_MARKER_COLOR, 16, 1);
        void* p = sakura_malloc_alignedN(length, alignof(size_t), kV8DefaultPoolName);
        SkrCZoneEnd(z);
        return p;
#else
        return sakura_malloc_aligned(length, alignof(size_t));
#endif
    }
    void Free(void* data, size_t length) override
    {
        if (data)
        {
#if defined(TRACY_TRACE_ALLOCATION)
            SkrCZoneNCS(z, "v8::free", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
            sakura_free_alignedN(data, alignof(size_t), kV8DefaultPoolName);
            SkrCZoneEnd(z);
#else
            sakura_free_aligned(data, alignof(size_t));
#endif
        }
    }
    void* Reallocate(void* data, size_t old_length, size_t new_length) override
    {
        SkrCZoneNCS(z, "v8::realloc", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
        void* new_mem = sakura_realloc_alignedN(data, new_length, alignof(size_t), kV8DefaultPoolName);
        SkrCZoneEnd(z);
        return new_mem;
    }
    void* Allocate(size_t length) override
    {
        void* p = AllocateUninitialized(length);
        memset(p, 0, length);
        return p;
    }
};
} // namespace skr

namespace skr
{
V8Isolate::V8Isolate()
{
}
V8Isolate::~V8Isolate()
{
    shutdown();
}

void V8Isolate::init()
{
    using namespace ::v8;

    // init isolate
    _isolate_create_params.array_buffer_allocator = SkrNew<V8Allocator>();
    _isolate                                      = Isolate::New(_isolate_create_params);
    _isolate->SetData(0, this);

    // auto microtasks
    _isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kAuto);

    // TODO. module support
    // _isolate->SetHostImportModuleDynamicallyCallback(_dynamic_import_module); // used for support module
    // _isolate->SetHostInitializeImportMetaObjectCallback(); // used for set import.meta
    // v8::Module::CreateSyntheticModule

    // TODO. promise support
    // _isolate->SetPromiseRejectCallback; // used for capture unhandledRejection
}
void V8Isolate::shutdown()
{
    if (_isolate)
    {
        // cleanup templates
        cleanup_templates();

        // dispose isolate
        _isolate->Dispose();
        _isolate = nullptr;

        // cleanup cores
        cleanup_bind_cores();
    }
}

// isolate operators
void V8Isolate::pump_message_loop()
{
    while (v8::platform::PumpMessageLoop(&get_v8_platform(), _isolate)) {};
}
void V8Isolate::gc(bool full)
{
    _isolate->RequestGarbageCollectionForTesting(full ? ::v8::Isolate::kFullGarbageCollection : ::v8::Isolate::kMinorGarbageCollection);

    _isolate->LowMemoryNotification();
    _isolate->IdleNotificationDeadline(0);
}

// bind object
V8BindCoreObject* V8Isolate::translate_object(::skr::ScriptbleObject* obj)
{
    //! NOTE. isolate & context must be setted before call this function

    using namespace ::v8;

    // get v8 context
    auto* isolate = Isolate::GetCurrent();
    auto  context = isolate->GetCurrentContext();

    // make scope
    HandleScope handle_scope(isolate);

    // find exist object
    if (auto found = _alive_objects.find(obj))
    {
        return found.value();
    }

    // get type
    auto type = get_type_from_guid(obj->iobject_get_typeid());

    // get template
    auto found_template = _record_templates.find(type);
    if (!found_template)
    {
        SKR_LOG_FMT_ERROR(
            u8"type {} template not found, please register it first",
            type->name()
        );
        return nullptr;
    }

    // make object
    Local<ObjectTemplate> instance_template = found_template.value()->ctor_template.Get(isolate)->InstanceTemplate();
    Local<Object>         object            = instance_template->NewInstance(context).ToLocalChecked();

    // make bind core
    auto object_bind_core  = _new_bind_data<V8BindCoreObject>();
    object_bind_core->type = type;
    object_bind_core->data = obj->iobject_get_head_ptr();
    object_bind_core->v8_object.Reset(isolate, object);
    object_bind_core->object = obj;

    // setup gc callback
    object_bind_core->v8_object.SetWeak(
        (V8BindCoreRecordBase*)object_bind_core,
        _gc_callback,
        WeakCallbackType::kInternalFields
    );

    // add extern data
    object->SetInternalField(0, External::New(isolate, object_bind_core));

    // add to map
    _alive_objects.add(obj, object_bind_core);

    // bind mixin core
    obj->set_mixin_core(this);

    return object_bind_core;
}
void V8Isolate::mark_object_deleted(::skr::ScriptbleObject* obj)
{
    if (auto found = _alive_objects.find(obj))
    {
        // reset core object ptr
        found.value()->invalidate();

        // move to deleted objects
        _deleted_objects.push_back(found.value());
        _alive_objects.remove(obj);
    }
}

// bind value
V8BindCoreValue* V8Isolate::create_value(const RTTRType* type, const void* source_data)
{
    //! NOTE. isolate & context must be setted before call this function
    using namespace ::v8;

    auto* isolate = Isolate::GetCurrent();
    auto  context = isolate->GetCurrentContext();

    HandleScope handle_scope(isolate);

    // get template
    auto found_template = _record_templates.find(type);
    if (!found_template)
    {
        SKR_LOG_FMT_ERROR(
            u8"type {} template not found, please register it first",
            type->name()
        );
        return nullptr;
    }

    void* alloc_mem;

    if (source_data)
    {
        // find copy ctor
        auto found_copy_ctor = type->find_copy_ctor();
        if (!found_copy_ctor)
        {
            return nullptr;
        }

        // alloc value
        alloc_mem = sakura_new_aligned(type->size(), type->alignment());

        // call copy ctor
        found_copy_ctor.invoke(alloc_mem, source_data);
    }
    else
    {
        // find default ctor
        auto found_default_ctor = type->find_default_ctor();
        if (!found_default_ctor)
        {
            return nullptr;
        }

        // alloc value
        alloc_mem = sakura_new_aligned(type->size(), type->alignment());

        // call default ctor
        found_default_ctor.invoke(alloc_mem);
    }

    // make object
    Local<ObjectTemplate> instance_template = found_template.value()->ctor_template.Get(isolate)->InstanceTemplate();
    Local<Object>         object            = instance_template->NewInstance(context).ToLocalChecked();

    // make bind core
    auto value_bind_core       = _new_bind_core<V8BindCoreValue>();
    value_bind_core->manager   = this;
    value_bind_core->type      = type;
    value_bind_core->data      = alloc_mem;
    value_bind_core->binder    = found_template.value()->as_value()->binder;
    value_bind_core->bind_data = found_template.value()->as_value();
    value_bind_core->v8_object.Reset(isolate, object);

    // setup source
    value_bind_core->from = V8BindCoreValue::ESource::Create;

    // setup gc callback
    value_bind_core->v8_object.SetWeak(
        (V8BindCoreRecordBase*)value_bind_core,
        _gc_callback,
        WeakCallbackType::kInternalFields
    );

    // add extern data
    object->SetInternalField(0, External::New(isolate, value_bind_core));

    // add to map
    _script_created_values.add(alloc_mem, value_bind_core);

    return value_bind_core;
}
V8BindCoreValue* V8Isolate::translate_value_field(const RTTRType* type, const void* data, V8BindCoreRecordBase* owner)
{
    //! NOTE. isolate & context must be setted before call this function
    using namespace ::v8;

    auto* isolate = Isolate::GetCurrent();
    auto  context = isolate->GetCurrentContext();

    HandleScope handle_scope(isolate);

    // get template
    auto found_template = _record_templates.find(type);
    if (!found_template)
    {
        SKR_LOG_FMT_ERROR(
            u8"type {} template not found, please register it first",
            type->name()
        );
        return nullptr;
    }

    // check if value
    if (!found_template.value()->is_value)
    {
        SKR_LOG_FMT_ERROR(
            u8"type {} is not a value type",
            type->name()
        );
        return nullptr;
    }

    if (owner)
    { // means field
        // find exported data
        if (auto found_data = owner->cache_value_fields.find(data))
        {
            return found_data.value();
        }

        // make object
        Local<ObjectTemplate> instance_template = found_template.value()->ctor_template.Get(isolate)->InstanceTemplate();
        Local<Object>         object            = instance_template->NewInstance(context).ToLocalChecked();

        // make bind core
        auto value_bind_core       = _new_bind_core<V8BindCoreValue>();
        value_bind_core->manager   = this;
        value_bind_core->type      = type;
        value_bind_core->data      = const_cast<void*>(data);
        value_bind_core->binder    = found_template.value()->as_value()->binder;
        value_bind_core->bind_data = found_template.value()->as_value();
        value_bind_core->v8_object.Reset(isolate, object);

        // setup source
        value_bind_core->from             = V8BindCoreValue::ESource::Field;
        value_bind_core->from_field_owner = owner;

        // setup gc callback
        value_bind_core->v8_object.SetWeak(
            (V8BindCoreRecordBase*)value_bind_core,
            _gc_callback,
            WeakCallbackType::kInternalFields
        );

        // add extern data
        object->SetInternalField(0, External::New(isolate, value_bind_core));

        // add to map
        owner->cache_value_fields.add(const_cast<void*>(data), value_bind_core);
        return value_bind_core;
    }
    else
    { // means static field
        // find exported data
        if (auto result = _static_field_values.find(data))
        {
            return result.value();
        }

        // make object
        Local<ObjectTemplate> instance_template = found_template.value()->ctor_template.Get(isolate)->InstanceTemplate();
        Local<Object>         object            = instance_template->NewInstance(context).ToLocalChecked();

        // create new core
        auto* value_bind_core      = _new_bind_core<V8BindCoreValue>();
        value_bind_core->manager   = this;
        value_bind_core->type      = type;
        value_bind_core->data      = const_cast<void*>(data);
        value_bind_core->binder    = found_template.value()->as_value()->binder;
        value_bind_core->bind_data = found_template.value()->as_value();
        value_bind_core->v8_object.Reset(isolate, object);

        // setup source
        value_bind_core->from = V8BindCoreValue::ESource::StaticField;

        // setup gc collback
        value_bind_core->v8_object.SetWeak(
            (V8BindCoreRecordBase*)value_bind_core,
            _gc_callback,
            WeakCallbackType::kInternalFields
        );

        // add extern data
        object->SetInternalField(0, External::New(isolate, value_bind_core));

        // add to map
        _static_field_values.add(const_cast<void*>(data), value_bind_core);
        return value_bind_core;
    }
}
V8BindCoreValue* V8Isolate::translate_value_temporal(const RTTRType* type, const void* data, V8BindCoreValue::ESource source)
{
    //! NOTE. isolate & context must be setted before call this function
    using namespace ::v8;

    auto* isolate = Isolate::GetCurrent();
    auto  context = isolate->GetCurrentContext();

    HandleScope handle_scope(isolate);

    // get template
    auto found_template = _record_templates.find(type);
    if (!found_template)
    {
        SKR_LOG_FMT_ERROR(
            u8"type {} template not found, please register it first",
            type->name()
        );
        return nullptr;
    }

    // check if value
    if (!found_template.value()->is_value)
    {
        SKR_LOG_FMT_ERROR(
            u8"type {} is not a value type",
            type->name()
        );
        return nullptr;
    }

    // make object
    Local<ObjectTemplate> instance_template = found_template.value()->ctor_template.Get(isolate)->InstanceTemplate();
    Local<Object>         object            = instance_template->NewInstance(context).ToLocalChecked();

    // make bind core
    auto value_bind_core       = _new_bind_core<V8BindCoreValue>();
    value_bind_core->manager   = this;
    value_bind_core->type      = type;
    value_bind_core->data      = const_cast<void*>(data);
    value_bind_core->binder    = found_template.value()->as_value()->binder;
    value_bind_core->bind_data = found_template.value()->as_value();
    value_bind_core->v8_object.Reset(isolate, object);

    // setup source
    value_bind_core->from = source;

    // setup gc callback
    value_bind_core->v8_object.SetWeak(
        (V8BindCoreRecordBase*)value_bind_core,
        _gc_callback,
        WeakCallbackType::kInternalFields
    );

    // add extern data
    object->SetInternalField(0, External::New(isolate, value_bind_core));

    // add to map
    _temporal_values.add(const_cast<void*>(data), value_bind_core);
    return value_bind_core;
}

// query template
v8::Local<v8::ObjectTemplate> V8Isolate::get_or_add_enum_template(const RTTRType* type)
{
    //! NOTE. isolate & context must be setted before call this function

    using namespace ::v8;
    // check
    SKR_ASSERT(type->is_enum());

    auto* isolate = Isolate::GetCurrent();

    // v8 scope
    EscapableHandleScope handle_scope(isolate);

    // find exists template
    if (auto result = _enum_templates.find(type))
    {
        return handle_scope.Escape(result.value()->enum_template.Get(isolate));
    }

    // get binder
    ScriptBinderRoot  binder      = _binder_mgr.get_or_build(type->type_id());
    ScriptBinderEnum* enum_binder = binder.enum_();

    // new bind data
    auto bind_data    = _new_bind_data<V8BindDataEnum>();
    bind_data->binder = binder.enum_();

    // object template
    auto object_template = ObjectTemplate::New(isolate);

    // add enum items
    for (const auto& [enum_item_name, enum_item] : enum_binder->items)
    {
        // get value
        Local<Value> enum_value = V8Bind::to_v8(enum_item->value);

        // set value
        object_template->Set(
            V8Bind::to_v8(enum_item->name, true),
            enum_value
        );
    }

    // add convert functions
    object_template->Set(
        V8Bind::to_v8(u8"to_string", true),
        FunctionTemplate::New(
            isolate,
            _enum_to_string,
            External::New(isolate, bind_data)
        )
    );
    object_template->Set(
        V8Bind::to_v8(u8"from_string", true),
        FunctionTemplate::New(
            isolate,
            _enum_from_string,
            External::New(isolate, bind_data)
        )
    );

    _enum_templates.add(type, bind_data);
    return handle_scope.Escape(object_template);
}
v8::Local<v8::FunctionTemplate> V8Isolate::get_or_add_record_template(const RTTRType* type)
{
    //! NOTE. isolate & context must be setted before call this function

    using namespace ::v8;

    // check
    SKR_ASSERT(type->is_record());

    auto* isolate = Isolate::GetCurrent();

    // v8 scope
    EscapableHandleScope handle_scope(isolate);

    // find exists template
    if (auto result = _record_templates.find(type))
    {
        return handle_scope.Escape(result.value()->ctor_template.Get(isolate));
    }

    // get binder
    ScriptBinderRoot binder = _binder_mgr.get_or_build(type->type_id());
    if (binder.is_object())
    {
        return handle_scope.Escape(_make_template_object(binder.object()));
    }
    else if (binder.is_value())
    {
        return handle_scope.Escape(_make_template_value(binder.value()));
    }
    else
    {
        return {};
    }
}

// clean up data
void V8Isolate::cleanup_templates()
{
    for (auto& [type, bind_data] : _record_templates)
    {
        bind_data->ctor_template.Reset();
        SkrDelete(bind_data);
    }
    _record_templates.clear();
    for (auto& [type, bind_data] : _enum_templates)
    {
        bind_data->enum_template.Reset();
        SkrDelete(bind_data);
    }
    _enum_templates.clear();
}
void V8Isolate::cleanup_bind_cores()
{
    for (auto& [obj, bind_core] : _alive_objects)
    {
        // delete object if only script owns it
        if (bind_core->object->ownership() == EScriptbleObjectOwnership::Script)
        {
            // remove mixin core first, for prevent delete callback
            bind_core->object->set_mixin_core(nullptr);

            SkrDelete(bind_core->object);
        }

        // delete core
        SkrDelete(bind_core);
    }
    _alive_objects.clear();
    for (auto& bind_core : _deleted_objects)
    {
        // just delete core
        SkrDelete(bind_core);
    }
    _deleted_objects.clear();
}

// convert
v8::Local<v8::Value> V8Isolate::to_v8(TypeSignatureView sig_view, const void* data)
{
    //! NOTE. isolate & context must be setted before call this function
    using namespace ::v8;

    if (sig_view.is_type())
    {
        // get binder
        GUID type_id;
        sig_view.jump_modifier();
        sig_view.read_type_id(type_id);
        ScriptBinderRoot binder = _binder_mgr.get_or_build(type_id);

        // convert
        return _to_v8(binder, const_cast<void*>(data));
    }
    else if (sig_view.is_generic_type())
    {
        SKR_LOG_ERROR(u8"generic type not supported yet");
        return {};
    }
    else
    {
        SKR_LOG_ERROR(u8"unsupported type signature");
        return {};
    }
}
bool V8Isolate::to_native(TypeSignatureView sig_view, void* data, v8::Local<v8::Value> v8_value, bool is_init)
{
    //! NOTE. isolate & context must be setted before call this function
    using namespace ::v8;

    if (sig_view.is_type())
    {
        // get binder
        GUID type_id;
        sig_view.jump_modifier();
        sig_view.read_type_id(type_id);
        ScriptBinderRoot binder = _binder_mgr.get_or_build(type_id);

        // convert
        return _to_native(binder, const_cast<void*>(data), v8_value, is_init);
    }
    else if (sig_view.is_generic_type())
    {
        SKR_LOG_ERROR(u8"generic type not supported yet");
        return {};
    }
    else
    {
        SKR_LOG_ERROR(u8"unsupported type signature");
        return {};
    }
}

// invoke script
bool V8Isolate::invoke_v8(
    v8::Local<v8::Value>    v8_this,
    v8::Local<v8::Function> v8_func,
    span<const StackProxy>  params,
    StackProxy              return_value
)
{
    auto*           isolate = v8::Isolate::GetCurrent();
    auto            context = isolate->GetCurrentContext();
    v8::HandleScope handle_scope(isolate);

    // get invoke binder
    auto binder = _binder_mgr.build_call_script_binder(params, return_value);

    // push params
    InlineVector<v8::Local<v8::Value>, 16> v8_params;
    for (const auto& param_binder : binder.params_binder)
    {
        // filter pure out
        if (param_binder.inout_flag == ERTTRParamFlag::Out) { continue; }

        // push param
        auto param = _make_v8_param(param_binder, params[param_binder.index].data);
        v8_params.add(param);
    }

    // call script
    auto v8_ret = v8_func->Call(context, v8_this, v8_params.size(), v8_params.data());

    // check return
    bool success_read_return = _read_v8_return(
        params,
        return_value,
        binder.params_binder,
        binder.return_binder,
        binder.return_count,
        v8_ret
    );

    // invalidate value params
    _cleanup_value_param(params, binder.params_binder);

    return success_read_return;
}
bool V8Isolate::invoke_v8_mixin(
    v8::Local<v8::Value>                v8_this,
    v8::Local<v8::Function>             v8_func,
    const ScriptBinderMethod::Overload& mixin_data,
    span<const StackProxy>              params,
    StackProxy                          return_value
)
{
    auto*           isolate = v8::Isolate::GetCurrent();
    auto            context = isolate->GetCurrentContext();
    v8::HandleScope handle_scope(isolate);

    // push params
    InlineVector<v8::Local<v8::Value>, 16> v8_params;
    for (const auto& param_binder : mixin_data.params_binder)
    {
        // filter pure out
        if (param_binder.inout_flag == ERTTRParamFlag::Out) { continue; }

        // push param
        auto param = _make_v8_param(param_binder, params[param_binder.index].data);
        v8_params.add(param);
    }

    // call script
    auto v8_ret = v8_func->Call(context, v8_this, v8_params.size(), v8_params.data());

    // check return
    bool success_read_return = _read_v8_return(
        params,
        return_value,
        mixin_data.params_binder,
        mixin_data.return_binder,
        mixin_data.return_count,
        v8_ret
    );

    // invalidate value params
    _cleanup_value_param(params, mixin_data.params_binder);

    return success_read_return;
}

// => IScriptMixinCore API
void V8Isolate::on_object_destroyed(ScriptbleObject* obj)
{
    mark_object_deleted(obj);
}
bool V8Isolate::try_invoke_mixin(ScriptbleObject* obj, StringView name, const span<const StackProxy> params, StackProxy ret)
{
    v8::Isolate::Scope isolate_scope(_isolate);

    auto* isolate = _isolate;
    auto  context = isolate->GetCurrentContext();

    // find bound object
    if (auto found = _alive_objects.find(obj))
    {
        // get bind core
        auto* bind_core = found.value();

        // find method overload data
        auto found_method = bind_core->bind_data->methods.find(name);
        if (!found_method)
        {
            SKR_LOG_FMT_ERROR(u8"mixin method {} not found", name);
            return false;
        }

        // get overload
        if (!found_method.value()->binder.is_mixin)
        {
            SKR_LOG_FMT_ERROR(u8"find method {} but not a mixin", name);
            return false;
        }
        const auto& overload = found_method.value()->binder.overloads[0];

        // find method in object
        auto v8_object = bind_core->v8_object.Get(v8::Isolate::GetCurrent());
        auto v8_func   = v8_object->Get(
            context,
            V8Bind::to_v8(name, true)
        );

        // check if function
        if (v8_func.IsEmpty() || !v8_func.ToLocalChecked()->IsFunction())
        {
            return false;
        }

        // check if native function
        auto v8_func_checked = v8_func.ToLocalChecked().As<v8::Function>();
        if (found_method.value()->v8_template.Get(isolate)->GetFunction(context).ToLocalChecked() == v8_func_checked)
        {
            return false;
        }

        // call script
        return invoke_v8_mixin(
            v8_object,
            v8_func_checked,
            overload,
            params,
            ret
        );
    }

    return false;
}

// template helper
v8::Local<v8::FunctionTemplate> V8Isolate::_make_template_object(ScriptBinderObject* object_binder)
{
    using namespace ::v8;

    auto* isolate = Isolate::GetCurrent();

    // v8 scope
    EscapableHandleScope handle_scope(isolate);

    // new bind data
    auto bind_data    = _new_bind_data<V8BindDataObject>();
    bind_data->binder = object_binder;

    // ctor template
    auto ctor_template = FunctionTemplate::New(
        isolate,
        _call_ctor,
        External::New(isolate, bind_data)
    );
    bind_data->ctor_template.Reset(isolate, ctor_template);

    // fill template
    _fill_record_template(object_binder, bind_data, ctor_template);

    // store template
    _record_templates.add(object_binder->type, bind_data);

    return handle_scope.Escape(ctor_template);
}
v8::Local<v8::FunctionTemplate> V8Isolate::_make_template_value(ScriptBinderValue* value_binder)
{
    using namespace ::v8;

    auto* isolate = Isolate::GetCurrent();

    // v8 scope
    EscapableHandleScope handle_scope(isolate);

    // new bind data
    auto bind_data    = _new_bind_data<V8BindDataValue>();
    bind_data->binder = value_binder;

    // ctor template
    auto ctor_template = FunctionTemplate::New(
        isolate,
        _call_ctor,
        External::New(isolate, bind_data)
    );
    bind_data->ctor_template.Reset(isolate, ctor_template);

    // fill template
    _fill_record_template(value_binder, bind_data, ctor_template);

    // store template
    _record_templates.add(value_binder->type, bind_data);

    return handle_scope.Escape(ctor_template);
}
void V8Isolate::_fill_record_template(
    ScriptBinderRecordBase*         binder,
    V8BindDataRecordBase*           bind_data,
    v8::Local<v8::FunctionTemplate> ctor_template
)
{
    using namespace ::v8;

    auto* isolate = Isolate::GetCurrent();

    // setup internal field count
    ctor_template->InstanceTemplate()->SetInternalFieldCount(1);

    // bind method
    for (const auto& [method_name, method_binder] : binder->methods)
    {
        auto method_bind_data    = _new_bind_data<V8BindDataMethod>();
        method_bind_data->binder = method_binder;
        auto v8_template         = FunctionTemplate::New(
            isolate,
            _call_method,
            External::New(isolate, method_bind_data)
        );
        method_bind_data->v8_template.Reset(isolate, v8_template);
        ctor_template->PrototypeTemplate()->Set(
            V8Bind::to_v8(method_name, true),
            v8_template
        );
        bind_data->methods.add(method_name, method_bind_data);
    }

    // bind static method
    for (const auto& [static_method_name, static_method_binder] : binder->static_methods)
    {
        auto static_method_bind_data    = _new_bind_data<V8BindDataStaticMethod>();
        static_method_bind_data->binder = static_method_binder;
        auto v8_template                = FunctionTemplate::New(
            isolate,
            _call_static_method,
            External::New(isolate, static_method_bind_data)
        );
        static_method_bind_data->v8_template.Reset(isolate, v8_template);
        ctor_template->Set(
            V8Bind::to_v8(static_method_name, true),
            v8_template
        );
        bind_data->static_methods.add(static_method_name, static_method_bind_data);
    }

    // bind field
    for (const auto& [field_name, field_binder] : binder->fields)
    {
        auto field_bind_data    = _new_bind_data<V8BindDataField>();
        field_bind_data->binder = field_binder;
        auto getter_template    = FunctionTemplate::New(
            isolate,
            _get_field,
            External::New(isolate, field_bind_data)
        );
        auto setter_template =
            FunctionTemplate::New(
                isolate,
                _set_field,
                External::New(isolate, field_bind_data)
            );
        field_bind_data->v8_template_getter.Reset(isolate, getter_template);
        field_bind_data->v8_template_setter.Reset(isolate, setter_template);
        ctor_template->PrototypeTemplate()->SetAccessorProperty(
            V8Bind::to_v8(field_name, true),
            getter_template,
            setter_template
        );
        bind_data->fields.add(field_name, field_bind_data);
    }

    // bind static field
    for (const auto& [static_field_name, static_field_binder] : binder->static_fields)
    {
        auto static_field_bind_data    = _new_bind_data<V8BindDataStaticField>();
        static_field_bind_data->binder = static_field_binder;
        auto getter_template           = FunctionTemplate::New(
            isolate,
            _get_static_field,
            External::New(isolate, static_field_bind_data)
        );
        auto setter_template =
            FunctionTemplate::New(
                isolate,
                _set_static_field,
                External::New(isolate, static_field_bind_data)
            );
        static_field_bind_data->v8_template_getter.Reset(isolate, getter_template);
        static_field_bind_data->v8_template_setter.Reset(isolate, setter_template);
        ctor_template->SetAccessorProperty(
            V8Bind::to_v8(static_field_name, true),
            getter_template,
            setter_template
        );
        bind_data->static_fields.add(static_field_name, static_field_bind_data);
    }

    // bind properties
    for (const auto& [property_name, property_binder] : binder->properties)
    {
        auto property_bind_data    = _new_bind_data<V8BindDataProperty>();
        property_bind_data->binder = property_binder;
        auto getter_template       = FunctionTemplate::New(
            isolate,
            _get_prop,
            External::New(isolate, property_bind_data)
        );
        auto setter_template = FunctionTemplate::New(
            isolate,
            _set_prop,
            External::New(isolate, property_bind_data)
        );
        ctor_template->PrototypeTemplate()->SetAccessorProperty(
            V8Bind::to_v8(property_name, true),
            getter_template,
            setter_template
        );
        bind_data->properties.add(property_name, property_bind_data);
    }

    // bind static properties
    for (const auto& [static_property_name, static_property_binder] : binder->static_properties)
    {
        auto static_property_bind_data    = _new_bind_data<V8BindDataStaticProperty>();
        static_property_bind_data->binder = static_property_binder;
        auto getter_template              = FunctionTemplate::New(
            isolate,
            _get_static_prop,
            External::New(isolate, static_property_bind_data)
        );
        auto setter_template = FunctionTemplate::New(
            isolate,
            _set_static_prop,
            External::New(isolate, static_property_bind_data)
        );
        static_property_bind_data->v8_template_getter.Reset(isolate, getter_template);
        static_property_bind_data->v8_template_setter.Reset(isolate, setter_template);
        ctor_template->SetAccessorProperty(
            V8Bind::to_v8(static_property_name, true),
            getter_template,
            setter_template
        );
        bind_data->static_properties.add(static_property_name, static_property_bind_data);
    }
}

// module callback
v8::MaybeLocal<v8::Promise> V8Isolate::_dynamic_import_module(
    v8::Local<v8::Context>    context,
    v8::Local<v8::Data>       host_defined_options,
    v8::Local<v8::Value>      resource_name,
    v8::Local<v8::String>     specifier,
    v8::Local<v8::FixedArray> import_assertions
)
{
    v8::Local<v8::Promise::Resolver> resolver;

    // v8::Local<v8::Module> module;
    // module->GetModuleNamespace();

    // log info
    skr::String spec;
    V8Bind::to_native(specifier, spec);
    SKR_LOG_FMT_INFO(u8"import module {}", spec);

    resolver->Resolve(context, {}).Check();

    return resolver->GetPromise();
}

// bind methods
void V8Isolate::_gc_callback(const ::v8::WeakCallbackInfo<V8BindCoreRecordBase>& data)
{
    using namespace ::v8;

    V8BindCoreRecordBase* bind_core = data.GetParameter();

    if (bind_core->is_value)
    {
        auto* value_core = bind_core->as_value();

        // do release if not field export case
        if (value_core->from == V8BindCoreValue::ESource::Create)
        {
            // call destructor
            value_core->type->invoke_dtor(value_core->data);

            // release data
            sakura_free_aligned(value_core->data, value_core->type->alignment());

            // reset object handle
            value_core->v8_object.Reset();

            // remove from map
            value_core->manager->_script_created_values.remove(value_core->data);

            // release core
            SkrDelete(value_core);
        }
        else
        {
            if (value_core->from == V8BindCoreValue::ESource::Field)
            {
                // remove from cache
                value_core->from_field_owner->cache_value_fields.remove(value_core->data);
            }
            else if (value_core->from == V8BindCoreValue::ESource::StaticField)
            {
                value_core->manager->_static_field_values.remove(value_core->data);
            }

            // reset object handle
            value_core->v8_object.Reset();

            // release core
            SkrDelete(value_core);
        }
    }
    else
    {
        auto* object_core = bind_core->as_object();

        // get data
        auto* skr_isolate = bind_core->manager;

        // remove alive object
        if (object_core->object)
        {
            // delete if has owner ship
            if (object_core->object->ownership() == EScriptbleObjectOwnership::Script)
            {
                // remove mixin first, for prevent delete callback
                object_core->object->set_mixin_core(nullptr);

                SkrDelete(object_core->object);
            }

            skr_isolate->_alive_objects.remove(object_core->object);
        }
        else
        {
            // remove from deleted records
            skr_isolate->_deleted_objects.remove(bind_core);
        }

        // reset object handle
        object_core->v8_object.Reset();

        // release core
        SkrDelete(object_core);
    }
}
void V8Isolate::_call_ctor(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get v8 basic info
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get user data
    auto* bind_data   = reinterpret_cast<V8BindDataRecordBase*>(info.Data().As<External>()->Value());
    auto* skr_isolate = bind_data->manager;

    // handle call
    if (info.IsConstructCall())
    {
        // get ctor info
        Local<Object> self = info.This();

        if (bind_data->is_value)
        {
            auto* value_data = bind_data->as_value();
            auto* binder     = value_data->binder;

            // check constructable
            if (!binder->is_script_newable)
            {
                Isolate->ThrowError("record is not constructable");
                return;
            }

            // match ctor
            const ScriptBinderCtor* final_ctor_binder = nullptr;
            int32_t                 highest_score     = std::numeric_limits<int32_t>::min();
            for (const auto& ctor_binder : binder->ctors)
            {
                auto match_result = V8Bind::match(ctor_binder.params_binder, ctor_binder.params_binder.size(), info);
                if (!match_result.matched) continue;
                if (match_result.match_score > highest_score)
                {
                    final_ctor_binder = &ctor_binder;
                    highest_score     = match_result.match_score;
                }
            }

            // invoke ctor
            if (final_ctor_binder)
            {
                // alloc memory
                void* alloc_mem = sakura_new_aligned(binder->type->size(), binder->type->alignment());

                // call ctor
                bool success = bind_data->manager->_call_native(
                    *final_ctor_binder,
                    info,
                    alloc_mem
                );

                // bad call
                if (!success)
                {
                    sakura_free_aligned(alloc_mem, binder->type->alignment());
                    Isolate->ThrowError("invoke ctor failed");
                }

                // make bind core
                V8BindCoreValue* bind_core = SkrNew<V8BindCoreValue>();
                bind_core->manager         = bind_data->manager;
                bind_core->type            = binder->type;
                bind_core->data            = alloc_mem;

                // setup source
                bind_core->from = V8BindCoreValue::ESource::Create;

                // setup gc callback
                bind_core->v8_object.Reset(Isolate, self);
                bind_core->v8_object.SetWeak(
                    (V8BindCoreRecordBase*)bind_core,
                    _gc_callback,
                    WeakCallbackType::kInternalFields
                );

                // add extern data
                self->SetInternalField(0, External::New(Isolate, bind_core));

                // add to map
                skr_isolate->_script_created_values.add(bind_core->data, bind_core);
            }
            else
            {
                // no ctor called
                Isolate->ThrowError("no ctor matched");
            }
        }
        else
        {
            auto* object_data = bind_data->as_object();
            auto* binder      = object_data->binder;

            // check constructable
            if (!binder->is_script_newable)
            {
                Isolate->ThrowError("record is not constructable");
                return;
            }

            // match ctor
            const ScriptBinderCtor* final_ctor_binder = nullptr;
            int32_t                 highest_score     = std::numeric_limits<int32_t>::min();
            for (const auto& ctor_binder : binder->ctors)
            {
                auto match_result = V8Bind::match(ctor_binder.params_binder, ctor_binder.params_binder.size(), info);
                if (!match_result.matched) continue;
                if (match_result.match_score > highest_score)
                {
                    final_ctor_binder = &ctor_binder;
                    highest_score     = match_result.match_score;
                }
            }

            // invoke ctor
            if (final_ctor_binder)
            {
                // alloc memory
                void* alloc_mem = sakura_new_aligned(binder->type->size(), binder->type->alignment());

                // call ctor
                bool success = bind_data->manager->_call_native(
                    *final_ctor_binder,
                    info,
                    alloc_mem
                );

                // bad call
                if (!success)
                {
                    sakura_free_aligned(alloc_mem, binder->type->alignment());
                    Isolate->ThrowError("invoke ctor failed");
                }

                // cast to ScriptbleObject
                void* casted_mem = binder->type->cast_to_base(type_id_of<ScriptbleObject>(), alloc_mem);

                // make bind core
                V8BindCoreObject* bind_core = skr_isolate->_new_bind_core<V8BindCoreObject>();
                bind_core->manager          = bind_data->manager;
                bind_core->type             = binder->type;
                bind_core->data             = alloc_mem;
                bind_core->binder           = object_data->binder;
                bind_core->bind_data        = object_data;
                bind_core->object           = reinterpret_cast<ScriptbleObject*>(casted_mem);

                // setup mixin
                bind_core->object->set_mixin_core(skr_isolate);

                // setup owner ship
                bind_core->object->ownership_take_script();

                // setup gc callback
                bind_core->v8_object.Reset(Isolate, self);
                bind_core->v8_object.SetWeak(
                    (V8BindCoreRecordBase*)bind_core,
                    _gc_callback,
                    WeakCallbackType::kInternalFields
                );

                // add extern data
                self->SetInternalField(0, External::New(Isolate, bind_core));

                // add to map
                skr_isolate->_alive_objects.add(bind_core->object, bind_core);
            }
            else
            {
                // no ctor called
                Isolate->ThrowError("no ctor matched");
            }
        }
    }
    else
    {
        Isolate->ThrowError("must be called with new");
    }
}
void V8Isolate::_call_method(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get v8 basic info
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get self data
    Local<Object> self      = info.This();
    auto*         bind_core = reinterpret_cast<V8BindCoreRecordBase*>(self->GetInternalField(0).As<External>()->Value());

    // check core
    if (!bind_core->is_valid())
    {
        Isolate->ThrowError("calling on a released object");
        return;
    }

    // get user data
    auto* bind_data = reinterpret_cast<V8BindDataMethod*>(info.Data().As<External>()->Value());

    // block ctor call
    if (info.IsConstructCall())
    {
        Isolate->ThrowError("method can not be called with new");
        return;
    }

    // call method
    bool success = bind_data->manager->_call_native(
        bind_data->binder,
        info,
        bind_core->data,
        bind_core->type
    );

    // throw
    if (!success)
    {
        Isolate->ThrowError("no matched method");
        return;
    }
}
void V8Isolate::_call_static_method(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get v8 basic info
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get user data
    auto* bind_data = reinterpret_cast<V8BindDataStaticMethod*>(info.Data().As<External>()->Value());

    // block ctor call
    if (info.IsConstructCall())
    {
        Isolate->ThrowError("method can not be called with new");
        return;
    }

    // call method
    bool success = bind_data->manager->_call_native(
        bind_data->binder,
        info
    );

    // throw
    if (!success)
    {
        Isolate->ThrowError("no matched method");
        return;
    }
}
void V8Isolate::_get_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get data
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get self data
    Local<Object> self      = info.This();
    auto*         bind_core = reinterpret_cast<V8BindCoreRecordBase*>(self->GetInternalField(0).As<External>()->Value());

    // check core
    if (!bind_core->is_valid())
    {
        Isolate->ThrowError("calling on a released object");
        return;
    }

    // get user data
    auto* bind_data = reinterpret_cast<V8BindDataField*>(info.Data().As<External>()->Value());

    // get field
    auto v8_field = bind_data->manager->_get_field_value_or_object(
        bind_data->binder,
        bind_core
    );
    info.GetReturnValue().Set(v8_field);
}
void V8Isolate::_set_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get data
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get self data
    Local<Object> self      = info.This();
    auto*         bind_core = reinterpret_cast<V8BindCoreRecordBase*>(self->GetInternalField(0).As<External>()->Value());

    // check core
    if (!bind_core->is_valid())
    {
        Isolate->ThrowError("calling on a released object");
        return;
    }

    // get user data
    auto* bind_data = reinterpret_cast<V8BindDataField*>(info.Data().As<External>()->Value());

    // set field
    bool success = bind_data->manager->_set_field_value_or_object(
        bind_data->binder,
        info[0],
        bind_core
    );

    // throw
    if (!success)
    {
        Isolate->ThrowError("no matched field");
        return;
    }
}
void V8Isolate::_get_static_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get data
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get user data
    auto* bind_data = reinterpret_cast<V8BindDataStaticField*>(info.Data().As<External>()->Value());

    // get field
    auto v8_field = bind_data->manager->_get_static_field(
        bind_data->binder
    );
    info.GetReturnValue().Set(v8_field);
}
void V8Isolate::_set_static_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get data
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get user data
    auto* bind_data = reinterpret_cast<V8BindDataStaticField*>(info.Data().As<External>()->Value());

    // set field
    bool success = bind_data->manager->_set_static_field(
        bind_data->binder,
        info[0]
    );

    // throw
    if (!success)
    {
        Isolate->ThrowError("no matched field");
        return;
    }
}
void V8Isolate::_get_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get data
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get self data
    Local<Object> self      = info.This();
    auto*         bind_core = reinterpret_cast<V8BindCoreRecordBase*>(self->GetInternalField(0).As<External>()->Value());

    // check core
    if (!bind_core->is_valid())
    {
        Isolate->ThrowError("calling on a released object");
        return;
    }

    // get user data
    auto* bind_data = reinterpret_cast<V8BindDataProperty*>(info.Data().As<External>()->Value());

    // invoke
    bool success = bind_data->manager->_call_native(
        bind_data->binder.getter,
        info,
        bind_core->data,
        bind_core->type
    );

    // throw
    if (!success)
    {
        Isolate->ThrowError("no matched property");
        return;
    }
}
void V8Isolate::_set_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get data
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get self data
    Local<Object> self      = info.This();
    auto*         bind_core = reinterpret_cast<V8BindCoreRecordBase*>(self->GetInternalField(0).As<External>()->Value());

    // check core
    if (!bind_core->is_valid())
    {
        Isolate->ThrowError("calling on a released object");
        return;
    }

    // get user data
    auto* bind_data = reinterpret_cast<V8BindDataProperty*>(info.Data().As<External>()->Value());

    // invoke
    bool success = bind_data->manager->_call_native(
        bind_data->binder.setter,
        info,
        bind_core->data,
        bind_core->type
    );

    // throw
    if (!success)
    {
        Isolate->ThrowError("no matched property");
        return;
    }
}
void V8Isolate::_get_static_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get data
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get user data
    auto* bind_data = reinterpret_cast<V8BindDataStaticProperty*>(info.Data().As<External>()->Value());

    // invoke
    bool success = bind_data->manager->_call_native(
        bind_data->binder.getter,
        info
    );

    // throw
    if (!success)
    {
        Isolate->ThrowError("no matched property");
        return;
    }
}
void V8Isolate::_set_static_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get data
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get user data
    auto* bind_data = reinterpret_cast<V8BindDataStaticProperty*>(info.Data().As<External>()->Value());

    // invoke
    bool success = bind_data->manager->_call_native(
        bind_data->binder.setter,
        info
    );

    // throw
    if (!success)
    {
        Isolate->ThrowError("no matched property");
        return;
    }
}
void V8Isolate::_enum_to_string(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get data
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get user data
    auto* bind_data = reinterpret_cast<V8BindDataEnum*>(info.Data().As<External>()->Value());

    // check value
    if (info.Length() != 1)
    {
        Isolate->ThrowError("enum to_string need 1 argument");
        return;
    }

    // get value
    int64_t  enum_singed;
    uint64_t enum_unsigned;
    if (bind_data->binder->is_signed)
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
    for (const auto& [enum_item_name, enum_item] : bind_data->binder->items)
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
void V8Isolate::_enum_from_string(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    // get data
    Isolate* Isolate = info.GetIsolate();

    // scopes
    HandleScope HandleScope(Isolate);

    // get user data
    auto* bind_data = reinterpret_cast<V8BindDataEnum*>(info.Data().As<External>()->Value());

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
    if (auto result = bind_data->binder->items.find(enum_item_name))
    {
        info.GetReturnValue().Set(V8Bind::to_v8(result.value()->value));
    }
    else
    {
        Isolate->ThrowError("no matched enum item");
    }
}

// convert helper
v8::Local<v8::Value> V8Isolate::_to_v8(
    ScriptBinderRoot binder,
    void*            native_data
)
{
    switch (binder.kind())
    {
    case ScriptBinderRoot::EKind::Primitive:
        return _to_v8_primitive(*binder.primitive(), native_data);
    case ScriptBinderRoot::EKind::Enum:
        return _to_v8_primitive(*binder.enum_()->underlying_binder, native_data);
    case ScriptBinderRoot::EKind::Mapping:
        return _to_v8_mapping(*binder.mapping(), native_data);
    case ScriptBinderRoot::EKind::Object:
        return _to_v8_object(*binder.object(), native_data);
    case ScriptBinderRoot::EKind::Value:
        return _to_v8_value(*binder.value(), native_data);
    default:
        SKR_UNREACHABLE_CODE()
    }
    return {};
}
bool V8Isolate::_to_native(
    ScriptBinderRoot     binder,
    void*                native_data,
    v8::Local<v8::Value> v8_value,
    bool                 is_init
)
{
    switch (binder.kind())
    {
    case ScriptBinderRoot::EKind::Primitive:
        return _to_native_primitive(*binder.primitive(), v8_value, native_data, is_init);
    case ScriptBinderRoot::EKind::Enum:
        return _to_native_primitive(*binder.enum_()->underlying_binder, v8_value, native_data, is_init);
    case ScriptBinderRoot::EKind::Mapping:
        return _to_native_mapping(*binder.mapping(), v8_value, native_data, is_init);
    case ScriptBinderRoot::EKind::Object:
        return _to_native_object(*binder.object(), v8_value, native_data, is_init);
    case ScriptBinderRoot::EKind::Value:
        return _to_native_value(*binder.value(), v8_value, native_data, is_init);
    default:
        SKR_UNREACHABLE_CODE()
    }
    return false;
}
v8::Local<v8::Value> V8Isolate::_to_v8_primitive(
    const ScriptBinderPrimitive& binder,
    void*                        native_data
)
{
    switch (binder.type_id.get_hash())
    {
    case type_id_of<int8_t>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<int8_t*>(native_data));
    case type_id_of<int16_t>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<int16_t*>(native_data));
    case type_id_of<int32_t>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<int32_t*>(native_data));
    case type_id_of<int64_t>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<int64_t*>(native_data));
    case type_id_of<uint8_t>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<uint8_t*>(native_data));
    case type_id_of<uint16_t>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<uint16_t*>(native_data));
    case type_id_of<uint32_t>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<uint32_t*>(native_data));
    case type_id_of<uint64_t>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<uint64_t*>(native_data));
    case type_id_of<float>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<float*>(native_data));
    case type_id_of<double>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<double*>(native_data));
    case type_id_of<bool>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<bool*>(native_data));
    case type_id_of<skr::String>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<skr::String*>(native_data));
    case type_id_of<skr::StringView>().get_hash():
        return V8Bind::to_v8(*reinterpret_cast<skr::StringView*>(native_data));
    default:
        SKR_UNREACHABLE_CODE()
        return {};
    }
}
void V8Isolate::_init_primitive(
    const ScriptBinderPrimitive& binder,
    void*                        native_data
)
{
    switch (binder.type_id.get_hash())
    {
    case type_id_of<int8_t>().get_hash():
    case type_id_of<int16_t>().get_hash():
    case type_id_of<int32_t>().get_hash():
    case type_id_of<int64_t>().get_hash():
    case type_id_of<uint8_t>().get_hash():
    case type_id_of<uint16_t>().get_hash():
    case type_id_of<uint32_t>().get_hash():
    case type_id_of<uint64_t>().get_hash():
    case type_id_of<float>().get_hash():
    case type_id_of<double>().get_hash():
    case type_id_of<bool>().get_hash():
        return;
    case type_id_of<skr::String>().get_hash():
        new (native_data) skr::String();
        return;
    case type_id_of<skr::StringView>().get_hash():
        new (native_data) skr::StringView();
        return;
    default:
        SKR_UNREACHABLE_CODE()
        return;
    }
}
bool V8Isolate::_to_native_primitive(
    const ScriptBinderPrimitive& binder,
    v8::Local<v8::Value>         v8_value,
    void*                        native_data,
    bool                         is_init
)
{
    if (!is_init)
    {
        _init_primitive(binder, native_data);
    }

    switch (binder.type_id.get_hash())
    {
    case type_id_of<int8_t>().get_hash():
        return V8Bind::to_native(v8_value, *reinterpret_cast<int8_t*>(native_data));
    case type_id_of<int16_t>().get_hash():
        return V8Bind::to_native(v8_value, *reinterpret_cast<int16_t*>(native_data));
    case type_id_of<int32_t>().get_hash():
        return V8Bind::to_native(v8_value, *reinterpret_cast<int32_t*>(native_data));
    case type_id_of<int64_t>().get_hash():
        return V8Bind::to_native(v8_value, *reinterpret_cast<int64_t*>(native_data));
    case type_id_of<uint8_t>().get_hash():
        return V8Bind::to_native(v8_value, *reinterpret_cast<uint8_t*>(native_data));
    case type_id_of<uint16_t>().get_hash():
        return V8Bind::to_native(v8_value, *reinterpret_cast<uint16_t*>(native_data));
    case type_id_of<uint32_t>().get_hash():
        return V8Bind::to_native(v8_value, *reinterpret_cast<uint32_t*>(native_data));
    case type_id_of<uint64_t>().get_hash():
        return V8Bind::to_native(v8_value, *reinterpret_cast<uint64_t*>(native_data));
    case type_id_of<float>().get_hash():
        return V8Bind::to_native(v8_value, *reinterpret_cast<float*>(native_data));
    case type_id_of<double>().get_hash():
        return V8Bind::to_native(v8_value, *reinterpret_cast<double*>(native_data));
    case type_id_of<bool>().get_hash():
        return V8Bind::to_native(v8_value, *reinterpret_cast<bool*>(native_data));
    case type_id_of<skr::String>().get_hash():
        return V8Bind::to_native(v8_value, *reinterpret_cast<skr::String*>(native_data));
    default:
        SKR_UNREACHABLE_CODE()
        return false;
    }
}
v8::Local<v8::Value> V8Isolate::_to_v8_mapping(
    const ScriptBinderMapping& binder,
    void*                      obj
)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    auto* type = binder.type;

    auto result = v8::Object::New(isolate);
    for (const auto& [field_name, field_data] : binder.fields)
    {
        // to v8
        auto v8_field_value = _get_field_mapping(field_data, obj, type);

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
bool V8Isolate::_to_native_mapping(
    const ScriptBinderMapping& binder,
    v8::Local<v8::Value>       v8_value,
    void*                      native_data,
    bool                       is_init
)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    auto* type      = binder.type;
    auto  v8_object = v8_value->ToObject(context).ToLocalChecked();

    // do init
    if (!is_init)
    {
        auto found_ctor = type->find_default_ctor();
        if (!found_ctor)
        {
            return false;
        }
        found_ctor.invoke(native_data);
    }

    for (const auto& [field_name, field_data] : binder.fields)
    {
        // clang-format off
        // find object field
        auto v8_field = v8_object->Get(
            context,
            V8Bind::to_v8(field_name, true)
        ).ToLocalChecked();
        
        // set field
        if (!_set_field_mapping(
            field_data,
            v8_field,
            native_data,
            type
        ))
        {
            return false;
        }
        // clang-format on
    }

    return true;
}
v8::Local<v8::Value> V8Isolate::_to_v8_object(
    const ScriptBinderObject& binder,
    void*                     native_data
)
{
    auto isolate = v8::Isolate::GetCurrent();

    auto* object_pointer = *reinterpret_cast<void**>(native_data);

    auto* type             = binder.type;
    void* cast_raw         = type->cast_to_base(type_id_of<ScriptbleObject>(), object_pointer);
    auto* scriptble_object = reinterpret_cast<ScriptbleObject*>(cast_raw);

    auto* bind_core = translate_object(scriptble_object);
    return bind_core->v8_object.Get(isolate);
}
bool V8Isolate::_to_native_object(
    const ScriptBinderObject& binder,
    v8::Local<v8::Value>      v8_value,
    void*                     native_data,
    bool                      is_init
)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    auto* type = binder.type;

    auto  v8_object     = v8_value->ToObject(context).ToLocalChecked();
    void* raw_bind_core = v8_object->GetInternalField(0).As<v8::External>()->Value();
    auto* bind_core     = reinterpret_cast<V8BindCoreObject*>(raw_bind_core);

    // check bind core
    if (!bind_core->is_valid())
    {
        isolate->ThrowError("calling on a released object");
        return false;
    }

    // do cast
    void* cast_ptr                         = bind_core->type->cast_to_base(type->type_id(), bind_core->data);
    *reinterpret_cast<void**>(native_data) = cast_ptr;
    return true;
}
v8::Local<v8::Value> V8Isolate::_to_v8_value(
    const ScriptBinderValue& binder,
    void*                    native_data
)
{
    auto isolate = v8::Isolate::GetCurrent();

    auto* bind_core = create_value(binder.type, native_data);
    return bind_core->v8_object.Get(isolate);
}
bool V8Isolate::_to_native_value(
    const ScriptBinderValue& binder,
    v8::Local<v8::Value>     v8_value,
    void*                    native_data,
    bool                     is_init
)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    auto* type = binder.type;

    auto  v8_object     = v8_value->ToObject(context).ToLocalChecked();
    void* raw_bind_core = v8_object->GetInternalField(0).As<v8::External>()->Value();
    auto* bind_core     = reinterpret_cast<V8BindCoreValue*>(raw_bind_core);

    // check bind core
    if (!bind_core->is_valid())
    {
        isolate->ThrowError("calling on a released object");
        return false;
    }

    // do cast
    void* cast_ptr = bind_core->type->cast_to_base(type->type_id(), bind_core->data);

    // assign to native
    if (!is_init)
    {
        // find copy ctor
        auto found_copy_ctor = type->find_copy_ctor();
        if (!found_copy_ctor)
        {
            return false;
        }

        // copy to native
        found_copy_ctor.invoke(native_data, cast_ptr);
        return true;
    }
    else
    {
        // find assign
        auto found_assign = type->find_assign();
        if (!found_assign)
        {
            return false;
        }

        // assign to native
        found_assign.invoke(native_data, cast_ptr);
        return true;
    }
}

// field convert helper
void* V8Isolate::_get_field_address(
    const RTTRFieldData* field,
    const RTTRType*      field_owner,
    const RTTRType*      obj_type,
    void*                obj
)
{
    SKR_ASSERT(obj != nullptr);
    void* field_owner_address = obj_type->cast_to_base(field_owner->type_id(), obj);
    return field->get_address(field_owner_address);
}
bool V8Isolate::_set_field_value_or_object(
    const ScriptBinderField& binder,
    v8::Local<v8::Value>     v8_value,
    V8BindCoreRecordBase*    bind_core
)
{
    // get field info
    void* field_address = _get_field_address(binder.data, binder.owner, bind_core->type, bind_core->data);

    // to native
    return _to_native(binder.binder, field_address, v8_value, true);
}
bool V8Isolate::_set_field_mapping(
    const ScriptBinderField& binder,
    v8::Local<v8::Value>     v8_value,
    void*                    obj,
    const RTTRType*          obj_type
)
{
    // get field info
    void* field_address = _get_field_address(binder.data, binder.owner, obj_type, obj);

    // to native
    return _to_native(binder.binder, field_address, v8_value, true);
}
v8::Local<v8::Value> V8Isolate::_get_field_value_or_object(
    const ScriptBinderField& binder,
    V8BindCoreRecordBase*    bind_core
)
{
    // get field info
    void* field_address = _get_field_address(
        binder.data,
        binder.owner,
        bind_core->type,
        const_cast<void*>(bind_core->data)
    );

    // to v8
    if (binder.binder.is_value())
    { // optimize for value case
        auto* value_binder    = binder.binder.value();
        auto* isolate         = v8::Isolate::GetCurrent();
        auto* value_bind_core = translate_value_field(value_binder->type, field_address, bind_core);
        return value_bind_core->v8_object.Get(isolate);
    }
    else
    {
        return _to_v8(binder.binder, field_address);
    }
}
v8::Local<v8::Value> V8Isolate::_get_field_mapping(
    const ScriptBinderField& binder,
    const void*              obj,
    const RTTRType*          obj_type
)
{
    // get field info
    void* field_address = _get_field_address(
        binder.data,
        binder.owner,
        obj_type,
        const_cast<void*>(obj)
    );

    return _to_v8(binder.binder, field_address);
}
bool V8Isolate::_set_static_field(
    const ScriptBinderStaticField& binder,
    v8::Local<v8::Value>           v8_value
)
{
    // get field info
    void* field_address = binder.data->address;

    // to native
    return _to_native(binder.binder, field_address, v8_value, true);
}
v8::Local<v8::Value> V8Isolate::_get_static_field(
    const ScriptBinderStaticField& binder
)
{
    // get field info
    void* field_address = binder.data->address;

    // to v8
    if (binder.binder.is_value())
    { // optimize for value case
        auto* value_binder = binder.binder.value();
        auto* value_core   = translate_value_field(value_binder->type, binder.data->address, nullptr);
        return value_core->v8_object.Get(v8::Isolate::GetCurrent());
    }
    else
    {
        return _to_v8(binder.binder, field_address);
    }
}

// param & return convert helper
struct V8StringViewStackProxy {
    Placeholder<v8::String::Utf8Value> v;
    skr::StringView                    view;

    static void* custom_mapping(void* obj)
    {
        return &reinterpret_cast<V8StringViewStackProxy*>(obj)->view;
    }
    ~V8StringViewStackProxy()
    {
        v.data_typed()->~Utf8Value();
    }
};
void V8Isolate::_push_param(
    DynamicStack&            stack,
    const ScriptBinderParam& param_binder,
    v8::Local<v8::Value>     v8_value
)
{
    switch (param_binder.binder.kind())
    {
    case ScriptBinderRoot::EKind::Primitive: {
        auto* primitive_binder = param_binder.binder.primitive();
        if (primitive_binder->type_id == type_id_of<StringView>())
        { // string view case, push proxy
            V8StringViewStackProxy* proxy = stack.alloc_param<V8StringViewStackProxy>(
                EDynamicStackParamKind::Direct,
                nullptr,
                StringViewStackProxy::custom_mapping
            );
            new (proxy) V8StringViewStackProxy();
            new (proxy->v.data_typed()) v8::String::Utf8Value(v8::Isolate::GetCurrent(), v8_value);
            proxy->view = { reinterpret_cast<const skr_char8*>(**proxy->v.data_typed()), (uint64_t)proxy->v.data_typed()->length() };
        }
        else
        {
            void* native_data = stack.alloc_param_raw(
                primitive_binder->size,
                primitive_binder->alignment,
                param_binder.pass_by_ref ? EDynamicStackParamKind::XValue : EDynamicStackParamKind::Direct,
                primitive_binder->dtor
            );
            SKR_ASSERT(_to_native_primitive(*primitive_binder, v8_value, native_data, false));
        }
        break;
    }
    case ScriptBinderRoot::EKind::Mapping: {
        auto*       mapping_binder = param_binder.binder.mapping();
        DtorInvoker dtor           = mapping_binder->type->dtor_invoker();
        void*       native_data    = stack.alloc_param_raw(
            mapping_binder->type->size(),
            mapping_binder->type->alignment(),
            param_binder.pass_by_ref ? EDynamicStackParamKind::XValue : EDynamicStackParamKind::Direct,
            dtor
        );
        SKR_ASSERT(_to_native_mapping(*mapping_binder, v8_value, native_data, false));
        break;
    }
    case ScriptBinderRoot::EKind::Enum: {
        auto* enum_binder       = param_binder.binder.enum_();
        auto* underlying_binder = enum_binder->underlying_binder;
        void* native_data       = stack.alloc_param_raw(
            underlying_binder->size,
            underlying_binder->alignment,
            param_binder.pass_by_ref ? EDynamicStackParamKind::XValue : EDynamicStackParamKind::Direct,
            underlying_binder->dtor
        );
        SKR_ASSERT(_to_native_primitive(*underlying_binder, v8_value, native_data, false));
    }
    case ScriptBinderRoot::EKind::Object: {
        void* native_data = stack.alloc_param_raw(
            sizeof(void*),
            alignof(void*),
            EDynamicStackParamKind::Direct,
            nullptr
        );
        SKR_ASSERT(_to_native_object(*param_binder.binder.object(), v8_value, native_data, false));
        break;
    }
    case ScriptBinderRoot::EKind::Value: {
        void* native_data;
        if (param_binder.pass_by_ref)
        { // optimize for param
            auto* value_binder = param_binder.binder.value();

            // get bind core
            auto* isolate   = v8::Isolate::GetCurrent();
            auto  context   = isolate->GetCurrentContext();
            auto  v8_object = v8_value->ToObject(context).ToLocalChecked();
            auto  bind_core = reinterpret_cast<V8BindCoreValue*>(v8_object->GetInternalField(0).As<v8::External>()->Value());

            // check bind core
            if (!bind_core->is_valid())
            {
                isolate->ThrowError("calling on a released object");
                return;
            }

            // push pointer
            auto* cast_ptr = bind_core->type->cast_to_base(value_binder->type->type_id(), bind_core->data);
            stack.add_param<void*>(cast_ptr);
        }
        else
        {
            auto*       value_binder = param_binder.binder.value();
            DtorInvoker dtor         = value_binder->type->dtor_invoker();
            native_data              = stack.alloc_param_raw(
                value_binder->type->size(),
                value_binder->type->alignment(),
                EDynamicStackParamKind::Direct,
                dtor
            );
            SKR_ASSERT(_to_native_value(*value_binder, v8_value, native_data, false));
        }
        break;
    }
    default:
        SKR_UNREACHABLE_CODE()
        break;
    }
}
void V8Isolate::_push_param_pure_out(
    DynamicStack&            stack,
    const ScriptBinderParam& param_binder
)
{
    switch (param_binder.binder.kind())
    {
    case ScriptBinderRoot::EKind::Primitive: {
        auto* primitive_binder = param_binder.binder.primitive();
        void* native_data      = stack.alloc_param_raw(
            primitive_binder->size,
            primitive_binder->alignment,
            param_binder.pass_by_ref ? EDynamicStackParamKind::XValue : EDynamicStackParamKind::Direct,
            primitive_binder->dtor
        );

        // call ctor
        _init_primitive(*param_binder.binder.primitive(), native_data);
        break;
    }
    case ScriptBinderRoot::EKind::Enum: {
        auto* enum_binder       = param_binder.binder.enum_();
        auto* underlying_binder = enum_binder->underlying_binder;
        void* native_data       = stack.alloc_param_raw(
            underlying_binder->size,
            underlying_binder->alignment,
            EDynamicStackParamKind::XValue,
            underlying_binder->dtor
        );

        // call ctor
        _init_primitive(*underlying_binder, native_data);
        break;
    }
    case ScriptBinderRoot::EKind::Mapping: {
        auto*       mapping_binder = param_binder.binder.mapping();
        DtorInvoker dtor           = mapping_binder->type->dtor_invoker();
        void*       native_data    = stack.alloc_param_raw(
            mapping_binder->type->size(),
            mapping_binder->type->alignment(),
            EDynamicStackParamKind::XValue,
            dtor
        );

        // call ctor
        auto ctor_data = mapping_binder->type->find_default_ctor();
        ctor_data.invoke(native_data);
        break;
    }
    case ScriptBinderRoot::EKind::Value: {
        auto* value_binder      = param_binder.binder.value();
        auto* create_value_core = create_value(value_binder->type);

        // push pointer
        stack.add_param<void*>(create_value_core->data);
        break;
    }
    case ScriptBinderRoot::EKind::Object: {
        SKR_UNREACHABLE_CODE()
        break;
    }
    default:
        SKR_UNREACHABLE_CODE()
        break;
    }
}
v8::Local<v8::Value> V8Isolate::_read_return(
    DynamicStack&                    stack,
    const Vector<ScriptBinderParam>& params_binder,
    const ScriptBinderReturn&        return_binder,
    uint32_t                         solved_return_count
)
{
    auto* isolate = v8::Isolate::GetCurrent();
    auto  context = isolate->GetCurrentContext();

    if (solved_return_count == 1)
    { // return single value
        if (return_binder.is_void)
        { // read from out param
            for (const auto& param_binder : params_binder)
            {
                if (param_binder.appare_in_return)
                {
                    return _read_return_from_out_param(
                        stack,
                        param_binder
                    );
                }
            }
        }
        else
        { // read return value
            if (stack.is_return_stored())
            {
                return _read_return(
                    stack,
                    return_binder
                );
            }
        }
    }
    else
    { // return param array
        v8::Local<v8::Array> out_array = v8::Array::New(isolate);
        uint32_t             cur_index = 0;

        // try read return value
        if (!return_binder.is_void)
        {
            if (stack.is_return_stored())
            {
                v8::Local<v8::Value> out_value = _read_return(
                    stack,
                    return_binder
                );
                out_array->Set(context, cur_index, out_value).Check();
                ++cur_index;
            }
        }

        // read return value from out param
        for (const auto& param_binder : params_binder)
        {
            if (param_binder.appare_in_return)
            {
                // clang-format off
                out_array->Set(context, cur_index,_read_return_from_out_param(
                    stack,
                    param_binder
                )).Check();
                // clang-format on
                ++cur_index;
            }
        }

        return out_array;
    }

    return {};
}
v8::Local<v8::Value> V8Isolate::_read_return(
    DynamicStack&             stack,
    const ScriptBinderReturn& return_binder
)
{
    if (!stack.is_return_stored()) { return {}; }

    void* native_data;
    switch (return_binder.binder.kind())
    {
    case ScriptBinderRoot::EKind::Primitive:
    case ScriptBinderRoot::EKind::Enum:
    case ScriptBinderRoot::EKind::Mapping:
    case ScriptBinderRoot::EKind::Value: {
        native_data = stack.get_return_raw();
        if (return_binder.pass_by_ref)
        {
            native_data = *reinterpret_cast<void**>(native_data);
        }
        break;
    }
    case ScriptBinderRoot::EKind::Object: {
        native_data = stack.get_return_raw();
        break;
    }
    default:
        SKR_UNREACHABLE_CODE()
        return {};
    }
    return _to_v8(return_binder.binder, native_data);
}
v8::Local<v8::Value> V8Isolate::_read_return_from_out_param(
    DynamicStack&            stack,
    const ScriptBinderParam& param_binder
)
{
    // get out param data
    void* native_data = stack.get_param_raw(param_binder.index);

    if (param_binder.binder.is_value())
    { // optimize for value case
        // get created value data
        native_data = *reinterpret_cast<void**>(native_data);
        if (auto result = _script_created_values.find(native_data))
        {
            return result.value()->v8_object.Get(v8::Isolate::GetCurrent());
        }
        else
        {
            SKR_UNREACHABLE_CODE();
            return {};
        }
    }
    else
    {
        // convert to v8
        return _to_v8(
            param_binder.binder,
            native_data
        );
    }
}

// call native helper
v8::Local<v8::Value> V8Isolate::_make_v8_param(
    const ScriptBinderParam& param_binder,
    void*                    native_data
)
{
    if (param_binder.binder.is_value())
    { // optimize for value case, pass by ref
        auto* bind_data = translate_value_temporal(
            param_binder.binder.value()->type,
            native_data,
            V8BindCoreValue::ESource::Param
        );
        return bind_data->v8_object.Get(v8::Isolate::GetCurrent());
    }
    return _to_v8(param_binder.binder, native_data);
}
bool V8Isolate::_read_v8_return(
    span<const StackProxy>           params,
    StackProxy                       return_value,
    const Vector<ScriptBinderParam>& params_binder,
    const ScriptBinderReturn&        return_binder,
    uint32_t                         solved_return_count,
    v8::MaybeLocal<v8::Value>        v8_return_value
)
{
    auto* isolate = v8::Isolate::GetCurrent();
    auto  context = isolate->GetCurrentContext();

    if (solved_return_count == 0)
    {
        return true;
    }
    else if (solved_return_count == 1)
    {
        if (v8_return_value.IsEmpty()) { return false; }

        if (return_binder.is_void)
        { // read to out param
            for (const auto& param_binder : params_binder)
            {
                if (param_binder.appare_in_return)
                {
                    return _to_native(
                        param_binder.binder,
                        params[param_binder.index].data,
                        v8_return_value.ToLocalChecked(),
                        true
                    );
                }
            }

            // must checked before
            SKR_UNREACHABLE_CODE()
            return false;
        }
        else
        { // read to return
            return _to_native(
                return_binder.binder,
                return_value.data,
                v8_return_value.ToLocalChecked(),
                false
            );
        }
    }
    else
    {
        if (v8_return_value.IsEmpty()) { return false; }
        if (!v8_return_value.ToLocalChecked()->IsArray()) { return false; }
        v8::Local<v8::Array> v8_array = v8_return_value.ToLocalChecked().As<v8::Array>();
        if (v8_array.IsEmpty() || v8_array->Length() != solved_return_count) { return false; }

        uint32_t cur_index = 0;

        // read return value
        bool success = true;
        if (!return_binder.is_void)
        {
            success &= _to_native(
                return_binder.binder,
                return_value.data,
                v8_array->Get(context, cur_index).ToLocalChecked(),
                false
            );
            ++cur_index;
        }

        // read out param
        for (const auto& param_binder : params_binder)
        {
            if (param_binder.appare_in_return)
            {
                success &= _to_native(
                    param_binder.binder,
                    params[param_binder.index].data,
                    v8_array->Get(context, cur_index).ToLocalChecked(),
                    true
                );
                ++cur_index;
            }
        }

        return success;
    }
}
void V8Isolate::_cleanup_value_param(
    span<const StackProxy>           params,
    const Vector<ScriptBinderParam>& params_binder
)
{
    for (const auto& param_binder : params_binder)
    {
        if (param_binder.binder.is_value() && param_binder.inout_flag != ERTTRParamFlag::Out)
        {
            auto found = _temporal_values.find(params[param_binder.index].data);
            if (found)
            {
                // invalidate value bind core
                found.value()->invalidate();
                _temporal_values.remove_at(found.index());
            }
            else
            {
                SKR_UNREACHABLE_CODE()
            }
        }
    }
}

// invoke helper
bool V8Isolate::_call_native(
    const ScriptBinderCtor&                        binder,
    const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack,
    void*                                          obj
)
{
    DynamicStack native_stack;

    // push param
    for (uint32_t i = 0; i < v8_stack.Length(); ++i)
    {
        _push_param(
            native_stack,
            binder.params_binder[i],
            v8_stack[i]
        );
    }

    // invoke
    native_stack.return_behaviour_discard();
    binder.data->dynamic_stack_invoke(obj, native_stack);

    return true;
}
bool V8Isolate::_call_native(
    const ScriptBinderMethod&                      binder,
    const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack,
    void*                                          obj,
    const RTTRType*                                obj_type
)
{
    // match overload
    const ScriptBinderMethod::Overload* final_overload = nullptr;
    int32_t                             highest_score  = std::numeric_limits<int32_t>::min();
    for (const auto& overload : binder.overloads)
    {
        auto result = V8Bind::match(overload.params_binder, overload.params_count, v8_stack);
        if (!result.matched) { continue; }
        if (result.match_score > highest_score)
        {
            highest_score  = result.match_score;
            final_overload = &overload;
        }
    }

    // invoke overload
    if (final_overload)
    {
        DynamicStack native_stack;

        // push param
        uint32_t v8_stack_index = 0;
        for (const auto& param_binder : final_overload->params_binder)
        {
            if (param_binder.inout_flag == ERTTRParamFlag::Out)
            { // pure out param, we will push a dummy xvalue
                _push_param_pure_out(
                    native_stack,
                    param_binder
                );
            }
            else
            {
                _push_param(
                    native_stack,
                    param_binder,
                    v8_stack[v8_stack_index]
                );
                ++v8_stack_index;
            }
        }

        // cast
        void* owner_address = obj_type->cast_to_base(final_overload->owner->type_id(), obj);

        // invoke
        native_stack.return_behaviour_store();
        if (final_overload->mixin_impl_data)
        {
            final_overload->mixin_impl_data->dynamic_stack_invoke(owner_address, native_stack);
        }
        else
        {
            final_overload->data->dynamic_stack_invoke(owner_address, native_stack);
        }

        // read return
        auto return_value = _read_return(
            native_stack,
            final_overload->params_binder,
            final_overload->return_binder,
            final_overload->return_count
        );
        if (!return_value.IsEmpty())
        {
            v8_stack.GetReturnValue().Set(return_value);
        }

        return true;
    }
    else
    {
        return false;
    }
}
bool V8Isolate::_call_native(
    const ScriptBinderStaticMethod&                binder,
    const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack
)
{
    // match overload
    const ScriptBinderStaticMethod::Overload* final_overload = nullptr;
    int32_t                                   highest_score  = std::numeric_limits<int32_t>::min();
    for (const auto& overload : binder.overloads)
    {
        auto result = V8Bind::match(overload.params_binder, overload.params_count, v8_stack);
        if (!result.matched) { continue; }
        if (result.match_score > highest_score)
        {
            highest_score  = result.match_score;
            final_overload = &overload;
        }
    }

    // invoke overload
    if (final_overload)
    {
        DynamicStack native_stack;

        // push param
        uint32_t v8_stack_index = 0;
        for (const auto& param_binder : final_overload->params_binder)
        {
            if (param_binder.inout_flag == ERTTRParamFlag::Out)
            { // pure out param, we will push a dummy xvalue
                _push_param_pure_out(
                    native_stack,
                    param_binder
                );
            }
            else
            {
                _push_param(
                    native_stack,
                    param_binder,
                    v8_stack[v8_stack_index]
                );
                ++v8_stack_index;
            }
        }

        // invoke
        native_stack.return_behaviour_store();
        final_overload->data->dynamic_stack_invoke(native_stack);

        // read return
        auto return_value = _read_return(
            native_stack,
            final_overload->params_binder,
            final_overload->return_binder,
            final_overload->return_count
        );
        if (!return_value.IsEmpty())
        {
            v8_stack.GetReturnValue().Set(return_value);
        }

        return true;
    }
    else
    {
        return false;
    }
}

} // namespace skr

namespace skr
{
static auto& _v8_platform()
{
    static v8::Platform* _platform = nullptr;
    return _platform;
}

void init_v8()
{
    // init flags
    char Flags[] = "--expose-gc";
    ::v8::V8::SetFlagsFromString(Flags, sizeof(Flags));

    // init platform
    _v8_platform() = ::v8::platform::NewDefaultPlatform_Without_Stl();
    ::v8::V8::InitializePlatform(_v8_platform());

    // init v8
    ::v8::V8::Initialize();
}
void shutdown_v8()
{
    // shutdown v8
    ::v8::V8::Dispose();

    // shutdown platform
    ::v8::V8::DisposePlatform();
    delete _v8_platform();
}
v8::Platform& get_v8_platform()
{
    return *_v8_platform();
}
} // namespace skr