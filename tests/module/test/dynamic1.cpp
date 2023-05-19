#include "module/module_manager.hpp"
#include "misc/log.h"

class SDynamicModule1 : public skr::IDynamicModule
{
    virtual void on_load() override
    {
        SKR_LOG_INFO("dynamic module 1 loaded!");
    }
    virtual int main_module_exec() override
    {
        SKR_LOG_INFO("dynamic module 1 executed as main module!");
        return 0;
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("dynamic module 1 unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SDynamicModule1, dynamic1);