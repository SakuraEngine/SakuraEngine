#pragma once
#include "module/module_manager.hpp"
#include "utils/log.h"

class RUNTIME_API SkrRuntimeModule : public skr::IDynamicModule
{
public:
    virtual void on_load() override;
    virtual void on_unload() override;

    static SkrRuntimeModule* Get();

    bool DPIAware = false;
};

RUNTIME_EXTERN_C RUNTIME_API bool skr_runtime_is_dpi_aware();