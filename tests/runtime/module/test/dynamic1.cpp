#include "SkrCore/module/module_manager.hpp"
#include "SkrCore/log.h"

class SDynamicModule1 : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override
    {
        SKR_LOG_INFO(u8"dynamic module 1 loaded!");
    }
    virtual int main_module_exec(int argc, char8_t** argv) override
    {
        SKR_LOG_INFO(u8"dynamic module 1 executed as main module!");
        return 0;
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO(u8"dynamic module 1 unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SDynamicModule1, dynamic1);