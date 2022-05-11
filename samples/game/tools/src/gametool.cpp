#include "module/module_manager.hpp"
#include "utils/log.h"

class SGameToolModule : public skr::IDynamicModule
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
IMPLEMENT_DYNAMIC_MODULE(SGameToolModule, GameTool);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "GameTool",
    "prettyname" : "GameTool",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
GameTool)