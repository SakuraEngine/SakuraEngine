#include "module/module_manager.hpp"
#include "utils/log.h"

class SDynamicModule1 : public skr::IDynamicModule
{
    virtual void on_load() override
    {
        SKR_LOG_INFO("dynamic module 1 loaded!");
    }
    virtual void main_module_exec() override
    {
        SKR_LOG_INFO("dynamic module 1 executed as main module!");
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("dynamic module 1 unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SDynamicModule1, dynamic1);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "dynamic1",
    "prettyname" : "dynamic1",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [{"name":"static0", "version":"0.0.1"}],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
dynamic1)