#include "SkrCore/module/module_manager.hpp"
#include "SkrCore/log.h"

class SDynamicModule2 : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override
    {
        SKR_LOG_INFO(u8"dynamic module 2 loaded!");
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO(u8"dynamic module 2 unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SDynamicModule2, dynamic2);