#include "platform/configure.h"
#include "imgui/skr_imgui.config.h"
#include "utils/log.h"
#include "imgui_skr_system.cpp"
#include "skr_imgui_rg.cpp"

IMPLEMENT_DYNAMIC_MODULE(SkrImGuiModule, SkrImGui);

void SkrImGuiModule::on_load(int argc, char** argv)
{
    SKR_LOG_TRACE("skr imgui loaded!");
}

void SkrImGuiModule::on_unload()
{
    SKR_LOG_TRACE("skr imgui unloaded!");
}