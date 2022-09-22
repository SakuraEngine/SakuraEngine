#include "module/module_manager.hpp"
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
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "SkrInputSystem",
    "prettyname" : "Sakura input system",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrRT", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
SkrInputSystem)