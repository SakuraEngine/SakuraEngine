#include "skr_scene/skr_scene.h"
#include "module/module_manager.hpp"
#include "utils/log.h"

IMPLEMENT_DYNAMIC_MODULE(SkrSceneModule, SkrScene);

void SkrSceneModule::on_load(int argc, char** argv)
{
    SKR_LOG_TRACE("skr scene loaded!");
}

void SkrSceneModule::on_unload()
{
    SKR_LOG_TRACE("skr scene unloaded!");
}
