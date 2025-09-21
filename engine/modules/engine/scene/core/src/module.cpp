#include "SkrSceneCore/module.h"
#include "SkrCore/module/module_manager.hpp"
#include "SkrCore/log.h"

IMPLEMENT_DYNAMIC_MODULE(SkrSceneCoreModule, SkrSceneCore);

void SkrSceneCoreModule::on_load(int argc, char8_t** argv)
{
    skr_log_set_level(SKR_LOG_LEVEL_INFO);
    skr_log_initialize_async_worker();
    SKR_LOG_TRACE(u8"skr scene loaded!");
}

void SkrSceneCoreModule::on_unload()
{
    SKR_LOG_TRACE(u8"skr scene unloaded!");
    skr_log_flush();
    skr_log_finalize_async_worker();
}
