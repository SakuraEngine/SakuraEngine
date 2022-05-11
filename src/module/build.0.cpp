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