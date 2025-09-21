#pragma once
#include <SkrCore/memory/rc.hpp>
#include <SkrRTTR/script/scriptble_object.hpp>
#include <SkrRTTR/script/stack_proxy.hpp>
#include <SkrCore/error_collector.hpp>
#include <SkrV8/v8_fwd.hpp>
#include <SkrV8/bind_template/v8_bind_template.hpp>
#include <SkrContainers/set.hpp>
#include <SkrV8/v8_bind_proxy.hpp>
#include <SkrV8/v8_inspector.hpp>

// v8 includes
#include <v8-isolate.h>
#include <v8-platform.h>
#include <v8-primitive.h>
#include "SkrV8/v8_isolate.generated.h"

// About JS AOT, just remark here, in mostly time it's useless
// see: https://mp2.dk/techblog/chowjs/
//      https://github.com/CanadaHonk/porffor?tab=readme-ov-file
//
// TODO. hot reload
// see: https://cloud.tencent.com/developer/article/1899559

namespace skr
{
struct V8BindProxyPool
{
    inline V8BindProxyPool() = default;
    inline ~V8BindProxyPool()
    {
        release();
    }

    template <std::derived_from<V8BindProxy> T>
    inline T* alloc()
    {
        auto* pooled = try_alloc();
        if (pooled) { return static_cast<T*>(pooled); }
        auto* result = SkrNew<T>();
        _used_proxies.push_back(result);
        return result;
    }
    inline V8BindProxy* try_alloc()
    {
        if (_free_proxies.is_empty()) { return nullptr; }
        auto result = _free_proxies.pop_back_get();
        _used_proxies.push_back(result);
        return result;
    }
    inline void take_back(V8BindProxy* proxy)
    {
        bool done_remove = _used_proxies.remove_swap(proxy);
        SKR_ASSERT(done_remove);
        _free_proxies.push_back(proxy);
    }
    inline void release()
    {
        for (auto* proxy : _used_proxies)
        {
            SkrDelete(proxy);
        }
        for (auto* proxy : _free_proxies)
        {
            SkrDelete(proxy);
        }

        _used_proxies.clear();
        _free_proxies.clear();
    }

private:
    Vector<V8BindProxy*> _free_proxies = {};
    Vector<V8BindProxy*> _used_proxies = {};
};

struct [[sattr(
    guid = "921187d2-4d38-42b5-81b1-cc79d5739cef"
)]] SKR_V8_API V8Isolate : IScriptMixinCore
{
    SKR_GENERATE_BODY(V8Isolate)
    SKR_DELETE_COPY_MOVE(V8Isolate)

    // ctor & dtor
    V8Isolate();
    ~V8Isolate();

    // init & shutdown
    void init();
    void shutdown();
    bool is_init() const;

    // isolate operators
    void pump_message_loop();
    void gc(bool full = true);

    // context management
    V8Context* main_context() const;
    V8Context* create_context(String name = {});
    void destroy_context(V8Context* context);

    // getter
    inline v8::Isolate* v8_isolate() const { return _isolate; }

    // invoke helper
    bool invoke_v8(
        v8::Local<v8::Value> v8_this,
        v8::Local<v8::Function> v8_func,
        Span<const StackProxy> params,
        StackProxy return_value
    );

    //==> IScriptMixinCore API
    void on_object_destroyed(
        ScriptbleObject* obj
    ) override;
    bool try_invoke_mixin(
        ScriptbleObject* obj,
        StringView name,
        const Span<const StackProxy> args,
        StackProxy result
    ) override;
    //==> IScriptMixinCore API

    // bind proxy management
    V8BindTemplate* solve_bind_tp(
        TypeSignatureView signature
    );
    V8BindTemplate* solve_bind_tp(
        const GUID& type_id
    );
    template <typename T>
    inline T* solve_bind_tp_as(const GUID& type_id)
    {
        return solve_bind_tp(type_id)->as<T>();
    }

    template <typename T>
    inline T* solve_bind_tp_as(TypeSignatureView type_id)
    {
        return solve_bind_tp(type_id)->as<T>();
    }

    // bind proxy pool
    template <std::derived_from<V8BindProxy> T>
    inline T* create_bind_proxy()
    {
        V8BindProxyPool& pool = _bind_proxy_pools.try_add_default(type_id_of<T>()).value();
        return pool.alloc<T>();
    }
    void destroy_bind_proxy(V8BindProxy* bind_proxy);

    // bind proxy management
    void register_bind_proxy(
        void* native_ptr,
        V8BindProxy* bind_proxy
    );
    void unregister_bind_proxy(
        void* native_ptr,
        V8BindProxy* bind_proxy
    );
    V8BindProxy* map_bind_proxy(
        void* native_ptr
    ) const;

    // 用于缓存调用期间为参数创建的临时 bind proxy
    void push_param_scope();
    void pop_param_scope();
    void push_param_proxy(
        V8BindProxy* bind_proxy
    );

    // debugger
    void init_debugger(int port = 9229);
    void shutdown_debugger();
    bool is_debugger_init() const;
    void pump_debugger_messages();
    void wait_for_debugger_connected(uint64_t timeout_ms = std::numeric_limits<uint64_t>::max());
    bool any_debugger_connected() const;

public:
    RC<IV8VFS> vfs = {};

private:
    // isolate data
    v8::Isolate* _isolate = nullptr;
    v8::Isolate::CreateParams _isolate_create_params = {};

    // bind tp cache
    Map<GUID, V8BindTemplate*> _bind_tp_map = {};
    Map<TypeSignature, V8BindTemplate*> _bind_tp_map_generic = {};

    // bind proxy pool
    Map<GUID, V8BindProxyPool> _bind_proxy_pools = {};

    // bind proxy cache
    Map<void*, V8BindProxy*> _bind_proxy_map = {};

    // context manage
    V8Context* _main_context = nullptr;
    Map<String, V8Context*> _contexts = {};

    // call v8 bind proxy manage
    Vector<V8BindProxy*> _call_v8_param_proxy = {};
    Vector<uint64_t> _call_v8_param_proxy_stack = {};

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