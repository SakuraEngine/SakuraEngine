#include <SkrV8/v8_context.hpp>
#include <SkrContainers/set.hpp>
#include <SkrCore/log.hpp>
#include <SkrRTTR/type.hpp>
#include <SkrV8/v8_bind.hpp>
#include <SkrV8/v8_isolate.hpp>
#include <SkrV8/v8_vfs.hpp>

// v8 includes
#include <libplatform/libplatform.h>
#include <v8-initialization.h>
#include <v8-template.h>
#include <v8-external.h>
#include <v8-function.h>
#include <v8-container.h>
#include <v8-exception.h>

namespace skr
{
// ctor & dtor
V8Context::V8Context()
{
}
V8Context::~V8Context()
{
}

::v8::Global<::v8::Context> V8Context::v8_context() const
{
    return ::v8::Global<::v8::Context>(_isolate->v8_isolate(), _context);
}

// context op
void V8Context::enter()
{
    using namespace ::v8;
    HandleScope handle_scope(_isolate->v8_isolate());
    _context.Get(_isolate->v8_isolate())->Enter();
}
void V8Context::exit()
{
    using namespace ::v8;
    HandleScope handle_scope(_isolate->v8_isolate());
    _context.Get(_isolate->v8_isolate())->Exit();
}

// build export
void V8Context::build_export(FunctionRef<void(V8VirtualModule&)> build_func)
{
    auto isolate = _isolate->v8_isolate();
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    auto context = _context.Get(isolate);
    v8::Context::Scope context_scope(context);

    // build module
    build_func(_virtual_module);
    _virtual_module.dump_bind_tp_error();

    // setup context
    _virtual_module.export_v8_to(
        isolate,
        context,
        _context.Get(isolate)->Global()
    );
}
bool V8Context::is_export_built() const
{
    return !_virtual_module.is_empty();
}
void V8Context::clear_export()
{
    if (is_export_built())
    {
        _virtual_module.erase_export(
            _isolate->v8_isolate(),
            _context.Get(_isolate->v8_isolate()),
            _context.Get(_isolate->v8_isolate())->Global()
        );
    }
}

// set & get global value
V8Value V8Context::get_global(StringView name)
{
    using namespace ::v8;

    // scopes
    auto isolate = _isolate->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = _context.Get(isolate);
    Context::Scope context_scope(context);

    // find value
    auto maybe_value = context->Global()->Get(
        context,
        V8Bind::to_v8(name, true)
    );
    if (maybe_value.IsEmpty())
    {
        return {};
    }
    else
    {
        auto value = maybe_value.ToLocalChecked();
        if (value->IsNullOrUndefined()) { return {}; }
        return {
            { isolate, value },
            this
        };
    }
}
bool V8Context::set_global_value(StringView name, const V8Value& value)
{
    using namespace ::v8;

    // scopes
    auto isolate = _isolate->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = _context.Get(isolate);
    Context::Scope context_scope(context);

    auto result = context->Global()->Set(
        context,
        V8Bind::to_v8(name, true),
        value.v8_value().Get(isolate)
    );
    return result.IsJust();
}

// exec
V8Value V8Context::exec(StringView script, bool as_module)
{
    using namespace ::v8;

    auto isolate = _isolate->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = _context.Get(isolate);
    Context::Scope context_scope(context);

    if (as_module)
    {
        // compile module
        auto maybe_module = _compile_module(
            isolate,
            script,
            u8"[CPP]"
        );
        if (maybe_module.IsEmpty())
        {
            SKR_LOG_ERROR(u8"compile module failed");
            return {};
        }

        // run module
        return _exec_module(
            isolate,
            context,
            maybe_module.ToLocalChecked(),
            true
        );
    }
    else
    {
        // compile script
        auto maybe_script = _compile_script(
            isolate,
            context,
            script,
            u8"[CPP]"
        );
        if (maybe_script.IsEmpty())
        {
            SKR_LOG_ERROR(u8"compile script failed");
            return {};
        }

        // run script
        return _exec_script(
            isolate,
            context,
            maybe_script.ToLocalChecked(),
            true
        );
    }
}
V8Value V8Context::exec_file(StringView file_path, bool as_module)
{
    using namespace ::v8;

    auto isolate = _isolate->v8_isolate();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = _context.Get(isolate);
    Context::Scope context_scope(context);

    // check vfs and load script
    if (!_isolate->vfs)
    {
        SKR_LOG_ERROR(u8"vfs is not set, cannot load script from file");
        return {};
    }
    auto normalized_path = _isolate->vfs->path_normalize(file_path);
    auto script_content = _isolate->vfs->load_script(normalized_path);
    if (!script_content)
    {
        SKR_LOG_FMT_ERROR(u8"failed to load script from file: {}", normalized_path.c_str());
        return {};
    }

    if (as_module)
    {
        // compile module
        auto maybe_module = _compile_module(
            isolate,
            script_content.value(),
            normalized_path
        );
        if (maybe_module.IsEmpty())
        {
            SKR_LOG_ERROR(u8"compile module failed");
            return {};
        }
        auto module = maybe_module.ToLocalChecked();

        // cache module path for evaluation
        _module_id_to_path.add(module->GetIdentityHash(), normalized_path);

        // run module
        auto run_result = _exec_module(
            isolate,
            context,
            module,
            true
        );

        // pop module path
        _module_id_to_path.remove(module->GetIdentityHash());

        return run_result;
    }
    else
    {
        // compile script
        auto maybe_script = _compile_script(
            isolate,
            context,
            script_content.value(),
            normalized_path
        );
        if (maybe_script.IsEmpty())
        {
            SKR_LOG_ERROR(u8"compile script failed");
            return {};
        }

        // run script
        return _exec_script(
            isolate,
            context,
            maybe_script.ToLocalChecked(),
            true
        );
    }

    return {};
}

// init & shutdown
void V8Context::_init(V8Isolate* isolate, String name)
{
    using namespace ::v8;

    _isolate = isolate;
    _name = name;
    _virtual_module.set_isolate(_isolate);

    Isolate::Scope isolate_scope(_isolate->v8_isolate());
    HandleScope handle_scope(_isolate->v8_isolate());

    // create context
    auto new_context = Context::New(_isolate->v8_isolate());
    _context.Reset(_isolate->v8_isolate(), new_context);

    // bind this
    new_context->SetAlignedPointerInEmbedderData(0, this);
}
void V8Context::_shutdown()
{
    // destroy context
    _context.Reset();
}

// exec helpers
v8::MaybeLocal<v8::Script> V8Context::_compile_script(
    v8::Isolate* isolate,
    v8::Local<v8::Context> context,
    StringView script,
    StringView path
)
{
    v8::Local<v8::String> source = V8Bind::to_v8(script, false);
    v8::ScriptOrigin origin(
        isolate,
        V8Bind::to_v8(path),
        0,
        0,
        true,
        -1,
        {},
        false,
        false,
        false,
        {}
    );
    return ::v8::Script::Compile(
        context,
        source
    );
}
v8::MaybeLocal<v8::Module> V8Context::_compile_module(
    v8::Isolate* isolate,
    StringView script,
    StringView path
)
{
    v8::ScriptOrigin origin(
        isolate,
        V8Bind::to_v8(path),
        0,
        0,
        true,
        -1,
        {},
        false,
        false,
        true,
        {}
    );
    ::v8::ScriptCompiler::Source source(
        V8Bind::to_v8(script),
        origin
    );
    return ::v8::ScriptCompiler::CompileModule(
        isolate,
        &source,
        v8::ScriptCompiler::kNoCompileOptions
    );
}
V8Value V8Context::_exec_script(
    v8::Isolate* isolate,
    v8::Local<v8::Context> context,
    v8::Local<v8::Script> script,
    bool dump_exception
)
{
    using namespace ::v8;
    TryCatch try_catch(isolate);

    // run script
    auto exec_result = script->Run(context);

    // dump exception
    if (try_catch.HasCaught())
    {
        if (dump_exception)
        {
            String exception_str;
            V8Bind::to_native(try_catch.Exception()->ToString(context).ToLocalChecked(), exception_str);
            SKR_LOG_FMT_ERROR(
                u8"[V8] uncaught exception: {}",
                exception_str.c_str()
            );
        }
        try_catch.ReThrow();
    }

    // return result
    if (exec_result.IsEmpty())
    {
        return {};
    }
    else
    {
        return {
            { isolate, exec_result.ToLocalChecked() },
            this
        };
    }
}
V8Value V8Context::_exec_module(
    v8::Isolate* isolate,
    v8::Local<v8::Context> context,
    v8::Local<v8::Module> module,
    bool dump_exception
)
{
    using namespace ::v8;
    TryCatch try_catch(isolate);

    // instantiate module
    auto instantiate_result = module->InstantiateModule(context, _resolve_module);
    if (instantiate_result.IsNothing())
    {
        SKR_LOG_ERROR(u8"instantiate module failed");
        return {};
    }

    // evaluate module
    auto eval_result = module->Evaluate(context);

    // check module error
    if (module->GetStatus() == v8::Module::kErrored)
    {
        // won't stop on first level exception, see https://github.com/nodejs/node/issues/50430
        _isolate->v8_isolate()->ThrowException(module->GetException());
    }

    // finish first level promise
    if (eval_result.ToLocalChecked()->IsPromise())
    {
        auto promise = eval_result.ToLocalChecked().As<v8::Promise>();
        while (promise->State() == v8::Promise::kPending)
        {
            _isolate->v8_isolate()->PerformMicrotaskCheckpoint();
        }
        if (promise->State() == v8::Promise::kRejected)
        {
            // promise->MarkAsHandled();
            _isolate->v8_isolate()->ThrowException(promise->Result());
        }
    }

    // dump exception
    if (try_catch.HasCaught())
    {
        if (dump_exception)
        {
            String exception_str;
            V8Bind::to_native(try_catch.Exception()->ToString(context).ToLocalChecked(), exception_str);
            SKR_LOG_FMT_ERROR(
                u8"[V8] uncaught exception: {}",
                exception_str.c_str()
            );
        }
        try_catch.ReThrow();
    }

    // return result
    if (eval_result.IsEmpty())
    {
        return {};
    }
    else
    {
        return {
            { isolate, eval_result.ToLocalChecked() },
            this
        };
    }
}

// callback
v8::MaybeLocal<v8::Module> V8Context::_resolve_module(
    v8::Local<v8::Context> context,
    v8::Local<v8::String> specifier,
    v8::Local<v8::FixedArray> import_assertions,
    v8::Local<v8::Module> referrer
)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto skr_isolate = reinterpret_cast<V8Isolate*>(isolate->GetData(0));
    auto skr_context = reinterpret_cast<V8Context*>(context->GetAlignedPointerFromEmbedderData(0));

    // get module name
    skr::String module_name;
    if (!V8Bind::to_native(specifier, module_name))
    {
        isolate->ThrowException(V8Bind::to_v8(u8"failed to convert module name"));
        SKR_LOG_ERROR(u8"failed to convert module name");
        return {};
    }

    // check module name
    bool is_relative_file =
        module_name.starts_with(u8"..") ||
        module_name.starts_with(u8".");

    // solve module
    if (is_relative_file)
    { // relative path
        // check vfs
        if (!skr_isolate->vfs)
        {
            isolate->ThrowException(
                V8Bind::to_v8(u8"vfs is not set, cannot load script from file")
            );
            SKR_LOG_ERROR(u8"vfs is not set, cannot load script from file");
            return {};
        }

        // get referrer module path
        auto find_referrer_path_result = skr_context->_module_id_to_path.find(referrer->GetIdentityHash());
        if (!find_referrer_path_result)
        {
            isolate->ThrowException(
                V8Bind::to_v8(u8"referrer module path not found")
            );
            return {};
        }

        // append .js suffix if needed
        // TODO. optimize it
        if (!module_name.ends_with(u8".js"))
        {
            module_name.append(u8".js");
        }

        // combine path and find cache
        auto full_path = skr_isolate->vfs->path_solve_relative_module(
            find_referrer_path_result.value(),
            module_name
        );
        auto find_module_result = skr_context->_path_to_module.find(full_path);
        if (find_module_result)
        { // found in cache
            return { find_module_result.value().Get(isolate) };
        }

        // load script
        auto script_content = skr_isolate->vfs->load_script(full_path);
        if (!script_content)
        {
            isolate->ThrowException(
                V8Bind::to_v8(skr::format(u8"failed to load script from file: {}", full_path))
            );
            SKR_LOG_FMT_ERROR(u8"failed to load script from file: {}", full_path);
            return {};
        }

        // compile module
        auto maybe_module = skr_context->_compile_module(
            isolate,
            script_content.value(),
            full_path
        );
        if (maybe_module.IsEmpty())
        {
            SKR_LOG_ERROR(u8"compile module failed");
            return {};
        }
        auto module = maybe_module.ToLocalChecked();

        // cache module id to path
        skr_context->_module_id_to_path.add(module->GetIdentityHash(), full_path);

        // evaluate module
        auto run_result = skr_context->_exec_module(
            isolate,
            context,
            module,
            true
        );

        // cache module
        skr_context->_path_to_module.add(full_path, v8::Global<v8::Module>(isolate, module));

        return module;
    }
    else
    { // absolute path
        // check vfs
        if (!skr_isolate->vfs)
        {
            isolate->ThrowException(V8Bind::to_v8(u8"vfs is not set, cannot load script from file"));
            SKR_LOG_ERROR(u8"vfs is not set, cannot load script from file");
            return {};
        }

        // append .js suffix if needed
        // TODO. optimize it
        if (!module_name.ends_with(u8".js"))
        {
            module_name.append(u8".js");
        }

        // normalize path and find cache
        auto normalized_path = skr_isolate->vfs->path_normalize(module_name);
        auto find_module_result = skr_context->_path_to_module.find(normalized_path);
        if (find_module_result)
        { // found in cache
            return { find_module_result.value().Get(isolate) };
        }

        // load script
        auto script_content = skr_isolate->vfs->load_script(normalized_path);
        if (!script_content)
        {
            isolate->ThrowException(
                V8Bind::to_v8(skr::format(u8"failed to load script from file: {}", normalized_path))
            );
            SKR_LOG_FMT_ERROR(u8"failed to load script from file: {}", normalized_path);
            return {};
        }

        // compile module
        auto maybe_module = skr_context->_compile_module(
            isolate,
            script_content.value(),
            normalized_path
        );
        if (maybe_module.IsEmpty())
        {
            SKR_LOG_ERROR(u8"compile module failed");
            return {};
        }
        auto module = maybe_module.ToLocalChecked();

        // cache module id to path
        skr_context->_module_id_to_path.add(module->GetIdentityHash(), normalized_path);

        // evaluate module
        auto run_result = skr_context->_exec_module(
            isolate,
            context,
            module,
            true
        );

        // cache module
        skr_context->_path_to_module.add(normalized_path, v8::Global<v8::Module>(isolate, module));

        return module;
    }
}
} // namespace skr