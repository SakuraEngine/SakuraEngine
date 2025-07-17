#include <SkrV8/v8_context.hpp>
#include <SkrCore/log.hpp>
#include <SkrV8/v8_isolate.hpp>
#include <SkrV8/v8_bind.hpp>
#include <SkrV8/v8_module.hpp>

#include <v8-exception.h>
#include <v8-function.h>

namespace skr
{
// setup by isolate
void V8Context::_init_basic(V8Isolate* isolate, String name)
{
    _isolate               = isolate;
    _name                  = name;
    _global_module.manager = &_isolate->script_binder_manger();
}

// ctor & dtor
V8Context::V8Context()
{
}
V8Context::~V8Context()
{
}

// init & shutdown
void V8Context::init()
{
    using namespace ::v8;
    Isolate::Scope isolate_scope(_isolate->v8_isolate());
    HandleScope    handle_scope(_isolate->v8_isolate());

    // create context
    auto new_context = Context::New(_isolate->v8_isolate());
    _context.Reset(_isolate->v8_isolate(), new_context);

    // bind this
    new_context->SetAlignedPointerInEmbedderData(1, this);
}
void V8Context::shutdown()
{
    // destroy context
    _context.Reset();
}

// build export
bool V8Context::build_global_export(FunctionRef<void(ScriptModule& module)> build_func)
{
    // build global module
    build_func(_global_module);

    // finalize
    {
        auto               isolate = _isolate->v8_isolate();
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope    handle_scope(isolate);
        auto               context = _context.Get(isolate);
        v8::Context::Scope context_scope(context);

        if (!_global_module.check_full_export())
        {
            // dump lost types
            for (const auto& lost_item : _global_module.lost_types())
            {
                String type_name;
                switch (lost_item.kind())
                {
                case ScriptBinderRoot::EKind::Object: {
                    auto* type = lost_item.object()->type;
                    type_name  = format(u8"{}::{}", type->name_space_str(), type->name());
                    break;
                }
                case ScriptBinderRoot::EKind::Value: {
                    auto* type = lost_item.value()->type;
                    type_name  = format(u8"{}::{}", type->name_space_str(), type->name());
                    break;
                }
                case ScriptBinderRoot::EKind::Enum: {
                    auto* type = lost_item.enum_()->type;
                    type_name  = format(u8"{}::{}", type->name_space_str(), type->name());
                    break;
                }
                default:
                    SKR_UNREACHABLE_CODE()
                    break;
                }

                SKR_LOG_FMT_ERROR(u8"lost type {} when export context global data", type_name);
            }
            return false;
        }
        else
        {
            for (const auto& [k, v] : _global_module.ns_mapper().root().children())
            {
                // clang-format off
                context->Global()->Set(
                    context,
                    V8Bind::to_v8(k, true),
                    V8Bind::export_namespace_node(v, _isolate)
                ).Check();
                // clang-format on
            }

            return true;
        }
    }
}

// getter
::v8::Global<::v8::Context> V8Context::v8_context() const
{
    return ::v8::Global<::v8::Context>(_isolate->v8_isolate(), _context);
}

// get global value
V8Value V8Context::get_global(StringView name)
{
    // scopes
    auto                   isolate = _isolate->v8_isolate();
    v8::Isolate::Scope     isolate_scope(isolate);
    v8::HandleScope        handle_scope(isolate);
    v8::Local<v8::Context> context = _context.Get(isolate);
    v8::Context::Scope     context_scope(context);

    // find value
    auto                  global      = context->Global();
    v8::Local<v8::String> name_v8     = V8Bind::to_v8(name, true);
    auto                  maybe_value = global->Get(context, name_v8);
    if (maybe_value.IsEmpty())
    {
        SKR_LOG_FMT_ERROR(u8"failed to get global value {}", name);
        return {};
    }

    // return value
    V8Value result;
    result.context = this;
    result.v8_value.Reset(isolate, maybe_value.ToLocalChecked());
    return result;
}

// run as script
V8Value V8Context::exec_script(StringView script, StringView file_path)
{
    auto                   isolate = _isolate->v8_isolate();
    v8::Isolate::Scope     isolate_scope(isolate);
    v8::HandleScope        handle_scope(isolate);
    v8::Local<v8::Context> context = _context.Get(isolate);
    v8::Context::Scope     context_scope(context);
    v8::TryCatch           try_catch(isolate);

    // compile script
    v8::Local<v8::String> source = V8Bind::to_v8(script, false);
    v8::ScriptOrigin      origin(
        isolate,
        V8Bind::to_v8(file_path.is_empty() ? u8"[CPP]" : file_path),
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
    auto may_be_script = ::v8::Script::Compile(
        context,
        source
    );
    if (may_be_script.IsEmpty())
    {
        SKR_LOG_ERROR(u8"compile script failed");
        return {};
    }

    // run script
    auto compiled_script = may_be_script.ToLocalChecked();
    auto exec_result     = compiled_script->Run(context);

    // dump exception
    if (try_catch.HasCaught())
    {
        String exception_str;
        V8Bind::to_native(try_catch.Exception()->ToString(context).ToLocalChecked(), exception_str);
        SKR_LOG_FMT_ERROR(u8"[V8] uncaught exception: {}\n  at: {}", exception_str.c_str(), file_path);
    }

    // return result
    if (!exec_result.IsEmpty())
    {
        V8Value result;
        result.context = this;
        result.v8_value.Reset(_isolate->v8_isolate(), exec_result.ToLocalChecked());
        return result;
    }

    return {};
}

// run as ES module
V8Value V8Context::exec_module(StringView script, StringView file_path)
{
    auto                   isolate = _isolate->v8_isolate();
    v8::Isolate::Scope     isolate_scope(isolate);
    v8::HandleScope        handle_scope(isolate);
    v8::Local<v8::Context> context = _context.Get(isolate);
    v8::Context::Scope     context_scope(context);
    v8::TryCatch           try_catch(isolate);

    // compile module
    v8::ScriptOrigin origin(
        isolate,
        V8Bind::to_v8(file_path.is_empty() ? u8"[CPP]" : file_path),
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
    auto                         source_str = V8Bind::to_v8(script, false);
    ::v8::ScriptCompiler::Source source(source_str, origin);
    auto                         maybe_module = ::v8::ScriptCompiler::CompileModule(
        isolate,
        &source,
        v8::ScriptCompiler::kNoCompileOptions
    );
    if (maybe_module.IsEmpty())
    {
        SKR_LOG_ERROR(u8"compile module failed");
        return {};
    }

    // instantiate module
    auto module             = maybe_module.ToLocalChecked();
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
        // won't stop, see https://github.com/nodejs/node/issues/50430
        _isolate->v8_isolate()->ThrowException(module->GetException());
    }

    // finish promise
    // TODO. generic promise api
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
        String exception_str;
        V8Bind::to_native(try_catch.Exception()->ToString(context).ToLocalChecked(), exception_str);
        SKR_LOG_FMT_ERROR(u8"[V8] uncaught exception: {}\n  at: {}", exception_str.c_str(), file_path);
        try_catch.ReThrow();
    }

    // return result
    if (!eval_result.IsEmpty())
    {
        V8Value result;
        result.context = this;
        result.v8_value.Reset(_isolate->v8_isolate(), eval_result.ToLocalChecked());
        return result;
    }

    return {};
}

// callback
v8::MaybeLocal<v8::Module> V8Context::_resolve_module(
    v8::Local<v8::Context>    context,
    v8::Local<v8::String>     specifier,
    v8::Local<v8::FixedArray> import_assertions,
    v8::Local<v8::Module>     referrer
)
{
    auto isolate     = v8::Isolate::GetCurrent();
    auto skr_isolate = reinterpret_cast<V8Isolate*>(isolate->GetData(0));

    // get module name
    skr::String module_name;
    if (!V8Bind::to_native(specifier, module_name))
    {
        isolate->ThrowException(V8Bind::to_v8(u8"failed to convert module name"));
        SKR_LOG_ERROR(u8"failed to convert module name");
        return {};
    }

    // solve module kind
    bool is_cpp_module = true;

    // solve module
    if (is_cpp_module)
    { // cpp module
        // find module
        auto found_module = skr_isolate->find_cpp_module(module_name);
        if (!found_module)
        {
            isolate->ThrowException(V8Bind::to_v8(u8"cannot find module"));
            SKR_LOG_FMT_ERROR(u8"module {} not found", module_name);
            return {};
        }

        // return module
        return found_module->v8_module();
    }
    else
    {
        return {};
    }
}

} // namespace skr