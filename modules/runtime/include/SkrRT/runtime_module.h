#pragma once
#include "SkrRT/module/module_manager.hpp"
#include "SkrRT/platform/thread.h"
#include "SkrRT/misc/log.h"
#include "SkrRT/platform/shared_library.hpp"
#include "SkrRT/platform/dstorage.h"
#ifdef _WIN32
#include "SkrRT/platform/win/dstorage_windows.h"
#endif

class RUNTIME_API SkrRuntimeModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char8_t** argv) override;
    virtual void on_unload() override;

    static SkrRuntimeModule* Get();

    bool DPIAware = false;
    skr::SharedLibrary tracyLibrary;
    SkrDStorageInstanceId dstorageInstance = nullptr;
#ifdef _WIN32
    skr_win_dstorage_decompress_service_id dstroageDecompressService = nullptr;
#endif
};

RUNTIME_EXTERN_C RUNTIME_API bool skr_runtime_is_dpi_aware();
RUNTIME_EXTERN_C RUNTIME_API SkrDStorageInstanceId skr_runtime_get_dstorage_instance();

#ifdef _WIN32
RUNTIME_EXTERN_C RUNTIME_API skr_win_dstorage_decompress_service_id 
skr_runtime_create_win_dstorage_decompress_service(const skr_win_dstorage_decompress_desc_t* desc);

RUNTIME_EXTERN_C RUNTIME_API skr_win_dstorage_decompress_service_id 
skr_runtime_get_win_dstorage_decompress_service();
#endif
