#include "module/module_manager.hpp"
#include "utils/log.h"

class SDynamicModule3 : public skr::IDynamicModule
{
    virtual void on_load() override
    {
        SKR_LOG_INFO("dynamic module 3 loaded!");
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("dynamic module 3 unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SDynamicModule3, dynamic3);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "dynamic3",
    "prettyname" : "dynamic3",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [{"name":"dynamic2", "version":"0.0.1"}],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
dynamic3);