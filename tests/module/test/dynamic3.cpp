#include "module/module_manager.hpp"
#include "misc/log.h"

class SDynamicModule3 : public skr::IDynamicModule
{
    virtual void on_load() override
    {
        SKR_LOG_INFO("dynamic module 3 loaded!");
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("dynamic module 3 unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SDynamicModule3, dynamic3);