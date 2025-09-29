#include <SkrV8/v8_isolate.hpp>
#include <SkrV8/v8_context.hpp>
#include <SkrContainers/set.hpp>
#include <SkrCore/log.hpp>
#include <SkrRTTR/type.hpp>
#include <SkrV8/v8_bind.hpp>
#include <SkrV8/v8_bind_proxy.hpp>
#include <SkrV8/v8_vfs.hpp>

#include <SkrV8/bind_template/v8_bind_template.hpp>
#include <SkrV8/bind_template/v8bt_primitive.hpp>
#include <SkrV8/bind_template/v8bt_enum.hpp>
#include <SkrV8/bind_template/v8bt_mapping.hpp>
#include <SkrV8/bind_template/v8bt_object.hpp>
#include <SkrV8/bind_template/v8bt_value.hpp>
#include <SkrV8/bind_template/v8bt_function_ref.hpp>

// v8 includes
#include <libplatform/libplatform.h>
#include <v8-initialization.h>
#include <v8-template.h>
#include <v8-external.h>
#include <v8-function.h>
#include <v8-container.h>

// allocator
namespace skr
{
struct V8Allocator final : ::v8::ArrayBuffer::Allocator
{
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
// ctor & dtor
V8Isolate::V8Isolate()
{
}
V8Isolate::~V8Isolate()
{
    if (is_init())
    {
        shutdown();
    }
}

// init & shutdown
void V8Isolate::init()
{
    using namespace ::v8;
    SKR_ASSERT(!is_init());

    // init isolate
    _isolate_create_params.array_buffer_allocator = SkrNew<V8Allocator>();
    _isolate = Isolate::New(_isolate_create_params);
    _isolate->SetData(0, this);

    // auto microtasks
    _isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kAuto);

    // TODO. dynamic module support
    // _isolate->SetHostImportModuleDynamicallyCallback(_dynamic_import_module); // used for support module
    // _isolate->SetHostInitializeImportMetaObjectCallback(); // used for set import.meta
    // v8::Module::CreateSyntheticModule

    // TODO. promise support
    // _isolate->SetPromiseRejectCallback; // used for capture unhandledRejection

    // init main context
    _main_context = SkrNew<V8Context>();
    _main_context->_init(this, u8"[Main]");
    _contexts.add(_main_context->name(), _main_context);
}
void V8Isolate::shutdown()
{
    SKR_ASSERT(is_init());

    // dispose main context
    if (_main_context)
    {
        _contexts.remove(_main_context->name());
        _main_context->_shutdown();
        SkrDelete(_main_context);
        _main_context = nullptr;
    }

    // dispose created contexts
    for (const auto& [name, context] : _contexts)
    {
        context->_shutdown();
        SkrDelete(context);
    }
    _contexts.clear();

    // clean up bind templates
    for (const auto& [guid, bind_tp] : _bind_tp_map)
    {
        SkrDelete(bind_tp);
    }
    _bind_tp_map.clear();
    for (const auto& [signature, bind_tp] : _bind_tp_map_generic)
    {
        SkrDelete(bind_tp);
    }
    _bind_tp_map_generic.clear();

    // clean up bind proxy cache
    _bind_proxy_map.clear();

    // clean up bind proxies
    _bind_proxy_pools.clear();

    // dispose isolate
    _isolate->Dispose();
    _isolate = nullptr;
}
bool V8Isolate::is_init() const
{
    return _isolate != nullptr;
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

// context management
V8Context* V8Isolate::main_context() const
{
    return _main_context;
}
V8Context* V8Isolate::create_context(String name)
{
    // solve name
    if (name.is_empty())
    {
        name = skr::format(u8"context_{}", _contexts.size());
    }

    auto* context = SkrNew<V8Context>();
    context->_init(this, name);
    _contexts.add(name, context);

    return context;
}
void V8Isolate::destroy_context(V8Context* context)
{
    bool removed = _contexts.remove(context->name());
    SKR_ASSERT(removed);
    context->_shutdown();
}

bool V8Isolate::invoke_v8(
    v8::Local<v8::Value> v8_this,
    v8::Local<v8::Function> v8_func,
    Span<const StackProxy> params,
    StackProxy return_value
)
{
    using namespace ::v8;

    auto* isolate = Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();
    HandleScope handle_scope(isolate);

    // solve bind template
    V8BTDataFunctionBase bind_tp;
    bind_tp.call_v8_setup(this, params, return_value);

    push_param_scope();

    // push params
    InlineVector<Local<v8::Value>, 16> v8_params;
    for (const auto& param : bind_tp.params_data)
    {
        if (!param.appare_in_param) { continue; }
        v8_params.add(
            param.bind_tp->make_param_v8(
                params[param.index].data,
                param
            )
        );
    }

    // call script
    auto v8_ret = v8_func->Call(
        context,
        v8_this,
        v8_params.size(),
        v8_params.data()
    );

    // check return
    bool success_read_return = bind_tp.call_v8_read_return(
        params,
        return_value,
        v8_ret
    );

    pop_param_scope();

    return success_read_return;
}

//==> IScriptMixinCore API
void V8Isolate::on_object_destroyed(
    ScriptbleObject* obj
)
{
    auto found = _bind_proxy_map.find(obj->rttr_get_head_ptr());
    SKR_ASSERT(found);
    found.value()->invalidate();
}
bool V8Isolate::try_invoke_mixin(
    ScriptbleObject* obj,
    StringView name,
    const Span<const StackProxy> params,
    StackProxy result
)
{
    using namespace ::v8;

    Isolate::Scope isolate_scope(_isolate);
    HandleScope handle_scope(_isolate);

    auto* isolate = _isolate;
    auto context = isolate->GetCurrentContext();
    if (context.IsEmpty())
    {
        context = _main_context->v8_context().Get(isolate);
    }

    // find bound object
    if (auto found_bp = _bind_proxy_map.find(obj->rttr_get_head_ptr()))
    {
        auto* bind_proxy = found_bp.value()->rttr_cast<V8BPObject>();

        // find method export data
        auto found_method = bind_proxy->bind_tp->find_method(name);
        if (!found_method)
        {
            SKR_LOG_FMT_ERROR(u8"mixin method {} data not found", name);
            return false;
        }

        // find method in object
        auto v8_object = bind_proxy->v8_object.Get(isolate);
        auto maybe_v8_func_value = v8_object->Get(
            context,
            V8Bind::to_v8(name, true)
        );

        // check exist
        if (maybe_v8_func_value.IsEmpty())
        {
            return false;
        }

        // check type
        auto v8_func_value = maybe_v8_func_value.ToLocalChecked();
        if (!v8_func_value->IsFunction())
        {
            return false;
        }

        // filter native function
        auto v8_func = v8_func_value.As<v8::Function>();
        if (found_method->v8_tp.Get(isolate)->GetFunction(context).ToLocalChecked() == v8_func)
        {
            return false;
        }

        // call script
        {
            push_param_scope();

            // push params
            InlineVector<Local<v8::Value>, 16> v8_params;
            for (const auto& param : found_method->params_data)
            {
                if (!param.appare_in_param) { continue; }
                v8_params.add(
                    param.bind_tp->make_param_v8(
                        params[param.index].data,
                        param
                    )
                );
            }

            // call script
            auto v8_ret = v8_func->Call(
                context,
                v8_object,
                v8_params.size(),
                v8_params.data()
            );

            // check return
            bool success_read_return = found_method->call_v8_read_return(
                params,
                result,
                v8_ret
            );

            pop_param_scope();

            return success_read_return;
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"V8Isolate::try_invoke_mixin: bind proxy not found for object {}", obj->rttr_get_head_ptr());
        return false;
    }
}
//==> IScriptMixinCore API

// bind proxy management
V8BindTemplate* V8Isolate::solve_bind_tp(
    TypeSignatureView signature
)
{
    {
        auto jumped_modifiers = signature.jump_modifier();
        if (!jumped_modifiers.is_empty())
        {
            SKR_LOG_WARN(u8"modifiers of signature will be ignored, to disable this warning, please jump modifier before call this function");
        }
    }

    if (signature.is_type())
    {
        GUID type_id;
        signature.read_type_id(type_id);
        return solve_bind_tp(type_id);
    }
    else if (signature.is_generic_type())
    {
        // find exist bind template
        if (auto found = _bind_tp_map_generic.find(signature))
        {
            return found.value();
        }

        // get generic type id
        GUID generic_id;
        uint32_t generic_param_count;
        auto solve_sig = signature.read_generic_type_id(generic_id, generic_param_count);

        switch (generic_id.get_hash())
        {
        case kFunctionRefGenericId.get_hash(): {
            // get function signature
            solve_sig = solve_sig.jump_next_type_or_data();

            // make bind tp
            auto* func_ref_bind_tp = V8BTFunctionRef::Create(this, solve_sig);
            _bind_tp_map_generic.add(signature, func_ref_bind_tp);
            return func_ref_bind_tp;
        }
        default: {
            SKR_LOG_FMT_ERROR(
                u8"V8Isolate::solve_bind_tp: generic type not supported: {}",
                generic_id
            );
        }
        }
    }
    else
    {
        SKR_UNREACHABLE_CODE()
    }

    return nullptr;
}
V8BindTemplate* V8Isolate::solve_bind_tp(
    const GUID& type_id
)
{
    // find exist bind template
    if (auto found = _bind_tp_map.find(type_id))
    {
        return found.value();
    }

    // try primitive
    if (auto primitive = V8BTPrimitive::TryCreate(this, type_id))
    {
        _bind_tp_map.add(type_id, primitive);
        return primitive;
    }

    auto* rttr_type = get_type_from_guid(type_id);
    if (!rttr_type)
    {
        SKR_LOG_FMT_ERROR(
            u8"V8Isolate::solve_bind_tp: type not found for guid: {}",
            type_id
        );
    }

    // try enum
    if (rttr_type->is_enum())
    {
        auto* enum_bind_tp = V8BTEnum::TryCreate(this, rttr_type);
        _bind_tp_map.add(type_id, enum_bind_tp);
        return enum_bind_tp;
    }

    // try mapping
    if (auto mapping = V8BTMapping::TryCreate(this, rttr_type))
    {
        _bind_tp_map.add(type_id, mapping);
        return mapping;
    }

    // try object
    if (auto object = V8BTObject::TryCreate(this, rttr_type))
    {
        _bind_tp_map.add(type_id, object);
        return object;
    }

    // try value
    if (auto value = V8BTValue::TryCreate(this, rttr_type))
    {
        _bind_tp_map.add(type_id, value);
        return value;
    }

    return nullptr;
}

// bind proxy pool
void V8Isolate::destroy_bind_proxy(V8BindProxy* bind_proxy)
{
    auto& pool = _bind_proxy_pools.find(bind_proxy->rttr_get_typeid()).value();
    pool.take_back(bind_proxy);
}

// bind proxy management
void V8Isolate::register_bind_proxy(
    void* native_ptr,
    V8BindProxy* bind_proxy
)
{
    _bind_proxy_map.add(native_ptr, bind_proxy);
}
void V8Isolate::unregister_bind_proxy(
    void* native_ptr,
    V8BindProxy* bind_proxy
)
{
    _bind_proxy_map.remove(native_ptr);
}
V8BindProxy* V8Isolate::map_bind_proxy(
    void* native_ptr
) const
{
    if (auto found = _bind_proxy_map.find(native_ptr))
    {
        return found.value();
    }
    return nullptr;
}

// 用于缓存调用期间为参数创建的临时 bind proxy
void V8Isolate::push_param_scope()
{
    _call_v8_param_proxy_stack.push_back(_call_v8_param_proxy.size());
}
void V8Isolate::pop_param_scope()
{
    auto last_size = _call_v8_param_proxy_stack.pop_back_get();
    while (last_size < _call_v8_param_proxy.size())
    {
        auto proxy = _call_v8_param_proxy.pop_back_get();
        proxy->invalidate();
    }
}
void V8Isolate::push_param_proxy(
    V8BindProxy* bind_proxy
)
{
    _call_v8_param_proxy.push_back(bind_proxy);
}

// debugger
void V8Isolate::init_debugger(int port)
{
    SKR_ASSERT(is_init());
    SKR_ASSERT(!is_debugger_init());
    _websocket_server.init(port);
    _inspector_client.server = &_websocket_server;
    _inspector_client.init(this);

    // notify main context created
    _inspector_client.notify_context_created(_main_context);

    // notify created context
    for (const auto& [name, context] : _contexts)
    {
        _inspector_client.notify_context_created(context);
    }
}
void V8Isolate::shutdown_debugger()
{
    SKR_ASSERT(is_init());
    SKR_ASSERT(is_debugger_init());
    _inspector_client.shutdown();
    _inspector_client.server = nullptr;
    _websocket_server.shutdown();
}
bool V8Isolate::is_debugger_init() const
{
    return _websocket_server.is_init();
}
void V8Isolate::pump_debugger_messages()
{
    _websocket_server.pump_messages();
}
void V8Isolate::wait_for_debugger_connected(uint64_t timeout_ms)
{
    auto start_time = std::chrono::high_resolution_clock::now();
    while (!_inspector_client.is_connected())
    {
        // pump v8 messages
        this->pump_message_loop();

        // pump net messages
        _websocket_server.pump_messages();

        // check timeout
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (elapsed_time >= timeout_ms)
        {
            break;
        }

        // sleep for a while
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
bool V8Isolate::any_debugger_connected() const
{
    return is_debugger_init() && _inspector_client.is_connected();
}

//============================global init============================
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