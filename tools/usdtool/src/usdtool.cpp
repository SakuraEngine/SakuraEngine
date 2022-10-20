#include "UsdCore/core.hpp"
#include "module/module_manager.hpp"
#include "utils/log.h"

class SUsdToolModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override
    {
        auto error_code = skd::USDCoreInitialize();
        if (error_code != 0) 
        {
            SKR_LOG_ERROR("USDCoreInitialize failed!");
        }
        SKR_LOG_INFO("game tool loaded!");
    }
    virtual int main_module_exec(int argc, char** argv) override
    {
        SKR_LOG_INFO("game tool executed as main module!");
        return 0;
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("game tool unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SUsdToolModule, UsdTool);