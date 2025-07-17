#pragma once
#include <SkrRTTR/script/script_tools.hpp>
#include <SkrRTTR/script/scriptble_object.hpp>
#include <SkrCore/log.hpp>
#include <SkrV8/v8_bind.hpp>
#include <SkrV8/v8_isolate.hpp>

// v8 includes
#include <v8-function.h>
#include <v8-context.h>
#include <v8-isolate.h>
#include <v8-persistent-handle.h>

namespace skr
{
struct V8Isolate;
struct V8Context;

// value tool
struct V8Value {
    bool is_empty() const;

    template <typename T>
    Optional<T> get() const;

    void reset();

    template <typename Ret, typename... Args>
    decltype(auto) call(Args&&... args) const;

    v8::Global<v8::Value> v8_value = {};
    V8Context*            context  = nullptr;
};

struct SKR_V8_API V8Context {
    SKR_RC_IMPL();
    friend struct V8Value;
    friend struct V8Isolate;

private:
    // setup by isolate
    void _init_basic(V8Isolate* isolate, String name);

public:
    // ctor & dtor
    V8Context();
    ~V8Context();

    // delete copy & move
    V8Context(const V8Context&)            = delete;
    V8Context(V8Context&&)                 = delete;
    V8Context& operator=(const V8Context&) = delete;
    V8Context& operator=(V8Context&&)      = delete;

    // init & shutdown
    void init();
    void shutdown();

    // build export
    bool build_global_export(FunctionRef<void(ScriptModule& module)> build_func);

    // getter
    ::v8::Global<::v8::Context> v8_context() const;
    inline V8Isolate*           isolate() const { return _isolate; }
    inline const ScriptModule&  global_module() const { return _global_module; }
    inline const String&        name() const { return _name; }

    // set global value
    template <typename T>
    void set_global(StringView name, T&& v);

    // get global value
    V8Value get_global(StringView name);

    // run as script
    V8Value exec_script(StringView script, StringView file_path = {});

    // run as ES module
    V8Value exec_module(StringView script, StringView file_path = {});

private:
    // callback
    static v8::MaybeLocal<v8::Module> _resolve_module(
        v8::Local<v8::Context>    context,
        v8::Local<v8::String>     specifier,
        v8::Local<v8::FixedArray> import_assertions,
        v8::Local<v8::Module>     referrer
    );

private:
    // owner
    V8Isolate* _isolate;

    // namespace tools
    ScriptModule _global_module;

    // context data
    v8::Persistent<v8::Context> _context;

    // name (for debug)
    String _name;
};
} // namespace skr

namespace skr
{
inline bool V8Value::is_empty() const
{
    return v8_value.IsEmpty();
}
template <typename T>
inline Optional<T> V8Value::get() const
{
    using namespace ::v8;

    // scopes
    auto*          isolate = context->isolate()->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope    handle_scope(isolate);

    // solve context
    Local<Context> solved_context = context->v8_context().Get(isolate);
    Context::Scope context_scope(solved_context);
    auto*          skr_isolate     = context->isolate();
    Local<Value>   solved_v8_value = v8_value.Get(isolate);

    if constexpr (
        std::is_integral_v<T> ||
        std::is_floating_point_v<T> ||
        std::is_same_v<T, bool> ||
        std::is_same_v<T, skr::String> ||
        std::is_same_v<T, skr::StringView>
    )
    {
        T result;
        if (V8Bind::to_native(solved_v8_value, result))
        {
            return { result };
        }
        else
        {
            return {};
        }
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        using RawType = std::remove_pointer_t<T>;

        if constexpr (std::derived_from<RawType, ScriptbleObject>)
        {
            T result;
            if (skr_isolate->to_native(type_of<RawType>(), &result, solved_v8_value, false))
            {
                return { result };
            }
            else
            {
                return {};
            }
        }
        else
        {
            return {};
        }
    }
    else
    {
        T                     result;
        TypeSignatureTyped<T> type_sig;
        if (skr_isolate->to_native(type_sig.view(), &result, solved_v8_value, true))
        {
            return { result };
        }
        else
        {
            return {};
        }
    }
}
inline void V8Value::reset()
{
    v8_value.Reset();
    context = nullptr;
}
template <typename Ret, typename... Args>
inline decltype(auto) V8Value::call(Args&&... args) const
{
    using namespace ::v8;

    // scopes
    auto*          isolate = context->isolate()->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope    handle_scope(isolate);

    // solve context
    Local<Context> solved_context = context->v8_context().Get(isolate);
    Context::Scope context_scope(solved_context);

    if constexpr (std::is_same_v<Ret, void>)
    {
        auto v8_value_local = v8_value.Get(isolate);
        if (!v8_value_local->IsFunction())
        {
            return false;
        }

        // call function
        auto* skr_isolate = context->isolate();
        skr_isolate->invoke_v8(
            context->v8_context().Get(isolate)->Global(),
            v8_value_local.As<v8::Function>(),
            { StackProxyMaker<Args>::Make(std::forward<Args>(args))... },
            {}
        );

        return true;
    }
    else
    {
        auto v8_value_local = v8_value.Get(isolate);
        if (!v8_value_local->IsFunction())
        {
            return Optional<Ret>{};
        }

        // call function
        auto*            skr_isolate = context->isolate();
        Placeholder<Ret> result_holder;
        bool             call_success = skr_isolate->invoke_v8(
            context->v8_context().Get(isolate)->Global(),
            v8_value_local.As<v8::Function>(),
            { StackProxyMaker<Args>::Make(std::forward<Args>(args))... },
            { .data = result_holder.data(), .signature = type_signature_of<Ret>() }
        );

        if (!call_success)
        {
            return Optional<Ret>{};
        }
        else
        {
            return Optional<Ret>{ std::move(*result_holder.data_typed()) };
        }
    }
}

template <typename T>
inline void V8Context::set_global(StringView name, T&& v)
{
    using DecayType = std::decay_t<T>;

    using namespace ::v8;

    // scopes
    Isolate::Scope isolate_scope(_isolate->v8_isolate());
    HandleScope    handle_scope(_isolate->v8_isolate());

    // solve context
    Local<Context> solved_context = _context.Get(_isolate->v8_isolate());
    Context::Scope context_scope(solved_context);
    Local<Object>  global = solved_context->Global();

    // translate value
    Local<Value> value;
    if constexpr (
        std::is_integral_v<DecayType> ||
        std::is_floating_point_v<DecayType> ||
        std::is_same_v<DecayType, bool> ||
        std::is_same_v<DecayType, skr::String> ||
        std::is_same_v<DecayType, skr::StringView>
    )
    {
        value = V8Bind::to_v8(v);
    }
    else if constexpr (std::is_pointer_v<DecayType>)
    {
        using RawType = std::remove_pointer_t<DecayType>;

        if constexpr (std::derived_from<RawType, ScriptbleObject>)
        {
            value = _isolate->translate_object(v)->v8_object.Get(_isolate->v8_isolate());
        }
        else
        {
            value = _isolate->create_value(type_of<RawType>(), v)->v8_object.Get(_isolate->v8_isolate());
        }
    }
    else
    {
        value = _isolate->create_value(type_of<DecayType>(), &v)->v8_object.Get(_isolate->v8_isolate());
    }

    // set value
    // clang-format off
    global->Set(
        solved_context,
        V8Bind::to_v8(name, true),
        value
    ).Check();
    // clang-format on
}
} // namespace skr