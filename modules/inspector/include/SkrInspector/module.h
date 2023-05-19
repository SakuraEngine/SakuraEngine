#pragma once
#include "SkrInspector/module.configure.h"
#include "module/module.hpp"

class SKR_INSPECT_API SkrInspectorModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char8_t** argv) override {}
    virtual void on_unload() override {}
};