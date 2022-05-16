#include "module_manager.cpp"
#include "runtime_module.h"

IMPLEMENT_DYNAMIC_MODULE(SkrRuntimeModule, SkrRT);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "SkrRT",
    "prettyname" : "Runtime",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
SkrRT);

extern "C" void dualX_register_types();
void SkrRuntimeModule::on_load()
{
    dualX_register_types();
    
    SKR_LOG_INFO("SkrRuntime module loaded!");
}
void SkrRuntimeModule::on_unload()
{
    SKR_LOG_INFO("SkrRuntime module unloaded!");
}
    