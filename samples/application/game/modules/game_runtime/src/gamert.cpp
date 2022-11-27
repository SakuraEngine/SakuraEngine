#include "GameRuntime/gamert.h"
#include "utils/make_zeroed.hpp"
#include "platform/configure.h"
#include "platform/filesystem.hpp"
#include "platform/memory.h"
#include "resource/resource_system.h"
#include "ecs/dual.h"
#include "runtime_module.h"
#include "platform/guid.hpp"

IMPLEMENT_DYNAMIC_MODULE(SGameRTModule, GameRuntime);


void SGameRTModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("game runtime loaded!");
}


int SGameRTModule::main_module_exec(int argc, char** argv)
{
    return 0;
}

void SGameRTModule::on_unload()
{

}

SGameRTModule* SGameRTModule::Get()
{
    auto mm = skr_get_module_manager();
    static auto rm = static_cast<SGameRTModule*>(mm->get_module("GameRuntime"));
    return rm;
}