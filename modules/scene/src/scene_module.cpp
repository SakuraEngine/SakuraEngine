#include "SkrScene/skr_scene.h"
#include "module/module_manager.hpp"
#include "misc/log.h"

IMPLEMENT_DYNAMIC_MODULE(SkrSceneModule, SkrScene);

void SkrSceneModule::on_load(int argc, char8_t** argv)
{
    SKR_LOG_TRACE("skr scene loaded!");
}

void SkrSceneModule::on_unload()
{
    SKR_LOG_TRACE("skr scene unloaded!");
}
