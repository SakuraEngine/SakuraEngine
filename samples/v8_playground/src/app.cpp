#include "SkrV8/ts_define_exporter.hpp"
#include "v8-exception.h"
#include <V8Playground/app.hpp>
#include <filesystem>

namespace skr
{
// ctor & dtor
V8PlaygroundApp::V8PlaygroundApp()
{
}
V8PlaygroundApp::~V8PlaygroundApp()
{
    shutdown();
}

// env init & shutdown
void V8PlaygroundApp::env_init()
{
    init_v8();
}
void V8PlaygroundApp::env_shutdown()
{
    shutdown_v8();
}

// init & shutdown
void V8PlaygroundApp::init()
{
    shutdown();
    _isolate = SkrNew<V8Isolate>();
    _isolate->init();
}
void V8PlaygroundApp::shutdown()
{
    if (_isolate)
    {
        _isolate->shutdown();
        SkrDelete(_isolate);
        _isolate      = nullptr;
    }
}

// debug
void V8PlaygroundApp::init_debugger(int port)
{
    _isolate->init_debugger(port);
}
void V8PlaygroundApp::shutdown_debugger()
{
    _isolate->shutdown_debugger();
}
void V8PlaygroundApp::pump_debugger_messages()
{
    _isolate->pump_debugger_messages();
}
void V8PlaygroundApp::wait_for_debugger_connected()
{
    _isolate->wait_for_debugger_connected();
}

// load native
void V8PlaygroundApp::load_native_types()
{
    load_all_types();
    _isolate->main_context()->build_global_export([](ScriptModule& module) {
        each_types_of_module(u8"V8Playground", [&](const RTTRType* type) -> bool {
            bool do_export =
                (type->is_record() && flag_all(type->record_flag(), ERTTRRecordFlag::ScriptVisible)) ||
                (type->is_enum() && flag_all(type->enum_flag(), ERTTREnumFlag::ScriptVisible));

            if (do_export)
            {
                auto ns = type->name_space_str();
                ns.remove_prefix(u8"skr");
                module.register_type(type, ns);
            }
            return true;
        });
    });
}

// run script
bool V8PlaygroundApp::run_script(StringView script_path)
{
    // load script
    String script = _load_from_file(script_path);
    if (script.is_empty())
    {
        SKR_LOG_FMT_ERROR(u8"Failed to load script: {}", script_path);
        return false;
    }

    // // scopes
    // auto                   isolate = _isolate->v8_isolate();
    // v8::Isolate::Scope     isolate_scope(isolate);
    // v8::HandleScope        handle_scope(isolate);
    // v8::Local<v8::Context> context = _main_context->v8_context().Get(isolate);
    // v8::Context::Scope     context_scope(context);

    // v8::TryCatch try_catch(isolate);

    // run script
    _isolate->main_context()->exec_module(script, script_path);

    // auto _inspect_str = +[](const skr_char8* str) {
    //     return v8_inspector::StringView{
    //         (const uint8_t*)str,
    //         strlen((const char*)str)
    //     };
    // };
    // auto _skr_str = [&](v8::Local<v8::Value> str) {
    //     return String::From(*v8::String::Utf8Value(isolate, str));
    // };

    // if (try_catch.HasCaught())
    // {
    //     String detail = _skr_str(try_catch.Message()->Get());
    //     String url    = _skr_str(try_catch.Message()->GetScriptResourceName());

    //     _inspector_client.v8_inspector()->exceptionThrown(
    //         _main_context->v8_context().Get(_isolate->v8_isolate()),
    //         _inspect_str(u8"uncaught exception"),
    //         try_catch.Exception(),
    //         _inspect_str(detail.c_str()),
    //         _inspect_str(url.c_str()),
    //         try_catch.Message()->GetLineNumber(context).FromMaybe(0),
    //         try_catch.Message()->GetStartColumn(context).FromMaybe(0),
    //         _inspector_client.v8_inspector()->createStackTrace(try_catch.Message()->GetStackTrace()),
    //         0
    //     );
    //     // _inspector_client.runMessageLoopOnPause(0);
    // }

    return true;
}

// dump types
bool V8PlaygroundApp::dump_types(StringView output_dir)
{
    // get name
    std::filesystem::path out_dir_path(output_dir.data_raw());
    std::filesystem::path out_file_path = out_dir_path / "global.d.ts";

    // create dir
    if (!std::filesystem::exists(out_dir_path))
    {
        if (!std::filesystem::create_directories(out_dir_path))
        {
            SKR_LOG_FMT_ERROR(u8"Failed to create dir: {}", out_dir_path.string());
            return false;
        }
    }
    else if (!std::filesystem::is_directory(out_dir_path))
    {
        SKR_LOG_FMT_ERROR(u8"Path is not a directory: {}", out_dir_path.string());
        return false;
    }

    // do export
    TSDefineExporter exporter;
    exporter.module = &_isolate->main_context()->global_module();
    return _save_to_file(String::From(out_file_path.c_str()), exporter.generate_global());
}

// helper
String V8PlaygroundApp::_load_from_file(StringView path)
{
    String result;
    auto   handle = fopen(path.data_raw(), "rb");
    if (handle)
    {
        // resize string to file size
        fseek(handle, 0, SEEK_END);
        auto size = ftell(handle);
        fseek(handle, 0, SEEK_SET);
        result.resize_unsafe(size);

        // read file to string
        fread(result.data_w(), 1, size, handle);
        fclose(handle);
    }
    else
    {
        return {};
    }
    return result;
}
bool V8PlaygroundApp::_save_to_file(StringView _path, StringView _content)
{
    auto handle = fopen(_path.data_raw(), "wb");
    if (!handle) { return false; }

    fwrite(_content.data(), 1, _content.size(), handle);
    fclose(handle);
    return true;
}
} // namespace skr