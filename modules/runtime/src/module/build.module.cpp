#include "module_manager.cpp"
#include "runtime_module.h"
#include "ecs/dual.h"

IMPLEMENT_DYNAMIC_MODULE(SkrRuntimeModule, SkrRT);

extern "C" void dualX_register_types();

#ifdef SKR_OS_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <shellscalingapi.h>
    #pragma comment(lib, "Shcore.lib")
#endif

auto log_locker = +[](bool isLocked, void* pMutex){
    if (isLocked)
    {
        skr_acquire_mutex((SMutex*)pMutex);
    }
    else
    {
        skr_release_mutex((SMutex*)pMutex);
    }
};

void SkrRuntimeModule::on_load(int argc, char** argv)
{
#ifdef TRACY_ENABLE
    tracyLibrary.load("TracyClient");
#endif
    // set lock for log
    skr_init_mutex_recursive(&log_mutex);
    log_set_lock(log_locker, &log_mutex);

    dualX_register_types();

    SKR_LOG_TRACE("SkrRuntime module loaded!");

#ifdef SKR_OS_WINDOWS
    ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    DPIAware = true;
#endif
}
void SkrRuntimeModule::on_unload()
{
    SKR_LOG_TRACE("SkrRuntime module unloaded!");

    skr_destroy_mutex(&log_mutex);

#ifdef TRACY_ENABLE
    tracyLibrary.unload();
#endif
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