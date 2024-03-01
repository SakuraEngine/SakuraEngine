#include "SkrScene/skr_scene.h"
#include "SkrModule/module_manager.hpp"
#include "SkrCore/log.h"

IMPLEMENT_DYNAMIC_MODULE(SkrSceneModule, SkrScene);

void SkrSceneModule::on_load(int argc, char8_t** argv)
{
    SKR_LOG_TRACE(u8"skr scene loaded!");
}

void SkrSceneModule::on_unload()
{
    SKR_LOG_TRACE(u8"skr scene unloaded!");
}
