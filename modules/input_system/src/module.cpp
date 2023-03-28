#define CONTAINER_LITE_IMPL
#include "module/module.hpp"
#include "utils/log.h"

class SkrInputSystemModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override
    {
        SKR_LOG_INFO("input system loaded!");
    }
    virtual int main_module_exec(int argc, char** argv) override
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