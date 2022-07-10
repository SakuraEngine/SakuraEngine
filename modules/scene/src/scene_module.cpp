#include "skr_scene/skr_scene.h"
#include "module/module_manager.hpp"
#include "utils/log.h"

IMPLEMENT_DYNAMIC_MODULE(SkrSceneModule, SkrScene);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "SkrScene",
    "prettyname" : "SakuraScene",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrRT", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
SkrScene)


void SkrSceneModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("skr scene loaded!");
}

void SkrSceneModule::on_unload()
{
    SKR_LOG_INFO("skr scene unloaded!");
}
