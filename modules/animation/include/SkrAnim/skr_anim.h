#pragma once
#include "SkrAnim/module.configure.h"
#include "module/module.hpp"

class SKR_ANIM_API SkrAnimModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override {}
    virtual void on_unload() override {}
};