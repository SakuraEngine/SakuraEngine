#include "SkrCore/module/module.hpp"
#include "SkrCore/log.h"

class SkrInputSystemModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override
    {
        SKR_LOG_INFO(u8"input system loaded!");
    }
    virtual int main_module_exec(int argc, char8_t** argv) override
    {
        SKR_LOG_INFO(u8"input system executed as main module!");
        return 0;
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO(u8"input system unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SkrInputSystemModule, SkrInputSystem);