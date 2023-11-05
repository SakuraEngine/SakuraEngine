#pragma once
#include "SkrGameAINav/module.configure.h"
#include "SkrRT/module/module.hpp"

class SKR_GAMEAI_NAV_API SkrGameAINavModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char8_t** argv) override {}
    virtual void on_unload() override {}
};