#pragma once
#include "module/module_manager.hpp"
#include "gainput/gainput.h"

class GAINPUT_LIBEXPORT SkrGAInputModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override;
    virtual void on_unload() override;
};