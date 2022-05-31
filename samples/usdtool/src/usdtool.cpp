#include "module/module_manager.hpp"
#include "utils/log.h"

class SUsdToolModule : public skr::IDynamicModule
{
    virtual void on_load() override
    {
        SKR_LOG_INFO("game tool loaded!");
    }
    virtual void main_module_exec() override
    {
        SKR_LOG_INFO("game tool executed as main module!");
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("game tool unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SUsdToolModule, UsdTool);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "UsdTool",
    "prettyname" : "UsdTool",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
UsdTool)