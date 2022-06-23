#include "platform/configure.h"
#include "imgui/skr_imgui.config.h"
#include "utils/log.h"
#include "imgui_skr_input.cpp"
#include "skr_imgui_rg.cpp"

IMPLEMENT_DYNAMIC_MODULE(SkrImGuiModule, SkrImGui);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "SkrImGui",
    "prettyname" : "SakuraImGui",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrRT", "version":"0.1.0"},
        {"name":"SkrRenderGraph", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
SkrImGui)

void SkrImGuiModule::on_load()
{
    SKR_LOG_INFO("skr imgui loaded!");
}

void SkrImGuiModule::on_unload()
{
    SKR_LOG_INFO("skr imgui unloaded!");
}