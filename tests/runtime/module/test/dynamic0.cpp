#include "SkrCore/module/module_manager.hpp"
#include "SkrCore/log.h"

class SDynamicModule0 : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override
    {
        SKR_LOG_INFO(u8"dynamic module 0 loaded!");
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO(u8"dynamic module 0 unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SDynamicModule0, dynamic0);