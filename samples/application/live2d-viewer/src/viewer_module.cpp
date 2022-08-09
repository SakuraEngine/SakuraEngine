#include "utils/log.h"
#include "imgui/skr_imgui.h"
#include "imgui/skr_imgui_rg.h"
#include "imgui/imgui.h"

class SLive2DViewerModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual int main_module_exec(int argc, char** argv) override;
    virtual void on_unload() override;
};

IMPLEMENT_DYNAMIC_MODULE(SLive2DViewerModule, Live2DViewer);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "Live2DViewer",
    "prettyname" : "Live2DViewer",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrLive2D", "version":"0.1.0"},
        {"name":"SkrImGui", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
Live2DViewer)

void SLive2DViewerModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("live2d viewer loaded!");
}

void SLive2DViewerModule::on_unload()
{
    SKR_LOG_INFO("live2d viewer unloaded!");
}

int SLive2DViewerModule::main_module_exec(int argc, char** argv)
{
    SKR_LOG_INFO("live2d viewer executed!");

    return 0;
}