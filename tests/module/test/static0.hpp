#pragma once
#include "module/module_manager.hpp"
#include "utils/log.h"

class SStaticModule0 : public skr::IStaticModule
{
    virtual void on_load() override;
    virtual void on_unload() override;
    virtual const char* get_meta_data(void) override;
};
IMPLEMENT_STATIC_MODULE(SStaticModule0, static0);