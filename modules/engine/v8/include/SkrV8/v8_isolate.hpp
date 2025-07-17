#pragma once
#include <SkrCore/memory/rc.hpp>
#include <SkrRTTR/script/script_binder.hpp>
#include <SkrRTTR/script/stack_proxy.hpp>
#include <SkrV8/v8_inspector.hpp>
#include <SkrV8/v8_bind_data.hpp>
#include <SkrV8/v8_script_loader.hpp>

// v8 includes
#include <v8-isolate.h>
#include <v8-platform.h>
#include <v8-primitive.h>
#ifndef __meta__
    #include "SkrV8/v8_isolate.generated.h"
#endif

namespace skr
{
struct V8Context;
struct V8Module;

enum class EV8ConvertCase
{
    Create,      // create from script/ set/get global value
    Field,       // set/get field value
    StaticField, // set/get static field value
    Param,       // set/get param value
    Return,      // set/get return value
};

// clang-format off
sreflect_struct(guid = "8aea5942-6e5c-4711-9502-83f252faa231")
SKR_V8_API V8Isolate: IScriptMixinCore {
    // clang-format on
    SKR_GENERATE_BODY()

    // friend struct V8Context;
    friend struct V8Value;

    // ctor & dtor
    V8Isolate();
    ~V8Isolate();

    // delate copy & move
    V8Isolate(const V8Isolate&)            = delete;
    V8Isolate(V8Isolate&&)                 = delete;
    V8Isolate& operator=(const V8Isolate&) = delete;
    V8Isolate& operator=(V8Isolate&&)      = delete;

    // init & shutdown
    void init();
    void shutdown();
    bool is_init() const;

    // isolate operators
    void pump_message_loop();
    void gc(bool full = true);

    // context
    V8Context* main_context() const;
    V8Context* create_context(String name = {});
    void       destroy_context(V8Context* context);

    // cpp module
    V8Module* add_cpp_module(StringView name);
    void      remove_cpp_module(V8Module* module);
    void      register_cpp_module_id(V8Module* module, int v8_module_id);
    void      unregister_cpp_module_id(V8Module* module, int v8_module_id);
    V8Module* find_cpp_module(StringView name) const;
    V8Module* find_cpp_module(int v8_module_id) const;

    // script loader & file modules

    // debugger
    void init_debugger(int port);
    void shutdown_debugger();
    bool is_debugger_init() const;
    void pump_debugger_messages();
    void wait_for_debugger_connected(uint64_t timeout_ms = std::numeric_limits<uint64_t>::max());
    bool any_debugger_connected() const;

    // getter
    inline v8::Isolate*               v8_isolate() const { return _isolate; }
    inline ScriptBinderManager&       script_binder_manger() { return _binder_mgr; }
    inline const ScriptBinderManager& script_binder_manger() const { return _binder_mgr; }

    // bind object
    V8BindCoreObject* translate_object(::skr::ScriptbleObject* obj);
    void              mark_object_deleted(::skr::ScriptbleObject* obj);

    // bind value
    V8BindCoreValue* create_value(const RTTRType* type, const void* source_data = nullptr);
    V8BindCoreValue* translate_value_field(const RTTRType* type, const void* data, V8BindCoreRecordBase* owner);
    V8BindCoreValue* translate_value_temporal(const RTTRType* type, const void* data, V8BindCoreValue::ESource source);

    // query template
    v8::Local<v8::ObjectTemplate>   get_or_add_enum_template(const RTTRType* type);
    v8::Local<v8::FunctionTemplate> get_or_add_record_template(const RTTRType* type);

    // clean up bind data
    void cleanup_templates();
    void cleanup_bind_cores();

    // convert
    v8::Local<v8::Value> to_v8(TypeSignatureView sig_view, const void* data);
    bool                 to_native(TypeSignatureView sig_view, void* data, v8::Local<v8::Value> v8_value, bool is_init);

    // invoke script
    bool invoke_v8(
        v8::Local<v8::Value>    v8_this,
        v8::Local<v8::Function> v8_func,
        span<const StackProxy>  params,
        StackProxy              return_value
    );
    bool invoke_v8_mixin(
        v8::Local<v8::Value>      v8_this,
        v8::Local<v8::Function>   v8_func,
        const ScriptBinderMethod& mixin_data,
        span<const StackProxy>    params,
        StackProxy                return_value
    );

    // => IScriptMixinCore API
    void on_object_destroyed(ScriptbleObject* obj) override;
    bool try_invoke_mixin(ScriptbleObject* obj, StringView name, const span<const StackProxy> params, StackProxy ret) override;
    // => IScriptMixinCore API

private:
    // template helper
    v8::Local<v8::FunctionTemplate> _make_template_object(ScriptBinderObject* object_binder);
    v8::Local<v8::FunctionTemplate> _make_template_value(ScriptBinderValue* value_binder);

    void _fill_record_template(
        ScriptBinderRecordBase*         binder,
        V8BindDataRecordBase*           bind_data,
        v8::Local<v8::FunctionTemplate> ctor_template
    );

    // module callback
    static v8::MaybeLocal<v8::Promise> _dynamic_import_module(
        v8::Local<v8::Context>    context,
        v8::Local<v8::Data>       host_defined_options,
        v8::Local<v8::Value>      resource_name,
        v8::Local<v8::String>     specifier,
        v8::Local<v8::FixedArray> import_assertions
    );

    // uniform new
    template <typename T>
    inline T* _new_bind_data()
    {
        auto* result    = SkrNew<T>();
        result->manager = this;
        return result;
    }
    template <typename T>
    inline T* _new_bind_core()
    {
        auto* result        = SkrNew<T>();
        result->skr_isolate = this;
        return result;
    }

    // bind methods
    static void _gc_callback(const ::v8::WeakCallbackInfo<V8BindCoreRecordBase>& data);
    static void _call_ctor(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _call_mixin(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _call_method(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _call_static_method(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _get_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _set_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _get_static_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _set_static_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _get_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _set_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _get_static_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _set_static_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _enum_to_string(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _enum_from_string(const ::v8::FunctionCallbackInfo<::v8::Value>& info);

    // convert helper
    v8::Local<v8::Value> _to_v8(
        ScriptBinderRoot binder,
        void*            native_data
    );
    bool _to_native(
        ScriptBinderRoot     binder,
        void*                native_data,
        v8::Local<v8::Value> v8_value,
        bool                 is_init
    );
    v8::Local<v8::Value> _to_v8_primitive(
        const ScriptBinderPrimitive& binder,
        void*                        native_data
    );
    void _init_primitive(
        const ScriptBinderPrimitive& binder,
        void*                        native_data
    );
    bool _to_native_primitive(
        const ScriptBinderPrimitive& binder,
        v8::Local<v8::Value>         v8_value,
        void*                        native_data,
        bool                         is_init
    );
    v8::Local<v8::Value> _to_v8_mapping(
        const ScriptBinderMapping& binder,
        void*                      obj
    );
    bool _to_native_mapping(
        const ScriptBinderMapping& binder,
        v8::Local<v8::Value>       v8_value,
        void*                      native_data,
        bool                       is_init
    );
    v8::Local<v8::Value> _to_v8_object(
        const ScriptBinderObject& binder,
        void*                     native_data
    );
    bool _to_native_object(
        const ScriptBinderObject& binder,
        v8::Local<v8::Value>      v8_value,
        void*                     native_data,
        bool                      is_init
    );
    v8::Local<v8::Value> _to_v8_value(
        const ScriptBinderValue& binder,
        void*                    native_data
    );
    bool _to_native_value(
        const ScriptBinderValue& binder,
        v8::Local<v8::Value>     v8_value,
        void*                    native_data,
        bool                     is_init
    );

    // field convert helper
    static void* _get_field_address(
        const RTTRFieldData* field,
        const RTTRType*      field_owner,
        const RTTRType*      obj_type,
        void*                obj
    );
    bool _set_field_value_or_object(
        const ScriptBinderField& binder,
        v8::Local<v8::Value>     v8_value,
        V8BindCoreRecordBase*    bind_core
    );
    bool _set_field_mapping(
        const ScriptBinderField& binder,
        v8::Local<v8::Value>     v8_value,
        void*                    obj,
        const RTTRType*          obj_type
    );
    v8::Local<v8::Value> _get_field_value_or_object(
        const ScriptBinderField& binder,
        V8BindCoreRecordBase*    bind_core
    );
    v8::Local<v8::Value> _get_field_mapping(
        const ScriptBinderField& binder,
        const void*              obj,
        const RTTRType*          obj_type
    );
    bool _set_static_field(
        const ScriptBinderStaticField& binder,
        v8::Local<v8::Value>           v8_value
    );
    v8::Local<v8::Value> _get_static_field(
        const ScriptBinderStaticField& binder
    );

    // param & return convert helper
    void _push_param(
        DynamicStack&            stack,
        const ScriptBinderParam& param_binder,
        v8::Local<v8::Value>     v8_value
    );
    void _push_param_pure_out(
        DynamicStack&            stack,
        const ScriptBinderParam& param_binder
    );
    v8::Local<v8::Value> _read_return(
        DynamicStack&                    stack,
        const Vector<ScriptBinderParam>& params_binder,
        const ScriptBinderReturn&        return_binder,
        uint32_t                         solved_return_count
    );
    v8::Local<v8::Value> _read_return(
        DynamicStack&             stack,
        const ScriptBinderReturn& return_binder
    );
    v8::Local<v8::Value> _read_return_from_out_param(
        DynamicStack&            stack,
        const ScriptBinderParam& param_binder
    );

    // call native helper
    v8::Local<v8::Value> _make_v8_param(
        const ScriptBinderParam& param_binder,
        void*                    native_data
    );
    bool _read_v8_return(
        span<const StackProxy>           params,
        StackProxy                       return_value,
        const Vector<ScriptBinderParam>& params_binder,
        const ScriptBinderReturn&        return_binder,
        uint32_t                         solved_return_count,
        v8::MaybeLocal<v8::Value>        v8_return_value
    );
    void _cleanup_value_param(
        span<const StackProxy>           params,
        const Vector<ScriptBinderParam>& params_binder
    );

    // invoke helper
    bool _call_native(
        const ScriptBinderCtor&                        binder,
        const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack,
        void*                                          obj
    );
    bool _call_native(
        const ScriptBinderMethod&                      binder,
        const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack,
        void*                                          obj,
        const RTTRType*                                obj_type
    );
    bool _call_native(
        const ScriptBinderStaticMethod&                binder,
        const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack
    );

private:
    // isolate data
    v8::Isolate*              _isolate               = nullptr;
    v8::Isolate::CreateParams _isolate_create_params = {};

    // binder manager
    ScriptBinderManager _binder_mgr = {};

    // template & bind data
    Map<const RTTRType*, V8BindDataRecordBase*> _record_templates = {};
    Map<const RTTRType*, V8BindDataEnum*>       _enum_templates   = {};

    // bind cores & objects
    Map<ScriptbleObject*, V8BindCoreObject*> _alive_objects         = {};
    Vector<V8BindCoreObject*>                _deleted_objects       = {};
    Map<void*, V8BindCoreValue*>             _script_created_values = {};
    Map<void*, V8BindCoreValue*>             _static_field_values   = {};
    Map<void*, V8BindCoreValue*>             _temporal_values       = {};

    // context manage
    RCUnique<V8Context>         _main_context = nullptr;
    Vector<RCUnique<V8Context>> _contexts     = {};

    // modules manage
    Map<String, RCUnique<V8Module>> _cpp_modules    = {};
    Map<int, V8Module*>             _cpp_modules_id = {};

    // script file loader
    RCUnique<IV8ScriptLoader> _script_loader = nullptr;

    // file module cache
    Map<String, v8::Global<v8::Module>> _file_module_cache    = {};
    Map<int, v8::Global<v8::Module>>    _file_module_cache_id = {};

    // debugger
    V8WebSocketServer _websocket_server = {};
    V8InspectorClient _inspector_client = {};
};
} // namespace skr

// global init
namespace skr
{
SKR_V8_API void init_v8();
SKR_V8_API void shutdown_v8();
SKR_V8_API v8::Platform& get_v8_platform();
} // namespace skr