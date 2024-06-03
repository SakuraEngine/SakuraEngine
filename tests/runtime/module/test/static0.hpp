#pragma once
#include "SkrCore/module/module_manager.hpp"
#include "SkrCore/log.h"

class SStaticModule0 : public skr::IStaticModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual void on_unload() override;
    virtual const char8_t* get_meta_data(void) override;
};
IMPLEMENT_STATIC_MODULE(SStaticModule0, static0);