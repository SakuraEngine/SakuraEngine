#pragma once
#include "module/module_manager.hpp"
#include "platform/thread.h"
#include "misc/log.h"
#include "platform/shared_library.hpp"
#include "platform/dstorage.h"

class RUNTIME_API SkrRuntimeModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char8_t** argv) override;
    virtual void on_unload() override;

    static SkrRuntimeModule* Get();

    SMutex log_mutex;
    bool DPIAware = false;
    SkrDStorageInstanceId dstorageInstance = nullptr;
    skr::SharedLibrary tracyLibrary;
};

RUNTIME_EXTERN_C RUNTIME_API bool skr_runtime_is_dpi_aware();
RUNTIME_EXTERN_C RUNTIME_API SkrDStorageInstanceId skr_runtime_get_dstorage_instance();