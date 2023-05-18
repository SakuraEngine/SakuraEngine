#include "module/module_manager.hpp"
#include "misc/log.h"

class SDynamicModule0 : public skr::IDynamicModule
{
    virtual void on_load() override
    {
        SKR_LOG_INFO("dynamic module 0 loaded!");
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("dynamic module 0 unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SDynamicModule0, dynamic0);