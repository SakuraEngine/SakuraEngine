#pragma once
#include "SkrBase/config.h"
#include "SkrModule/module.hpp"

class SKR_ANIM_API SkrAnimModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char8_t** argv) override {}
    virtual void on_unload() override {}
};