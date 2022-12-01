#pragma once
#include "SkrDevCore/module.configure.h"
#include "module/module.hpp"

class SKR_DEVCORE_API SkrDevCoreModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override {}
    virtual void on_unload() override {}
};