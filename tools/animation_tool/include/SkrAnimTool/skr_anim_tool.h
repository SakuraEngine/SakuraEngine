#pragma once
#include "SkrAnimTool/module.configure.h"
#include "module/module.hpp"

class SKR_ANIMTOOL_API SkrAnimToolModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override {}
    virtual void on_unload() override {}
};