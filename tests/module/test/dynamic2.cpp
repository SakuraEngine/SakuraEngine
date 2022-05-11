#include "module/module_manager.hpp"
#include "utils/log.h"

class SDynamicModule2 : public skr::IDynamicModule
{
    virtual void on_load() override
    {
        SKR_LOG_INFO("dynamic module 2 loaded!");
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("dynamic module 2 unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SDynamicModule2, dynamic2);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "dynamic2",
    "prettyname" : "dynamic2",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [{"name":"dynamic1", "version":"0.0.1"}],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
dynamic2);