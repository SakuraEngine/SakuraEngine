#pragma once
#include "module/module_manager.hpp"
#include "gainput/gainput.h"

class GAINPUT_LIBEXPORT SkrGAInputModule : public skr::IDynamicModule
{
public:
    virtual void on_load() override;
    virtual void on_unload() override;
};