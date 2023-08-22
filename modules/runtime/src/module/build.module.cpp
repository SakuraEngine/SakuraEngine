#include "module_manager.cpp"
#include "SkrRT/platform/configure.h"
#include "SkrRT/platform/crash.h"
#include "SkrRT/runtime_module.h"
#include "SkrRT/ecs/dual.h"

#include "SkrProfile/profile.h"

IMPLEMENT_DYNAMIC_MODULE(SkrRuntimeModule, SkrRT);

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
        skr_mutex_acquire((SMutex*)pMutex);
    }
    else
    {
        skr_mutex_release((SMutex*)pMutex);
    }
};

void SkrRuntimeModule::on_load(int argc, char8_t** argv)
{
    // set lock for log
    skr_initialize_crash_handler();
    skr_log_initialize_async_worker();

    SkrDStorageConfig config = {};
    dstorageInstance = skr_create_dstorage_instance(&config);

#ifdef SKR_OS_WINDOWS
    ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    DPIAware = true;
#endif

    SKR_LOG_TRACE(u8"SkrRuntime module loaded!");
}

void SkrRuntimeModule::on_unload()
{
    dual_shutdown();

    skr_runtime_free_dstorage_instance();

    SKR_LOG_TRACE(u8"SkrRuntime module unloaded!");
    skr_log_finalize_async_worker();
    skr_finalize_crash_handler();

#ifdef SKR_PROFILE_ENABLE
    if (tracy::GetProfiler().IsConnected())
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
        tracy::GetProfiler().RequestShutdown();
        while( !tracy::GetProfiler().HasShutdownFinished() ) { std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) ); };
    }
    tracyLibrary.unload();
#endif
}

SkrRuntimeModule* SkrRuntimeModule::Get()
{
    auto mm = skr_get_module_manager();
    static auto rm = static_cast<SkrRuntimeModule*>(mm->get_module(u8"SkrRT"));
    return rm;
}

SKR_EXTERN_C SKR_RUNTIME_API bool skr_runtime_is_dpi_aware()
{
    if (!SkrRuntimeModule::Get()) 
    {
#ifdef SKR_OS_WINDOWS
        PROCESS_DPI_AWARENESS awareness;
        GetProcessDpiAwareness(NULL, &awareness);
        return (awareness == PROCESS_PER_MONITOR_DPI_AWARE);
#endif
        return false;
    }
    return SkrRuntimeModule::Get()->DPIAware;
}

SkrDStorageInstanceId skr_runtime_get_dstorage_instance()
{
    if (auto rtModule = SkrRuntimeModule::Get()) 
    {
        return rtModule->dstorageInstance;
    }
    return nullptr;
}

void skr_runtime_free_dstorage_instance()
{
    if (auto rtModule = SkrRuntimeModule::Get()) 
    {
#ifdef _WIN32
        if (auto service = rtModule->dstroageDecompressService)
            skr_win_dstorage_free_decompress_service(service);
#endif
        if (auto inst = skr_runtime_get_dstorage_instance())
        {
            skr_free_dstorage_instance(inst);
        }
    }
}

SKR_EXTERN_C SKR_RUNTIME_API skr::ModuleManager* skr_get_module_manager()
{
    static auto sModuleManager = eastl::make_unique<skr::ModuleManagerImpl>();
    return sModuleManager.get();
}

#ifdef _WIN32

skr_win_dstorage_decompress_service_id skr_runtime_create_win_dstorage_decompress_service(const skr_win_dstorage_decompress_desc_t* desc)
{
    if (!SkrRuntimeModule::Get()) 
    {
        return skr_win_dstorage_create_decompress_service(desc);
    }
    SkrRuntimeModule::Get()->dstroageDecompressService = skr_win_dstorage_create_decompress_service(desc);
    return SkrRuntimeModule::Get()->dstroageDecompressService;
}

skr_win_dstorage_decompress_service_id skr_runtime_get_win_dstorage_decompress_service()
{
    if (!SkrRuntimeModule::Get()) 
        return nullptr;
    return SkrRuntimeModule::Get()->dstroageDecompressService;
}

#endif

