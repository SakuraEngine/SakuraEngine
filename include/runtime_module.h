#pragma once
#include "module/module_manager.hpp"
#include "utils/log.h"

class RUNTIME_API SkrRuntimeModule : public skr::IDynamicModule
{
    virtual void on_load() override;
    virtual void on_unload() override;
};