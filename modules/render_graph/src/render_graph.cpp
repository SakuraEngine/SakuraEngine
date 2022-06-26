#include "render_graph/api.h"
#include "utils/log.h"

IMPLEMENT_DYNAMIC_MODULE(SkrRenderGraphModule, SkrRenderGraph);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "SkrRenderGraph",
    "prettyname" : "SakuraRenderGraph",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [{"name":"SkrRT", "version":"0.1.0"}],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
SkrRenderGraph)

void SkrRenderGraphModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("skr render graph loaded!");
}

void SkrRenderGraphModule::on_unload()
{
    SKR_LOG_INFO("skr render graph unloaded!");
}