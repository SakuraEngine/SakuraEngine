#pragma once
#include "module/module_manager.hpp"
#include "utils/log.h"

class RUNTIME_API SkrRuntimeModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override;
    virtual void on_unload() override;

    static SkrRuntimeModule* Get();

    bool DPIAware = false;
    struct dual_storage_t* ecs_world;
};

RUNTIME_EXTERN_C RUNTIME_API bool skr_runtime_is_dpi_aware();
RUNTIME_EXTERN_C RUNTIME_API struct dual_storage_t* skr_runtime_get_dual_storage();