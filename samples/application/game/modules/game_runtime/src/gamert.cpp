#include "GameRuntime/gamert.h"
#include "SkrRT/misc/make_zeroed.hpp"
#include "SkrRT/platform/configure.h"
#include "SkrRT/platform/filesystem.hpp"
#include "SkrRT/platform/memory.h"
#include "SkrRT/resource/resource_system.h"
#include "SkrRT/ecs/dual.h"
#include "SkrRT/runtime_module.h"
#include "SkrRT/platform/guid.hpp"

IMPLEMENT_DYNAMIC_MODULE(SGameRTModule, GameRuntime);


void SGameRTModule::on_load(int argc, char8_t** argv)
{
    SKR_LOG_INFO(u8"game runtime loaded!");
}


int SGameRTModule::main_module_exec(int argc, char8_t** argv)
{
    return 0;
}

void SGameRTModule::on_unload()
{

}

SGameRTModule* SGameRTModule::Get()
{
    auto mm = skr_get_module_manager();
    static auto rm = static_cast<SGameRTModule*>(mm->get_module(u8"GameRuntime"));
    return rm;
}