#include "cgpu/api.h"
#include "utils.h"
#include "platform/thread.h"
#include <filesystem>
#include <fstream>
#include "filewatch.hpp"

// Watch triangle_module32.wasm file automatically
struct WasmWatcher {
    WasmWatcher()
    {
        auto cpath = std::filesystem::absolute("triangle_module32.wasm");
        watcher = new filewatch::FileWatch<std::filesystem::path>(cpath,
            [this](const std::filesystem::path& path, const filewatch::Event change_type) {
                if (change_type == filewatch::Event::renamed_new)
                {
                    skr_thread_sleep(50);
                    std::ifstream ifs(path.c_str());
                    this->content = std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
                    updated = true;
                }
            });
        std::ifstream ifs(cpath.c_str());
        content = std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        updated = true;
    }
    ~WasmWatcher()
    {
        if (wa_module) swa_free_module(wa_module);
        if (wa_runtime) swa_free_runtime(wa_runtime);
        if (wa_instance) swa_free_instance(wa_instance);
        delete watcher;
    }
    SWAModuleId glob_latest_available()
    {
        if (updated)
        {
            if (wa_module) swa_free_module(wa_module);
            wa_module = nullptr;
            if (wa_runtime) swa_free_runtime(wa_runtime);
            wa_runtime = nullptr;
            if (wa_instance) swa_free_instance(wa_instance);
            wa_instance = nullptr;

            SWAInstanceDescriptor inst_desc = { ESWA_BACKEND_WASM3 };
            wa_instance = swa_create_instance(&inst_desc);
            SWARuntimeDescriptor runtime_desc = { "wa_runtime", 64 * 1024 };
            wa_runtime = swa_create_runtime(wa_instance, &runtime_desc);
            SWAModuleDescriptor module_desc;
            module_desc.name = "raster_cmd";
            module_desc.wasm = (const uint8_t*)content.c_str();
            module_desc.wasm_size = (uint32_t)content.size();
            module_desc.bytes_pinned_outside = true;
            module_desc.strong_stub = false;
            wa_module = swa_create_module(wa_runtime, &module_desc);

            swa::utilx::link(wa_module, "env", "cgpu_render_encoder_set_viewport", cgpu_render_encoder_set_viewport);
            swa::utilx::link(wa_module, "env", "cgpu_render_encoder_set_scissor", cgpu_render_encoder_set_scissor);
            swa::utilx::link(wa_module, "env", "cgpu_render_encoder_bind_pipeline", cgpu_render_encoder_bind_pipeline);
            swa::utilx::link(wa_module, "env", "cgpu_render_encoder_draw", cgpu_render_encoder_draw);
            updated = false;
        }
        return wa_module;
    }
    std::atomic_bool updated = false;
    filewatch::FileWatch<std::filesystem::path>* watcher = nullptr;
    std::string content;
    SWAInstanceId wa_instance = nullptr;
    SWARuntimeId wa_runtime = nullptr;
    SWAModuleId wa_module = nullptr;
};

SWAModuleId get_available_wasm(void* watcher)
{
    WasmWatcher* W = (WasmWatcher*)watcher;
    return W->glob_latest_available();
}

void* watch_wasm()
{
    return new WasmWatcher();
}

void unwatch_wasm(void* watcher)
{
    delete (WasmWatcher*)watcher;
}

struct SourceWatcher {
    void Compile()
    {
        auto bat_dir = watch_path / "compile_32.bat";
        auto dirstr = bat_dir.string();
        std::string cmdstr;
        {
            std::filesystem::path valid = dirstr;
            cmdstr = cmdstr.append(valid.string());
            system(cmdstr.c_str());
            std::cout << "Recompiled with " << cmdstr << "\n";
        }
    }

    SourceWatcher(std::filesystem::path path)
        : watch_path(path)
    {
        watcher = new filewatch::FileWatch<std::filesystem::path>(path,
            [this](const std::filesystem::path& path, const filewatch::Event change_type) {
                if (change_type == filewatch::Event::modified)
                {
                    auto pathstr = path.string();
                    if (pathstr.find(".wa.") != std::string::npos)
                    {
                        Compile();
                    }
                }
            });
        Compile();
    }
    ~SourceWatcher()
    {
        delete watcher;
    }
    filewatch::FileWatch<std::filesystem::path>* watcher;
    std::filesystem::path watch_path;
};

void* watch_source()
{
    try
    {
        std::filesystem::path watch_path = __FILE__;
        watch_path = watch_path.parent_path();
        return new SourceWatcher(watch_path);
    } //
    catch (std::system_error err)
    {
        std::cout << err.what() << std::endl;
    }
    return nullptr;
}

void unwatch_source(void* watcher)
{
    delete (SourceWatcher*)watcher;
}