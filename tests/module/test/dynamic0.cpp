#include "module/module_manager.hpp"
#include "utils/log.h"

class SDynamicModule0 : public skr::IDynamicModule
{
    virtual void on_load() override
    {
        SKR_LOG_INFO("dynamic module 0 loaded!");
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("dynamic module 0 unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SDynamicModule0, dynamic0);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "dynamic0",
    "prettyname" : "dynamic0",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
dynamic0);