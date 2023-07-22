#include "SkrRT/module/module_manager.hpp"
#include "SkrRT/misc/log.h"

class SGameToolModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override
    {
        SKR_LOG_INFO(u8"game tool loaded!");
    }
    virtual int main_module_exec(int argc, char8_t** argv) override
    {
        SKR_LOG_INFO(u8"game tool executed as main module!");
        return 0;
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO(u8"game tool unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SGameToolModule, GameTool);