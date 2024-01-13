#include "SkrRenderGraph/api.h"
#include "SkrCore/log.h"

IMPLEMENT_DYNAMIC_MODULE(SkrRenderGraphModule, SkrRenderGraph);

void SkrRenderGraphModule::on_load(int argc, char8_t** argv)
{
    SKR_LOG_TRACE(u8"skr render graph loaded!");
}

void SkrRenderGraphModule::on_unload()
{
    SKR_LOG_TRACE(u8"skr render graph unloaded!");
}