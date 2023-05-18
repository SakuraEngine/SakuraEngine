#include "SkrRenderGraph/api.h"
#include "misc/log.h"

IMPLEMENT_DYNAMIC_MODULE(SkrRenderGraphModule, SkrRenderGraph);

void SkrRenderGraphModule::on_load(int argc, char8_t** argv)
{
    SKR_LOG_TRACE("skr render graph loaded!");
}

void SkrRenderGraphModule::on_unload()
{
    SKR_LOG_TRACE("skr render graph unloaded!");
}