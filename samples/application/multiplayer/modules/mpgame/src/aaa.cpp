#include "module/module_manager.hpp"
#include "misc/log.h"

class SMPModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override
    {
        SKR_LOG_INFO("mpgame loaded!");
    }
    virtual int main_module_exec(int argc, char8_t** argv) override
    {
        SKR_LOG_INFO("mpgame executed as main module!");
        return 0;
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("mpgame unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SMPModule, MPGame);