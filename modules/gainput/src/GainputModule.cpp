#include "gainput/GainputModule.h"
#include "utils/log.h"

IMPLEMENT_DYNAMIC_MODULE(SkrGAInputModule, SkrGAInput);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "SkrGAInput",
    "prettyname" : "SakuraGAInput",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [{"name":"SkrRT", "version":"0.1.0"}],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
SkrGAInput)

void SkrGAInputModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("skr gainput loaded!");
}

void SkrGAInputModule::on_unload()
{
    SKR_LOG_INFO("skr gainput unloaded!");
}