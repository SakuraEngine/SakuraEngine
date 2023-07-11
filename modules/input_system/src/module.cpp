#include "SkrRT/module/module.hpp"
#include "SkrRT/misc/log.h"

class SkrInputSystemModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override
    {
        SKR_LOG_INFO("input system loaded!");
    }
    virtual int main_module_exec(int argc, char8_t** argv) override
    {
        SKR_LOG_INFO("input system executed as main module!");
        return 0;
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("input system unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SkrInputSystemModule, SkrInputSystem);