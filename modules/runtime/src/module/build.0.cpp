#include "module_manager.cpp"
#include "runtime_module.h"
#include "ecs/dual.h"

IMPLEMENT_DYNAMIC_MODULE(SkrRuntimeModule, SkrRT);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "SkrRT",
    "prettyname" : "Runtime",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
SkrRT);

extern "C" void dualX_register_types();

#ifdef SKR_OS_WINDOWS
    #include <shellscalingapi.h>
#endif
void SkrRuntimeModule::on_load(int argc, char** argv)
{
    dualX_register_types();

    SKR_LOG_INFO("SkrRuntime module loaded!");

#ifdef SKR_OS_WINDOWS
    ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    DPIAware = true;
#endif
}
void SkrRuntimeModule::on_unload()
{
    SKR_LOG_INFO("SkrRuntime module unloaded!");
}

SkrRuntimeModule* SkrRuntimeModule::Get()
{
    auto mm = skr_get_module_manager();
    static auto rm = static_cast<SkrRuntimeModule*>(mm->get_module("SkrRT"));
    return rm;
}

RUNTIME_EXTERN_C RUNTIME_API bool skr_runtime_is_dpi_aware()
{
    return SkrRuntimeModule::Get()->DPIAware;
}

RUNTIME_EXTERN_C RUNTIME_API skr::ModuleManager* skr_get_module_manager()
{
    static auto sModuleManager = eastl::make_unique<skr::ModuleManagerImpl>();
    return sModuleManager.get();
}