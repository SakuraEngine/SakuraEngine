#include "module/module_manager.hpp"
#include "misc/log.h"

class SDynamicModule2 : public skr::IDynamicModule
{
    virtual void on_load() override
    {
        SKR_LOG_INFO("dynamic module 2 loaded!");
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("dynamic module 2 unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SDynamicModule2, dynamic2);