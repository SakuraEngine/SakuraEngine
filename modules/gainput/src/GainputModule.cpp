#include "gainput/GainputModule.h"
#include "utils/log.h"

IMPLEMENT_DYNAMIC_MODULE(SkrGAInputModule, SkrGAInput);

void SkrGAInputModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("skr gainput loaded!");
}

void SkrGAInputModule::on_unload()
{
    SKR_LOG_INFO("skr gainput unloaded!");
}