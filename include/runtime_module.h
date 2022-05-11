#pragma once
#include "module/module_manager.hpp"
#include "utils/log.h"

class RUNTIME_API SkrRuntimeModule : public skr::IDynamicModule
{
    virtual void on_load() override
    {
        SKR_LOG_INFO("SkrRuntime module loaded!");
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("SkrRuntime module unloaded!");
    }
};