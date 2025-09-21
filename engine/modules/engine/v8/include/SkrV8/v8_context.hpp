#pragma once
#include <SkrV8/v8_fwd.hpp>
#include <SkrV8/v8_bind.hpp>
#include <SkrV8/v8_virtual_module.hpp>
#include <SkrV8/v8_value.hpp>
#include <SkrCore/memory/rc.hpp>
#include <SkrRTTR/script/stack_proxy.hpp>

// v8 includes
#include <v8-isolate.h>
#include <v8-platform.h>
#include <v8-primitive.h>

namespace skr
{
struct SKR_V8_API V8Context
{
    SKR_RC_IMPL();

    friend struct V8Isolate;

    // ctor & dtor
    V8Context();
    ~V8Context();

    // delete copy & move
    V8Context(const V8Context&) = delete;
    V8Context(V8Context&&) = delete;
    V8Context& operator=(const V8Context&) = delete;
    V8Context& operator=(V8Context&&) = delete;

    // getter
    ::v8::Global<::v8::Context> v8_context() const;
    inline V8Isolate* isolate() const { return _isolate; }
    inline const String& name() const { return _name; }

    // context op
    void enter();
    void exit();

    // build export
    void build_export(FunctionRef<void(V8VirtualModule&)> build_func);
    bool is_export_built() const;
    void clear_export();
    inline const V8VirtualModule& virtual_module() const { return _virtual_module; }

    // set & get global value
    V8Value get_global(StringView name);
    bool set_global_value(StringView name, const V8Value& value);
    template <typename T>
    inline bool set_global(StringView name, const T& value)
    {
        V8Value v(this);
        if (!v.set<T>(value)) { return false; }
        return set_global_value(name, v);
    }

    // exec
    V8Value exec(StringView script, bool as_module = false);
    V8Value exec_file(StringView file_path, bool as_module = true);

private:
    // init & shutdown
    void _init(V8Isolate* isolate, String name);
    void _shutdown();

    // exec helpers
    v8::MaybeLocal<v8::Script> _compile_script(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context,
        StringView script,
        StringView path
    );
    v8::MaybeLocal<v8::Module> _compile_module(
        v8::Isolate* isolate,
        StringView script,
        StringView path
    );
    V8Value _exec_script(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context,
        v8::Local<v8::Script> script,
        bool dump_exception
    );
    V8Value _exec_module(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context,
        v8::Local<v8::Module> module,
        bool dump_exception
    );

    // callback
    static v8::MaybeLocal<v8::Module> _resolve_module(
        v8::Local<v8::Context> context,
        v8::Local<v8::String> specifier,
        v8::Local<v8::FixedArray> import_assertions,
        v8::Local<v8::Module> referrer
    );

private:
    V8Isolate* _isolate = nullptr;
    v8::Persistent<v8::Context> _context = {};
    String _name = {};
    V8VirtualModule _virtual_module = {};

    // module/script cache
    Map<String, v8::Global<v8::Module>> _path_to_module = {};
    Map<int, String> _module_id_to_path = {};
};
} // namespace skr