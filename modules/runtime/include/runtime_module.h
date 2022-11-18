#pragma once
#include "module/module_manager.hpp"
#include "platform/thread.h"
#include "utils/log.h"

class RUNTIME_API SkrRuntimeModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override;
    virtual void on_unload() override;

    static SkrRuntimeModule* Get();

    SMutex log_mutex;
    bool DPIAware = false;
};

RUNTIME_EXTERN_C RUNTIME_API bool skr_runtime_is_dpi_aware();