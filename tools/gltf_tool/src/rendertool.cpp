#include "module/module_manager.hpp"
#include "utils/log.h"

class SGLTFToolModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override
    {
        SKR_LOG_INFO("render tool loaded!");
    }
    virtual int main_module_exec(int argc, char** argv) override
    {
        SKR_LOG_INFO("render tool executed as main module!");
        return 0;
    }
    virtual void on_unload() override
    {
        SKR_LOG_INFO("render tool unloaded!");
    }
};
IMPLEMENT_DYNAMIC_MODULE(SGLTFToolModule, GLTFTool);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "GLTFTool",
    "prettyname" : "GLTFTool",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
GLTFTool)